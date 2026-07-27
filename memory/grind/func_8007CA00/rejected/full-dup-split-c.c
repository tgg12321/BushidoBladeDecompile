/* REJECTED (s4, measured) — full tail duplication per branch (no shared
 * label at all), split-c form in each branch, betting on natural cross-jump
 * to merge the 2-subu suffix while each branch keeps its own li.
 * MEASURED DEAD: 46 insns (target 44), 40 diff lines. The duplicated tails
 * push arg0 out of $a0 (`move a1,a0` appears in the dispatch delay slot) and
 * the whole register file cascades (div idiom lands in a0/v1, loads read
 * from a1). Both branches' li DO land in target-like delay-slot positions,
 * but the RA cost dwarfs the gain. Do NOT re-propose full duplication. */
extern u8 D_8009BE74;
extern u8 D_8009BE77;
s32 func_8007CA00(s16 *arg0) {
    s32 v1, a, t;
    switch (D_8009BE74) {
    case 1:
        if (D_8009BE77 != 0) {
            s32 c = 0x400;
            v1 = arg0[2];
            a = arg0[0];
            c = c - v1;
            return c - a;
        }
        t = arg0[0];
        goto ret;
    case 2:
        if (0 != D_8009BE77) {
            s32 c = 0x400;
            v1 = ((s16)(*((u16 *)(arg0 + 2)))) / 2;
            a = arg0[0];
            c = c - v1;
            return c - a;
        }
        t = ((s32)((s16)(*((u16 *)arg0)))) / 2;
        goto ret;
    default:
        t = arg0[0];
    ret:
        return t;
    }
}
