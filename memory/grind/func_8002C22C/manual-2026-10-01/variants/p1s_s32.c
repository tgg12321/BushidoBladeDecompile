void func_8002C22C(void) {
    s32 *scr = (s32 *)0x1F8002B8;
    PracticeMenuRec *rec1 = &g_practice_menu_table[1];

    scr[0xA8/4] = 0;
    scr[0xAC/4] = 0;
    scr[0xB0/4] = 0;
    scr[0xB8/4] = 0;
    scr[0xBC/4] = 0;
    scr[0xC0/4] = 0;

    if (D_800A3824 & 1) {
        scr[0xA8/4] = SPAD->unk48[0][0].x;
        scr[0xAC/4] = SPAD->unk48[0][0].y;
        scr[0xB0/4] = SPAD->unk48[0][0].z;
        scr[0xA8/4] += SPAD->unk48[0][1].x;
        scr[0xAC/4] += SPAD->unk48[0][1].y;
        scr[0xB0/4] += SPAD->unk48[0][1].z;
        scr[0xB8/4] = g_practice_menu_table[0].unk_234[0].x;
        scr[0xBC/4] = g_practice_menu_table[0].unk_234[0].y;
        scr[0xC0/4] = g_practice_menu_table[0].unk_234[0].z;
        scr[0xB8/4] += g_practice_menu_table[0].unk_234[1].x;
        scr[0xBC/4] += g_practice_menu_table[0].unk_234[1].y;
        scr[0xC0/4] += g_practice_menu_table[0].unk_234[1].z;
    } else {
        scr[0xA8/4] = SPAD->unk00[0][0].x;
        scr[0xAC/4] = SPAD->unk00[0][0].y;
        scr[0xB0/4] = SPAD->unk00[0][0].z;
        scr[0xA8/4] += SPAD->unk00[0][1].x;
        scr[0xAC/4] += SPAD->unk00[0][1].y;
        scr[0xB0/4] += SPAD->unk00[0][1].z;
        scr[0xB8/4] = g_practice_menu_table[0].unk_210[0].x;
        scr[0xBC/4] = g_practice_menu_table[0].unk_210[0].y;
        scr[0xC0/4] = g_practice_menu_table[0].unk_210[0].z;
        scr[0xB8/4] += g_practice_menu_table[0].unk_210[1].x;
        scr[0xBC/4] += g_practice_menu_table[0].unk_210[1].y;
        scr[0xC0/4] += g_practice_menu_table[0].unk_210[1].z;
    }
    if (D_800A3824 & 2) {
        scr[0xA8/4] += SPAD->unk48[1][0].x;
        scr[0xAC/4] += SPAD->unk48[1][0].y;
        scr[0xB0/4] += SPAD->unk48[1][0].z;
        scr[0xB8/4] += rec1->unk_234[0].x;
        scr[0xBC/4] += rec1->unk_234[0].y;
        scr[0xC0/4] += rec1->unk_234[0].z;
        scr[0xA8/4] += SPAD->unk48[1][1].x;
        scr[0xAC/4] += SPAD->unk48[1][1].y;
        scr[0xB0/4] += SPAD->unk48[1][1].z;
        scr[0xB8/4] += rec1->unk_234[1].x;
        scr[0xBC/4] += rec1->unk_234[1].y;
        scr[0xC0/4] += rec1->unk_234[1].z;
    } else {
        scr[0xA8/4] += SPAD->unk00[1][0].x;
        scr[0xAC/4] += SPAD->unk00[1][0].y;
        scr[0xB0/4] += SPAD->unk00[1][0].z;
        scr[0xB8/4] += rec1->unk_210[0].x;
        scr[0xBC/4] += rec1->unk_210[0].y;
        scr[0xC0/4] += rec1->unk_210[0].z;
        scr[0xA8/4] += SPAD->unk00[1][1].x;
        scr[0xAC/4] += SPAD->unk00[1][1].y;
        scr[0xB0/4] += SPAD->unk00[1][1].z;
        scr[0xB8/4] += rec1->unk_210[1].x;
        scr[0xBC/4] += rec1->unk_210[1].y;
        scr[0xC0/4] += rec1->unk_210[1].z;
    }
    scr[0x13C/4] = ((scr[0xA8/4] * 3) + scr[0xB8/4]) >> 4;
    scr[0x140/4] = ((scr[0xAC/4] * 3) + scr[0xBC/4]) >> 4;
    scr[0x144/4] = ((scr[0xB0/4] * 3) + scr[0xC0/4]) >> 4;
}
