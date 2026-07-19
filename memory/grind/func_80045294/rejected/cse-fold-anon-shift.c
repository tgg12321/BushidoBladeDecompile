/* Rejected but NEAR-HIT: inline the a0<<4 into s4's init and declare v1
 * AFTER as a CSE-reuse target. Score 2 (weighted), build_insns 84 (target 83).
 *
 * OBJDUMP AT PROLOGUE (with this form):
 *   move  s0,s2       <-- move16 first (matches target order!)
 *   sll   v0,s2,0x4   <-- sll into ANONYMOUS temp v0
 *   move  v1,v0       <-- EXTRA copy propagating v0 -> v1
 *
 * MECHANISM: the anonymous a0<<4 subexpression in s4's init gets a fresh
 * pseudo (allocated to $2/v0). Later `s32 v1 = a0<<4` CSE-reuses the pseudo
 * but v1 is a distinct pseudo, so RA emits a move copy. The extra insn
 * disqualifies the form.
 *
 * WHAT IT PROVES: the sched2 order the target wants IS reachable by giving
 * the sll's source pseudo a LATER assignment position than move16's — the
 * inline subexpression pushes the sll's LUID up. Combined with s3's sched2
 * dump (priorities equal, tie broken by LUID), this confirms the closing
 * lever is somewhere in the LUID domain, and the single blocker is
 * eliminating that CSE-move copy without pushing sll's LUID back down.
 *
 * NEXT LEVER: express a0<<4 exactly ONCE and have that expression target
 * v1 directly (not an anonymous pseudo). Candidates: comma-expression
 * assignment `s4 = (v1 = a0<<4, tbl[v1])`, or move v1 into a per-loop scope
 * so it's the SOLE pseudo for a0<<4.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + (a0 << 4));
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    s32 v1 = a0 << 4;
    /* ... body unchanged ... */
}
