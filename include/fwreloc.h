#pragma once

#define RELOC_BOOT7 true
#define RELOC_BOOT9 true
#define RELOC_GUI7 false
#define RELOC_GUI9 false
#define RELOC_GUIDATA false

#define LOADADDR_BOOT7(p) (RELOC_BOOT7 ? &p.boot7.tmpaddr : &p.boot7.ramaddr)
#define LOADADDR_BOOT9(p) (RELOC_BOOT9 ? &p.boot9.tmpaddr : &p.boot9.ramaddr)