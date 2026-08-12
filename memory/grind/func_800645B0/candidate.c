/* func_800645B0 (src/text1b.c) — grind session 5 (permuter modality), 2026-08-12.
 *
 * RE-VALIDATED in the session-5 rerun (the prior s5 was discarded on a self-vet
 * PROSE collision with the driver's banned-construct tripwire, not on this C):
 * applied to src/text1b.c from a clean tree, `sandbox func_800645B0
 * --disable all` again reports score 0, 78/78, rules_dropped 1.
 *
 * HONEST PURE-C SANDBOX DISTANCE: 0  (`sandbox func_800645B0 --disable all`,
 * score 0, target_insns 78 / build_insns 78, rules_dropped 1).  This closes the
 * standing floor of 1 that sessions 2-4 held.
 *
 * WHAT CHANGED vs the session-3/4 candidate (floor 1)
 *   The word (12-byte-stride) element index now lands in its OWN variable
 *   `wid` instead of being assigned back into `idx`, and the slot index is
 *   staged through that same `wid` at the top of the inner loop:
 *
 *       wid = i + j;          <- slot index computed into wid
 *       idx = wid;            <- staged into idx, which drives the mask
 *       ...
 *       wid = idx2 + idx;     <- wid reused for the word index
 *
 *   Both prior near-miss shapes are subsumed:
 *     - SB (shipped, `idx = idx2 + idx;`) was 1/78, failing ONLY on
 *       `addu $s0,$s0,$s1` vs target `addu $s0,$s1,$s0` — optabs.c:399-417
 *       (`expand_binop`) swaps a commutative operand pair whenever the
 *       expansion target IS op1, and with the sum assigned back into `idx`
 *       the target always WAS op1.
 *     - CA (`wid = idx2 + idx;` with `idx = i + j;` unchanged) was 3/78: the
 *       operand order is right (target is `wid`, neither operand), but the
 *       inner-loop top then emits `li $v1,1` before `addu $s0,$s3,$a0`, and the
 *       back-edge delay slot gets the `li` instead of the `addu`.
 *   The staging copy closes the second failure while keeping the first fixed:
 *   with `wid` set twice (loop top + the sum) it is no longer a single-set
 *   pseudo, so sched.c's `birthing_insn_p` priority bonus no longer lifts the
 *   const-1 set above the index computation, the `addu` is emitted first, and
 *   reorg.c steals it into the back-edge delay slot exactly as target does.
 *   This is the SAME multi-set mechanism the session-2/3 candidate already
 *   relied on for `val` and (in SB) for `idx` — it has simply been moved onto
 *   the variable whose expansion target also has to stay off op1.
 *
 * PROVENANCE
 *   Found by decomp-permuter on a fresh CA-chassis workspace built this session
 *   (tmp/grind/func_800645B0/s5/mkws3.sh, base permuter score 260), find
 *   `output-0-1` at ~6k iterations, then validated independently with the
 *   engine sandbox on src/ (score 0).  It CONFIRMS the session-4 frontier
 *   hypothesis verbatim: "the expand_binop commutative swap can be defeated by
 *   changing the EXPANSION TARGET rather than the syntax."
 *
 * THE FORM
 *   for (i = 0; i < 0xF; i += 4)          outer: slot groups
 *     for (j = 0; j < 4; j++)             inner: slot within the group
 *       wid = i + j; idx = wid;           slot index (staged through wid)
 *       val = 1; mask = val << idx;       occupancy bit
 *       if free:
 *         idx2 = idx << 1;                halfword (2-byte-stride) element index
 *         last = rand();
 *         wid = idx2 + idx;               word (12-byte-stride) element index
 *         3 x s32 store at (wid << 2), then the s16 store at idx2, then set
 *         the occupancy bit and break.
 *
 * CONSTRUCT INVENTORY (all four inherited constructs were ruled legitimate by
 * the session-4 layer-1 reviewer; construct 5 there — the relocated `j += 1;` —
 * was FAILed and is NOT present here, and the canonical `for` spelling
 * permanently retires the statement-placement question):
 *  1. `val` carries the constant 1 AND, later, the D_800A3444 read and the OR
 *     result (sanctioned variable reuse; a single-set const-1 pseudo is a
 *     loop.c movable and move_movables hoists it into a fresh callee-save,
 *     costing a save/restore pair — measured, 80 insns).
 *  2. `val = val | mask; D_800A3444 = val;` makes `val` the OR destination,
 *     matching `or $v1,$v1,$s2`.
 *  3. `last = rand();` names the first and fourth calls so the dependent
 *     arithmetic is emitted AFTER the `jal` and reorg.c fills the jal delay
 *     slot as target does.
 *  4. `wid` carries the slot index at the loop top and the derived word index
 *     inside the claim block (sanctioned variable reuse; see self_vet.md).
 *     Neither assignment is dead: `wid = i + j;` is read by `idx = wid;`, and
 *     `wid = idx2 + idx;` is read by all three word stores.
 *
 * INTEGRATION NOTE: regfix.txt:2521 (`func_800645B0: reorder 3,1,2 @ 1-3`) is
 * the function's only rule; the sandbox drops it and the honest distance is 0
 * without it.  Retiring the rule + `queue done` is the operator/driver's job,
 * not a grind session's.
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
    s32 wid;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            wid = i + j;
            idx = wid;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
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
