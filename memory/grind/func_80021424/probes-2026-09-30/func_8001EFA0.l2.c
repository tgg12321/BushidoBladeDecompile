void func_8001EFA0(void) {
    PadState sp10;
    s16 var_v0;

    D_800A37B8 += 1;
    D_800A3778 = camera_GetBoneData();
    func_8001BCF0((u8 *)&g_practice_menu_table[D_800A3748], (D_800A37B8 << 12) / 105);
    func_8001E404();
    func_80039320();
    func_8002006C();
    func_8001BE08(&sp10);
    func_80023F08(0, (s32)&sp10);
    func_80023F08(1, (s32)&sp10);
    func_8002C61C();
    func_80030D7C();
    func_800321E8();
    func_800397A0();
    func_80046DA8(1);
    func_800335D8();

    if (g_practice_menu_table[D_800A3748].unk_96 != 0 && D_800A38DC == 1) {
        D_800A37B8 = 0x69;
    }

    if (D_800A37B8 >= 0x69 || (D_80102788.pressed & 0x400040)) {
        switch (D_800A38DC) {
        case 4:
            var_v0 = 0xC;
            break;
        case 1:
            if (D_800A3748 == 0) {
                func_8001DA2C();
                D_800A3768 = 2;
                func_80033BC0();
                return;
            }
            var_v0 = 0xC;
            break;
        case 6:
            var_v0 = 0xC;
            break;
        default:
            func_8001DA2C();
            var_v0 = 2;
            break;
        }
        D_800A3834 = var_v0;
    }
}
