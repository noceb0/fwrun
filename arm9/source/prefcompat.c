// fwrun
// system preference patcher (for 3ds and ique+korean compatibility) (a la ntrldr)

// ntrldr was one of the first pieces of software i ever wrote in C
// im a lot better with it now, and hopefully this file (which is essentially a
// reimplementation of ntrldr with more features) is a testament to that :P

#include <stdio.h>
#include <nds.h>
#include <nds/system.h>

#include "main.h"
#include "prefcompat.h"
#include "fwunpack.h"

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
//       unfortunately, i couldnt figure out how to do that right yet
void patch_preferences() {
#ifdef PREFSTUB
    printf("dumping prefstub...");
    writeFirmware(0x3FE00, &PREFSTUB, 0x200);
#else
    if (isDSiMode() || params.type != FW_NORMAL) {
        printf("reading preferences...\n");
        readFirmware(0x3FE00, &prefSlots, sizeof(prefSlots));
        for (u32 i = 0; i < 2; i++) {
            printf("fix slot %X flags...\n", i);
            prefSlots[i].valid = MAGIC_VALID; // 3ds sets this value to something dumb and causes firmware to bootloop
            prefSlots[i].crc = swiCRC16(0xFFFF, &prefSlots[i], 0x70);
#ifndef ONLY_USE_NTRLDR_METHOD
            switch (params.type) {
                case FW_IQUE:
                case FW_KOREAN:
                    printf("fix extended settings...\n");
                    prefSlots[i].extended.present = 1;
                    prefSlots[i].extended.supported_languages = (params.type == FW_IQUE) ? LANG_SUPPORT_IQUE : LANG_SUPPORT_KOREAN;
                    prefSlots[i].extended.language = (params.type == FW_IQUE) ? LANG_CHINESE : LANG_KOREAN;
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
#endif
}