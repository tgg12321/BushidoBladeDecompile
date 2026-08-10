/*
 * MOVOVL.EXE game code — func_801DA070 @ 0x801DA070 (12B stub)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   gp-relative getter: returns a global (lw $v0, 0x50($gp))
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

extern s32 D_801F6620;

s32 func_801DA070(void) {
    return D_801F6620;
}
