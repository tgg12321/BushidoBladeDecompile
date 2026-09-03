/* func_800645B0 (src/text1b.c) -- MATCHING FORM.  Honest distance 0 / 78
 * (target_insns 78, build_insns 78, rules_dropped 0, zero cheat-asm), and the
 * full clean-driver build SHA1 == the oracle 62efab4f73f992798c43e8c730aa43baa10bb4fa,
 * both measured in grind session s19 (2026-09-02, synthesis modality) with this
 * exact body in src/text1b.c.
 *
 * WHAT CLOSED IT.  Two independent changes off the s18 frontier, neither of
 * which had been combined before:
 *
 *  1. `idx = idx * 12;` (the s18 "a2" chassis).  The *3 word index written as
 *     `idx = idx2 + idx;` can never emit the target's `addu $s0,$s1,$s0`,
 *     because optabs.c expand_binop (tools/gcc-2.7.2/optabs.c:409-420) swaps a
 *     commutative binop's operands whenever the expansion target rtx IS op1 --
 *     and for `idx = <anything> + idx` the target rtx is idx.  Routing the add
 *     through expand_mult gives it a fresh temp as its target, so no swap
 *     happens and stream index 20 is exact.  `idx * 12` is also the natural
 *     spelling: D_800F0D78 / D_800F0D7C / videoDec are one 3-word record, so
 *     the byte offset for slot `idx` is idx * 12, and `idx2 = idx << 1` is the
 *     halfword record's byte offset.  On its own this chassis measured 3/78:
 *     it lost the inner-loop head (stream 11/12) and the back-edge delay slot
 *     (65), because with the sum in a temp `idx` is left single-set and
 *     sched.c birthing_insn_p (sched.c:2526) lifts the loop-top addu to
 *     max_priority.
 *
 *  2. The const-1 LICM-defeat carrier moved from `val` to `last`.  Sessions
 *     1-18 all carried the loop-invariant `1` in `val`; that defeats loop.c's
 *     hoist either way (both locals are set in two basic blocks of the loop, so
 *     count_loop_regs_set at loop.c:3040 marks them may_not_move), but it also
 *     decides WHICH of the two scratch locals is block-local and therefore
 *     handled by local-alloc rather than global-alloc.  With `last` carrying
 *     the constant, `val` is confined to the D_800A3444 read-modify-write
 *     inside the `if`, and the whole allocation -- including the loop head and
 *     the delay slot the a2 chassis had lost -- lands exactly on the target.
 *     Measured this session: a2 + `val` carrier = 3/78, a2 + `last` carrier =
 *     0/78.  The same carrier swap also closes two other chassis to 0/78 (the
 *     s17 WD fresh-dest chassis, and WD with the byte offset folded into
 *     `wid`), so the lever is chassis-independent; this body is the one that
 *     needs no invented local at all -- it uses only the target's own seven.
 *
 * CONSTRUCTS.  One FAKE-annotated construct: the const-1 staged through `last`
 * (sanctioned family: .claude/rules/defeat-licm-hoist-var-reuse.md, borrow
 * gated by .claude/rules/staged-value-reused-variable.md).  Everything else is
 * ordinary C: the byte-offset multiply, the halfword shift, and the
 * read-modify-write through `val` (layer-1 ruled the RMW spelling legitimate on
 * 2026-08-12).  No dead stores, no wraps, no statement reordering, no invented
 * locals, no pins, no asm.  Self-vet: memory/grind/func_800645B0/self_vet.md.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            /* FAKE: the shift's constant 1 is staged through `last`, the
             * scratch local that holds each rand() result below (its previous
             * value is dead here -- the last read of it is the `last & 7` of
             * the preceding iteration).  mechanism: GCC 2.7.2 loop.c
             * count_loop_regs_set (loop.c:3040) marks a register set in two
             * basic blocks of the loop `may_not_move`, so scan_loop never
             * admits the const-1 as a movable and move_movables cannot hoist
             * it; written with a single-set carrier the `li` is hoisted into a
             * fresh callee-save and the function costs two extra instructions
             * (measured 12/80).  lever-exhaustion:
             * memory/grind/func_800645B0/hypotheses.md, sessions s1-s19. */
            last = 1;
            mask = last << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                idx = idx * 12;
                *((s32 *)(((s32)(&D_800F0D78)) + idx)) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + idx)) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + idx)) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
