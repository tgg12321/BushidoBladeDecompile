/* candidate.c - func_800283D0 (saTan2KabutoWareMove), grind s14 2026-08-26 (synthesis)
 *
 * HONEST FLOOR WITH THIS BODY: sandbox --disable all = 11  (was 17 at s12/s13).
 * build_insns 216 vs target 215.
 *
 * THE ONE CHANGE vs the s12/s13 17-floor body: the tail block (the
 * `var_s1 == 0` arm of the `>` case) no longer declares the named pointer
 * local `u8 *temp_a0 = temp_s4 + (temp_s5 * 0x10);`.  The three fields it
 * addressed are now read with the scaled index written out at each site -
 * `*(s32 *)(temp_s4 + (temp_s5 * 0x10) + 0x114)` etc. - and the negation
 * `temp_v1_4` plus the whole `temp_v1_5` product are written BEFORE the
 * `+0x118` load.  Ordinary C: three member reads spelled from the object
 * base instead of through a cached cursor.  No new locals, nothing dead,
 * nothing annotated, no sanctioned-family claim needed.
 *
 * WHY IT WORKS - CLUSTER E IS CLOSED, EXACTLY AS THE SOLVER PREDICTED.
 * s7 modelled cluster E (the $a0/$a1 exchange across emitted slots
 * 159-182) as a LOCAL-alloc quantity-order decision in the tail block, and
 * `inverse.py local --swap 0,3` gave the complete single-atom vector set:
 * span(qty0) 30 -> <= 24, reachable from the birth end by moving the tail
 * POINTER quantity's birth from index 2 to anywhere in 8..19.  s7's
 * B_ptr_late (declare `temp_v1_4` before the pointer) reached birth 6 - four
 * of the six insns - and every later session's declaration-order probe
 * (s12 Q4, this session's p3/t2) is score-neutral for the same reason: while
 * the pointer is a NAMED LOCAL, its RTL is emitted at the head of the block
 * no matter what follows it.  Deleting the local entirely lets CSE form the
 * pointer pseudo at its FIRST USE, which is now inside the `temp_v1_5`
 * product, after both `(&Judge)[...]` index computations.  Measured from a
 * fresh QTYDBG dump of THIS body (block 42, main pass):
 *     before:  qty0 reg192 birth 2  death 32 refs 6 -> $a1   (wrong)
 *              qty3 reg209 birth 16 death 20 refs 2 -> $a0
 *     after :  qty2 reg208 birth 12 death 32 refs 6 -> $a0   (target)
 *              qty3 reg204 birth 16 death 20 refs 2 -> $a1   (target)
 * birth 2 -> 12 and span 30 -> 20, inside the solver's [8,19] birth window.
 * Cluster E is gone from the emitted diff; the score falls 17 -> 11.
 *
 * WHAT IS LEFT AT 11 (normalized objdump diff, tmp/grind/func_800283D0/s14/):
 *   1. THE STORE SINK, ~4 pts.  Emitted slots 83-88 and 146-148: target puts
 *      `sh v0,0x286(s0)` at the head of the shared store/calls block and
 *      `move a3,zero` in the jal delay slot; ours sinks the store into the
 *      delay slot.  sched1 owns this (s13's E-s13-1, dumps read).  Closing it
 *      needs a SOURCE-level label between the store and the argument set-up,
 *      which costs the arm its two duplicated arg1 references and with them
 *      cluster A.  Re-measured on THIS chassis: t4 (variant A's arm + this
 *      tail) = 17, i.e. the pin is still worth exactly -6 in cluster A and
 *      +6 in the store region.  The tension is unchanged by the tail fix.
 *   2. CLUSTER B, ~4 pts.  Emitted slots 45-48 (ours `j / nop`, target
 *      `nop / j`) and 126/131 (ours `nop` + a later `li v0,1`, target
 *      `li v0,1` in the branch delay slot).  This is also the whole of our
 *      216-vs-215 insn surplus.  Unmoved since s10; no measured C dial.
 *   3. Two commutative `addu` operand orders (slots 96 and 162).  Both
 *      re-probed this session and both score-NEUTRAL in either spelling -
 *      they are consequences of allocation, not levers.
 *
 * s15 (synthesis) RE-MEASURED THIS BODY AT 11/216 - unchanged, still the best
 * form.  What changed is the model: cluster A's decision is now the closed-form
 * inequality pri(73) > pri(143) under global.c's allocno_compare, and target's
 * own asm shows target reaches the seat with nrefs(arg1) = 7 (i.e. through
 * livelen(temp_s3) >= 20, NOT through the duplicated call pair this body uses).
 * The duplication here is a workaround that is incompatible with the store pin;
 * see evidence.md E-s15-1 / E-s15-5 before spending another ref-count probe.
 *
 * kengo:MED  |  sa_tan2/saTan2KabutoWareMove  |  216i @ floor 11
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
