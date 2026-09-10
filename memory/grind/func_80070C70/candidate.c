/* [s10 forensics 2026-09-10] STILL THE FLOOR at 39, and s10 measured a NEW reason to prefer it:
 * this body reproduces the target FRAME EXACTLY - 0 spill slots,
 * ".frame $sp,128 # vars= 80, regs= 6/0, args= 24" - as do all six do/while declaration variants.
 * Every top-test chassis spends 24 extra bytes on three orphaned combine `(use (reg))` pseudos that
 * regclass types ST_REGS and reload can only give stack slots (see hypotheses.md H-s10-1/H-s10-6).
 * s9 ranked the top-test chassis as the shorter road; on the frame axis that is wrong. This body is
 * missing exactly ONE thing: the cse_set_around_loop hoist of the D_800A3558 / D_800A35B0 reads out
 * of the second loop into its guard.  That is the frontier.
 */
/* [s9 solver 2026-09-10] FLOOR UNCHANGED at 39; this do/while body is still the SCORE floor
 * and is byte-for-byte the s8 body.  What s9 changed is the strategic ranking and one axis:
 *   1. RE-MEASURED FIRST: this body scores exactly 39 at 194 insns on the current HEAD chassis.
 *   2. THE SOLVER LAYER IS TYPED AND EMPTY.  tools/ra_solver/goal_from_tgt.py classify (the
 *      object-based entry point - inverse_compose.py classify refuses zero-rule functions)
 *      reports FIRST DIVERGENCE: PRE-RA / "next tool: none" for this body AND for both
 *      top-test bodies.  No RA seat search and no scheduler search can move this function at
 *      its present distance; the work is C-level structure.
 *   3. THE 16-BIT-READ SPELLING AXIS IS NOW A CLASS KILL, not 30 coincidences.
 *      tools/gcc-2.7.2/config/mips/mips.md:2346 - extendhisi2's expander does
 *      `if (optimize && GET_CODE (operands[1]) == MEM) operands[1] = force_not_mem (...)`
 *      and then emits ashl+ashr by 16.  Under -O the one-insn memory pattern that would emit
 *      a bare `lh` is unreachable from expand, so the declared type and the per-site cast of
 *      D_800A3558 (and of D_800A3590[]) never reach codegen.  Verified in the .rtl dump on a
 *      body with `extern s16 D_800A3558;` read bare.  Do not re-run declaration sweeps.
 *   4. THE TOP-TEST CHASSIS IS NOW THE SHORTER ROAD, and score was hiding it.  See
 *      memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c: it scores 49 (one WORSE
 *      than chassis-toptest-cse-48.c) but its whole object-level shape residual is the 13
 *      prologue/epilogue insns that differ only by the 24-byte frame overshoot plus one
 *      `addiu #,#,12` against one `nop`.  The 48-point body still has `lw x2` where the
 *      target has `lhu x2`.  Read the s9 F12 frontier in hypotheses.md before touching this
 *      do/while body again.
 */
/* [s8 structural 2026-09-10] FLOOR UNCHANGED at 39; the body below is still the best form.
 * s8 spent the structural modality on frontier F7 (kill the three orphaned `(use (reg))`
 * pseudos that cost the top-test chassis 24 frame bytes) and on F8 (the two-live-register
 * `prim.p_geom + 0xC` sites).  Both are now measured dead on this chassis:
 *   - F8 is BACKWARDS.  Computing p_static before storing p_geom (`s32 g = *(s32*)(ctx+4);
 *     prim.p_static = g + 0xC; prim.p_geom = g;`) costs a point on BOTH chassis (39->40
 *     do/while, 49->50 top-test) and doing it at both pre-loop sites costs two (41 / 51).
 *     The target's insn order is not reachable by re-ordering these two source statements.
 *   - The exit-test spelling axis is exhausted for the frame.  Every one of 13 further
 *     D_800A3558 declaration x per-site-cast combinations (including the ASYMMETRIC forms
 *     s7 never tried, which is what the target's `lhu $a2` + `lh $v0` pair looks like)
 *     scores 49 with the IDENTICAL spill triple p116/p165/p170 at vars=104.
 *   - Carrying the two loop values in C locals (read at the top of the body, read at the
 *     bottom of the body, or used in the bound too) is 42/52/59/60/63 - all worse than 39.
 *   - Partial hoists of the bound (n = (s16)D_800A3558 + 1, n = D_800A35B0 + 1, ...) are
 *     54-62; the full hoist is 60 and buys a 7th callee-saved register.
 * The best top-test variant is now 48, not 49: INLINING `t` in the body (writing
 * prim.p_static = prim.p_geom + 0xC twice instead of through the named local `t`) is worth
 * a point there - and costs two on the do/while chassis (39->41).  Banked as
 * memory/grind/func_80070C70/chassis-toptest-cse-48.c.
 * NOTE FOR THE NEXT SESSION: p116 is now IDENTIFIED (s7 only guessed).  It is the `ashift`
 * intermediate of the (s16) sign-extension inside the loop's OWN tail exit test
 * (.jump insn 267: `(set (reg/s:SI 116) (ashift (reg/s:SI 114) (const_int 16)))`), and 165
 * is the same insn in the guard copy.  All three orphans are combine's distribute_notes
 * emitting `(use (reg))` at a CODE_LABEL (combine.c:10834-10840) because the folded insn
 * left a REG_DEAD note with nowhere to go.  See the s8 section of hypotheses.md.
 */
/* [s7 synthesis 2026-09-10] The body below is STILL the floor (39) and is unchanged.
 * BUT the strategic chassis changed: see memory/grind/func_80070C70/chassis-toptest-cse-49.c.
 * On a top-test `for` spelling of the second loop, cse_set_around_loop DOES fire and
 * reproduces the target's loop-carried $a1/$a2 structure exactly (guard loads / body uses
 * registers / tail reloads).  Its whole 49 is 14 prologue+epilogue diffs from the 24 extra
 * frame bytes (three orphaned `(use (reg))` pseudos 116/165/170) plus ~4 seat diffs.  If
 * those three slots die, that chassis goes well below 39.  Read the s7 section of
 * hypotheses.md before spending anything on this do/while body.
 */
/* candidate.c - func_80070C70 - session 6 (enumerate). Honest floor 39 (was 53 at s5).
 *
 * FLOOR HISTORY: 194 (HEAD, no C body) -> 101 (s1) -> 99 (s3) -> 56 (s4) -> 53 (s5) -> 39 (s6).
 * Instruction count is now 194 == the target's 194, and the FRAME MATCHES EXACTLY
 * (.frame $sp,128 / vars= 80, regs= 6/0, args= 24 -- identical to the target's prologue
 * and epilogue offsets). Every remaining difference is register seating / scheduling.
 *
 * THE BODY BELOW IS ORDINARY C EXCEPT FOR ONE CONSTRUCT THAT STILL NEEDS A FAMILY VET:
 *   `s32 ctx = var_s0 * 3;` -- a fresh, block-scoped, once-written / once-read named
 *   intermediate whose value IS consumed in the target's bytes (it is the byte OFFSET
 *   giv `addiu $s2,$s2,3` that the target re-adds to %hi(D_800A3560) every iteration).
 *   It is worth 4 points and 1 instruction on its own (53 -> 49 on the s5 for-loop
 *   chassis) and it is what lets loop.c reduce D_800A3560's giv to an OFFSET instead of
 *   a full ADDRESS. Before any candidate-ready, vet it against the named-intermediate
 *   6 prongs (.claude/rules/narrow-byte-args-packed-call.md + the 2026-08-17 clarification
 *   in .claude/rules/no-new-park-categories.md); it needs a FAKE annotation if that
 *   family is the right one. Measured alternatives: hoisting `ctx` to function scope is
 *   also 40 (still a named intermediate); a `u8 *p = D_800A3560 + var_s0 * 3;` pointer
 *   local is also 49 on the for chassis; INLINING it (`D_800A3560[var_s0 * 3]` or
 *   `*(D_800A3560 + var_s0 * 3)`) costs 4 points.
 *
 * THE TWO s6 MOVES (both measured, both structural):
 *   1. `s32 ctx = var_s0 * 3;` (see above): 53 -> 49, build_insns 193 -> 194.
 *   2. THE SECOND LOOP GOES BACK TO AN if-GUARDED do/while: 49 -> 43, and the frame
 *      collapses from 152 to the target's 128. s5 had moved it to a top-test `for`
 *      because that is the only shape that makes jump.c:2163 duplicate_loop_exit_test
 *      fire, which in turn is the ONLY way cse.c:7909 cse_set_around_loop can hoist the
 *      loop-head reads into the previous iteration's tail (the test is literally
 *      `REG_LOOP_TEST_P (src_elt->exp)`, cse.c:7936, and only duplicate_loop_exit_test
 *      ever sets that flag).  But the SAME transform is what costs the frame: it hands a
 *      NEW pseudo to every exit-test register whose whole live range sits inside the exit
 *      code (jump.c:2246 `reg_map[REGNO (reg)] = gen_reg_rtx (...)`), combine then deletes
 *      the copies it can fold and parks the orphaned REG_DEAD notes on `(use (reg))` insns
 *      at the following CODE_LABEL (combine.c:10839), and reload's alter_reg gives each of
 *      those three refs-but-never-set pseudos an 8-byte spill slot.  MEASURED DIRECTLY with
 *      the instrumented cc1's BB2_FRAME_DEBUG census (tmp/grind/func_80070C70/s6/framedbg.sh):
 *        FRAMEDBG ctx=stack_temp size=48 frame_offset=48      <- prim
 *        FRAMEDBG ctx=stack_temp size=32 frame_offset=80      <- icon
 *        FRAMEDBG ctx=spill_new_p116 size=8 frame_offset=88   <- the 24 bytes of overshoot
 *        FRAMEDBG ctx=spill_new_p165 size=8 frame_offset=96
 *        FRAMEDBG ctx=spill_new_p170 size=8 frame_offset=104
 *      On the if-guarded do/while the census stops at icon: vars= 80, frame 128, exact.
 *   3. Swapping the `||` operands of the mode test (`(D_800A35BC == 2) || (...)`) is worth
 *      one more point and restores build_insns to 194: 40 -> 39.
 *
 * COMPANION EDITS in src/text1b.c that are part of the measured 39 (unchanged from s5):
 *   1. extern u8  D_800A3560[];   (was: extern u8  D_800A3560;)   both occurrences
 *   2. extern s16 D_800A3590[];   (was: extern s16 D_800A3590;)   both occurrences
 *   3. typedef struct IconC70 { s16 sp48; s16 sp4A; s16 sp4C; s16 sp4E; s16 sp50[12]; } IconC70;
 *      The 0x20 size is now PROVEN independently of any frame guesswork: the BB2_FRAME_DEBUG
 *      census shows prim=48 + icon=32 = vars 80 = the target's `vars= 80` exactly, with no
 *      temps left over.  The `sp50[12]` spelling of the trailing 24 bytes is still a
 *      placeholder for the real member list.
 *   4. D_800A3558 keeps its ordinary `extern s32 D_800A3558;` declaration -- u16 and s16 are
 *      BYTE-NEUTRAL on both the for and the do/while chassis (27-variant sweep, s6/k and s6/h).
 *
 * RESIDUAL AT 39 (194 insns vs target 194 -- a pure seating/ordering residual now):
 *   (i)  The target keeps $a1 = D_800A35B0 and $a2 = lhu D_800A3558 LIVE across the loop:
 *        they are loaded once in the guard block (80070DF4/DFC) and re-loaded in the loop
 *        TAIL (80070ECC/ED0/ED4), and the body consumes them with `sll $v0,$a2,16 / sra /
 *        addu $v0,$a1,$v0`.  We re-load both INSIDE the body every iteration instead.  This
 *        is exactly cse_set_around_loop, and its gate (REG_LOOP_TEST_P) is unreachable on
 *        the do/while chassis -- see the frontier: the two effects have to be reconciled.
 *   (ii) `t = prim.p_geom + 0xC` lands in the SAME register as p_geom (`addiu $v0,$v0,12`)
 *        where the target keeps two live registers (`addiu $v1,$v0,12`), at three sites.
 *   (iii) two scheduling ties: `addiu $a0,$sp,24` vs `addu $s0,$zero,$zero`, and the
 *        position of `sw $s3,0x2C($sp)`.
 *
 * SWEEP EVIDENCE (all measured this session, sandbox --disable all + the vars= gradient):
 *   - 30-variant index x mode-counter x D_800A3558-declaration cross product (s6/struct/):
 *     {49: 6, 52: 6, 53: 6, 56: 6, 88: 3, 90: 3}.  Named index local (i2) or pointer local
 *     (i5) = 49; inlined index (i1/i4) = 53; a comma-initialised second biv (i3) = 88.
 *   - 40-variant bound x condition sweep on the for chassis (s6/bc/), vars= gradient:
 *     every `<` form = 104, every `<=` form = 96, `>=` = 96.  No spelling reaches 80.
 *   - 27-variant declaration x bound x condition sweep on the for chassis (s6/k/): all 104.
 *   - 8 loop-structure variants (s6/g/): if-guarded do/while = vars 80 / score 43; while,
 *     for(;;)+break, for+break, for with `>` reversed = vars 104; `<=`/`!(>)` = vars 96.
 *   - 27-variant declaration x bound x condition sweep on the do/while chassis (s6/h/):
 *     {40: 3, 41: 3, 42: 3, 43: 6, 44: 6, 45: 6}; the winning axis is the body condition
 *     spelling, and the D_800A3558 declaration is byte-neutral in all 9 pairs.
 *   - 10-variant body-spelling sweep (s6/l/): {39: 1, 40: 8, 43: 1}.
 */
void func_80070C70(s32 arg0) {
    s32 c60 = 0x60;
    PrimC70 prim;
    IconC70 icon;
    s32 ctx_or_var_s2;
    s32 var_s0;
    s32 var_s3;
    s32 t;
    u8 code;

    prim.zero10 = 0;
    prim.mode = 0;
    prim.zero1C = 0;
    prim.width = 0x100;
    prim.height = 0x100;
    prim.byte28 = 0;
    ctx_or_var_s2 = (s32)*(s32 **)(D_800A35A8 + 0x64);
    prim.zero1C = 0;
    prim.mode = 0;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 4);
    var_s0 = 0;
    prim.p_static = prim.p_geom + 0xC;
    prim.link = *(s32 *)(arg0 + 0x10);
    prim.code = 1;
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    icon.sp4C = 0xE7;
    icon.sp48 = 0xCC;
    icon.sp4A = 0x25;
    icon.sp4E = 1;
    func_80069898(arg0, (s32 *)&icon, 1);
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    var_s3 = 0xA;
    prim.p_static = prim.p_geom + 0x48;
    do {
        prim.mode = var_s0 << 6;
        prim.code = var_s3;
        prim.link = *(s32 *)(arg0 + 0x10);
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
        var_s0 += 1;
        prim.p_geom += 0xC;
    } while (var_s0 < 6);
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 0x28, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 8);
    if (D_800A35B0 + (s16)D_800A3558 + 1 > 0) {
        var_s0 = 0;
        do {
            s32 ctx = var_s0 * 3;
            code = D_800A3560[ctx];
            if ((code != 5) && (code != 16)) {
                t = prim.p_geom + 0xC;
                prim.p_static = t;
                prim.p_static = t + (D_800A3590[var_s0] << 4);
                if ((D_800A35BC == 2) || (((s16)D_800A3558 + D_800A35B0) != 0)) {
                    prim.mode = 0x50 + var_s0 * 0x16C;
                } else {
                    prim.mode = 0x105;
                }
                prim.code = 1;
                prim.link = *(s32 *)(arg0 + 0x10);
                *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
            }
            var_s0++;
        } while (var_s0 < D_800A35B0 + (s16)D_800A3558 + 1);
    }
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    func_80070F78(arg0, (s32 *)&prim);
    func_8006ECF4(arg0);
    func_80072E10(arg0);
    func_80073200(arg0);
}
