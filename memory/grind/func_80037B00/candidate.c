/* func_80037B00 — BEST HONEST FORM as of s6 (2026-08-26).
   sandbox --disable all: score=11, target_insns=36, build_insns=35,
   cc1 `.frame $sp,8,$31 # vars= 8` (TARGET FRAME REPRODUCED HONESTLY).

   Supersedes the s0..s5 goto-only form (score 15, build_insns 34, vars=0).

   WHY THIS SHAPE. The outer loop is written as a TOP-TESTED `while` with NO
   explicit pre-guard. GCC 2.7.2 rotates it and materialises a duplicated
   entry guard in basic block 0; combine folds that guard's comparison into a
   bare `blez`, leaving the compare pseudo referenced but with no home. It
   reaches reload as an UNALLOCATED allocno, `alter_reg` hands it a stack slot,
   `get_frame_size()` counts 4 -> MIPS_STACK_ALIGN rounds to 8, and the
   `addiu $sp,$sp,-8` / `addiu $sp,$sp,8` pair target carries appears at ZERO
   instruction cost. This is the phantom-slot producer #1 documented in
   .claude/rules/phantom-slot-frame-lever.md ("folded loop-guard compare"),
   reached from ordinary live C with no dead declaration and no coercion
   construct. Adding an explicit `if (count <= 0) return 0;` guard back in
   DELETES the rotation guard (jump threading) and the frame collapses to
   vars=0 — see rejected/explicit-guard-kills-rotation-orphan.c.

   The inner loop keeps the s0..s5 goto spelling verbatim because that is the
   spelling that reproduces target's block layout exactly (out-of-line
   `block_6c` fail arm, `bne` sense, no peeled first load). Writing the inner
   loop as a `do { } while` with `break`s makes GCC rotate and peel it too —
   see rejected/for-loop-rotates-inner-loop-too.c.

   REMAINING RESIDUAL (35 ours vs 36 target), fully characterised:
     (a) target's `addu $t3,$v0,$zero` copy in the loop preheader is absent —
         we load D_800A38C8 straight into the loop-bound register. Splitting
         the source into `var_v0 = D_800A38C8; var_t3 = var_v0;` does NOT
         restore it (copy-prop folds it) — rejected/split-count-copy-folded-
         by-copyprop.c.
     (b) `lui/addiu` for D_80102810 is emitted at stream positions 1-2 instead
         of target's 6-7 (i.e. above the `lw`/`blez` instead of inside the
         guarded preheader).
     (c) ONE register 2-swap left: the outer counter gets $t0 / the inner end
         pointer gets $t1; target has counter=$t1, end=$t0. The s0..s5
         "5-way coupled rotation" is otherwise GONE — $a1/$a2/$v1/$v0/$a3/
         $t2/$t3 all land on target's registers in this form. */
s32 func_80037B00(u8 *arg0) {
    s32 var_t1;
    s32 var_t3;
    s32 var_t2;
    s8 *var_a3;
    s8 *var_a1;
    s8 *var_a2;
    s8 *var_t0;
    s32 var_v1;
    s32 var_v0;

    var_t1 = 0;
    var_t3 = D_800A38C8;
    var_a3 = (s8 *)&D_80102810;
    while (var_t1 < var_t3) {
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
