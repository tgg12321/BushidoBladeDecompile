/* candidate.c - func_800283D0 (saTan2KabutoWareMove), grind s9 2026-08-26 (forensics)
 *
 * HONEST FLOOR WITH THIS BODY: sandbox --disable all = 23  (was 25 at s8).
 * build_insns 216 vs target 215 - see the CAVEAT below; this is the first body
 * in the ledger that is NOT at 215==215, and it is banked anyway because the
 * honest floor is the score and 23 < 25.  The 215-insn alternative that scores
 * 25 is kept beside it as candidate_alt_215insn.c.
 *
 * The body is the s8 body with TWO changes, both plain ordinary C.
 *
 * (1) TAIL SELECTION now reads
 *         { s32 sel = (var_s1 == 0) ? 0xB : 0x19; var_v0_2 = sel; }
 *     instead of s8's multi-write
 *         { s32 sel = 0x19; if (var_s1 == 0) sel = 0xB; var_v0_2 = sel; }
 *     It emits a stream BYTE-IDENTICAL to s8's (verified slot-for-slot), i.e.
 *     it is exactly as good, but the intermediate is now ONCE-WRITTEN and
 *     ONCE-READ.  That was the whole classification blocker s8 left open: the
 *     multi-write spelling fitted no frozen SOTN family, while a fresh,
 *     once-written / once-read, int-typed local carrying a real consumed value
 *     is the named-intermediate SHAPE (family-selection table, row 2).  s9 did
 *     NOT complete a self-vet for it (the floor is not 0, so the question is
 *     not yet live) - but the form to vet is now this one, not s8's.
 *     Mechanism unchanged from s8: jump.c find_cross_jump compares patterns
 *     with rtx_renumbered_equal_p, which compares GET_MODE; the SImode
 *     constant loads PAT-MISMATCH against the ==5 copy's HImode ones on the
 *     first compared insn, so both selection copies survive with target's
 *     canonical `bnez $s1 / li 0x19 / j / li 0xB` order.
 *     NOTE: the ternary ARM ORDER is load-bearing.  `(var_s1 != 0) ? 0x19 :
 *     0xB` also defeats the merge (215 insns) but emits `beqz / li 0xB / j /
 *     li 0x19` - the inverted sense - and measures 28.  Only the
 *     `(var_s1 == 0) ? 0xB : 0x19` arm order reproduces target.
 *
 * (2) The D_800A38A8 / D_800A3876 block now exits with `goto block_49;`
 *     instead of an inline `return ret;`.  25 -> 23, and it CLOSES CLUSTER C
 *     (the 4-diff v0-vs-v1 rename at emitted slots 126/132/133/135).
 *     Attribution, read out of the .combine / .flow / .sched / .lreg / .greg
 *     dumps of this TU (tmp/grind/func_800283D0/dumps/):
 *       - Pre-sched1 (.combine:2894-2918, .flow:4370-4394) that block reads
 *         368 (set (reg:HI 170) 1) -> 370 (sh D_800A38A8) ->
 *         373 (set (reg:HI 171) -1) -> 375 (sh D_800A3876) ->
 *         378 (set (reg/i:SI 2 v0) (reg/v:SI 79)).
 *       - sched1 (.sched:4853) HOISTS insn 378 to the FRONT of the block:
 *         378, 368, 370, 373, 375.
 *       - Consequence: hard reg v0 is live from the top of the block, so
 *         local-alloc cannot give it to pseudos 170/171 and they take v1
 *         (.greg:3150 `(set (reg:HI 3 v1) (const_int 1))`).  Target has no
 *         such copy at the head of that block, so its constants take v0.
 *       - `goto block_49;` deletes the block-local return-value copy entirely,
 *         v0 is free, and slots 131-136 now read
 *         `li v0,1 / lui at / sh v0 / li v0,-1 / lui at / sh v0` = target.
 *
 * THE CAVEAT (the whole remaining cost of change 2, and it is ONE
 * instruction): with the copy gone, reorg no longer fills the delay slot of
 * the `beqz v0,<globals block>` at slot 125.  In the 25-floor body reorg stole
 * `li v1,1` from the branch target into that slot; in this body the same steal
 * would place `li v0,1` there - a register the branch itself reads - and reorg
 * declines, emitting a `nop` at slot 126 and leaving the `li v0,1` at 131.
 * Target DOES make exactly that steal (`beqz v0,.L800285DC` with
 * `addiu v0,zero,1` in the delay slot), so it is reachable; the reason ours
 * declines has not been read out of reorg.c yet and is frontier item 1 for
 * session 10.  Kill the nop and this body is 215 insns with cluster C closed.
 *
 * MEASURED AND DEAD THIS SESSION (all banked under rejected/):
 *   - `var_v0_2 = <ternary>;` with no intermediate: 30 / 211 - HImode, merges.
 *   - Declaring var_v0_2 as s32 (both copies SImode): 30 / 211 - merges.
 *   - Both copies canonical if/else: 30 / 211 - the s2 baseline, merges.
 *   - Storing to the s16 field directly in the ==5 copy (fresh s16 local, or
 *     the existing var_v0): 30 / 211.  An instrumented BB2_XJUMP_DEBUG trace
 *     shows the merge STILL fires (e1=372 vs e2=407, three MATCHes plus a
 *     LABEL-BONUS) because an earlier jump pass re-merges the duplicated
 *     store+return tail back into block_48 - the STRUCTURAL route to defeating
 *     the cross-jump is closed, the mode split is the only lever.
 *   - Same trick in the tail copy: 29 / 211.
 *   - Swapping the two global stores: 24 (worse; target stores 1 first).
 *   - `return 1;` in the globals block: 26.
 *   - Turning the `temp_v1_2 == 0xE` inline return into `goto block_49;`: 38.
 *
 * SYMMETRY NOTE: putting the s32 intermediate in the ==5 copy instead of the
 * tail copy emits a byte-identical stream.  Either placement works; the tail
 * placement is kept.  Likewise `goto block_49;` on the do_calls return as well
 * measures the same 23.
 *
 * Remaining residual at 23: cluster A (s2/s3 callee-saved rotation, ~10 slots,
 * s7 window L73 92->[38,65] / L143 14->[20,21]), cluster B (slots 45/47
 * nop-vs-li v0,1 exit-block sharing), cluster E (a0/a1 at slots
 * 159/161/168/171, s7 window span(qty0) 30 -> <=24), plus the one unfilled
 * delay slot described above.
 *
 * S10 (rederive) RE-CONFIRMED THIS BODY AT 23 / 216 AND CLOSED TWO AXES:
 *   - A fresh m2c re-derivation differs from this body in exactly five structural
 *     choices and ALL FIVE are regressive on the 23-floor chassis (30 / 29 / 49 /
 *     25 / 33; all five at once = 65).  There is no matched sibling body in the
 *     tree to transplant from either.  The m2c/transplant route is spent.
 *   - The exit-form dimension over the four early-exit sites is now fully
 *     enumerated and 23 is its minimum; cluster B does not move under it.
 *   - The `||` guard spelling is pinned: de Morgan regresses (31 and 27).
 *   - Frontier item 1 re-attributed with BB2_DBR_DEBUG measurements: reorg
 *     declines the steal because `mark_target_live_regs` reports v0 LIVE at the
 *     FALL-THROUGH (oppregs=0x20630084), not because the branch reads v0.  See
 *     evidence.md s10 section 4 for the exact log lines and the open sub-question.
 *
 * kengo:MED  |  sa_tan2/saTan2KabutoWareMove  |  216i @ floor 23
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
                            func_80032854(*(s16 *)(arg0 + 4), 1, arg1, (s16 *)0);
                            func_80032854(*(s16 *)(arg0 + 4), 0x25, arg1, (s16 *)0);
                            if (*(s16 *)(temp_s3 + 0x288) == 5) {
                                if (((u32)(*(u16 *)(arg0 + 0xE) - 6) < 2U) || ((u32)(*(u16 *)(temp_s4 + 0xE) - 6) < 2U)) {
                                    var_v0_2 = 0x19;
                                    if (var_s1 == 0) {
                                        var_v0_2 = 0xB;
                                    }
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
                            if (var_s1 != 0) {
                                *(s16 *)(arg0 + 0x286) = 0x19;
                            } else {
                                goto set_0xB;
                            }
                            goto do_calls;
                        }
                        func_80032854(*(s16 *)(arg0 + 4), 0x26, arg1, (s16 *)0);
                        func_80032854(*(s16 *)(arg0 + 4), 0x2D, arg1, (s16 *)0);
                        var_v0_2 = 0x1A;
                        if (var_s1 == 0) {
                            u8 *temp_a0 = temp_s4 + (temp_s5 * 0x10);
                            s32 temp_v1_4 = -*(s16 *)(arg0 + 0x1CA);
                            s32 temp_a0_2 = *(s32 *)(temp_a0 + 0x118);
                            s32 var_a1 = temp_a0_2;
                            s32 temp_v1_5 = (s32)((&Judge)[((temp_v1_4 + 0x400) & 0xFFF)] * *(s32 *)(temp_a0 + 0x114) + (&Judge)[(temp_v1_4 & 0xFFF)] * *(s32 *)(temp_a0 + 0x11C)) >> 0xC;
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
/* kengo:MED  |  sa_tan2/saTan2KabutoWareMove  |  215i */
