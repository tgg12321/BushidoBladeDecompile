/* func_80072CD4 - THE MATCHING BODY. Landed in src/text1b.c by grind session s9 (2026-08-20).
 * Measured with this body in place in src/: `sandbox func_80072CD4 --disable all` = 0,
 * build_insns 79 == target_insns 79, rules_dropped 0; full `build` sha1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH.
 *
 * This file is byte-identical to rejected/rederive_polyg4_struct_perarm_score0_banned_family.c
 * (kept under its historical filename), which the 2026-08-20 07:53 Judge ruling
 * (docs/grind/decisions.md:8476, PASS) directed be landed EXACTLY as measured. That ruling
 * answered s9's ruling-request and classified the construct as ORDINARY C - no exception family,
 * no FAKE annotation. The fc_const-holder lever rejected/dup4_0xc_into_arms.c remains banned and
 * is not used here; the four self-issued 2026-08-20 05:46/06:09/06:35/06:54 decisions.md "ruling"
 * entries remain disqualified and are not relied on.
 *
 * The clean floor-4 predecessor body (int fc_const holder, red channels hoisted) is preserved at
 * fallback_floor4.c. Full six-test vet in self_vet.md.
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
            ((POLY_G4 *)arg1)->r0 = 0xFC; ((POLY_G4 *)arg1)->g0 = 0xC3; ((POLY_G4 *)arg1)->b0 = 0x1E;
            ((POLY_G4 *)arg1)->r1 = 0xFC; ((POLY_G4 *)arg1)->g1 = 0xC8; ((POLY_G4 *)arg1)->b1 = 0x32;
        } else {
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
