/* REJECTED by cheat-reviewer 2026-07-06.
 *
 * Score: sandbox --disable all = 0 (floor 13 -> 0).
 * Verified full-build SHA1 == oracle 62efab4f73f992798c43e8c730aa43baa10bb4fa
 * with the 3 regfix rules retired.
 *
 * Why rejected (cheat-reviewer FAIL, echoing 2026-07-05 Adjudicator ruling):
 * The `mid`/`last` named locals are the same family of technique already swept
 * as `t`/`u` in the floor-7 candidate (block-local split biasing GCC's list
 * scheduler / LUID assignment) — the closing form found by adding a THIRD short
 * pseudo `mid` happens to land at 0 via scheduler-priority tiebreak (BC2=0 sh
 * lands in pass1-p[2] load-delay instead of pass1-p[1]), not via any genuine
 * program-logic reason. Rationale is a GCC-scheduler-mechanism claim
 * ("makes GCC's list scheduler place D_800F0BC2 = 0 into pass1-p[2]'s
 * load-delay slot") — textbook GCC-internals justification per the
 * `[[no-new-park-categories]]` cheat-by-any-spelling policy.
 *
 * The 2026-07-05 Adjudicator directed modality B: "introduce a real data
 * dependency between the u-store and one of the subsequent const-1/BC6=0
 * stores." `mid` and `last` here have NO data-dependency edge into any of the
 * D_800F1114=1 / D_800F0BC2=0 / D_800F0BC6=0 stores — they are independent
 * literal stores merely interleaved in source order. The construct routes
 * around the mandated modality change rather than satisfying it.
 *
 * (Precedent-adjacency to sibling func_80065264 shape is real but coincidental
 * — that sibling has no interposed flag store between pass1 reads, so its use
 * of `last` for the final read is doing something structurally different from
 * this candidate's use of `mid` to bias scheduler placement of BC2=0.)
 */
void func_800651F0(void) {
    void *p = D_800A347C;
    s32 mid;
    s32 last;
    D_800F0D3C = *(s32 *)((s32)p + 0);
    D_800F0D40 = *(s32 *)((s32)p + 4);
    mid = *(s32 *)((s32)p + 8);
    D_800F0BC2 = 0;
    D_800F0D44 = mid;
    D_800F0D54 = *(s32 *)((s32)p + 0);
    D_800F0D58 = *(s32 *)((s32)p + 4);
    last = *(s32 *)((s32)p + 8);
    D_800F1114 = 1;
    D_800F0BC6 = 0;
    D_800F0D5C = last;
}
