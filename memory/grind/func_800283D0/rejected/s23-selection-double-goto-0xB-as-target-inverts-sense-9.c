/* candidate.c - func_800283D0 (saTan2KabutoWareMove), grind s22 2026-08-27 (structural)
 *
 * HONEST FLOOR WITH THIS BODY: sandbox --disable all = 6 / 216 insns
 * (s16-s21 body was 10 / 216).  FIRST FLOOR MOVE SINCE s16.
 *
 * WHAT THIS BODY IS.  It is s21's V1 chassis - i.e. the s16 body with target's
 * own `<` arm (`var_v0_4 = 0x19; if (var_s1 == 0) goto set_0xB; store;
 * goto do_calls;`), which E-s21-2 proved byte-closes the store sink (emitted
 * 83-88) and the arm store (emitted 148) - PLUS one `do { ... } while (0);`
 * wrap around the `==` arm's existing call pair.  Nothing else changed.
 *
 * WHY THE WRAP.  s21 reduced the whole $s2/$s3 seat swap (cluster A) to the
 * closed-form inequality pri(arg1-carrier) > pri(temp_s3) = 2142 under
 * global.c:635-656 allocno_compare, with exactly two solutions: raise
 * nrefs(carrier) to >= 8, or shorten livelen(carrier) to <= 65.  s21 measured
 * solution (b) (body P4) at a hard cap of 16 and killed every semantics-
 * preserving route to (a) it could find (P5, the duplicated `>` call pair,
 * collapses the tail to 81 / 183).  The do-while(0) wrap reaches solution (a)
 * directly and at zero emitted cost: flow.c increments REG_N_REFS by
 * `loop_depth`, which is 2 inside the NOTE_INSN_LOOP_BEG/END pair the wrap
 * leaves behind, so every reference sited inside the wrap counts twice.
 *
 * MEASURED, not inferred (tmp/grind/func_800283D0/s22/alloc_V1.txt vs
 * alloc_A3.txt, instrumented cc1 ALLOCDBG):
 *      V1 (no wrap):  73 arg1    nrefs=7 livelen=92  pri=1521 -> $s3  WRONG
 *                    143 temp_s3 nrefs=3 livelen=14  pri=2142 -> $s2  WRONG
 *      A3 (wrapped):  73 arg1    nrefs=9 livelen=92  pri=2934 -> $s2  TARGET
 *                    143 temp_s3 nrefs=3 livelen=14  pri=2142 -> $s3  TARGET
 * Every live length is unchanged and every other callee-saved seat
 * ($s0/$s1/$s4/$s5/$s6) is unchanged; only nrefs(arg0) 19->21 and
 * nrefs(arg1) 7->9 move, i.e. exactly the two pseudos referenced inside the
 * wrap, +1 per reference.  This is the same mechanism s4 measured in 2026-07
 * and (wrongly, see below) discarded.
 *
 * FAMILY / POLICY.  `do { <any body> } while (0);` is a SANCTIONED pure-C
 * match device for ANY codegen effect INCLUDING register allocation, with a
 * mandatory inline FAKE annotation - owner ruling 2026-07-06, recorded in
 * `.claude/rules/do-while-zero-exception.md` ("the former scoping to the
 * reorg.c label-note mechanism is abolished").  s4 killed this construct on
 * the ground that RA weighting was out of the carve-out's scope; that ground
 * was superseded by the 2026-07-06 ruling and s4's kill is hereby RETRACTED
 * as to policy (its measurements stand).  The wrap here is single-level (no
 * nesting justification needed), annotated at the construct site, and wraps
 * two REAL statements that are executed exactly once either way - it asserts
 * nothing false.
 *
 * RESIDUAL AT 6 (tmp/grind/func_800283D0/s22/diff_A3.txt; `lui at,0` entries
 * are unresolved-relocation artifacts of objdumping the .o, not diffs):
 *   1. emitted 45-48 - ours `beq / li v0,1 / j / nop`, target
 *      `beq / nop / j / li v0,1`.  Same insn count; reorg.c filled our
 *      conditional branch's slot from the fall-through thread and target
 *      filled the unconditional `j`'s slot instead.
 *   2. emitted 126 + 131 - ours `nop` in the `beqz v0` delay slot plus a
 *      standalone `li v0,1`; target steals the branch TARGET block's
 *      `li v0,1` (the value stored to D_800A38A8) into the slot.  This is
 *      cluster B and it is the WHOLE 216-vs-215 insn surplus.
 *   3. emitted 96 `addu s3,s0,v0` vs `addu s3,v0,s0` and emitted 162
 *      `addu a0,a0,s4` vs `addu a0,s4,a0` - commutative operand orders,
 *      re-confirmed score-neutral in either spelling this session (F2).
 * Cluster A, the store sink and the arm store are all CLOSED in this body.
 *
 * s22 KILLS (do not re-propose - full detail in evidence.md):
 *   - s20's W2b cluster-B hoist re-applied on THIS chassis (F1) = 8, i.e.
 *     the hoist family's +2 price is chassis-independent, exactly as s20's
 *     frontier item 3 asked.
 *   - Bare do-while(0) wraps placed to bend reorg.c (G1 around the
 *     D_800A38A8 store pair, G2/G3 around the 0x19/0xB selection, G5 around
 *     the range-check `return 1;`) are all byte-NEUTRAL: 6 / 216.
 *   - Re-spelling a branch's false edge as an explicit source `goto` into a
 *     label placed immediately AFTER a NOTE_INSN_LOOP_BEG - the shape that
 *     would make reorg.c's mostly_true_jump return 2 and fill from the
 *     target thread - is neutral for the `||` test (H4 = 6) and regressive
 *     for the range-check chain (H5 = 10 / 213, the `return 1` cross-jumps
 *     away).  Prediction is therefore NOT cluster B's lever; E-s18-8's
 *     resource refusal stands.
 *   - Wrapping only ONE of the pair (A1/A2/B1/C1) also flips the seats
 *     (nrefs 8, pri 2553) but scores 10: the loop note landing BETWEEN the
 *     two calls lengthens livelen(arg1) 92 -> 94 and perturbs the call
 *     block.  The wrap must contain BOTH calls.
 *   - Wrapping all three call pairs at once (D4) = 24; wrapping any two
 *     (D1/D2/D3) = 6, i.e. no better than the single minimal wrap.
 * kengo:MED  |  sa_tan2/saTan2KabutoWareMove  |  216i @ floor 6
 */
s32 func_800283D0(u8 *arg0, u8 *arg1) {
    s32 temp_a1;
    u8 *temp_s4;
    s32 temp_v1;
    s32 var_s1;
    s16 var_v0;
    s32 ret;

    temp_s4 = *(u8 **)(arg0);
    ret = 1;
    temp_a1 = *(u16 *)(arg0 + 0x6A);
    temp_v1 = temp_a1 & 0xFFFF;
    if (temp_v1 != 4) {
        if (temp_v1 != 0x14) {
            u16 temp_v0 = *(u16 *)(temp_s4 + 0x6A);
            if ((temp_v0 != 4) && (temp_v0 != 0x14)) {
                s32 d_val;
                s32 temp_a1_2;
                s32 temp_s5;

                if (temp_v1 != 0x13) {
                    if (((u32)(temp_a1 - 0x19) >= 2U) && (temp_v1 != 2) && (temp_v1 != 0x26) && (temp_v1 != 0x1B) && (temp_v1 != 0x15) && (temp_v1 != 0x25) && (temp_v1 != 0x2C) && (temp_v1 != 0xC)) {
                        return 1;
                    }
                    var_s1 = 0;
                    goto block_15;
                }
                var_s1 = 0;
            block_15:
                d_val = D_800A3824;
                temp_a1_2 = (d_val >> *(s16 *)(arg0 + 4)) & 1;
                temp_s5 = (d_val >> *(s16 *)(temp_s4 + 4)) & 1;
                if (*(s16 *)(arg0 + 0x8C) != 0) {
                    var_s1 = temp_a1_2 == 0;
                }
                if (var_s1 != 0) {
                    s16 temp_v1_2 = *(s16 *)(arg0 + 0xC);
                    if (temp_v1_2 != 0x1D) {
                        if (temp_v1_2 != 0xE) {
                            goto block_20;
                        }
                        return ret;
                    }
                    goto block_49;
                }
            block_20:
                {
                    s16 temp_v1_3 = *(s16 *)(temp_s4 + temp_s5 * 2 + 0x288);
                    if (temp_v1_3 == 0) {
                        if (*(s16 *)(arg0 + (temp_a1_2 * 2) + 0x288) > 0) {
                            var_v0 = 0x19;
                            if (var_s1 == 0) {
                            set_0xB:
                                var_v0 = 0xB;
                            }
                        do_store_calls:
                            *(s16 *)(arg0 + 0x286) = var_v0;
                        do_calls:
                            func_80032854(*(s16 *)(arg0 + 4), 1, arg1, (s16 *)0);
                            func_80032854(*(s16 *)(arg0 + 4), 0x25, arg1, (s16 *)0);
                            return ret;
                        }
                        goto block_49;
                    }
                    {
                        u8 *temp_s3 = arg0 + (temp_a1_2 * 2);
                        s16 temp_v0_3 = *(s16 *)(temp_s3 + 0x288);
                        s16 var_v0_2;
                        if (temp_v0_3 == temp_v1_3) {
                            do { /* FAKE: do-while(0) loop-note ref weighting, mechanism: flow.c REG_N_REFS += loop_depth feeding global.c allocno_compare, lever-exhaustion: memory/grind/func_800283D0/hypotheses.md */
                                func_80032854(*(s16 *)(arg0 + 4), 1, arg1, (s16 *)0);
                                func_80032854(*(s16 *)(arg0 + 4), 0x25, arg1, (s16 *)0);
                            } while (0);
                            if (*(s16 *)(temp_s3 + 0x288) == 5) {
                                if (((u32)(*(u16 *)(arg0 + 0xE) - 6) < 2U) || ((u32)(*(u16 *)(temp_s4 + 0xE) - 6) < 2U)) {
                                    if (var_s1 == 0) {
                                        goto sel0B;
                                    }
                                    var_v0_2 = 0x19;
                                    goto block_48;
                                sel0B:
                                    var_v0_2 = 0xB;
                                    goto block_48;
                                }
                                D_800A38A8 = 1;
                                D_800A3876 = -1;
                                goto block_49;
                            }
                            {
                                s32 sel = (var_s1 == 0) ? 0xB : 0x19;
                                var_v0_2 = sel;
                            }
                            goto block_48;
                        }
                        if (temp_v1_3 < temp_v0_3) {
                            s16 var_v0_4 = 0x19;
                            if (var_s1 == 0) {
                                goto set_0xB;
                            }
                            *(s16 *)(arg0 + 0x286) = var_v0_4;
                            goto do_calls;
                        }
                        func_80032854(*(s16 *)(arg0 + 4), 0x26, arg1, (s16 *)0);
                        func_80032854(*(s16 *)(arg0 + 4), 0x2D, arg1, (s16 *)0);
                        var_v0_2 = 0x1A;
                        if (var_s1 == 0) {
                            s32 temp_v1_4 = -*(s16 *)(arg0 + 0x1CA);
                            s32 temp_v1_5 = (s32)((&Judge)[((temp_v1_4 + 0x400) & 0xFFF)] * *(s32 *)(temp_s4 + (temp_s5 * 0x10) + 0x114) + (&Judge)[(temp_v1_4 & 0xFFF)] * *(s32 *)(temp_s4 + (temp_s5 * 0x10) + 0x11C)) >> 0xC;
                            s32 temp_a0_2 = *(s32 *)(temp_s4 + (temp_s5 * 0x10) + 0x118);
                            s32 var_a1 = temp_a0_2;
                            s32 var_v0_3;
                            if (temp_a0_2 < 0) {
                                var_a1 = -temp_a0_2;
                            }
                            var_v0_3 = temp_v1_5;
                            if (temp_v1_5 < 0) {
                                var_v0_3 = -temp_v1_5;
                            }
                            if (var_v0_3 < var_a1) {
                                var_v0_2 = 0x14;
                                if (temp_a0_2 > 0) {
                                    var_v0_2 = 0x13;
                                }
                            } else {
                                var_v0_2 = 0x15;
                                if (temp_v1_5 <= 0) {
                                    var_v0_2 = 0x16;
                                }
                            }
                        }
                    block_48:
                        *(s16 *)(arg0 + 0x286) = var_v0_2;
                    }
                }
            block_49:
                return ret;
            }
            goto block_13;
        }
        return ret;
    }
block_13:
    return ret;
}
