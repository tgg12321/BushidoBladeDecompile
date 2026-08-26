/* func_80037B00 — ALTERNATIVE BASE "vF2", discovered s8 (2026-08-26).
   NOT the floor form (candidate.c is, at score 9); this measures score=10,
   build_insns=34, target_insns=36, cc1 `.frame $sp,0,$31 # vars= 0`.
   It is banked because its residual is a DIFFERENT, cleaner two-item list
   than candidate.c's, and it is the ONLY measured form that puts target's
   `lui/addiu D_80102810` pair in the guarded preheader at zero cost.

   SHAPE: explicit `if (D_800A38C8 > 0) { ... do { ... } while (var_t1 <
   D_800A38C8); }`, with `var_t1 = 0;` placed BEFORE the `if`, and the loop
   bound RE-READ in the do-while's bottom test (not cached in a local).

   WHAT THIS FORM GETS RIGHT THAT candidate.c DOES NOT:
     * `lui a3 / addiu a3` sit INSIDE the guarded preheader (after the blez),
       exactly like target — because the la is a source statement in the
       if-body, so the guard branch precedes it in RTL order.  candidate.c
       emits them in block 0 above the `lw`, costing two emission-position
       penalties, and that is structurally unfixable there (see evidence.md
       s8: only loop.c writes after the duplicated guard).
     * `move t3,v0` (target's `addu $t3,$v0,$zero`) is present — the second
       read of D_800A38C8 in the bottom test is hoisted by loop.c and cse'd
       into a copy off the guard's load.  Caching the count in a local
       instead (rejected/if-dowhile-count-local-no-preheader-copy.c) makes
       cse merge the two and the copy disappears.
     * The blez delay slot is filled with the counter init (`move t2,zero`)
       because `var_t1 = 0;` sits before the `if` and reorg can take the
       PRECEDING insn.  With the init inside the if-body the slot takes a
       nop (+1 insn — tmp/grind/func_80037B00/s8/vF_if_dowhile_reload.c,
       35 insns).
     * The end pointer already lands on target's $t0 here (allocno 78, rank
       5), so candidate.c's $t0/$t1 2-swap does NOT exist in this form.

   WHAT IS STILL WRONG (exactly two things):
     (a) NO 8-BYTE FRAME.  `vars= 0`: the explicit `if` guard is the only
         entry test, loop.c's duplicate_loop_exit_test never fires, and there
         is no orphaned compare pseudo for reload's alter_reg to pay off.
         Target's `addiu $sp,$sp,-8` / `addiu $sp,$sp,8` are the two missing
         instructions (34 vs 36).
     (b) ONE ADJACENT RA SWAP, counter vs flag: ours has counter($t2)/flag
         ($t1), target has counter($t1)/flag($t2).  Allocnos 73 (counter,
         refs 7, live 23, pri 0.6087) and 74 (flag, refs 6, live 16, pri
         0.75) are adjacent in rank; pri(73) must land in (0.75, pri(78) =
         0.888).  refs 7 needs live_length in {16,17,18} (have 23); refs 8
         needs live_length in (27,32) and the whole function is ~26 insns.
         Both cells are out of reach under this instruction stream — see
         hypotheses.md s8 for the full table.

   The frame is the axis to attack from here: adding it makes the multiset
   exact (36) and leaves one adjacent swap, the same shape candidate.c is in
   but with the emission positions already correct. */
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
    if (D_800A38C8 > 0) {
        var_a3 = (s8 *)&D_80102810;
        do {
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
        } while (var_t1 < D_800A38C8);
    }
    return 0;
}
