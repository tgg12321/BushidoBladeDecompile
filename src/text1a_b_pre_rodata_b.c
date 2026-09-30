/* Rodata continuation of the text1a_b_pre cluster (split out of
 * src/text1a_b_pre_rodata.c on 2026-09-30 so that text1b.o can supply func_80058580's
 * jump tables at 0x8001585C between the two halves).
 * 0x800158B4..0x80015987: the sound-bank loader's strings (snd_LoadCommonVab,
 * func_8005C2A8), func_80061064's string and func_80065800's two tables. */
#include "common.h"
/* D_800158B4: 1 string, 24B @ 0x800158B4 */
const char D_800158B4[24] = "common_vab start:%08x\n";

/* D_800158CC: 1 string, 20B @ 0x800158CC */
const char D_800158CC[20] = "vab id:%d mistake\n";

/* D_800158E0: 24B @ 0x800158E0 — "eff prim over :%d \n" + alignment + empty trailing string */
const char D_800158E0[24] = "eff prim over :%d \n";

/* jtbl_800158F8: 18 words (72B) @ 0x800158F8 */
const u32 jtbl_800158F8[18] = {
    0x80065AC8,
    0x800659E4,
    0x800659E4,
    0x80065A34,
    0x80065A34,
    0x80065AA0,
    0x80065BCC,
    0x80065BCC,
    0x80065D1C,
    0x80065D1C,
    0x80065AEC,
    0x80065AEC,
    0x80065DC8,
    0x80065DC8,
    0x80065DC8,
    0x80065DC8,
    0x80065EF0,
    0x80065EF0,
};

/* jtbl_80015940: 18 words (72B) @ 0x80015940 */
const u32 jtbl_80015940[18] = {
    0x80066168,
    0x80066324,
    0x80066324,
    0x8006663C,
    0x8006663C,
    0x80066784,
    0x8006692C,
    0x80066968,
    0x800665D0,
    0x800665D0,
    0x8006692C,
    0x80066968,
    0x80066550,
    0x80066550,
    0x80066550,
    0x80066550,
    0x80066324,
    0x80066324,
};

/* NOTE: the cluster continues in src/text1a_b_mid_rodata.c. The bytes from
 * 0x80015988 through 0x80015A0B are supplied by build/src/text1b_tu1c.o: the
 * compiler-generated switch tables for func_8006B578 and func_8006ECF4 plus
 * the intervening warning string and func_8006E534 table. */
