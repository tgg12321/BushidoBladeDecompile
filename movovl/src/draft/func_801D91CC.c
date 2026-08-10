/*
 * MOVOVL.EXE game code — func_801D91CC @ 0x801D91CC (1360B)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   main(): env init (ResetCallback/CdInit/PadInit/ResetGraph), loads TITLE.TIM via CdRead+LoadImage, pad-driven title loop, movie playback via func_801D9F98, shutdown (PadStop/ResetGraph/StopCallback)
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

M2C_UNK CdControl(M2C_UNK, s32, M2C_UNK);           /* extern */
M2C_UNK CdInit();                                   /* extern */
M2C_UNK CdRead(M2C_UNK, s32, M2C_UNK);              /* extern */
s32 CdReadSync(M2C_UNK, M2C_UNK);                   /* extern */
M2C_UNK ClearImage(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK DrawPrim(M2C_UNK *, s8, s8);                /* extern */
M2C_UNK DrawSync(M2C_UNK);                          /* extern */
M2C_UNK LoadImage(M2C_UNK *, s32);                  /* extern */
M2C_UNK MoveImage(M2C_UNK *, M2C_UNK, s32);         /* extern */
M2C_UNK PadInit(M2C_UNK);                           /* extern */
s32 PadRead(M2C_UNK);                               /* extern */
M2C_UNK PadStop();                                  /* extern */
M2C_UNK PutDispEnv(M2C_UNK *);                      /* extern */
M2C_UNK PutDrawEnv(void *);                         /* extern */
M2C_UNK ResetCallback();                            /* extern */
M2C_UNK ResetGraph(M2C_UNK);                        /* extern */
M2C_UNK SetDefDispEnv(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK SetDefDrawEnv(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK SetDispMask(M2C_UNK);                       /* extern */
M2C_UNK SetDrawTPage(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK SetGraphDebug(M2C_UNK);                     /* extern */
M2C_UNK SetPolyG4(M2C_UNK *);                       /* extern */
M2C_UNK SetSemiTrans(M2C_UNK *, M2C_UNK);           /* extern */
M2C_UNK StopCallback();                             /* extern */
M2C_UNK VSync(M2C_UNK);                             /* extern */
M2C_UNK func_801D919C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK); /* static */
M2C_UNK func_801D98DC(s32, s32, s32, s32, s32);     /* static */
M2C_UNK func_801D9C0C();                            /* static */
s32 func_801D9F98(s32);                             /* static */
s32 func_801DA070();                                /* static */
M2C_UNK func_801DA07C();                            /* static */
extern s32 D_801F65D0;
extern M2C_UNK D_801F65D4;
extern M2C_UNK D_801F65DC;

void func_801D91CC(void *arg1) {
    M2C_UNK sp120;
    M2C_UNK spF8;
    s8 spB4;
    M2C_UNK sp9C;
    s8 sp58;
    M2C_UNK sp40;
    s8 sp3D;
    M2C_UNK sp2C;
    s8 sp29;
    M2C_UNK sp18;
    s32 temp_s0;
    s32 temp_v0;
    s32 var_a1;
    s32 var_a2;
    s32 var_s0;
    s32 var_s0_2;
    s32 var_s1;
    s32 var_s3;
    s32 var_s3_2;
    s32 var_s3_3;
    s32 var_s4;
    s32 var_s5;
    s32 var_s6;
    s32 var_v1;

    func_801DA07C();
    var_s6 = 0;
    ResetCallback();
    CdInit();
    PadInit(0);
    ResetGraph(0);
    SetGraphDebug(0);
    VSync(0);
    SetDispMask(0);
    func_801D919C(0xFF, 0, 0xFF, 0);
    CdControl(2, M2C_FIELD(arg1, s32 *, 0x10), 0);
    CdRead(0x4C, M2C_FIELD(arg1, s32 *, 0xC), 0x80);
loop_1:
    var_s1 = 0;
    if (CdReadSync(1, 0) > 0) {
        VSync(0);
        goto loop_1;
    }
    var_s4 = 0;
    LoadImage(&D_801F65DC, M2C_FIELD(arg1, s32 *, 0xC) + 0x14);
    DrawSync(0);
    SetDefDispEnv(&sp18, 0, 0, 0x140, 0xF0);
    SetDefDispEnv(&sp2C, 0, 0xF0, 0x140, 0xF0);
    sp3D = 1;
    sp29 = 1;
    PutDispEnv(&sp18);
    ClearImage(&D_801F65D4, 0, 0, 0);
    DrawSync(0);
    SetDispMask(1);
    VSync(4);
    var_s5 = 0;
    var_s3 = 0;
    func_801D98DC(M2C_FIELD(arg1, s32 *, 0), M2C_FIELD(arg1, s32 *, 4), M2C_FIELD(arg1, s32 *, 8), M2C_FIELD(arg1, s32 *, 0xC), M2C_FIELD(arg1, s32 *, 0x14));
    func_801D9C0C();
loop_4:
    var_s1 = var_s1 == 0;
    if (var_s5 != 0) {
        var_s1 = 0;
    }
    temp_s0 = var_s4;
    var_s4 = PadRead(0);
    if (var_s4 & ~temp_s0 & 0x40) {
        goto block_7;
    }
    if ((var_s5 == 0) && (D_801F65D0 != 0)) {
        var_s5 = 1;
        SetDefDispEnv(&sp18, 0x1E0, 0, 0x140, 0xF0);
        sp29 = 0;
        PutDispEnv(&sp18);
        var_s1 = 0;
    }
    if (func_801D9F98(var_s1) == 0) {
        if (func_801DA070() != 0) {
            func_801D919C(0, 0, 0, 0);
            var_s3 = 0x258;
        } else {
block_7:
            var_s6 = 1;
        }
    } else {
        DrawSync(0);
        VSync(0);
        PutDispEnv(&sp18 + (var_s1 * 0x14));
        goto loop_4;
    }
    if (var_s6 == 0) {
        var_s3_2 = var_s3 - 1;
        var_s0 = var_s4;
        if (var_s3_2 != 0) {
loop_17:
            temp_v0 = PadRead(0);
            if (temp_v0 & ~var_s0 & 0x40) {
                var_s6 = 1;
            } else {
                VSync(0);
                var_s3_2 -= 1;
                var_s0 = temp_v0;
                if (var_s3_2 != 0) {
                    goto loop_17;
                }
            }
        }
        if (var_s6 == 0) {
            SetDefDispEnv(&sp18, 0, 0, 0x140, 0xF0);
            SetDefDispEnv(&sp2C, 0, 0xF0, 0x140, 0xF0);
            sp3D = 0;
            sp29 = 0;
            SetDefDrawEnv(&sp40, 0, 0, 0x140, 0xF0);
            SetDefDrawEnv(&sp9C, 0, 0xF0, 0x140, 0xF0);
            var_s3_3 = 0;
            spB4 = 0;
            sp58 = 0;
            var_s0_2 = 0 & 1;
            do {
                PutDrawEnv(&sp40 + (var_s0_2 * 0x5C));
                MoveImage(&D_801F65DC, 0, var_s0_2 * 0xF0);
                SetDrawTPage(&sp120, 0, 1, 0x40);
                DrawPrim(&sp120);
                SetPolyG4(&spF8);
                SetSemiTrans(&spF8, 1);
                var_a1 = (s32) (var_s3_3 << 8) / 64;
                M2C_FIELD(&spF8, s16 *, 8) = 0;
                M2C_FIELD(&spF8, s16 *, 0xA) = 0;
                M2C_FIELD(&spF8, s16 *, 0x10) = 0x140;
                M2C_FIELD(&spF8, s16 *, 0x12) = 0;
                M2C_FIELD(&spF8, s16 *, 0x18) = 0;
                M2C_FIELD(&spF8, s16 *, 0x1A) = 0xF0;
                M2C_FIELD(&spF8, s16 *, 0x20) = 0x140;
                M2C_FIELD(&spF8, s16 *, 0x22) = 0xF0;
                var_v1 = var_a1;
                var_a2 = var_a1;
                if (var_a1 < 0) {
                    var_a2 = 0;
                } else if (var_a1 >= 0x100) {
                    var_a2 = 0xFF;
                }
                if (var_v1 < 0) {
                    var_v1 = 0;
                } else if (var_v1 >= 0x100) {
                    var_v1 = 0xFF;
                }
                if (var_a1 < 0) {
                    var_a1 = 0;
                } else if (var_a1 >= 0x100) {
                    var_a1 = 0xFF;
                }
                M2C_FIELD(&spF8, s8 *, 4) = (s8) var_a2;
                M2C_FIELD(&spF8, s8 *, 5) = (s8) var_a2;
                M2C_FIELD(&spF8, s8 *, 6) = (s8) var_a2;
                M2C_FIELD(&spF8, s8 *, 0xC) = (s8) var_v1;
                M2C_FIELD(&spF8, s8 *, 0xD) = (s8) var_v1;
                M2C_FIELD(&spF8, s8 *, 0xE) = (s8) var_v1;
                M2C_FIELD(&spF8, s8 *, 0x14) = (s8) var_v1;
                M2C_FIELD(&spF8, s8 *, 0x15) = (s8) var_v1;
                M2C_FIELD(&spF8, s8 *, 0x16) = (s8) var_v1;
                M2C_FIELD(&spF8, s8 *, 0x1C) = (s8) var_a1;
                M2C_FIELD(&spF8, s8 *, 0x1D) = (s8) var_a1;
                M2C_FIELD(&spF8, s8 *, 0x1E) = (s8) var_a1;
                DrawPrim(&spF8, (s8) var_a1, (s8) var_a2);
                DrawSync(0);
                VSync(0);
                PutDispEnv(&sp18 + ((var_s3_3 & 1) * 0x14));
                var_s3_3 += 1;
                var_s0_2 = var_s3_3 & 1;
            } while (var_s3_3 < 0x40);
        }
    }
    VSync(0);
    SetDispMask(0);
    ClearImage(&D_801F65D4, 0, 0, 0);
    DrawSync(0);
    PadStop();
    ResetGraph(3);
    StopCallback();
}
