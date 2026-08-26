void func_80061710(s32 *arg0, s32 arg1) {
    s32 *p;
    u8 *q;
    s32 val;
    D_800A3468 = (s32)&D_800F116C;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000E;
        q = &D_800F115C + 2;
        break;
    case 1:
        val = 0x21000F;
        q = &D_800F115C + 3;
        break;
    default:
        goto done;
    }
    *q = 0;
    D_800F1180 = (s32)q;
    D_800F116C = val;
done:
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800F1148 = *p;
    D_800A3464 = 0x10FF10;
}
