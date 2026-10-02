void func_80020D38(void) {
    switch ((&D_800A38C4)[1]) {
    case 0xFFFF:
        seq_Reset();
        break;
    }
    (&D_800A38C4)[1] = 0;
}
