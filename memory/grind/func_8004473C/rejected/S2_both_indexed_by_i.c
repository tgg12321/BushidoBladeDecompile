void func_8004473C(void)
{
    Rec4473C *src;
    Rec4473C *dst;
    s32 i;

    D_800A9CF8.unk10 = (s32)game_GetCharData();
    src = (Rec4473C *)D_800A9CF8.unk10;
    dst = (Rec4473C *)D_800A9CF8.unkC;
    for (i = 0; i < D_800A9CF8.unk6; i++) {
        dst[i].unk0 = 0;
        dst[i].unk1 = 0;
        dst[i].unk2 = 0;
        dst[i].unk4 = D_800A9CF8.unk0;
        dst[i].unk6 = 0;
        dst[i].unk8 = 0;
        dst[i].unkA = 4;
        dst[i].unkC = 0;
        dst[i].unk10 = 0;
        dst[i].unk12 = 0;
        dst[i].unk14 = 0;
        dst[i].unk4C = src[i].unk2C;
        dst[i].unk50 = src[i].unk30;
        dst[i].unk54 = src[i].unk34;
        dst[i].unk58 = -1;
    }
}
