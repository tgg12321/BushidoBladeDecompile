/* Rodata sub-TU: the second half of the 101C.rodata_text1a_b_pre cluster.
 * Split out 2026-09-16 at 0x80015988 so that func_8006B578's compiler-generated
 * switch table -- previously hand-extracted here as jtbl_80015988 -- is emitted
 * by build/src/text1b.o, the TU that actually owns the switch. GCC 2.7.2 places a
 * switch's ADDR_VEC in the .rodata of the TU containing the switch, so the table
 * now comes from real compiler output rather than a transcribed array. bb2.ld
 * orders the run as: text1a_b_pre_rodata.o, text1b.o, text1a_b_mid_rodata.o.
 * Same pattern bb2.ld already uses for func_80077B30's table via text1b_b.o. */
#include "common.h"
/* D_800159A0: 16B @ 0x800159A0 — "warning\n" + alignment + empty trailing string */
const char D_800159A0[16] = "warning\n";

/* jtbl_800159B0: 8 words (32B) @ 0x800159B0 */
const u32 jtbl_800159B0[8] = {
    0x8006E5D8,
    0x8006E618,
    0x8006E5F0,
    0x8006E618,
    0x8006E5E4,
    0x8006E628,
    0x8006E5E4,
    0x00000000,
};

/* jtbl_800159D0: 15 words (60B) @ 0x800159D0 */
const u32 jtbl_800159D0[15] = {
    0x8006EE74,
    0x8006EF10,
    0x8006EF10,
    0x8006EE88,
    0x8006EF10,
    0x8006EF10,
    0x8006EF10,
    0x8006EF10,
    0x8006EF10,
    0x8006EF10,
    0x8006EF10,
    0x8006EF10,
    0x8006EE38,
    0x8006EE4C,
    0x8006EE60,
};

/* jtbl_80015A0C: 6 words (24B) @ 0x80015A0C */
const u32 jtbl_80015A0C[6] = {
    0x800748F0,
    0x80074984,
    0x800749D8,
    0x80074A58,
    0x80074AB0,
    0x00000000,
};

/* jtbl_80015A24: 6 words (24B) @ 0x80015A24 */
const u32 jtbl_80015A24[6] = {
    0x80077438,
    0x80077460,
    0x800774B0,
    0x80077540,
    0x800775D0,
    0x80077670,
};

