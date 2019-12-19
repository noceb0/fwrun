# fwrun

boot nds firmware dumps on nds/dsi/3ds

international firmware is fully functional on dsi/3ds, and ique/korean firmware is partially working (though opening pictochat/download play/settings crashes on nds and ique/korean on dsi/3ds; it's probably a memory corruption that i havent fixed yet)

![korean nds firmware running on an o3ds](https://raw.githubusercontent.com/noceb0/fwrun/master/repo/k_3ds.jpg)


## special thanks

- chishm (fwunpack (used to parse the firmware header and unpack firmware.bin))
- devkitPro (devkitarm, libnds, nds-bootloader (on which i base my hardware reset/relocate/jump to bootcode stuff))