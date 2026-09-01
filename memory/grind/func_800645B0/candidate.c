/* 2026-08-24 MIGRATION NOTE: HEAD is now INCLUDE_ASM — migrated in
   a7892ba2 (2026-08-24 sweep 2); all rules retired and all in-source cheat-asm removed
   from main. Statements below about "HEAD", pins, rules carried, or
   "applied to src" describe the PRE-MIGRATION tree (banked at
   retired-chassis-2026-08/body.c). This body must be pasted over the
   INCLUDE_ASM line before any sandbox re-measure. */
/* func_800645B0 (src/text1b.c) — the "SB" chassis, the standing honest floor.
 * `sandbox func_800645B0 --disable all` = score 1, target_insns 78,
 * build_insns 78, rules_dropped 1 — re-measured in grind session 9c
 * (permuter modality, 2026-08-13) with this exact body in src/text1b.c.
 *
 * PROVENANCE OF THIS FILE.  The previous contents of candidate.c were the
 * session-9 "WC" body, which added `val = idx; idx = idx2 + val;` before the
 * *3 sum and measured 0 / 78.  The layer-1 cheat-reviewer FAILed it (the
 * staging was a directed-sweep artifact chosen to defeat optabs.c's
 * commutative swap and cherry-picked from six functionally-identical
 * spellings), the driver banned the construct for this function, and its
 * binding next-action was: revert to `idx = idx2 + idx;` and treat 1 / 78 as
 * the honest floor.  That is exactly this body.  Do NOT re-derive a staging
 * spelling for the sum — every variable-staging respelling is the same banned
 * construct.
 *
 * WHAT IT IS.  The canonical for-loop spelling of the function: an outer group
 * loop stepping i by 4, an inner loop over the four slots in the group, the
 * first free slot claimed and its three word fields plus one halfword field
 * initialised from rand(), the occupancy bit OR'd into D_800A3444, and the
 * group abandoned via `break`.
 *
 * THE ENTIRE RESIDUAL is one instruction's operand order: at index 20 the
 * target has `addu $s0,$s1,$s0` and this body emits `addu $s0,$s0,$s1`.
 * Mechanism (H24, session 6): optabs.c:403-421 (`expand_binop`) swaps a
 * commutative operand pair when the expansion target IS op1, and the sum must
 * be written back into `idx` because that second set of `idx` is what denies
 * sched.c's `birthing_insn_p` priority lift on the loop-top `addu idx,i,j`.
 *
 * s11 (2026-08-20, escalation post-unpark): floor re-confirmed 1/78 on that
 * day's tree with this exact body.  The 2026-08-20 or-tree-shape-shift
 * carve-out has NO move here: the swapped sum `idx = idx + idx2;` is
 * byte-identical (H24 re-confirmed — the target's operand order is an
 * RTL-expansion impossibility, not a policy-blocked choice), the loop-top
 * swap `j + i` is strictly worse (3/78), and no 3+-term assoc+commutative
 * expression exists for the grouping sub-axis.  See hypotheses.md H55/H56 and
 * the 2026-08-20 decisions.md disposition entry.
 *
 * s12 (2026-08-25, escalation + the owner's 2026-08-24 solver directive):
 * floor re-confirmed 1/78 on that day's tree with this exact body, and the
 * residual mechanically re-derived (78 vs 78 insns, ONE difference: index 20
 * `addu s0,s0,s1` vs target `addu s0,s1,s0`) by
 * tmp/grind/func_800645B0/s12/diff.py.  Two results the next session must not
 * re-derive: (1) `wid = idx2 + idx;` with a FRESH destination local emits
 * target's operand order EXACTLY -- ordinary C, no staging -- at a cost of 3/78,
 * the residual moving wholesale to the inner-loop head (banked at
 * rejected/wd-fresh-dest-sum-exact-operand-order-costs-loop-head.c); and (2)
 * the 1-vs-3 trade is CLOSED-FORM from tools/gcc-2.7.2/optabs.c:398-421 --
 * with the sum's expansion target == idx's pseudo, BOTH `idx = idx2 + idx`
 * (swaps, target == op1) and `idx = idx + idx2` (never swaps) emit (idx, idx2),
 * so target's operand order requires a destination distinct from both operands,
 * which is exactly what drops reg_n_sets[idx] to 1 and hands the loop head
 * back.  See hypotheses H58/H59.  Also killed: `mask = 1 << idx;` (dropping the
 * `val = 1;` naming) = 12/78 at 80 insns -- the named local is load-bearing.
 *
 *
 * s14 (2026-08-30, escalation/disposition): floor RE-MEASURED on that day's tree
 * with this exact body -- score 1, target_insns 78, build_insns 78,
 * rules_dropped 0.  The last live frontier item was killed with measurements
 * (H63): the slot index cannot be a real induction variable, because the TARGET
 * recomputes `i + j` with a register-register `addu $s0,$s3,$a0` twice (peeled
 * above the inner-loop label at 0x800645DC and in the back-edge delay slot at
 * 0x800646B4).  IV1 (`idx = i;` + `idx += 1;`, j retained, WD fresh-destination
 * sum) = 16 / 78 at 83 insns; IV2 (index-only inner loop, j deleted) = 25 / 78
 * at 89 insns.  Banked at rejected/induction-variable-index-costs-five-insns.c
 * and rejected/index-only-inner-loop-drops-j-89-insns.c.  Both endgame-lock
 * gates re-run and FAILED (scan_hand_coded tier=LOW 0/8; no SOTN-master
 * precedent for the staging construct in docs/reference/sotn-construct-index.md),
 * so the 2026-07-27 standing ruling was applied -- see the 2026-08-30
 * REFUSED / OWNER-ACCEPTED INCOMPLETE entry in docs/grind/decisions.md.
 * SIBLING CHASSIS (both banked, both 3 away, both structurally distinct):
 *   chassis_jd_inline_index_arith.c — 3 / 78, index arithmetic written inline
 *     with no idx2/wid locals; residual is the three loop-top points only.
 *   rejected/maintained-index-nonfold-reset-costs-one-insn.c ("OA") — 3 / 79.
 *
 * s15 (2026-09-01, forensics; owner ruling 2026-09-01 Ruling A reopen): floor
 * RE-MEASURED on that day's tree with this exact body -- score 1,
 * target_insns 78, build_insns 78, rules_dropped 0.  The reopen's named probe
 * (the "two-distinct-once-written-locals" spelling of the banned multi-write
 * carrier) was executed in both directions and measured 3 / 78 each, with the
 * residual landing on the WD loop-head positions.  Dump-proven cause: cse.c
 * DELETES a once-written copy intermediate (p1.rtl.txt insn 41 / p2.rtl.txt
 * insn 60 are absent from the corresponding .cse dumps, along with their
 * pseudos), so reg_n_sets for the loop-top carrier stays 1 and sched.c's
 * birthing_insn_p keeps its lift.  Raising reg_n_sets here REQUIRES a
 * multi-write carrier, which the 2026-08-31 amended named-intermediate family
 * excludes by name.  See hypotheses H64/H65 and
 * rejected/two-once-written-locals-copy-deleted-by-cse.c.
 *
 * s16 (2026-09-01, forensics): floor RE-MEASURED on that day's tree with this
 * exact body -- score 1, target_insns 78, build_insns 78, rules_dropped 0.
 * THIS BODY IS STILL THE FLOOR, BUT ITS FRAMING IS OBSOLETE: H68 falsified
 * H59's closed-form "you cannot have both halves".  On the WD chassis (fresh
 * destination `wid = idx2 + idx;`, target's operand order at index 20), adding
 * a second REAL, NON-COPY write to `idx` -- `idx = last & 7;`, the masked
 * random value the halfword store consumes, computed into the slot index that
 * is dead from the *3 sum onward -- keeps reg_n_sets[idx] == 2 alive through
 * combine into sched.c, denies birthing_insn_p's lift, and reproduces the
 * target's loop head (11/12) and back-edge delay slot (65) as well.  Measured
 * 2 / 78 at 78 insns; the ONLY residual is two register names (`andi s0,v0,7`
 * / `sh s0,X(at)` against the target's `$v0`).  See hypotheses H66/H67/H68 and
 * rejected/both-halves-idx-second-real-nonco-andi-regseat-2of78.c.  The next
 * attack on this function is a REGISTER-SEAT question (ra_solver /
 * inverse_compose classify with the owner's 2026-09-01 Ruling-C
 * --target-object escape), not a scheduling or RTL-expansion question.
 * Also killed this session: every once-set const-1 carrier (loop.c hoists the
 * invariant set out of both loops and RA seats it callee-saved: +2 insns), and
 * every second write to the carrier whose RHS is a bare register (cse.c deletes
 * pseudo->pseudo copies, combine.c deletes hardreg->pseudo call returns).
 *
 * s16 (2026-09-01, escalation/disposition -- the SIXTH session on this ladder;
 * NOTE the evidence.md block immediately below self-labels "Session 16", which
 * is the previous session; driver numbering for THIS one is s16 and its scratch
 * is tmp/grind/func_800645B0/s16/): floor RE-MEASURED on this tree with this
 * exact body -- score 1, target_insns 78, build_insns 78, rules_dropped 0.
 * THIS BODY IS STILL THE FLOOR.  The s15 frontier (the register-SEAT residual of
 * the h form) was taken to a typed verdict with the owner's 2026-09-01 Ruling-C
 * instrument and both remaining frontier items were KILLED:
 *   - `inverse_compose.py classify --target-object build/src/text1b.o
 *     --ours-object tmp/sandbox/func_800645B0/text1b.o` on the h build:
 *     FIRST DIVERGENCE = **RA** (78 vs 78, the two andi/sh insns only).
 *     goal_from_tgt narrows it to UNIQUE pseudo 74 -> $v0, and
 *     `inverse.py global --goal {"74":2} --depth 2` returns **FORECLOSED /
 *     NEGATIVE**: pseudo 74 crosses one call, so global.c:897
 *     prune_preferences strips the call-used $v0 before find_reg ever sees it
 *     (and flow records a hard conflict with $v0).  Same NEGATIVE for the k
 *     (byte-offset) build under its full goal {"74":16,"78":16,"73":3} and
 *     under each narrowed sub-goal.
 *   - Frontier item 2 ("some OTHER real value may seat better than
 *     `idx = last & 7;`") is KILLED as a FAMILY: routing the OR result through
 *     idx instead (`idx = val | mask;`) also measures 2 / 78 with the identical
 *     two-insn shape one statement later (`or s0,v1,s2` / `sw s0,0(gp)` vs
 *     target's $v1).  Every value borrowed into idx inherits idx's callee-saved
 *     seat; the target computes all of them caller-saved.  Banked at
 *     rejected/or-result-routed-through-idx-same-callee-saved-seat-2of78.c.
 *   - The one C lever on the call-crossing (move `last = rand();` after the sum
 *     so idx dies before the call) works on the seat and destroys the schedule:
 *     11 / 78, because reorg.c then steals the sum into the jal delay slot.  The
 *     target's own stream proves ITS idx is live across that jal, so the
 *     call-crossing is a property of the target.  Banked at
 *     rejected/rand-moved-after-sum-steals-the-jal-delay-slot-11of78.c.
 *   - Also killed: `idx = idx * 3;` (the honest multiply spelling of the *3
 *     offset) is byte-identical to this body -- synth_mult + CSE reduce it to
 *     the same commutative PLUS with target == op1, so optabs.c:400-419 swaps
 *     identically.  Banked at
 *     rejected/mul3-strength-reduce-folds-to-the-same-addu-operand-order.c.
 * Disposition filed: the 2026-09-01 RESOLVED BY STANDING RULING (2026-07-27):
 * FORECLOSED entry in docs/grind/decisions.md (both endgame-lock gates re-run
 * and FAILED this session: scan_hand_coded tier=LOW 0/8; precedent census
 * NEGATIVE on the uncapped index at pin aa53500).
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
