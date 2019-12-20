#ifndef MAIN_PARAMS_H
#define MAIN_PARAMS_H

// get vscode to shut the fuck up
#define NDS

// define these to bundle in arm9/data/firmware.bin and arm9/data/bios7
// both of these being defined also disables fat init
#define EMBEDDED_FIRMWARE
#define EMBEDDED_BIOS7

#include "fwunpack.h"

#define BIOS7_KEY1_OFFSET 0x30

extern fwunpackParams params;

#endif