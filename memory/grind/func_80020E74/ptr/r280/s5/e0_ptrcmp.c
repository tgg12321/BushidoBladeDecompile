s32 func_800224E0(PracticeMenuRec *arg0) {
    u8 *p;
    u8 *end;
    s32 val;
    s32 i;

    p = D_8008EB1C[D_800A384C];
    end = p + 2;
    val = D_8008DB1C[arg0->unk_00->unk_0A][arg0->unk_00->unk_0E];
    do {
        for (i = 0; i < 3; i++) {
            if (*p == ((val >> (i * 4)) & 0xF)) {
                return i;
            }
        }
        p++;
    } while (p < end);
    return 0;
}
