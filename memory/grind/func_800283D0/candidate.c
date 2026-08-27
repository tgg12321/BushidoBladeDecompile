/* candidate.c � func_800283D0 (saTan2KabutoWareMove), grind s2 2026-08-26
 * Honest floor with THIS body: sandbox --disable all = 28 (down from 30);
 * build_insns 215 == target 215 EXACTLY (was 211).
 * This body is APPLIED in src/code6cac_b.c.
 *
 * s1 changes retained verbatim (s32 raw holder; var_s1=0 placement; inline
 * `return 1;` mixed exit; in-arm recompute of temp_a1_2*2).
 *
 * s2 change (the only edit vs the s1 body): the TAIL (`!= 5`) copy of the
 * var_v0_2 = 0x19/0xB selection is spelled as an if/else instead of
 * `var_v0_2 = 0x19; if (var_s1 == 0) var_v0_2 = 0xB;`.  With BOTH copies
 * spelled identically, GCC's jump2 cross-jump pass merges them
 * (instrumented cc1 BB2_XJUMP_DEBUG: `DO_CROSS_JUMP jump=362 newjpos=351
 * newlpos=386`, a 3-insn backward tail match) and we emit 211 insns where
 * the target emits 215.  Spelling one copy as an if/else makes the first
 * backward-compared insn `set(reg<-11)` vs `set(reg<-25)`, find_cross_jump
 * PAT-MISMATCHes on insn 1, and both copies survive.
 *
 * KNOWN-IMPERFECT: the if/else copy emits `beqz s1 / li v0,11 / j / li v0,25`
 * where target has `bnez s1 / li v0,25 / j / li v0,11` (4 diffs).  The
 * byte-exact answer needs BOTH copies in the canonical `v=0x19; if(!s1) v=0xB;`
 * order AND unmerged � not yet found.  See hypotheses.md H8.
 *
 * s3 (2026-08-26, structural): body UNCHANGED, floor re-measured flat at 28 /
 * 215 insns.  Ten ordinary-C structural respellings measured against this body;
 * eight are byte-neutral (declaration order, deleting the temp_s3 / temp_a0
 * locals, integer-typed pointer plus, ternary and two-goto-label spellings of
 * the tail selection) and two are worse (multiply-operand swap 29, sinking the
 * temp_a0_2 abs 31/216).  The tail a0/a1 cluster is now MODELLED: it is a
 * local-alloc quantity-order decision, pri(qty0 pointer)=4000 vs
 * pri(qty3 Judge[] element)=5000, ties broken by ascending qty number so a tie
 * suffices - see evidence.md 2026-08-26 session 3.
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
                                return ret;
                            }
                            if (var_s1 != 0) {
                                var_v0_2 = 0x19;
                            } else {
                                var_v0_2 = 0xB;
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
