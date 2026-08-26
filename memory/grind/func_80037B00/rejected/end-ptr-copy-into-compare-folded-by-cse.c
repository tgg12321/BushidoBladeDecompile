/* func_80037B00 — BEST HONEST FORM as of s8 (2026-08-26).
   sandbox --disable all: score=5, target_insns=36, build_insns=36,
   cheat_asm_stripped=3, rules_dropped=0, cc1 `.frame $sp,8,$31 # vars= 8`.
   Supersedes the s7 form (score 9).  THE INSTRUCTION STREAM IS NOW EXACT,
   POSITION FOR POSITION — every one of the 36 instructions matches target's
   opcode, operands and order, INCLUDING the frame adjusts, the preheader
   `addu $t3,$v0,$zero`, the `lui/addiu` pair for D_80102810 in the guarded
   preheader, and the `blez` delay slot.  The ONLY remaining defect is a
   single register 2-swap: our outer counter takes $t0 and our inner end
   pointer takes $t1, where target has counter=$t1 and end=$t0.

   WHY THIS SHAPE — four independent levers, all ordinary live C:

   1. THE ENTRY GUARD IS WRITTEN AS A SOURCE-LEVEL `if`, AND ITS CONDITION IS
      THE LOOP'S OWN TEST (`if (var_t1 < var_t3)` with var_t1 already 0).
      This is what puts `lui/addiu D_80102810` INSIDE the guarded preheader:
      the guard branch precedes the `var_a3 = ...` statement in RTL order.
      s7 spent the session trying to get that pair into the preheader via giv
      / strength-reduction spellings and paid +2 instructions every time
      (rejected/giv-index-costs-offset-biv-add.c,
      rejected/giv-pure-coalesced-into-walking-ptr.c).  The la never needed
      hoisting — it needed to be WRITTEN AFTER THE GUARD.
      Writing the guard as `if (D_800A38C8 > 0)` instead measures score 10 /
      37 insns: cc1 then emits TWO `blez` branches, because loop.c's
      `duplicate_loop_exit_test` copies the while's exit test in front of
      NOTE_INSN_LOOP_BEG regardless of the source guard, and the two
      conditions are not syntactically the same jump for jump2 to merge
      (rejected/if-while-init-before-if-double-guard.c).  Spelling the guard
      AS the loop test is what lets the two collapse to one branch.

   2. THE OUTER LOOP STAYS A TOP-TESTED `while` (inherited from s6).  That is
      what makes loop.c duplicate the exit test; combine folds the duplicate's
      comparison into a bare `blez`, the compare pseudo survives as a
      referenced but homeless allocno, reload's `alter_reg` hands it a stack
      slot, `get_frame_size()` counts 4, MIPS_STACK_ALIGN rounds it to 8, and
      target's `addiu $sp,$sp,-8` / `addiu $sp,$sp,8` appear at ZERO
      instruction cost.  Producer #1 ("folded loop-guard compare") of
      .claude/rules/phantom-slot-frame-lever.md.  A `do { } while` outer loop
      loses the duplication and with it the frame — that form measures 34
      insns (memory/grind/func_80037B00/alt_base_vF2.c, score 10).

   3. `var_t1 = 0;` SITS BEFORE THE `if`, NOT INSIDE IT.  reorg fills a branch
      delay slot from the PRECEDING insn first, so the counter init lands in
      the `blez` delay slot exactly as target has it.  With the init inside
      the if-body the slot takes a nop and the form costs +1
      (tmp/grind/func_80037B00/s8/vF_if_dowhile_reload.c).

   4. `var_t3 = D_800A38C8;` IS READ INSIDE THE if-BODY, BEFORE THE `la`.
      cse sees the global already loaded by the guard and turns this into a
      register copy — target's `addu $t3,$v0,$zero` — and because it is a
      source statement placed before the `var_a3` assignment, the copy is
      emitted BEFORE the `lui/addiu` pair, which is target's order.  Reading
      the bound in the loop test instead (the s7 lever) also produces the
      copy, but as loop.c's transfer copy inserted in front of
      NOTE_INSN_LOOP_BEG — i.e. AFTER the la, costing two emission-position
      penalties (tmp/grind/func_80037B00/s8/vH_guard_is_loop_test.c, score 7).
      Caching the bound BEFORE the `if` instead kills the copy entirely, since
      cse then has a single pseudo for guard and loop test
      (rejected/if-dowhile-count-local-no-preheader-copy.c).

   The inner loop keeps the s0..s5 goto spelling verbatim: it is the spelling
   that reproduces target's block layout exactly (out-of-line `block_6c` fail
   arm, `bne` sense, no peeled first load).  A `do { } while` with `break`s
   makes GCC rotate and peel the inner loop too —
   rejected/for-loop-rotates-inner-loop-too.c.

   REMAINING RESIDUAL — ONE ADJACENT ALLOCNO SWAP.
   From this form's own .lreg/.greg (recomputed, not inherited): eleven
   allocnos, of which eight mutually conflict and take $3,$5,$6,$7,$8,$9,$10,
   $11 in priority rank order (pri = floor_log2(refs)*refs/live_length).
   79 = 6/4 = 3.00, 76 = 10/11 = 2.727, 77 = 8/10 = 2.400, 75 = 9/20 = 1.350,
   73 (counter) = 8/23 = 1.0435, 78 (end ptr) = 4/9 = 0.888, 74 = 6/16 = 0.750,
   81 = 3/21 = 0.143, 72 = 3/24 = 0.125.  The greg's
   `;; 11 regs to allocate: 79 76 77 75 73 83 78 74 82 81 72` matches that
   order verbatim (83 is the frame orphan, ST_REGS, takes no GR; 82 takes $2).
   Target needs 73 and 78 transposed, i.e. pri(78) inside
   (pri(73) = 1.0435, pri(75) = 1.350) — a window three times wider than the
   one s7 faced.  Reachable cells: refs 5 at live_length 8 or 9; refs 6 at
   live_length 9, 10 or 11; refs 4 at live_length 6 or 7.  Out-of-loop
   references weigh 1 and in-loop references weigh 2 (flow.c:434 / 2081), and
   a `(set (reg X) (plus (reg X) c))` counts TWO occurrences — see
   hypotheses.md s8. */
s32 func_80037B00(u8 *arg0) {
    s32 var_t1;
    s32 var_t2;
    s8 *var_a3;
    s8 *var_a1;
    s8 *var_a2;
    s8 *var_t0;
    s32 var_v1;
    s32 var_v0;
    s32 var_t3;
    s8 *var_e;

    var_t1 = 0;
    if (var_t1 < D_800A38C8) {
        var_t3 = D_800A38C8;
        var_a3 = (s8 *)&D_80102810;
        while (var_t1 < var_t3) {
            var_t2 = 0;
            var_a1 = var_a3;
            var_a2 = (s8 *)arg0;
            var_t0 = var_a3 + 0x15;
            var_e = var_t0;
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
            if ((s32)var_a1 < (s32)var_e) {
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
    }
    return 0;
}
