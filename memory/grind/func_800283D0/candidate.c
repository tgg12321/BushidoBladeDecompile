/* candidate.c - func_800283D0 (saTan2KabutoWareMove), grind s25 2026-08-27 (synthesis)
 *
 * HONEST FLOOR WITH THIS BODY: sandbox --disable all = 2 / 215 insns.
 * (s24 body was 3 / 215; s23 4 / 215; s22 6 / 216; s16-s21 10 / 216.)
 *
 * WHAT CHANGED FROM THE s24 BODY (one hunk, three ordinary-C locals).
 * s24 left ONE residual at emitted 161 - ours `addu a0,a0,s4` vs target
 * `addu a0,s4,a0` - and modelled it in closed form: target needs the VALUE
 * expansion of `temp_s4 + temp_s5 * 0x10` (the sum assigned to a pointer local,
 * which comes out in SOURCE order) AND it needs that pointer's local-alloc
 * quantity to keep the $a0 seat.  s24's body AB supplied the value form and
 * LOST the seat: introducing `s32 *tail` moved the quantity's birth from luid
 * 12 to luid 6 (span 20 -> 26), so
 *     pri = floor_log2(refs) * refs * 10000 / span
 * fell 6000 -> 4615, under the Judge[]-element quantity's 5000, and the whole
 * tail renamed a0<->a1 (9 / 215).
 *
 * s25 closes it by DELAYING the pointer's RTL birth instead of touching refs:
 * the two Judge table indices are given their own named locals `idx0` / `idx1`
 * BEFORE the pointer, so expand emits the negu/addiu/andi index chain first and
 * the pointer's `plus` is born after it.  The tail quantity keeps birth 12 /
 * death 32, refs=6, span=20, pri=6000, ord5 -> $a0, exactly as the s24 body's
 * inline address form did - but the `addu` now carries target's operand order
 * because the sum is expanded as a VALUE.  Floor 3 -> 2.
 *
 * The three locals are ordinary C: `tail` is the pointer to the 0x114/0x118/
 * 0x11C triple that the three loads all index, and `idx0` / `idx1` are the two
 * table indices.  V1 (idx0 only) and V3 (the first Judge ELEMENT hoisted
 * instead of its index) both also measure 2 / 215; V2 (both indices named) is
 * banked because it is the symmetric spelling.
 *
 * REMAINING RESIDUAL (2 points, one cluster, emitted 45-47):
 *   ours   44 beq v1,v0,L / 45 li v0,1 / 46 j L / 47 nop
 *   target 44 beq v1,v0,L / 45 nop     / 46 j L / 47 li v0,1
 * s25 CONFIRMED the mechanism end to end by building the TU with the
 * instrumented cc1 under BB2_NO_FT_STEAL=1 (reorg.c:3817, the
 * `if (own_fallthrough && ! BB2_NO_FT ())` guard): with the fall-through steal
 * disabled this site becomes byte-identical to target.  The whole 2-point
 * residual is `fill_eager_delay_slots` -> `fill_slots_from_thread` on the range
 * chain's last `beq` with own_fallthrough = 1.
 *
 * FAMILY / POLICY (unchanged from s23/s24, still open before any submission):
 *   1. the `do { calls } while (0);` wrap (.claude/rules/do-while-zero-exception.md);
 *   2. the `sel19` arm's duplicated `*(s16 *)(arg0 + 0x286) = 0x19;` store
 *      (.claude/rules/duplicated-statement-into-arms.md).
 *
 * s25 KILLS (do not re-propose - detail in evidence.md):
 *   - V4: the ARRAY_REF-on-a-cast spelling `((s32 *)(temp_s4 + temp_s5*0x10))[0x45]`
 *     is still ADDRESS context - 3 / 215, operand order unchanged.  The
 *     value-vs-address law needs a real pointer LOCAL, not a parenthesised cast.
 *   - W1 (`return 1;` for the temp_v0 == 4 / == 0x14 exit) 2 / 215 byte-neutral;
 *     W2 / W3 (`block_13: return 1;`) 8 / 216.
 *   - Y1 / Y2 / Y3 / Y4: giving the chain fall-through `return 1;` an explicit
 *     `ret_one:` label plus an inbound `goto` (from the temp_v1 == 0x14 exit,
 *     from the temp_v0 exits, from block_13, and both) - Y3 and Y4 are
 *     BYTE-IDENTICAL to this body (only .L label numbering differs), so the
 *     label does not survive to reorg.  cse rewrites every OTHER `return 1;` as
 *     `move v0,s6` (ret is live in $s6), so those gotos never reference the
 *     block, and the one goto that does (the == 0x14 exit) is consumed by
 *     reorg's own redirect.
 * kengo:MED  |  sa_tan2/saTan2KabutoWareMove  |  215i @ floor 2
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
                            s32 idx0 = (temp_v1_4 + 0x400) & 0xFFF;
                            s32 idx1 = temp_v1_4 & 0xFFF;
                            s32 *tail = (s32 *)(temp_s4 + (temp_s5 * 0x10));
                            s32 temp_v1_5 = (s32)((&Judge)[idx0] * tail[0x45] + (&Judge)[idx1] * tail[0x47]) >> 0xC;
                            s32 temp_a0_2 = tail[0x46];
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
