/* REJECTED (s15b, 2026-09-07) -- DEAD CODE, not admissible, but the measurement matters.
 * A trailing real `while (i < 2) { lim = 0xC8; i++; }` placed after the main do-while
 * measures **score 1, build_insns 90** -- the target's 91 words minus the maspsx label-nop.
 * It is the first arming spelling that emits ZERO words: loop.c scans it first (loop numbers
 * run last-first), moves the `lim` movable there, sets moved_once[75] (loop.c:1912), and the
 * main loop's move_movables then doubles insn_count (loop.c:1609-1611) so all four switch
 * comparison constants print "not desirable" -- exactly the target's movable shape.  The
 * emitted cost is zero ONLY because the loop is provably dead: `i` is already 2 when control
 * reaches it, and the downstream jump/cse passes delete the whole region.
 * INADMISSIBLE: the loop has no observable effect (cheat checklist T1), no human would write
 * it (T2), and its entire rationale is loop.c's moved_once flag (T3).  It is the same intent
 * as the goto-loop already banned for this function.  Banked as the measured proof that the
 * arming axis's zero-word requirement and semantic deadness are the SAME property.
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
    while (i < 2) {
        lim = 0xC8;
        i++;
    }
}
