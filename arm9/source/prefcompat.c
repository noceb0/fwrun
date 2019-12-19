#include <stdio.h>
#include <nds.h>
#include <nds/system.h>

#include "prefcompat.h"
#include "fwunpack.h"

// until i can get my extended settings patching working
//#define ONLY_USE_NTRLDR_METHOD

// used to inject known good settings into nvram
#ifdef PREFSTUB_IQUE
#define PREFSTUB prefstub_ique_bin
#include "prefstub_ique_bin.h"
#endif
#ifdef PREFSTUB_KOREAN
#define PREFSTUB prefstub_korean_bin
#include "prefstub_korean_bin.h"
#endif

static fwPreferences prefSlots[2];

// TODO: THIS WILL PROBABLY BRICK DEVICES WITH 512K FLASH
//       we should be getting the preference base dynamically with the flash header
//		 unfortunately, i wasnt smart enough to figure out how to do that right yet
void patch_preferences(fwunpackParams* params) {
#ifdef PREFSTUB
	printf("dumping prefstub...");
	writeFirmware(0x3FE00, &PREFSTUB, 0x200);
#else
	if (isDSiMode() || params->type != FW_NORMAL) {
		printf("reading preferences...\n");
		readFirmware(0x3FE00, &prefSlots, sizeof(prefSlots));
		for (u32 i = 0; i < 2; i++) {
			printf("fix slot %X flags...\n", i);
			prefSlots[i].valid = MAGIC_VALID; // 3ds sets this value to something dumb and causes firmware to bootloop
			prefSlots[i].crc = swiCRC16(0xFFFF, &prefSlots[i], 0x70);
#ifndef ONLY_USE_NTRLDR_METHOD
			switch (params->type) {
				case FW_IQUE:
				case FW_KOREAN:
					printf("fix extended settings...\n");
					prefSlots[i].extended.present = 1;
					prefSlots[i].extended.supported_languages = (params->type == FW_IQUE) ? LANG_SUPPORT_IQUE : LANG_SUPPORT_KOREAN;
					prefSlots[i].extended.language = (params->type == FW_IQUE) ? LANG_CHINESE : LANG_KOREAN;
					prefSlots[i].extended.crc = swiCRC16(0xffff, &prefSlots[i].extended, 0x8A);
					break;
				default:
					break;
			}
#endif
		}
		writeFirmware(0x3FE00, &prefSlots, sizeof(prefSlots));
	}
	else printf("dont need to patch nvram...\n");
#if 0
	if (isDSiMode() || params->type != FW_NORMAL) {
			printf("reading preferences...\n");
			readFirmware(0x3FE00, &prefSlots, 0x200);
			FILE* f = fopen("/dbgold.bin", "wb");
			fwrite(&prefSlots, 2, sizeof(fwPreferences), f);

			u32 currentSlot = prefSlots[0].update_counter > prefSlots[1].update_counter ? 0 : 1;
			fwPreferences* current = &prefSlots[currentSlot];
			printf("active prefslot %X (%X)\n", currentSlot, current->update_counter);

			if (isDSiMode() && current->valid != 0xFC) {
				printf("normalizing flags...\n");
				current->valid = 0xFC;
				current->crc = swiCRC16(0xffff, &current, 0x70);
			}

			switch (params->type) {
				case FW_IQUE:
				case FW_KOREAN:
					printf("target is ique or korean\n");
					printf("fix extended settings...\n");
					current->extended.present = 1;
					current->extended.supported_languages = (params->type == FW_IQUE) ? LANG_SUPPORT_IQUE : LANG_SUPPORT_KOREAN;
					current->extended.language = (params->type == FW_IQUE) ? LANG_CHINESE : LANG_KOREAN;
					current->extended.crc = swiCRC16(0xffff, &current->extended, 0x8A);
					break;
				default:
					break;
			}

			printf("writing back prefs...\n");
			current->update_counter++;
			fwrite(&prefSlots, 2, sizeof(fwPreferences), f);
			fclose(f);
			writeFirmware(currentSlot ? 0x3FE00 : 0x3FF00, &current, 0x100);
	}
	else printf("dont need to patch nvram...\n");
#endif
#endif
}