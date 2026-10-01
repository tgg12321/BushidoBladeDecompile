/* Per-frame update of the two g_practice_menu_table records.  The three
 * scratchpad points per character at SPAD->unk00 and the two at SPAD->unk48
 * are copied into each record's unk_210 / unk_234 (func_8002C22C reads them
 * back); unk_18C is the centroid of SPAD->unkA8[k][1..3] and unk_174 the
 * midpoint of SPAD->unkA8[k][4..5]. */
void func_8002C61C(void) {
    /* FAKE: pointer aliases to g_practice_menu_table[0] / [1]
     * (pointer-alias-fake-exception).  The target keeps both record bases in
     * $s1 / $s0 from the prologue (lui/addiu s1, addiu s0,s1,0x44C) and reads
     * unk_3C / unk_286 / unk_0C / unk_F4 / unk_28C at displacements off them.
     * Without the pointers every access is a symbol+offset constant address,
     * which GO_IF_LEGITIMATE_ADDRESS (tools/gcc-2.7.2/config/mips/mips.h:2286)
     * accepts as is, so no base register exists (all-direct form 38).  Which
     * accesses go through them is measured per site: unk_AD through the
     * global 2 (the forced address pseudo of the record-0 load is shared by
     * the record-0 store along cse's skip-blocks path, cse.c:8150, so the load
     * keeps its s1-relative form, final .s banked), unk_6A through the
     * pointers 4, the unk_210 / unk_234 copy loops through the pointers 30.
     * Ledger: memory/grind/func_8002C22C/manual-2026-10-01/scores.txt */
    PracticeMenuRec *s1 = &g_practice_menu_table[0];
    PracticeMenuRec *s0 = &g_practice_menu_table[1];
    s32 i;
    u16 mode;

    mode = g_practice_menu_table[0].unk_6A;

    if (mode == 0xF || mode == 0x1C || mode == 0x1D || mode == 0x1E ||
        mode == 0x1F || mode == 0x20 || mode == 0x21) {
        func_80026DA4();
    } else if (mode == 0x11) {
        func_8002C0DC();
    } else {
        func_8002872C();
        func_800288C8();
        D_800A3824 = func_80029454();
        if (D_800A3824 < 0) goto do_calc;
        func_8002C22C();
        if (D_800A3824 < 0) goto do_calc;
        if (s1->unk_AD != 0 || s0->unk_AD != 0) {
            func_800283D0((u8 *)s1, (u8 *)0x1F8003F4);
            func_800283D0((u8 *)s0, (u8 *)0x1F8003F4);
            s0->unk_AD = 0;
            s1->unk_AD = 0;
            goto after_calc;
        }
    do_calc:
        func_8002AB08(0);
    after_calc:

        if (s1->unk_3C >= 3 && s0->unk_3C >= 3 &&
            D_800A38A8 != 0 && s1->unk_286 == -1 &&
            s0->unk_286 == -1 && s1->unk_0C != 0x1F &&
            s0->unk_0C != 0x1F) {
            s32 diff = s1->unk_F4.y - s0->unk_F4.y;
            if (diff < 0) diff = -diff;
            if (diff < 0x3E8) {
                s1->unk_286 = 0xA;
                s0->unk_286 = 0xA;
                s0->unk_28C = 0;
                s1->unk_28C = 0;
                D_800A3910 = 0;
                D_800A389C = 0;
            }
        }

    }

    if ((u16)g_practice_menu_table[0].unk_6A == 5) {
        D_800A3748 = 1;
        D_800A3834 = 0x1C;
    } else if ((u16)g_practice_menu_table[1].unk_6A == 5) {
        D_800A3748 = 0;
        D_800A3834 = 0x1C;
    }

    for (i = 0; i < 3; i++) {
        g_practice_menu_table[0].unk_210[i] = SPAD->unk00[0][i];
        g_practice_menu_table[1].unk_210[i] = SPAD->unk00[1][i];
    }

    for (i = 0; i < 2; i++) {
        g_practice_menu_table[0].unk_234[i] = SPAD->unk48[0][i];
        g_practice_menu_table[1].unk_234[i] = SPAD->unk48[1][i];
    }

    for (i = 0; i < 2; i++) {
        g_practice_menu_table[i].unk_18C.x = (SPAD->unkA8[i][1].x + SPAD->unkA8[i][2].x + SPAD->unkA8[i][3].x) / 3;
        g_practice_menu_table[i].unk_18C.y = (SPAD->unkA8[i][1].y + SPAD->unkA8[i][2].y + SPAD->unkA8[i][3].y) / 3;
        g_practice_menu_table[i].unk_18C.z = (SPAD->unkA8[i][1].z + SPAD->unkA8[i][2].z + SPAD->unkA8[i][3].z) / 3;
        g_practice_menu_table[i].unk_174.x = (SPAD->unkA8[i][4].x + SPAD->unkA8[i][5].x) / 2;
        g_practice_menu_table[i].unk_174.y = (SPAD->unkA8[i][4].y + SPAD->unkA8[i][5].y) / 2;
        g_practice_menu_table[i].unk_174.z = (SPAD->unkA8[i][4].z + SPAD->unkA8[i][5].z) / 2;
    }

    {
        s16 saved = s1->unk_286;
        if (saved == -1) {
            func_80031B24();
            if (s1->unk_286 == saved) {
                func_80032314();
            }
        }
    }
}
