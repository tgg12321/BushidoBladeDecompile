/* candidate.c - func_800283D0 (saTan2KabutoWareMove), grind s11 2026-08-26 (rederive)
 *
 * HONEST FLOOR WITH THIS BODY: sandbox --disable all = 20  (was 23 at s9/s10).
 * build_insns 212 vs target 215 - three instructions SHORT, all three in one
 * place (see THE CAVEAT).  Banked anyway because the honest floor is the score.
 *
 * THE ONE CHANGE vs the s9/s10 body: the `temp_v1_3 < temp_v0_3` arm no longer
 * reaches the shared call pair with `goto do_calls;`.  It writes its own
 * selection, its own store and its OWN COPY of the two func_80032854 calls and
 * returns.  Plain ordinary C - a branch arm spelled out in full instead of
 * jumping into a neighbouring arm's tail.  No new locals beyond the arm-local
 * `s16 var_v0_4`, nothing dead, nothing annotated.
 *
 * WHY IT WORKS - CLUSTER A (the $s2/$s3 callee-saved rotation) IS NOW CLOSED.
 * This is not a guess; it is the dial s7's RA-solver inverse named, driven to
 * the value the inverse predicted, and confirmed in the model afterwards:
 *   - s7 (re-measured UNCHANGED on the 23-floor body this session, see
 *     evidence.md s11 section 2) proved the callee-saved map can only be
 *     attacked through two allocnos: pseudo 73 = `arg1` and pseudo 143 =
 *     `temp_s3`.  The single-atom windows are livelen(73) 92 -> [38,65],
 *     nrefs(73) 7 -> [8,9,10,11], livelen(143) 14 -> [20,21], nrefs(143) 3 -> 2.
 *   - global.c allocno_compare ranks by
 *       pri = floor_log2(nrefs)*nrefs / allocno_live_length * 10000 * size.
 *     On the 23-floor body that is pri(143) = 3/14 -> 2142 versus
 *     pri(73) = 14/92 -> 1521, so 143 sorted FIRST and took $s2.  It is a clean
 *     ~40% inversion, NOT a tie.
 *   - Duplicating the two calls gives `arg1` two more RTL references at
 *     global-alloc time.  Measured after the change: nrefs_flow(73) 7 -> 9,
 *     which lands inside the [8,11] window; the ra_solver forward model now
 *     reports `base meets goal? True` and the emitted prologue is target's
 *     (`sw $s2,0x20($sp) / addu $s2,$a1,$zero ... sw $s3,0x24($sp)`), with
 *     every one of the ten $s2<->$s3 slots (4, 85, 90, 96, 97, 102, 107, 111,
 *     148, 153) now correct.
 *   - The duplication is legitimate precisely BECAUSE it survives to
 *     global-alloc and is then undone: jump.c cross-jumping runs in jump2,
 *     AFTER reload, so the source-level copy is real RTL while the allocator
 *     sorts and is re-merged before the bytes are emitted.
 *
 * THE CAVEAT (all three missing instructions, one cause): jump2's
 * find_cross_jump merges the whole `<` arm away, leaving a single
 * `bnez $v0,<shared block>`, where target keeps a FOUR-instruction arm
 * (`beqz $s1,.L80028518 / addiu $v0,0x19 / j .L80028520 / sh $v0,0x286($s0)`)
 * that jumps into the shared block at two different points.  Target's arm
 * survives because its inner branch has the INVERTED sense relative to the
 * shared block's (`beqz $s1` vs `bnez $s1`), which stops the common-tail walk.
 * The s9 body got that shape for free from the `goto set_0xB;` spelling.
 * Two spellings that keep the inverted sense were measured this session and
 * both score the same 20 but with WORSE instruction counts, because the store
 * then sinks into the jal delay slot in the shared block:
 *     R6 `goto set_0xB` + duplicated calls .......... 20 / 219
 *     R7 duplicate in the 0xB path, `goto do_calls`
 *        kept alive for the 0x19 path ............... 20 / 222
 * The score is flat at 20 across all three, so the choice is free; the 212-insn
 * spelling is banked as the compact one.  Closing the last 3 instructions means
 * finding a spelling that duplicates the calls (for the nrefs dial) AND keeps
 * BOTH the inverted inner branch AND a live label between the store and the
 * arg set-up (which is what pins the store in front of `li $a1,1`).
 *
 * MEASURED AND DEAD THIS SESSION:
 *   - R1 sibling-idiom dispatch (the eight-way state test respelled as
 *     positive `if (x == K) goto set_s1;` tests with a fall-through
 *     `return 1;`, exactly func_8002798C's idiom - a MATCHED sibling in this
 *     same file): 20/216-equivalent, i.e. score-NEUTRAL at 23 on the s10 body.
 *     Free either way; not a lever, but the author-idiomatic spelling is now
 *     known to cost nothing.  Kept as variants/R1_sibling_dispatch.c.
 *   - R2 `(temp_a1_2 * 2) + arg0` instead of `arg0 + (temp_a1_2 * 2)` to chase
 *     target's `addu $s3,$v0,$s0` operand order: score-neutral, GCC
 *     canonicalises.  The operand order is a CONSEQUENCE of the register
 *     assignment, not an independent diff.
 *   - R3 hoisting `temp_s3` above the `temp_v1_3 == 0` test so both reads share
 *     it (a plausible original shape, and it also targets the livelen(143)
 *     dial): 59 / 215.  Badly regressive - it adds a fourth reference, and the
 *     model needs livelen(143) in [53,58] at nrefs 4.
 *
 * Remaining residual at 20: cluster B (the nop-vs-`li v0,1` delay-slot fill at
 * emitted slots 45/47), cluster E (the $a0/$a1 exchange in the tail Judge
 * product, a LOCAL-alloc seat - s7 window span(qty0) 30 -> <=24 or refs(qty0)
 * 6 -> 8..10), the store-vs-`move a3,zero` order in the shared call block, and
 * the 3 instructions of the over-merged `<` arm described above.
 *
 * kengo:MED  |  sa_tan2/saTan2KabutoWareMove  |  212i @ floor 20
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
                        u8 *temp_s3 = (temp_a1_2 * 2) + arg0;
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
                            s16 var_v0_4 = 0xB;
                            if (var_s1 != 0) {
                                var_v0_4 = 0x19;
                            }
                            *(s16 *)(arg0 + 0x286) = var_v0_4;
                            func_80032854(*(s16 *)(arg0 + 4), 1, arg1, (s16 *)0);
                            func_80032854(*(s16 *)(arg0 + 4), 0x25, arg1, (s16 *)0);
                            return ret;
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
