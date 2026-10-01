void func_800213A0(s16 *arg0) {
    s16 a1 = arg0[0x86 / 2];
    if (a1 != arg0[0x88 / 2]) {
        if (a1 != arg0[0x8E / 2]) {
            return;
        }
    }
    {
        s16 *v = (s16 *)D_800A3860[arg0[0x4A / 2]];
        arg0[0x86 / 2] = (s16)((a1 + 1) % v[0x14 / 2]);
    }
}
