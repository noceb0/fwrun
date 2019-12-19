#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	$(shell basename $(CURDIR))
export TOPDIR		:=	$(CURDIR)


.PHONY: bootloader arm7 arm9

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: $(TARGET).nds

#---------------------------------------------------------------------------------
$(TARGET).nds	:	arm7/$(TARGET).elf arm9/$(TARGET).elf
	ndstool	-c $(TARGET).nds -7 arm7/$(TARGET).elf -9 arm9/$(TARGET).elf -b $(TARGET).bmp "fwrun;boot firmware.bin;bitflip"

#---------------------------------------------------------------------------------
arm7 arm7/$(TARGET).elf:
	$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
arm9 arm9/$(TARGET).elf: bootloader
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
data:
	@mkdir -p build

bootloader: data
	@$(MAKE) -C bootloader LOADBIN=$(CURDIR)/build/load.bin

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	$(MAKE) -C bootloader clean
	rm -rf build $(TARGET).nds $(TARGET).arm7 $(TARGET).arm9
