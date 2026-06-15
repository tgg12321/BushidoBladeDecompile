/* func_80085270 (main.c) — SPU motion init. HEAD body (clean pure-C; the 2
 * cheats are EXTERNAL regfix reorder rules, no in-body cheat).
 * Matches ONLY with 2 regfix reorders covering pure cc1 list-scheduler
 * permutations. See notes.md — dual scheduling diff (A: sll/sra latency-gap;
 * B: loop-const + load-latency scheduling). */
void func_80085270(s16 a0, s16 a1) {
    s32 *base_ptr = (s32*)((u8*)&D_80106F28 + ((s32)(a0 << 16) >> 14));
    u8 *p = (u8*)(*base_ptr + (s16)a1 * 0xB0);
    s32 i;
    s16 *hp;
    u8 *ip;

    *(s32*)(p + 0x98) &= ~1;
    *(s32*)((*base_ptr + (s16)a1 * 0xB0) + 0x98) &= ~2;
    *(s32*)((*base_ptr + (s16)a1 * 0xB0) + 0x98) &= ~8;
    *(s32*)((*base_ptr + (s16)a1 * 0xB0) + 0x98) &= ~0x400;
    *(s32*)((*base_ptr + (s16)a1 * 0xB0) + 0x98) |= 4;

    spu_NotifyChannel((s16)(a0 | (a1 << 8)));
    spu_ResetCounter();

    i = 0;
    p[0x14] = 0;
    *(s32*)(p + 0x88) = 0;
    p[0x1C] = 0;
    p[0x18] = 0;
    p[0x19] = 0;
    p[0x1E] = 0;
    p[0x1A] = 0;
    p[0x1B] = 0;
    p[0x1F] = 0;
    p[0x17] = 0;
    p[0x21] = 0;
    p[0x1C] = 0;
    p[0x1D] = 0;
    p[0x15] = 0;
    p[0x16] = 0;

    *(s32*)(p + 0x90) = *(s32*)(p + 0x84);
    *(s32*)(p + 0x94) = *(s32*)(p + 0x8C);
    *(s16*)(p + 0x54) = *(u16*)(p + 0x56);
    *(s32*)(p + 0x0) = *(s32*)(p + 0x4);
    *(s32*)(p + 0x8) = *(s32*)(p + 0x4);

    hp = (s16*)p;
    do {
        ip = p + i;
        ip[0x37] = (u8)i;
        ip[0x27] = 0x40;
        *(s16*)((u8*)hp + 0x60) = 0x7F;
        hp++;
        i++;
    } while (i < 0x10);
    *(s16*)(p + 0x5C) = 0x7F;
    *(s16*)(p + 0x5E) = 0x7F;
}
