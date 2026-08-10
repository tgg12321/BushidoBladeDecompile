/*
 * MOVOVL.EXE game code — func_801D9DC4 @ 0x801D9DC4 (172B)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   frame image upload: LoadImage of decoded slice
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

M2C_UNK LoadImage(s16 *, s32);                      /* extern */
extern void *D_801F65E4;
extern s32 D_801F6628;

void func_801D9DC4(s32 arg0) {
    s16 sp16;
    s16 sp14;
    s16 sp12;
    s16 sp10;
    s16 var_s2;
    s32 var_a1;
    s32 var_s0;
    s32 var_s1;

    var_a1 = D_801F6628 + 0x50000;
    if (M2C_FIELD(D_801F65E4, u8 *, 3) == 0) {
        var_a1 += 0x38400;
    }
    sp14 = 0x18;
    sp16 = 0xF0;
    if (arg0 != 0) {
        sp12 = 0xF0;
    } else {
        sp12 = 0;
    }
    var_s2 = 0;
    var_s1 = 0;
    var_s0 = var_a1;
    do {
        sp10 = var_s2;
        LoadImage(&sp10, var_s0);
        var_s1 += 1;
        var_s0 += 0x2D00;
        var_s2 += 0x18;
    } while (var_s1 < 0x14);
}
