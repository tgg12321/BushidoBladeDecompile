/* REJECTED FORM — cheat-reviewer FAIL 2026-06-10 (session af84f434ed62cc877).
 *
 * This form reaches sandbox 0 / full-register objdump 0/121 vs target.
 * Mechanism: the two adjacent `p += 3;` statements give biv p TWO increments
 * (biv_count=2), so loop.c's giv decision `benefit -= add_cost * biv_count`
 * = 2+2 - 2*2 = 0 -> "giv not worth while" -> the two DEST_ADDR givs
 * (p+6, p+10) are NOT combined/reduced -> halfword offsets stay inline
 * (target bytes). The combine RTL pass then merges the adjacent adds back
 * to one `addiu p,p,12`. With a single `p += 6` (biv_count=1) the combined
 * giv benefit is 2 -> reduced -> IV anchored at base+10 (`addiu a0,s1,10`,
 * loads at -4/0) which is what the 3 regfix substs at regfix.txt:691-694
 * were papering over.
 *
 * Reviewer verdict: FAIL on tests 1 (semantic purpose), 2 (human
 * programmer), 3 (GCC-internals justification) — the split's only effect
 * is inflating biv_count; emitted bytes identical to `p += 6`.
 * Re-review with proven-spelling-class evidence: see meta.json.
 * Do NOT re-derive/commit this without explicit user sanction.
 */
void func_80022224(s32 arg0, s32 *arg1, s32 *arg2) {
    s32 dists[6];
    s16 *base;
    s16 *p;
    s32 *d;
    s32 dx;
    s32 dz;
    s32 i;
    s32 best;
    s32 *r;
    s32 *w;

    base = (s16 *)(stage_GetDataPtr() + (D_800A36A4 * 3) * 0x10);
    i = 0;
    p = base;
    d = dists;
    do {
        dx = p[3] - arg2[0];
        dz = p[5] - arg2[2];
        *d = dx * dx + dz * dz;
        i++;
        d++;
        p += 3;
        p += 3;
    } while (i < 4);

    best = 0;
    for (i = 1; i < 4; i++) {
        if (dists[i] < dists[best]) {
            best = i;
        }
    }
    dists[best] = -1;

    best = 0;
    for (i = 1; i < 4; i++) {
        if (dists[i] > dists[best]) {
            best = i;
        }
    }
    dists[best] = -1;

    r = dists;
    i = 0;
    {
        s32 stop = -1;
        w = r;
        for (; i < 4; i++) {
            if (*r != stop) {
                w[4] = i;
                w++;
            }
            r++;
        }
    }

    base += dists[4 + (func_80079154() & 1)] * 6 + 3;
    arg1[0] = base[0];
    arg1[1] = base[1];
    arg1[2] = base[2];
}
