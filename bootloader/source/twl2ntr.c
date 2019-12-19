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

#include "fwparams.h"

//---------------------------------------------------------------------------------
static u8 readTSC(u8 reg) {
//---------------------------------------------------------------------------------

	while (REG_SPICNT & 0x80);

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_TOUCH | SPI_CONTINUOUS;
	REG_SPIDATA = 1 | (reg << 1);

	while (REG_SPICNT & 0x80);

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_TOUCH;
	REG_SPIDATA = 0;

	while (REG_SPICNT & 0x80);
	return REG_SPIDATA;
}

//---------------------------------------------------------------------------------
static void writeTSC(u8 reg, u8 value) {
//---------------------------------------------------------------------------------

	while (REG_SPICNT & 0x80);

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_TOUCH | SPI_CONTINUOUS;
	REG_SPIDATA = reg << 1;

	while (REG_SPICNT & 0x80);

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_TOUCH;
	REG_SPIDATA = value;
}

void twl2ntr_arm9(fwunpackParams* params) {

    if(params->isDsi) {

        // switch the TSC into NTR compat mode
        writeTSC(0,3);
        writeTSC(3,0);
        readTSC(0x22);
        writeTSC(0x22,0xF0);
        writeTSC(0,0);
        writeTSC(0x52,0x80);
        writeTSC(0x51,0x00);
        writeTSC(0,3);
        readTSC(0x02);
        writeTSC(2,0x98);
        writeTSC(0,0xff);
        writeTSC(5,0);
        
        // causes a bunch of TWL stuff to switch into NTR-friendly mode
        REG_SCFG_ROM = 0x703;

    }

}