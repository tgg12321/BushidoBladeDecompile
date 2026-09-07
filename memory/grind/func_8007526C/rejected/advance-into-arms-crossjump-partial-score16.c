/* REJECTED as a submission but CONFIRMED as a mechanism -- s12 (2026-09-07, structural).
 * score 16, build_insns 95 (target 91), baseline 13/93.
 *
 * WHAT IT DISPROVES.  s11 banked an instance kill saying loop-time insn_count and emitted
 * build_insns move 1:1 for every ordinary-C statement, which would make the Judge-named
 * "insn_count >= 120 while still collapsing to 91" axis unreachable.  That coupling is NOT 1:1.
 * Duplicating the real loop-advance statement `i++` out of its single post-switch position into all
 * five switch arms (the sanctioned duplicated-statement-into-arms shape,
 * .claude/rules/duplicated-statement-into-arms.md) measures:
 *     loop-time insn_count  91 -> 96   (+5, one copy per arm; .loop: "Loop from 14 to 275: 96 real insns.")
 *     emitted build_insns   93 -> 95   (+2)
 * i.e. jump2 cross-jumping re-merged 3 of the 5 copies to identical bytes while loop.c had already
 * counted all 5.  GCC 2.7.2 runs the first jump pass with cross-jumping OFF and only jump2 with it
 * on, which is why the copies survive to loop time.
 *
 * WHY IT IS STILL NOT A SUBMISSION.  The observed exchange rate is 5 loop-time insns per 2 surviving
 * emitted insns.  Reaching insn_count 120 (the count at which threshold 119 rejects the four
 * savings-1/lifetime-1 comparison constants at loop.c:1631) needs +29 loop-time insns, which at this
 * rate costs about +12 emitted words against a target that is exactly 91.  A usable form needs a
 * duplication whose copies merge COMPLETELY (0 surviving words); this one does not.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
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
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            i++;
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            i++;
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            i++;
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            i++;
            break;
        default:
            i++;
            break;
        }
    } while (i < 2);
}
