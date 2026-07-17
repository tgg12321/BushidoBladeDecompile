/* REJECTED s3 (2026-07-17, structural): type-narrowing axis, param side.
 * `base` retyped `u32 *` on the clean+stop (sanctioned floor-4) base.
 * Measured: sandbox --disable all = 4 (59/59) — IDENTICAL to the invariant
 * floor. Param TYPE is inert for the prologue pair, as the s2 credited
 * mechanism predicts: expand_function_start emits both entry copies before
 * any statement RTL in param order regardless of type; the a0 chain
 * (move/lw/addu) is type-invariant at 32-bit width, so the priority tie
 * (4 = 4) and the LUID tie-break are untouched.
 * This closes the "type narrowing" structural lever on the param side.
 */
s32 hirahira_w_frie(u32 *base, s16 *offsets) {
    s32 *slots = (s32 *)(base + 1);
    s32 count = 0;
    s32 *dest = (s32 *)((s32)base + base[1]);
    s32 v1;
    s32 *walker;
    s32 size;
    s32 cur_off;
    s32 ret;

    v1 = *offsets;
    offsets++;
    if (v1 == -2) {
        ret = (s32)dest;
        goto done;
    }

    {
    s32 stop = -2; /* FAKE: constant-holder, named-local-fake-exception */
    walker = slots;
    do {
        if (v1 >= 0) {
            size = walker[1];
            cur_off = walker[0];
            *slots = (s32)dest - (s32)base;
            slots++;
            count++;
            size = size - cur_off;
            func_800520B8((s32)base + cur_off, (s32)dest, size);
            size = (u32)size >> 2;
            size = size << 2;
            dest = (s32 *)((s32)dest + size);
        }
        walker++;
        v1 = *offsets;
        offsets++;
    } while (v1 != stop);
    }
    ret = (s32)dest;

done:
    v1 = ret - (s32)base;
    *base = count;
    *slots = v1;
    return ret;
}
