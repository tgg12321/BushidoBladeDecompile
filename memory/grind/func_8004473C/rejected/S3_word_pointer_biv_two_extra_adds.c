void func_8004473C(void)
{
    s32 *sp;
    Rec4473C *dst;
    s32 i;

    D_800A9CF8.unk10 = (s32)game_GetCharData();
    sp = &((Rec4473C *)D_800A9CF8.unk10)->unk34;
    dst = (Rec4473C *)D_800A9CF8.unkC;
    for (i = 0; i < D_800A9CF8.unk6; i++, dst++, sp += 0x1A) {
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
        dst->unk4C = sp[-2];
        dst->unk50 = sp[-1];
        dst->unk54 = sp[0];
        dst->unk58 = -1;
    }
}
