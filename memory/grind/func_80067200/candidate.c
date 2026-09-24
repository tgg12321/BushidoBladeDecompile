extern s16 D_800A3438[];
extern SVECTOR D_800F0B78[];
/* 20-byte record table at 0x800EFC78: 4 rows (arg1) of 48 records. Object
 * model evidence: asm/funcs/func_80067200.s addresses it as
 * base + arg1*0x3C0 + i*20 with halfword stores at +0..+0xC, +0x10, +0x12
 * (+0xE untouched here); 0x3C0 / 20 = 48 = the loop's record count. */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
    s16 unk10;
    s16 unk12;
} Unk800EFC78Record;
extern Unk800EFC78Record D_800EFC78[][48];
/* func_80067200 -- ordinary C plus five GTE islands, each the body of one
 * PsyQ Run-time Library Release 4.3 inline_c.h (DMPSX) macro:
 * gte_SetRotMatrix(r0) :297-310 (twice), gte_ldv0(r0) :16-20,
 * gte_rtv0() :499-502, gte_stlvnl(r0) :1111-1117. Instruction text, "r"
 * operand and clobbers are the header's; only separators/whitespace differ,
 * except gte_rtv0's command word: the header carries the DMPSX placeholder
 * `.word 0x0000013f`, which Sony's DMPSX tool rewrote after compilation; this
 * build has no DMPSX pass, so the island carries the post-DMPSX word
 * 0x4A486012 (cop2 MVMVA sf=1 mx=rot v=V0 cv=none lm=0) that the original
 * binary contains at 0x80067630. */
u8 func_80067200(s32 arg0, s32 arg1, s32 arg2) {
    SVECTOR v;
    s32 r[6];
    MATRIX m;
    SVECTOR ang;
    VECTOR out;
    SVECTOR sc;
    s16 base;
    s16 mask;
    s16 amask;
    s16 aoff;
    u8 count;
    s32 i;
    Unk800EFC78Record *p;

    if (arg0 < 4) {
        base = 0x41;
        mask = 0x3F;
    } else {
        base = 0x31;
        mask = 0x1F;
    }
    D_800A3438[arg1] = 0;
    /* gte_SetRotMatrix(r0) --- inline_c.h :297-310 */
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
    if (arg2 == 0) {
        D_800F0C10[arg1][0].unk0 = ((s32 *)D_800A347C)[0];
        D_800F0C10[arg1][0].unk4 = ((s32 *)D_800A347C)[1];
        D_800F0C10[arg1][0].unk8 = ((s32 *)D_800A347C)[2];
        D_800F0B78[arg1].vx = ((u16 *)D_800A3478)[0];
        D_800F0B78[arg1].vy = ((u16 *)D_800A3478)[1];
        D_800F0B78[arg1].vz = ((u16 *)D_800A3478)[2];
    }
    count = 0x30;
    r[5] = rand();
    if (arg0 < 2) {
        amask = 0xFF;
        aoff = 0x7F;
    } else if (arg0 < 4) {
        amask = 0xFFF;
        aoff = 0;
    } else if (arg0 < 6) {
        amask = 0x7F;
        aoff = 0x3F;
    } else if (arg0 < 8) {
        amask = 0x7F;
        aoff = 0x3F;
    }
    sc.vx = D_800F0B78[arg1].vx;
    sc.vy = D_800F0B78[arg1].vy;
    sc.vz = D_800F0B78[arg1].vz;
    for (i = (count >> 1) * arg2; i < (count >> 1) * (arg2 + 1); i++) {
        r[0] = r[5] ^ rand();
        r[1] = r[0] ^ rand();
        r[2] = r[1] ^ rand();
        r[3] = r[2] ^ rand();
        r[4] = r[3] ^ rand();
        r[5] = r[4] ^ rand();
        ang.vy = r[0] = (r[0] & amask) - aoff;
        ang.vx = r[1] = (r[1] & amask) - aoff;
        ang.vz = r[2] = (r[2] & amask) - aoff;
        r[3] &= mask;
        r[4] &= mask;
        r[5] &= mask;
        v.vx = sc.vx * (base + r[3]) / mask;
        v.vy = sc.vy * (base + r[4]) / mask;
        v.vz = sc.vz * (base + r[5]) / mask;
        p = &D_800EFC78[arg1][i];
        p->unk6 = i / 16;
        p->unk12 = 0;
        p->unk4 = 0;
        p->unk2 = 0;
        p->unk0 = 0;
        p->unk10 = 1;
        RotMatrix((s16 *)&ang, (u8 *)&m);
        /* gte_SetRotMatrix(r0) --- inline_c.h :297-310 */
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
            :: "r"(&m) : "$12", "$13", "$14");
        /* gte_ldv0(r0) --- inline_c.h :16-20 (no clobber list) */
        __asm__ volatile(
            "lwc2   $0, 0(%0)\n"
            "lwc2   $1, 4(%0)\n"
            :: "r"(&v));
        /* gte_rtv0() --- inline_c.h :499-502, post-DMPSX command word */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A486012\n");
        /* gte_stlvnl(r0) --- inline_c.h :1111-1117 */
        __asm__ volatile(
            "swc2   $25, 0(%0)\n"
            "swc2   $26, 4(%0)\n"
            "swc2   $27, 8(%0)\n"
            :: "r"(&out) : "memory");
        p->unk8 = out.vx;
        p->unkA = out.vy;
        p->unkC = out.vz;
    }
    return 1;
}
