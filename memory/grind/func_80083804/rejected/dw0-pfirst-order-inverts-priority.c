/*
 * REJECTED (session 4) — score 20, measured twice.
 *
 * The do-while(0) family reaches 16 because the wrap's loop-depth reference
 * weighting lifts p to 8 refs / live_length 7 (pri 34285) against count's
 * 8/8 (30000), so p wins $s0 outright. The remaining 2-point positional
 * residual at 16 is that the two address materialisations emit
 * count-pair-then-p-pair while the target emits p-pair-then-count-pair, so the
 * obvious next move is to assign p first.
 *
 * It fails for exactly the reason session 3's tie-based form failed: emission
 * order and register roles are coupled through live_length. Assigning p first
 * moves p's birth one statement earlier (live_length 7 -> 8, priority
 * 34285 -> 30000) and count's one statement later (live_length 8 -> 7, priority
 * 30000 -> 38571). count retakes $s0, the whole loop body reverts to the wrong
 * registers, and the score goes 16 -> 20.
 *
 * Measured in both declaration orders — sweep5.log d1_dw0_pfirst (count
 * declared first) and d3_dw0_pdecl_pfirst (p declared first) — identical
 * allocno tables, identical score 20. Declaration order is inert here because
 * there is no tie for it to break.
 *
 * CONSEQUENCE FOR THE FRONTIER: the coupling survives the change of mechanism
 * (tie-break in s3, strict priority in s4). Any form that fixes the emission
 * order must pay for p's extra live-length point with roughly 3 extra weighted
 * references (p at 11 refs / len 8 = 41250 would be needed to beat count's
 * 38571), or must lengthen count's live range without adding count references
 * (frontier F4b).
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        do { p = &D_8008D070; count = (s32)&D_00000000; } while (0);
        if (count != 0) {
            do {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            } while (count != 0);
        }
    }
}
