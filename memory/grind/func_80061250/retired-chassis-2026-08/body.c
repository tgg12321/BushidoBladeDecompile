void func_80061250(s32 *arg0) {
    s32 *v1 = (s32 *)&D_800F116C;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if ((&D_800F1159)[0] != 0) {
        if ((&D_800F1159)[1] != 0) {
            (&D_800F1159)[1] = 0;
            (&D_800F1159)[0] = 0;
        }
        if ((&D_800F1159)[0] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)&D_800F1159;
    *(s32 *)D_800A3468 = 0x210009;
    goto end;
check_one_zero:
    if ((&D_800F1159)[1] == 0) {
        D_800F1180 = (s32)((&D_800F1159) + 1);
        *v1 = 0x21000A;
    }
end:
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0xFF0060;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
