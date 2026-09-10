//@sub extern s32 D_800A3558;|||extern s16 D_800A3558;
//@sub extern s32 func_80069898(s32 a0, s32 *p, s32 mode);
|||
/* REPRESENTATION BANNER: this file is a CANDIDATE, not the state of main.  On main
 * func_80070C70 is committed as INCLUDE_ASM("asm/funcs", func_80070C70) per the 2026-08-19
 * asm-until-matched ruling.  Install with tmp/grind/func_80070C70/s15/setup15.py (which
 * rebuilds src/text1b.c from tmp/grind/func_80070C70/s14/text1b.pristine.c with the two
 * //@sub companion edits above plus the array declarations) followed by
 * tmp/grind/func_80070C70/s15/inst.py <this file>.
 *
 * ============================ S15 (synthesis): 22 -> 7 / 194 ============================
 * THE MERGE.  s12 measured the TOP-TEST (for-loop) chassis at 31 and priced it out as
 * "9 worse than the do/while 22, because its three combine-orphaned spill pseudos cost
 * 24 frame bytes".  s13 - one session LATER, and only ever applied to the do/while chassis -
 * discovered that the object at sp+0x48 is a `u16 rect[]` and that the do/while chassis needs
 * it OVERSIZED to rect[16] (0x20 bytes) to reach the target's vars=80 frame.
 * Those two findings had never been put together.  They cancel exactly:
 *   do/while chassis:  rect must supply 0x20 bytes   (rect[16], oversized => a FAKE)
 *   top-test chassis:  the 3 orphan spill slots ALREADY supply 24 of those bytes, so rect
 *                      must supply only 8 => `u16 rect[4]` - the HONEST, evidence-backed
 *                      declaration (func_80069898 at src/text1b.c:5413 takes u16 * and reads
 *                      arg1[0..3]; both on-main callers declare exactly `u16 rect[4]`).
 * MEASURED: top-test + rect[4] = 17/194 (vs 31 with the 0x20 icon, vs 22 for the do/while).
 * The "frame penalty" that foreclosed the top-test chassis for four sessions was never a
 * penalty - it was the compiler supplying, for free, the 24 bytes the do/while chassis had to
 * fake.  And the top-test chassis is the one that gets cse_set_around_loop (cse.c:7933 needs
 * REG_LOOP_TEST_P, set only by jump.c:2253 duplicate_loop_exit_test).
 *
 * 17 -> 7: the loop bound's ADDITION ASSOCIATION.  With the exit test now duplicated into a
 * guard block, the target's guard computes ((1 + D_800A35B0) + D_800A3558); every spelling
 * measured before s15 computed ((D_800A35B0 + D_800A3558) + 1).  `1 + D_800A35B0 +
 * D_800A3558` (and `D_800A35B0 + 1 + D_800A3558`) = 7/194; the parenthesised
 * `D_800A35B0 + ((s16)D_800A3558 + 1)` costs an extra insn (196) and 21 points.
 *
 * WHAT IS LEFT AT 7 - two pure INSTRUCTION-ORDER ties, register allocation is now EXACT
 * (tmp/grind/func_80070C70/s15/ours.txt vs s13/tgt.txt; the target's $a1 = D_800A35B0,
 * $a2 = lhu D_800A3558, $a0 = the sll index are all reproduced, and so are the lhu/lh/lw
 * opcode triple and the sum-first mode test that s14 proved unreachable on the do/while side):
 *   (1) 2 insns - the long-standing prologue tie, `addiu a0,sp,24` scheduled before (target)
 *       vs after (ours) `move s0,zero`.
 *   (2) 5 insns - the guard block's schedule.  Target:  move s0,zero / lhu a2,0(gp) /
 *       lw v0,24(s1) / lw a1,0(gp) / addiu v0,v0,12 / sw v0,24(s1) / lh v0,0(gp).
 *       Ours:      lw v0,24(s1) / move s0,zero / addiu v0,v0,12 / sw v0,24(s1) /
 *                  lhu a2,0(gp) / lw a1,0(gp) / lh v0,0(gp).
 *       Same multiset, same registers; the target interleaves the three gp loads into the
 *       `*(arg0+0x18) += 0xC` chain's latency slots and we emit that chain first.  This is
 *       sched.c rank_for_schedule (tools/gcc-2.7.2/sched.c:2408) - INSN_PRIORITY first, then
 *       the three-way dependence class against last_scheduled_insn.
 *
 * AXES SWEPT AND EXHAUSTED ON THIS NEW CHASSIS THIS SESSION (all void-and-re-measured from
 * the do/while ledger, 100 spellings):
 *   - rect size: 2/4/5/6/8/10/12/15/16 -> {2,4}=17, everything >=5 = 31.  rect[4] is both the
 *     optimum AND the correct declaration.  s16 rect[4] is also 7.
 *   - mode-test operand order x bound parenthesisation (20 cells): sum-first NOW WINS by 5
 *     (it lost by 8 on the do/while) - the exact reversal s14 predicted would follow from the
 *     register seats.
 *   - bound association (12 cells): `1 + D_800A35B0 + D_800A3558` / `D_800A35B0 + 1 +
 *     D_800A3558` = 7; every other association 17-21.
 *   - (s16) casts on D_800A3558 at both use sites: BYTE-NEUTRAL under the s16 extern, so the
 *     candidate carries none (cleanest spelling).
 *   - `s32 var_s3 = 0xA;` carrier for the first loop's prim.code: byte-neutral -> dropped.
 *   - position of `prim.p_geom = *(s32 *)(ctx_or_var_s2 + 8);` (3): last is optimum (49/50).
 *   - position of the second loop's `var_s0 = 0` (7 positions incl. hoisted out of the for):
 *     completely inert, all 7.
 *   - first-loop body order (6): ABC unique optimum (others 9-15).
 *   - second-loop body order (24 permutations): PMLC unique optimum (next best 12).
 *
 * THE ONE REMAINING FAKE: `s32 c60 = 0x60;`.  Re-measured on THIS chassis - the literal
 * spelling is 14/191, the constant-holder local is 7/194.  Still load-bearing, still needs
 * .claude/rules/named-local-fake-exception.md + a /* FAKE *\/ annotation at submission.
 * `u16 rect[16]` - the OTHER FAKE the do/while chassis needed - IS NOW GONE: rect[4] is the
 * honest declaration and it is the optimum here.
 *
 * ============================ S16 (enumerate): floor HELD at 7 / 194 ============================
 * 317 spellings swept (tmp/grind/func_80070C70/s16/v1,v2,v4,v5,v6 + the .json histograms); this
 * body is the unique optimum on every axis.  Dead axes, with their costs: the guard region's
 * local spelling (a named temp for the increment 8, p_geom via a named g 25, any hoisted bound
 * or mode-test local >= 42); the mode-test operand order (8) and clause order (15); the four
 * other bound associations (>= 15); all 13 source positions of the entry-block var_s0 = 0 and
 * the for-/while-loop rewrites of the FIRST loop (all inert at 7); all 120 permutations of the
 * five scalar declarations plus 9 positions each of rect/c60/prim (all inert at 7, or 45 when
 * the frame layout breaks); every explicit {s16,u16,s32} carrier for D_800A3558/D_800A35B0
 * read before the increment store (best 23).
 *
 * WHY THE GUARD BLOCK DOES NOT MOVE (read from tmp/grind/func_80070C70/dumps/text1b.sched2,
 * slice saved as s16/f.sched2): insns 479 (lhu a2 = D_800A3558), 482 (lw a1 = D_800A35B0) and
 * 480 (lh v0 = D_800A3558) each carry a DATA dependence on insn 253, the sw v0,24(s1) store of
 * *(arg0 + 0x18) += 0xC - GCC 2.7.2's sched_analyze cannot disambiguate a symbol_ref MEM from a
 * (plus (reg s1) 24) MEM, so the loads are PINNED after the store.  The scheduler is obeying a
 * dependence, not breaking a tie; the target's a1/a2 loads precede that store, so the question
 * is where loop.c inserts the invariant hoists relative to the pre-loop store.  The OTHER tie
 * (insn 49 `s0 = 0` vs insn 71 `a0 = sp + 24`, a pure swap) is a genuine rank_for_schedule
 * decision and is the one BB2_PRIO_DEBUG (sched.c:1504) can answer directly.
 */
void func_80070C70(s32 arg0) {
    s32 c60 = 0x60;
    PrimC70 prim;
    u16 rect[4];
    s32 ctx_or_var_s2;
    s32 var_s0;
    s32 t;
    u8 code;
    s32 g;

    prim.zero10 = 0;
    prim.mode = 0;
    prim.zero1C = 0;
    prim.width = 0x100;
    prim.height = 0x100;
    prim.byte28 = 0;
    ctx_or_var_s2 = (s32)*(s32 **)(D_800A35A8 + 0x64);
    prim.zero1C = 0;
    prim.mode = 0;
    g = *(s32 *)(ctx_or_var_s2 + 4);
    var_s0 = 0;
    t = g + 0xC;
    prim.p_geom = g;
    prim.p_static = t;
    prim.link = *(s32 *)(arg0 + 0x10);
    prim.code = 1;
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    rect[2] = 0xE7;
    rect[0] = 0xCC;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898((GameObj *)arg0, (u16 *)rect, 1);
    g = *(s32 *)(ctx_or_var_s2);
    t = g + 0x48;
    prim.p_geom = g;
    prim.p_static = t;
    do {
        prim.mode = var_s0 << 6;
        prim.link = *(s32 *)(arg0 + 0x10);
        prim.code = 0xA;
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
        var_s0 += 1;
        prim.p_geom += 0xC;
    } while (var_s0 < 6);
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 0x28, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 8);
    for (var_s0 = 0; var_s0 < 1 + D_800A35B0 + D_800A3558; var_s0++) {
            s32 ctx = var_s0 * 3;
            code = D_800A3560[ctx];
            if ((code != 5) && (code != 16)) {
                g = prim.p_geom;
                t = g + 0xC;
                prim.p_static = t;
                prim.p_static = t + (D_800A3590[var_s0] << 4);
                if (((D_800A35B0 + D_800A3558) != 0) || (D_800A35BC == 2)) {
                    prim.mode = 0x50 + var_s0 * 0x16C;
                } else {
                    prim.mode = 0x105;
                }
                prim.link = *(s32 *)(arg0 + 0x10);
                prim.code = 1;
                *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
            }
    }
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    func_80070F78(arg0, (s32 *)&prim);
    func_8006ECF4(arg0);
    func_80072E10(arg0);
    func_80073200(arg0);
}
