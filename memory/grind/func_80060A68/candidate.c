/* [s24 2026-09-04 - solver modality.  SPINE CHANGE: candidate.c is now the T1 body
 * (the p10 read above the Z0 store), not the E2 body.  Both measure engine score 2 on
 * today's HEAD chassis; T1 is 65 build insns against target's 66, E2 is 66.  The E2 body
 * is preserved verbatim, unchanged and NOT disproven, at
 * memory/grind/func_80060A68/e2-spine-floor2-cse-merged.c.
 *
 * WHY THE SPINE MOVED.  s24 measured both against the REAL target bytes (an
 * engine.score.normalized_insns diff of the sandbox object against build/src/text1b.o,
 * which is the canonical INCLUDE_ASM object and therefore literally asm/funcs/
 * func_80060A68.s assembled):
 *
 *     E2 : 66 insns, raw diff 4, TWO `lw ?,16(v1)` at slots 11 ($a1) and 19 ($a0)
 *          - target's exact first two slots and seats - and NO THIRD LOAD.
 *     T1 : 65 insns, raw diff 3, THREE `lw ?,16(v1)` at slots 4, 18, 21
 *          against target's 11, 19, 22.
 *
 * `tools/ra_solver/goal_from_tgt.py classify text1b func_80060A68` on E2 returns
 * FIRST DIVERGENCE: PRE-RA, "next tool: none - the residual is upstream of every model"
 * (ours only: nop x1; target only: lw #,16(#) x1).  E2's missing load is the s20 cse
 * merge, closed as a class at cse.c:1701; no RA or scheduler model can express it.
 *
 * T1's residual, by contrast, is ONE INSTRUCTION MOVED and nothing else:
 *
 *     ours   : lhu v0,0(v1) | lw a1,16(v1) | sll | lui at | addu at | sw zero
 *              | lw v0,12(v1) | lw a0,12(v1) |                | lw v0,0(v0) ...
 *     target : lhu v0,0(v1) | nop          | sll | lui at | addu at | sw zero
 *              | lw v0,12(v1) | lw a0,12(v1) | lw a1,16(v1) | lw v0,0(v0) ...
 *
 * Every one of the other 64 instructions, every nop included, is identical.  That is a
 * pure permutation of an identical instruction multiset - exactly what tools/sched_solver
 * models exactly - so T1 is the spine a solver session can actually work on.
 *
 * THE RESIDUAL, STATED AS A PREDICATE (s24's main deliverable; do not re-derive it).
 * The scheduler model extracted clean (`extract.py text1b` -> parity=True, 1764 blocks;
 * this function's pass1 block 0 of 42 insns and pass2 block 0 of 44 insns both replay
 * BASELINE EXACT).  The cc1-index -> RTL-uid table is in evidence.md; the insns that
 * matter are uid 9 `lw $3,D_800A3468`, uid 16 `lhu $2,0($3)`, uid 12 `lw $5,16($3)`
 * (THE P10 LOAD), uid 21 `sll`, uid 25 `sw $0,D_800F10D0($2)` (the Z0 store),
 * uid 28 `lw $2,12($3)`, uid 35 `lw $4,12($3)`, uid 30 `lw $2,0($2)`, uid 32
 * `sw $2,32($3)`.  sched.c builds the block BACKWARDS, so the pick list is the reverse
 * of the emission order.  Our pass-2 pick tail is
 *     ... 32, 30, 35, 28, 25, 21, 12, 16, 147, 145, 9
 * and target's must be
 *     ... 32, 30, 12, 35, 28, 25, 21, 16, 147, 145, 9
 * i.e. UID 12 MUST BE PICKED BETWEEN UID 30 AND UID 35.  `rank_for_schedule` is priority
 * descending, then dependence class, then LUID descending, and the harvested pass-2
 * values are pri(30)=4, pri(35)=pri(28)=pri(25)=pri(21)=3, pri(12)=pri(16)=2, with
 * luid(12)=17, luid(30)=21, luid(35)=22.  So the requirement is exactly:
 *
 *     INSN_PRIORITY(12) = 3 AND pass-2 LUID(12) > 22,   or   INSN_PRIORITY(12) = 4.
 *
 * `priority()` is the longest dependence path FROM THE BLOCK HEAD, and with the p10 read
 * written first uid 12's only LOG_LINKS predecessor is uid 9 (pri 1, load cost 2), so
 * pri(12) = 1 + 2 - 1 = 2 and CANNOT be raised without giving the load a predecessor -
 * which means moving it later in source.  pass-2 LUID is the sched1 emission position, so
 * the whole question collapses to: sched1 must emit the p10 load after uid 35 and before
 * uid 32, with the Z0 store as the only store above it in source.
 *
 * WHAT s24 MEASURED AGAINST THAT PREDICATE (8 bodies, zero FAKE constructs in any).
 *
 * (1) DEPTH-1 SOLVER SEARCH, FULL ATOM VOCABULARY - CLEAN NEGATIVE.
 *     `perturb.py --pass 2 --block 0 --goal-before 30:12 --goal-before 12:35 --depth 1`
 *     searched all 4634 single atoms (add_dep / del_dep / luid / luid_move / cost) and
 *     reports NO perturbation reaches the goal.  The pass-1 sub-goal (`--goal-before
 *     12:35`, 4400 atoms) returns exactly ONE vector: `del_dep 25 <- 12`, i.e. drop the
 *     Z0 store's ANTI-dependence on the p10 load.  That vector is the right shape and has
 *     NO C SPELLING under GCC 2.7.2: both memrefs are `(plus reg X)` with a varying index
 *     so `memrefs_conflict_p` cannot disambiguate them, and C offers no `restrict`, no
 *     type-based aliasing, and no `RTX_UNCHANGING_P` on an ordinary load.
 *
 * (2) THE SIX-POSITION COLLAPSE - the statement-move axis below the Z0 store is CLOSED.
 *     Six bodies put the p10 read directly after the Z0 store, after copy 1, after copy 2,
 *     and - three of them - INSIDE a copy statement, between the copy's load and its store
 *     (`c1 = *(s32 *)(*(s32 *)(outer + 0xC) + 0); p10 = ...; *(s32 *)(outer + 0x20) = c1;`
 *     and the copy-2 equivalent), which was the only shape that keeps the Z0 store as the
 *     read's sole preceding store.  ALL SIX score 5 / build 67 with their three 0x10 loads
 *     at 18 ($a0) / 23 ($v0) / 24 ($a1) - the p10 load LAST - and their raw cc1 streams are
 *     BYTE-IDENTICAL to each other (empty diffs).  The six positions are normalised to one
 *     stream upstream of the scheduler, so splitting a copy buys no LUID control.  Only two
 *     regimes are reachable by moving this statement: p10 load FIRST (this body, 65 insns,
 *     score 2) or p10 load LAST (67 insns, score 5).  Target's middle seat is in neither.
 *
 * TOOL NOTE for the next solver session.  Upstream `tools/sched_solver/goalmap.py`
 * ABORTS object-mode goal derivation for this function: `_macro_expand_counts` counts
 * `sw $0,SYM($idx)` as one insn, but GNU as assembles it to lui/addu/sw, so the honest
 * text stream is estimated at 63 against the object's 65.  A read-only fork with that
 * third case added is at tmp/grind/func_80060A68/s24/goalmap.py, driven by
 * tmp/grind/func_80060A68/s24/run_perturb.py.  tools/ is not an editable surface for a
 * grind session; if the operator wants it upstream it is a three-line change.
 *
 * The dispatch call keeps the s23 `()` spelling (func_80060B70 at src/text1b.c:3174 and
 * the callee prototype `u8 func_80063AF0(void)` at src/text1b.c:4042); s23 proved it
 * byte-identical to the old `(idx, temp_a1)` spelling on E2.  This body carries ZERO FAKE
 * constructs and re-declares none of the five banned constructs.
 *
 * DISPOSITION.  The function is ACTIVE.  This is a `progress` session: the floor is
 * unchanged at 2 and the ladder is not exhausted.
 */
void func_80060A68(void) {
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    s32 outer;
    s32 idx;
    u16 temp2;
    s32 p10;
    u16 temp_a1;
    s32 result;

    outer = D_800A3468;
    p10 = *(s32 *)(outer + 0x10);
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);
    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(p10 + 4);
    idx = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(void)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))();
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
