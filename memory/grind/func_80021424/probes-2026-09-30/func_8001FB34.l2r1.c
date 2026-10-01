s32 func_8001FB34(PracticeMenuRec *arg0, s32 arg1) {
    s16 v1;
    s32 v0;
    v1 = D_800A38DC;
    if (v1 == 2) return 0;
    if (v1 == 5) return 0;
    if (v1 == 3) return 0;
    if (v1 != 0) goto check2;
    if (D_800A385C != 0) return 0;
check2:
    v1 = arg0->unk_00->unk_0C;
    if (v1 == 0xD) return 0;
    if (v1 == 0x1C) return 0;
    v1 = arg0->unk_0A;
    if (v1 != 0xE) goto check3;
    if (arg0->unk_330 == 0) return 0;
    v1 = arg0->unk_332;
    if (v1 == 0xA) goto check3;
    return 0;
check3:
    v0 = 1;
    if (arg1 != 0) {
        v0 = arg0->unk_26C;
        v0 = (v0 != 0);
    }
    return v0;
}
