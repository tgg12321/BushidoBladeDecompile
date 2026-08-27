/* s42 ADDENDUM (2026-08-27, rederive).  This form is UNCHANGED and was RE-MEASURED on the
 * live chassis this session: `sandbox func_80057CC8 --disable all` -> score 16,
 * target_insns 111, build_insns 108, rules_dropped 0.  Two s42 results bear on it:
 *   (1) Its register map is SHAPE-INVARIANT.  The in-file sibling idiom transplanted from
 *       the two COMPLETED neighbours func_80048530 (src/text1b.c:312) and func_800611A4
 *       (src/text1b.c:3252) -- an s32 base `vt = *(s32 *)(arg0 + 4)` with ONE reused s16*
 *       cursor instead of this form's asymmetric `table[pi*2]` + `next_vert` pair -- lands
 *       on a register-IDENTICAL disposition table at the same score 16 / 108 insns
 *       (rejected/s42-sibling-idiom-s32-base-reused-cursor-REGEQUIV-score16.c).  The seat
 *       rotation that separates this form from the target is therefore not an artefact of
 *       how this particular source is spelled.
 *   (2) The rival post-call-address regime is now FORECLOSED, not merely behind.  Five
 *       independent post-call forms (s42 e1/g1/h1/h2/h3) all take a NINTH callee-save
 *       ($fp/$s8 for arg3) because the vertex-table base must be carried across the call;
 *       the target saves only $s0-$s7 (asm/funcs/func_80057CC8.s:3-16) and gets away with
 *       eight solely via the banned second materialization `lw $a0,0x4($s2)` (line 50).
 *       So this 108-insn pre-call form is the only ban-compliant regime that even has the
 *       target's callee-save COUNT, and 16 is the ban-compliant floor.
 *   Full argument: hypotheses.md / evidence.md, both under the [s42] headings.
 */
/* BEST MEASURED FORM (grind s38b, 2026-08-27, synthesis modality).
 * MEASURED THIS SESSION on the live chassis: `sandbox func_80057CC8 --disable all`
 * -> score 16, target_insns 111, build_insns 108, rules_dropped 0.  Re-measured a
 * second time with the /* FAKE * / annotation below in place: still 16 / 108.
 *
 * WHAT THIS FORM IS.  It is the s34/s35/s36 score-20 form plus exactly ONE added
 * statement, `next_vert = &Judge;`, placed after `scale = arg0[2] * 40;`, with the two
 * sine-table reads then spelled `*(next_vert + ...)` instead of `*(&Judge + ...)`.
 * Nothing else differs.  The annotation-free score-20 predecessor is preserved at
 * tmp/grind/func_80057CC8/s37/v20.c.
 *
 * FAMILY / RULING STATUS -- SETTLED.  s38 emitted a ruling-request on which sanctioned
 * family covers the added statement; the ruling came back and is recorded in the
 * function's judge_constraints:  the construct is sanctioned ONLY as
 * .claude/rules/staged-value-reused-variable.md.  Do NOT cite
 * .claude/rules/defeat-licm-hoist-var-reuse.md (loop-scoped; this function has no loop)
 * and do NOT cite .claude/rules/pointer-alias-fake-exception.md (its canonical shape is a
 * FRESH local, and s37 measured a fresh local at 20 -- the alias half is provably not the
 * lever).  The ruling requires the bound-4 annotation to name local-alloc.c block-local
 * pre-seating AND to state the liveness argument; the annotation in the body below does
 * both, and bounds 5/6 must still be re-checked at submission time.
 *
 * THE MECHANISM (dump-verified, tmp/grind/func_80057CC8/dumps/text1b.lreg).  The
 * next-neighbour address is a BLOCK-LOCAL call-crossing quantity ("Register 88 used 3
 * times across 4 insns IN BLOCK 4; crosses 1 call"), so local-alloc seats it BEFORE
 * global.c runs.  A second SET of the same C variable in the FINAL basic block removes
 * the "in block 4" tag entirely (lreg then prints "Register 88 used 6 times across 4
 * insns; crosses 1 call" with no block tag), i.e. pseudo 88 becomes a global allocno and
 * local-alloc no longer pre-seats it.  That is the whole four-point gain (20 -> 16) at an
 * unchanged 108 instructions.
 *
 * THE LEVER IS REGIME-SPECIFIC (measured, do not assume it carries).  It is worth ZERO
 * points in the post-call-address regime (s38: 31 with and 31 without) and ZERO points in
 * the arm-selected-address regime (s38b: 21 with and 21 without, both spellings --
 * rejected/s38b-arm-address-basefirst-plus-lever-score21.c and
 * rejected/s38b-arm-address-nocast-twins-plus-lever-score21.c).  It pays only where the
 * address pseudo is genuinely block-local, i.e. the merge-block-offset regime this form
 * uses.
 *
 * THE EXACT REGISTER MAP (s40c, read straight off the greg dump's "Register
 * dispositions" table, tmp/grind/func_80057CC8/s40c/d16.greg -- this CORRECTS the s38b
 * header above it, which reported the score-20 predecessor's map by mistake.  On the live
 * chassis cxs HAS reached $s1; the lever's four points are exactly that).
 *   ours   : $s0 = cys (pseudo 129), $s1 = cxs (119), $s2 = next-ADDRESS (88),
 *            $s3 = arg0 (72), $s4/$s5 = raw cx/cy (83/86), $s6 = arg2 (74),
 *            $s7 = arg3 (75)   (8 callee-saves, 6 quantities cross the call)
 *   target : $s0 = cys, $s1 = cxs,          $s2 = arg0, $s3 = next-INDEX, $s4/$s5 = raw
 *            cx/cy, $s6 = arg2, $s7 = arg3  (8 callee-saves, 6 quantities cross)
 * The full normalised diff (tmp/grind/func_80057CC8/s38b/base16.hon.s vs
 * asm/funcs/func_80057CC8.s) contains NOTHING except (a) that one seat rotation applied
 * to ~12 lines, and (b) the address-formation block: ours is one pre-call `addu $17,$17,$6`
 * where the target has four post-call insns `sll $3,$19,16 / lw $4,4($18) / sra $3,$3,14 /
 * addu $3,$3,$4`.  That is the entire 111-vs-108 instruction gap.
 *
 * WHY IT IS NOT 0 -- THE CLOSED-FORM DILEMMA (s38b, both horns now measured).
 *   Horn 1, PRE-CALL address formation (this form).  Exactly 6 quantities cross the call,
 *   matching the target, so 8 callee-saves suffice and the count is 108.  But sched1 runs
 *   BEFORE local-alloc and sinks the address add into the slot preceding the jal, so the
 *   address pseudo's live_length is 4; its allocno priority floor_log2(n)*n/4 is >= 0.5 for
 *   any n, while arg0's is 2*5/54 = 0.185 over the whole function.  The address therefore
 *   ALWAYS outranks arg0 in global.c:635 allocno_compare and takes $s1 -- exactly the seat
 *   the target gives cxs.  arg0 cannot be raised (its live_length is the whole function and
 *   shortening it makes it stop crossing the call entirely: measured 47) and the address
 *   cannot be lowered (only a scheduling barrier would pin its def, a forbidden family).
 *   Horn 2, POST-CALL address formation (rejected/s38-postcall-address-*.c, 31 @ 110).
 *   Here the seat ORDER is right -- measured map $s0 cys, $s1 cxs (both matching target),
 *   then $s2 table, $s3 arg0, $s4 off -- but SEVEN quantities cross instead of six, because
 *   the vertex-table base and the wrapped offset must both survive the call.  GCC takes a
 *   ninth callee-save ($s8/$fp, `sw s8,56(sp)`), costing +2 instructions, and the extra
 *   `table` allocno displaces arg0 from $s2 to $s3.
 *   The target has it both ways ONLY because it re-derives the base from arg0 after the
 *   call (`lw $a0,0x4($s2)`, asm/funcs/func_80057CC8.s:50) -- arg0 does double duty as the
 *   scale source AND the base source, so the base costs zero extra crossing quantities.
 *   Every ban-compliant substitute for that double duty was measured and costs exactly one
 *   supernumerary crossing quantity: centre-relative (s33, 38), prev-address + delta
 *   (s32 next-differences, 42), carried table + offset (s38, 31).  The residual is
 *   therefore ONE supernumerary live-across-call quantity, and removing it is precisely
 *   the second source-level materialization the owner refused on 2026-07-20.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s32 pi;
    u16 cy;
    s16 *table;
    s16 *next_vert;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        s32 off = tmp * 4;
        if ((s16) tmp >= (s32)arg0[3]) {
            off = 0;
        }
        next_vert = (s16 *)(off + (s32)table);
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    ang_next = ratan2(next_vert[0] - (s16) cx, next_vert[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    /* FAKE: stages the sine-table base address &Judge through `next_vert`, the
     * next-neighbour vertex pointer, whose previous value is dead here (its last
     * read is the `ang_next` ratan2 argument three statements above, and it is
     * never read again after this point), and whose staged value is read by the
     * two statements immediately below; mechanism: local-alloc.c block-local
     * pre-seating -- the second SET removes pseudo 88's `in block 4` tag so
     * local-alloc no longer seats it ahead of global.c, worth 4 points (20 -> 16);
     * lever-exhaustion: memory/grind/func_80057CC8/hypotheses.md (38 sessions) and
     * the s37 negative controls banked as rejected/s37-fresh-jt-pointer-local-
     * no-gain-score20.c, s37-jt-initialised-at-declaration-score48-110insns.c,
     * s37-table-reuse-for-judge-base-no-gain-score20.c */
    next_vert = &Judge;
    *arg2 = cx + ((scale * (s32)(*(next_vert + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(next_vert + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}