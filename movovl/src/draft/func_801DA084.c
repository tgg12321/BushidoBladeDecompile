/*
 * MOVOVL.EXE game code — func_801DA084 @ 0x801DA084 (16B stub)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   _start entry bootstrap: set $gp=0x801F65D0, jump to main
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

M2C_UNK func_801D91CC();                            /* static */
extern M2C_UNK _gp;

void func_801DA084(void) {
    func_801D91CC();
}
