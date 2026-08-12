/* func_800645B0 (src/text1b.c) — grind session 6 (forensics modality), 2026-08-12.
 *
 * !! THIS FILE WAS REPLACED THIS SESSION.  The previous contents (the
 * "session 5" permuter find, honest sandbox distance 0) carried the loop-top
 * variable-staging pair that the layer-1 cheat-reviewer FAILed and that the
 * driver now enforces as a BANNED construct for this function.  Per the
 * reviewer's binding next-action it has been reverted; the disproven body is
 * banked at rejected/loop-top-staging-pair-layer1-fail.c.  Do not re-derive it.
 *
 * HONEST PURE-C SANDBOX DISTANCE: 1  (`sandbox func_800645B0 --disable all`,
 * score 1, target_insns 78 / build_insns 78, rules_dropped 1).  This is the
 * standing honest floor and it is the same "SB" body sessions 3 and 4 shipped.
 *
 * THE FORM
 *   for (i = 0; i < 0xF; i += 4)          outer: slot groups
 *     for (j = 0; j < 4; j++)             inner: slot within the group
 *       idx = i + j;                      slot index
 *       val = 1; mask = val << idx;       occupancy bit
 *       if free:
 *         idx2 = idx << 1;                halfword (2-byte-stride) element index
 *         last = rand();
 *         idx = idx2 + idx;               word (12-byte-stride) element index
 *         3 x s32 store at (idx << 2), then the s16 store at idx2, then set
 *         the occupancy bit and break.
 *
 * CONSTRUCT INVENTORY (constructs 1-3 were ruled LEGITIMATE by the session-4
 * layer-1 reviewer and are unchanged; there is no fourth construct here):
 *  1. `val` carries the constant 1 AND, later, the D_800A3444 read and the OR
 *     result (sanctioned variable reuse; a single-set const-1 pseudo is a
 *     loop.c movable and move_movables hoists it into a fresh callee-save,
 *     costing a save/restore pair — measured three ways, 80 insns vs 78).
 *  2. `val = val | mask; D_800A3444 = val;` makes `val` the OR destination,
 *     matching the target's in-place `or $v1,$v1,$s2`.
 *  3. `last = rand();` names the first and fourth calls so the dependent
 *     arithmetic is emitted AFTER the `jal` and reorg.c fills the jal delay
 *     slot as the target does.
 *  4. `idx` carries the slot index and then the derived word index.  Neither
 *     store is dead (`idx = i + j;` is read by the shift and the mask;
 *     `idx = idx2 + idx;` is read by all three word-stride stores), and the
 *     target's own allocation keeps both values in $s0.
 *
 * THE REMAINING 1 POINT, AND WHY IT CANNOT BE PAID ON THIS CHASSIS
 *   The entire residual is instruction index 20: target `addu $s0,$s1,$s0`,
 *   build `addu $s0,$s0,$s1`.  optabs.c:411-421 (`expand_binop`) puts the
 *   emitted PLUS operands in that order for BOTH source spellings — `idx =
 *   idx + idx2;` needs no swap and `idx = idx2 + idx;` is swapped by clause 2
 *   (`target == op1`).  The only construct that reaches `expand_binop` with
 *   `target == 0` (where neither clause can fire) is a destination whose
 *   DECL_RTL is narrower than a word, and every such spelling pays 1-3 real
 *   instructions for the truncate/extend (79-81 against a 78-insn target;
 *   measured, sweep18).  So the target's operand order requires the sum's
 *   DESTINATION PSEUDO to be different from the pseudo holding `i + j`, with
 *   the two coinciding only at register allocation — i.e. the "CA" body, whose
 *   own residual is the inner-loop top.  All remaining search belongs there.
 *
 * INTEGRATION NOTE: regfix.txt:2521 (`func_800645B0: reorder 3,1,2 @ 1-3`) is
 * the function's only rule and the sandbox drops it.  Retiring the rule +
 * `queue done` is the operator/driver's job, not a grind session's.
 */
extern s32 rand(void);
extern void *D_800A347C;
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
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                idx = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (idx << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (idx << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (idx << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
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
