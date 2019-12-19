#ifndef FWPARAMS_H
#define FWPARAMS_H

#include <nds/ndstypes.h>

typedef enum {
    FW_NORMAL, FW_FLASHME, FW_IQUE, FW_KOREAN
} fwType;

typedef struct {
    u32 romaddr;
    u8* ramaddr;
    u8* tmpaddr;
    u32 size;
} fwAddrs;

typedef struct {
    u8* fwdata;
    fwAddrs boot9;
    fwAddrs boot7;
    fwAddrs gui9;
    fwAddrs gui7;
    fwAddrs guidata;
    fwType type;
    u32 isDsi;
} fwunpackParams;

#endif