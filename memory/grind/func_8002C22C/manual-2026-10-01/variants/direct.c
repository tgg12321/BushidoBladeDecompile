void func_8002C22C(void) {
    u8 *scr = (u8 *)0x1F8002B8;

    *(s32 *)(scr + 0xA8) = 0;
    *(s32 *)(scr + 0xAC) = 0;
    *(s32 *)(scr + 0xB0) = 0;
    *(s32 *)(scr + 0xB8) = 0;
    *(s32 *)(scr + 0xBC) = 0;
    *(s32 *)(scr + 0xC0) = 0;

    if (D_800A3824 & 1) {
        *(s32 *)(scr + 0xA8) = *(s32 *)0x1F800048;
        *(s32 *)(scr + 0xAC) = *(s32 *)0x1F80004C;
        *(s32 *)(scr + 0xB0) = *(s32 *)0x1F800050;
        *(s32 *)(scr + 0xA8) += *(s32 *)0x1F800054;
        *(s32 *)(scr + 0xAC) += *(s32 *)0x1F800058;
        *(s32 *)(scr + 0xB0) += *(s32 *)0x1F80005C;
        *(s32 *)(scr + 0xB8) = g_practice_menu_table[0].unk_234[0].x;
        *(s32 *)(scr + 0xBC) = g_practice_menu_table[0].unk_234[0].y;
        *(s32 *)(scr + 0xC0) = g_practice_menu_table[0].unk_234[0].z;
        *(s32 *)(scr + 0xB8) += g_practice_menu_table[0].unk_234[1].x;
        *(s32 *)(scr + 0xBC) += g_practice_menu_table[0].unk_234[1].y;
        *(s32 *)(scr + 0xC0) += g_practice_menu_table[0].unk_234[1].z;
    } else {
        *(s32 *)(scr + 0xA8) = *(s32 *)0x1F800000;
        *(s32 *)(scr + 0xAC) = *(s32 *)0x1F800004;
        *(s32 *)(scr + 0xB0) = *(s32 *)0x1F800008;
        *(s32 *)(scr + 0xA8) += *(s32 *)0x1F80000C;
        *(s32 *)(scr + 0xAC) += *(s32 *)0x1F800010;
        *(s32 *)(scr + 0xB0) += *(s32 *)0x1F800014;
        *(s32 *)(scr + 0xB8) = g_practice_menu_table[0].unk_210[0].x;
        *(s32 *)(scr + 0xBC) = g_practice_menu_table[0].unk_210[0].y;
        *(s32 *)(scr + 0xC0) = g_practice_menu_table[0].unk_210[0].z;
        *(s32 *)(scr + 0xB8) += g_practice_menu_table[0].unk_210[1].x;
        *(s32 *)(scr + 0xBC) += g_practice_menu_table[0].unk_210[1].y;
        *(s32 *)(scr + 0xC0) += g_practice_menu_table[0].unk_210[1].z;
    }
    if (D_800A3824 & 2) {
        *(s32 *)(scr + 0xA8) += *(s32 *)0x1F800060;
        *(s32 *)(scr + 0xAC) += *(s32 *)0x1F800064;
        *(s32 *)(scr + 0xB0) += *(s32 *)0x1F800068;
        *(s32 *)(scr + 0xB8) += g_practice_menu_table[1].unk_234[0].x;
        *(s32 *)(scr + 0xBC) += g_practice_menu_table[1].unk_234[0].y;
        *(s32 *)(scr + 0xC0) += g_practice_menu_table[1].unk_234[0].z;
        *(s32 *)(scr + 0xA8) += *(s32 *)0x1F80006C;
        *(s32 *)(scr + 0xAC) += *(s32 *)0x1F800070;
        *(s32 *)(scr + 0xB0) += *(s32 *)0x1F800074;
        *(s32 *)(scr + 0xB8) += g_practice_menu_table[1].unk_234[1].x;
        *(s32 *)(scr + 0xBC) += g_practice_menu_table[1].unk_234[1].y;
        *(s32 *)(scr + 0xC0) += g_practice_menu_table[1].unk_234[1].z;
    } else {
        *(s32 *)(scr + 0xA8) += *(s32 *)0x1F800024;
        *(s32 *)(scr + 0xAC) += *(s32 *)0x1F800028;
        *(s32 *)(scr + 0xB0) += *(s32 *)0x1F80002C;
        *(s32 *)(scr + 0xB8) += g_practice_menu_table[1].unk_210[0].x;
        *(s32 *)(scr + 0xBC) += g_practice_menu_table[1].unk_210[0].y;
        *(s32 *)(scr + 0xC0) += g_practice_menu_table[1].unk_210[0].z;
        *(s32 *)(scr + 0xA8) += *(s32 *)0x1F800030;
        *(s32 *)(scr + 0xAC) += *(s32 *)0x1F800034;
        *(s32 *)(scr + 0xB0) += *(s32 *)0x1F800038;
        *(s32 *)(scr + 0xB8) += g_practice_menu_table[1].unk_210[1].x;
        *(s32 *)(scr + 0xBC) += g_practice_menu_table[1].unk_210[1].y;
        *(s32 *)(scr + 0xC0) += g_practice_menu_table[1].unk_210[1].z;
    }
    *(s32 *)(scr + 0x13C) = ((*(s32 *)(scr + 0xA8) * 3) + *(s32 *)(scr + 0xB8)) >> 4;
    *(s32 *)(scr + 0x140) = ((*(s32 *)(scr + 0xAC) * 3) + *(s32 *)(scr + 0xBC)) >> 4;
    *(s32 *)(scr + 0x144) = ((*(s32 *)(scr + 0xB0) * 3) + *(s32 *)(scr + 0xC0)) >> 4;
}
