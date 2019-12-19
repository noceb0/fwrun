
#ifndef GET_DATA_H
#define GET_DATA_H

#include "nds/ndstypes.h"

typedef u8  (* FN_GET_U8 ) (void) ;
typedef u16 (* FN_GET_U16) (void) ;
typedef u32 (* FN_GET_U32) (void) ;
typedef void (* FN_SET_ADDRESS) (u8* addr) ;

typedef struct {
	FN_GET_U8	get_u8;
	FN_GET_U16	get_u16;
	FN_GET_U32	get_u32;
	FN_SET_ADDRESS	set_address;
} GET_DATA;

#endif
