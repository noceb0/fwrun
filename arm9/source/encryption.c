

#include "encryption.h"
#include "main.h"

#include <string.h>

u32 keybuf[KEY1_SIZE / sizeof(u32)];

void crypt_64bit_up(u8* ptr) {
	u32 x = *((u32*)&ptr[4]);
	u32 y = *((u32*)&ptr[0]);
	u32 z;

	for (int i = 0; i < 0x10; i++) {
		z = keybuf[i] ^ x;
		x = keybuf[0x012 + ((z>>24)&0xff)];
		x = keybuf[0x112 + ((z>>16)&0xff)] + x;
		x = keybuf[0x212 + ((z>> 8)&0xff)] ^ x;
		x = keybuf[0x312 + ((z>> 0)&0xff)] + x;
		x = y ^ x;
		y = z;
	}

	*((u32*)&ptr[0]) = x ^ keybuf[0x10];
	*((u32*)&ptr[4]) = y ^ keybuf[0x11];
}

void crypt_64bit_down(u8* ptr) {
	u32 x = *((u32*)&ptr[4]);
	u32 y = *((u32*)&ptr[0]);
	u32 z;

	for (int i = 0x11; i > 0x01; i--) {
		z = keybuf[i] ^ x;
		x = keybuf[0x012 + ((z>>24)&0xff)];
		x = keybuf[0x112 + ((z>>16)&0xff)] + x;
		x = keybuf[0x212 + ((z>> 8)&0xff)] ^ x;
		x = keybuf[0x312 + ((z>> 0)&0xff)] + x;
		x = y ^ x;
		y = z;
	}

	*((u32*)&ptr[0]) = x ^ keybuf[0x01];
	*((u32*)&ptr[4]) = y ^ keybuf[0x00];
}

u32 bswap_32bit(u32 in) {
	u8 a,b,c,d;
	a = (u8)((in >>  0) & 0xff);
	b = (u8)((in >>  8) & 0xff);
	c = (u8)((in >> 16) & 0xff);
	d = (u8)((in >> 24) & 0xff);

	u32 out = (a << 24) | (b << 16) | (c << 8) | (d << 0);

	return out;
}

u8 keycode[12];
void apply_keycode(u32 modulo) {
	u32 scratch[2];

	crypt_64bit_up(keycode+4);
	crypt_64bit_up(keycode+0);
	memset(scratch, 0, 8);

	for (int i = 0; i < 0x12; i+=1) {
		keybuf[i] = keybuf[i] ^ bswap_32bit(*((u32*)&keycode[(i*4) % modulo]));
	}
	for (int i = 0; i < 0x412; i+=2) {
		crypt_64bit_up((u8*)scratch);
		keybuf[i] = scratch[1];
		keybuf[i+1] = scratch[0];
	}
}

void init_keycode(u32 idcode, u32 level, u32 modulo) {
	memcpy(keybuf, params.key1data, KEY1_SIZE);
	printf("keybuff %X\n", keybuf);
	*((u32*)&keycode[0]) = idcode;
	*((u32*)&keycode[4]) = idcode/2;
	*((u32*)&keycode[8]) = idcode*2;

	if (level >= 1) apply_keycode(modulo);	// first apply (always)
	if (level >= 2) apply_keycode(modulo);	// second apply (optional)
	*((u32*)&keycode[4]) = *((u32*)&keycode[4]) * 2;
	*((u32*)&keycode[8]) = *((u32*)&keycode[8]) / 2;
	if (level >= 3) apply_keycode(modulo);	// third apply (optional)
}
