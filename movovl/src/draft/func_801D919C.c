/*
 * MOVOVL.EXE game code — func_801D919C @ 0x801D919C (48B, leaf)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   wraps CdMix — CD-audio volume/mix setter
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

M2C_UNK CdMix(s8 *);                                /* extern */

void func_801D919C(s8 arg0, s8 arg1, s8 arg2, s8 arg3) {
    s8 sp13;
    s8 sp12;
    s8 sp11;
    s8 sp10;

    sp10 = arg0;
    sp11 = arg1;
    sp12 = arg2;
    sp13 = arg3;
    CdMix(&sp10);
}
