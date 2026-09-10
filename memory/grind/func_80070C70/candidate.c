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
 *   in .claude/rules/no-new-park-categories.md); it needs a /* FAKE */ annotation if that
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
