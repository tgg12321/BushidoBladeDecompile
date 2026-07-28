/* s3 (2026-07-28): post-greg noop-move route (jump2 runs with noop_moves=1 +
   cross_jump=1 AFTER reload, toplev.c:3142) — every C spelling measured dead.
   Representative below: split loop bound through fresh `tmp` (guard on tmp,
   loop latch on a2; cross-bb copy survives cse [loop unreachable in the ebb]
   and combine [no cross-bb LOG_LINKS]). Measured: blez $2, n=22 — the fresh
   bb0-resident pseudo ALWAYS steals $2 because v0 is not live in bb0 (no
   conflict), and any short-lived bb0 pseudo has priority ~7500 > a2's ~3333
   so it allocates first and takes the first free reg = $2. Mirror variant
   (guard on a2, loop on tmp) fails identically. Conflict-free same-bb copies
   get combine-merged instead (I1 dest dies in copy -> merged, no len); both-
   live copies allocate different regs -> REAL move -> byte drift. There is
   no C-reachable noop-move in this function. */
s32 func_80037AA4(void) {
    s8 *var_v1;
    s32 var_a1;
    s32 var_a2;
    s32 var_a0;
    s32 var_v0;
    s32 tmp;

    var_a1 = 0;
    var_a0 = 0;
    tmp = D_800A38C8;
    if (var_a1 < tmp) {
        var_a2 = tmp;
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
