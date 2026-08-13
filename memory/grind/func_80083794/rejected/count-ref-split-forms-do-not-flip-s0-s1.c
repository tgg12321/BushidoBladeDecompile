/* func_80083794 — REJECTED: the counter-REF-SPLIT family (session 5, permuter).
 *
 * WHY THIS FILE EXISTS. Sessions 2 and 4 attacked residual class B (target has
 * $s0 = ctor pointer `p`, $s1 = `count`; we get the reverse) from the "raise
 * p's ref count" side and killed it. Session 5's isolated-objective permuter
 * campaign (ws3/ws4, target normalized so the proven-unreachable classes A and
 * C stop drowning the signal) surfaced the OTHER side of the same lever for
 * the first time in five sessions: LOWER `count`'s ref count by splitting its
 * references across two pseudos. tmp/grind/func_80083794/s5/ws4/output-478-1
 * is the first form ever produced for this function whose loop reads through
 * $s0 (`lw v0,0(s0)` / `beqz s1` / `bnez s1`) — i.e. target's register roles.
 *
 * ...but it is SEMANTICS-BROKEN, and that is the whole finding. It reads:
 *
 *     s32 new_var; s32 count; void (**p)(void);
 *     count = (s32)&D_00000000;
 *     if (D_800A2668 == 0) {
 *         do { } while (0);
 *         D_800A2668 = 1;
 *         new_var = count;
 *         p = &D_8008D070;
 *         while (new_var != 0) { (*p++)(); count--; }   <-- INFINITE LOOP
 *     }
 *
 * The loop TESTS `new_var` and decrements `count`, so `count--` is dead, flow
 * deletes it, and the counter pseudo loses its two in-loop references. THAT is
 * the only reason the allocno priority inverts and `p` wins $s0. The permuter
 * does not preserve semantics; this form loops forever on a non-empty ctor
 * table. Its emitted body is also missing the target's `addiu $s1,$s1,-0x1`
 * (position 20) entirely and carries a load-delay `nop` at position 18.
 *
 * WHAT WAS MEASURED (honest sandbox, `--disable all`, via
 * tmp/grind/func_80083794/s5/sweep.py + variants.json; raw JSON in
 * s5/sweep_out.json). Six SEMANTICS-PRESERVING spellings of the same ref-split
 * intent, i.e. every way to give the loop's test variable and the initialised
 * variable separate identities without changing what the function does:
 *
 *   variant                        score  insns  p_reg  temp  frame
 *   x1_guard_count_loop_copy         23     28    s1     v0    -32
 *   x2_plain_copy_rename             18     28    s1     v0    -32
 *   x3_guard_count_endptr_loop       22     29     ?      ?    -32
 *   x4_copy_before_p                 22     28    s1     v0    -32
 *   x5_guard_copy_while              24     30    s1     v0    -32
 *   x6_countdown_predec_guarded      18     28    s1     v0    -32
 *
 * `p` is in $s1 in EVERY one; the temp is $v0 in every one; the frame is -32 in
 * every one. Nothing beats the score-18 floor. The mechanism is that GCC's
 * copy-propagation folds `n = count;` back into a single pseudo, so a
 * semantics-preserving "split" is not a split at all — the references still
 * belong to one allocno. The ONLY way to actually remove the two in-loop
 * references is to stop the loop from decrementing the variable it tests, which
 * is precisely the semantic break.
 *
 * CONCLUSION. Residual class B is not merely unfound: within this compiler's
 * allocno-priority rule the target's assignment requires the counter to carry
 * FEWER loop-weighted references than the pointer, and every reference the
 * counter carries (init, guard, decrement, back-branch) is semantically
 * load-bearing in a correct implementation. Target has all four of those
 * references AND the opposite assignment.
 */

/* --- the six measured semantics-preserving forms ------------------------- */

/* x1_guard_count_loop_copy — score 23 */
void x1(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        count = (s32)&D_00000000;
        p = &D_8008D070;
        if (count != 0) {
            s32 n = count;
            do {
                (*p++)();
            } while (--n);
        }
    }
}

/* x2_plain_copy_rename — score 18 (ties the floor; copy folded away) */
void x2(void) {
    s32 count;
    s32 n;
    void (**p)(void);

    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        count = (s32)&D_00000000;
        n = count;
        p = &D_8008D070;
        while (n != 0) {
            (*p++)();
            n--;
        }
    }
}

/* x3_guard_count_endptr_loop — score 22, and 29 emitted instructions */
void x3(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        count = (s32)&D_00000000;
        p = &D_8008D070;
        if (count != 0) {
            void (**end)(void) = p + count;
            do {
                (*p++)();
            } while (p != end);
        }
    }
}

/* x4_copy_before_p — score 22 */
void x4(void) {
    s32 count;
    s32 n;
    void (**p)(void);

    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        count = (s32)&D_00000000;
        p = &D_8008D070;
        n = count;
        while (n != 0) {
            (*p++)();
            n--;
        }
    }
}

/* x5_guard_copy_while — score 24, 30 emitted instructions */
void x5(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        count = (s32)&D_00000000;
        p = &D_8008D070;
        if (count != 0) {
            s32 n = count;
            while (n != 0) {
                (*p++)();
                n--;
            }
        }
    }
}

/* x6_countdown_predec_guarded — score 18 (ties the floor) */
void x6(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        count = (s32)&D_00000000;
        p = &D_8008D070;
        if (count != 0) {
            do {
                (*p++)();
            } while (--count);
        }
    }
}
