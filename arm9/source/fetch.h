#pragma once

#include <stdio.h>

#include "main.h"
#include "nds/ndstypes.h"

void fetchSeek(u32 offset);
u8 nextByte(bool encrypted);
void nextBlock(u8* buffer, u32 size);