/* 28 / 215. `var_v0_2 = (var_s1 != 0) ? 0x19 : 0xB;` DOES defeat the
 * cross-jump (215 insns) but emits `beqz $s1 / li 0xB / j / li 0x19` where
 * target has `bnez $s1 / li 0x19 / j / li 0xB`: 3 diffs at slots 138-141.
 * The ternary arm order decides the branch sense; only the
 * `(var_s1 == 0) ? 0xB : 0x19` order reproduces target, and only WITH an
 * int-typed intermediate (without one it is HImode and re-merges at 211).
 */
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
 *
 * s4 (2026-08-27, permuter): body UNCHANGED, floor re-measured flat at 28 /
 * 215 insns and src restored to exactly this body at session end.  Four
 * decomp-permuter campaigns (~20k iterations, workspaces under
 * tmp/grind/func_800283D0/s4/perm_[abcd], all harvested and stopped in
 * session) produced ONE semantically-valid improving find: a
 * `do { ... } while (0);` wrap around the block_48 region, 28 -> 26.  It is
 * banked NOT adopted (rejected/dowhile0-refweight-out-of-scope.c) because its
 * measured mechanism is flow.c loop-depth REG_N_REFS weighting feeding
 * global.c allocno_compare, which is OUTSIDE the LABEL_OUTSIDE_LOOP_P /
 * reorg.c scope that .claude/rules/do-while-zero-exception.md sanctions.  Its
 * value is the measurement: live lengths pinned, ref counts +1 per in-wrap
 * reference, and the callee-saved cluster reorders as ONE permutation - so
 * the live axis for the s2/s3 rotation is REG_N_REFS, not live_length.  The
 * lower-scoring finds (22) are semantically invalid AND directionally
 * impossible (target stores `sh $v0, 0x286($s0)` at all three sites).
 *
 * s5 (2026-08-26, synthesis): body UNCHANGED, floor re-measured flat at 28 /
 * 215 insns.  The global-allocation model behind the s2/s3 cluster is now
 * ground-truthed with BB2_ALLOC_DEBUG=1 (it confirms every hand-derived
 * number) and its flip window is MEASURED with a liveness dial: 6-10 insns of
 * extra live length inside pseudo 143's range (and outside pseudo 73's, since
 * arg1 dies at the equal-arm's second call) flip the whole callee-saved map to
 * target's.  The ref-count axis is FORECLOSED - target's own asm has exactly
 * our reference counts for $s2/$s3/$s4/$s5 - and there is no honest place for
 * 6-10 instructions, so the cluster is a forensics target now, not a spelling
 * one.  Diamond 2's last structural route (delete the shared block_48 label,
 * store+return in each arm) measured 30/211 and is banked as
 * rejected/diamond2-no-shared-label-remerges.c.
 *
 * s6 (2026-08-26, synthesis): body UNCHANGED, floor re-measured flat at 28 /
 * 215 insns and the ALLOCDBG table re-verified byte-identical to s5's.  Three
 * products.  (1) The residual is FIVE clusters, not three: A s2/s3 rotation
 * (12), B return-1 exit-block sharing (4, slots 45-47), C v0/v1 rename (4),
 * D diamond-2 selection order (4), E tail a0/a1 quantity order (7).  (2) Cluster
 * B, dismissed since s1 as a near-neutral reorg wobble, is really a CFG fact:
 * target's `.L80028488` is a LABELLED shared `j / li v0,1` block with three
 * early-exit predecessors, which is why reorg cannot sink the `li` into the
 * preceding beq's delay slot; ours is unlabelled because our three early exits
 * return `ret` and cross-jump into the `move v0,s6` block.  Both constant-1
 * respellings measure 34/216 (jump2 keeps the late copy and inverts the chain's
 * last beq) - banked as rejected/early-exit-return-const1-relocates-shared-block.c
 * and rejected/shared-ret-one-label-relocates-shared-block.c.  (3) The s2/s3
 * window has a second solution branch never derived before: L73 (arg1's home) in
 * [38,65] flips the whole callee-saved map, needs liveness REMOVED rather than
 * 6-10 instructions added, and so is not blocked by the 215==215 budget that
 * foreclosed the L143 route.  The named-intermediate escape hatch under that
 * route is now closed with a measurement (cse/combine delete the copies before
 * life_analysis; every ALLOCDBG row is unmoved).
 *
 * s7 (2026-08-26, solver): body UNCHANGED, floor re-measured flat at 28 / 215.
 * Layer triage (goal_from_tgt classify, object-level) says the FIRST divergence
 * is PRE-RA and consists of exactly ONE instruction shape - ours emits a beqz
 * where target emits a bnez.  The remaining residual is 21 register renames +
 * 4 scheduler moves + 2 immediate/reloc-differing pairs, so the 215-insn stream
 * has no hidden selection difference anywhere.  Exhaustive single-atom sweeps
 * of the validated forward models replace every hand-derived window: the
 * callee-saved cluster is a 73<->143 exchange reachable ONLY through those two
 * allocnos (L73 92->[38,65], L143 14->[20,21], R73 7->[8,11], R143 3->2, with a
 * joint diagonal region mapped), and the tail a0/a1 cluster is a LOCAL-alloc
 * decision in block 41 between qty0 = reg 184 (the temp_a0 pointer) and qty3 =
 * reg 201 (a Judge[] element), whose complete vector set is span(qty0) 30-><=24
 * or refs(qty0) 6->8..10 - s3's alternative "span(qty3) >= 6" is REFUTED.  One
 * ordinary-C declaration reorder (temp_v1_4 declared before temp_a0) moves the
 * pointer's birth 2 -> 6 (span 30 -> 26) while staying at 28 / 215: 4 of the 6
 * insns the window needs, the first lever ever shown to move that dial.  The
 * death-side route is now closed (hoisting the abs measures 44 / 214).
 *
 * s8 (2026-08-26, forensics): FLOOR 28 -> 25 (215/215 insns), the first floor
 * movement since s2.  Two forensic products drive it.
 *
 * (1) PASS ATTRIBUTION CORRECTION.  The cross-jump pass ("jump2") is NOT an
 * early/pre-RA pass: toplev.c:3141 calls jump_optimize(insns, 1, 1, 0) AFTER
 * sched2 and AFTER global/local register allocation and reload, as the last
 * pass before reorg (.dbr).  So every cross-jump decision on this function
 * (clusters B and D) is taken on POST-RA, POST-SCHEDULING RTL in which every
 * pseudo has already been replaced by its hard register.  No RA-level or
 * scheduling-level lever can influence the merge, and the merge cannot be what
 * chooses a branch SENSE (the beqz/bnez at slot 138 comes from RTL expansion of
 * the source's branch shape).
 *
 * (2) THE MERGE THAT COSTS FOUR INSNS, MEASURED EXACTLY.  With both var_v0_2
 * selection copies spelled canonically, instrumented cc1 (BB2_XJUMP_DEBUG)
 * prints, for the tail copy's `j block_48` (uid 362) against the ==5 copy's
 * `j block_48` (uid 397, a jump_chain partner, so find_cross_jump starts at
 * minimum=2):
 *     MATCH i1=358 i2=393 set(reg<-11)  min->1
 *     MATCH i1=354 i2=389 set           min->0     (the two bnez insns)
 *     MATCH i1=351 i2=386 set(reg<-25)  min->-1
 *     LABEL-BONUS i1=347 (label)        min->-2; break   => WIN
 * i.e. the block is merged with THREE units of slack.  Defeating it needs the
 * backward walk to fail on the FIRST compared insn.
 *
 * (3) THE LEVER: find_cross_jump compares patterns with rtx_renumbered_equal_p,
 * which compares GET_MODE.  Because var_v0_2 is `s16`, both copies' constant
 * loads are `(set (reg/v:HI 2 v0) (const_int 25))` / movhi_internal2.  Routing
 * the tail copy's selection through an s32 intermediate makes ITS constant
 * loads `(set (reg:SI 2 v0) ...)` / movsi_internal2 - RTL-DISTINCT but
 * BYTE-IDENTICAL (`addiu $v0,$zero,0x19` either way), so find_cross_jump
 * PAT-MISMATCHes on insn 1, both copies survive, and BOTH now carry target's
 * canonical `bnez $s1 / li 0x19 / j / li 0xB` order.  Cluster D (4 diffs) is
 * CLOSED; slots 127-130 and 138-141 now match target exactly.
 *
 * CLASSIFICATION IS OPEN - DO NOT SUBMIT THIS AS candidate-ready WITHOUT A
 * RULING.  `s32 sel` is a fresh local that is written twice (init + conditional
 * re-write) and read once, carrying a real consumed value.  That does NOT fit
 * the named-intermediate family (its once-written prong excludes multi-write),
 * and no other frozen SOTN family covers it.  The sanctioned variable-reuse
 * spelling of the identical lever was measured and FAILS: borrowing the
 * existing s32 `d_val` measures 33 / 216 (the copy does not coalesce; see
 * rejected/reuse-dval-selection-holder-no-coalesce.c).  Next session must
 * either obtain a ruling or find a mode-splitting spelling that is plainly
 * ordinary C.
 *
 * Remaining residual at 25: cluster A (s2/s3 callee-saved rotation, ~10 slots),
 * cluster B (slots 45-47 nop/li v0,1 exit-block sharing), cluster C (v0/v1 at
 * slots 126/132/133/135), cluster E (a0/a1 at slots 159/161/168/171).
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
                            var_v0_2 = (var_s1 != 0) ? 0x19 : 0xB;
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
