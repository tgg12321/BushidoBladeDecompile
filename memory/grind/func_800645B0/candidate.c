/* func_800645B0 (src/text1b.c) — CLOSING FORM, grind session 9 (structural
 * modality, 2026-08-12).  `sandbox func_800645B0 --disable all` = score 0,
 * target_insns 78, build_insns 78, rules_dropped 1 — measured THIS session with
 * this exact body in src/text1b.c.  Supersedes the standing floor of 1 that
 * held from session 2 through session 8.
 *
 * ------------------------------------------------------------------------
 * WHAT CHANGED FROM THE SESSION-6/8 "SB" BODY (the 1/78 floor)
 * ------------------------------------------------------------------------
 * Exactly one statement.  The SB body wrote the *3 sum as
 *
 *     idx = idx2 + idx;          /_ 1 / 78: addu $s0,$s0,$s1, target wants $s0,$s1,$s0 _/
 *
 * and its ENTIRE residual was that single instruction's operand order.  This
 * body stages the addend through `val` first:
 *
 *     val = idx;
 *     idx = idx2 + val;          /_ 0 / 78: addu $s0,$s1,$s0 _/
 *
 * Nothing else in the function moved: same loop skeleton, same declarations,
 * same statement order, same store spellings, same `val` roles, same
 * `last = rand();` naming.  The diff is one added assignment.
 *
 * ------------------------------------------------------------------------
 * WHY IT CLOSES (the mechanism, and why no other spelling can)
 * ------------------------------------------------------------------------
 * H24 (session 6) established the wall: optabs.c:403-421 (`expand_binop`)
 * swaps a commutative operand pair when `target == op1`.  The SB body needs
 * the sum's destination to BE `idx` — that second set of `idx` is what denies
 * sched.c's `birthing_insn_p` priority lift on the loop-top `addu idx,i,j`
 * and therefore gets the inner-loop top emitted in the target's order — but
 * writing the sum into `idx` with `idx` as the addend makes `target == op1`
 * and the swap is unconditional.  H39 (session 8) then proved that ANY second
 * set of `idx` must be an instruction the target already has, which leaves
 * exactly two candidates: this sum, and the <<2 byte offset (walled at
 * local-alloc by H30/H31).  So the whole function came down to: emit the
 * target's operand order for a PLUS whose destination is `idx`.
 *
 * Staging the addend does exactly that and nothing else.  With the addend in
 * a different pseudo, neither optabs clause can fire (clause 1 needs a non-REG
 * `op0`, and `idx2` is a REG), so the pair is emitted unswapped; the staging
 * copy itself is coalesced away, so the function still assembles to 78
 * instructions.  `reg_n_sets[idx]` is unchanged at 2, so the loop top keeps
 * the order SB already had.
 *
 * ------------------------------------------------------------------------
 * MEASURED ALTERNATIVES (this session, tmp/grind/func_800645B0/s9b/)
 * ------------------------------------------------------------------------
 * sweep30.py — SB control WA = 1/78.  SIX staging spellings all reach 0/78:
 *   WB fresh local `k` before the sum; WC (THIS BODY) staged through the
 *   existing `val`; WD copy before the `idx2` shift; WE copy before the
 *   `rand()` call; WF copy in an inner block scope; WG the staged value also
 *   feeding the halfword shift.  WH (staging through `last`, which forces the
 *   `rand()` call later) is 2/78.
 *   WC is the shipped one because [[staged-value-reused-variable]] bound 2
 *   requires borrowing a variable the function ALREADY has for a real job —
 *   WB/WD/WE/WF/WG all invent a variable and are therefore NOT covered by the
 *   sanctioned family even though they measure identically.
 * sweep31.py — the no-staging route: derive the addend arithmetically from
 *   `idx2` instead, so no copy exists at all.  `idx2 + (idx2 >> 1)` = 2/79,
 *   `idx2 + (u32)idx2 >> 1` = 2/79, `idx2 + (idx2 / 2)` = 14/79.  Every one
 *   pays at least one real instruction for the re-derivation against a
 *   78-instruction target.  Banked at
 *   rejected/sum-addend-rederived-from-idx2-costs-an-instruction.c.
 *
 * ------------------------------------------------------------------------
 * POLICY POSITION
 * ------------------------------------------------------------------------
 * Sanctioned family: staged value through a reused variable
 * (.claude/rules/staged-value-reused-variable.md, owner ruling 2026-07-03;
 * commit d9e490c0).  The separately-standing `val` dual-role construct that
 * sessions 3-8 already shipped is the frozen-list entry "Variable reuse for
 * codegen control", .claude/rules/no-new-park-categories.md:170.
 * All six bounds of the staged-value rule
 * are answered construct-by-construct in memory/grind/func_800645B0/self_vet.md.
 * The staged value is real and consumed by the next statement (zero dead
 * code); `val` is a pre-existing multiply-assigned local; the borrow window is
 * from the mask shift to the D_800A3444 re-load and is provably safe; the
 * FAKE annotation carries what + mechanism + lever-exhaustion.
 *
 * This is NOT the loop-top staging construct the driver bans for this
 * function: the loop top here is byte-for-byte the SB spelling, no statement
 * was reordered, and the mechanism is optabs.c's commutative swap in RTL
 * expansion — an axis no previous session ever put a lever on — not the
 * first-pass-scheduler tie / delay-slot-steal interaction that this function's
 * three prior layer-1 FAILs all turned on.
 *
 * INTEGRATION NOTE: regfix.txt:2521 (`func_800645B0: reorder 3,1,2 @ 1-3`) is
 * the function's only rule and the sandbox drops it.  Retiring the rule +
 * `queue done` is the operator/driver's job, not a grind session's.
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
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                /* FAKE: the slot index staged through `val` -- `val`'s const-1
                 * value is dead from the mask shift above until `val` is
                 * re-loaded from D_800A3444 below, and the staged value is not
                 * read after that re-load, so the borrow is safe; the value is
                 * real and consumed by the very next statement.  Mechanism:
                 * optabs.c `expand_binop` (optabs.c:403-421) swaps a
                 * commutative operand pair whenever the expansion target IS
                 * op1, so writing the *3 sum back into `idx` with `idx` as the
                 * addend can only ever emit `addu $s0,$s0,$s1`; staging the
                 * addend in another pseudo leaves the pair unswapped and emits
                 * the original's `addu $s0,$s1,$s0`.  lever-exhaustion:
                 * memory/grind/func_800645B0/hypotheses.md sessions 1-9
                 * (H24/H25/H31/H35/H39 close every other route to this operand
                 * order; sweep31 measures the no-staging arithmetic spellings
                 * at 79 instructions against a 78-instruction target). */
                val = idx;
                idx = idx2 + val;
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
