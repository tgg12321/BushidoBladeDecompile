/* REJECTED (s5, chassis C variant 2) — per-arm duplicated `D_800A34F8` read
 * instead of the shared `a0 = D_800A34F8;` pre-branch read.
 *
 * Intent: [[split-read-defeats-hoist]] (SOTN-sanctioned "duplicate the read into
 * the branch arms"). If each arm reads the global itself, the two mask operands
 * are distinct pseudos, so CSE cannot fold the then-arm and else-arm
 * `lui 0xFFFF` births into one — which is exactly the residual (target has TWO
 * `lui 0xFFFF` per case; our build shares ONE, filled fall-through into the
 * branch delay slot).
 *
 * Measured: permuter directed sweep s5c, combination score 235 == base (no
 * improvement) — GCC's CSE still folds the repeated non-volatile global loads
 * (the arm's own store to D_800A34F8 happens AFTER both reads in every arm, so
 * nothing invalidates the load), and the lui births re-merge exactly as before.
 * Artifacts: tmp/grind/func_8006B92C/s5/chassisC_base.c + chassisC_scores.txt.
 */
    case 1:
        if ((D_800A34F8 & 0xE000) == 0x4000) {
            D_800A34F8 = D_800A34F8 & 0xFFFF1FFF;
        } else {
            var_v1 = D_800A34F8 & 0xFFFF1FFF;
            var_v0 = ((D_800A34F8 >> 13) & 7) + 1;
            goto complete_store;
        }
        goto do_call;
    case 2:
        if ((D_800A34F8 & 0xE000) == 0) {
            D_800A34F8 = (D_800A34F8 & 0xFFFF1FFF) | 0x4000;
        } else {
            var_v1 = D_800A34F8 & 0xFFFF1FFF;
            var_v0 = ((D_800A34F8 >> 13) & 7) - 1;
        complete_store:
            var_v1 |= ((var_v0 & 7) << 13);
            D_800A34F8 = var_v1;
        }
