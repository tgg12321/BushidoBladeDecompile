/* REJECTED (s15b, 2026-09-07) -- dead stores to a local do NOT reach loop.c.
 * 32 dead stores `dz = 1..32;` at the top of the loop body: score 13, build_insns 93,
 * loop insn_count 91 -- BIT-IDENTICAL to the baseline, all four switch-comparison
 * constants still hoisted.  cse1 ends with delete_dead_from_cse (tools/gcc-2.7.2/cse.c:8684),
 * whose own comment says it exists so that "loop ... won't try to move dead invariants out
 * of loops".  Any dead / unused local computation is therefore invisible to
 * count_loop_regs_set.  This closes the whole dead-store / dead-scalar-local / constant-holder
 * FAKE family as a source of loop-time insn_count for this function.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;
    s32 dz;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        dz = 1;
        dz = 2;
        dz = 3;
        dz = 4;
        dz = 5;
        dz = 6;
        dz = 7;
        dz = 8;
        dz = 9;
        dz = 10;
        dz = 11;
        dz = 12;
        dz = 13;
        dz = 14;
        dz = 15;
        dz = 16;
        dz = 17;
        dz = 18;
        dz = 19;
        dz = 20;
        dz = 21;
        dz = 22;
        dz = 23;
        dz = 24;
        dz = 25;
        dz = 26;
        dz = 27;
        dz = 28;
        dz = 29;
        dz = 30;
        dz = 31;
        dz = 32;
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
