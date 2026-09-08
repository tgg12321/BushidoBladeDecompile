/* s10 ADDENDUM (2026-09-08, forensics). Body UNCHANGED; re-measured 26 / 96 on
   HEAD this session (the dispatch brief again said "measurement unavailable").

   THE RESIDUAL IS ONE REGISTER PERMUTATION. Side-by-side against the target
   (tmp/grind/func_8002E6B0/s10/tgt.n vs zz_reaudit_candidate.n): identical
   callee-save usage, including the never-used s4 slot the target also saves.
   Every differing line except the two known extra moves is a rename under one
   permutation - $v0 <-> $v1 throughout, plus an a0 -> a1 -> a2 three-cycle in
   the block-1 delta region.

   PASS ATTRIBUTION (s10). mips.h defines no REG_ALLOC_ORDER, so local-alloc's
   find_free_reg scans hard registers ASCENDING and $v0 (reg 2) is the first
   register tried for every unsuggested quantity - which is the whole reason
   twelve pseudos land "in 2." and only four "in 3.". `used` gains bit 2 only
   if hard reg 2 is genuinely live over the quantity's range, or via the
   just_try_suggested pass. GCC 2.7.2 gives a scalar return value the HARD
   register (a source `return 0;` is `(set (reg/i:SI 2) (const_int 0))`, no
   pseudo), but in the inline-return bodies those sets sit in the exit ARMS, so
   reg 2 is never live on the fall-through path and the seat does not move
   (measured 40 / 93 and 30 / 95). The target instead writes $v0 unconditionally
   mid-block-1 and never touches it across block 2. The open question is
   therefore exactly: which C shape makes the return register live on the
   FALL-THROUGH path from mid-block-1 through block 2.

   KILLED THIS SESSION (do not re-propose): the declaration-SCOPE axis (three
   distinct function-scope dz/dx pairs are BYTE-IDENTICAL to the per-block
   braces; only cross-block NAME reuse changes anything, at 55-60); alternative
   staging carriers (cross_point / cross_center score 45-48, i.e. exactly the
   no-borrow control, because the carrier is overwritten on the next statement
   and the staging store is dead before RA); and hoisting any block's dz/dx
   computation above the first `if` (58-70). */
/* s9 ADDENDUM (2026-09-08, enumerate). Body UNCHANGED; re-measured 26 / 96 on
   HEAD this session (the dispatch brief said "measurement unavailable" - the
   ledger's 26 is correct on the current -mel -msoft-float chassis).

   s9 swept 1,228 SPELLINGS of the three cross-product blocks with
   tools/spelling_enum.py + tools/sweep_variants.py and found NOTHING below 26
   (264 spellings tie it). Axes covered per block: which sub-expressions are
   named locals vs inlined (dz/dx/ax/az and dz/dx/px/pz), declaration order,
   assignment order, and commutative operand order on every product. Do not
   re-spell any of that - see the s9 section of evidence.md for the nine
   histograms.

   NEW STRUCTURAL FACT: block 2 (this block, the borrow block) has exactly ONE
   byte-relevant degree of freedom. All three block-2 sweeps split 50/50 between
   26 and 43 with no intermediate score, and the discriminator is a single bit:
   whether the borrowed pseudo is the FIRST operand of its multiply, as written
   below. Every other block-2 spelling choice is byte-neutral. Blocks 1 and 3 by
   contrast span 26..54, so they are spelling-sensitive but bounded below by 26.

   The FAKE-ablation control was re-measured too: the plain no-borrow form is 45
   (93 insns, one FEWER than the target's 94), so the borrow is worth +19 and is
   not masking any lever s7/s8 measured inert.

   WHAT IS LEFT (the enumerator structurally cannot reach it): declaration SCOPE
   (function-scope vs the per-block braces below), any form spanning an early-exit
   `if`, and the object model (four s32* args vs a struct/vector type). */
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
