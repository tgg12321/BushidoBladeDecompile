/* func_8006CCC8 — REJECTED (measured worse), s4.
 * Combining `field < 4` and `field >= 0` into a single `&&` expression instead
 * of nested `if (field < 4) { if (field >= 0 && ...) }` regressed the sandbox
 * --disable all floor from 39 to 65 on the s4 chassis (immediately before/after
 * this exact swap, all other source unchanged). Kept the nested-if form in
 * candidate.c. Not a cheat question — pure ordinary-C codegen difference, just
 * the wrong choice; recorded so a future session doesn't re-try it blind.
 *
 * The regressed fragment (drop-in replacement for the equivalent block in
 * candidate.c):
 */
        field = *(s16 *)(mask + (u8 *)D_800A34FC + 0x28);
        if (field != 4) {
            if (field != 3) {
                if (field < 4 && field >= 0 && (*arg1 & (0xF0 << shift))) {
                    func_8005C650(0, 0x7F, 0x7F);
                    t = i;
                    func_8006CBD4(t, *arg1);
                }
            } else {
                /* ...case3 body unchanged... */
            }
        } else {
            /* ...case4 body unchanged... */
        }
