/* func_80072CD4 — PENDING-RULING body (grind session s13, 2026-09-01).
 *
 * STATUS: NOT a candidate. This file is parked here, NOT in candidate.c, because the
 * per-arm duplication it carries is still listed in memory/grind/func_80072CD4/state.json
 * `banned_constructs` from the 2026-08-20 layer-1 FAIL wave. Submitting it would trip
 * grindlib.check_banned_constructs and discard the session before any reviewer saw it.
 * s13 therefore emitted `ruling-request`, not `candidate-ready`. Do not apply this to
 * src/text1b.c until a Judge clears those entries.
 *
 * MEASUREMENT (s13, current chassis, this session):
 *   `& tools/wteng.ps1 main sandbox func_80072CD4 --disable all`
 *   -> score 0, build_insns 79 == target_insns 79, rules_dropped 0, cheat_asm_stripped 166.
 *   Raw: tmp/grind/func_80072CD4/s13/sandbox_perarm_score0.json.
 *   The body is identical to rejected/rederive_polyg4_struct_perarm_score0_banned_family.c
 *   except for the two FAKE annotations required by prerequisite 4 of
 *   .claude/rules/duplicated-statement-into-arms.md, which the earlier submissions omitted.
 *
 * FAMILY CLAIM: duplicated-statement-into-arms (.claude/rules/duplicated-statement-into-arms.md),
 * a FROZEN-list sanctioned family. Prerequisite audit:
 *   1. statement is REAL on its path — YES. `r0 = 0xFC` / `r1 = 0xFC` are live colour
 *      components of the POLY_G4 the function is initialising; the target's own bytes store
 *      0xFC at +0x04 and +0x0C. Not a dead store.
 *   2. byte-neutrality — YES, in the strongest available form: the emitted function is
 *      byte-identical to the target (sandbox 0, 79 == 79). The duplication materialises NO
 *      extra instruction; jump2's cross_jump re-merges the copies.
 *   3. lever-exhaustion documented — YES: twelve sessions across five modalities
 *      (structural s5/s6, synthesis s7/s8, forensics s9, escalation s10/s11/s12) in
 *      memory/grind/func_80072CD4/hypotheses.md; 35 banked rejected forms; the s12 cc1psx
 *      calibration probe killing the toolchain-fidelity hypothesis.
 *   4. FAKE annotation — present below (this was MISSING from every prior submission).
 *   5. layer-1 + layer-2 review — pending, gated on the ruling this session requests.
 *
 * PRECEDENT (in-hand, re-verified from source this session, not "same spirit"):
 *   sotn-decomp master @ aa53500226ee84be763f3e8702b27de06456b3a7,
 *   src/main/psxsdk/libspu/s_sca.c:131 and :135 (SpuSetCommonAttr) — matched Sony PsyQ
 *   library C, zero INCLUDE_ASM, PSX per config/splat.us.main.yaml:266 — writes the
 *   IDENTICAL statement `cnt = _spu_RXX->rxx.spucnt;` as the first statement of BOTH arms of
 *   an if/else where one copy above the join would do, four times in one function (8 copies).
 *   That is the same HEAD-duplication shape as this body's `r0/r1 = 0xFC`.
 *   Verification transcript: tmp/grind/func_80072CD4/s13/sotn_s_sca_head_dup_verification.txt;
 *   owner-lane population census: memory/grind/func_80072CD4/census-2026-09-01.md.
 */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} POLY_G4;

s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            /* FAKE: r0/r1 = 0xFC duplicated into this arm instead of written once above the
             * join, mechanism: jump2 cross_jump (toplev.c:3142, runs AFTER sched2 at :3117) so
             * the re-merged common head keeps the merge-block slot the sunk merge-block stores
             * cannot reach, lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md s5-s12 */
            ((POLY_G4 *)arg1)->r0 = 0xFC; ((POLY_G4 *)arg1)->g0 = 0xC3; ((POLY_G4 *)arg1)->b0 = 0x1E;
            ((POLY_G4 *)arg1)->r1 = 0xFC; ((POLY_G4 *)arg1)->g1 = 0xC8; ((POLY_G4 *)arg1)->b1 = 0x32;
        } else {
            /* FAKE: r0/r1 = 0xFC duplicated into this arm instead of written once above the
             * join, mechanism: jump2 cross_jump (toplev.c:3142, runs AFTER sched2 at :3117) so
             * the re-merged common head keeps the merge-block slot the sunk merge-block stores
             * cannot reach, lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md s5-s12 */
            ((POLY_G4 *)arg1)->r0 = 0xFC; ((POLY_G4 *)arg1)->g0 = 0xC3; ((POLY_G4 *)arg1)->b0 = 0x50;
            ((POLY_G4 *)arg1)->r1 = 0xFC; ((POLY_G4 *)arg1)->g1 = 0xDC; ((POLY_G4 *)arg1)->b1 = 0x46;
        }
        ((POLY_G4 *)arg1)->r2 = 0xFC; ((POLY_G4 *)arg1)->g2 = 0x82; ((POLY_G4 *)arg1)->b2 = 0;
        ((POLY_G4 *)arg1)->r3 = 0x32; ((POLY_G4 *)arg1)->g3 = 0x28; ((POLY_G4 *)arg1)->b3 = 0xA;
    } else {
        ((POLY_G4 *)arg1)->r0 = 0x10; ((POLY_G4 *)arg1)->g0 = 0x30; ((POLY_G4 *)arg1)->b0 = 0x60;
        ((POLY_G4 *)arg1)->r1 = 0x18; ((POLY_G4 *)arg1)->g1 = 0; ((POLY_G4 *)arg1)->b1 = 0x40;
        ((POLY_G4 *)arg1)->r2 = 0x30; ((POLY_G4 *)arg1)->g2 = 0; ((POLY_G4 *)arg1)->b2 = 0x60;
        ((POLY_G4 *)arg1)->r3 = 0; ((POLY_G4 *)arg1)->g3 = 0; ((POLY_G4 *)arg1)->b3 = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
