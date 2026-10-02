void func_80020D38(void) {
    if ((&D_800A38C4)[1] != 0xFFFF) {
    } else {
        seq_Reset();
    }
    (&D_800A38C4)[1] = 0;
}
