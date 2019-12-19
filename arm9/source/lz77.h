
#ifndef LZ77_H
#define LZ77_H

#include "nds/ndstypes.h"

#include "get_data.h"

void Decompress_LZ77(GET_DATA get_data, u8* dest, int dest_size);

#endif