/* REJECTED -- s10 (2026-09-07).  score 13, build_insns 93, loop insn_count 92 -- i.e. the
 * carrier is INVISIBLE to loop.c.  One carrier shown; the 8-carrier form behaves identically.
 *
 * WHAT IT KILLS.  s9's frontier asked whether a threshold-decay carrier can be free.  The
 * cheapest conceivable carrier is a loop-invariant value whose only use is a dead register copy
 * (`m0 = (s32)base + 0x101; d0 = m0;` with d0 never read), the idea being that loop.c would
 * hoist `m0` (decaying threshold by 3 at loop.c:1904) and flow would then delete both insns.
 * MEASURED: cse1 deletes the whole chain BEFORE loop.c ever runs.  Pass-attributed by dump --
 * the constant 257 appears in tmp/grind/func_8007526C/dumps/text1b.rtl and text1b.jump and is
 * ABSENT from text1b.cse and text1b.loop.  A carrier therefore needs a LIVE use at loop.c time,
 * which means it survives into the output; s9 measured 8 memory-store carriers at score 17
 * (15 surviving instructions).  See hypotheses.md [s10] for the arithmetic: the minimum viable
 * carrier count is 7 at 2 insns each, so the decay axis cannot beat the do-nothing floor of 13.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 m0;
    s32 d0;

    base = D_800A36A0;
    i = 0;
    do {
        p = base + i * 2;
        m0 = (s32)base + 0x101; d0 = m0;
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
