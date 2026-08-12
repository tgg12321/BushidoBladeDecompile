/* func_800645B0 (src/text1b.c) — grind session 3 (structural modality), 2026-08-12.
 *
 * HONEST PURE-C SANDBOX DISTANCE: 1  (`sandbox func_800645B0 --disable all`,
 * score 1, target_insns 78 / build_insns 78, rules_dropped 1).  Unchanged from
 * the session-2 floor, but this is a STRICTLY CLEANER SPELLING of it and the
 * residual has been re-characterised down to a single named GCC decision.
 *
 * DELTA vs the session-2 candidate (variant "AA"): both loops are now written
 * in the canonical `for` form.  The previous form was a nested do/while with an
 * explicit `j += 1;` statement placed between `mask = val << idx;` and the
 * occupancy test.  Sweep 16 measured the two BYTE-IDENTICAL (SA==CA at 3/78,
 * SB==AA at 1/78), so the `for` spelling costs nothing and is what a human
 * writes for "scan slots 0..14 in groups of four".  This also permanently
 * retires the question of where `j += 1;` sits: the canonical loop form
 * compiles to the same bytes, so no statement placement is load-bearing here.
 *
 * THE FORM
 *   for (i = 0; i < 0xF; i += 4)          outer: slot groups
 *     for (j = 0; j < 4; j++)             inner: slot within the group
 *       idx  = i + j;                     slot index
 *       val  = 1; mask = val << idx;      occupancy bit
 *       if free:
 *         idx2 = idx << 1;                halfword (2-byte-stride) element index
 *         last = rand();
 *         idx  = idx2 + idx;              word (12-byte-stride) element index
 *         3 x s32 store at (idx << 2), then the s16 store at idx2, then set
 *         the occupancy bit and break.
 *
 * WHY EACH CONSTRUCT IS THERE (all inherited and previously ruled legitimate;
 * see memory/grind/func_800645B0/self_vet.md history and evidence.md):
 *  1. `val` carries the constant 1 AND (later) the D_800A3444 read and the OR
 *     result.  Reusing it is the sanctioned variable-reuse family
 *     ([[defeat-licm-hoist-var-reuse]]): a single-set const-1 pseudo is a
 *     loop.c movable and move_movables hoists it into a fresh callee-save,
 *     costing a save/restore pair (measured again this session: KA/KB/TA all
 *     80 insns).  The multi-set `val` also lands in $v1, the register the
 *     target uses for both roles.
 *  2. `val = val | mask; D_800A3444 = val;` (not `D_800A3444 = val | mask;`)
 *     makes `val` the OR destination, matching `or $v1,$v1,$s2`.
 *  3. `last = rand();` names the first call so the *3 sum is emitted AFTER the
 *     `jal` and reorg.c fills the jal delay slot with the `sll` (target's
 *     shape).  The same naming is used for the fourth call.
 *  4. `idx = idx2 + idx;` reuses the slot-index variable for the derived word
 *     index — the target's own allocation does exactly this ($s0 = i+j, then
 *     $s1+$s0, then $s0<<2) — and, being a REAL second set, it removes
 *     sched.c's birthing_insn_p bonus from `addu idx,i,j` so the inner-loop
 *     block emits the `addu` first and reorg.c steals it into the back-edge
 *     delay slot, as target.
 *
 * WHAT REMAINS (1 point, one instruction, mechanism fully named)
 *   idx | target              | this build
 *   20  | addu s0,s1,s0       | addu s0,s0,s1
 * optabs.c:399-417 (`expand_binop`) swaps a commutative operand pair whenever
 * the expansion `target == op1`, so ANY `idx = <x> + idx;` expands as
 * (plus idx x) — already visible in the .rtl dump, before cse runs.  Emitting
 * the target's (plus idx2 idx) therefore requires the sum's DESTINATION pseudo
 * to differ from `idx` (variant CA, `wid = idx2 + idx;`), which costs
 * construct 4 above and puts the loop top back at 3 points.
 *
 * Session 3 measured the two halves of that trade to one instruction each and
 * proved they are the SAME instruction budget from opposite sides:
 *   CA (sum in its own `wid`)  = 3/78, unmasked diff is ONLY the loop-top
 *      order (indices 11/12 swapped, plus its copy at 65).
 *   IA (idx MAINTAINED: set before the inner loop and again at its tail, sum
 *      in `wid`) = 1/78, unmasked diff is ONLY index 11, where GCC constant-
 *      folds `j == 0` in the loop-entry copy and emits `move s0,s3` where the
 *      target has `addu s0,s3,a0`.
 * Session 3 also established (reading asm/funcs/func_800645B0.s directly) that
 * the target's `addu $s0,$s3,$a0` appears TWICE — immediately before the inner
 * loop label and in the back-edge delay slot — which is reorg.c's
 * steal-from-target + loop-entry duplicate.  So the ORIGINAL C recomputes the
 * slot index at the loop top (the CA shape), and no hand-written entry copy can
 * reproduce that insn because GCC folds `j == 0` in it.
 *
 *
 * SESSION 5 (permuter modality, 2026-08-12) — form UNCHANGED, floor still 1.
 * Two decomp-permuter campaigns were run against a newly-built, clean
 * per-function workspace (tmp/grind/func_800645B0/s4/mkws.sh — offset-0
 * target.o, honest pipeline with no regfix/asmfix, validated at 78 vs 78
 * instructions with the single known diff).  Campaign 1 sampled this exact
 * chassis (permuter base score 10) for 39,731 iterations and returned one
 * find, at score 10 — equal to base, and a dead store (`idx = (last =
 * rand());`).  Campaign 2 sampled the structurally different IA
 * maintained-index chassis (base score 200) for 52,757 iterations; its best
 * find was the already-banked MA shape (sandbox 2/78), and every other find
 * broke the loop's semantics.  ~92,500 iterations produced nothing below this
 * form, so the permuter axis is measured dead for both chassis and the shipped
 * body below is retained verbatim.
 * INTEGRATION NOTE: regfix.txt:2521 (`func_800645B0: reorder 3,1,2 @ 1-3`) is
 * the function's only rule; the sandbox drops it.  Retiring it is the
 * operator/driver's job, not a grind session's.
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
