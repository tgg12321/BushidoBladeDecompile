/* REJECTED (session 5, permuter modality, 2026-08-12) — NOT proposed.
 *
 * decomp-permuter find `tmp/grind/func_800645B0/s5/ws3/output-0-2` on the CA
 * chassis: permuter score 0, i.e. it would very likely also sandbox at 0.  It
 * wraps the inner-loop-top slot-index assignment in a bare do-while(0):
 *
 *     do { idx = i + j; } while (0);
 *
 * WHY IT IS DEAD (policy, not measurement).  `do { ... } while (0);` is the ONE
 * sanctioned no-semantic-purpose wrapper in this tree, but the carve-out is
 * explicitly narrow: it applies ONLY to the LABEL_OUTSIDE_LOOP_P / reorg.c
 * `relax_delay_slots` invert-jump interaction, and only as a last resort with
 * documented lever-exhaustion plus a `/* FAKE */` annotation
 * (.claude/rules/no-new-park-categories.md, do-while-zero-exception).  Here the
 * mechanism is NOT that interaction — it is the NOTE_INSN_LOOP_BEG boundary
 * changing where cc1's first-pass scheduler may move the const-1 set relative
 * to the index `addu`, i.e. a scheduling-tie steer.  That is the same intent as
 * this function's already-BANNED construct (the relocated `j += 1;` chosen to
 * flip which of `addu $s0,$s3,$a0` / `li $v1,1` the scheduler and reorg.c pick
 * first), just spelled with a wrapper instead of a statement move.  A banned
 * construct respelled is the same construct.
 *
 * The sibling score-0 find from the same campaign (`output-0-1`, the
 * `wid = i + j; idx = wid;` staging chain) is a sanctioned-family construct and
 * is the session's candidate; see memory/grind/func_800645B0/candidate.c and
 * self_vet.md.  This one is banked so no future session re-proposes it.
 *
 * Body as the permuter emitted it (rest of the function identical to the CA
 * chassis, i.e. candidate.c with `idx = i + j;` at the loop top and no staging):
 */
#if 0
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            do { idx = i + j; } while (0);
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                /* ... three word stores through wid, s16 store through idx2,
                   occupancy OR, break — as candidate.c ... */
            }
        }
    }
#endif
