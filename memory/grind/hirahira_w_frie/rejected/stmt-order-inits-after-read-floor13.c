/* REJECTED: distance 13 (WORSE than clean baseline 6). Measured s2 2026-07-17.
 * Geometry probe: made v1 = *offsets the first executable statement and pushed
 * slots/count/dest inits after it as plain assignments.
 * KILLED: statement geometry cannot move the param entry copies (emitted by
 * expand_function_start before any statement, a0 first, always), and moving
 * the s5/s6 inits out of decl-initializer position DISTURBS their scheduling —
 * addiu s5 fell into the body and move s6,zero into the beq delay slot,
 * instead of interleaving into the prologue region like target. Decl-position
 * initializers for slots/count/dest are load-bearing for the matched shape.
 */
s32 hirahira_w_frie(s32 *base, s16 *offsets) {
    s32 *slots;
    s32 count;
    s32 *dest;
    s32 v1;
    s32 *walker;
    s32 size;
    s32 cur_off;
    s32 ret;

    v1 = *offsets;
    offsets++;
    slots = base + 1;
    count = 0;
    dest = (s32 *)((s32)base + base[1]);
    if (v1 == -2) {
        ret = (s32)dest;
        goto done;
    }
    /* ... loop body identical to clean form ... */
}
