#pragma once

// get vscode to shut the fuck up
#define NDS

// define to bundle in key1
//#define EMBEDDED_BIOS7

#include "fwunpack.h"

#define BIOS7_KEY1_OFFSET 0x30

extern fwunpackParams params;

#ifndef FWRUN_LOADER
#include <stdio.h>
extern FILE* image;
#endif