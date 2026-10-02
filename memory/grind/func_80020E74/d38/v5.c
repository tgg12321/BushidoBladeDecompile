void func_80020D38(void) {
    u16 v = (&D_800A38C4)[1];

    if (v == 0xFFFF) {
        seq_Reset();
    }
    (&D_800A38C4)[1] = 0;
}
