/* REJECTED s3 (2026-07-17, structural): type-narrowing axis, a1-chain side.
 * Entry-read local `v1` retyped `s16` on the clean+stop base.
 * Measured: sandbox --disable all = 21, build 66 vs target 59 (+7 insns) —
 * strictly worse. The narrowing forces sll/sra sign-extensions on v1's
 * arithmetic uses (the tail `v1 = ret - (s32)base;` truncates through s16),
 * so the a1-chain consumer CANNOT be narrowed byte-neutrally. Together with
 * param-type-widen-u32-base-floor4.c this kills the type-narrowing lever on
 * both entry chains. (offsets retyped u16* was NOT measured: lh->lhu is a
 * byte-level divergence predetermined by the target's signed load, not a
 * scheduling question.)
 */
s32 hirahira_w_frie(s32 *base, s16 *offsets) {
    s32 *slots = base + 1;
    s32 count = 0;
    s32 *dest = (s32 *)((s32)base + base[1]);
    s16 v1;
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
