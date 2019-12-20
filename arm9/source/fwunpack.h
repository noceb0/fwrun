#ifndef FWUNPACK_H
#define FWUNPACK_H

#include "nds/ndstypes.h"

#include "fwparams.h"

#define CONSOLE_TYPE_PHAT 0x00
#define CONSOLE_TYPE_PHAT_NEW 0xFF
#define CONSOLE_TYPE_LITE 0x20
// TODO: was there a korean phat?
#define CONSOLE_TYPE_KOREAN_LITE 0x35
#define CONSOLE_TYPE_IQUE 0x43
#define CONSOLE_TYPE_IQUE_LITE 0x63

typedef struct {
    u16 part3_romaddr;
    u16 part4_romaddr;
    u16 part34_crc16;
    u16 part12_crc16;
    u8 fw_identifier[4];
    u16 part1_romaddr;
    u16 part1_ramaddr;
    u16 part2_romaddr;
    u16 part2_ramaddr;
    u16 shift_amounts;
    u16 part5_romaddr;
    u8 fw_timestamp[5];
    u8 console_type;
    u16 unused1;
    u16 user_settings_offset;
    u16 unknown1;
    u16 unknown2;
    u16 part5_crc16;
    u16 unused2;
} fwHeader;

extern int fwunpack_stage1();
extern int fwunpack_stage3();

#endif