# fwrun

boot nds firmware dumps on nds/dsi/3ds

international firmware is fully functional on dsi/3ds, and ique/korean firmware is partially working (though opening pictochat/download play/settings crashes on nds and ique/korean on dsi/3ds; it's probably a memory corruption that i havent fixed yet)

![korean nds firmware running on an o3ds](https://raw.githubusercontent.com/noceb0/fwrun/master/repo/k_3ds.jpg)


## how do i use it?
there arent binaries yet, so you'll have to build it yourself with devkitarm+libnds
```
git clone https://github.com/noceb0/fwrun.git
cd fwrun
make -j4
```

this will produce an fwrun.nds file

you will also need firmware.bin (nds firmware flash chip image) and bios7.bin (nds arm7 bios) which must be acquired from a real ds (bios7.bin can be dumped from a dsi or 3ds in ntr mode, but firmware.bin must be dumped from an nds) using something like fwtool

place these files in the same directory as fwrun.nds and you should be ready to go


## special thanks

- chishm (fwunpack (used to parse the firmware header and unpack firmware.bin))
- devkitPro Team (devkitarm, libnds, nds-bootloader (on which i base my hardware reset/relocate/jump to bootcode stuff))