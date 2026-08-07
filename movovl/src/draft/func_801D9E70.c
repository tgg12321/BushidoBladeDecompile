/*
 * MOVOVL.EXE game code — func_801D9E70 @ 0x801D9E70 (296B)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   per-frame MDEC decode pump: CdSync, DecDCTin/DecDCTout, slice upload + vlc-frame helpers — MovWait/MovSync-shaped
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

s32 CdSync(M2C_UNK, M2C_UNK *);                     /* extern */
M2C_UNK DecDCTin(s32, M2C_UNK);                     /* extern */
M2C_UNK DecDCTout(s32, s32, s32, s32);              /* extern */
M2C_UNK printf(M2C_UNK *, M2C_UNK);                 /* extern */
M2C_UNK func_801D9B74();                            /* static */
M2C_UNK func_801D9D2C();                            /* static */
M2C_UNK func_801D9DC4(s32);                         /* static */
extern M2C_UNK D_801D884C;
extern void *D_801F65E4;
extern s32 D_801F6628;

s32 func_801D9E70(s32 arg0) {
    M2C_UNK sp10;
    M2C_UNK var_a1;
    s16 temp_a1;
    s32 temp_lo;
    s32 var_a2;

    var_a1 = 2;
    if (CdSync(1, &sp10) == 5) {
        printf(&D_801D884C, 2);
        M2C_FIELD(D_801F65E4, s8 *, 6) = 2;
        return 0;
    }
    if (M2C_FIELD(D_801F65E4, u8 *, 1) != 0) {
        var_a1 = 3;
    }
    DecDCTin(D_801F6628 + (M2C_FIELD(D_801F65E4, u8 *, 2) * 0x28000), var_a1);
    temp_a1 = M2C_FIELD(D_801F65E4, s16 *, 0x22);
    var_a2 = temp_a1 - 1;
    if (var_a2 < 0) {
        var_a2 = temp_a1 + 0xE;
    }
    temp_lo = M2C_FIELD(D_801F65E4, s16 *, 0x20) * 0x10 * ((var_a2 >> 4) + 1);
    DecDCTout(D_801F6628 + ((M2C_FIELD(D_801F65E4, u8 *, 3) * 0x38400) + 0x50000), temp_lo >> 1, var_a2, temp_lo);
    func_801D9DC4(arg0);
    func_801D9B74();
    func_801D9D2C();
    M2C_FIELD(D_801F65E4, u8 *, 3) = (u8) (M2C_FIELD(D_801F65E4, u8 *, 3) == 0);
    return 1;
}
