/* s6 P6c — REJECTED (score 6 -> 19, build_insns 141 -> 139).
 *
 * Change vs h2a: compute the counter unconditionally BEFORE the if, so its
 * (global) pseudo var_v0 is live across the branch and across the else arm's
 * mask compute, hoping to occupy $v0 over the constant's live range and push
 * the constant to $v1. Semantically harmless (var_v0 is only read on the else
 * path).
 *
 * Measured dead: global.c does not honour var_v0's $v0 preference under the
 * longer live range — the pre-branch block already needs $v0 for the case-1
 * compare constant (`li v0,0x4000`), so var_v0 is allocated $a1 instead
 * (`addiu a1,v0,1`), $v0 is free again by the else block, and the mask
 * constant lands in $v0 exactly as before. Worse still, with the counter
 * hoisted the else arms become pure mask+goto, so reorg fills the delay slot
 * with the shared `lui $v0` again and BOTH cases lose their then-arm lui
 * (139 insns, 4 short).
 *
 * Conclusion: extending a GLOBAL pseudo's live range across the branch does
 * not block $v0 for the block-local constant — global.c runs AFTER
 * local-alloc.c, so a global pseudo can never reserve a register against a
 * local one. Only a value that is BLOCK-LOCAL and live across the constant's
 * range can do it, and sched1 prevents any such range from existing (see
 * s6_perarm_local_temps.c).
 */
    case 1:
        a0 = D_800A34F8;
        var_v0 = ((a0 >> 13) & 7) + 1;      /* hoisted out of the else arm */
        if ((a0 & 0xE000) == 0x4000) {
            D_800A34F8 = a0 & 0xFFFF1FFF;
        } else {
            var_v1 = a0 & 0xFFFF1FFF;
            goto complete_store;
        }
        goto do_call;
