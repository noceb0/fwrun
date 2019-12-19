#include "part345_comp.h"
#include <string.h>

u32 ror_u32 (u32 val, int bits) {
	return (val >> bits) | (val << (32 - bits));
}

u32 get_u32 (u8* data, int offset) {
	return (data[offset+0] << 0) | (data[1] << 8) | (data[offset+2] << 16) | (data[offset+3] << 24);
}

// Big endian version
u32 get_u32_be (u8* data, int offset) {
	return (data[offset+0] << 24) | (data[offset+1] << 16) | (data[offset+2] << 8) | (data[offset+3] << 0);
}

void put_u32 (u8* data, int offset, u32 val) {
	data [offset + 0] = (u8) (val >>  0);
	data [offset + 1] = (u8) (val >>  8);
	data [offset + 2] = (u8) (val >> 16);
	data [offset + 3] = (u8) (val >> 24);
}

void put_u16 (u8* data, int offset, u16 val) {
	data [offset + 0] = (u8) (val >>  0);
	data [offset + 1] = (u8) (val >>  8);
}


u32 byteswap_u32 (u32 val) {
	u32 temp;
	temp = (val ^ ror_u32 (val, 16)) & 0xff00ffff;
	val = ror_u32 (val, 8);
	val = val ^ (temp >> 8);
	return val;
}
	
typedef struct {
	u32 shift;	
	u32 dat1;	
	u8* addr;	
	u32 data;	
	u32 dat4;	
} DATA_BITS;

DATA_BITS data_bits_0;		// 0x023def20
DATA_BITS data_bits_1;		// 0x023def24

u16 data_table_0[0x100];	// 0x023def60 size 0x200 bytes
u16 data_table_1[0x800];	// 0x023e0160 size 0x1000 bytes
u16 data_table_2[0x800];	// 0x023df160 size 0x1000 bytes

u16 data_table_3[0x2000];	// 0x023e1160 size 0x4000 bytes
u16 data_table_4[0x2000];	// 0x023e5160 size 0x4000 bytes


u8 get_bit (DATA_BITS* data_bits) {
	u8 bit;
	bit = data_bits->data >> 31;
	if (data_bits->shift == 31) {
		data_bits->data = get_u32_be (data_bits->addr, 0);
		data_bits->addr += 4;
		data_bits->shift = 0;
	} else {
		data_bits->data <<= 1;
		data_bits->shift += 1;
	}
	return bit;
}

u32 get_bits (DATA_BITS* data_bits, u32 num_bits) {
	u32 retval = data_bits->data >> (32 - num_bits);
	u32 temp;
	if ((data_bits->shift + num_bits) == 32) {
		data_bits->data = get_u32_be (data_bits->addr, 0);
		data_bits->addr += 4;
		data_bits->shift = 0;
	} else if ((data_bits->shift + num_bits) < 32) {
		data_bits->data <<= num_bits;
		data_bits->shift += num_bits;
	} else {	// 32 < (data_bits->shift + num_bits) < 64
		temp = get_u32_be (data_bits->addr, 0);
		data_bits->addr += 4;
		retval |= temp >> (64 - (data_bits->shift + num_bits));
		data_bits->shift += (num_bits - 32);
		data_bits->data = temp << data_bits->shift;
	}
	return retval;
}

//fw_decompress
u32 part345_decompress (u8* dest, u8* src) {
	u32 size, decompressed_size;
	u32 offset;
	u32 bits;
	u16 data;
	u32 temp_data;
	bool loop;
	u16* temp_table;
	
	size = get_u32_be (src, 4) & 0x00ffffff;
	decompressed_size = size;

	if (dest == NULL) {
		return decompressed_size;
	}
	
	offset = get_u32_be (src, 8);
	
	data_bits_0.shift = 0;
	data_bits_0.dat1 = 0x200;
	data_bits_0.addr = src + 12;
	data_bits_0.data = 0;

	data_bits_1.shift = 0;
	data_bits_1.dat1 = 0x800;
	data_bits_1.addr = src + offset;
	data_bits_1.data = 0;
	
	
	get_bits (&data_bits_0, 0x20);
	get_bits (&data_bits_1, 0x20);
	
	// Build the first tree
	temp_table = data_table_0;
	data = data_bits_0.dat1;
 	loop = true;
	while (loop) {
		while ((bits = get_bit (&data_bits_0)) != 0) {
			*(temp_table++) = (data | 0x8000);
			*(temp_table++) = (data | 0x4000);
			data += 1;
		}
		
		bits = get_bits (&data_bits_0, 9);

		do {
			temp_data = *(--temp_table);
			if ((temp_data & 0x8000) == 0) {
				data_table_1[temp_data & 0x3FFF] = (u16)bits;	
				break;
			} 
			data_table_2[temp_data & 0x3FFF] = (u16)bits;	
			bits = temp_data & 0x3FFF;
			if (temp_table == data_table_0) {
				loop = false;
				break;
			}
		} while (temp_table != data_table_0);
	}
	data_bits_0.dat4 = bits;	
	
	// Build the second tree
	temp_table = data_table_0;
	data = data_bits_1.dat1;
	loop = true;
	while (loop) {
		while ((bits = get_bit (&data_bits_1)) != 0) {
			*(temp_table++) = (data | 0x8000);
			*(temp_table++) = (data | 0x4000);
			data += 1;
		}
		
		bits = get_bits (&data_bits_1, 11);
	
		do {
			temp_data = *(--temp_table);		
			if ((temp_data & 0x8000) == 0) {
				data_table_3[temp_data & 0x3FFF] = bits;	
				break;
			} 
			data_table_4[temp_data & 0x3FFF] = bits;		
			bits = temp_data & 0x3FFF;
			if (temp_table == data_table_0) {
				loop = false;
				break;
			}
		} while (temp_table != data_table_0);
	}
	data_bits_1.dat4 = bits;

	// Decompress the data
	while (size > 0) {
		data = data_bits_0.dat4;
	
		while (data >= 0x200) {
			bits = get_bit (&data_bits_0);
			if (bits == 0) {
				data = data_table_1[data];	
			} else {
				data = data_table_2[data];	
			}
		}
		
		if (data < 0x100) {
			*(dest++) = (u8)data;
			size--;
		} else {
			temp_data = data_bits_1.dat4; 
			
			while ( temp_data >= 0x800) {
				bits = get_bit(&data_bits_1); 
				if (bits == 0) {
					temp_data = data_table_3[temp_data];
				} else {
					temp_data = data_table_4[temp_data];
				}
			}
			
			u32 copy_length = data - 253;
			u8* copy_source = (dest - temp_data) - 1;
			size -= copy_length;
			
			while (copy_length != 0) {
				*(dest++) = *(copy_source++);
				copy_length--;
			}
		}
	}
	
	return decompressed_size;
}

	
	
	
