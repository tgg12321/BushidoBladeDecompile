/* hirahira_w_frie — s4 permuter find (ws_inv output-38-1, adapted): a SECOND
 * independent sanctioned floor-4 chassis. Sandbox --disable all = 4 (59/59),
 * measured 2026-07-17; objdump residual verified as EXACTLY the same
 * prologue pair (sw s4/move s4,a0 early vs late) as the goto-form clean+stop.
 *
 * Novelty: no goto/label, no `stop`-independent preheader issue — the entry
 * comparison is staged through the existing (then-dead) `ret` local
 * (staged-value-reused-variable family, sanctioned 2026-07-03; would need
 * a FAKE annotation if ever committed). The staging closes the 2-insn
 * subu-operand gap of inverted-entry-nogoto-floor6.c by keeping the tail's
 * ret-base compute on the v0-bound pseudo.
 *
 * FILED AS REJECTED because it does NOT beat the sanctioned floor 4 — it
 * corroborates the floor's chassis-invariance stochastically (permuter
 * descended to weighted 38 = this exact form and never below across the
 * s4 campaigns). The prologue pair remains closed ONLY by the owner-gated
 * tombstone construct (candidate.c).
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
    ret = v1 != -2;
    if (ret) {
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
