/* func_80037B00 — BEST HONEST FORM as of s7 (2026-08-26).
   sandbox --disable all: score=9, target_insns=36, build_insns=36,
   cc1 `.frame $sp,8,$31 # vars= 8`.  Supersedes the s6 form (score 11,
   build_insns 35).  THE INSTRUCTION MULTISET NOW MATCHES TARGET EXACTLY:
   `goal_from_tgt.py classify` types the whole remaining residual as RA
   (`$t0 -> $t1 x4`, `$t1 -> $t0 x2`) plus one emission-position difference.

   WHY THIS SHAPE (two independent levers, both ordinary live C):

   1. TOP-TESTED `while` WITH NO EXPLICIT PRE-GUARD (inherited from s6).
      GCC 2.7.2 rotates the loop and materialises a duplicated entry guard in
      basic block 0; combine folds that guard's comparison into a bare `blez`,
      leaving the compare pseudo (87) referenced but homeless.  It reaches
      reload as an UNALLOCATED allocno, `alter_reg` hands it a stack slot,
      `get_frame_size()` counts 4 -> MIPS_STACK_ALIGN rounds to 8, and target's
      `addiu $sp,$sp,-8` / `addiu $sp,$sp,8` pair appears at ZERO instruction
      cost.  Producer #1 ("folded loop-guard compare") of
      .claude/rules/phantom-slot-frame-lever.md.  Re-adding an explicit
      `if (n <= 0) return 0;` guard DELETES the rotation guard (jump threading)
      and the frame collapses to vars=0 —
      rejected/explicit-guard-kills-rotation-orphan.c.

   2. THE LOOP BOUND IS READ BY THE LOOP TEST ITSELF — there is no `count`
      local at all (NEW in s7).  `while (var_t1 < D_800A38C8)` puts the `lw` in
      the loop's exit test; the rotation guard in block 0 keeps its own copy of
      that load, LICM hoists the in-loop load into the preheader, and cse
      unifies the two into target's `addu $t3,$v0,$zero` preheader COPY.  That
      copy is the instruction the s0..s6 forms were missing.  Writing the copy
      by hand in the source (`var_v0 = D_800A38C8; var_t3 = var_v0;`) does NOT
      work — copy propagation folds it (s6,
      rejected/split-count-copy-folded-by-copyprop.c).  It has to be created by
      a pass, and this is the spelling that makes loop.c + cse create it.

   The inner loop keeps the s0..s5 goto spelling verbatim: it is the spelling
   that reproduces target's block layout exactly (out-of-line `block_6c` fail
   arm, `bne` sense, no peeled first load).  A `do { } while` with `break`s
   makes GCC rotate and peel the inner loop too —
   rejected/for-loop-rotates-inner-loop-too.c.

   REMAINING RESIDUAL (36 ours vs 36 target — same multiset, 2 axes):

   (a) EMISSION POSITION of the `lui/addiu` pair for D_80102810: ours is at
       stream positions 1-2 (block 0, above the `lw`/`blez`), target's is at
       6-7 (inside the guarded preheader, after `addu $t3,$v0,$zero`).  Only
       loop.c places insns in a preheader, so the `la` has to be created INSIDE
       the loop as loop-invariant code / a giv initial value.  Both spellings of
       that measured +2 insns this session (score 19) — see
       rejected/giv-index-costs-offset-biv-add.c and
       rejected/giv-pure-coalesced-into-walking-ptr.c.

   (b) ONE register 2-swap: our outer counter takes $t0 and our inner end
       pointer takes $t1; target has counter=$t1, end=$t0.  The global.c
       priority model (validated AGAIN on this form, see evidence.md s7)
       gives pri(counter 73) = 3*8/22 = 1.0909 and pri(end 78) = 2*4/9 = 0.888,
       and the eight mutually-conflicting allocnos take $3,$5,$6,$7,$8,$9,$10,$11
       in rank order — so the swap is exactly "make pri(78) land in
       (1.0909, pri(75)=1.125)".  The arithmetic admits ONE reachable cell:
       refs 4 -> 5 at live_length 9 (pri 1.111).  See hypotheses.md for the
       full foreclosure table. */
s32 func_80037B00(u8 *arg0) {
    s32 var_t1;
    s32 var_t2;
    s8 *var_a3;
    s8 *var_a1;
    s8 *var_a2;
    s8 *var_t0;
    s32 var_v1;
    s32 var_v0;

    var_t1 = 0;
    var_a3 = (s8 *)&D_80102810;
    while (var_t1 < D_800A38C8) {
        var_t2 = 0;
        var_a1 = var_a3;
        var_a2 = (s8 *)arg0;
        var_t0 = var_a3 + 0x15;
    loop_inner:
        var_v1 = (u8)*var_a2;
        if (var_v1 == 0) {
            goto block_5c;
        }
        var_v0 = (u8)*var_a1;
        if (var_v1 != var_v0) {
            goto block_6c;
        }
        var_a1 += 1;
        var_a2 += 1;
        if ((s32)var_a1 < (s32)var_t0) {
            goto loop_inner;
        }
    block_5c:
        var_t1 += 1;
        if (var_t2 != 0) {
            goto block_74;
        }
        return 1;
    block_6c:
        var_t2 = 1;
        goto block_5c;
    block_74:
        var_a3 += 0x28;
    }
    return 0;
}
