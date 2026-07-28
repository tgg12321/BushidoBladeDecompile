/* REJECTED s4 (2026-07-28): permuter zero-find on the g0 floor-4 seed
   (ws_g0/output-0-1, 565 iters) — `volatile unsigned int pad;` unused
   volatile local forcing the 8-byte frame. Textbook dead-volatile-local
   frame coercion (dead-vars-local-array / volatile-coercion family,
   FORBIDDEN — frame-byte-reservation mechanism, explicitly outside the
   2026-07-01 scalar carve-out). Identified and rejected without
   application per no-new-park-categories permuter-vetting discipline.
   Recorded as closing-form-space evidence: the permuter can close g0
   with a frame cheat; the only non-cheat-catalog zero found is the
   g1 + constant-holder form (see candidate.c). */
s32 func_80037AA4(void) {
    volatile unsigned int pad; /* THE CHEAT — do not re-propose */
    s8 *var_v1;
    s32 var_a1;
    s32 var_a2;
    s32 var_a0;
    s32 var_v0;

    var_a1 = 0;
    var_a0 = 0;
    var_a2 = D_800A38C8;
    if (var_a2 > 0) {
        var_v1 = (s8 *)&D_80102810;
        do {
            var_v0 = *(s32 *)(var_v1 + 0x18);
            var_a1 += 1;
            var_a0 += var_v0;
            var_v1 += 0x28;
        } while (var_a1 < var_a2);
    }
    var_v0 = var_a0;
    if (var_a0 < 0) {
        var_v0 = var_a0 + 0x1FFF;
    }
    var_a0 = var_v0 >> 0xD;
    return 0xF - var_a0;
}
