void func_80020D38(void) {
    if ((&D_800A38C4)[1] == 0xFFFF) {
        seq_Reset();
        goto end;
    }
end:
    (&D_800A38C4)[1] = 0;
}
