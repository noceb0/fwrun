/*
fwunpack

	Nintendo DS Firmware Unpacker
    Copyright (C) 2007  Michael Chisholm (Chishm)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <nds/arm9/decompress.h>

#include "fwunpack.h"
#include "fwreloc.h"
#include "main.h"

#include "encryption.h"
#include "fetch.h"
#include "lz77.h"
#include "part345_comp.h"

#define FW_HEADER_SIZE 0x200

#define COMPRESSION_TYPE_LZ77 1

int decompressAll(u32 offset, u8** dest, bool alloc, bool decrypt) {
	fetchSeek(offset);

	int compression_type = (nextByte(decrypt) & 0xF0) >> 4;
	int decompressed_size = nextByte(decrypt);
	decompressed_size |= nextByte(decrypt) << 8;
	decompressed_size |= nextByte(decrypt) << 16;

	if (alloc) *dest = malloc(decompressed_size);

	switch (compression_type) {
		case COMPRESSION_TYPE_LZ77:
			// TODO: nds has bios functions for lz77, we should use that
			Decompress_LZ77(*dest, decompressed_size, decrypt);
			break;
		default:
			printf("CANNOT DECOMPRESS TYPE %d\n", compression_type);
			decompressed_size = 0;
			break;
	}

	return decompressed_size;
}

#define decompress(src, dest, alloc) decompressAll(src, dest, alloc, false)
#define decrypt_decompress(src, dest, alloc) decompressAll(src, dest, alloc, true)

int fwunpack_stage1() {

	fwHeader* fw_header = params.fwhdr;

	params.boot9.romaddr = fw_header->part1_romaddr * (4 << ((fw_header->shift_amounts>>0) & 7));
	params.boot9.ramaddr = (u8*)(0x02800000 - fw_header->part1_ramaddr * (4 << ((fw_header->shift_amounts>>3) & 7)));

	params.boot7.romaddr = fw_header->part2_romaddr * (4 << ((fw_header->shift_amounts>>6) & 7));
	params.boot7.ramaddr = (u8*)((fw_header->shift_amounts & 0x1000 ? 0x02800000 : 0x03810000) - fw_header->part2_ramaddr * (4 << ((fw_header->shift_amounts>>9) & 7)));

	params.gui9.romaddr = fw_header->part3_romaddr * 8;
	params.gui7.romaddr = fw_header->part4_romaddr * 8;

	params.guidata.romaddr = fw_header->part5_romaddr * 8;
	
	printf ("boot9 at 0x%08X\n", params.boot9.romaddr);
	printf ("boot7 at 0x%08X\n", params.boot7.romaddr);
	
	printf ("guidata at 0x%08X\n", params.guidata.romaddr);
	printf ("gui9 at 0x%08X\n", params.gui9.romaddr);
	printf ("gui7 at 0x%08X\n", params.gui7.romaddr);

	// Start unpacking
	init_keycode(*(u32*)fw_header->fw_identifier, 2, 0x0C); // idcode (usually "MACP"), level 2

	// detect flashme firmware and use boot pt2 instead
	fetchSeek(0x17C);
	if (nextByte(false) != 0xFF) {
		params.type = FW_FLASHME;

		printf ("flashme, use part2\n");
		fwHeader fmheader;
		fetchSeek(0x3F680);
		nextBlock(&fmheader, sizeof(fwHeader));
		params.boot9.romaddr = fmheader.part1_romaddr * (4 << ((fmheader.shift_amounts>>0) & 7));
		params.boot9.ramaddr = (u8*)(0x02800000 - fmheader.part1_ramaddr * (4 << ((fmheader.shift_amounts>>3) & 7)));

		params.boot7.romaddr = fmheader.part2_romaddr * (4 << ((fmheader.shift_amounts>>6) & 7));
		params.boot7.ramaddr = (u8*)((fmheader.shift_amounts & 0x1000 ? 0x02800000 : 0x03810000) - fmheader.part2_ramaddr * (4 << ((fmheader.shift_amounts>>9) & 7)));

		printf ("boot9p2 at 0x%08X\n", params.boot9.romaddr);
		printf ("boot7p2 at 0x%08X\n", params.boot7.romaddr);
	}
	else switch (fw_header->console_type) {
		case CONSOLE_TYPE_KOREAN_LITE:
			params.type = FW_KOREAN;
			break;
		case CONSOLE_TYPE_IQUE:
		case CONSOLE_TYPE_IQUE_LITE:
			params.type = FW_IQUE;
			break;
		default:
			params.type = FW_NORMAL;
			break;
	}

	// boot7 cant be unpacked in place before the mpu is reset
	printf("boot7 -> 0x%08X\n", params.boot7.ramaddr);
	printf("unpacking boot7...");
	if (params.type == FW_FLASHME) params.boot7.size = decompress(params.boot7.romaddr, LOADADDR_BOOT7(params), RELOC_BOOT7);
	else params.boot7.size = decrypt_decompress(params.boot7.romaddr, LOADADDR_BOOT7(params), RELOC_BOOT7);
	printf("0x%04X\n", params.boot7.size);
	printf("tmp 0x%08X\n", params.boot7.tmpaddr);

	printf("boot9 -> 0x%08X\n", params.boot9.ramaddr);
	printf("unpacking boot9...");
	if (params.type == FW_FLASHME) params.boot9.size = decompress(params.boot9.romaddr, LOADADDR_BOOT9(params), RELOC_BOOT9);
	else params.boot9.size = decrypt_decompress(params.boot9.romaddr, LOADADDR_BOOT9(params), RELOC_BOOT9);
	printf("0x%04X\n", params.boot9.size);
	printf("tmp 0x%08X\n", params.boot9.tmpaddr);

	return 0;

}

void decompress345(fwAddrs* addrs, bool alloc) {
	// get the final decompressed size
	u8 smol[32];
	memset(smol, 0, 32);
	fetchSeek(addrs->romaddr);
	nextBlock(smol, 32);
	printf("+4 = %X\n", *((u32*)&smol[4]));
	addrs->size = part345_decompress(NULL, smol);
	printf("0x%04X\n", addrs->size);

	// allocate a source and destination buffer
	printf("allocate buffers...\n");
	if (alloc) addrs->tmpaddr = malloc(addrs->size);
	u8* tmpsrc = malloc(addrs->size);

	// read the compressed data into memory
	printf("read data...\n");
	fetchSeek(addrs->romaddr);
	nextBlock(tmpsrc, addrs->size);
	
	// decompress the data
	printf("decompressing...\n");
	part345_decompress(alloc ? addrs->tmpaddr : addrs->ramaddr, tmpsrc);

	// dont bother freeing the dest buffer because it will be used during the relocation stage
	// (this is very messy but keeps other stuff from messing with our data)
	printf("free src buffer...\n");
	free(tmpsrc);
}

int fwunpack_stage3() {

	printf("unpacking gui7...");
	decompress345(&params.gui7, RELOC_GUI7);

	printf("unpacking gui9...");
	decompress345(&params.gui9, RELOC_GUI9);

	printf("unpacking guidata...");
	decompress345(&params.guidata, RELOC_GUIDATA);

	return 0;

}