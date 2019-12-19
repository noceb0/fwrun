
#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include "nds/ndstypes.h"
void init_keycode (u32 idcode, u32 level, u32 modulo);
void crypt_64bit_down (u8* ptr);
void crypt_64bit_up (u8* ptr);

#endif
