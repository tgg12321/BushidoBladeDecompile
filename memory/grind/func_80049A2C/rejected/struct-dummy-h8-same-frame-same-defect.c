/* H8 (s4) KILLED — struct-typed aggregate `struct { s32 a; s32 b; } dummy;`
 * measured 2026-07-20:
 *   sandbox --disable all = 0, build_insns=126, cheat_asm_stripped=396.
 *
 * Confirms prong (a): GCC 2.7.2 does NOT scalarize the two-field struct;
 * it reserves the same +8 locals frame slot as `s32 dummy[2]` (aggregate
 * decl → mips.c compute_frame_size reserves vars=8 unconditionally).
 *
 * Fails prong (b) reviewer-disposition: struct is unwritten + unread with
 * `(void) dummy;` sink; identical "no semantic purpose" defect as
 * `s32 dummy[2]` under [[inline-asm-policy]] expanded cheat catalog +
 * [[no-new-park-categories]] cheats-by-any-spelling posture. The
 * 2026-07-01 [[dead-vars-local-array]] carve-out is written-never-read
 * arrays with dead stores oracle-present; a fully-dead struct is not
 * within that carve-out and no SOTN precedent found for a fully-dead
 * 8-byte struct pad.
 *
 * Therefore H8 does NOT constitute a distinct sanctioned closing form.
 * Aggregate-only conclusion (s3) reinforced: the +8 frame slot is
 * reachable ONLY via an aggregate declaration, and every aggregate
 * spelling shares the same reviewer-visible defect.
 */
void func_80049A2C(s32 arg0, s32 arg1, s32 arg2) {
    /* ...body as candidate.c, but with: */
    struct { s32 a; s32 b; } dummy;
    /* ... */
    (void) dummy;
}
