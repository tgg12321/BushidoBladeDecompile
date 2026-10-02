void func_80020D38(void) {
    s32 v;

    do {
        v = (&D_800A38C4)[1];
    } while (0);
    if (v == 0xFFFF) {
        seq_Reset();
    }
    (&D_800A38C4)[1] = 0;
}
