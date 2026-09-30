void func_800167EC(void) {
    s32 i;
    FileRecord *rec = &D_80106A50;

    D_800A3710 = 0;
    rec->flags = 0;
    rec->unk_00 = 0x7007;
    rec->unk_04 = 0;
    for (i = 0; i < 3; i++) {
        rec->times[i].unk_0 = 0;
        rec->times[i].unk_1 = 0;
        rec->times[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
}
