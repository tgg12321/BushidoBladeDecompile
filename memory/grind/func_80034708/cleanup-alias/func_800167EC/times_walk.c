void func_800167EC(void) {
    s32 i;
    FileTimeRec *t;

    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    t = D_80106A50.times;
    for (i = 0; i < 3; i++, t++) {
        t->unk_0 = 0;
        t->unk_1 = 0;
        t->unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
}
