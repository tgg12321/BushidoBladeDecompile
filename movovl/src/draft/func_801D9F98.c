/*
 * MOVOVL.EXE game code — func_801D9F98 @ 0x801D9F98 (216B)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   play-one-movie top level: decode pump loop, stream teardown, CdControlF, CD-audio helper — MovPlay-shaped
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

M2C_UNK CdControlF(M2C_UNK, M2C_UNK);               /* extern */
s32 CdReady(M2C_UNK, M2C_UNK *);                    /* extern */
M2C_UNK DecDCToutCallback(M2C_UNK);                 /* extern */
M2C_UNK StClearRing();                              /* extern */
M2C_UNK StUnSetRing();                              /* extern */
M2C_UNK VSync(M2C_UNK);                             /* extern */
M2C_UNK func_801D919C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK); /* static */
s32 func_801D9E70();                                /* static */
extern void *D_801F65E4;
extern s32 D_801F6620;

s32 func_801D9F98(void) {
    M2C_UNK sp10;
    s32 var_v0;

    if ((func_801D9E70() == 0) || (var_v0 = 1, (M2C_FIELD(D_801F65E4, u8 *, 6) != 1))) {
        DecDCToutCallback(0);
        StUnSetRing();
        StClearRing();
        CdControlF(9, 0);
        if (M2C_FIELD(D_801F65E4, u8 *, 6) == 2) {
            func_801D919C(0, 0, 0, 0);
loop_4:
            if (CdReady(1, &sp10) == 5) {
                CdControlF(7, 0);
                VSync(4);
                goto loop_4;
            }
            D_801F6620 = 0;
            return 0;
        }
        D_801F6620 = 1;
        var_v0 = 0;
        /* Duplicate return node #8. Try simplifying control flow for better match */
        return var_v0;
    }
    return var_v0;
}
