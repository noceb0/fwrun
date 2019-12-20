/*-----------------------------------------------------------------
 boot.c

 BootLoader
 Loads a file into memory and runs it

 All resetMemory and startBinary functions are based
 on the MultiNDS loader by Darkain.
 Original source available at:
 http://cvs.sourceforge.net/viewcvs.py/ndslib/ndslib/examples/loader/boot/main.cpp

License:
 Copyright (C) 2005  Michael "Chishm" Chisholm

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

 If you use this code, please give due credit and email me about your
 project at chishm@hotmail.com

Helpful information:
 This code runs from VRAM bank C on ARM7
------------------------------------------------------------------*/

// nds and dldi-specific stuff removed by nocebo

#include <nds/ndstypes.h>
#include <nds/dma.h>
#include <nds/system.h>
#include <nds/interrupts.h>
#include <nds/timers.h>
#define ARM9
#undef ARM7
#include <nds/memory.h>
#include <nds/arm9/video.h>
#include <nds/arm9/input.h>
#undef ARM9
#define ARM7
#include <nds/arm7/audio.h>
#include <nds/arm7/sdmmc.h>
#include "boot.h"

#include "fwparams.h"
#include "fwreloc.h"

void arm7clearRAM();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Important things
#define TEMP_MEM 0x02FFD000
#define TWL_HEAD 0x02FFE000
#define NDS_HEAD 0x02FFFE00
#define TEMP_ARM9_START_ADDRESS (*(vu32*)0x02FFFFF4)
#define ARM9_PTR 0x02FFFE24

extern unsigned long _start;
extern fwunpackParams* params;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Firmware stuff

#define FW_READ        0x03

void boot_readFirmware (uint32 address, uint8 * buffer, uint32 size) {
  uint32 index;

  // Read command
  while (REG_SPICNT & SPI_BUSY);
  REG_SPICNT = SPI_ENABLE | SPI_CONTINUOUS | SPI_DEVICE_NVRAM;
  REG_SPIDATA = FW_READ;
  while (REG_SPICNT & SPI_BUSY);

  // Set the address
  REG_SPIDATA =  (address>>16) & 0xFF;
  while (REG_SPICNT & SPI_BUSY);
  REG_SPIDATA =  (address>>8) & 0xFF;
  while (REG_SPICNT & SPI_BUSY);
  REG_SPIDATA =  (address) & 0xFF;
  while (REG_SPICNT & SPI_BUSY);

  for (index = 0; index < size; index++) {
    REG_SPIDATA = 0;
    while (REG_SPICNT & SPI_BUSY);
    buffer[index] = REG_SPIDATA & 0xFF;
  }
  REG_SPICNT = 0;
}


static inline void copyLoop(u32* dest, const u32* src, u32 size) {
	size = (size +3) & ~3;
	do {
		*dest++ = *src++;
	} while (size -= 4);
}


/*-------------------------------------------------------------------------
resetMemory_ARM7
Clears all of the NDS's RAM that is visible to the ARM7
Written by Darkain.
Modified by Chishm:
 * Added STMIA clear mem loop
--------------------------------------------------------------------------*/
void resetMemory_ARM7 (void)
{
	int i;
	u8 settings1, settings2;
	u32 settingsOffset = 0;

	REG_IME = 0;

	for (i=0; i<16; i++) {
		SCHANNEL_CR(i) = 0;
		SCHANNEL_TIMER(i) = 0;
		SCHANNEL_SOURCE(i) = 0;
		SCHANNEL_LENGTH(i) = 0;
	}

	REG_SOUNDCNT = 0;

	//clear out ARM7 DMA channels and timers
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}

	//arm7clearRAM();

	REG_IE = 0;
	REG_IF = ~0;
	(*(vu32*)(0x04000000-4)) = 0;  //IRQ_HANDLER ARM7 version
	(*(vu32*)(0x04000000-8)) = ~0; //VBLANK_INTR_WAIT_FLAGS, ARM7 version
	REG_POWERCNT = 1;  //turn off power to stuff

	// Get settings location
	boot_readFirmware((u32)0x00020, (u8*)&settingsOffset, 0x2);
	settingsOffset *= 8;

	// Reload DS Firmware settings
	boot_readFirmware(settingsOffset + 0x070, &settings1, 0x1);
	boot_readFirmware(settingsOffset + 0x170, &settings2, 0x1);

	if ((settings1 & 0x7F) == ((settings2+1) & 0x7F)) {
		boot_readFirmware(settingsOffset + 0x000, (u8*)0x02FFFC80, 0x70);
	} else {
		boot_readFirmware(settingsOffset + 0x100, (u8*)0x02FFFC80, 0x70);
	}

	((vu32*)0x040044f0)[2] = 0x202DDD1D;
	((vu32*)0x040044f0)[3] = 0xE1A00005;
	while((*(vu32*)0x04004400) & 0x2000000);

}


/*-------------------------------------------------------------------------
startBinary_ARM7
Jumps to the ARM7 NDS binary in sync with the display and ARM9
Written by Darkain.
Modified by Chishm:
 * Removed MultiNDS specific stuff
--------------------------------------------------------------------------*/
void startBinary_ARM7 (void) {

	REG_IME = 0;
	while (REG_VCOUNT != 191);
	while (REG_VCOUNT == 191);

	// Start ARM9
	*((vu32*)0x02FFFE24) = params->boot9.ramaddr;
	ARM9_START_FLAG = 1;

	// Start ARM7
	VoidFn arm7code = (VoidFn*)(params->boot7.ramaddr);
	arm7code();
}

void mpu_reset();
void mpu_reset_end();
void resetMemory2_ARM9();
void startBinary_ARM9();

void twl2ntr_arm9(fwunpackParams* params);

int main() {

	// ARM9 clears its memory part 2
	// copy ARM9 function to RAM, and make the ARM9 jump to it
	copyLoop((void*)TEMP_MEM, (void*)resetMemory2_ARM9, resetMemory2_ARM9_size);
	(*(vu32*)ARM9_PTR) = (u32)TEMP_MEM;	// Make ARM9 jump to the function
	// Wait until the ARM9 has completed its task
	while ((*(vu32*)ARM9_PTR) == (u32)TEMP_MEM);

	// ARM9 sets up mpu
	// copy ARM9 function to RAM, and make the ARM9 jump to it
	copyLoop((void*)TEMP_MEM, (u32*)mpu_reset, 0x100);
	(*(vu32*)ARM9_PTR) = (u32)TEMP_MEM;	// Make ARM9 jump to the function
	// Wait until the ARM9 has completed its task
	while ((*(vu32*)ARM9_PTR) == (u32)TEMP_MEM);

	// Get ARM7 to clear RAM
	resetMemory_ARM7();

	if (RELOC_BOOT7) copyLoop(params->boot7.ramaddr, params->boot7.tmpaddr, params->boot7.size);
	if (RELOC_BOOT9) copyLoop(params->boot9.ramaddr, params->boot9.tmpaddr, params->boot9.size);
	if (RELOC_GUI7) copyLoop(params->gui7.ramaddr, params->gui7.tmpaddr, params->gui7.size);
	if (RELOC_GUI9) copyLoop(params->gui9.ramaddr, params->gui9.tmpaddr, params->gui9.size);
	if (RELOC_GUIDATA) copyLoop(params->guidata.ramaddr, params->guidata.tmpaddr, params->guidata.size);

	// ARM9 enters a wait loop
	// copy ARM9 function to RAM, and make the ARM9 jump to it
	copyLoop((void*)TEMP_MEM, (void*)startBinary_ARM9, startBinary_ARM9_size);
	(*(vu32*)ARM9_PTR) = (u32)TEMP_MEM;	// Make ARM9 jump to the function

	// switch hardware into nds mode
	twl2ntr_arm9(params);

	// let's go!
	startBinary_ARM7();

	return 0;
	
}