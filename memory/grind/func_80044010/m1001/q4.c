void func_80044010(s32 a0_raw, s32 a1_raw) {
    s32 *hdr = (s32 *)a0_raw;
    s16 slot = a1_raw;
    s32 *p;
    s32 v;
    s32 i;
    v = *hdr;
    *hdr = (u16)(v | 0x8000);
    p = hdr + 1;
    D_80103608[slot] = p;
    D_80103658[slot] = v & 0x7FFF;
    if (v & 0x8000) {
        return;
    }
    for (i = 0; i < (u16)v; i++) {
        *p++ += (s32)hdr;
    }
}
