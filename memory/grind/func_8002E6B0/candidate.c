/* s6 CANDIDATE (unchanged body, floor 26 / 96 insns, re-measured on HEAD this session).

   IMPORTANT INHERITANCE FIX (s6): the s5 header comment contained a literal
   close-comment sequence inside its prose, which terminated the block comment
   early and made this file FAIL TO COMPILE when installed verbatim into
   src/code6cac_b.c (the sandbox reported "func_8002E6B0 not found in ...o").
   The prose is now written without any nested comment delimiters. If you install
   candidate.c and the sandbox says the function is missing, look here first.

   THE BODY. Chassis: the s4 "ret-var + goto end" exit form (s32 ret = 0; both
   early exits `goto end;`, tail `ret = (cc ^ cp) >= 0; end: return ret;`) PLUS one
   borrow in block 2: block 2's `dz` is staged through the return carrier `ret`
   (`((ret = dz) * (center_x - arg0[0]))`), and `ret` is restored to its real value
   0 at the end of block 2 so the second early exit still returns 0.

   WHY IT WORKS (measured, not inferred): staging dz through the ret pseudo itself
   supplies the block-1/2 register pressure that buys the target's SIXTH
   callee-save while the ret-var chassis supplies the target's exit shape; s4 had
   measured those two halves as anti-correlated. The side-by-side reproduces the
   target's exit structure exactly (move v0,zero in the first bltz's delay slot,
   BOTH bltz straight to the epilogue, no j, no orphan zero block) AND sw s5,20(sp)
   / mflo s5.

   RESIDUAL (2 insns + a register permutation, unchanged in s6): our ret pseudo
   lands in $v1 rather than $v0, which costs a trailing `move v0,v1`, and the
   `ret = 0;` restore costs a `move v1,zero` in the second bltz's delay slot.

   s6 MECHANISM FINDING for the $v1-vs-$v0 seat (ground truth, .greg for this very
   body, tmp/grind/func_8002E6B0/s6/greg_seg.txt): the ret pseudo is reg 96. It is
   allocno ORDER 0 (pri 17368, allocated FIRST), so priority is NOT the obstacle.
   The obstacle is that reg 96 carries a PRE-ALLOCATION hard-register conflict with
   $v0 (";; 96 conflicts: ... 2 29 64 66"), and global.c:907-908 then strips reg
   96's $v0 copy preference because prune_preferences masks preferences by
   hard_reg_conflicts - which is why reg 96's dump line carries NO ";; 96
   preferences:" entry at all, while the two xor operands (reg 94 = 8 insns, reg 95
   = 7 insns) DO carry "preferences: 2 3 4" and reg 95 duly takes $v0. dump_conflicts
   is called at global.c:580, i.e. AFTER prune_preferences but BEFORE any allocation,
   so those conflicts are genuine inputs and not post-assignment fallout. Any lever
   that only moves refs / live length / birth order will not close this seat; the
   lever has to remove reg 96's hard conflict with $v0.

   FAMILY: variable reuse - an EXISTING local (the real return carrier) borrowed to
   stage a real, immediately-consumed value, then written back with its own real
   value (.claude/rules/defeat-licm-hoist-var-reuse.md, gated by
   .claude/rules/staged-value-reused-variable.md). It is a FAKE construct: any
   candidate-ready must carry the FAKE annotation (what + mechanism +
   lever-exhaustion) and quote that rule's scope sentence verbatim. The best
   FAKE-FREE form on this chassis remains v12 at 40
   (rejected/s4_v12_plain_no_varreuse_40.c).

   TIES AT 26 (byte-identical object): p1 = the staging moved to block 2's SECOND
   product; p2 = the staging carrying `center_x - arg0[0]` instead of dz; q3 = both
   of those at once; plus the s5 ties d2 / e6 / e1 / i1 / i2.
   TIES AT 26 WITH A DISTINCT OBJECT (fresh permuter basins, s6): q1 = the FINAL
   xor's operands swapped only; q2 = the FIRST if's xor operands swapped only.
   Both are in rejected/ as s6_final_xor_swap_* and s6_first_if_xor_swap_*. */
s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3)
{
  s32 center_x = ((arg0[0] + arg1[0]) + arg2[0]) / 3;
  s32 center_z = ((arg0[2] + arg1[2]) + arg2[2]) / 3;
  s32 cross_center;
  s32 cross_point;
  s32 ret = 0;
  {
    s32 dz = arg1[2] - arg0[2];
    s32 dx = arg1[0] - arg0[0];
    cross_center = (dz * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));
    cross_point = (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
  }
  if ((cross_center ^ cross_point) < 0)
  {
    goto end;
  }
  {
    s32 dz = arg2[2] - arg0[2];
    s32 dx = arg2[0] - arg0[0];
    cross_center = ((ret = dz) * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));
    cross_point = (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
    ret = 0;
  }
  if ((cross_center ^ cross_point) < 0)
  {
    goto end;
  }
  {
    s32 dz = arg2[2] - arg1[2];
    s32 dx = arg2[0] - arg1[0];
    cross_center = (dz * (center_x - arg1[0])) - (dx * (center_z - arg1[2]));
    cross_point = (dz * (arg3[0] - arg1[0])) - (dx * (arg3[2] - arg1[2]));
  }
  ret = (cross_center ^ cross_point) >= 0;
  end:
  return ret;

}
