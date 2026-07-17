/* hirahira_w_frie — s4 fresh permuter chassis: inverted entry, no goto/label,
 * single shared tail. Sandbox --disable all = 6 (59/59), measured 2026-07-17.
 * Residual = the invariant 4-insn prologue pair + 2 insns for the tail
 * `subu v1,s2,s4` vs target `subu v1,v0,s4` (GCC copy-propagates dest into
 * the ret-base compute, so v1 is computed from s2 instead of v0).
 * The subu gap closes with ret-staging (see inverted-entry-ret-staged-floor4.c);
 * the prologue pair does not close in any sanctioned spelling.
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
    if (v1 != -2) {
        s32 stop = -2;
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
    v1 = ret - (s32)base;
    *base = count;
    *slots = v1;
    return ret;
}
