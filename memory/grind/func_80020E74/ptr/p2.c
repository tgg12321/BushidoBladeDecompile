void func_80020CDC(void) {
    u16 *p = D_800A38C4;
    if (p[1] == 0xFFFF) {
        seq_Reset();
    }
    D_800A3880 = 0;
    p[1] = 0;
    p[0] = 0;
    D_800A38C0[1] = 0xFF;
    D_800A38C0[0] = 0xFF;
}
void func_80020D38(void) {
    u16 *p = D_800A38C4;
    if (p[1] == 0xFFFF) {
        seq_Reset();
    }
    p[1] = 0;
}
