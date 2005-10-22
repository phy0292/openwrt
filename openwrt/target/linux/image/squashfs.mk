squashfs-prepare:
	$(MAKE) -C squashfs prepare $(MAKE_TRACE)

squashfs-compile: prepare-targets
	$(MAKE) -C squashfs compile $(MAKE_TRACE)
	
squashfs-clean:
	$(MAKE) -C squashfs clean $(MAKE_TRACE)
	rm -f $(KDIR)/root.squashfs

$(KDIR)/root.squashfs: install-prepare
	@mkdir -p $(KDIR)/root/jffs
	$(STAGING_DIR)/bin/mksquashfs-lzma $(KDIR)/root $@ -nopad -noappend -root-owned -le $(MAKE_TRACE)

squashfs-install: install-targets $(KDIR)/root.squashfs $(BOARD)-compile
	$(TRACE) target/linux/image/$(BOARD)/install
	$(MAKE) -C $(BOARD) install KERNEL="$(KERNEL)" FS="squashfs"

squashfs-install-ib:
	mkdir -p $(IB_DIR)/staging_dir_$(ARCH)/bin
	cp $(STAGING_DIR)/bin/mksquashfs-lzma $(IB_DIR)/staging_dir_$(ARCH)/bin
	
prepare-targets: squashfs-prepare
compile-targets: squashfs-compile
install-targets: squashfs-install
install-ib: squashfs-install-ib
clean: squashfs-clean
