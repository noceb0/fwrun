#pragma once

#include <nds/ndstypes.h>

typedef enum {
    FW_NORMAL, FW_FLASHME, FW_IQUE, FW_KOREAN
} fwType;

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

typedef struct {
    u32 romaddr;
    u8* ramaddr;
    u8* tmpaddr;
    u32 romsize;
    u32 size;
} fwAddrs;

typedef struct {
    fwHeader* fwhdr;
    u8* key1data;
    fwAddrs boot9;
    fwAddrs boot7;
    fwAddrs gui9;
    fwAddrs gui7;
    fwAddrs guidata;
    fwType type;
    u32 isDsi;
} fwunpackParams;