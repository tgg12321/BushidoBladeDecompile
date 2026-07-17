/* REJECTED as a 0-candidate (it is the honest CLEAN floor): distance 6.
 * Fully clean form — no aliases, no stop holder, literal -2 both compares.
 * Residual = exactly the two ledger clusters:
 *   4 insns: prologue pair order (ours a0-pair first, target a1-pair first)
 *   2 insns: preheader order (ours move s1,s5 then li s7,-2; target reversed)
 * KILLED hypothesis: sched1 does NOT order a1's copy first on critical path —
 * both chains tie at priority 4 (move+lh(2)+beq vs move+lw(2)+addu) and
 * rank_for_schedule's LUID tie-break keeps expand_function_start's a0-first
 * emission. Measured s2 2026-07-17.
 */
s32 hirahira_w_frie(s32 *base, s16 *offsets) {
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
    } while (v1 != -2);
    ret = (s32)dest;

done:
    v1 = ret - (s32)base;
    *base = count;
    *slots = v1;
    return ret;
}
