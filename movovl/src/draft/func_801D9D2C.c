/*
 * MOVOVL.EXE game code — func_801D9D2C @ 0x801D9D2C (152B, frameless leaf)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   pure computation helper (no calls)
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

extern void *D_801F65E4;
extern M2C_UNK D_FFFFFFF;

void func_801D9D2C(void) {
    M2C_UNK *var_a1;

    if (M2C_FIELD(D_801F65E4, u8 *, 0) == 0) {
        var_a1 = &D_FFFFFFF;
        do {
            if (var_a1 == NULL) {
                M2C_FIELD(D_801F65E4, u8 *, 4) = (u8) (M2C_FIELD(D_801F65E4, u8 *, 4) == 0);
                M2C_FIELD(D_801F65E4, u16 *, 0x1C) = (u16) M2C_FIELD((D_801F65E4 + (M2C_FIELD(D_801F65E4, u8 *, 4) * 8)), u16 *, 0xC);
                M2C_FIELD(D_801F65E4, u16 *, 0x1E) = (u16) M2C_FIELD((D_801F65E4 + (M2C_FIELD(D_801F65E4, u8 *, 4) * 8)), u16 *, 0xE);
            }
            var_a1 -= 1;
        } while (M2C_FIELD(D_801F65E4, u8 *, 0) == 0);
    }
    M2C_FIELD(D_801F65E4, u8 *, 0) = 0U;
}
