/* Rodata sub-TU: the tail of the 101C.rodata_text1a_b_pre cluster. bb2.ld
 * inserts text1b.o before this file so that text1b's compiler-generated switch
 * tables occupy their original addresses. As of func_8006ECF4's completion,
 * text1b.o supplies 0x80015988..0x80015A0B and this tail resumes at 0x80015A0C.
 * The same pattern is used below for func_80077B30 via text1b_b.o. */
#include "common.h"
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
