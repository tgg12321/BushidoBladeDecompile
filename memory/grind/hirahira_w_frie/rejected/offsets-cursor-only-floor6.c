/* REJECTED: distance 6 (no improvement over clean baseline). Measured s2 2026-07-17.
 * Probe: semantically-true walking cursor for offsets ONLY
 * (`s16 *cursor = offsets;` as first decl, offsets never used again),
 * base kept raw.
 * KILLED: a single OFFSETS-side rename cannot flip the prologue pair.
 * Mechanism: base's entry copy (move pseudo<-a0) keeps LUID 1; the cursor's
 * combine-merged move lands later; sched1 priority tie (4=4) falls to LUID ->
 * a0 pair still first. The flip requires relocating the A0/base copy, i.e.
 * a base-side rename (measured: base-only rename scores 2 — see evidence.md;
 * that spelling is tombstone-family and ruling-pending, NOT committable).
 */
s32 hirahira_w_frie(s32 *base, s16 *offsets) {
    s16 *cursor = offsets;
    s32 *slots = base + 1;
    s32 count = 0;
    s32 *dest = (s32 *)((s32)base + base[1]);
    /* ... body identical to clean form but reading via cursor ... */
}
