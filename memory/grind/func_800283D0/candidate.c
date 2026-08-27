/* candidate.c - func_800283D0 (saTan2KabutoWareMove), grind s12 2026-08-26 (structural)
 *
 * HONEST FLOOR WITH THIS BODY: sandbox --disable all = 17  (was 20 at s11).
 * build_insns 216 vs target 215.
 *
 * THE ONE CHANGE vs the s11 body: in the `temp_v1_3 < temp_v0_3` arm the
 * selection variable is now initialised to 0xB and overwritten with 0x19 in the
 * `var_s1 != 0` case, i.e. the OPPOSITE initialisation order from the
 * `temp_v1_3 == 0 / > 0` path (path1), which initialises to 0x19 and overwrites
 * with 0xB.  Everything else is identical to s11.  Ordinary C: two branch arms
 * that happen to spell the same two-way choice from opposite ends.  No new
 * locals, nothing dead, nothing annotated.
 *
 * WHY IT WORKS - CLUSTER A AND THE ARM ARE NOW BOTH CLOSED AT ONCE.
 * s11 proved the two requirements are independent and could not be satisfied
 * together: the arm has to DUPLICATE the two func_80032854 calls (that is the
 * only reachable dial on nrefs_flow(pseudo 73 = arg1), 7 -> 9, into s7's [8,11]
 * window, which is what wins $s2 for arg1); but any duplicate that is spelled
 * IDENTICALLY to path1 is merged away wholesale by jump2's find_cross_jump,
 * taking target's 4-insn arm with it.  s12's measurement shows the resolution:
 * the walk-back in find_cross_jump is what decides how much of the duplicate
 * survives, and it stops at the first pair of insns that differ.  With the
 * inverted initialisation the two blocks are
 *     path1 : li v0,0x19 ; bne s1,0,Lsh ; li v0,0xB  ; Lsh: sh ; calls ; j end
 *     arm   : li v0,0xB  ; beq s1,0,Lsh ; li v0,0x19 ; Lsh: sh ; calls ; j end
 * so the walk matches `j end`, both calls and the `sh`, then compares
 * `li v0,0x19` against `li v0,0xB`, differs, and STOPS.  The arm therefore
 * merges only its tail from the store onward and keeps its own two-insn
 * selection plus the redirected branch - which is exactly target's four
 * instructions at .L80028610.  With the s11 spelling (both blocks initialised
 * to 0x19) the walk also matched the branch and the `li`, so the whole arm
 * collapsed into a single `bnez` and 3 instructions were lost.
 * Measured: seats now correct at every one of the twelve $s2/$s3 slots
 * (3, 4, 10, 85, 90, 96, 97, 102, 107, 111, 149, 154) AND the arm's
 * `beqz s1 / li v0,0x19 / j / sh` shape is reproduced.
 *
 * MEASURED AND DEAD THIS SESSION (all on this chassis, all banked in rejected/):
 *   - V1 arm spelled purely with gotos (`goto set_0xB` + `goto do_calls`,
 *     no duplicated calls): arm bytes EXACTLY target's, but nrefs_flow(73)
 *     falls back to 7, `base meets goal? False`, and all twelve $s2/$s3 slots
 *     inverted.  23 / 216.  This is the clean proof that the arm shape and the
 *     nrefs dial are carried by the same construct.
 *   - V2/V3 arm duplicates only the FIRST call and re-enters at a `do_call2`
 *     label before the second (nrefs 8, inside the window): 22 / 219 and
 *     22 / 212.  Seats flip correctly at nrefs 8, but the arm still merges.
 *   - V4/V5 arm with BOTH edges fully duplicated (no selection variable, a
 *     store+calls+return written out per edge): 24 / 224 and 26 / 224.
 *   - V7 0xB edge duplicates, 0x19 edge stores then `goto do_calls`: 20 / 222.
 *   - V8 (= s11's R6 re-measured) 0xB edge `goto set_0xB`, 0x19 edge duplicates:
 *     20 / 219.
 *   - V9 inverted-init selection + `goto do_calls` (no duplicated calls):
 *     25 / 216 - again the nrefs dial lost.
 *   - P1 path1's OWN store+calls duplicated into its two selection edges (a
 *     second, independent route to more arg1 refs): 29 / 224.
 *   - P2 path1's store duplicated per edge with a shared `do_calls`: 19 / 222.
 *   - Q1 `(temp_a1_2 * 2) + arg0` instead of `arg0 + (temp_a1_2 * 2)`: 17,
 *     score-NEUTRAL - re-confirms s11's R2 on this chassis; the `addu $s3,$v0,$s0`
 *     operand order is a consequence of allocation, not an independent lever.
 *   - Q4 tail declaration order (temp_v1_5's product computed before the
 *     0x118 load): 17, score-NEUTRAL - does not move cluster E.
 *
 * REMAINING RESIDUAL AT 17 (from the normalized objdump-vs-target diff,
 * tmp/grind/func_800283D0/s12/):
 *   1. THE STORE SINK, ~4 pts.  Ours emits `li a1,1 / move a2,s2 / lh a0 /
 *      move a3,zero / jal / sh v0,0x286(s0)<delay>`; target emits
 *      `sh v0,0x286(s0) / li a1,1 / move a2,s2 / lh a0 / jal / move a3,zero`.
 *      The store is sunk into the jal's delay slot by sched2 + reorg because
 *      nothing separates it from the argument set-up.  In target `.L80028520`
 *      sits between them, kept alive by the arm's `j .L80028520` - and that
 *      label exists because target's arm merged only from AFTER the store on
 *      its 0x19 edge (it keeps its own `sh`), whereas ours merges AT the store.
 *      Same one-instruction difference in the walk-back that s12 exploited.
 *   2. CLUSTER B, 2 pts: ours fills the dispatch chain's last `beq` delay slot
 *      with `li v0,1` and leaves the following `j`'s slot empty; target does the
 *      reverse.  Plus the same shape at emitted slot 126.  Unmoved since s10.
 *   3. CLUSTER E, ~8 pts: the $a0/$a1 exchange in the tail Judge product
 *      (emitted slots 159/161/168/169/171/178/181) - a LOCAL-alloc seat, s7's
 *      window span(qty0) 30 -> <=24 or refs(qty0) 6 -> 8..10.  Untouched.
 *
 * kengo:MED  |  sa_tan2/saTan2KabutoWareMove  |  216i @ floor 17
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
