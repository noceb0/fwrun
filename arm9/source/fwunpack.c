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

#include "encryption.h"
#include "get_data.h"
#include "get_encrypted_data.h"
#include "get_normal_data.h"
#include "lz77.h"
#include "part345_comp.h"

#define FW_HEADER_SIZE 0x200

#define COMPRESSION_TYPE_LZ77 1

int decompressAll(GET_DATA get_data, u8* src, u8** dest, bool alloc) {
	get_data.set_address(src);

	int compression_type = (get_data.get_u8() & 0xF0) >> 4;
	int decompressed_size = get_data.get_u8();
	decompressed_size |= get_data.get_u8() << 8;
	decompressed_size |= get_data.get_u8() << 16;

	if (alloc) *dest = (u8*) malloc (decompressed_size);
	//printf("decrypt_decompress(): malloc returned 0x%08X", *dest);

	switch (compression_type) {
		case COMPRESSION_TYPE_LZ77:
			// TODO: nds has bios functions for lz77, we should use that
			Decompress_LZ77(get_data, *dest, decompressed_size);
			break;
		default:
			printf("CANNOT DECOMPRESS TYPE %d\n", compression_type);
			decompressed_size = 0;
			break;
	}

	return decompressed_size;
}

#define decompress(src, dest, alloc) decompressAll(get_normal_data, src, dest, alloc)
#define decrypt_decompress(src, dest, alloc) decompressAll(get_encrypted_data, src, dest, alloc)

int fwunpack_stage1(fwunpackParams* params) {

	fwHeader* fw_header = (fwHeader*)params->fwdata;

	params->boot9.romaddr = fw_header->part1_romaddr * (4 << ((fw_header->shift_amounts>>0) & 7));
	params->boot9.ramaddr = (u8*)(0x02800000 - fw_header->part1_ramaddr * (4 << ((fw_header->shift_amounts>>3) & 7)));

	params->boot7.romaddr = fw_header->part2_romaddr * (4 << ((fw_header->shift_amounts>>6) & 7));
	params->boot7.ramaddr = (u8*)((fw_header->shift_amounts & 0x1000 ? 0x02800000 : 0x03810000) - fw_header->part2_ramaddr * (4 << ((fw_header->shift_amounts>>9) & 7)));

	params->gui9.romaddr = fw_header->part3_romaddr * 8;
	params->gui7.romaddr = fw_header->part4_romaddr * 8;

	params->guidata.romaddr = fw_header->part5_romaddr * 8;
	
	printf ("boot9 at 0x%08X\n", params->boot9.romaddr);
	printf ("boot7 at 0x%08X\n", params->boot7.romaddr);
	
	printf ("guidata at 0x%08X\n", params->guidata.romaddr);
	printf ("gui9 at 0x%08X\n", params->gui9.romaddr);
	printf ("gui7 at 0x%08X\n", params->gui7.romaddr);

	// Start unpacking
	init_keycode ( ((u32*)params->fwdata)[2] , 2, 0x0C); // idcode (usually "MACP"), level 2

	// detect flashme firmware
	if (params->fwdata[0x17C] != 0xFF) {
		params->type = FW_FLASHME;

		printf ("flashme, use part2\n");
		fwHeader* fmheader = (fwHeader*)(params->fwdata + 0x3F680);
		params->boot9.romaddr = fmheader->part1_romaddr * (4 << ((fmheader->shift_amounts>>0) & 7));
		params->boot9.ramaddr = (u8*)(0x02800000 - fmheader->part1_ramaddr * (4 << ((fmheader->shift_amounts>>3) & 7)));

		params->boot7.romaddr = fmheader->part2_romaddr * (4 << ((fmheader->shift_amounts>>6) & 7));
		params->boot7.ramaddr = (u8*)((fmheader->shift_amounts & 0x1000 ? 0x02800000 : 0x03810000) - fmheader->part2_ramaddr * (4 << ((fmheader->shift_amounts>>9) & 7)));

		printf ("boot9p2 at 0x%08X\n", params->boot9.romaddr);
		printf ("boot7p2 at 0x%08X\n", params->boot7.romaddr);
	}
	else switch (fw_header->console_type) {
		case CONSOLE_TYPE_KOREAN_LITE:
			params->type = FW_KOREAN;
			break;
		case CONSOLE_TYPE_IQUE:
		case CONSOLE_TYPE_IQUE_LITE:
			params->type = FW_IQUE;
			break;
		default:
			params->type = FW_NORMAL;
			break;
	}

	// boot7 cant be unpacked in place before the mpu is reset
	printf("boot7 -> 0x%08X\n", params->boot7.ramaddr);
	printf("unpacking boot7...");
	if (params->type == FW_FLASHME) params->boot7.size = decompress(params->fwdata + params->boot7.romaddr, LOADADDR_BOOT7(&params), RELOC_BOOT7);
	else params->boot7.size = decrypt_decompress(params->fwdata + params->boot7.romaddr,  LOADADDR_BOOT7(&params), RELOC_BOOT7);
	printf("0x%04X\n", params->boot7.size);
	printf("tmp 0x%08X\n", params->boot7.tmpaddr);

	printf("boot9 -> 0x%08X\n", params->boot9.ramaddr);
	printf("unpacking boot9...");
	if (params->type == FW_FLASHME) params->boot9.size = decompress(params->fwdata + params->boot9.romaddr, LOADADDR_BOOT9(&params), RELOC_BOOT9);
	else params->boot9.size = decrypt_decompress(params->fwdata + params->boot9.romaddr, LOADADDR_BOOT9(&params), RELOC_BOOT9);
	printf("0x%04X\n", params->boot9.size);
	printf("tmp 0x%08X\n", params->boot9.tmpaddr);

	return 0;

}

void decompress345(fwunpackParams* params, fwAddrs* addrs, bool alloc) {
	if (alloc) {
		addrs->size = part345_decompress(NULL, params->fwdata + addrs->romaddr);
		printf("0x%04X\n", addrs->size);
		addrs->tmpaddr = (u8*)malloc(addrs->size);
		printf("tmp 0x%08X\n", addrs->tmpaddr);
		part345_decompress(addrs->tmpaddr, params->fwdata + addrs->romaddr);
	}
	else {
		addrs->size = part345_decompress(addrs->ramaddr, params->fwdata + addrs->romaddr);
		printf("0x%04X\n", addrs->size);
	}
}

int fwunpack_stage3(fwunpackParams* params) {

	printf("unpacking gui7...");
	decompress345(params, &params->gui7, RELOC_GUI7);

	printf("unpacking gui9...");
	decompress345(params, &params->gui9, RELOC_GUI9);

	printf("unpacking guidata...");
	decompress345(params, &params->guidata, RELOC_GUIDATA);

}