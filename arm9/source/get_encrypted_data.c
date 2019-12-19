
#include "get_encrypted_data.h"
#include "encryption.h"
#include <string.h>
#include <stdio.h>


#define DATA_BUFFER_SIZE 8

u8 data_buffer[DATA_BUFFER_SIZE];
u8* src_pointer;
int data_pos;

u8 get_u8_e (void) {
	u8 output;
	if (data_pos > 7) {
		data_pos = 0;
		src_pointer += DATA_BUFFER_SIZE;
		memcpy (data_buffer, src_pointer, DATA_BUFFER_SIZE);
		crypt_64bit_down (data_buffer);
	}
	output = data_buffer[data_pos];
	data_pos += 1;
	return output;
}

u16 get_u16_e (void) {
	u16 output;
	output = get_u8_e();
	output |= (get_u8_e() << 8);
	return output;
}

u32 get_u32_e (void) {
	u32 output;
	output = get_u8_e();
	output |= (get_u8_e() << 8);
	output |= (get_u8_e() << 16);
	output |= (get_u8_e() << 24);
	return output;
}

void set_address_e (u8* addr) {
	src_pointer = addr;
	memcpy (data_buffer, src_pointer, DATA_BUFFER_SIZE);
	data_pos = 0;
	crypt_64bit_down (data_buffer);
}

const GET_DATA get_encrypted_data = {
	(FN_GET_U8)			get_u8_e,
	(FN_GET_U16)		get_u16_e,
	(FN_GET_U32)		get_u32_e,
	(FN_SET_ADDRESS)	set_address_e
};

