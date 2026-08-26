void func_800871D4(s32 mode)
{
    s32 bitsUpper;
    s32 bitsLower;
    u16 voice;
    s32 idx;
    s32 okof1;
    s32 okof2;
    u16 old1;
    u16 old2;
    u16 on1;

    voice = D_8010280A;
    if (voice < 16) {
        bitsLower = 1 << voice;
        bitsUpper = 0;
    } else {
        bitsLower = 0;
        bitsUpper = 1 << (voice - 16);
    }
    idx = voice * 54;
    *(s8 *)((u8 *)&D_800F4E35 + idx) = 0;
    old1 = D_801078D8;
    old2 = D_801078DA;
    *(s16 *)((u8 *)&D_800F4E1C + idx) = 0;
    *(s16 *)((u8 *)&D_800F4E18 + idx) = 0;
    on1 = D_800F1B10;
    okof2 = old2 | bitsUpper;
    okof1 = old1 | bitsLower;
    D_801078D8 = okof1;
    D_800F1B10 = on1 & ~okof1;
    D_801078DA = okof2;
    D_800F1B12 &= ~okof2;
}
