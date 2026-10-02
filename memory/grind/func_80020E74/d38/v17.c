static void clear_slot1(void) {
    (&D_800A38C4)[1] = 0;
}
void func_80020D38(void) {
    if ((&D_800A38C4)[1] == 0xFFFF) {
        seq_Reset();
    }
    clear_slot1();
}
