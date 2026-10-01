void func_8002C22C(void) {
    u8 *scr = (u8 *)0x1F8002B8;
    PracticeMenuRec *rec0 = &g_practice_menu_table[0];
    PracticeMenuRec *rec1 = &g_practice_menu_table[1];

    *(s32 *)(scr + 0xA8) = 0;
    *(s32 *)(scr + 0xAC) = 0;
    *(s32 *)(scr + 0xB0) = 0;
    *(s32 *)(scr + 0xB8) = 0;
    *(s32 *)(scr + 0xBC) = 0;
    *(s32 *)(scr + 0xC0) = 0;

    if (D_800A3824 & 1) {
        *(s32 *)(scr + 0xA8) = SPAD->unk48[0][0].x;
        *(s32 *)(scr + 0xAC) = SPAD->unk48[0][0].y;
        *(s32 *)(scr + 0xB0) = SPAD->unk48[0][0].z;
        *(s32 *)(scr + 0xA8) += SPAD->unk48[0][1].x;
        *(s32 *)(scr + 0xAC) += SPAD->unk48[0][1].y;
        *(s32 *)(scr + 0xB0) += SPAD->unk48[0][1].z;
        *(s32 *)(scr + 0xB8) = rec0->unk_234[0].x;
        *(s32 *)(scr + 0xBC) = rec0->unk_234[0].y;
        *(s32 *)(scr + 0xC0) = rec0->unk_234[0].z;
        *(s32 *)(scr + 0xB8) += rec0->unk_234[1].x;
        *(s32 *)(scr + 0xBC) += rec0->unk_234[1].y;
        *(s32 *)(scr + 0xC0) += rec0->unk_234[1].z;
    } else {
        *(s32 *)(scr + 0xA8) = SPAD->unk00[0][0].x;
        *(s32 *)(scr + 0xAC) = SPAD->unk00[0][0].y;
        *(s32 *)(scr + 0xB0) = SPAD->unk00[0][0].z;
        *(s32 *)(scr + 0xA8) += SPAD->unk00[0][1].x;
        *(s32 *)(scr + 0xAC) += SPAD->unk00[0][1].y;
        *(s32 *)(scr + 0xB0) += SPAD->unk00[0][1].z;
        *(s32 *)(scr + 0xB8) = rec0->unk_210[0].x;
        *(s32 *)(scr + 0xBC) = rec0->unk_210[0].y;
        *(s32 *)(scr + 0xC0) = rec0->unk_210[0].z;
        *(s32 *)(scr + 0xB8) += rec0->unk_210[1].x;
        *(s32 *)(scr + 0xBC) += rec0->unk_210[1].y;
        *(s32 *)(scr + 0xC0) += rec0->unk_210[1].z;
    }
    if (D_800A3824 & 2) {
        *(s32 *)(scr + 0xA8) += SPAD->unk48[1][0].x;
        *(s32 *)(scr + 0xAC) += SPAD->unk48[1][0].y;
        *(s32 *)(scr + 0xB0) += SPAD->unk48[1][0].z;
        *(s32 *)(scr + 0xB8) += rec1->unk_234[0].x;
        *(s32 *)(scr + 0xBC) += rec1->unk_234[0].y;
        *(s32 *)(scr + 0xC0) += rec1->unk_234[0].z;
        *(s32 *)(scr + 0xA8) += SPAD->unk48[1][1].x;
        *(s32 *)(scr + 0xAC) += SPAD->unk48[1][1].y;
        *(s32 *)(scr + 0xB0) += SPAD->unk48[1][1].z;
        *(s32 *)(scr + 0xB8) += rec1->unk_234[1].x;
        *(s32 *)(scr + 0xBC) += rec1->unk_234[1].y;
        *(s32 *)(scr + 0xC0) += rec1->unk_234[1].z;
    } else {
        *(s32 *)(scr + 0xA8) += SPAD->unk00[1][0].x;
        *(s32 *)(scr + 0xAC) += SPAD->unk00[1][0].y;
        *(s32 *)(scr + 0xB0) += SPAD->unk00[1][0].z;
        *(s32 *)(scr + 0xB8) += rec1->unk_210[0].x;
        *(s32 *)(scr + 0xBC) += rec1->unk_210[0].y;
        *(s32 *)(scr + 0xC0) += rec1->unk_210[0].z;
        *(s32 *)(scr + 0xA8) += SPAD->unk00[1][1].x;
        *(s32 *)(scr + 0xAC) += SPAD->unk00[1][1].y;
        *(s32 *)(scr + 0xB0) += SPAD->unk00[1][1].z;
        *(s32 *)(scr + 0xB8) += rec1->unk_210[1].x;
        *(s32 *)(scr + 0xBC) += rec1->unk_210[1].y;
        *(s32 *)(scr + 0xC0) += rec1->unk_210[1].z;
    }
    *(s32 *)(scr + 0x13C) = ((*(s32 *)(scr + 0xA8) * 3) + *(s32 *)(scr + 0xB8)) >> 4;
    *(s32 *)(scr + 0x140) = ((*(s32 *)(scr + 0xAC) * 3) + *(s32 *)(scr + 0xBC)) >> 4;
    *(s32 *)(scr + 0x144) = ((*(s32 *)(scr + 0xB0) * 3) + *(s32 *)(scr + 0xC0)) >> 4;
}
