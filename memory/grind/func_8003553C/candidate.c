/* func_8003553C - best form as of grind session 6 (FORENSICS); body UNCHANGED
 * since session 3. Honest floor still `sandbox --disable all` = 2 (43/43 insns),
 * re-measured this session with this body in src/code6cac_b2_pre.c.
 *
 * SESSION 6 changed NOTHING in the body and did not move the floor. What it
 * produced is the exact GCC decision that creates the residual, read out of the
 * compiler's own scheduler trace (cc1 -da writes a `;; ready list at T-n` log
 * into base.i.sched). It supersedes the s2 and s3 explanations, both of which
 * were partly wrong, and it converts session 5's empirical law ("the early li
 * and the early store are inseparable") from an 840-form measurement into a
 * theorem about two lines of sched.c.
 *
 * THE MECHANISM, END TO END (dumps: tmp/grind/func_8003553C/s6/rtl_{INC,T,HT,W,D}/)
 *  1. sched1 does NOT reorder this block. Verified on the incumbent and on the
 *     target-statement-order form: base.i.sched is insn-for-insn identical to
 *     base.i.combine. Every emitted reordering is sched2's (post-RA).
 *  2. sched1 DOES relocate ONE kind of insn: a single-set pseudo's definition.
 *     In the holder form (`s16 w = 640;`) the const set survives cse/loop/flow/
 *     combine at the very top of the RTL (with REG_EQUAL), and sched1 moves it
 *     down to sit immediately before its first use. The trace shows exactly why:
 *         ;; ready list at T-11: 78 (2) 75 (2) 9 (7f000001), now 9 78 75
 *     0x7f000001 is LAUNCH_PRIORITY (sched.c:187). schedule_block (sched.c:3985)
 *     stamps the just-scheduled insn with it, and schedule_insn -> adjust_priority
 *     (sched.c:2534-2575) propagates that value to any newly-ready predecessor for
 *     which birthing_insn_p() is true, deliberately "to shorten register lives".
 *     Because schedule_block builds the block BACKWARD (sched.c:3813 "The first
 *     insn scheduled becomes the new tail"), a LAUNCH_PRIORITY insn is emitted
 *     immediately BEFORE the use that just made it ready. That is the "sink".
 *  3. birthing_insn_p (sched.c:2496-2528) returns true for a SET of a REG that is
 *     live at that point IF AND ONLY IF `reg_n_sets[REGNO] == 1`. Our 640 pseudo
 *     always has exactly one set, so the sink is unconditional and no placement,
 *     spelling, declaration order or helper factoring can avoid it.
 *  4. RA then works on that sunk layout. local-alloc allocates by density
 *     (short, busy ranges first), so the 240 and 128 allocnos take $v0; the 640
 *     allocno gets $v1 ONLY when its live range overlaps one of them. With both
 *     640 stores in the post-load group its sunk range lies entirely after them,
 *     so it also gets $v0 (greg: `80 in 2`). With our leading 0x10 store its range
 *     spans the whole block, so it gets $v1 (greg: `75 in 3`) - target's register.
 *  5. sched2 (where birthing_insn_p is disabled: `if (reload_completed == 1)
 *     return 0;`) then computes INSN_PRIORITY as the longest dependence path from
 *     the block HEAD. A `li` with no predecessors has priority 1, the minimum, so
 *     backward scheduling picks it LAST and it lands at the block head - which is
 *     exactly where target has `addiu $v1,$zero,0x280` (block insn 0). But when
 *     the 640 shares $v0 with the other two constants, the shared hard register
 *     chains all three `li`s and their stores into one totally ordered path, and
 *     the 640 `li` inherits that chain's depth - which is why every
 *     target-statement-order form emits it right after `sb $v0,0xE`.
 *
 * WHAT THIS MEANS FOR THE NEXT SESSION
 *   The whole residual now reduces to ONE boolean in sched.c:2516:
 *   `reg_n_sets[<the 640 pseudo>] == 1`. If the 640-valued pseudo had TWO OR MORE
 *   sets, birthing_insn_p returns 0, adjust_priority leaves its priority at 1, and
 *   sched1 leaves the definition at the block head with both uses in the post-load
 *   group - the live range then overlaps the 240/128 ranges, local-alloc is forced
 *   onto $v1, and (per the s3 diagnostic pin, which proved the rest) every store
 *   lands in target's slot. Two spellings of "two sets" were measured this session
 *   and BOTH fail, for different reasons (see rejected/):
 *     * a redundant duplicate `w = 640;` before the second store is deleted by
 *       cse2 - reg_n_sets is back to 1 and the output is bit-identical to the
 *       plain target-order form;
 *     * reusing ONE variable for 240 and then 640 really does give reg_n_sets 2,
 *       but one C variable is one pseudo and therefore ONE hard register, while
 *       target holds 240 in $v0 and 640 in $v1 - output again bit-identical.
 *   So the open question is narrow and precise: is there a pure-C construct that
 *   gives the 640-valued pseudo two surviving sets while keeping it a pseudo
 *   distinct from the 240 and 128 pseudos, at zero instruction cost?
 *
 * Sessions 1-5 headers (levers L1/L2/L3, the killed axes, the 840-form search)
 * are preserved verbatim below.
 *
 func_8003553C — best form as of grind session 5 (permuter); body UNCHANGED
 * since session 3. Honest floor still sandbox --disable all = 2 (43/43 insns).
 *
 * SESSION 5 replaced decomp-permuter (whose objective s4 measured to be
 * ANTI-CORRELATED here) with a CORRECTED-OBJECTIVE search of my own:
 * tmp/grind/func_8003553C/s5/{search_lib,hill,search}.py compile a generated
 * standalone form through the real pipeline and score it as the objdump
 * line-diff against target.o (~1.5 s/form, 8-way parallel), additionally
 * recording the constant-materialisation signature
 * "<idx of li 640>/<idx of li 240>/<idx of li 128>@<640's register>"
 * (target and this body both = 7/8/11@v1).
 *
 * 840 forms were measured across statement order x declaration order x
 * 3 coordinate-store spellings x 2 RGB spellings x 2 OT-base spellings, by
 * exhaustive 1-token-move hill climbing from three chassis (this body, its
 * x3-leading mirror, and TARGET's own statement order) plus 200 uniform-random
 * permutations. Results:
 *   * The honest minimum over all 840 forms is 2 — this body.
 *   * Hill climbing from TARGET's own statement order (difflines 10) converges
 *     in two iterations onto THIS EXACT form, so the basin has a single
 *     attractor and this body is its global optimum.
 *   * DECISIVE: 428 forms reach the target constant signature (li 640 at block
 *     index 7 holding $v1) and EVERY ONE of them contains a pre-OT-load 640
 *     store; NO form in the entire space materialises 640 at the block head
 *     with both of its stores in the post-load group. The early li is CAUSED BY
 *     the early store — they are not separable by ordering, declaration order,
 *     or store spelling. That closes session 4's frontier items 1 and 3, which
 *     both asked for exactly that combination.
 *   * Also killed in s5: the static-helper-parameter axis. A `static void`
 *     helper is NOT inlined by GCC 2.7.2 at -O2 (real call, 19 insns);
 *     `static inline` does inline (43 insns) but the parameter constant is
 *     propagated and then sunk exactly as a local holder is — 640 lands in $v0
 *     at index 13/27 in all four helper partitionings (difflines 12/46/28/18).
 *
 * Original session-4 header follows.
 *
 * func_8003553C — best form as of grind session 4 (permuter); body UNCHANGED
 * since session 3.
 *
 * SESSION 4 (permuter modality) added no floor drop and closed the permuter
 * axis with a measurement: two fresh-seed campaigns (chassis A = this body,
 * permuter base 225; chassis B = target's own statement order, base 270) ran
 * ~17.6k iterations between them and produced ~85 output forms, NONE of which
 * beats this body's objdump difflines of 2 at 43 instructions. The reason is
 * that the permuter's weighted objective is ANTI-CORRELATED with the honest
 * distance here: this body (one instruction displaced by 17 slots, sandbox 2)
 * is charged permuter score 225, while chassis-B forms that are objectively
 * ~5x worse score 50-65 — a long-range displacement reads to the permuter's
 * differ as a large pile of reorderings. See evidence.md section "Session 4".
 * Also killed in s4: the matched sibling func_80072BC4's `*(u8 *)((s32)p + N)`
 * store spelling (inert, difflines 10 = control 10).
 *
 * Original session-3 header follows.
 *
 * func_8003553C — best form as of grind session 3 (structural).
 * Honest pure-C floor: sandbox --disable all = 2, unchanged by session 3 (was 4
 * entering s2, 17 entering s1). Applied in src/code6cac_b2_pre.c. Zero
 * regfix/asmfix rules, zero inline asm, zero pins, zero dead stores, zero
 * volatile.
 *
 * SESSION 3 added no floor drop but identified the residual's mechanism, which
 * changes what the next attempt should aim at (full detail in evidence.md):
 * the misplaced `sh $v1,0x10` is a REGISTER-ALLOCATION consequence, not an
 * ordering one. Target holds 640 in $v1 for the whole block; when the 640 stores
 * are late (target's source order) sched1 sinks the constant's set down next to
 * them, local-alloc then reuses $v0 (dead since the 128 constant's last use),
 * and the resulting REG_DEP_OUTPUT against `li $v0,128` pins the `li 640`
 * mid-RGB-block. A DIAGNOSTIC-ONLY pin of 640 to $3 (never committable) makes
 * every store land in target's slot, including this one. Three axes were killed
 * with measurements: the OT-base-load spelling (2/2), MEM_IN_STRUCT_P on the
 * HImode stores (2/8/9/8 — the barrier really does dissolve and nothing moves,
 * because the scheduler is movement-minimizing), and extending another
 * constant's live range to force a second register (6/11/11).
 *
 * The primitive is a POLY_G4 (0x24 bytes) allocated by bump-pointer D_800A38B4:
 *   +0x04..06 rgb0, +0x08/0A x0,y0, +0x0C..0E rgb1, +0x10/12 x1,y1,
 *   +0x14..16 rgb2, +0x18/1A x2,y2, +0x1C..1E rgb3, +0x20/22 x3,y3.
 * Full-screen gradient quad: rgb0/rgb1 = (0,0,0x80), rgb2/rgb3 = black,
 * corners (0,0) (640,0) (0,240) (640,240); linked into OT slot
 * D_800A374C + 0x401C.
 *
 * THREE measured levers are baked into the shape below (evidence.md); do NOT
 * "tidy" any of them back into ascending field order:
 *   L1  ot_Link's first argument is hoisted into the local `ot` and computed
 *       BETWEEN the RGB byte block and the trailing coordinate stores. GCC
 *       2.7.2 treats the lw of D_800A374C as may-alias with the stores through
 *       `p`, so the load's emitted position is pinned to its source position
 *       relative to those stores, and that position also splits the stores into
 *       a pre-load group and a post-load group that cannot be reordered across
 *       it. Measured 14 -> 10 (s1).
 *   L2  the x1 (0x10 = 640) store leads the body, ahead of the two 240 stores.
 *       An li is hoisted only a few slots above its FIRST USE, so a 640-valued
 *       store must sit in the pre-load group for `li $v1,640` to head the block
 *       and for 640 to stay live in $v1 across the whole body (target's shape).
 *       Measured 10 -> 4 (s1).
 *   L3  the primitive pointer is copied into `q` and `p` is advanced BEFORE the
 *       ot_Link call, with the global store `D_800A38B4 = p;` after it. Both
 *       values are then simultaneously live, so GCC emits `move $a1,$s0` for the
 *       argument, `addiu $s0,$s0,0x24` in the pre-jal slot, and the `sw` to the
 *       global after the call — exactly target. (Advancing the GLOBAL early
 *       drags the sw before the jal; `next = p + 0x24;` staged in a plain extra
 *       local is coalesced back into $s0 and keeps the addiu after the delay
 *       slot. Both were measured dead in s1.) Measured 4 -> 2 (s2).
 *
 * Residual 2 = ONE misplaced instruction, for the next session:
 *   R1  our `sh $v1,0x10($s0)` is emitted at block position 6 (right after
 *       li $v0,128, i.e. as late as the scheduler will sink it inside the
 *       pre-load group); target emits it in the post-load group, between
 *       `sh zero,0x0A` and `sh zero,0x12` (ascending field order). Everything
 *       else in the function — prologue, both %hi/%lo pairs, the whole RGB
 *       block in field order, the move/addiu/jal/sw tail, the epilogue — is
 *       already instruction-for-instruction identical to target.
 */
void func_8003553C(void) {
    u8 *p;
    u8 *q;
    u32 *ot;

    p = (u8 *)D_800A38B4;
    initPolyG4(p);
    *(s16 *)(p + 0x10) = 640;
    *(s16 *)(p + 0x1A) = 240;
    *(s16 *)(p + 0x22) = 240;
    p[4] = 0;
    p[5] = 0;
    p[6] = 0x80;
    p[0xC] = 0;
    p[0xD] = 0;
    p[0xE] = 0x80;
    p[0x14] = 0;
    p[0x15] = 0;
    p[0x16] = 0;
    p[0x1C] = 0;
    p[0x1D] = 0;
    p[0x1E] = 0;
    ot = (u32 *)(D_800A374C + 0x401C);
    *(s16 *)(p + 8) = 0;
    *(s16 *)(p + 0xA) = 0;
    *(s16 *)(p + 0x12) = 0;
    *(s16 *)(p + 0x18) = 0;
    *(s16 *)(p + 0x20) = 640;
    q = p;
    p += 0x24;
    ot_Link(ot, (u32 *)q);
    D_800A38B4 = p;
}
