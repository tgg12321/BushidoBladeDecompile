/* REJECTED (s2): splitting ANY single t-chain segment into a separate
 * single-death local `u` — tried on ALL SIX group-1 segments ([2],[3],[5],
 * [6],[7],[8]; s2 artifacts u2.c..u8.c) — positional diffs 33-35 vs 28 for
 * the unsplit form, and const NEVER flips to $v1.
 *
 * Why dead (measured, two coupled failures):
 * 1. sched1 (pre-RA, -fschedule-insns) un-pins the split segment's store:
 *    only a WAR dependence from t's NEXT redefinition pins a store in this
 *    function; a 1-def u has no redefinition, so its sw sinks toward the
 *    function end (u2: sw $3,8 sunk to insn 36; u3/probe5: sw $3,12 sunk).
 * 2. The deferred store EXTENDS u's live range into the return region, so
 *    local-alloc gives u $v1 (v0 conflicts with the hard return copy) and
 *    the const KEEPS $v0 — the exact opposite of the intended eviction;
 *    t is displaced to $a2.
 *
 * Also eliminated analytically (no compile needed): alternating two temps
 * t/u across segments — both become multi-death -> both global-alloc ->
 * both cannot share $v0, but target has ALL temps in $v0.
 */
/* representative variant (u on the [3] segment): */
s32 func_8006E49C(s32 arg0, s32 *arg1) {
    s32 t;
    s32 u;
    arg1[0] = arg0;
    arg0 += 0x9C40;
    t = arg0 + 0x5DC0;
    arg1[2] = t;
    u = arg0 + 0x61F8;
    arg1[1] = arg0;
    arg0 += 0x6838;
    arg1[3] = u;
    /* ... rest identical to candidate.c ... */
    return 0;
}
