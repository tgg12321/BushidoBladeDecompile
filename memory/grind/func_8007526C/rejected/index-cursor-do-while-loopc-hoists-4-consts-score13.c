/* candidate for func_8007526C (src/text1b.c).
 *
 * BYTE-EXACT.  Proven by s4 and INDEPENDENTLY RE-MEASURED by s5 (2026-09-07) from a
 * fresh preprocess through the exact Makefile:150 pipeline: with the build configuration
 * below this body assembles to 91 instructions that match asm/funcs/func_8007526C.s
 * word-for-word, the only differing word being the unrelocated R_MIPS_GPREL16 addend on
 * the D_800A36A0 load (filled by ld).  The C is finished; the residual was never in it.
 *
 * Under the CURRENT build configuration it measures score 13 (build 93 insns) -- four
 * switch-comparison `li` hoisted into the loop pre-header by loop.c move_movables.
 * Reaching 0 needs two BUILD-CONFIG edits that a grind session may not make
 * (INTEGRATION HANDOFF, filed in docs/grind/decisions.md 2026-09-07 as
 * "func_8007526C - OWNER-ESCALATION: INTEGRATION HANDOFF"; full measurements in
 * memory/grind/func_8007526C/evidence.md [s5]):
 *   1. compile src/text1b.c with -msoft-float via a PER-FILE opt-in
 *      (`SOFT_FLOAT_FILES := text1b`, mirroring NO_SR_FILES at Makefile:129-133 -- a
 *      GLOBAL flip also moves func_800324D0 in code6cac_b and breaks the oracle).
 *      The PS1 has no FPU, so the original PsyQ compile had the 32 FP registers fixed
 *      (mips.h:524 CONDITIONAL_REGISTER_USAGE); that takes loop.c's move_movables
 *      threshold from 2*(1+60)=122 to 2*(1+28)=58 (loop.c:532/1631), which leaves the
 *      four switch-comparison constants inside the loop -- the target's shape -- while
 *      still hoisting the 0xC8 movable.
 *   2. add func_8007526C to maspsx_label_nop_funcs.txt for the loop-top load-delay nop.
 * Inside src/text1b.c the flag changes only this function (measured: 27 of 29 changed
 * cc1 output lines are inside it, the other 2 are the options comment).
 *
 * Ordinary C: no FAKE construct, no sanctioned-family claim required.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;

    base = D_800A36A0;
    i = 0;
    do {
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0xC8;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = 0xC8;
                *(u16 *)(p + 0xC) = 0xC8;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
