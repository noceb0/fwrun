#include <stdio.h>

#include "fwunpack.h"

void* memmem(const void* haystack, size_t haystacklen, const void* needle, size_t needlelen);

// magic values used by fw2nds
u8 boot7n0[4] = { 0x6C, 0xF8, 0x7F, 0x02 }; // gui7 tmpaddr is -0x4 from here
u8 boot9n0[4] = { 0x14, 0x48, 0x15, 0x49 }; // gui9/guidata tmpaddr and flash load disable patches near here

#define OFFSET_GUI7_7N0 (-0x4)
#define OFFSET_GUI9_9N0 0x54
#define OFFSET_GUIDATA_9N0 0x9C

// fw2nds flash load disable patches, originally diffed out of firmware.nds
u32 nop9n0[7] = {
    0x04, // prevents a flash read (gui9?)
    0x10, // prevents a flash read (gui7?)
    0x1C, // prevents a crc check (gui9?)
    0x26, // prevents a crc check (gui7?)
    0x36, // unknown?
    0x72, // prevents an flash read (guidata?)
    0x7C  // prevents a crc check (guidata?)
};

int fwpatch(fwunpackParams* params) {

    // special region 0 offset in boot7
    u8* search7 = params->boot7.tmpaddr ? params->boot7.tmpaddr : params->boot7.ramaddr;
    u8* sect7n0 = memmem(search7, params->boot7.size, boot7n0, 4);
    if (!sect7n0) {
        printf("7n0 needle not found!\n");
        return 1;
    }
    printf("landmark 7n0 @ 0x%08X\n", sect7n0);
    params->gui7.ramaddr = *(u32*)(sect7n0 + OFFSET_GUI7_7N0);
    printf("gui7 -> 0x%08X\n", params->gui7.ramaddr);

    // special region 0 offset in boot9
    u8* search9 = params->boot9.tmpaddr ? params->boot9.tmpaddr : params->boot9.ramaddr;
    u8* sect9n0 = memmem(search9, params->boot9.size, boot9n0, 4);
    if (!sect9n0) {
        printf("9n0 needle not found!\n");
        return 1;
    }
    printf("landmark 9n0 @ 0x%08X\n", sect9n0);
    params->gui9.ramaddr = *(u32*)(sect9n0 + OFFSET_GUI9_9N0);
    params->guidata.ramaddr = *(u32*)(sect9n0 + OFFSET_GUIDATA_9N0);
    printf("gui9 -> 0x%08X\nguidata -> 0x%08X\n", params->gui9.ramaddr, params->guidata.ramaddr);
    
    // apply patches to boot9
    printf("patch boot9\n");
    for (u32 i = 0; i < 7; i++) {
        void* nextnop = sect9n0 + nop9n0[i];
        for (u8* j = nextnop; j < (u8*)nextnop + 4; j++) *(u8*)j = 0;
        //u32* nextnop = sect9n0 + 2 + nop9n0[i];
        //printf("nop 0x%08X\n", nextnop);
        //*nextnop = 0;
    }

    return 0;

}