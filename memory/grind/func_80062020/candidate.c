/* s14 SECOND-RUN BANNER (synthesis, 2026-09-03) - READ THIS FIRST; it supersedes the
 * banner below it on two points.
 *
 * 1. FLOOR RE-MEASURED on the live chassis with this body pasted over the INCLUDE_ASM at
 *    src/text1b.c:3932: sandbox func_80062020 --disable all = score 4, target_insns 38,
 *    build_insns 35, rules_dropped 0, cheat_asm_stripped 165.  Seventh consecutive session
 *    at floor 4.  src/text1b.c restored to HEAD immediately afterwards.  This body remains
 *    the best UNCONTESTED form.
 *
 * 2. THE HELPER SPLIT IS DEAD BY RULING.  The first s14 run's ruling-request on the
 *    `static __inline__` helper split was ANSWERED and REFUSED.  The standing Judge
 *    constraint now reads: "The address expression for the terminator row may not be
 *    materialised twice by any means, including a function-call/inline boundary between the
 *    writes.  Do not respell the once-called static __inline__ helper split (one helper or
 *    two, any names, any parameterisation, macro or decayed-array variants)."  Point 2 of
 *    the banner below is therefore OBSOLETE: memory/grind/func_80062020/pending-ruling/
 *    holds dead forms, NOT promotable ones.  Do not resubmit them in any spelling.
 *
 * 3. PASS RE-ATTRIBUTION (the s14 second-run finding).  The deciding pass is RTL EXPAND -
 *    not cse2 (s8's attribution) and not combine's LOG_LINK availability (s13's).  With this
 *    body applied, `pwsh tools/grinder/dump.ps1 func_80062020` shows all three terminator
 *    MEMs already based on one pseudo in tmp/grind/func_80062020/dumps/text1b.rtl (the FIRST
 *    -da dump); in the banned dual-spelling body the column-a store is already
 *    (set (mem:SI (plus:SI (reg 74) (symbol_ref "D_800F1198"))) (const_int 0)) at expand,
 *    with no address pseudo ever created.  GO_IF_LEGITIMATE_ADDRESS at
 *    tools/gcc-2.7.2/config/mips/mips.h:2286 accepts REG + CONSTANT_ADDRESS verbatim, so a
 *    `&SYM + index` C tree becomes the MEM address directly while a pointer-local deref
 *    becomes a displacement off that pointer's pseudo.  Consequence: each store's address
 *    form is fixed by its OWN C address tree, so the target's mix (two shared-pseudo stores
 *    at DISP8/DISP4 plus one inline-symbolic store, same symbol, same index) requires two
 *    different address trees for the same lvalue base - the banned construct.  Banked as a
 *    CLASS kill with that predicate cite.  There is no pass-level lever left to look for.
 *
 * 4. SPECIES IS A SPECIES OF ONE.  tmp/grind/func_80062020/s14/species_sameobj.py re-scanned
 *    the whole original EXE: 5 functions co-locate a register-materialised shared base
 *    (>=2 non-zero displacements) with an at-form %lo indexed ref, and func_80062020 is the
 *    ONLY one where both forms address the SAME symbol.  s9's "32-function species" was
 *    built on the at-form alone; func_80061064 / CD_cw / SpuSetReverbModeParam are NOT
 *    instances, so the "solve a simpler sibling first" frontier item is retired.
 */
/* s14 BANNER (synthesis, 2026-09-03) — READ THIS FIRST.
 * 1. Floor re-measured on the live chassis with this body in place: sandbox
 *    func_80062020 --disable all = score 4, target_insns 38, build_insns 35,
 *    rules_dropped 0, cheat_asm_stripped 165.  Sixth consecutive session at floor 4.
 *    This body remains the best UNCONTESTED form and candidate.c deliberately still holds it.
 * 2. BUT s14 measured TWO DISTANCE-0 BODIES that carry no dead statement and no
 *    same-body dual spelling — the first honest forms ever to reach the target arrangement.
 *    They are NOT here because their construct class (a `static __inline__` helper used once,
 *    which after inlining supplies the second single-use address def H-s13-8's law requires)
 *    is a first reach: not in the frozen sanctioned family list, not in the forbidden catalog.
 *    They live at memory/grind/func_80062020/pending-ruling/ and the s14 outcome JSON carries
 *    the ruling question.  Both were verified: score 0, 38/38 insns, and the two-helper form
 *    also passed a full verify-oracle (build_sha1 == 62efab4f...bb4fa, build_matches true).
 *    If the ruling is favourable, promote pending-ruling/s14-two-helper-split-score0.c
 *    verbatim — it needs no further search.
 * 3. s14 also ran the mandated kill re-audit of the s13 score-0 dead-conditional body.
 *    tools/fake_ablate.py refuses it ("no FAKE-annotated constructs found" — the banked copy
 *    carries no markers), so the ablation grid was spelled by hand
 *    (tmp/grind/func_80062020/s14/sweep14.py, shapes A0..A3): the identical-arms conditional
 *    ALONE gives DISP8|DISP4|DISP0 and the pointer re-assignment ALONE gives DISP8|DISP4|DISP0;
 *    only both together reach the target.  The s13 kill stands, re-confirmed on this chassis.
 */
/* s13 RE-MEASURE (synthesis, 2026-09-03, second run): this body, pasted over the
 * INCLUDE_ASM line at src/text1b.c:3932, measures sandbox func_80062020 --disable all
 * = score 4, target_insns 38, build_insns 35, rules_dropped 0, cheat_asm_stripped 165.
 * It remains the best UNCONTESTED form. src/ was reverted to HEAD after the measurement.
 * Do NOT replace it with the dead-conditional body that measures 0 - that body is a cheat
 * and is banked at rejected/epilogue-deadcond-identical-arms-crossjump-score0-s13.c. */
/* MIGRATION NOTE (operator, 2026-09-01): main carries INCLUDE_ASM for this
 * function (asm-until-matched, owner ruling 2026-08-19). Any "HEAD"/"main"
 * state claims in the comments below predate that migration and describe the
 * chassis they were measured on, not the committed tree - verify against the
 * dated evidence.md facts before relying on them. */
/* func_80062020 (src/text1b.c) - CANDIDATE (best UNCONTESTED form), honest floor 4.
 *
 * MIGRATION BANNER (asm-until-matched, owner ruling 2026-08-19): func_80062020 is
 * represented on main as `INCLUDE_ASM("asm/funcs", func_80062020);` at src/text1b.c:3932.
 * This body was pasted over that line during grind s11 ONLY to re-measure the floor, and
 * src/text1b.c was restored to HEAD immediately afterwards. It is NOT applied to main.
 *
 * MEASURED (grind s11, 2026-08-31, live chassis, this body in place):
 *   sandbox func_80062020 --disable all -> score 4, build_insns 35, target_insns 38,
 *     rules_dropped 0, cheat_asm_stripped 166 (all from OTHER functions in text1b.c)
 *   Identical to the digit to the s8/s9/s10 measurements - the chassis is stable across
 *   four sessions and every banked chassis-relative conclusion remains spendable.
 *
 * WHY THIS BODY AND NOT THE DISTANCE-0 BODY. The distance-0 body (mixed epilogue: row[1]
 * and row[2] through the pointer, column 0 through the full `*(s32 *)((u8 *)&D_800F1198 +
 * ofs)` expression) reached distance 0 and a full-build oracle SHA1 match, and the ordered
 * adjudication mandated by owner ruling 6a (docs/grind/decisions.md:14836) and unblocked by
 * the owner ruling of 2026-08-31 was then RUN TO COMPLETION on it: Judge 2026-08-31 22:25
 * FAIL (comment-only defect), the comments were fixed, and layer-1 2026-08-31 22:31 FAILed
 * the fixed submission ON THE MERITS ("the exact construct already FAILed by two prior
 * Judges; deleting the incriminating comments does not change what the code does"). The
 * construct is now a mechanically-enforced entry in state.json banned_constructs, and
 * respelling a banned construct is the same construct. Ruling 6a's own "proof fails" branch
 * therefore governs: fidelity-limited INCLUDE_ASM at floor 4. The distance-0 body is banked
 * at rejected/judge-fail-0831-2224.c and rejected/layer1-fail-0831-2231.c; do not resubmit
 * it, in any spelling, absent a new owner grant.
 *
 * WHAT THIS BODY COSTS. It emits `sw $zero,0($v0)` for column a where the target emits the
 * LO_SUM lui/addu/sw triple - 35 build insns against a 38-insn target, residual 4. It is
 * clean pure C: 0 rules, 0 pins, 0 volatile, 0 dead vars, one uniform tree shape for all
 * three terminator stores, no construct that any reviewer has objected to across s1-s11.
 *
 * RE-MEASURED (grind s12, 2026-09-01, live chassis, this body pasted over the INCLUDE_ASM
 * at src/text1b.c:3932 and then restored): sandbox func_80062020 --disable all -> score 4,
 * build_insns 35, target_insns 38, rules_dropped 0. Fifth consecutive session at floor 4, no
 * chassis drift. s12 executed the owner's 2026-09-01 Ruling-A named probe (cse2's code-free
 * skip/abandon gates) and KILLED it on six grounds; func_80062020 is FORECLOSED by the
 * 2026-07-27 standing ruling (docs/grind/decisions.md, 2026-09-01 entry). This remains the
 * best admissible form.
 */
void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    s32 *p;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
        *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 4);
        i = i + 1;
        *(s32 *)((u8 *)&D_800F119C + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 8);
        *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    ofs = i + i;
    ofs = ofs + i;
    ofs = ofs << 2;
    p = (s32 *)((u8 *)&D_800F1198 + ofs);
    p[2] = 0;
    p[1] = 0;
    p[0] = 0;
}
