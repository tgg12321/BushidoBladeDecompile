void func_800167EC(void) {
    s32 i = 0;

    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    do {
        D_80106A50.times[i].unk_0 = 0;
        D_80106A50.times[i].unk_1 = 0;
        D_80106A50.times[i].unk_4 = 0x1A5E0;
        i++;
    } while (i < 3);
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
}
