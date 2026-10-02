void func_80020D38(void) {
    s32 i;

    for (i = 1; i < 2; i++) {
        if ((&D_800A38C4)[i] == 0xFFFF) {
            seq_Reset();
        }
        (&D_800A38C4)[i] = 0;
    }
}
