/* 0x8009B388: two adjacent 8-byte sprite cells (0x8009B388..0x8009B397),
   the cell table func_8005D554 hands func_80073728 for the row's header
   record 2 (cell 0) and records 3/4 (cell 1). One object: spelled
   &D_8009B388[0] / [1] the function matches, with the base kept in $s7 as
   the original does; spelled as two separate symbols the same body measures
   40 at 174 insns (memory/grind/func_8005D554/evidence.md, s24). Replaces
   the splat per-cell scalars D_8009B388 / D_8009B390 in C. */
extern Unk8009B400Record D_8009B388[2];
extern u8 D_8009B2E0[];
extern s32 D_800A326C;
/* (s24) = the ablation table in memory/grind/func_8005D554/evidence.md, s24. */
s32 func_8005D554(s32 arg0, s32 arg1) {
    Env5E54C s;
    s32 i;
    u32 base_x;
    u32 base_y;
    s32 ft4;
    s32 row_off;
    s32 x;
    s32 y;
    s32 tmp1;  /* FAKE: carrier, see the y sites (s24: 15 without it) */
    s32 tmp2;  /* FAKE: carrier, see the y sites (s24: 15 without it) */
    s32 scale; /* FAKE: constant 0x100 in a local; the literal scores 15 (s24) */
    s32 ot;    /* FAKE: constant 1 in a local; the literal scores 2 (s24) */
    u8 *hdr0;     /* FAKE: pointer alias of D_8009B2E0; direct use scores 41 (s24) */
    u8 *hdr1;     /* FAKE: hdr0 + 0xC named; folding it scores 2 (s24) */
    u8 *hdr1_row; /* FAKE: hoisted row address; inlining it scores 24 (s24) */

    D_800A326C %= 4;

    ft4 = arg0;
    if (arg1 > 0) arg1 -= 1;

    D_800A3418 ^= rand();
    base_x = ((u32)(D_800A3418 * 0x260)) >> 0xF;
    D_800A3418 ^= rand();
    base_y = ((u32)(D_800A3418 * 0xDC)) >> 0xF;

    i = 0;
    if (i < ((D_800A326C + 1) * 2)) {
        scale = 0x100;
        ot = 1;
        row_off = arg1 * 0x3C;
        hdr0 = D_8009B2E0;
        hdr1 = hdr0 + 0xC;
        hdr1_row = hdr1 + row_off;
        do {
            s.has_color = 0;
            /* FAKE: integer sum keeps `addu v0,s0,fp`; the pointer sum scores 1 (s24) */
            s.header = (Unk8009B398Record *)(row_off + (s32)hdr0);
            s.table = &D_8009B388[0];
            D_800A3418 ^= rand();
            s.scale_y = scale;
            s.scale_x = scale;
            D_800A3418 ^= rand();
            /* FAKE: split init; one expression scores 8 at 178 insns (s24) */
            x = (s32)base_x - 0x19;
            x += ((u32)(D_800A3418 * 0x32) >> 0xF);
            /* FAKE: tmp1 is written twice (y base, then a copy of ft4) only so
               sched1 places the y base after the argument setup; without the
               early base 15, without the second write 54, one carrier for both
               halves 30 (s24) */
            tmp1 = (s32)base_y - 0xC;
            s.x = x;
            D_800A3418 ^= rand();
            y = tmp1 + ((u32)(D_800A3418 * 0x19) >> 0xF);
            s.semi = 0;
            s.ot_idx = ot;
            tmp1 = ft4;
            s.ft4_out = tmp1;
            s.y = y;
            ft4 = func_80073728((s32)&s, 0);

            s.has_color = 0;
            s.scale_y = scale;
            s.scale_x = scale;
            s.table = &D_8009B388[1];
            s.header = (Unk8009B398Record *)(hdr1_row + (D_800A3418 & 1) * 0xC);
            D_800A3418 ^= rand();
            /* FAKE: split init, as in the first half */
            x = (s32)base_x - 0x32;
            x += ((u32)(D_800A3418 * 0x64) >> 0xF);
            /* FAKE: tmp2 written twice, as tmp1 above */
            tmp2 = (s32)base_y - 0x19;
            s.x = x;
            D_800A3418 ^= rand();
            y = tmp2 + ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.semi = 0;
            s.ot_idx = ot;
            tmp2 = ft4;
            s.ft4_out = tmp2;
            s.y = y;
            ft4 = func_80073728((s32)&s, 0);
            i += 1;
        } while (i < ((D_800A326C + 1) * 2));
    }
    D_800A326C += 1;
    return ft4;
}
