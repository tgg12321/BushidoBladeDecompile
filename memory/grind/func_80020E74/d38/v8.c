void func_80020D38(void) {
    u16 *p;

    if ((&D_800A38C4)[1] == 0xFFFF) {
        seq_Reset();
    }
    p = &D_800A38C4;
    p[1] = 0;
}
