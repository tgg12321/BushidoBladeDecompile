s32 func_80089EB0(u32 a0) {
    s32 shift;
    s32 entry;
    u32 *p;
    u32 t0, a3reg, a2;

    shift = g_spu_addr_shift;
    entry = g_spu_voice_key_c;
    a0 <<= shift;
    if (entry != 0) goto body;
    return 0;
body:
    t0 = 0x80000000U;
    a3reg = 0x40000000U;
    a2 = 0x0FFFFFFFU;
    p = (u32 *)entry;
    do {
        entry = p[0];
        if (entry & t0) {
            p += 2;
            continue;
        }
        if (entry & a3reg) break;
        entry &= a2;
        if ((u32)entry >= a0) return 1;
        if (a0 < (u32)entry + p[1]) return 1;
        p += 2;
    } while (1);
    return 0;
}
