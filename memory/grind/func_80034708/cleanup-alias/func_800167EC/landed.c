void func_800167EC(void) {
    s32 i = 0;
    u32 c = 0x1A5E0;
    FileRecord *rec;

    rec = &D_80106A50;
    D_800A3710 = 0;
    D_80106A50.flags = 0;
    rec->unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        rec->times[i].unk_0 = 0;
        rec->times[i].unk_1 = 0;
        rec->times[i].unk_4 = c;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
}
