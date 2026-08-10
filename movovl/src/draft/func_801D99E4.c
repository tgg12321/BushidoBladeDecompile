/*
 * MOVOVL.EXE game code — func_801D99E4 @ 0x801D99E4 (136B)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   STR stream start: DecDCTReset, DecDCToutCallback, StSetRing, StSetStream, CdControlB, CdRead2 — MovInit/MovOpen-shaped
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

s32 CdControlB(M2C_UNK, s32, M2C_UNK);              /* extern */
s32 CdRead2(M2C_UNK);                               /* extern */
M2C_UNK DecDCTReset(M2C_UNK);                       /* extern */
M2C_UNK DecDCToutCallback(M2C_UNK);                 /* extern */
M2C_UNK StSetRing(s32, M2C_UNK);                    /* extern */
M2C_UNK StSetStream(u8, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
extern void *D_801F65E4;
extern s32 D_801F6618;

s32 func_801D99E4(s32 arg0, M2C_UNK arg1) {
    s32 var_v0;

    DecDCTReset(0);
    DecDCToutCallback(arg1);
    StSetRing(D_801F6618, 0x20);
    StSetStream(M2C_FIELD(D_801F65E4, u8 *, 1), 1, -1, 0, 0);
    var_v0 = 0;
    if (CdControlB(2, arg0, 0) != 0) {
        var_v0 = CdRead2(0x1C0);
    }
    return var_v0;
}
