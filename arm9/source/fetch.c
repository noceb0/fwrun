#include "fetch.h"
#include "encryption.h"

#include <string.h>

u32 base = 0;

#define POS() (ftell(image) - base)

// decryptification bullshit
#define DATA_BUFFER_SIZE 8
u8 enc_buffer[DATA_BUFFER_SIZE];
u8 plain_buffer[DATA_BUFFER_SIZE];
int data_pos = 0;

void cycleDecBuff() {
    data_pos = 0;
    fread(plain_buffer, 1, DATA_BUFFER_SIZE, image);
    memmove(enc_buffer, plain_buffer, DATA_BUFFER_SIZE);
	crypt_64bit_down(enc_buffer);
}

void fetchSeek(u32 offset) {
    printf("seek to %X\n", offset);
    fseek(image, offset, SEEK_SET);
    cycleDecBuff();
    //fseek(image, offset, SEEK_SET);
    base = ftell(image);
    printf("ftell = %X\n", base);
}

u8 nextByte(bool encrypted) {
    if (data_pos >= DATA_BUFFER_SIZE) cycleDecBuff();
	u8 buffer = (encrypted ? enc_buffer : plain_buffer)[data_pos];
	data_pos += 1;
    return buffer;
}

void nextBlock(u8* buffer, u32 size) {
    for (u32 i = 0; i < size; i++) {
        buffer[i] = nextByte(false);
    }
}