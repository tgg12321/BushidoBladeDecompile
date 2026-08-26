s32 func_80037A20(s32 arg0, s32 arg1) {
    s32 *var_s0;
    s32 var_s1;
    s32 sp10[8];

    var_s0 = (s32 *)&D_80102810;
    sprintf(sp10, (s32)(&g_str_memcard_fmt), arg0, arg1);
    var_s1 = 0;
    if (firstfile(sp10, var_s0) != 0) {
        do {
            var_s1++;
            var_s0 = (s32 *)(((u8 *)var_s0) + 0x28);
        } while (nextfile(var_s0) != 0);
    }
    D_800A38C8 = var_s1;
    return var_s1;
}
