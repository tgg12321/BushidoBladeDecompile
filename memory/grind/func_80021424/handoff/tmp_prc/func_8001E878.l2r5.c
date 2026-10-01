void func_8001E878(void) {
    PadState buf;
    s32 v0;
    v0 = camera_GetBoneData();
    D_800A3778 = v0;
    func_8001A820((s32)&g_practice_menu_table[0].unk_168, (s32)&g_practice_menu_table[1].unk_168, (s32)&g_practice_menu_table[0], (s32)&g_practice_menu_table[1]);
    if (D_800A38BA != 0) {
        func_8001B478((s32)&g_practice_menu_table[D_800A36F6]);
    }
    func_8001E404();
    func_80039320();
    func_8002006C();
    func_8001BE20(0, &buf);
    func_80023F08(0, (s32)&buf);
    func_8001BE20(1, &buf);
    func_80023F08(1, (s32)&buf);
    func_8002C61C();
    func_80030D7C();
    func_800321E8();
    func_800397A0();
    if (D_800A38BA != 0 && D_800A36FA == 0) {
        func_8001E800();
    } else {
        func_8003E6A0(g_practice_menu_table[0].unk_F4.x, g_practice_menu_table[0].unk_F4.z);
        func_8003E6A0(g_practice_menu_table[1].unk_F4.x, g_practice_menu_table[1].unk_F4.z);
    }
    func_80046DA8((D_800A3690 ^ 1) != 0);
    func_8001CE60();
    func_800335D8();
    func_8001C8DC();
}
