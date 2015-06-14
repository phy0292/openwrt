/*
 *
 *  Copyright (C) 2012 OpenWrt.org
 *  Copyright (C) 2012 Mikko Hissa <mikko.hissa@uta.fi>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "md5.h"

#define HDR_LEN                 0x60
#define BUF_SIZE                0x200
#define VERSION_SIZE            0x10
#define MD5_SIZE                0x10
#define PAD_SIZE                0x20

#define DEFAULT_BLOCK_SIZE      65535

#define DEFAULT_HEAD_VALUE      0x0
#define DEFAULT_VERSION         "123"
#define DEFAULT_MAGIC           0x12345678

typedef struct {
	uint32_t head;
	uint32_t vendor_id;
	uint32_t product_id;
	uint8_t  version[VERSION_SIZE];
	uint32_t firmware_type;
	uint32_t filesize;
	uint32_t zero;
	uint8_t  md5sum[MD5_SIZE];
	uint8_t  pad[PAD_SIZE];
	uint32_t chksum;
	uint32_t magic;
} img_header;

typedef struct {
	uint8_t id;
	char * name;
} firmware_type;

typedef enum {
	NONE, ENCODE, DECODE
} op_mode;

static firmware_type FIRMWARE_TYPES[] = {
	{ 0x01, "bootloader" },
	{ 0x02, "kernel" },
	{ 0x03, "kernelapp" },
	{ 0x04, "apps" },
	/* The types below this line vary by manufacturer */
	{ 0x05, "littleapps (D-Link)/factoryapps (EnGenius)" },
	{ 0x06, "sounds (D-Link)/littleapps (EnGenius)" },
	{ 0x07, "userconfig (D-Link)/appdata (EnGenius)" },
	{ 0x08, "userconfig (EnGenius)"},
	{ 0x09, "odmapps (EnGenius)"},
	{ 0x0a, "factoryapps (D-Link)" },
	{ 0x0b, "odmapps (D-Link)" },
	{ 0x0c, "langpack (D-Link)" }
};

static long get_file_size(const char *filename)
{
	FILE *fp_file;
	long result;

	fp_file = fopen(filename, "r");
	if (!fp_file)
		return -1;
	fseek(fp_file, 0, SEEK_END);
	result = ftell(fp_file);
	fclose(fp_file);
	return result;
}

static int header_checksum(void *data, int len)
{
	int i;
	int sum;

	sum = 0;
	if (data != NULL && len >= 0) {
		for (i = 0; i < len; ++i)
			sum += *(unsigned char *) (data + i);
		return sum;
	}

	return -1;
}

static int md5_file(const char *filename, uint8_t *dst)
{
	FILE *fp_src;
	MD5_CTX ctx;
	char buf[BUF_SIZE];
	size_t bytes_read;

	MD5_Init(&ctx);

	fp_src = fopen(filename, "r+b");
	if (!fp_src) {
		return -1;
	}
	while (!feof(fp_src)) {
		bytes_read = fread(&buf, 1, BUF_SIZE, fp_src);
		MD5_Update(&ctx, &buf, bytes_read);
	}
	fclose(fp_src);

	MD5_Final(dst, &ctx);

	return 0;
}

static int encode_image(const char *input_file_name,
		const char *output_file_name, img_header *header, int block_size)
{
	char buf[BUF_SIZE];
	size_t bytes_read;
	size_t pad_len = 0;
	size_t bytes_avail;

	FILE *fp_input;
	FILE *fp_output;

	int i;
	long magic;

	fp_input = fopen(input_file_name, "r+b");
	if (!fp_input) {
		fprintf(stderr, "Cannot open %s !!\n", input_file_name);
		return -1;
	}

	fp_output = fopen(output_file_name, "w+b");
	if (!fp_output) {
		fprintf(stderr, "Cannot open %s !!\n", output_file_name);
		fclose(fp_input);
		return -1;
	}

	header->filesize = get_file_size(input_file_name);
	if (!header->filesize) {
		fprintf(stderr, "File %s open/size error!\n", input_file_name);
		fclose(fp_input);
		fclose(fp_output);
		return -1;
	}
	/*
	 * Zero padding
	 */
	if (block_size > 0) {
		pad_len = block_size - (header->filesize % block_size);
	}

	if (md5_file(input_file_name, (uint8_t *) &header->md5sum) < 0) {
		fprintf(stderr, "MD5 failed on file %s\n", input_file_name);
		fclose(fp_input);
		fclose(fp_output);
		return -1;
	}
	header->zero = 0;
	header->chksum = header_checksum(header, HDR_LEN);
	header->head = htonl(header->head);
	header->vendor_id = htonl(header->vendor_id);
	header->product_id = htonl(header->product_id);
	header->firmware_type = htonl(header->firmware_type);
	header->filesize = htonl(header->filesize);
	header->chksum = htonl(header->chksum);
	magic = header->magic;
	header->magic = htonl(header->magic);

	fwrite(header, HDR_LEN, 1, fp_output);

	while (!feof(fp_input) || pad_len > 0) {

		if (!feof(fp_input))
			bytes_read = fread(&buf, 1, BUF_SIZE, fp_input);
		else
			bytes_read = 0;

		/*
		 * No more bytes read, start padding
		 */
		if (bytes_read < BUF_SIZE && pad_len > 0) {
			bytes_avail = BUF_SIZE - bytes_read;
			memset( &buf[bytes_read], 0, bytes_avail);
			bytes_read += bytes_avail < pad_len ? bytes_avail : pad_len;
			pad_len -= bytes_avail < pad_len ? bytes_avail : pad_len;
		}

		for (i = 0; i < bytes_read; i++)
			buf[i] ^= magic >> (i % 8) & 0xff;
		fwrite(&buf, bytes_read, 1, fp_output);
	}

	fclose(fp_input);
	fclose(fp_output);
	return 1;
}

int decode_image(const char *input_file_name, const char *output_file_name)
{
	img_header header;
	char buf[BUF_SIZE];

	FILE *fp_input;
	FILE *fp_output;
	unsigned int i;

	size_t bytes_read;
	size_t bytes_written;

	fp_input = fopen(input_file_name, "r+b");
	if (!fp_input) {
		fprintf(stderr, "Cannot open %s !!\n", input_file_name);
		fclose(fp_input);
		return -1;
	}

	fp_output = fopen(output_file_name, "w+b");
	if (!fp_output) {
		fprintf(stderr, "Cannot open %s !!\n", output_file_name);
		fclose(fp_output);
		return -1;
	}

	if (fread(&header, 1, HDR_LEN, fp_input) != HDR_LEN) {
		fprintf(stderr, "Incorrect header size!!");
		fclose(fp_input);
		fclose(fp_output);
		return -1;
	}

	header.head = ntohl(header.head);
	header.vendor_id = ntohl(header.vendor_id);
	header.product_id = ntohl(header.product_id);
	header.firmware_type = ntohl(header.firmware_type);
	header.filesize = ntohl(header.filesize);
	header.chksum = ntohl(header.chksum);
	header.magic = ntohl(header.magic);

	bytes_written = 0;
	while (!feof(fp_input)) {

		bytes_read = fread(&buf, 1, BUF_SIZE, fp_input);
		for (i = 0; i < bytes_read; i++)
			buf[i] ^= header.magic >> (i % 8) & 0xff;

		/*
		 * Handle padded source file
		 */
		if (bytes_written + bytes_read > header.filesize) {
			bytes_read = header.filesize - bytes_written;
			if (bytes_read > 0)
				fwrite(&buf, bytes_read, 1, fp_output);
			break;
		}

		fwrite(&buf, bytes_read, 1, fp_output);
		bytes_written += bytes_read;
	}

	fclose(fp_input);
	fclose(fp_output);

	return 1;
}

static void usage(const char *progname, int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;
	int i;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream, "\n"
			"Options:\n"
			"  -e <file>		encode image file <file>\n"
			"  -d <file>		decode image file <file>\n"
			"  -o <file>		write output to the file <file>\n"
			"  -t <type>		set image type to <type>\n"
			"			valid image <type> values:\n");
	for (i = 0; i < sizeof(FIRMWARE_TYPES) / sizeof(firmware_type); i++) {
		fprintf(stream, "			%-5i= %s\n", FIRMWARE_TYPES[i].id,
				FIRMWARE_TYPES[i].name);
	}
	fprintf(stream, "  -v <version>		set image version to <version>\n"
			"  -r <vendor>		set image vendor id to <vendor>\n"
			"  -p <product>		set image product id to <product>\n"
			"  -m <magic>		set encoding magic <magic>\n"
			"  -z			enable image padding to <blocksize>\n"
			"  -b <blocksize>	set image <blocksize>, defaults to %u\n"
			"  -h			show this screen\n", DEFAULT_BLOCK_SIZE);
	exit(status);
}

int main(int argc, char *argv[])
{
	int opt;
	char *input_file, *output_file, *progname = NULL;
	op_mode mode = NONE;
	int tmp, i, pad = 0;
	int block_size;
	img_header header;

	block_size = DEFAULT_BLOCK_SIZE;
	progname = basename(argv[0]);

	memset(&header, 0, sizeof( img_header ));
	header.magic = DEFAULT_MAGIC;
	header.head = DEFAULT_HEAD_VALUE;
	strncpy( (char*)&header.version, DEFAULT_VERSION, VERSION_SIZE - 1);

	while ((opt = getopt(argc, argv, ":o:e:d:t:v:r:p:m:b:h?z")) != -1) {
		switch (opt) {
		case 'e':
			input_file = optarg;
			mode = ENCODE;
			break;
		case 'd':
			input_file = optarg;
			mode = DECODE;
			break;
		case 'o':
			output_file = optarg;
			break;
		case 't':
			tmp = strtol(optarg, 0, 10);
			for (i = 0; i < sizeof(FIRMWARE_TYPES) / sizeof(firmware_type);
					i++) {
				if (FIRMWARE_TYPES[i].id == tmp) {
					header.firmware_type = FIRMWARE_TYPES[i].id;
					break;
				}
			}
			if (header.firmware_type == 0) {
				fprintf(stderr, "Invalid firmware type \"0\"!\n");
				usage(progname, EXIT_FAILURE);
			}
			break;
		case 'v':
			strncpy( (char*)&header.version, optarg,
					VERSION_SIZE - 1);
			break;
		case 'r':
			header.vendor_id = strtol(optarg, 0, 0);
			break;
		case 'p':
			header.product_id = strtol(optarg, 0, 0);
			break;
		case 'm':
			header.magic = strtoul(optarg, 0, 16);
			break;
		case 'z':
			pad = 1;
			break;
		case 'b':
			block_size = strtol(optarg, 0, 10);
			break;
		case 'h':
			usage(progname, EXIT_SUCCESS);
			break;
		case ':':
			fprintf(stderr, "Option -%c requires an operand\n", optopt);
			usage(progname, EXIT_FAILURE);
			break;
		case '?':
			fprintf(stderr, "Unrecognized option: -%c\n", optopt);
			usage(progname, EXIT_FAILURE);
			break;
		default:
			usage(progname, EXIT_FAILURE);
			break;
		}
	}

	/* Check required arguments*/
	if (header.firmware_type == 0) {
		fprintf(stderr, "Firmware type must be defined\n");
		usage(progname, EXIT_FAILURE);
	} else if (input_file == 0 || output_file == 0) {
		fprintf(stderr, "Input and output files must be defined\n");
		usage(progname, EXIT_FAILURE);
	} else if (header.vendor_id == 0 || header.product_id == 0) {
		fprintf(stderr, "Vendor ID and Product ID must be defined and non-zero\n");
		usage(progname, EXIT_FAILURE);
	}

	switch (mode) {
	case NONE:
		fprintf(stderr, "A mode must be defined\n");
		usage(progname, EXIT_FAILURE);
		break;
	case ENCODE:
		if (encode_image(input_file, output_file, &header, pad ? block_size : 0)
				< 0)
			return EXIT_FAILURE;
		break;
	case DECODE:
		if (decode_image(input_file, output_file) < 0)
			return EXIT_FAILURE;
		break;
	}

	return EXIT_SUCCESS;
}
