#include "common.h"
#include "include_asm.h"

/* Padding NOP macro - emits NOP instructions between functions to match original layout */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* --- Functions 0x8007B244 - 0x8007FF7C (text2 segment) --- */

INCLUDE_ASM("asm/funcs", func_8007B244);
INCLUDE_ASM("asm/funcs", func_8007B2A0);
INCLUDE_ASM("asm/funcs", func_8007B33C);
INCLUDE_ASM("asm/funcs", func_8007B3A8);
INCLUDE_ASM("asm/funcs", func_8007B4D0);
INCLUDE_ASM("asm/funcs", func_8007B564);
INCLUDE_ASM("asm/funcs", func_8007B600);
INCLUDE_ASM("asm/funcs", func_8007B664);
INCLUDE_ASM("asm/funcs", func_8007B6C8);
INCLUDE_ASM("asm/funcs", func_8007B78C);
INCLUDE_ASM("asm/funcs", func_8007B844);
INCLUDE_ASM("asm/funcs", func_8007B8DC);
INCLUDE_ASM("asm/funcs", func_8007B93C);
INCLUDE_ASM("asm/funcs", func_8007B9B0);
INCLUDE_ASM("asm/funcs", func_8007BAB4);
INCLUDE_ASM("asm/funcs", func_8007BBD0);
INCLUDE_ASM("asm/funcs", func_8007BC08);
INCLUDE_ASM("asm/funcs", func_8007C0B0);
INCLUDE_ASM("asm/funcs", func_8007C0E8);
INCLUDE_ASM("asm/funcs", func_8007C118);
INCLUDE_ASM("asm/funcs", func_8007C154);
INCLUDE_ASM("asm/funcs", func_8007C1D8);
INCLUDE_ASM("asm/funcs", func_8007C21C);
INCLUDE_ASM("asm/funcs", func_8007C248);
INCLUDE_ASM("asm/funcs", func_8007C2A0);
INCLUDE_ASM("asm/funcs", func_8007C4B8);
INCLUDE_ASM("asm/funcs", func_8007C748);
INCLUDE_ASM("asm/funcs", func_8007C7A0);
INCLUDE_ASM("asm/funcs", func_8007C86C);
INCLUDE_ASM("asm/funcs", func_8007C938);
INCLUDE_ASM("asm/funcs", func_8007C97C);
INCLUDE_ASM("asm/funcs", func_8007CA00);
INCLUDE_ASM("asm/funcs", func_8007CAB0);
INCLUDE_ASM("asm/funcs", func_8007CAC8);
INCLUDE_ASM("asm/funcs", func_8007CBB0);
INCLUDE_ASM("asm/funcs", func_8007CE0C);
INCLUDE_ASM("asm/funcs", func_8007D048);
INCLUDE_ASM("asm/funcs", func_8007D2CC);
INCLUDE_ASM("asm/funcs", func_8007D2F4);
INCLUDE_ASM("asm/funcs", func_8007D308);
INCLUDE_ASM("asm/funcs", func_8007D358);
INCLUDE_ASM("asm/funcs", func_8007D3A4);
INCLUDE_ASM("asm/funcs", func_8007D3D4);
INCLUDE_ASM("asm/funcs", func_8007D3F8);
INCLUDE_ASM("asm/funcs", func_8007D6D8);
INCLUDE_ASM("asm/funcs", func_8007D9C4);
INCLUDE_ASM("asm/funcs", func_8007DB20);
INCLUDE_ASM("asm/funcs", func_8007DC68);
INCLUDE_ASM("asm/funcs", func_8007DC9C);
INCLUDE_ASM("asm/funcs", func_8007DE08);
INCLUDE_ASM("asm/funcs", func_8007DEE4);
INCLUDE_ASM("asm/funcs", func_8007DF10);
PAD_NOPS_1; /* 1 NOP after func_8007DF10 */
INCLUDE_ASM("asm/funcs", func_8007DF20);
INCLUDE_ASM("asm/funcs", func_8007DF5C);
INCLUDE_ASM("asm/funcs", func_8007DFEC);

/* Data blob D_8007E08C between func_8007DFEC and func_8007E094 */
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "    .include \"asm/funcs/D_8007E08C.s\"\n"
    "    .set reorder\n"
    "    .set at\n"
);

INCLUDE_ASM("asm/funcs", func_8007E094);
PAD_NOPS_2; /* 2 NOPs after func_8007E094 */
INCLUDE_ASM("asm/funcs", func_8007E11C);
PAD_NOPS_3; /* 3 NOPs after func_8007E11C */
INCLUDE_ASM("asm/funcs", func_8007E1AC);
PAD_NOPS_1; /* 1 NOP after func_8007E1AC */
INCLUDE_ASM("asm/funcs", func_8007E43C);
PAD_NOPS_3; /* 3 NOPs after func_8007E43C */
INCLUDE_ASM("asm/funcs", func_8007E4DC);
PAD_NOPS_1; /* 1 NOP after func_8007E4DC */
INCLUDE_ASM("asm/funcs", func_8007E5EC);
INCLUDE_ASM("asm/funcs", func_8007E74C);
INCLUDE_ASM("asm/funcs", func_8007E8AC);
INCLUDE_ASM("asm/funcs", func_8007E8DC);
PAD_NOPS_3; /* 3 NOPs after func_8007E8DC */
INCLUDE_ASM("asm/funcs", func_8007EA0C);
PAD_NOPS_2; /* 2 NOPs after func_8007EA0C */
INCLUDE_ASM("asm/funcs", func_8007EB4C);
PAD_NOPS_1; /* 1 NOP after func_8007EB4C */
INCLUDE_ASM("asm/funcs", func_8007EC5C);
PAD_NOPS_1; /* 1 NOP after func_8007EC5C */
INCLUDE_ASM("asm/funcs", func_8007ED6C);
INCLUDE_ASM("asm/funcs", func_8007EDBC);
PAD_NOPS_3; /* 3 NOPs after func_8007EDBC */
INCLUDE_ASM("asm/funcs", func_8007EEEC);
INCLUDE_ASM("asm/funcs", func_8007EF1C);
INCLUDE_ASM("asm/funcs", func_8007EF4C);
INCLUDE_ASM("asm/funcs", func_8007EF6C);
PAD_NOPS_3; /* 3 NOPs after func_8007EF6C */
INCLUDE_ASM("asm/funcs", func_8007EF8C);
PAD_NOPS_1; /* 1 NOP after func_8007EF8C */
INCLUDE_ASM("asm/funcs", func_8007EF9C);
INCLUDE_ASM("asm/funcs", func_8007EFBC);
INCLUDE_ASM("asm/funcs", func_8007EFDC);
PAD_NOPS_2; /* 2 NOPs after func_8007EFDC */
INCLUDE_ASM("asm/funcs", func_8007EFFC);
INCLUDE_ASM("asm/funcs", func_8007F0BC);
PAD_NOPS_1; /* 1 NOP after func_8007F0BC */
INCLUDE_ASM("asm/funcs", func_8007F21C);
PAD_NOPS_1; /* 1 NOP after func_8007F21C */
INCLUDE_ASM("asm/funcs", func_8007F24C);
PAD_NOPS_3; /* 3 NOPs after func_8007F24C */
INCLUDE_ASM("asm/funcs", func_8007F2AC);
PAD_NOPS_2; /* 2 NOPs after func_8007F2AC */
INCLUDE_ASM("asm/funcs", func_8007F2DC);
PAD_NOPS_2; /* 2 NOPs after func_8007F2DC */
INCLUDE_ASM("asm/funcs", func_8007F35C);
PAD_NOPS_1; /* 1 NOP after func_8007F35C */
INCLUDE_ASM("asm/funcs", func_8007F5EC);
PAD_NOPS_1; /* 1 NOP after func_8007F5EC */
INCLUDE_ASM("asm/funcs", func_8007F87C);
PAD_NOPS_2; /* 2 NOPs after func_8007F87C */
INCLUDE_ASM("asm/funcs", func_8007FA1C);
PAD_NOPS_2; /* 2 NOPs after func_8007FA1C */
INCLUDE_ASM("asm/funcs", func_8007FBBC);
PAD_NOPS_2; /* 2 NOPs after func_8007FBBC */
INCLUDE_ASM("asm/funcs", func_8007FD5C);
INCLUDE_ASM("asm/funcs", func_8007FEDC);
PAD_NOPS_1; /* 1 NOP after func_8007FEDC */
INCLUDE_ASM("asm/funcs", func_8007FF7C);
