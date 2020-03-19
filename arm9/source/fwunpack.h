#pragma once

#include "nds/ndstypes.h"

#include "fwparams.h"

#define CONSOLE_TYPE_PHAT 0x00
#define CONSOLE_TYPE_PHAT_NEW 0xFF
#define CONSOLE_TYPE_LITE 0x20
// TODO: was there a korean phat?
#define CONSOLE_TYPE_KOREAN_LITE 0x35
#define CONSOLE_TYPE_IQUE 0x43
#define CONSOLE_TYPE_IQUE_LITE 0x63

extern int fwunpack_stage1();
extern int fwunpack_stage3();