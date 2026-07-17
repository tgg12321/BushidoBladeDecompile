/* REJECTED s2b (2026-07-17, structural): K&R old-style definition with the
 * param DECLARATION block in reversed order (offsets declared before base).
 * Hypothesis: store_parm_decls might chain DECL_ARGUMENTS in decl-block order,
 * flipping expand_function_start's entry-copy emission order without a rename.
 * MEASURED: sandbox --disable all = 4 (identical to clean+stop baseline).
 * KILLED: c-decl.c store_parm_decls orders DECL_ARGUMENTS by the IDENTIFIER
 * LIST (base, offsets), which is ABI-fixed; the decl block order is inert.
 */
s32 hirahira_w_frie(base, offsets)
s16 *offsets;
s32 *base;
{
    s32 *slots = base + 1;
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
