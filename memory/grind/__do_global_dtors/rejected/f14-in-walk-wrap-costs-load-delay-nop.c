/*
 * REJECTED (session 12, structural) — the IN-WALK do-while(0) wrap with the
 * ORIGINAL statement order. Score 15, build_insns 26 (the accepted form scores
 * 13 at 25). Kept because it is the exact trap that makes the placement axis
 * look dead when it is not.
 *
 * WHAT IT GETS RIGHT. The wrap around `p++` weights TWO of p's four raw
 * references instead of the initialiser's one, so p reaches 6 weighted refs /
 * live_length 8 = 15000 at a SINGLE wrap level and takes $s0 ahead of count's
 * unchanged 5/7 = 14285. The register roles — the thing that used to need two
 * nested levels — are won at depth 1 (s12 f14sweep.py W1_pinc_d1,
 * f14bsweep.py X0_control_d1).
 *
 * WHY IT LOSES TWO POINTS ANYWAY. With `f()` still ahead of `count--`, the
 * NOTE_INSN_LOOP_BEG/END pair fences `addu $16,$16,4` out of its position
 * between the load and the call. The scheduler then fills the jal delay slot
 * with `addu $16,$16,4` instead of `addu $17,$17,-1`, which leaves
 * `lw $2,0($16)` feeding `jal $31,$2` back-to-back, and maspsx inserts a
 * load-delay nop — a 26th instruction the target does not have in that place.
 * Emitted loop, this form:   lw / jal (delay: addu $16) / addu $17 / bne
 * Emitted loop, accepted:    lw / addu $16 / jal (delay: addu $17) / bne
 * The target's is the second. Moving `count--` ahead of `f()` — measured FREE
 * on this chassis in s11 F13 cell V7 — gives the scheduler a second delay-slot
 * candidate and restores the target's loop schedule at 25 instructions, 13.
 *
 * THE FENCE IS THE NOTE, NOT THE WRAPPED STATEMENT. s12 cell X6 puts an EMPTY
 * `do { } while (0);` in the same in-walk position: it adds no reference to
 * either allocno, wins no register (score 21, the ladder's un-flipped value),
 * and STILL emits 26 instructions. So any in-walk note perturbs the loop
 * schedule; whether that costs anything depends entirely on the statement
 * order it is embedded in.
 *
 * ALSO REJECTED IN THE SAME FAMILY (all 15 / 26 insns, all winning the roles):
 *   - wrap enclosing the bump AND the call (X2) — 7 weighted refs, same nop
 *   - wrap on the bump moved AFTER the call (X4) — 6 refs, same nop
 *   - wrap on `f = *p;` at depth 2 (f14 W2_pread_d2) — 6 refs, same nop
 *   - wrap on the read+bump pair with the original order (f14 W3_pair_d1/d2,
 *     W6_split_d1/d2) — 7 and 10 refs, same nop
 * And two directional controls, both 20 at every depth, which are what make the
 * per-reference weighting a measurement rather than a story:
 *   - wrap on the WHOLE walk body (f14 W4) lifts BOTH allocnos (p +3, count +2
 *     per level: 7 vs 7 at d1, 10 vs 9 at d2) and never changes the ordering —
 *     session 4's nested-wrap result reproduced at a new placement
 *   - wrap on count's initialiser (f14 W5) lifts count only (5 / 6 / 7 refs),
 *     i.e. the wrong allocno, and p never moves
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        p = &D_8008D070;
        count = (s32)&D_00000000;
        if (count != 0) {
        again:
            {
                void (*f)(void) = *p;
                do { p++; } while (0);   /* wins $s0 at depth 1 — but costs a
                                            load-delay nop in this order */
                f();
                count--;
            }
            if (count != 0) {
                goto again;
            }
        }
    }
}
