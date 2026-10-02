/* Rodata continuation of the text1a_b_pre cluster (split out of
 * src/text1a_b_pre_rodata.c on 2026-09-30 so that text1b.o (since the Q89 split, text1b_tu1b.o) can supply func_80058580's
 * jump tables at 0x8001585C between the two halves).
 * 0x800158B4..0x800158DF: the sound-bank loader's strings (snd_LoadCommonVab,
 * func_8005C2A8), rodata of text1b_tu1b.c's TU (docs/grind/rodata-align-2026-09-30.md
 * section 9). */
#include "common.h"
/* D_800158B4: 1 string, 24B @ 0x800158B4 */
const char D_800158B4[24] = "common_vab start:%08x\n";

/* D_800158CC: 1 string, 20B @ 0x800158CC */
const char D_800158CC[20] = "vab id:%d mistake\n";


/* NOTE: the cluster continues in src/text1a_b_mid_rodata.c. The bytes from
 * 0x800158E0 through 0x80015A0B are supplied by build/src/text1b_tu1c.o
 * (func_80061064's string, func_80065800's two compiler-generated switch
 * tables, func_8006B578's compiler-generated switch table and the warning string, up to 0x800159AF) and build/src/text1b_tu1d.o (the func_8006E534 and
 * func_8006ECF4 tables, from 0x800159B0). */
