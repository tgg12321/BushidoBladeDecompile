/* REJECTED / MECHANISM PROOF (s1 2026-09-07) -- NOT A SUBMISSION.  Eight extra loop-invariant
 * movables (distinct non-zero constants 0x101..0x108, each STORED to memory so the use is
 * register-required and cse cannot fold it away, giving lifetime >= 1).
 *
 * MEASURED: sandbox score 17, build_insns 106 (target 91).  The point is not the score, it is the
 * .loop dump (tmp/grind/func_8007526C/s1/decay8.loop.txt):
 *     Loop from 14 to 303: 108 real insns.
 *     Insn 25/30/35/40/45 (regno 75-79, life 1, savings 1)  moved to 311/313/315/317/319
 *     Insn 50/55/60      (regno 80-82)                      not desirable
 *     Insn 269/275/281/284 (regno 134/136/137/138)          NOT DESIRABLE   <-- the four switch
 *                                                                              comparison constants
 *     Insn 116 (regno 100, life 3, savings 2)               moved to 321    <-- the 0xC8 still hoists
 * i.e. `threshold -= 3` at tools/gcc-2.7.2/loop.c:1904 decays threshold 122 -> 119 -> 116 -> 113
 * -> 110 -> 107 across the first five moved movables, and 107 * 1 * 1 = 107 < insn_count 108 makes
 * every later savings-1/lifetime-1 movable fail the desirability test at loop.c:1631 -- while the
 * 0xC8 movable (107 * 2 * 3 = 642 >= 108) still hoists.  That is EXACTLY the target's movable
 * shape (asm/funcs/func_8007526C.s:2-3 pre-header, `addiu $v0,$zero,N` inside the dispatch), with
 * no build-flag change and no arming loop.  This is a SECOND, independent route to the target
 * shape alongside s8's moved_once doubling.
 *
 * REJECTED because the eight carriers are dead stores of invented constants into unused table
 * slots: no semantic purpose, no sanctioned family, and 13 surviving instructions of cost.  See
 * hypotheses.md [s1] for the arithmetic bounding how cheap any legitimate carrier could be.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 m0,m1,m2,m3,m4,m5,m6,m7;

    base = D_800A36A0;
    i = 0;
    do {
        p = base + i * 2;
        m0 = 257; *(u16 *)(p + 0x40) = m0;
        m1 = 258; *(u16 *)(p + 0x42) = m1;
        m2 = 259; *(u16 *)(p + 0x44) = m2;
        m3 = 260; *(u16 *)(p + 0x46) = m3;
        m4 = 261; *(u16 *)(p + 0x48) = m4;
        m5 = 262; *(u16 *)(p + 0x4A) = m5;
        m6 = 263; *(u16 *)(p + 0x4C) = m6;
        m7 = 264; *(u16 *)(p + 0x4E) = m7;
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
