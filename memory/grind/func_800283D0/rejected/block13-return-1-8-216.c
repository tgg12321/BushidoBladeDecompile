/* candidate.c - func_800283D0 (saTan2KabutoWareMove), grind s23 2026-08-27 (structural)
 *
 * HONEST FLOOR WITH THIS BODY: sandbox --disable all = 4 / 215 insns.
 * (s22 body was 6 / 216; s16-s21 body was 10 / 216.)  THE INSTRUCTION COUNT NOW
 * MATCHES TARGET EXACTLY for the first time in 23 sessions, and CLUSTER B - the
 * unfilled `beqz $v0` delay slot that has been the whole 216-vs-215 surplus
 * since s3 - IS CLOSED.
 *
 * WHAT THIS BODY IS.  It is s22's A3 body (s21's V1 chassis + the sanctioned
 * do-while(0) wrap that wins cluster A's $s2/$s3 seat swap) with ONE further
 * change, in the `== 5` selection inside the `==` arm.  s22 spelled it
 *
 *      var_v0_2 = 0x19;
 *      if (var_s1 == 0) { var_v0_2 = 0xB; }
 *      goto block_48;              // block_48 stores var_v0_2 to arg0+0x286
 *
 * and this body spells the SAME selection with the 0x19 edge as the branch's
 * own target thread, carrying its own copy of the (real, already-existing)
 * store statement:
 *
 *      if (var_s1 != 0) { goto sel19; }
 *      var_v0_2 = 0xB;
 *      goto block_48;
 *  sel19:
 *      *(s16 *)(arg0 + 0x286) = 0x19;
 *      goto block_49;
 *
 * The two forms are semantically identical (s1 != 0 stores 0x19, s1 == 0 stores
 * 0xB, both then fall into `return ret`), and the duplicated store RE-MERGES in
 * jump2: emitted 128-131 are byte-identical to target (`bnez s1,.L800286F8 /
 * li v0,25 / j .L800286F8 / li v0,11`), i.e. both edges still reach the ONE
 * shared store.  The duplication is invisible in the bytes; what it changes is
 * WHICH reorg.c pass fills a delay slot.
 *
 * WHY IT WORKS (the named mechanism, measured not inferred - see E-s23-1/2).
 * s19/s20 proved the `beqz $v0` slot is refused because
 * `insn_sets_resource_p (trial, &opposite_needed)` is 1: the fall-through
 * selection block owns a `$v0` write which `fill_simple_delay_slots` steals
 * backward into the selection's OWN branch, and `update_block` leaves a
 * `(use (insn N))` marker inside the window `mark_target_live_regs` walks, so
 * `$v0` is live on the opposite thread by the time `fill_eager_delay_slots`
 * reaches the `beqz`.  s19's fix (hoisting the `0x19` out) removed the write but
 * gave up the selection branch's own slot (the +2 the whole W1/W2b/F1 hoist
 * family pays, chassis-independently).  This body instead moves the `0x19`
 * write to the OTHER SIDE of the selection branch: it is now the head of that
 * branch's TARGET thread, so it is taken by `fill_slots_from_thread` with
 * `INSN_FROM_TARGET_P` set, and `update_block` (reorg.c:2270) returns EARLY
 * without emitting any marker.  Both slots are filled, exactly as target does.
 * This is E-s19-7's option (a) - the one route s19 named and no session had yet
 * spelled - and it costs nothing.
 *
 * FAMILY / POLICY.  Two constructs in this body are family-relevant and BOTH
 * need a ruling before any candidate-ready submission:
 *   1. the `do { calls } while (0);` wrap (unchanged from s22, annotated in
 *      place, `.claude/rules/do-while-zero-exception.md`);
 *   2. the `sel19` arm's duplicated `*(s16 *)(arg0 + 0x286) = 0x19;` store.
 *      This is a REAL statement duplicated into a branch arm that re-merges
 *      byte-neutrally (emitted 128-131 == target), which is the shape
 *      `.claude/rules/duplicated-statement-into-arms.md` describes - that rule
 *      mandates a /* FAKE * / annotation, which this body does NOT yet carry
 *      because the store here is not merely a codegen decoy: it is target's own
 *      control flow (the same double-goto shape s21 read off target's `<` arm).
 *      The next session must either annotate it per that rule or ask for a
 *      ruling; do not submit candidate-ready without settling it.
 *
 * RESIDUAL AT 4 (tmp/grind/func_800283D0/s23/diff_X2.txt; `lui at,0` / `jal 0`
 * entries are unresolved-relocation artifacts of objdumping the .o):
 *   1. emitted 45-47 - ours `beq / li v0,1 / j / nop`, target
 *      `beq / nop / j / li v0,1`.  MECHANISM NOW NAMED (E-s23-3): jump_insn 78
 *      (the range chain's last `beq`) STEALS the `li v0,1` back out of
 *      jump_insn 84's already-filled slot via
 *      `steal_delay_list_from_fallthrough`; the steal is allowed only because
 *      $v0 is absent from `opposite_needed` = the exact flow live-in of the
 *      `beq`'s target block (oppregs=0x20000380).  fill_simple ALREADY produces
 *      target's arrangement (`DBRDBG simp insn=84 trial=82 elig=1`); a later
 *      pass undoes it.  This is a LIVENESS question, not a spelling question.
 *   2. emitted 96 `addu s3,s0,v0` vs `addu s3,v0,s0` and emitted 161
 *      `addu a0,a0,s4` vs `addu a0,s4,a0` - commutative operand order.  Both
 *      source-order spellings are banked neutral (s3, s22); the order is decided
 *      after fold, so it is not a source-order axis.
 *
 * s23 KILLS (do not re-propose - detail in evidence.md):
 *   - X1: the same double-goto selection but with the 0x19 arm going to the
 *     SHARED block_48 store (`sel19: var_v0_2 = 0x19; goto block_48;`) = 6, i.e.
 *     byte-identical to s22.  The arm must carry its OWN store; with the shared
 *     store the `0x19` write is still emitted before the branch.
 *   - X3: the mirrored spelling (0xB edge as the branch target) = 9 - GCC
 *     re-inverts the branch sense away from target's `bnez s1`, the same
 *     re-inversion s20's W3 measured.
 *   - Q1/Q2: relocating the range check's `return 1;` to a `ret1:` label at the
 *     end of the function (Q1) or just before block_13 (Q2), to deny
 *     jump_insn 78 a stealable fall-through, = 10 / 215 and 8 / 216.  The
 *     relocation costs more than the two points it could buy.
 * kengo:MED  |  sa_tan2/saTan2KabutoWareMove  |  215i @ floor 4
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
    return 1;
}
