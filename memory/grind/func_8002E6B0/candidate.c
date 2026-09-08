/* s8 ADDENDUM (2026-09-08, structural). Body UNCHANGED; re-measured 26 / 96 on
   HEAD this session. Two corrections to the s7 header below:
     - reg 96 has SIX sets, not four: the tail (cc ^ cp) >= 0 expands to
       xor / not / lshiftrt all written INTO reg 96, exactly as the target does.
     - the 96/$v0 conflict is contributed by only THREE local_alloc quantities,
       not twelve: pseudos 117 (block-1 branch condition), 121 (a block-2
       product) and 138 (block-2 branch condition). Reg 96 is dead across
       block 3, so the seven block-3 $v0 quantities cannot reach the seat.
   s8 also confirmed this basin absorbs seven more structural spellings
   byte-identically (declaration order in any permutation, ret = 0 statement
   placement, restore spelled as a subtract, tail split through ret) - see
   the s8 section of hypotheses.md. Do not re-spell any of them. */
/* s7 CANDIDATE (body UNCHANGED from s5/s6; floor 26 / 96 insns, re-measured on
   HEAD this session; target is 94 insns).

   INHERITANCE WARNING kept from s6: do not write a literal close-comment
   sequence inside this header. The s5 header did, which terminated the block
   comment early and made this file fail to compile when installed verbatim into
   src/code6cac_b.c; the sandbox then reported "func_8002E6B0 not found in
   tmp/sandbox/func_8002E6B0/code6cac_b.o" instead of a score. If you install
   candidate.c and see that error, look here first.

   THE BODY. Chassis: the s4 ret-var + goto-end exit form (s32 ret = 0; both
   early exits goto end; tail ret = (cc ^ cp) >= 0; end: return ret;) PLUS one
   borrow in block 2: block 2's dz is staged through the return carrier ret
   ((ret = dz) * (center_x - arg0[0])), and ret is restored to its real value 0
   at the end of block 2 so the second early exit still returns 0.

   WHY IT WORKS (measured, not inferred): staging dz through the ret pseudo
   supplies the block-1/2 register pressure that buys the target's SIXTH
   callee-save while the ret-var chassis supplies the target's exit shape; s4 had
   measured those two halves as anti-correlated. The side-by-side reproduces the
   target's exit structure exactly and its sw s5,20(sp) / mflo s5.

   RESIDUAL: exactly two insns, both moves, plus the register permutation that
   follows from them. Our ret pseudo lands in $v1 rather than $v0, which costs a
   trailing move v0,v1; and the ret = 0 restore costs a move v1,zero in the
   second bltz's delay slot (the target fills that slot with real block-3 work,
   subu a0,t5,t3, because it needs no restore).

   s7 MECHANISM - THE SEAT IS NOW FULLY TYPED, AND IT IS A LOCAL-ALLOC PROBLEM.
   Ground truth from the instrumented cc1 (BB2_FINDREG_DEBUG=96 on
   tools/gcc-2.7.2/cc1; capture in tmp/grind/func_8002E6B0/s7/findregdbg.txt) for
   this exact body: pseudo 96 is ret, allocno ORDER 0, conflicts = {2, 29},
   someone_prefers = {4}, used_so_far contains 2, pass0_used =
   {0,1,2,4,16..23,26..31} so the free set is {3,5,6,7,8..15,24,25} and the
   ascending scan takes 3 = $v1; pass1_used = {0,1,2,26,27,28,29,31}. Reg 2 is
   excluded in BOTH passes solely by the conflict, so removing that single graph
   edge is necessary AND sufficient to seat ret in $v0.
   Where the edge comes from: NOT the block-entry path (the .lreg per-block
   live-at-start sets contain no reg 2 at all - see
   tmp/grind/func_8002E6B0/s7/lreg_blocks.txt, which falsifies the s6 frontier),
   and NOT the epilogue copy (mark_reg_clobber returns early unless its setter is
   a CLOBBER, global.c:1528-1529, and the REG_DEAD note is processed before
   mark_reg_store). It comes from local_alloc: the .lreg dump's
   ";; Register N in H." list (tmp/grind/func_8002E6B0/s7/local_alloc_map.txt)
   shows local_alloc put TWELVE pseudos in $v0 and only FOUR in $v1, and
   record_one_conflict IORs hard_regs_live into hard_reg_conflicts at
   global.c:1392 on every birth of reg 96 - of which this body has four. The
   target mirrors us: both its branch conditions go to $v1 and ret sits in $v0.
   So no global-alloc priority, live-length or preference lever can reach this
   seat; the lever must change local_alloc's choice.

   FAMILY: variable reuse - an EXISTING local (the real return carrier) borrowed
   to stage a real, immediately-consumed value, then written back with its own
   real value (.claude/rules/defeat-licm-hoist-var-reuse.md, gated by
   .claude/rules/staged-value-reused-variable.md). It is a FAKE construct: any
   candidate-ready must carry the FAKE annotation (what, mechanism,
   lever-exhaustion) and quote that rule's scope sentence verbatim. The best
   FAKE-FREE form on THIS chassis is 45 (rejected/s7_plain_noborrow_on_26_chassis_45.c);
   the older FAKE-free 40 belongs to the different s4 v12 chassis.

   TIES AT 26, BYTE-IDENTICAL OBJECT (md5 921b8948): s5's d2 / e6 / e1 / i1 / i2,
   s6's p1 / p2 / q3, and s7's r3 (restore moved up inside block 2).
   TIES AT 26 WITH A DISTINCT OBJECT (unsearched permuter basins): s6's q1
   (078e7ad4, final xor operands swapped) and q2 (1b1fa1c0, first if's xor
   operands swapped), and s7's r2 (3bae0255, branch condition carried in
   cross_point). All three are in rejected/. */
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
