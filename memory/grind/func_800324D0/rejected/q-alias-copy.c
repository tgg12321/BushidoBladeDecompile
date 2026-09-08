/* REJECTED (s1, 2026-08-20): plain pointer-alias copy in the >=0x80 arm.
 * Measured: sandbox 27 (flat, no change vs baseline). The .greg allocno census
 * was unchanged (same 7 allocnos) — cse1's canon_reg coalesces a bare reg-reg
 * copy (`q = v1`) before local-alloc ever sees it, so no preference is planted.
 * KILL generalizes: any once-written pointer alias whose only definition is a
 * direct copy of a live variable is deleted pre-RA and cannot influence
 * allocation. A surviving intermediate needs a non-copy definition (load or
 * arithmetic) AND >= 2 real uses (combine deletes single-use defs in-block).
 */
void func_800324D0(u8 *a0) {
    u8 *v1;
    u8 v0;
    u32 a2;
    u8 a1;

    v1 = *(u8 **)(a0 + 0x58);
    v0 = 0xFF;
    /* ... 11 sb stores as baseline ... */
    v0 = v1[4];
    v1 += 5;
    if (v0 == 0) return;
    do {
        a2 = v0;
        if (a2 == 0xFF) {
            v1 += 6;
        } else if (a2 < 0x80) {
            v1++;
        } else {
            u8 *q;          /* <-- the probe */
            a2 -= 0x80;
            q = v1;         /* bare copy: cse-coalesced, dead before RA */
            a1 = *q;
            v1 = q + 1;
            /* switch as baseline */
        }
        v0 = *v1;
        v1++;
    } while (v0 != 0);
}
