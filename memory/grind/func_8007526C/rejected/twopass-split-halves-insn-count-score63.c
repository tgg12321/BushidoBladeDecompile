/* REJECTED / MECHANISM PROOF -- s12 (2026-09-07, structural).  score 63, build_insns 99 (target 91).
 * This is frontier item #1 of the s11 ledger measured to completion: the SEMANTICALLY REAL
 * two-pass split (pass A over both entries handling only the shrink states 2/4, pass B over both
 * entries handling only the grow states 1/3).  The split is order-equivalent to the interleaved
 * single pass -- no arm reads or writes another entry's record, and a state change an arm makes is
 * never re-dispatched inside the same call -- so it is admissible C, unlike the dead arming loop.
 *
 * IT ARMS THE DOUBLING AND THE DOUBLING STILL LOSES.  .loop dump (tmp/grind/func_8007526C/s12/
 * v5_twopass.loop.txt, extracted from dumps/text1b.loop):
 *     Loop from 131 to 298: 59 real insns.        <- pass B, scanned FIRST (last-first, loop.c:435)
 *     Insn 136: regno 75 (life 106) ... moved         <- arms moved_once[75] (loop.c:1912)
 *     Insn 276/279: regno 131/132 (life 1) ... moved  <- pass B's two constants hoist anyway
 *     Loop from 14 to 125: 37 real insns.         <- pass A, scanned second
 *     Insn 19: regno 75 ... halved since already moved  moved   <- the doubling DOES fire
 *     Insn 103/106: regno 92/93 (life 1) ... moved     <- and pass A's constants hoist ANYWAY
 * Because splitting the loop halves its insn_count, the doubled count (37*2 = 74) is still far
 * below the decayed threshold (122 - 3 = 119), so loop.c:1631 keeps every constant desirable.
 * The arming route is therefore SELF-DEFEATING for any split of the main loop: the doubling only
 * beats the threshold while insn_count stays above ~60, and any split that supplies the second
 * loop drops it below that.  Arming needs a second loop of ADDITIONAL work, which this function's
 * semantics do not contain.
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
            break;
        }
        i++;
    } while (i < 2);
}
