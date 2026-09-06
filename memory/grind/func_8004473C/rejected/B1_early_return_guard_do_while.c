void func_8004473C(void)
{
    Rec4473C *src;
    Rec4473C *dst;
    s32 i;

    src = (Rec4473C *)game_GetCharData();
    D_800A9CF8.unk10 = (s32)src;
    dst = (Rec4473C *)D_800A9CF8.unkC;
    if (D_800A9CF8.unk6 <= 0) {
        return;
    }
    i = 0;
    do {
        dst->unk0 = 0;
        dst->unk1 = 0;
        dst->unk2 = 0;
        dst->unk4 = D_800A9CF8.unk0;
        dst->unk6 = 0;
        dst->unk8 = 0;
        dst->unkA = 4;
        dst->unkC = 0;
        dst->unk10 = 0;
        dst->unk12 = 0;
        dst->unk14 = 0;
        dst->unk4C = src->unk2C;
        dst->unk50 = src->unk30;
        dst->unk54 = src->unk34;
        dst->unk58 = -1;
        i++;
        dst++;
        src++;
    } while (i < D_800A9CF8.unk6);
}
