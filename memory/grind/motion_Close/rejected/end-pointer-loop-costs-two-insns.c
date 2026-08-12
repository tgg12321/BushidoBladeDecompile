/* REJECTED (session 2) — the honest end-pointer spelling. Flips the roles the
 * right way (p takes $s0) but costs two instructions the target does not have.
 *
 * Measured: sandbox --disable all score 21, build_insns 27 (target 26; the
 * session-2 floor is 17). ALLOCDBG: p -> n_refs=11 live_length=9 pri=36666
 * (takes $s0), end -> n_refs=4 pri=13333.
 *
 * This is the one *natural* C spelling (a human would write it) that gives p
 * the register the target uses — which is why it is worth recording rather
 * than merely deleting. It loses because materializing `end = p + count`
 * requires the count symbol AND an add that the target never performs: the
 * target counts down in $s1 and never forms an end pointer. So the form that
 * fixes the registers necessarily breaks the instruction sequence, and the
 * form that keeps the instruction sequence necessarily loses the registers.
 * Together with H1 (frame) this is the second independent reason no pure-C
 * body can match motion_Close.
 */

void motion_Close(void) {
    void (**p)(void);
    void (**end)(void);

    if (D_800A2668 != 0) {
        p = &D_8008D070;
        end = p + (s32)&D_00000000;
        if (p != end) {
            do {
                void (*f)(void) = *p;
                p++;
                f();
            } while (p != end);
        }
    }
}
