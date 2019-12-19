/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2010
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/
#include <string.h>
#include <nds.h>
#include <nds/memory.h>
#include <nds/arm9/video.h>
#include <sys/stat.h>
#include <limits.h>

#include "fwparams.h"

#include "load_bin.h"

#include "loader_arm9.h"
#define LCDC_BANK_C (u16*)0x06840000

typedef struct {
    u32 __branch;
    fwunpackParams* params;
} strap7Args;

static void vramcpy(void* dst, const void* src, int len) {
	u16* dst16 = (u16*)dst;
	u16* src16 = (u16*)src;
	for ( ; len > 0; len -= 2) {
		*dst16++ = *src16++;
	}
}	

int loaderRun(fwunpackParams* params) {

	// Direct CPU access to VRAM bank C
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_LCD;
	// Load the loader/patcher into the correct address
	vramcpy(LCDC_BANK_C, load_bin, load_bin_size);

    // put strap7 args in place
    strap7Args* args = ((strap7Args*)LCDC_BANK_C);
    args->params = params;

    nocashMessage("irqDisable()");
	irqDisable(IRQ_ALL);

	VRAM_C_CR = VRAM_ENABLE | VRAM_C_ARM7_0x06000000;
	REG_EXMEMCNT |= ARM7_OWNS_ROM | ARM7_OWNS_CARD;
	*((vu32*)0x02FFFFFC) = 0;
	*((vu32*)0x02FFFE04) = (u32)0xE59FF018;
	*((vu32*)0x02FFFE24) = (u32)0x02FFFE04;

    nocashMessage("resetARM7()");
    resetARM7(0x06000000);

    nocashMessage("swiSoftReset()");
	swiSoftReset();
	return true;

}