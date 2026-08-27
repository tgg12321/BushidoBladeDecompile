/* candidate.c - func_800283D0 (saTan2KabutoWareMove), grind s24 2026-08-27 (synthesis)
 *
 * HONEST FLOOR WITH THIS BODY: sandbox --disable all = 3 / 215 insns.
 * (s23 body was 4 / 215; s22 6 / 216; s16-s21 10 / 216.)  Instruction count
 * still matches target exactly.
 *
 * WHAT CHANGED FROM s23.  Exactly one edit: the `u8 *temp_s3 = arg0 +
 * (temp_a1_2 * 2);` local is DELETED and both of its uses are spelled as full
 * address expressions:
 *
 *     s16 temp_v0_3 = *(s16 *)(arg0 + (temp_a1_2 * 2) + 0x288);
 *     ...
 *     if (*(s16 *)(arg0 + (temp_a1_2 * 2) + 0x288) == 5) { ... }
 *
 * This is ordinary C - no construct, no annotation - and it closes residual
 * cluster 2 of s23 (emitted 96 `addu s3,s0,v0` -> `addu s3,v0,s0`).  cse still
 * builds the shared address pseudo and it still lives in $s3 across the call
 * pair, so the emitted instruction sequence is unchanged apart from the
 * operand order.
 *
 * WHY IT WORKS (measured from the .rtl/.lreg dumps, E-s24-1/2).  BOTH residual
 * `addu` operand orders are FIXED AT EXPAND - the .rtl dump already carries the
 * final operand order, so neither combine, regmove nor reload ever touches it,
 * and no post-expand lever exists.  The order is decided by the EXPANSION
 * CONTEXT of the `plus`, not by the source operand order:
 *   - a `plus` expanded as a MEM ADDRESS (EXPAND_SUM) always comes out
 *     (shift, pointer) in this function - insns 183, 199 and 526 all do, and
 *     swapping the source order (C1), moving the constant (C2) or spelling it
 *     as an ARRAY_REF (C5) does NOT change it;
 *   - a `plus` expanded as a VALUE (an assignment to a pointer local) comes out
 *     in SOURCE order, i.e. (pointer, shift).
 * Target wants the ADDRESS form at emitted 96 and the VALUE form at emitted 161.
 * s23's body had a pointer local at 96 (value form -> wrong) and inline
 * addresses at 161 (address form -> wrong).  Deleting the local at 96 fixes 96.
 *
 * THE REMAINING 3 POINTS - two divergences, both fully attributed:
 *   1. emitted 45-47: ours `beq / li v0,1 / j / nop`, target `beq / nop / j /
 *      li v0,1`.  UNCHANGED from s23 and still a LIVENESS question, not a
 *      spelling one: jump_insn 78 steals the `li v0,1` back out of jump_insn
 *      84's slot via steal_delay_list_from_fallthrough, permitted only because
 *      oppregs = 0x20000380 (the flow live-in of the beq's TARGET block) lacks
 *      $v0.  s24 additionally KILLED the "merge the two `var_s1 = 0;` blocks"
 *      route (R1a): byte-neutral at 3, GCC had already cross-jumped them.
 *   2. emitted 161 `addu a0,a0,s4` vs target `addu a0,s4,a0`.  Now MODELLED
 *      END TO END and it is a two-body problem, which is why it is still open:
 *      the VALUE form (a `s32 *tail` pointer local, bodies B/AB) DOES produce
 *      target's operand order, but it stretches the tail-pointer local-alloc
 *      quantity's live span from 20 to 26 and its priority
 *      floor_log2(refs)*refs*10000/span from 6000 to 4615, which drops it below
 *      the Judge[]-element quantity's 5000, so it loses the $a0 seat to $a1 and
 *      the whole tail renames (9 / 215).  To win, a body needs the VALUE form
 *      AND pri(tail qty) > 5000, i.e. span <= 23 (currently 26) or refs >= 7
 *      (currently 6, a COMBINED quantity = sll producer + pointer).
 *
 * s24 KILLS (all measured this session, banked in rejected/):
 *   - B  = s23 body + `s32 *tail` pointer local for the 0x114/0x118/0x11C loads: 10.
 *   - AB = A + the same tail pointer local: 9 (right operand order, lost seat).
 *   - AB1 (0x118 load first) / AB2 (tail decl hoisted above temp_v1_4): 9 - the
 *     tail quantity's birth/death do not move with source position.
 *   - A1 (0x118 load first on the A chassis): 9.  A2/A3 (multiply operand swap): 10.
 *   - C1 (tail address source-order swap), C2 (constant before the shift),
 *     C5 (ARRAY_REF `((s32 *)temp_s4)[temp_s5*4 + 0x45]`): all byte-neutral at 3
 *     - the address-context operand order is NOT a source-order axis.
 *   - C4 (`s32 tail_off = temp_s5 * 0x10;` intermediate): 9.
 *   - R1a (merge the two `var_s1 = 0;` blocks so the beq targets a block with a
 *     different live-in): byte-neutral at 3.
 *
 * POLICY.  Two constructs remain family-relevant and MUST be settled before any
 * candidate-ready submission (unchanged from s23):
 *   1. the `do { calls } while (0);` wrap (annotated in place,
 *      `.claude/rules/do-while-zero-exception.md`);
 *   2. the `sel19` arm's duplicated `*(s16 *)(arg0 + 0x286) = 0x19;` store -
 *      the shape `.claude/rules/duplicated-statement-into-arms.md` covers, which
 *      mandates a FAKE annotation.  Annotate or ruling-request.
 * kengo:MED  |  sa_tan2/saTan2KabutoWareMove  |  215i @ floor 3
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
                        s16 temp_v0_3 = *(s16 *)(arg0 + (temp_a1_2 * 2) + 0x288);
                        s16 var_v0_2;
                        if (temp_v0_3 == temp_v1_3) {
                            do { /* FAKE: do-while(0) loop-note ref weighting, mechanism: flow.c REG_N_REFS += loop_depth feeding global.c allocno_compare, lever-exhaustion: memory/grind/func_800283D0/hypotheses.md */
                                func_80032854(*(s16 *)(arg0 + 4), 1, arg1, (s16 *)0);
                                func_80032854(*(s16 *)(arg0 + 4), 0x25, arg1, (s16 *)0);
                            } while (0);
                            if (*(s16 *)(arg0 + (temp_a1_2 * 2) + 0x288) == 5) {
                                if (((u32)(*(u16 *)(arg0 + 0xE) - 6) < 2U) || ((u32)(*(u16 *)(temp_s4 + 0xE) - 6) < 2U)) {
                                    if (var_s1 != 0) {
                                        goto sel19;
                                    }
                                    var_v0_2 = 0xB;
                                    goto block_48;
                                sel19:
                                    *(s16 *)(arg0 + 0x286) = 0x19;
                                    goto block_49;
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
