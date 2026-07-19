/* Novel 1 (s8): i-before-v1 with ashift operand written as `i` instead of `a0`.
   Hypothesis: cse.c substitution direction might prefer keeping operand 77 (i)
   and NOT rewrite it to 72 (a0), which would keep a0's live range short and
   still emit sll on $s0 — different bytes but might match target somehow.
   OR: cse would substitute 77->72, keeping ashift on 72; then i=a0 preserves
   long a0 live range while sll's LUID stays after move16's LUID.
   Result: score=11 build_insns=83 — identical rotation shape to s3 decl-init-decouple.
   cse.c evidently does NOT substitute 77->72 back; it accepts the (ashift 77)
   as-is (since i is fresh) and RA still rotates a0 to $21 because a0's live
   range collapses at insn 15 (i=a0), same as s7 measurement.
   Verdict: KILLED. Same pass-level RA rotation as decl-init-decouple/i-before-v1.
*/
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1 = i << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* ... rest identical to candidate ... */
}
