/* func_80037B00 - MATCHED (s13, 2026-08-26).
   sandbox --disable all: score=0, target_insns=36, build_insns=36,
   rules_dropped=0.  Full build SHA1 == oracle
   (62efab4f73f992798c43e8c730aa43baa10bb4fa).  Zero regfix/asmfix rules,
   zero inline asm, zero FAKE constructs - COMPLETED-C.

   WHAT CLOSED IT (the s12 residual, in one edit).
   s8..s12 reproduced target's 36 instructions position-for-position with a
   single adjacent register transposition: our outer counter (allocno 73)
   took $t0 and our inner end pointer (allocno 78) took $t1, where target has
   counter=$t1 and end=$t0.  global.c ranks allocnos by
   pri = floor_log2(refs) * refs / live_length; the frame-bearing chassis
   measured pri(73) = 3*8/23 = 1.0435 and pri(78) = 2*4/9 = 0.888, so 73 was
   allocated first and took the lower seat.

   s12 concluded both chassis "reduce to the SAME quantity: L73 must reach 28"
   and spent the session (correctly) proving every route to a longer counter
   live range dead.  That framing was one-sided: the inequality
   pri(78) > pri(73) can equally be reached from the NUMERATOR of 78.  refs is
   not an instruction count - flow.c accumulates `REG_N_REFS += loop_depth`,
   so a reference's weight is its LOOP NESTING DEPTH.  The inner
   character-compare loop was spelled with `goto loop_inner`, and a goto
   back-edge carries no NOTE_INSN_LOOP_BEG/END pair, so flow saw the whole
   outer body at depth 2 and charged the end pointer's two references 2 + 2 = 4.

   Spelling the inner loop as a real `while (1) { ... break; ... }` gives it
   the loop notes.  The end pointer's def stays at outer depth 2 but its use -
   the bottom `slt $v0,$a1,$t0` - is now at depth 3, so refs(78) goes 4 -> 5
   with live_length(78) UNCHANGED at 9:
       pri(78) = floor_log2(5)*5/9 = 2*5/9 = 1.111  >  pri(73) = 1.0435
   and 1.111 < pri(75) = 1.350, so 78 slots in EXACTLY between var_a3 and the
   counter.  Measured greg order goes
       79 76 77 75 73 83 78 74 82 81 72   (73 in 8, 78 in 9)   - s8..s12
    -> 79 76 77 75 78 73 83 74 82 81 72   (78 in 8, 73 in 9)   - this form,
   i.e. end=$t0, counter=$t1, flag=$t2 - target's assignment exactly.  Nothing
   else moves: refs(73) stays 8, L73 stays 23, L74 stays 16, refs(75)/L75 stay
   9/20, the phantom 8-byte frame survives (`vars= 8`), and the emitted
   instruction count is identical (30 cc1 lines before maspsx, 36 after).

   The other three levers are inherited from s8 and still load-bearing:
     1. the entry guard is a source-level `if` whose condition IS the loop's
        own test (`if (var_t1 < D_800A38C8)`), which puts the `lui/addiu` for
        D_80102810 inside the guarded preheader and collapses the two exit
        tests to a single `blez`;
     2. the outer loop stays a top-tested `while`, so loop.c's
        duplicate_loop_exit_test leaves an orphan compare pseudo that reload's
        alter_reg pays a stack slot - target's `addiu $sp,$sp,-8` / `+8` pair
        at zero instruction cost (the counter-naming guard is what keeps that
        pseudo live past combine - s12 measured four counter-free guards, all
        frameless);
     3. `var_t1 = 0;` sits BEFORE the `if`, so reorg fills the `blez` delay
        slot with the counter init;
     4. `var_t3 = D_800A38C8;` is read inside the if-body before the `la`, so
        cse turns it into target's `addu $t3,$v0,$zero` in target's position.

   The `while (1)` + `break` inner loop is also the plainer reading of a
   bounded strncmp: compare up to 0x15 bytes of arg0 against the current
   0x28-byte table entry, stop at NUL, flag a mismatch, advance.  The matched
   same-file sibling func_80037AA4 (same table D_80102810, same bound
   D_800A38C8) uses the same counter-naming entry guard and carries the same
   phantom frame - this is the original author's house idiom. */
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

    var_t1 = 0;
    if (var_t1 < D_800A38C8) {
        var_t3 = D_800A38C8;
        var_a3 = (s8 *)&D_80102810;
        while (var_t1 < var_t3) {
            var_t2 = 0;
            var_a1 = var_a3;
            var_a2 = (s8 *)arg0;
            var_t0 = var_a3 + 0x15;
            while (1) {
                var_v1 = (u8)*var_a2;
                if (var_v1 == 0) {
                    break;
                }
                var_v0 = (u8)*var_a1;
                if (var_v1 != var_v0) {
                    goto block_6c;
                }
                var_a1 += 1;
                var_a2 += 1;
                if ((s32)var_a1 >= (s32)var_t0) {
                    break;
                }
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
