s32 sys_VSync(s32 a0) {
    s32 s0_val;
    s32 s1_val;

    s0_val = *D_800A1510;
    s1_val = (*D_800A1514 - D_800A1518) & 0xFFFF;

    if (a0 < 0) {
        return g_sys_dma_region;
    }
    if (a0 == 1) {
        return s1_val;
    }

    {
        s32 frame;
        s32 count;

        if (a0 > 0) {
            s32 base = D_800A151C - 1;
            frame = base + a0;
        } else {
            frame = D_800A151C;
        }
        count = 0;
        if (a0 > 0) {
            count = a0 - 1;
        }
        func_80082A14(frame, count);
        do { } while (0);
    }

    s0_val = *D_800A1510;
    func_80082A14(g_sys_dma_region + 1, 1);

    if (s0_val & 0x400000) {
        volatile s32 *ptr = D_800A1510;
        if ((s32)(s0_val ^ *ptr) >= 0) {
            do {
            } while (!((s0_val ^ *ptr) & 0x80000000));
        }
    }

    D_800A151C = g_sys_dma_region;
    D_800A1518 = *D_800A1514;

    return s1_val;
}
