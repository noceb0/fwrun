// get vscode to shut the fuck up
#define NDS

#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <fat.h>

#include "fwunpack.h"
#include "fwpatch.h"
#include "loader_arm9.h"
#include "prefcompat.h"

// embed /arm9/data/firmware.bin
#ifdef EMBEDDED_FIRMWARE
#include "firmware_bin.h"
#endif

static fwunpackParams params;

void hang() {
	while(1) swiWaitForVBlank();
}

int fwRead() {
#ifndef EMBEDDED_FIRMWARE
	FILE* fw_bin = fopen("firmware.bin", "rb");
	if (!fw_bin) return 1;
	fseek(fw_bin, 0, SEEK_END);
	size_t fw_size = ftell(fw_bin);
	fseek(fw_bin, 0, SEEK_SET);
	params.fwdata = (u8*)malloc(fw_size);
	fread(params.fwdata, 1, fw_size, fw_bin);
	fclose(fw_bin);
	printf("0x%06X\n", fw_size);
#else
	params.fwdata = firmware_bin;
	printf("embedded\n");
#endif
	return 0;
}

int main(void) {

	consoleDemoInit();

	params.isDsi = isDSiMode();

	printf("fwrun\n\n");
	memset(&params, sizeof params, 1);

#ifndef EMBEDDED_FIRMWARE
	if (!fatInitDefault()) {
		printf("fat init failure!\n");
		hang();
		return 1;
	}
#endif

	printf("reading firmware.bin...");
	if (fwRead()) {
		printf("error!\n");
		hang();
		return 1;
	}

	printf("1) begin unpacking\n");
	fwunpack_stage1(&params);

	printf("2) patch bootcode\n");
	if (fwpatch(&params)) {
		printf("patching failed!\n");
		hang();
		return 1;
	}

	printf("3) finish unpacking\n");
	fwunpack_stage3(&params);

	printf("4) patch preferences\n");
	patch_preferences(&params);

	printf("5) pass control to firmware\n");
	loaderRun(&params);

	return 0;
}
