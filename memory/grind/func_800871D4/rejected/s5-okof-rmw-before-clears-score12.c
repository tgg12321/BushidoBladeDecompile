void func_800871D4(s32 mode)
{
    s32 bitsUpper;
    s32 bitsLower;
    u16 voice;
    s32 idx;

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
    D_801078D8 |= bitsLower;
    D_801078DA |= bitsUpper;
    *(s16 *)((u8 *)&D_800F4E1C + idx) = 0;
    *(s16 *)((u8 *)&D_800F4E18 + idx) = 0;
    D_800F1B10 &= ~D_801078D8;
    D_800F1B12 &= ~D_801078DA;
}
