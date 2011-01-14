/*
 * Realtek RTL8366 SMI interface driver
 *
 * Copyright (C) 2009-2010 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>

#ifdef CONFIG_RTL8366S_PHY_DEBUG_FS
#include <linux/debugfs.h>
#endif

#include "rtl8366_smi.h"

#define RTL8366_SMI_ACK_RETRY_COUNT         5
#define RTL8366_SMI_CLK_DELAY               10 /* nsec */

static inline void rtl8366_smi_clk_delay(struct rtl8366_smi *smi)
{
	ndelay(RTL8366_SMI_CLK_DELAY);
}

static void rtl8366_smi_start(struct rtl8366_smi *smi)
{
	unsigned int sda = smi->gpio_sda;
	unsigned int sck = smi->gpio_sck;

	/*
	 * Set GPIO pins to output mode, with initial state:
	 * SCK = 0, SDA = 1
	 */
	gpio_direction_output(sck, 0);
	gpio_direction_output(sda, 1);
	rtl8366_smi_clk_delay(smi);

	/* CLK 1: 0 -> 1, 1 -> 0 */
	gpio_set_value(sck, 1);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 0);
	rtl8366_smi_clk_delay(smi);

	/* CLK 2: */
	gpio_set_value(sck, 1);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sda, 0);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 0);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sda, 1);
}

static void rtl8366_smi_stop(struct rtl8366_smi *smi)
{
	unsigned int sda = smi->gpio_sda;
	unsigned int sck = smi->gpio_sck;

	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sda, 0);
	gpio_set_value(sck, 1);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sda, 1);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 1);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 0);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 1);

	/* add a click */
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 0);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 1);

	/* set GPIO pins to input mode */
	gpio_direction_input(sda);
	gpio_direction_input(sck);
}

static void rtl8366_smi_write_bits(struct rtl8366_smi *smi, u32 data, u32 len)
{
	unsigned int sda = smi->gpio_sda;
	unsigned int sck = smi->gpio_sck;

	for (; len > 0; len--) {
		rtl8366_smi_clk_delay(smi);

		/* prepare data */
		gpio_set_value(sda, !!(data & ( 1 << (len - 1))));
		rtl8366_smi_clk_delay(smi);

		/* clocking */
		gpio_set_value(sck, 1);
		rtl8366_smi_clk_delay(smi);
		gpio_set_value(sck, 0);
	}
}

static void rtl8366_smi_read_bits(struct rtl8366_smi *smi, u32 len, u32 *data)
{
	unsigned int sda = smi->gpio_sda;
	unsigned int sck = smi->gpio_sck;

	gpio_direction_input(sda);

	for (*data = 0; len > 0; len--) {
		u32 u;

		rtl8366_smi_clk_delay(smi);

		/* clocking */
		gpio_set_value(sck, 1);
		rtl8366_smi_clk_delay(smi);
		u = !!gpio_get_value(sda);
		gpio_set_value(sck, 0);

		*data |= (u << (len - 1));
	}

	gpio_direction_output(sda, 0);
}

static int rtl8366_smi_wait_for_ack(struct rtl8366_smi *smi)
{
	int retry_cnt;

	retry_cnt = 0;
	do {
		u32 ack;

		rtl8366_smi_read_bits(smi, 1, &ack);
		if (ack == 0)
			break;

		if (++retry_cnt > RTL8366_SMI_ACK_RETRY_COUNT)
			return -EIO;
	} while (1);

	return 0;
}

static int rtl8366_smi_write_byte(struct rtl8366_smi *smi, u8 data)
{
	rtl8366_smi_write_bits(smi, data, 8);
	return rtl8366_smi_wait_for_ack(smi);
}

static int rtl8366_smi_read_byte0(struct rtl8366_smi *smi, u8 *data)
{
	u32 t;

	/* read data */
	rtl8366_smi_read_bits(smi, 8, &t);
	*data = (t & 0xff);

	/* send an ACK */
	rtl8366_smi_write_bits(smi, 0x00, 1);

	return 0;
}

static int rtl8366_smi_read_byte1(struct rtl8366_smi *smi, u8 *data)
{
	u32 t;

	/* read data */
	rtl8366_smi_read_bits(smi, 8, &t);
	*data = (t & 0xff);

	/* send an ACK */
	rtl8366_smi_write_bits(smi, 0x01, 1);

	return 0;
}

int rtl8366_smi_read_reg(struct rtl8366_smi *smi, u32 addr, u32 *data)
{
	unsigned long flags;
	u8 lo = 0;
	u8 hi = 0;
	int ret;

	spin_lock_irqsave(&smi->lock, flags);

	rtl8366_smi_start(smi);

	/* send READ command */
	ret = rtl8366_smi_write_byte(smi, 0x0a << 4 | 0x04 << 1 | 0x01);
	if (ret)
		goto out;

	/* set ADDR[7:0] */
	ret = rtl8366_smi_write_byte(smi, addr & 0xff);
	if (ret)
		goto out;

	/* set ADDR[15:8] */
	ret = rtl8366_smi_write_byte(smi, addr >> 8);
	if (ret)
		goto out;

	/* read DATA[7:0] */
	rtl8366_smi_read_byte0(smi, &lo);
	/* read DATA[15:8] */
	rtl8366_smi_read_byte1(smi, &hi);

	*data = ((u32) lo) | (((u32) hi) << 8);

	ret = 0;

 out:
	rtl8366_smi_stop(smi);
	spin_unlock_irqrestore(&smi->lock, flags);

	return ret;
}
EXPORT_SYMBOL_GPL(rtl8366_smi_read_reg);

int rtl8366_smi_write_reg(struct rtl8366_smi *smi, u32 addr, u32 data)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&smi->lock, flags);

	rtl8366_smi_start(smi);

	/* send WRITE command */
	ret = rtl8366_smi_write_byte(smi, 0x0a << 4 | 0x04 << 1 | 0x00);
	if (ret)
		goto out;

	/* set ADDR[7:0] */
	ret = rtl8366_smi_write_byte(smi, addr & 0xff);
	if (ret)
		goto out;

	/* set ADDR[15:8] */
	ret = rtl8366_smi_write_byte(smi, addr >> 8);
	if (ret)
		goto out;

	/* write DATA[7:0] */
	ret = rtl8366_smi_write_byte(smi, data & 0xff);
	if (ret)
		goto out;

	/* write DATA[15:8] */
	ret = rtl8366_smi_write_byte(smi, data >> 8);
	if (ret)
		goto out;

	ret = 0;

 out:
	rtl8366_smi_stop(smi);
	spin_unlock_irqrestore(&smi->lock, flags);

	return ret;
}
EXPORT_SYMBOL_GPL(rtl8366_smi_write_reg);

int rtl8366_smi_rmwr(struct rtl8366_smi *smi, u32 addr, u32 mask, u32 data)
{
	u32 t;
	int err;

	err = rtl8366_smi_read_reg(smi, addr, &t);
	if (err)
		return err;

	err = rtl8366_smi_write_reg(smi, addr, (t & ~mask) | data);
	return err;

}
EXPORT_SYMBOL_GPL(rtl8366_smi_rmwr);

static int rtl8366_mc_is_used(struct rtl8366_smi *smi, int mc_index, int *used)
{
	int err;
	int i;

	*used = 0;
	for (i = 0; i < smi->num_ports; i++) {
		int index = 0;

		err = smi->ops->get_mc_index(smi, i, &index);
		if (err)
			return err;

		if (mc_index == index) {
			*used = 1;
			break;
		}
	}

	return 0;
}

static int rtl8366_set_vlan(struct rtl8366_smi *smi, int vid, u32 member,
			    u32 untag, u32 fid)
{
	struct rtl8366_vlan_4k vlan4k;
	int err;
	int i;

	/* Update the 4K table */
	err = smi->ops->get_vlan_4k(smi, vid, &vlan4k);
	if (err)
		return err;

	vlan4k.member = member;
	vlan4k.untag = untag;
	vlan4k.fid = fid;
	err = smi->ops->set_vlan_4k(smi, &vlan4k);
	if (err)
		return err;

	/* Try to find an existing MC entry for this VID */
	for (i = 0; i < smi->num_vlan_mc; i++) {
		struct rtl8366_vlan_mc vlanmc;

		err = smi->ops->get_vlan_mc(smi, i, &vlanmc);
		if (err)
			return err;

		if (vid == vlanmc.vid) {
			/* update the MC entry */
			vlanmc.member = member;
			vlanmc.untag = untag;
			vlanmc.fid = fid;

			err = smi->ops->set_vlan_mc(smi, i, &vlanmc);
			break;
		}
	}

	return err;
}

static int rtl8366_get_pvid(struct rtl8366_smi *smi, int port, int *val)
{
	struct rtl8366_vlan_mc vlanmc;
	int err;
	int index;

	err = smi->ops->get_mc_index(smi, port, &index);
	if (err)
		return err;

	err = smi->ops->get_vlan_mc(smi, index, &vlanmc);
	if (err)
		return err;

	*val = vlanmc.vid;
	return 0;
}

static int rtl8366_set_pvid(struct rtl8366_smi *smi, unsigned port,
			    unsigned vid)
{
	struct rtl8366_vlan_mc vlanmc;
	struct rtl8366_vlan_4k vlan4k;
	int err;
	int i;

	/* Try to find an existing MC entry for this VID */
	for (i = 0; i < smi->num_vlan_mc; i++) {
		err = smi->ops->get_vlan_mc(smi, i, &vlanmc);
		if (err)
			return err;

		if (vid == vlanmc.vid) {
			err = smi->ops->set_vlan_mc(smi, i, &vlanmc);
			if (err)
				return err;

			err = smi->ops->set_mc_index(smi, port, i);
			return err;
		}
	}

	/* We have no MC entry for this VID, try to find an empty one */
	for (i = 0; i < smi->num_vlan_mc; i++) {
		err = smi->ops->get_vlan_mc(smi, i, &vlanmc);
		if (err)
			return err;

		if (vlanmc.vid == 0 && vlanmc.member == 0) {
			/* Update the entry from the 4K table */
			err = smi->ops->get_vlan_4k(smi, vid, &vlan4k);
			if (err)
				return err;

			vlanmc.vid = vid;
			vlanmc.member = vlan4k.member;
			vlanmc.untag = vlan4k.untag;
			vlanmc.fid = vlan4k.fid;
			err = smi->ops->set_vlan_mc(smi, i, &vlanmc);
			if (err)
				return err;

			err = smi->ops->set_mc_index(smi, port, i);
			return err;
		}
	}

	/* MC table is full, try to find an unused entry and replace it */
	for (i = 0; i < smi->num_vlan_mc; i++) {
		int used;

		err = rtl8366_mc_is_used(smi, i, &used);
		if (err)
			return err;

		if (!used) {
			/* Update the entry from the 4K table */
			err = smi->ops->get_vlan_4k(smi, vid, &vlan4k);
			if (err)
				return err;

			vlanmc.vid = vid;
			vlanmc.member = vlan4k.member;
			vlanmc.untag = vlan4k.untag;
			vlanmc.fid = vlan4k.fid;
			err = smi->ops->set_vlan_mc(smi, i, &vlanmc);
			if (err)
				return err;

			err = smi->ops->set_mc_index(smi, port, i);
			return err;
		}
	}

	dev_err(smi->parent,
		"all VLAN member configurations are in use\n");

	return -ENOSPC;
}

int rtl8366_enable_vlan(struct rtl8366_smi *smi, int enable)
{
	int err;

	err = smi->ops->enable_vlan(smi, enable);
	if (err)
		return err;

	smi->vlan_enabled = enable;

	if (!enable) {
		smi->vlan4k_enabled = 0;
		err = smi->ops->enable_vlan4k(smi, enable);
	}

	return err;
}
EXPORT_SYMBOL_GPL(rtl8366_enable_vlan);

static int rtl8366_enable_vlan4k(struct rtl8366_smi *smi, int enable)
{
	int err;

	if (enable) {
		err = smi->ops->enable_vlan(smi, enable);
		if (err)
			return err;

		smi->vlan_enabled = enable;
	}

	err = smi->ops->enable_vlan4k(smi, enable);
	if (err)
		return err;

	smi->vlan4k_enabled = enable;
	return 0;
}

int rtl8366_enable_all_ports(struct rtl8366_smi *smi, int enable)
{
	int port;
	int err;

	for (port = 0; port < smi->num_ports; port++) {
		err = smi->ops->enable_port(smi, port, enable);
		if (err)
			return err;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(rtl8366_enable_all_ports);

int rtl8366_reset_vlan(struct rtl8366_smi *smi)
{
	struct rtl8366_vlan_mc vlanmc;
	int err;
	int i;

	rtl8366_enable_vlan(smi, 0);
	rtl8366_enable_vlan4k(smi, 0);

	/* clear VLAN member configurations */
	vlanmc.vid = 0;
	vlanmc.priority = 0;
	vlanmc.member = 0;
	vlanmc.untag = 0;
	vlanmc.fid = 0;
	for (i = 0; i < smi->num_vlan_mc; i++) {
		err = smi->ops->set_vlan_mc(smi, i, &vlanmc);
		if (err)
			return err;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(rtl8366_reset_vlan);

static int rtl8366_init_vlan(struct rtl8366_smi *smi)
{
	int port;
	int err;

	err = rtl8366_reset_vlan(smi);
	if (err)
		return err;

	for (port = 0; port < smi->num_ports; port++) {
		u32 mask;

		if (port == smi->cpu_port)
			mask = (1 << smi->num_ports) - 1;
		else
			mask = (1 << port) | (1 << smi->cpu_port);

		err = rtl8366_set_vlan(smi, (port + 1), mask, mask, 0);
		if (err)
			return err;

		err = rtl8366_set_pvid(smi, port, (port + 1));
		if (err)
			return err;
	}

	return rtl8366_enable_vlan(smi, 1);
}

#ifdef CONFIG_RTL8366S_PHY_DEBUG_FS
int rtl8366_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}
EXPORT_SYMBOL_GPL(rtl8366_debugfs_open);

static ssize_t rtl8366_read_debugfs_vlan_mc(struct file *file,
					      char __user *user_buf,
					      size_t count, loff_t *ppos)
{
	struct rtl8366_smi *smi = (struct rtl8366_smi *)file->private_data;
	int i, len = 0;
	char *buf = smi->buf;

	len += snprintf(buf + len, sizeof(smi->buf) - len,
			"%2s %6s %4s %6s %6s %3s\n",
			"id", "vid","prio", "member", "untag", "fid");

	for (i = 0; i < smi->num_vlan_mc; ++i) {
		struct rtl8366_vlan_mc vlanmc;

		smi->ops->get_vlan_mc(smi, i, &vlanmc);

		len += snprintf(buf + len, sizeof(smi->buf) - len,
				"%2d %6d %4d 0x%04x 0x%04x %3d\n",
				i, vlanmc.vid, vlanmc.priority,
				vlanmc.member, vlanmc.untag, vlanmc.fid);
	}

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

#define RTL8366_VLAN4K_PAGE_SIZE	64
#define RTL8366_VLAN4K_NUM_PAGES	(4096 / RTL8366_VLAN4K_PAGE_SIZE)

static ssize_t rtl8366_read_debugfs_vlan_4k(struct file *file,
					    char __user *user_buf,
					    size_t count, loff_t *ppos)
{
	struct rtl8366_smi *smi = (struct rtl8366_smi *)file->private_data;
	int i, len = 0;
	int offset;
	char *buf = smi->buf;

	if (smi->dbg_vlan_4k_page >= RTL8366_VLAN4K_NUM_PAGES) {
		len += snprintf(buf + len, sizeof(smi->buf) - len,
				"invalid page: %u\n", smi->dbg_vlan_4k_page);
		return simple_read_from_buffer(user_buf, count, ppos, buf, len);
	}

	len += snprintf(buf + len, sizeof(smi->buf) - len,
			"%4s %6s %6s %3s\n",
			"vid", "member", "untag", "fid");

	offset = RTL8366_VLAN4K_PAGE_SIZE * smi->dbg_vlan_4k_page;
	for (i = 0; i < RTL8366_VLAN4K_PAGE_SIZE; i++) {
		struct rtl8366_vlan_4k vlan4k;

		smi->ops->get_vlan_4k(smi, offset + i, &vlan4k);

		len += snprintf(buf + len, sizeof(smi->buf) - len,
				"%4d 0x%04x 0x%04x %3d\n",
				vlan4k.vid, vlan4k.member,
				vlan4k.untag, vlan4k.fid);
	}

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t rtl8366_read_debugfs_pvid(struct file *file,
					 char __user *user_buf,
					 size_t count, loff_t *ppos)
{
	struct rtl8366_smi *smi = (struct rtl8366_smi *)file->private_data;
	char *buf = smi->buf;
	int len = 0;
	int i;

	len += snprintf(buf + len, sizeof(smi->buf) - len, "%4s %4s\n",
			"port", "pvid");

	for (i = 0; i < smi->num_ports; i++) {
		int pvid;
		int err;

		err = rtl8366_get_pvid(smi, i, &pvid);
		if (err)
			len += snprintf(buf + len, sizeof(smi->buf) - len,
				"%4d error\n", i);
		else
			len += snprintf(buf + len, sizeof(smi->buf) - len,
				"%4d %4d\n", i, pvid);
	}

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t rtl8366_read_debugfs_reg(struct file *file,
					 char __user *user_buf,
					 size_t count, loff_t *ppos)
{
	struct rtl8366_smi *smi = (struct rtl8366_smi *)file->private_data;
	u32 t, reg = smi->dbg_reg;
	int err, len = 0;
	char *buf = smi->buf;

	memset(buf, '\0', sizeof(smi->buf));

	err = rtl8366_smi_read_reg(smi, reg, &t);
	if (err) {
		len += snprintf(buf, sizeof(smi->buf),
				"Read failed (reg: 0x%04x)\n", reg);
		return simple_read_from_buffer(user_buf, count, ppos, buf, len);
	}

	len += snprintf(buf, sizeof(smi->buf), "reg = 0x%04x, val = 0x%04x\n",
			reg, t);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t rtl8366_write_debugfs_reg(struct file *file,
					  const char __user *user_buf,
					  size_t count, loff_t *ppos)
{
	struct rtl8366_smi *smi = (struct rtl8366_smi *)file->private_data;
	unsigned long data;
	u32 reg = smi->dbg_reg;
	int err;
	size_t len;
	char *buf = smi->buf;

	len = min(count, sizeof(smi->buf) - 1);
	if (copy_from_user(buf, user_buf, len)) {
		dev_err(smi->parent, "copy from user failed\n");
		return -EFAULT;
	}

	buf[len] = '\0';
	if (len > 0 && buf[len - 1] == '\n')
		buf[len - 1] = '\0';


	if (strict_strtoul(buf, 16, &data)) {
		dev_err(smi->parent, "Invalid reg value %s\n", buf);
	} else {
		err = rtl8366_smi_write_reg(smi, reg, data);
		if (err) {
			dev_err(smi->parent,
				"writing reg 0x%04x val 0x%04lx failed\n",
				reg, data);
		}
	}

	return count;
}

static ssize_t rtl8366_read_debugfs_mibs(struct file *file,
					 char __user *user_buf,
					 size_t count, loff_t *ppos)
{
	struct rtl8366_smi *smi = file->private_data;
	int i, j, len = 0;
	char *buf = smi->buf;

	len += snprintf(buf + len, sizeof(smi->buf) - len, "%-36s",
			"Counter");

	for (i = 0; i < smi->num_ports; i++) {
		char port_buf[10];

		snprintf(port_buf, sizeof(port_buf), "Port %d", i);
		len += snprintf(buf + len, sizeof(smi->buf) - len, " %12s",
				port_buf);
	}
	len += snprintf(buf + len, sizeof(smi->buf) - len, "\n");

	for (i = 0; i < smi->num_mib_counters; i++) {
		len += snprintf(buf + len, sizeof(smi->buf) - len, "%-36s ",
				smi->mib_counters[i].name);
		for (j = 0; j < smi->num_ports; j++) {
			unsigned long long counter = 0;

			if (!smi->ops->get_mib_counter(smi, i, j, &counter))
				len += snprintf(buf + len,
						sizeof(smi->buf) - len,
						"%12llu ", counter);
			else
				len += snprintf(buf + len,
						sizeof(smi->buf) - len,
						"%12s ", "error");
		}
		len += snprintf(buf + len, sizeof(smi->buf) - len, "\n");
	}

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations fops_rtl8366_regs = {
	.read	= rtl8366_read_debugfs_reg,
	.write	= rtl8366_write_debugfs_reg,
	.open	= rtl8366_debugfs_open,
	.owner	= THIS_MODULE
};

static const struct file_operations fops_rtl8366_vlan_mc = {
	.read	= rtl8366_read_debugfs_vlan_mc,
	.open	= rtl8366_debugfs_open,
	.owner	= THIS_MODULE
};

static const struct file_operations fops_rtl8366_vlan_4k = {
	.read	= rtl8366_read_debugfs_vlan_4k,
	.open	= rtl8366_debugfs_open,
	.owner	= THIS_MODULE
};

static const struct file_operations fops_rtl8366_pvid = {
	.read	= rtl8366_read_debugfs_pvid,
	.open	= rtl8366_debugfs_open,
	.owner	= THIS_MODULE
};

static const struct file_operations fops_rtl8366_mibs = {
	.read = rtl8366_read_debugfs_mibs,
	.open = rtl8366_debugfs_open,
	.owner = THIS_MODULE
};

static void rtl8366_debugfs_init(struct rtl8366_smi *smi)
{
	struct dentry *node;
	struct dentry *root;

	if (!smi->debugfs_root)
		smi->debugfs_root = debugfs_create_dir(dev_name(smi->parent),
						       NULL);

	if (!smi->debugfs_root) {
		dev_err(smi->parent, "Unable to create debugfs dir\n");
		return;
	}
	root = smi->debugfs_root;

	node = debugfs_create_x16("reg", S_IRUGO | S_IWUSR, root,
				  &smi->dbg_reg);
	if (!node) {
		dev_err(smi->parent, "Creating debugfs file '%s' failed\n",
			"reg");
		return;
	}

	node = debugfs_create_file("val", S_IRUGO | S_IWUSR, root, smi,
				   &fops_rtl8366_regs);
	if (!node) {
		dev_err(smi->parent, "Creating debugfs file '%s' failed\n",
			"val");
		return;
	}

	node = debugfs_create_file("vlan_mc", S_IRUSR, root, smi,
				   &fops_rtl8366_vlan_mc);
	if (!node) {
		dev_err(smi->parent, "Creating debugfs file '%s' failed\n",
			"vlan_mc");
		return;
	}

	node = debugfs_create_u8("vlan_4k_page", S_IRUGO | S_IWUSR, root,
				  &smi->dbg_vlan_4k_page);
	if (!node) {
		dev_err(smi->parent, "Creating debugfs file '%s' failed\n",
			"vlan_4k_page");
		return;
	}

	node = debugfs_create_file("vlan_4k", S_IRUSR, root, smi,
				   &fops_rtl8366_vlan_4k);
	if (!node) {
		dev_err(smi->parent, "Creating debugfs file '%s' failed\n",
			"vlan_4k");
		return;
	}

	node = debugfs_create_file("pvid", S_IRUSR, root, smi,
				   &fops_rtl8366_pvid);
	if (!node) {
		dev_err(smi->parent, "Creating debugfs file '%s' failed\n",
			"pvid");
		return;
	}

	node = debugfs_create_file("mibs", S_IRUSR, smi->debugfs_root, smi,
				   &fops_rtl8366_mibs);
	if (!node)
		dev_err(smi->parent, "Creating debugfs file '%s' failed\n",
			"mibs");
}

static void rtl8366_debugfs_remove(struct rtl8366_smi *smi)
{
	if (smi->debugfs_root) {
		debugfs_remove_recursive(smi->debugfs_root);
		smi->debugfs_root = NULL;
	}
}
#else
static inline void rtl8366_debugfs_init(struct rtl8366_smi *smi) {}
static inline void rtl8366_debugfs_remove(struct rtl8366_smi *smi) {}
#endif /* CONFIG_RTL8366S_PHY_DEBUG_FS */

static int rtl8366_smi_mii_init(struct rtl8366_smi *smi)
{
	int ret;
	int i;

	smi->mii_bus = mdiobus_alloc();
	if (smi->mii_bus == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	smi->mii_bus->priv = (void *) smi;
	smi->mii_bus->name = dev_name(smi->parent);
	smi->mii_bus->read = smi->ops->mii_read;
	smi->mii_bus->write = smi->ops->mii_write;
	snprintf(smi->mii_bus->id, MII_BUS_ID_SIZE, "%s",
		 dev_name(smi->parent));
	smi->mii_bus->parent = smi->parent;
	smi->mii_bus->phy_mask = ~(0x1f);
	smi->mii_bus->irq = smi->mii_irq;
	for (i = 0; i < PHY_MAX_ADDR; i++)
		smi->mii_irq[i] = PHY_POLL;

	ret = mdiobus_register(smi->mii_bus);
	if (ret)
		goto err_free;

	return 0;

 err_free:
	mdiobus_free(smi->mii_bus);
 err:
	return ret;
}

static void rtl8366_smi_mii_cleanup(struct rtl8366_smi *smi)
{
	mdiobus_unregister(smi->mii_bus);
	mdiobus_free(smi->mii_bus);
}

int rtl8366_sw_get_port_pvid(struct switch_dev *dev, int port, int *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	return rtl8366_get_pvid(smi, port, val);
}
EXPORT_SYMBOL_GPL(rtl8366_sw_get_port_pvid);

int rtl8366_sw_set_port_pvid(struct switch_dev *dev, int port, int val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	return rtl8366_set_pvid(smi, port, val);
}
EXPORT_SYMBOL_GPL(rtl8366_sw_set_port_pvid);

int rtl8366_sw_get_port_mib(struct switch_dev *dev,
			    const struct switch_attr *attr,
			    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	int i, len = 0;
	unsigned long long counter = 0;
	char *buf = smi->buf;

	if (val->port_vlan >= smi->num_ports)
		return -EINVAL;

	len += snprintf(buf + len, sizeof(smi->buf) - len,
			"Port %d MIB counters\n",
			val->port_vlan);

	for (i = 0; i < smi->num_mib_counters; ++i) {
		len += snprintf(buf + len, sizeof(smi->buf) - len,
				"%-36s: ", smi->mib_counters[i].name);
		if (!smi->ops->get_mib_counter(smi, i, val->port_vlan,
					       &counter))
			len += snprintf(buf + len, sizeof(smi->buf) - len,
					"%llu\n", counter);
		else
			len += snprintf(buf + len, sizeof(smi->buf) - len,
					"%s\n", "error");
	}

	val->value.s = buf;
	val->len = len;
	return 0;
}
EXPORT_SYMBOL_GPL(rtl8366_sw_get_port_mib);

int rtl8366_sw_get_vlan_info(struct switch_dev *dev,
			     const struct switch_attr *attr,
			     struct switch_val *val)
{
	int i;
	u32 len = 0;
	struct rtl8366_vlan_4k vlan4k;
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	char *buf = smi->buf;
	int err;

	if (!smi->ops->is_vlan_valid(smi, val->port_vlan))
		return -EINVAL;

	memset(buf, '\0', sizeof(smi->buf));

	err = smi->ops->get_vlan_4k(smi, val->port_vlan, &vlan4k);
	if (err)
		return err;

	len += snprintf(buf + len, sizeof(smi->buf) - len,
			"VLAN %d: Ports: '", vlan4k.vid);

	for (i = 0; i < smi->num_ports; i++) {
		if (!(vlan4k.member & (1 << i)))
			continue;

		len += snprintf(buf + len, sizeof(smi->buf) - len, "%d%s", i,
				(vlan4k.untag & (1 << i)) ? "" : "t");
	}

	len += snprintf(buf + len, sizeof(smi->buf) - len,
			"', members=%04x, untag=%04x, fid=%u",
			vlan4k.member, vlan4k.untag, vlan4k.fid);

	val->value.s = buf;
	val->len = len;

	return 0;
}
EXPORT_SYMBOL_GPL(rtl8366_sw_get_vlan_info);

int rtl8366_sw_get_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	struct switch_port *port;
	struct rtl8366_vlan_4k vlan4k;
	int i;

	if (!smi->ops->is_vlan_valid(smi, val->port_vlan))
		return -EINVAL;

	smi->ops->get_vlan_4k(smi, val->port_vlan, &vlan4k);

	port = &val->value.ports[0];
	val->len = 0;
	for (i = 0; i < smi->num_ports; i++) {
		if (!(vlan4k.member & BIT(i)))
			continue;

		port->id = i;
		port->flags = (vlan4k.untag & BIT(i)) ?
					0 : BIT(SWITCH_PORT_FLAG_TAGGED);
		val->len++;
		port++;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(rtl8366_sw_get_vlan_ports);

int rtl8366_sw_set_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	struct switch_port *port;
	u32 member = 0;
	u32 untag = 0;
	int err;
	int i;

	if (!smi->ops->is_vlan_valid(smi, val->port_vlan))
		return -EINVAL;

	port = &val->value.ports[0];
	for (i = 0; i < val->len; i++, port++) {
		member |= BIT(port->id);

		if (!(port->flags & BIT(SWITCH_PORT_FLAG_TAGGED)))
			untag |= BIT(port->id);

		/*
		 * To ensure that we have a valid MC entry for this VLAN,
		 * initialize the port VLAN ID here.
		 */
		err = rtl8366_set_pvid(smi, port->id, val->port_vlan);
		if (err < 0)
			return err;
	}

	return rtl8366_set_vlan(smi, val->port_vlan, member, untag, 0);
}
EXPORT_SYMBOL_GPL(rtl8366_sw_set_vlan_ports);

int rtl8366_sw_get_vlan_fid(struct switch_dev *dev,
			    const struct switch_attr *attr,
			    struct switch_val *val)
{
	struct rtl8366_vlan_4k vlan4k;
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	int err;

	if (!smi->ops->is_vlan_valid(smi, val->port_vlan))
		return -EINVAL;

	err = smi->ops->get_vlan_4k(smi, val->port_vlan, &vlan4k);
	if (err)
		return err;

	val->value.i = vlan4k.fid;

	return 0;
}
EXPORT_SYMBOL_GPL(rtl8366_sw_get_vlan_fid);

int rtl8366_sw_set_vlan_fid(struct switch_dev *dev,
			    const struct switch_attr *attr,
			    struct switch_val *val)
{
	struct rtl8366_vlan_4k vlan4k;
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	int err;

	if (!smi->ops->is_vlan_valid(smi, val->port_vlan))
		return -EINVAL;

	if (val->value.i < 0 || val->value.i > attr->max)
		return -EINVAL;

	err = smi->ops->get_vlan_4k(smi, val->port_vlan, &vlan4k);
	if (err)
		return err;

	return rtl8366_set_vlan(smi, val->port_vlan,
				vlan4k.member,
				vlan4k.untag,
				val->value.i);
}
EXPORT_SYMBOL_GPL(rtl8366_sw_set_vlan_fid);

int rtl8366_sw_get_vlan_enable(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);

	if (attr->ofs > 2)
		return -EINVAL;

	if (attr->ofs == 1)
		val->value.i = smi->vlan_enabled;
	else
		val->value.i = smi->vlan4k_enabled;

	return 0;
}
EXPORT_SYMBOL_GPL(rtl8366_sw_get_vlan_enable);

int rtl8366_sw_set_vlan_enable(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	int err;

	if (attr->ofs > 2)
		return -EINVAL;

	if (attr->ofs == 1)
		err = rtl8366_enable_vlan(smi, val->value.i);
	else
		err = rtl8366_enable_vlan4k(smi, val->value.i);

	return err;
}
EXPORT_SYMBOL_GPL(rtl8366_sw_set_vlan_enable);

struct rtl8366_smi *rtl8366_smi_alloc(struct device *parent)
{
	struct rtl8366_smi *smi;

	BUG_ON(!parent);

	smi = kzalloc(sizeof(*smi), GFP_KERNEL);
	if (!smi) {
		dev_err(parent, "no memory for private data\n");
		return NULL;
	}

	smi->parent = parent;
	return smi;
}
EXPORT_SYMBOL_GPL(rtl8366_smi_alloc);

int rtl8366_smi_init(struct rtl8366_smi *smi)
{
	int err;

	if (!smi->ops)
		return -EINVAL;

	err = gpio_request(smi->gpio_sda, dev_name(smi->parent));
	if (err) {
		dev_err(smi->parent, "gpio_request failed for %u, err=%d\n",
			smi->gpio_sda, err);
		goto err_out;
	}

	err = gpio_request(smi->gpio_sck, dev_name(smi->parent));
	if (err) {
		dev_err(smi->parent, "gpio_request failed for %u, err=%d\n",
			smi->gpio_sck, err);
		goto err_free_sda;
	}

	spin_lock_init(&smi->lock);

	dev_info(smi->parent, "using GPIO pins %u (SDA) and %u (SCK)\n",
		 smi->gpio_sda, smi->gpio_sck);

	err = smi->ops->detect(smi);
	if (err) {
		dev_err(smi->parent, "chip detection failed, err=%d\n", err);
		goto err_free_sck;
	}

	err = smi->ops->setup(smi);
	if (err) {
		dev_err(smi->parent, "chip setup failed, err=%d\n", err);
		goto err_free_sck;
	}

	err = rtl8366_init_vlan(smi);
	if (err) {
		dev_err(smi->parent, "VLAN initialization failed, err=%d\n",
			err);
		goto err_free_sck;
	}

	err = rtl8366_enable_all_ports(smi, 1);
	if (err)
		goto err_free_sck;

	err = rtl8366_smi_mii_init(smi);
	if (err)
		goto err_free_sck;

	rtl8366_debugfs_init(smi);

	return 0;

 err_free_sck:
	gpio_free(smi->gpio_sck);
 err_free_sda:
	gpio_free(smi->gpio_sda);
 err_out:
	return err;
}
EXPORT_SYMBOL_GPL(rtl8366_smi_init);

void rtl8366_smi_cleanup(struct rtl8366_smi *smi)
{
	rtl8366_debugfs_remove(smi);
	rtl8366_smi_mii_cleanup(smi);
	gpio_free(smi->gpio_sck);
	gpio_free(smi->gpio_sda);
}
EXPORT_SYMBOL_GPL(rtl8366_smi_cleanup);

MODULE_DESCRIPTION("Realtek RTL8366 SMI interface driver");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL v2");
