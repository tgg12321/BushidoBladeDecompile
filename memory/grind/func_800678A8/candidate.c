extern s16 D_800EFC8A[];
extern s16 D_800A3438[];
extern s16 D_800F0B98[];
extern u16 D_8009B890[];
extern u16 D_8009B8B0[];
extern u16 D_8009B998[];
extern u16 D_8009B9B8[];
u8 func_800678A8(s32 arg0, s32 arg1) {
    extern s32 D_800A34EC;
    extern s32 D_800A37D4;
    extern s32 D_800A3724;
    s32 outer = D_800A34EC;
    s16 *p2 = (s16 *)(outer + 2);
    s16 *p6C = (s16 *)(outer + 0x6C);
    u16 *tbl;

    D_800A3724 = outer + 0x1AC;
    *(s32 *)(outer + 0x80) = D_800A37D4;
    *(s32 *)(outer + 4) = 0x895440;
    *(s32 *)D_800A3490 = 0x2E;

    if (arg0 < 2) {
        D_800A3488 = (s32)D_8009B890;
        *(s16 *)(outer + 2) = 7;
        *(s16 *)(outer + 0) = 7;
        *(s16 *)D_800A34A8 = 0x40;
        *(s16 *)D_800A34AC = 0x20;
        D_800F0B98[arg0] = 3;
    } else if (arg0 < 4) {
        D_800A3488 = (s32)D_8009B890;
        *(s16 *)(outer + 2) = 7;
        *(s16 *)(outer + 0) = 7;
        *(s16 *)D_800A34A8 = 0x20;
        *(s16 *)D_800A34AC = 0x10;
        D_800F0B98[arg0] = 3;
    } else if (arg0 < 6) {
        s16 lv = D_800EFC8A[arg1 * 0x1E0] >> 3;
        *(s16 *)(outer + 0x70) = lv;
        if (lv >= 4) {
            *(s16 *)(outer + 0x70) = 3;
        }
        if (D_800A34F0[arg0 - 4] != 0) {
            D_800A3488 = (s32)&D_8009B9B8[*(s16 *)(outer + 0x70) * 4];
        } else {
            D_800A3488 = (s32)&D_8009B998[*(s16 *)(outer + 0x70) * 4];
        }
        *(s16 *)(outer + 0) = 0x1F;
        *p2 = 0x20;
        *(s16 *)D_800A34A8 = 0xC0;
        *(s16 *)D_800A34AC = 0x30;
        *(s32 *)D_800A3490 = 0xF;
        D_800F0B98[arg0] = 1;
    } else if (arg0 < 8) {
        *(s32 *)D_800A3490 = 0x2E;
        D_800A3488 = (s32)D_8009B8B0;
        *(s16 *)(outer + 2) = 0xF;
        *(s16 *)(outer + 0) = 0xF;
        *(s16 *)D_800A34A8 = 0xC0;
        *(s16 *)D_800A34AC = 0x60;
        D_800F0B98[arg0] = 2;
    }

    tbl = (u16 *)D_800A3488;
    *(s32 *)D_800A3494 = (((tbl[0] >> 4) & 0x3F) + (tbl[1] << 6)) << 16;
    *(s32 *)D_800A3490 <<= 16;
    *(s16 *)D_800A3498 = ((u16 *)D_800A3488)[2];
    *(s16 *)D_800A34A0 = ((u16 *)D_800A3488)[3];
    *(s16 *)D_800A349C = *(u16 *)outer + ((u16 *)D_800A3488)[2];
    *(s16 *)D_800A34A4 = *(u16 *)p2 + ((u16 *)D_800A3488)[3];
    *(s16 *)D_800A34D4 = *(u16 *)D_800A3498 + (*(u16 *)D_800A34A0 << 8);
    *(s16 *)D_800A34D8 = *(u16 *)D_800A349C + (*(u16 *)D_800A34A0 << 8);
    *(s16 *)D_800A34DC = *(u16 *)D_800A3498 + (*(u16 *)D_800A34A4 << 8);
    *(s16 *)D_800A34E0 = *(u16 *)D_800A349C + (*(u16 *)D_800A34A4 << 8);

    /* PsyQ libgte inline macro gte_SetRotMatrix(r0) --- PsyQ Run-time Library
     * Release 4.3 inline_c.h (DMPSX v3) :297-310,
     * verbatim body, operand and clobbers. */
    __asm__ volatile(
        "lw     $12, 0(%0)\n"
        "lw     $13, 4(%0)\n"
        "ctc2   $12, $0\n"
        "ctc2   $13, $1\n"
        "lw     $12, 8(%0)\n"
        "lw     $13, 12(%0)\n"
        "lw     $14, 16(%0)\n"
        "ctc2   $12, $2\n"
        "ctc2   $13, $3\n"
        "ctc2   $14, $4\n"
        :: "r"(D_800A3474) : "$12", "$13", "$14");

    if (D_800A3438[arg1] < D_800F0B98[arg0]) {
        D_800F0C10[arg1][D_800A3438[arg1]].unk0 = D_800F0C10[arg1][0].unk0;
        D_800F0C10[arg1][D_800A3438[arg1]].unk4 = D_800F0C10[arg1][0].unk4;
        D_800F0C10[arg1][D_800A3438[arg1]].unk8 = D_800F0C10[arg1][0].unk8;
    }

    /* PsyQ libgte inline macro gte_ReadGeomScreen(r0) --- PsyQ Run-time Library
     * Release 4.3 inline_c.h (DMPSX v3) :1236-1242,
     * verbatim body, operand and clobbers. */
    __asm__ volatile(
        "cfc2   $12, $26\n"
        "nop\n"
        "sw     $12, 0(%0)\n"
        :: "r"(D_800A34B0) : "$12", "memory");

    *p6C = (D_800A3438[arg1] + 1) * 16;
    if (D_800A3438[arg1] < D_800F0B98[arg0] - 1) {
        D_800A3438[arg1]++;
    }
}
