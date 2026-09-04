/* CD_datasync — SESSION 46 (synthesis).  BEST BANKED FORM.  7 / 91.
 * sandbox CD_datasync --disable all => score 7, target_insns 91, build_insns 91
 * (engine-confirmed twice this session: directly, and as fake_ablate's
 * keep-all cell).  Harness name: z_iv_ik_w2 (tmp/grind/CD_datasync/s46).
 *
 * WHY THIS REPLACES THE OLD candidate.c (the m011 chassis, banked as
 * alt_m011_f1only_7.c) — IT IS THE FIRST FORM IN 46 SESSIONS THAT HOLDS TWO
 * OF TARGET'S THREE WINDOW FEATURES AT ONCE.
 * =========================================================================
 * s45 left the residual as a HARD FORK on one C variable (the order of the
 * two tbl_125c value statements):
 *   value order arg4-then-arg5 -> leaf lbu order idx0-first (TARGET), but
 *                                 arg2's lui/lw pair at 51/52   (WRONG)
 *   value order arg5-then-arg4 -> arg2's pair at 43/44 (TARGET), but the
 *                                 leaf lbu order swapped        (WRONG)
 * s46 BROKE that fork.  The lever is the one s45 named and never swept:
 * give the two arguments DIFFERENT STATEMENT COUNTS, so that "the first
 * statement in the block" and "the first completed value" stop being the
 * same event.  Here arg4 is a TWO-statement chain (index read, then table
 * read) and arg5 is a two-statement scaled-offset chain whose FIRST
 * statement is interleaved ahead of arg4's — pattern B A A B:
 *      i5 = idx_1494[1];      <- arg5's index read, first statement
 *      i4 = idx_1494[0];      <- arg4's index read
 *      val4 = tbl_125c[i4];   <- arg4's value completes FIRST
 *      k5   = i5 * 4;
 * That makes arg4 the first COMPLETED value (which fixes the leaf lbu order
 * to target's idx0-first) while arg5's index read is the first STATEMENT
 * (which puts arg2's lui/lw pair at target's slots 43/44).  Measured over
 * 680 whole-function compiles this session; 48 + 62 of them score 7 and
 * exactly 24 hold both features.  Every one of the 680 forms with arg4
 * spelled as a named VALUE local and arg5 spelled as a scaled offset /
 * pointer / index chain lands in this class.
 *
 * WINDOW NOW (build idx | target, `!!` = differs):
 *   41 !! lbu  $v0,0($s1)     | lbu  $a0,0($s1)   <- idx0 FIRST (feature F1 OK,
 *   42 !! lbu  $v1,1($s1)     | lbu  $v0,1($s1)      register naming only)
 *   43    lui  $a1,%hi(S)     | lui  $a1,%hi(S)   <- MATCH (feature F2)
 *   44    lw   $a1,%lo(S)($a1)| lw   $a1,%lo(S)($a1) <- MATCH
 *   45    sll  $v0,$v0,2      | sll  $v0,$v0,2    <- MATCH (new vs s45)
 *   46    addu $v0,$v0,$s0    | addu $v0,$v0,$s0  <- MATCH (new vs s45)
 *   47 !! sll  $v1,$v1,2      | sll  $a0,$a0,2
 *   48 !! addu $v1,$s0,$v1    | lw   $v1,0($v0)
 *   49 !! lw   $a3,0($v0)     | lui  $v0,%hi(S)
 *   50 !! lui  $v0,%hi(S)     | lbu  $v0,%lo(S)($v0)
 *   51 !! lbu  $v0,%lo(S)($v0)| addu $a0,$a0,$s0
 *   52 !! lw   $v1,0($v1)     | sll  $v0,$v0,2
 *   53 !! sll  $v0,$v0,2      | addu $v0,$v0,$s3
 *   54 !! addu $v0,$v0,$s3    | sw   $v1,16($sp)
 *   55 !! sw   $v1,16($sp)    | lw   $a2,0($v0)
 *   56 !! lw   $a2,0($v0)     | lw   $a3,0($a0)
 *
 * THE REMAINING RESIDUAL, RE-ATTRIBUTED (this is the s46 headline):
 * Target completes arg5's (stack argument) address chain FIRST and issues
 * arg4's `lw $a3` as the LAST memory reference of the block (slot 56);
 * we issue `lw $a3` at 49.  Call that feature F3.  Across all 680 forms F3
 * is decided ENTIRELY by arg4's spelling: 150/150 forms whose arg4 value is
 * loaded INLINE in the printf call (scaled offset `k`, `ik`, or fully
 * inline) hold F3; 0/530 forms whose arg4 value is a NAMED LOCAL hold it.
 * F1&F2 hold only in the named-local set.  The two sets are disjoint, so the
 * fork moved from "value-statement order" onto "arg4 value named vs loaded
 * in the call" — and target's `sw $v1,16($sp)` at slot 54 was held by NONE
 * of the 680 (observed slots: 47, 50, 51, 53, 55).
 *
 * WHY THAT IS NOT A SCHEDULING QUESTION ANY MORE.  The sched_solver
 * object-goal path was made to run on this function for the first time this
 * session (`perturb.py --target-object build/src/system.o --ours-object
 * tmp/sandbox/CD_datasync/system.o`; hon->tgt alignment 85 equal / 5 replace
 * / 1 moved out of 91).  It localises the whole residual to ONE block —
 * pass 2, block 3, 21 insns — and to ONE instruction: target picks UID 105
 * (luid 8) seven slots earlier in the backward pick order than we do.  That
 * move is NOT a tie-break the scheduler could have taken: it violates two
 * dependence edges in our own graph, deps[119] and deps[130] both listing
 * [105, 14].  Kind 14 is an anti/output dependence, i.e. a WAR edge created
 * by REGISTER REUSE — our window recycles $v0/$v1 across the two index
 * chains where target keeps idx0 in $a0.  sched2 runs after reload, so those
 * registers are an INPUT to it.  The residual is therefore a register-
 * assignment fact wearing an ordering costume, and the next attack is the
 * seat of the idx0 chain ($a0 vs $v0), not another statement permutation.
 *
 * The do{}while(0) wrapper is unchanged and load-bearing: fake_ablate this
 * session scored keep-all 7 / build_insns 91 vs drop-1 31 / 75.  It carries
 * the same sanctioned FAKE annotation (owner ruling 2026-07-06,
 * .claude/rules/do-while-zero-exception.md).
 *
 * S47 ADDENDUM (solver modality) - THE SEAT IS NOW DERIVED, NOT GUESSED.
 * inverse_compose classify (object path) types the residual RA (identical
 * register-blanked multisets, six insns differing only in register names).
 * extract.py shows only 5 pseudos reach GLOBAL alloc, so the window is
 * LOCAL-alloc territory; local_extract.py's block-3 table maps one-to-one
 * onto the window and the goal is qty0 (this form's arg4/idx0 address chain,
 * first_reg 95, [8,20), refs 12) moving $v0 -> $a0 with qty1 following it
 * $v1 -> $v0.  ra_solver's stock local search calls that unreachable, but
 * only because its LIVE_EXTEND atom offers death+1/+2/+4 and the vector
 * needs +8; from the bounds-widened fork (tmp/grind/CD_datasync/s47/
 * inverse_extend.py) the same goal is REACHABLE with 176 depth-2 vectors,
 * all of the form "qty0 dies later (20->28+)" + "qty1 dies earlier
 * (24-><=22)".  Spelling the first half alone (arg4 loaded inline in the
 * call) was measured: the chain does become last-allocated and lands in
 * $v1, because nothing occupies $v1 across its span.  The missing input is
 * target's arg5 VALUE living in $v1 from its load across the arg3 address
 * chain to the sw 16($sp) - i.e. feature F4 and the $a0 seat are the same
 * requirement.  arg3's own spelling is NOT the lever for it: 906 compiles
 * this session, every named-arg3 form >= 8, sw slot 54 never reached.
 *
 * S48 ADDENDUM (forensics) - THE MISSING QUANTITY IS DECIDED BY sched1, NOT
 * BY ANY arg5 SPELLING.  s47 left the goal as "make the arg5 VALUE span the
 * arg3 address chain" (feature F4), to be spelled by arg5's C form.  s48
 * swept that axis on the QUANTITY TABLE rather than the score, as the
 * frontier demanded: 32 forms, arg4 in {inl,k,ik} x arg5 in {v,iv,pv,pd} x
 * every statement interleaving, each compiled through the instrumented cc1
 * with BB2_QTY_DEBUG.  28 of 32 - every form with a named arg4 statement -
 * emit a BYTE-IDENTICAL block-3 table (q0 [8,14)->$v0, q1 [14,18)->$v0,
 * q2 [20,36)->$v1, q3 [22,34)->$v0).  The arg5 VALUE quantity is q1 and it
 * dies at luid 18 in ALL of them, four luids before the arg3 chain is born
 * at 22.  Only arg4='inl' perturbs the table at all, and those forms are
 * 13/14.
 *
 * The dumps name the pass.  At expand (system.rtl) the stack-argument store
 * `sw val5,16($sp)` is insn 128 and sits LATE - between the arg4 address
 * addu (126) and the argument-register moves (130-136), which is exactly
 * target's slot-54 position.  The FIRST scheduling pass (sched.c
 * schedule_block, which runs BEFORE local_alloc) hoists it to sit directly
 * behind its only producer; the post-.sched order of the block is
 *   89 93 97 99 101 130 [128] 105 119 109 122 124 126 116 134 136 132 138
 * So the arg5 value's live range is collapsed to 2 luids before local_alloc
 * ever runs.  F4 is a sched1 placement fact.
 *
 * The solver was also re-run against the F3-family model, as the s47
 * frontier's second item asked (tmp/grind/CD_datasync/s48/inv_f3.txt).  On
 * that chassis the goal collapses from s47's four-quantity vector to the
 * single atom {qty2: $v1 -> $a0}, and it is REACHABLE at DEPTH 1 with 14
 * distinct vectors at cost 2 - the two live_extend families being "qty0
 * dies later (14->24)" and "qty2 born earlier (20->18..12)".  s48 spelled
 * the second one: arg4's element address as a named pointer local moves
 * qty2's birth 20 -> 16 and gives it a [16,36) span (39 forms swept), and
 * it STILL takes $v1, because only q3 overlaps it.  $a0 needs TWO
 * quantities alive across [20,36) and allocated first, and the only
 * candidate for the second is the arg5 value that sched1 shortens.
 *
 * MEASUREMENT HYGIENE: the s46/s47 sweep harness reports `n= lev= nop=`;
 * the engine score is lev + (nop - 3) for this chassis, NOT lev.  Reading
 * lev alone makes every F3-family form look like a 7.  Both s48 sweeps are
 * re-scored with the nop column (tmp/grind/CD_datasync/s48/res.json).
 */
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

loop:
    v0 = VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    /* FAKE: do{}while(0) - loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of.
     * mechanism: flow.c loop_depth ref-weighting -> global.c allocno_compare.
     * lever-exhaustion: memory/grind/CD_datasync/hypotheses.md (s9 H37/H38). */
    do {
        s32 i5;
        s32 k5;
        s32 i4;
        s32 val4;
        puts(&g_str_cd_timeout);
        i5 = idx_1494[1];
        i4 = idx_1494[0];
        val4 = tbl_125c[i4];
        k5 = i5 * 4;
        printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], val4, *(s32 *)((u8 *)tbl_125c + k5));
        CD_flush();
    } while (0);
    v0 = -1;
    goto check;

success:
    v0 = 0;

check:
    if (v0 != 0) {
        return -1;
    }
    if (*D_800A14C0 & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
