
#include "get_normal_data.h"
#include "encryption.h"
#include <string.h>
#include <stdio.h>


#define DATA_BUFFER_SIZE 8

u8* normal_src_pointer;

u8 get_normal_u8 (void) {
	u8 output;
	output = normal_src_pointer[0];
	normal_src_pointer += 1;
	return output;
}

u16 get_normal_u16 (void) {
	u16 output;
	output = get_normal_u8();
	output |= (get_normal_u8() << 8);
	return output;
}

u32 get_normal_u32 (void) {
	u32 output;
	output = get_normal_u8();
	output |= (get_normal_u8() << 8);
	output |= (get_normal_u8() << 16);
	output |= (get_normal_u8() << 24);
	return output;
}

void set_normal_address (u8* addr) {
	normal_src_pointer = addr;
}

const GET_DATA get_normal_data = {
	(FN_GET_U8)			get_normal_u8,
	(FN_GET_U16)		get_normal_u16,
	(FN_GET_U32)		get_normal_u32,
	(FN_SET_ADDRESS)	set_normal_address
};

