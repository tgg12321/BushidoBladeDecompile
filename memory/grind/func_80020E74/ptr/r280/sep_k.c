void func_80021280(s32 a0) {
    s32 a1 = 0;
    PracticeMenuRec *a2 = &g_practice_menu_table[a0];
    s32 a3 = a2->unk_48;
    u16 *v1 = D_800A38C4;

loop1_21280:
    if (a3 == *v1) goto done1_21280;
    a1++;
    v1++;
    if (a1 < 2) goto loop1_21280;
done1_21280:

    {
        u16 val = a2->unk_48;
        a2->unk_4A = a1;
        a2->unk_4C = 0;

        if ((u32)(val >> 12) < 2) {
            u16 t1;
            s32 t4;
            s32 t3;
            s32 t2;
            u8 t0;
            s32 mode;
            s32 k;

            k = 0;
            t1 = val;
            t4 = 4;
            t3 = 3;
            t2 = 1;
            mode = D_800A38DC;
            t0 = D_800A384C;
        loop2_21280:
            {
                u16 nibble = (t1 >> (k << 2)) & 0xF;
                if (nibble != t4) goto not4_21280;
                a2->unk_88 = k;
                if (mode != t3) goto store4_21280;
                if (a0 != t2) goto store4_21280;
                if (t0 != nibble) goto next_21280;
            store4_21280:
                a2->unk_8A = a2->unk_26C;
                goto next_21280;
            not4_21280:
                if (nibble != 5) goto next_21280;
                a2->unk_8E = k;
                if (mode != 0) goto store5_21280;
                if (D_800A385C == 0) goto store5_21280;
                if (a0 == 0) {
                    /* FAKE: loop tail duplicated into this arm (cross-jump
                       re-merges to identical bytes; lifts the counter's
                       reg_n_refs so it beats the pointer for $a1) */
                    k++;
                    if (k < 3) goto loop2_21280;
                    return;
                }
            store5_21280:
                a2->unk_90 = a2->unk_26C;
            }
        next_21280:
            k++;
            if (k < 3) goto loop2_21280;
        }
    }
}
