void func_800167EC(void) {
    s32 i;

    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_04 = 0;
    D_80106A50.unk_00 = 0x7007;
    for (i = 0; i < 3; i++) {
        D_80106A50.times[i].unk_0 = 0;
        D_80106A50.times[i].unk_1 = 0;
        D_80106A50.times[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
}
