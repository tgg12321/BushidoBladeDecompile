void func_800651F0(void) {
    s32 *p = (s32 *)D_800A347C;
    s32 t;
    s32 u;
    t = p[0]; D_800F0D3C = t;
    t = p[1]; D_800F0D40 = t;
    t = p[2];
    D_800F0BC2 = 0;
    D_800F0D44 = t;
    u = p[0]; D_800F0D54 = u;
    u = p[1]; D_800F0D58 = u;
    p = (s32 *)p[2];
    D_800F1114 = 1;
    D_800F0BC6 = 0;
    D_800F0D5C = (s32)p;
}
