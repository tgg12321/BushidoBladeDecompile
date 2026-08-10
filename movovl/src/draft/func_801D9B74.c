/*
 * MOVOVL.EXE game code — func_801D9B74 @ 0x801D9B74 (152B)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   VLC-decode one frame: func_801D9A6C + DecDCTvlc + StFreeRing
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

M2C_UNK DecDCTvlc(s32, s32);                        /* extern */
M2C_UNK StFreeRing(s32);                            /* extern */
M2C_UNK printf(M2C_UNK *);                          /* extern */
s32 func_801D9A6C();                                /* static */
extern M2C_UNK D_801D8838;
extern void *D_801F65E4;
extern s32 D_801F6628;

void func_801D9B74(void) {
    s32 temp_v0;

    temp_v0 = func_801D9A6C();
    if (temp_v0 == 0) {
        printf(&D_801D8838);
        M2C_FIELD(D_801F65E4, s8 *, 6) = 2;
        return;
    }
    M2C_FIELD(D_801F65E4, u8 *, 2) = (u8) (M2C_FIELD(D_801F65E4, u8 *, 2) == 0);
    DecDCTvlc(temp_v0, D_801F6628 + (M2C_FIELD(D_801F65E4, u8 *, 2) * 0x28000));
    StFreeRing(temp_v0);
}
