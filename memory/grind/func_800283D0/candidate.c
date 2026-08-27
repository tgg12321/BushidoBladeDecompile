/* candidate.c - func_800283D0 (saTan2KabutoWareMove), grind s16 2026-08-26 (synthesis)
 *
 * HONEST FLOOR WITH THIS BODY: sandbox --disable all = 10 / 216 insns
 * (s14/s15 body was 11 / 216).
 *
 * THE ONE CHANGE vs the s14/s15 11-floor body: the `<` arm (the
 * `temp_v1_3 < temp_v0_3` case).  It used to be a self-contained selection
 *
 *     s16 var_v0_4 = 0xB;
 *     if (var_s1 != 0) { var_v0_4 = 0x19; }
 *     store; calls(1); calls(0x25); return ret;
 *
 * and is now target's own shape - the 0xB edge is a source `goto` into
 * path1's selection statement, and only the 0x19 value is computed locally:
 *
 *     s16 var_v0_4 = 0x19;
 *     if (var_s1 == 0) { goto set_0xB; }
 *     store; calls(1); calls(0x25); return ret;
 *
 * Ordinary C: one arm of a two-way selection reuses the other path's
 * assignment instead of restating it (mixed exit forms / shared label, the
 * same idiom this body already uses for block_15 / block_20 / block_49).
 * No new locals, nothing dead, nothing annotated.
 *
 * WHY IT WORKS.  The arm still supplies the two duplicated `arg1` call
 * references that carry nrefs(pseudo 73) to 9 and win cluster A - measured
 * from ALLOCDBG on THIS body: 73 (arg1) ord 17, nrefs 9, livelen 98,
 * pri 2755 -> $s2, and 143 (temp_s3) ord 20, nrefs 3, livelen 14, pri 2142
 * -> $s3, i.e. exactly target seats.  What the `goto set_0xB` buys on top is
 * the arm's BRANCH STRUCTURE: our 0xB edge now branches to path1's `addiu
 * $v0,0xB` (target's .L80028518) instead of keeping a local copy, which is
 * bit-for-bit what target does at .L80028610.
 *
 * NOTE (s16 re-attribution, supersedes E-s15-5): target's `<` arm is NOT
 * variant A's arm.  Target compiles with nrefs(73) = 9 - the duplicated call
 * pair IS target's own mechanism for the cluster-A seat, not a workaround -
 * and the livelen(143) >= 20 route that E-s15-5 deduced is NOT required.
 * See evidence.md E-s16-3.
 *
 * WHAT IS LEFT AT 10 (normalized objdump diff, tmp/grind/func_800283D0/s16/):
 *   1. CLUSTER B, ~4 pts, and the whole 216-vs-215 insn surplus.  Emitted
 *      45-48 (ours `j / nop`, target `nop / j`) and 126/131 (ours `nop` in
 *      the `beqz $v0` delay slot plus a later `li v0,1`; target steals the
 *      branch target block's `li v0,1` into the slot).  reorg.c
 *      steal_delay_list_from_target refusal - no measured C dial yet.
 *   2. THE STORE SINK, ~2 pts.  Emitted 83-88: target puts `sh v0,0x286(s0)`
 *      at the head of the shared store/calls block; ours sinks it into the
 *      first jal delay slot (sched1, E-s13-1).
 *   3. THE ARM STORE, ~1-2 pts.  Emitted 148: target fills the arm's `j`
 *      delay slot with the arm's OWN `sh v0,0x286(s0)`; ours has no store
 *      left there (jump2 merged it into path1's) and reorg steals `li a1,1`
 *      from the branch target block instead.  Items 2 and 3 are the same
 *      question: whether path1's store sits at a block boundary.
 *   4. Two commutative `addu` operand orders (96 `addu s3,s0,v0` and 162
 *      `addu a0,a0,s4`).  Re-probed in earlier sessions, score-neutral in
 *      either spelling - consequences of allocation, not levers.
 *
 * s18 ADDENDUM (forensics, 2026-08-27) - this body is UNCHANGED and still
 * measures 10 / 216.  What s18 added:
 *   - Cluster A has a SECOND solution: livelen(temp_s3) >= 20 flips the
 *     $s2/$s3 seats with nrefs(arg1) still 7 (E-s18-3, body V6).  It is
 *     measured NET-NEGATIVE on a store-pinned body (V6 = 19/214) because the
 *     six insns it needs displaced cross target's reload/compare group.
 *   - The complete ref map is banked (E-s18-1): arg1 has exactly 7 refs (the
 *     param copy + six call arguments), temp_s3 exactly 3 (def + two 0x288
 *     loads).  Target's ASM shows the SAME counts, so target's seats are only
 *     explicable by a source duplicate that jump2 merges AFTER RA (E-s18-6).
 *   - Cluster B's dbr refusal is NAMED (E-s18-8): jump_insn 344's slot is
 *     refused solely by insn_sets_resource_p(trial=368, &opposite_needed) -
 *     $v0 is live in mark_target_live_regs of the fall-through 0x19/0xB
 *     selection.  NOT a LABEL_NUSES refusal.
 *
 * kengo:MED  |  sa_tan2/saTan2KabutoWareMove  |  216i @ floor 10
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
                            s16 var_v0_4 = 0x19;
                            if (var_s1 == 0) {
                                goto set_0xB;
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
