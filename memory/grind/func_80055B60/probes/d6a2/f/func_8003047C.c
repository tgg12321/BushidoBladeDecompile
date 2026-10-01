void func_8003047C(PracticeMenuRec *a0) {
    s32 i;
    s16 val;

    a0->unk_330 = 0;
    i = 0;
    do {
        val = D_8008E338[a0->unk_0A][i];
        a0->unk_332[i] = val;
        if (val != -1) {
            a0->unk_330++;
        }
        i++;
    } while (i < 5);
    D_800A36F2[a0->unk_04] = D_8008E338[a0->unk_0A][0];
}
