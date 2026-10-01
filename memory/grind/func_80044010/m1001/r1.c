void func_80044010(s32 a0_raw, s32 a1_raw) {
    s32 *p = (s32 *)a0_raw;
    s16 slot = a1_raw;
    s32 *base = p;
    s32 v;
    s32 i;
    u16 n;
    v = *p;
    *p = (v | 0x8000) & 0xFFFF;
    p++;
    D_80103608[slot] = p;
    D_80103658[slot] = v & 0x7FFF;
    if (!(v & 0x8000)) {
        n = v;
        for (i = 0; i < n; i++) {
            *p++ += (s32)base;
        }
    }
}
