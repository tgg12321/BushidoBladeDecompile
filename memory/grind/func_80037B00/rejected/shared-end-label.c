/* REJECTED s2 2026-07-28 — sandbox score 19 (baseline 15), +4 weighted.
 * Shared-end-label restructure: added s32 ret local, both exit paths set ret and
 * goto end; end: return ret;
 * Effect: build_insns=34 same, but weighted diff grew 4. Adding ret as another
 * live scalar shifted the RA scavenger to a WORSE rename cycle for the 5-way
 * rotation. Shared-end-label is contraindicated for this function.
 * KILLED as a shape lever. */
s32 func_80037B00(u8 *arg0) {
    s32 var_t1, var_t3, var_t2, var_v1, var_v0, ret;
    s8 *var_a3, *var_a1, *var_a2, *var_t0;

    var_t1 = 0;
    var_v0 = D_800A38C8;
    ret = 0;
    if (var_v0 <= 0) goto end;
    /* ... loop body ... */
    /* block_5c: var_t1 += 1; if (var_t2 != 0) goto block_74; ret = 1; goto end; */
    /* ... */
end:
    return ret;
}
