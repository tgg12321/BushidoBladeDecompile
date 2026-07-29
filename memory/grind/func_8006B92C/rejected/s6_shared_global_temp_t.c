/* s6 P6a — REJECTED (score 6 -> 12, but build_insns 141 -> 143 == target).
 *
 * Change vs the s3/s5 h2a candidate: ONE function-scope temp `t` used in BOTH
 * else arms, holding the shift subexpression, written before the mask:
 *     else { t = (a0 >> 13) & 7; var_v1 = a0 & 0xFFFF1FFF; var_v0 = t +/- 1; }
 *
 * Because `t` is referenced from two basic blocks it becomes a GLOBAL pseudo
 * (global.c allocates it, landing in $a1), and the else block's leading insn
 * after sched1 is the `srl` rather than the mask's `lui`. reorg.c then fills
 * the bne/bnez delay slot with `srl $v0,$a0,0xd` instead of a `lui`, so the
 * then-arm's `lui $v0` is no longer redundant with the delay-slot insn and
 * survives: instruction COUNT reaches target's 143 for the first time in this
 * function's history.
 *
 * Rejected because the register picture diverges further than h2a: the counter
 * chain is split across $a1 (`andi a1,v0,7` / `addiu v0,a1,1`) where target
 * keeps it entirely in $v0, and the delay slot carries the srl where target
 * carries `lui $v1`. Honest distance 12 vs 6.
 *
 * Kept as evidence: it proves the 2-insn shortfall is NOT a fill-priority
 * problem — the count closes as soon as the else block does not open with an
 * insn that duplicates the then-arm's constant in the SAME hard register.
 */
        } else {
            t = (a0 >> 13) & 7;             /* s32 t; declared at fn scope */
            var_v1 = a0 & 0xFFFF1FFF;
            var_v0 = t + 1;                 /* case 2: t - 1 */
            goto complete_store;
        }
