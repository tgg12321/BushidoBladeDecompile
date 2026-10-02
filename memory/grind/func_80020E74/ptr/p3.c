void func_80020CDC(void) {
    s32 i = 1;
    if (D_800A38C4[i] == 0xFFFF) {
        seq_Reset();
    }
    D_800A3880 = 0;
    D_800A38C4[1] = 0;
    D_800A38C4[0] = 0;
    D_800A38C0[1] = 0xFF;
    D_800A38C0[0] = 0xFF;
}
void func_80020D38(void) {
    s32 i = 1;
    if (D_800A38C4[i] == 0xFFFF) {
        seq_Reset();
    }
    D_800A38C4[1] = 0;
}
