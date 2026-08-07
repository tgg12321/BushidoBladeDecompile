/*
 * MOVOVL.EXE game code — func_801D9964 @ 0x801D9964 (128B, frameless leaf)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   pure computation helper (no calls)
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

extern void *D_801F65E4;

void func_801D9964(void) {
    s16 var_a0;

    M2C_FIELD(D_801F65E4, s8 *, 2) = 0;
    M2C_FIELD(D_801F65E4, s8 *, 3) = 0;
    M2C_FIELD(D_801F65E4, s8 *, 4) = 0;
    M2C_FIELD(D_801F65E4, s16 *, 0xC) = 0;
    M2C_FIELD(D_801F65E4, s16 *, 0xE) = 0;
    M2C_FIELD(D_801F65E4, s16 *, 0x14) = 0;
    M2C_FIELD(D_801F65E4, s16 *, 0x16) = 0;
    M2C_FIELD(D_801F65E4, u8 *, 1) = 1U;
    var_a0 = 0x10;
    M2C_FIELD(D_801F65E4, s16 *, 0x1C) = 0;
    M2C_FIELD(D_801F65E4, s16 *, 0x1E) = 0;
    if (M2C_FIELD(D_801F65E4, u8 *, 1) != 0) {
        var_a0 = 0x18;
    }
    M2C_FIELD(D_801F65E4, s8 *, 0) = 0;
    M2C_FIELD(D_801F65E4, s16 *, 0x20) = var_a0;
    M2C_FIELD(D_801F65E4, s16 *, 8) = 0x140;
    M2C_FIELD(D_801F65E4, s16 *, 0xA) = 0xF0;
}
