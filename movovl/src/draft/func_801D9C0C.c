/*
 * MOVOVL.EXE game code — func_801D9C0C @ 0x801D9C0C (288B)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   movie open/close driver: calls stream-start + vlc-frame helpers, StUnSetRing/StClearRing teardown
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

M2C_UNK DecDCToutCallback(M2C_UNK);                 /* extern */
M2C_UNK StClearRing();                              /* extern */
M2C_UNK StUnSetRing();                              /* extern */
M2C_UNK func_801D9714();                            /* static */
M2C_UNK func_801D9964(s32, M2C_UNK, s32 *, s32 *);  /* static */
s32 func_801D99E4(M2C_UNK *, M2C_UNK (*)());        /* static */
M2C_UNK func_801D9B74();                            /* static */
extern void *D_801F65E4;
extern s32 D_801F6618;
extern s32 *D_801F6628;
extern M2C_UNK D_801F6828;

s32 func_801D9C0C(void) {
    s32 *var_a2;
    s32 *var_a3;
    s32 *var_v1;
    s32 *var_v1_2;
    s32 *var_v1_3;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_t0;

    var_a0 = 0x3FFF;
    var_v1 = D_801F6618 + 0xFFFC;
    do {
        *var_v1 = 0;
        var_a0 -= 1;
        var_v1 -= 4;
    } while (var_a0 >= 0);
    var_t0 = 0;
    var_a2 = D_801F6628;
    var_a3 = var_a2;
    var_a0_2 = 0;
    do {
        var_v1_2 = var_a3;
loop_4:
        *var_v1_2 = 0;
        var_a0_2 += 1;
        var_v1_2 += 4;
        if (var_a0_2 <= 0x9FFF) {
            goto loop_4;
        }
        var_a0_3 = 0;
        var_v1_3 = var_a2 + 0x50000;
loop_6:
        *var_v1_3 = 0;
        var_a0_3 += 1;
        if (var_a0_3 <= 0x1C1FF) {
            goto loop_6;
        }
        var_a2 += 0x38400;
        var_a3 += 0x28000;
        var_t0 += 1;
        var_a0_2 = 0;
    } while (var_t0 < 2);
    M2C_FIELD(D_801F65E4, s8 *, 5) = 1;
    func_801D9964(0, 0x1C1FF, var_a2, var_a3);
    if (func_801D99E4(&D_801F6828, func_801D9714) != 0) {
        func_801D9B74();
        M2C_FIELD(D_801F65E4, s8 *, 6) = 1;
        return 1;
    }
    DecDCToutCallback(0);
    StUnSetRing();
    StClearRing();
    return 0;
}
