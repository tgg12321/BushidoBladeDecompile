/* REJECTED (s5, chassis C variant 3) — hoist the mask compute ABOVE the if, so
 * both arms consume one shared `var_v1 = a0 & 0xFFFF1FFF;`.
 *
 * Intent: [[hoist-shared-arm-computation-defeats-copy-pref]]. Both arms of each
 * case genuinely need `a0 & 0xFFFF1FFF` (the then-arm stores it directly, the
 * else-arm ORs the new field into it), so the hoist is an ordinary CSE-shaped
 * rewrite, not a coercion. Hope was that a single pre-branch mask birth removes
 * the then-arm's fall-through `lui $v0` fill candidate, leaving reorg.c's
 * fill_slots_from_thread nothing to fill from the fall-through thread and
 * forcing a dead-branch fill from the else thread (target's shape).
 *
 * Measured: permuter directed sweep s5c, combination score 285 (WORSE than the
 * 235 base). The pre-branch birth simply moves the lui/ori/and out of both arms
 * and the delay slot is then filled by the compare's own `andi`, losing the
 * per-arm mask compute that h2a's floor-6 depends on.
 * Artifacts: tmp/grind/func_8006B92C/s5/chassisC_base.c + chassisC_scores.txt.
 */
    case 1:
        a0 = D_800A34F8;
        var_v1 = a0 & 0xFFFF1FFF;
        if ((a0 & 0xE000) == 0x4000) {
            D_800A34F8 = var_v1;
        } else {
            var_v0 = ((a0 >> 13) & 7) + 1;
            goto complete_store;
        }
        goto do_call;
    case 2:
        a0 = D_800A34F8;
        var_v1 = a0 & 0xFFFF1FFF;
        if ((a0 & 0xE000) == 0) {
            D_800A34F8 = var_v1 | 0x4000;
        } else {
            var_v0 = ((a0 >> 13) & 7) - 1;
        complete_store:
            var_v1 |= ((var_v0 & 7) << 13);
            D_800A34F8 = var_v1;
        }
