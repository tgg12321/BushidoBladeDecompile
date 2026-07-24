/* REJECTED (s3, structural) — fully-dead 8-byte pad under the 2026-07-13
 * OVERSIZED-LOCALS carve-out. This is the ONE axis s2 did not evaluate
 * (s2 checked only the 2026-07-01 WRITTEN-never-read carve-out).
 *
 * Would close the distance-2 residual (the empty 8-byte zero-store leaf
 * frame) by declaring an unused pad so cc1's assign_stack_local reserves
 * vars=8 with all accesses DCE'd => bare `addiu sp,-8` / `addiu sp,8`,
 * zero frame stores = target's residual exactly (proven by s1 F2 / s2 g1).
 *
 * WHY REJECTED:
 *  - It is a fully-DEAD (never-referenced) local array = the forbidden
 *    `s32 buf[N];` form. find_unused_local_arrays flags it, mark_done refuses.
 *  - The OVERSIZED-LOCALS carve-out's frame-math prerequisite is satisfied
 *    only TRIVIALLY here (frame 8 - saves 0 - args 0 = 8 > written 0; holds
 *    for ANY zero-store phantom leaf frame), so it does NOT distinguish a
 *    genuine oversized-locals object (func_80037540: written prefix + live
 *    buffer, non-trivially forced slack) from plain frame coercion.
 *  - No written prefix, no live locals object => the weakest fully-dead-pad
 *    FALLBACK. No SOTN precedent for an UNWRITTEN local array acting as a
 *    phantom-frame carrier (the WRITTEN carve-out's SOTN evidence all WRITES
 *    the array; the OVERSIZED census covers written-prefix / annotated pads).
 *  - Sanctioning it requires wiring a prerequisite-aware engine-detector
 *    allowlist (engine/ = forbidden grind surface) + owner ruling.
 *  - Owner already ruled the identical zero-store phantom-frame species
 *    (AddTbpOfst_80047EE8, InitHiraRmd_80047FBC, +6) option (b) REFUSED /
 *    OWNER-ACCEPTED INCOMPLETE on 2026-07-22.
 *
 * => not grind-closable; escalated owner-gated (docs/grind/decisions.md 2026-07-24).
 */
void func_80017FA0(s32 *a0) {
    s32 pad[2];               /* FAKE fully-dead pad — FORBIDDEN, see above */
    volatile s32 *scr = (volatile s32 *)0x1F800000;
    s32 temp;
    s32 *ptr;

    temp = a0[3];
    if (temp == 0) {
        goto end;
    }
    ptr = (s32 *)temp;
    scr[0x2E] = ptr[0] << 7;
    /* ... inner double-loop identical to candidate.c ... */
    scr[0x18] = ((s32 *)a0[3])[1];
end:
    ;
}
