/*
 * MOVOVL.EXE game code — func_801D98DC @ 0x801D98DC (136B)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   debug printf dump (2 printf calls)
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

M2C_UNK printf(M2C_UNK *, M2C_UNK);                 /* extern */
extern M2C_UNK D_801D8800;
extern M2C_UNK D_801D881C;
extern s32 D_801F6618;
extern s32 D_801F661C;
extern s32 D_801F6624;
extern s32 D_801F6628;
extern M2C_UNK D_801F6828;

void func_801D98DC(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    M2C_FIELD(&D_801F6828, s32 *, 0) = (s32) M2C_FIELD(arg0, s32 *, 0);
    M2C_FIELD(&D_801F6828, s32 *, 4) = (s32) M2C_FIELD(arg0, s32 *, 4);
    M2C_FIELD(&D_801F6828, s32 *, 8) = (s32) M2C_FIELD(arg0, s32 *, 8);
    M2C_FIELD(&D_801F6828, s32 *, 0xC) = (s32) M2C_FIELD(arg0, s32 *, 0xC);
    M2C_FIELD(&D_801F6828, s32 *, 0x10) = (s32) M2C_FIELD(arg0, s32 *, 0x10);
    M2C_FIELD(&D_801F6828, s32 *, 0x14) = (s32) M2C_FIELD(arg0, s32 *, 0x14);
    D_801F6624 = arg1;
    D_801F6618 = arg2;
    D_801F6628 = arg3;
    D_801F661C = arg4;
    printf(&D_801D8800, 0x10000);
    printf(&D_801D881C, 0xC0800);
}
