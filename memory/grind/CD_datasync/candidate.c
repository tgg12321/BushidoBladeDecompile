/* s54 UPDATE (2026-09-04, synthesis).  FLOOR UNCHANGED at 2 / 91, but the
 * BODY CHANGED in one review-relevant way: the `pp` pointer alias is now a
 * plain declaration-with-initializer (`void **pp = &D_800F19C0;`) instead of
 * a mid-block assignment carrying a redundant `(void **)` cast.  D_800F19C0
 * is ALREADY declared `extern void *D_800F19C0;` (src/system.c:526), so
 * `&D_800F19C0` is already `void **` and the cast was pure noise - it was the
 * single line the driver's DECLARATION-PUN auto-scan flagged as a layer-1
 * hazard.  Measured byte-inert: a_base 2/91, b_nocast 2/91, d_declinit_top
 * 2/91 (tmp/grind/CD_datasync/s54/forms).  The alias now matches the
 * pointer-alias rule's own idiom verbatim (`Type* t = &g_Thing;`).
 *
 * s54 SYNTHESIS RESULT - THE SEAT/ORDER COUPLING IS NOW READ OFF THE DUMPS,
 * NOT INFERRED, AND ONE OF THE TWO STANDING FRONTIER ITEMS IS DEAD:
 *   Block-3 local-alloc quantity tables of all three banked bases (s53's
 *   dumps2 / dumps4 / dumps7, re-read this session):
 *     score-2 base : qty1 reg94 b12 d22 r4 -> ord3 got $a0 | qty2 reg86 b18 d24 r4 -> ord2 got $v1
 *     score-4 base : qty1 reg95 b14 d22 r4 -> ord3 got $a0 | qty2 reg88 b18 d24 r4 -> ord2 got $v1
 *     order-exact  : qty1 reg97 b16 d22 r4 -> ord2 got $v1 | qty2 reg86 b18 d24 r4 -> ord3 got $a0
 *   TARGET seats are arg5-value -> $v1 and arg4-address -> $a0, i.e. arg5's
 *   quantity must be SORTED FIRST.  arg4 is quantity number 1 and arg5 is
 *   quantity number 2 on ALL THREE bases - INVARIANT under every window
 *   ordering measured in 2000+ compiles.  Since qty_compare_1's tie-break is
 *   literally `return *q1 - *q2` (local-alloc.c:1683), the tie at birth 16 is
 *   ALWAYS won by arg4.  s51's/s53's frontier item "renumber the quantities so
 *   the tie resolves the other way" is therefore killed on measurement, not on
 *   theory: no C form has ever moved those two numbers, and arg5's birth is
 *   pinned at 18 while arg4's is the only free variable (12/14/16).
 *   What survives is escape (b) only: arg4-address death > 22 (its death is
 *   invariant at 22 = its own `lw`, one slot before arg5's `sw` at 24, in
 *   SCHED1's numbering - note the FINAL emission has the sw at 59 and the lw
 *   at 61, so the two passes genuinely disagree on that pair and the span can
 *   move without disturbing a single final slot).
 *
 * s54 also re-ran the mandated FAKE kill re-audit on the ORDER-EXACT base
 * (s53 had only ablated the score-2 base): keep-all 7/91, drop pp 16/92,
 * drop do{}while(0) 32/74, drop both 32/74.  Neither FAKE unit masks the
 * contested quantities on either base; every inherited order-lever instance
 * kill stands.
 */
/* s53 UPDATE (2026-09-04, structural).  BODY UNCHANGED - still the floor at
 * 2 / 91 (re-verified live: score 2, build_insns 91, target_insns 91,
 * rules_dropped 0; mandated FAKE re-audit keep-all 2, drop-pp 11 / 92,
 * drop-do{}while(0) 32 / 74, so both FAKE units stay load-bearing).  s53 read
 * the block-3 local-alloc quantity tables of all THREE banked bases and turned
 * the order/seat coupling into an arithmetic identity: the arg5-VALUE quantity
 * is invariant (birth 18, death 24, refs 4 => pri 13333) and the arg4-ADDRESS
 * quantity's death is invariant at 22, so its pri is 8000 / 10000 / 13333 for
 * births 12 (this body) / 14 (score-4) / 16 (order-exact).  Seats are correct
 * iff arg4's birth <= 15; the emission order is correct iff arg4's index-scale
 * sll outranks arg5's address addu on INSN_LUID, which happens ONLY at birth
 * 16 - exactly the priority tie that loses the seats on quantity number
 * (local-alloc.c:1659-1684).  Four arithmetic escapes follow; s53 killed two
 * of them on the order-exact base with 360 whole-function compiles: arg3
 * spelling/position (220 forms, best 7) and the arg5 value carrier / refs
 * merge (44 forms, every carrier regresses 8-12), plus arg2 spelling and pp
 * position (96 forms, best 7).  The live escape is (b): lengthen the
 * arg4-address quantity by making `lw $a3` die LATER IN SCHED1'S OUTPUT - note
 * sched1 and sched2 disagree here (sched1 has lw $a3 at 22 before sw 16($sp)
 * at 24; the final emission has sw at 59 and lw $a3 at 61), so that span can
 * move without disturbing the final order at all.
 */
/* s52 UPDATE (2026-09-04, structural).  BODY UNCHANGED - still the floor at
 * 2 / 91 (re-verified live this session: score 2, build 91, target 91,
 * rules_dropped 0).  s52 swept 903 whole-function compiles across four
 * structural axes and killed all four on this chassis: (1) sibling arg5
 * spellings x every chain interleaving (200 forms) - nothing below 2, but it
 * found a THIRD BASE at 4 / 91, banked as
 * progress/s52-third-base-seats-exact-sll-half-sunk-4.c, whose window is
 * SEAT-EXACT like this one AND has arg4's `sll $a0` already sunk from slot 50
 * to 51 (target 52), leaving only the leaf-lbu transposition and that one
 * slot; (2) named intermediates for arg2's value / arg3's index (309 forms) -
 * `a1v` destroys the pp lever (2 -> 7 everywhere), `a2i` is inert late and
 * costs 5 early; (3) splitting arg4's index and address across two locals to
 * decouple birth order from emission order (96 forms) - uniformly 7, and
 * local-alloc.c:1660-1684 says why (priority = floor_log2(refs)*refs*size /
 * (death-birth), tie on quantity number; a pseudo's birth IS its setting insn,
 * so order and seats cannot be decoupled by naming); (4) type narrowing of
 * every carrier (48 forms) - byte-inert except `s8`, which costs a sign-extend.
 * Also newly measured: the `pp` statement POSITION is byte-inert at all 100
 * matched pairs.  Next session inherits a three-body bracket, not a two-body
 * one: score 2 (seats + leaf order, sll at 50), score 4 (seats + sll at 51,
 * leaf transposed), score 7 (order-exact, seats swapped).
 */
/* s51 UPDATE (2026-09-04, rederive).  THIS BODY IS UNCHANGED and remains the
 * floor at 2 / 91 (re-verified live this session: score 2, build 91, target 91,
 * rules_dropped 0).  s51 transplanted CD_ready's s69 statement-order lever and
 * found a SECOND, DISJOINT BASE: with arg5's chain emitted before arg4's shift
 * chain the window is EMISSION-ORDER IDENTICAL to target across build idx
 * 46-63 and the whole residual is a two-quantity seat swap ($a0 <-> $v1
 * between arg4's address and arg5's value) - score 7 / 91, banked as
 * progress/s51-order-exact-seats-swapped-7.c.  So the function is a two-body
 * problem: THIS body has the seats and misses the order by one insn; that one
 * has the order and misses the seats.  Killed on the new base: the index
 * carrier variable (fresh local == function-scope v0, byte-identical at all 40
 * matched interleavings), nested do{}while(0) refs weighting (28 forms, all
 * regress), declaration order (48 permutations, all byte-identical at 7), and
 * arg4 chain shape (16 forms, inert or worse).  Pass attribution is read, not
 * guessed: both scheduling passes decide the transposition at the SAME tie -
 * `RANKDBG last=114 y=112 cls=3 x=104 cls2=3 val=0`, resolved by
 * INSN_LUID (tools/gcc-2.7.2/sched.c:2463).
 */
/* CD_datasync - SESSION 50 (rederive).  BEST BANKED FORM.  2 / 91.
 * sandbox CD_datasync --disable all => score 2, target_insns 91, build_insns 91
 * (engine-confirmed this session, rules_dropped 0).
 * Harness name: c_a2_b2_w2_pp1 (tmp/grind/CD_datasync/s50/forms_c).
 *
 * THE 49-SESSION PLATEAU AT 7 IS BROKEN: 7 -> 4 -> 2 IN ONE SESSION, AND THE
 * LEVER CAME FROM A SIBLING LEDGER, NOT FROM THIS ONE.
 * =========================================================================
 * WHAT THIS SESSION DID.  Mandated modality was `rederive`.  The three named
 * re-derivation sources were checked against the ledger first: fresh m2c is
 * spent (s21 - its accumulator tail is a regression, its argument expressions
 * are the fully-inline 13-attractor), the 5-project reference corpus is spent
 * (s8), and the psyz/PsyQ-4.0 axis is a verified dead end
 * (research-psyz-axis-2026-08-19.md: psyz leaves CD_datasync as INCLUDE_ASM).
 * The one un-spent source was the SIBLING LEDGER.  CD_datasync's ledger last
 * looked at its siblings in s16/s20, when CD_sync (splat `cpu_side_move_dir_4`)
 * stood at 7/160 with "the identical residual".  CD_sync has since ground its
 * own floor to 2/160 and SOLVED the shared printf window.  Nothing propagated
 * that back here.  memory/grind/CD_sync/candidate.c carries the answer.
 *
 * THE TRANSPLANTED WINDOW (CD_sync's spelling, symbol-translated):
 *   - arg2 (D_800F19C0) is read through a POINTER-ALIAS LOCAL `void **pp`
 *     instead of inline.  THIS IS THE WHOLE FIRST HALF OF THE WIN.
 *   - arg4's element ADDRESS is carried in an s32 INTEGER local built by
 *     split-init (`t0 = idx[0]; t0 *= 4; t0 = (s32)((u8 *)tbl_125c + t0);`)
 *     and dereferenced at the call site as `*(s32 *)t0`.  Every previous
 *     session spelled that address as an `s32 *` POINTER local (s48/s49's
 *     ip/ikp/dp family, measured 10) or as a named VALUE (measured 7); the
 *     integer-typed address had never been written in 49 sessions.
 *   - arg3 stays on the `tbl_11dc` pointer local.  CD_sync spells arg3 as the
 *     direct global `D_800A11DC[D_800A11D5]`; transplanting THAT costs three
 *     instructions of build (bi 91 -> 88) and scores 16-21.  Keep the pointer.
 *
 * MEASUREMENTS (all engine sandbox, all bi=91 unless stated):
 *   pp ABSENT, integer-address arg4   : best 8   (16 forms at 8, 4 at 10)
 *   pp PRESENT, integer-address arg4  : best 4   (80 of 100 interleavings)
 *   pp PRESENT + arg5 as a named value from an index local (b2/b3/b4): 2
 *   pp added to the OLD s46 chassis (named val4 arg4): 7 - no change.
 * So `pp` is worth 4 instructions, and it is worth them ONLY in the
 * integer-address chassis: the two levers are not additive, they are a pair.
 *
 * WINDOW NOW (build idx | target) - the residual is ONE INSTRUCTION MOVED:
 *   46    lbu  $a0,0($s1)      | lbu  $a0,0($s1)     <- F1 held
 *   47    lbu  $v0,1($s1)      | lbu  $v0,1($s1)
 *   48    lui  $a1,%hi(S)      | lui  $a1,%hi(S)     <- F2 held
 *   49    lw   $a1,%lo(S)($a1) | lw   $a1,%lo(S)($a1)
 *   50 !! sll  $a0,$a0,2       | sll  $v0,$v0,2
 *   51 !! sll  $v0,$v0,2       | addu $v0,$v0,$s0
 *   52 !! addu $v0,$v0,$s0     | sll  $a0,$a0,2
 *   53    lw   $v1,0($v0)      | lw   $v1,0($v0)
 *   54    lui  $v0,%hi(D_800A11D5)  | same
 *   55    lbu  $v0,%lo(D_800A11D5)($v0) | same
 *   56    addu $a0,$a0,$s0     | addu $a0,$a0,$s0
 *   57    sll  $v0,$v0,2       | sll  $v0,$v0,2
 *   58    addu $v0,$v0,$s3     | addu $v0,$v0,$s3
 *   59    sw   $v1,16($sp)     | sw   $v1,16($sp)    <- F4 HELD (slot 54 of
 *   60    lw   $a2,0($v0)      | lw   $a2,0($v0)        the old numbering -
 *   61    lw   $a3,0($a0)      | lw   $a3,0($a0)     <- F3 HELD, LAST memref)
 * Every register name matches.  F1, F2, F3 and F4 - the four window features
 * the ledger has tracked since s45, never previously held together - are ALL
 * held simultaneously.  The entire remaining residual is that our `sll $a0`
 * (arg4's index scaling) is emitted at slot 50 and target emits it at 52,
 * i.e. one insn has to sink past arg5's `sll $v0` + `addu $v0,$v0,$s0` pair.
 * Score 2 = one transposition counted at both endpoints.
 *
 * WHAT WAS MEASURED DEAD AGAINST THAT LAST TRANSPOSITION (s50, 652 whole-
 * function compiles on this chassis, all with pp present):
 *   - Statement ORDER is inert: all interleavings of arg4's and arg5's
 *     statement chains (272 d-forms) hold 2 or regress; the extremes
 *     (whole arg5 chain first, whole arg4 chain first) both score 2.
 *   - arg4 chain SHAPE is inert at 2: `t0 *= 4` vs `t0 <<= 2` vs `t0 = t0<<2`
 *     vs folding the scale into the add (`t0 * 4` inside the address
 *     expression) vs `t0 += (s32)tbl_125c` vs `(s32)tbl_125c + i4 * 4`.
 *   - arg4 DEREF shape is inert at 2: `*(s32 *)t0` == `((s32 *)t0)[0]`.
 *     Re-typing the address back to `s32 *p4` REGRESSES (>=7) - the integer
 *     type is load-bearing, not cosmetic.
 *   - Reading idx[0] through a second `u8 *` base local, and leaving arg4's
 *     value fully inline in the call with only the scaled index named, both
 *     regress.
 *   - arg5 in {named value from index local, index*4 + integer base, fully
 *     inline scaled expression} are byte-equivalent at 2; the `ix <<= 2`
 *     split-shift form of arg5 costs 2 more (score 4).
 *
 * FAKE CONSTRUCTS AND THEIR MEASURED WEIGHT:
 *   - do{}while(0) wrapper: unchanged from s9, still load-bearing.  s50
 *     re-ran the mandated kill re-audit with tools/fake_ablate.py on the
 *     CLOSEST banked instance kill (the F3-holding r_ik_iv_w0 form, s48):
 *     keep-all 8 / bi 92, drop-1 31 / bi 75.  The wrapper is worth 24
 *     instructions and is not masking the lever; that instance kill stands.
 *   - `void **pp` pointer alias to D_800F19C0: worth 4 instructions in this
 *     chassis (measured pp0 best 8 vs pp1 best 4 over 100 matched forms).
 *     Family: C-level pointer alias to a global,
 *     .claude/rules/pointer-alias-fake-exception.md.  In-repo precedent for
 *     this exact symbol and shape: memory/grind/CD_sync/candidate.c.
 *
 * FRONTIER FOR s51 - the whole function is now ONE sunk instruction.
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
        s32 arg5;
        s32 i5;
        s32 t0;
        void **pp = &D_800F19C0; /* FAKE: pointer-alias staging the D_800F19C0 load early; mechanism: local-alloc.c update_equiv_regs refs-2 sink defeat; lever-exhaustion: memory/grind/CD_datasync/hypotheses.md s50 */
        puts(&g_str_cd_timeout);
        i5 = idx_1494[1];
        t0 = idx_1494[0];
        t0 *= 4;
        arg5 = tbl_125c[i5];
        t0 = (s32)((u8 *)tbl_125c + t0);
        printf(&D_800161C8, *pp, tbl_11dc[D_800A11D5], *(s32 *)t0, arg5);
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
