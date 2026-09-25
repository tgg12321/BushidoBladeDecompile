extern s32 ReadGeomScreen(void);
/* Draw lane `lane`'s live slots (up to 10, see func_80063BD0): per slot whose
   bit is set in D_800A3454[lane], emit one textured POLY_FT4 billboard at the
   slot's position (relative to *D_800A3470) through the composite of
   D_800A3474 and the slot's matrix, keep it when its depth is in range, then
   link every new quad into the OT at its depth. Returns 1. */
s32 func_80063E10(s32 lane) {
    extern SVECTOR D_8009B920[];
    extern SVECTOR D_8009BBE4;
    extern SVECTOR D_8009BBEC;
    extern SVECTOR D_8009BBF4;
    extern SVECTOR D_8009BBFC;
    extern s32 D_800A3720;
    extern s32 D_800A37D4;
    s32 count;
    MATRIX *mats;
    MATRIX *cm;
    s32 *sxy;
    u16 *zbuf;
    u16 *zn;
    MATRIX *m;
    POLY_FT4 *prim;
    POLY_FT4 *end;
    s32 i;
    s32 k;
    s32 bit;

    prim = (POLY_FT4 *)D_800A34EC;
    mats = (MATRIX *)((u8 *)prim + 0x28);
    cm = (MATRIX *)((u8 *)prim + 0x168);
    sxy = (s32 *)((u8 *)prim + 0x188);
    zbuf = (u16 *)((u8 *)prim + 0x19C);
    zn = (u16 *)((u8 *)prim + 0x1B0);
    if (D_800A344C[lane] < 10) {
        count = D_800A344C[lane];
    } else {
        count = 10;
    }
    func_800644FC(&count, mats, lane);
    D_800A3488 = (s32)&D_8009B920[*(s32 *)D_800A3480];
    prim = (POLY_FT4 *)D_800A37D4;
    *(s32 *)D_800A3490 = 0xE;
    *(s32 *)D_800A3494 = (((((u16 *)D_800A3488)[0] >> 4) & 0x3F) + (((u16 *)D_800A3488)[1] << 6)) << 16;
    *(s32 *)D_800A3490 = *(s32 *)D_800A3490 << 16;
    *(u16 *)D_800A3498 = ((u16 *)D_800A3488)[2];
    *(u16 *)D_800A34A0 = ((u16 *)D_800A3488)[3];
    *(u16 *)D_800A349C = ((u16 *)D_800A3488)[2] + 7;
    *(u16 *)D_800A34A4 = ((u16 *)D_800A3488)[3] + 0xF;
    *(u16 *)D_800A34D4 = *(u16 *)D_800A3498 + (*(u16 *)D_800A34A0 << 8);
    *(u16 *)D_800A34D8 = *(u16 *)D_800A349C + (*(u16 *)D_800A34A0 << 8);
    *(u16 *)D_800A34DC = *(u16 *)D_800A3498 + (*(u16 *)D_800A34A4 << 8);
    *(u16 *)D_800A34E0 = *(u16 *)D_800A349C + (*(u16 *)D_800A34A4 << 8);
    *zn = 0;
    *(s32 *)D_800A34B0 = ReadGeomScreen();
    for (i = 0; i < count; i++) {
        bit = 1 << i;
        if (!(D_800A3454[lane] & bit)) {
            continue;
        }
        m = &mats[i];
        ((u8 *)prim)[3] = 9;
        prim->code = 0x2F;
        *(s32 *)&prim->u0 = *(u16 *)D_800A34D4 + *(s32 *)D_800A3494;
        *(s32 *)&prim->u1 = *(u16 *)D_800A34D8 + *(s32 *)D_800A3490;
        *(u16 *)&prim->u2 = *(u16 *)D_800A34DC;
        *(u16 *)&prim->u3 = *(u16 *)D_800A34E0;
        m->t[0] = D_800F0EC8[lane][i].unk0 - ((s32 *)D_800A3470)[0];
        m->t[1] = D_800F0EC8[lane][i].unk4 - ((s32 *)D_800A3470)[1];
        m->t[2] = D_800F0EC8[lane][i].unk8 - ((s32 *)D_800A3470)[2];
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
        /* gte_ldclmv(r0) --- inline_c.h :150-159 */
        __asm__ volatile(
            "lhu    $12, 0(%0)\n"
            "lhu    $13, 6(%0)\n"
            "lhu    $14, 12(%0)\n"
            "mtc2   $12, $9\n"
            "mtc2   $13, $10\n"
            "mtc2   $14, $11\n"
            :: "r"(m) : "$12", "$13", "$14");
        /* gte_rtir() --- inline_c.h :514-517, post-DMPSX command word */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A49E012\n");
        /* gte_stclmv(r0) --- inline_c.h :1148-1157 */
        __asm__ volatile(
            "mfc2   $12, $9\n"
            "mfc2   $13, $10\n"
            "mfc2   $14, $11\n"
            "sh     $12, 0(%0)\n"
            "sh     $13, 6(%0)\n"
            "sh     $14, 12(%0)\n"
            :: "r"(cm) : "$12", "$13", "$14", "memory");
        /* gte_ldclmv(r0) --- inline_c.h :150-159 */
        __asm__ volatile(
            "lhu    $12, 0(%0)\n"
            "lhu    $13, 6(%0)\n"
            "lhu    $14, 12(%0)\n"
            "mtc2   $12, $9\n"
            "mtc2   $13, $10\n"
            "mtc2   $14, $11\n"
            :: "r"((u8 *)m + 2) : "$12", "$13", "$14");
        /* gte_rtir() --- inline_c.h :514-517, post-DMPSX command word */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A49E012\n");
        /* gte_stclmv(r0) --- inline_c.h :1148-1157 */
        __asm__ volatile(
            "mfc2   $12, $9\n"
            "mfc2   $13, $10\n"
            "mfc2   $14, $11\n"
            "sh     $12, 0(%0)\n"
            "sh     $13, 6(%0)\n"
            "sh     $14, 12(%0)\n"
            :: "r"((u8 *)cm + 2) : "$12", "$13", "$14", "memory");
        /* gte_ldclmv(r0) --- inline_c.h :150-159 */
        __asm__ volatile(
            "lhu    $12, 0(%0)\n"
            "lhu    $13, 6(%0)\n"
            "lhu    $14, 12(%0)\n"
            "mtc2   $12, $9\n"
            "mtc2   $13, $10\n"
            "mtc2   $14, $11\n"
            :: "r"((u8 *)m + 4) : "$12", "$13", "$14");
        /* gte_rtir() --- inline_c.h :514-517, post-DMPSX command word */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A49E012\n");
        /* gte_stclmv(r0) --- inline_c.h :1148-1157 */
        __asm__ volatile(
            "mfc2   $12, $9\n"
            "mfc2   $13, $10\n"
            "mfc2   $14, $11\n"
            "sh     $12, 0(%0)\n"
            "sh     $13, 6(%0)\n"
            "sh     $14, 12(%0)\n"
            :: "r"((u8 *)cm + 4) : "$12", "$13", "$14", "memory");
        /* gte_SetTransMatrix(r0) --- inline_c.h :360-369 */
        __asm__ volatile(
            "lw     $12, 20(%0)\n"
            "lw     $13, 24(%0)\n"
            "ctc2   $12, $5\n"
            "lw     $14, 28(%0)\n"
            "ctc2   $13, $6\n"
            "ctc2   $14, $7\n"
            :: "r"(D_800A3474) : "$12", "$13", "$14");
        /* gte_ldlv0(r0) --- inline_c.h :101-110 */
        __asm__ volatile(
            "lhu    $13, 4(%0)\n"
            "lhu    $12, 0(%0)\n"
            "sll    $13, $13, 16\n"
            "or     $12, $12, $13\n"
            "mtc2   $12, $0\n"
            "lwc2   $1, 8(%0)\n"
            :: "r"(m->t) : "$12", "$13");
        /* gte_rt() --- inline_c.h :494-497, post-DMPSX command word */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A480012\n");
        /* gte_stlvnl(r0) --- inline_c.h :1111-1117 */
        __asm__ volatile(
            "swc2   $25, 0(%0)\n"
            "swc2   $26, 4(%0)\n"
            "swc2   $27, 8(%0)\n"
            :: "r"(cm->t) : "memory");
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
            :: "r"(cm) : "$12", "$13", "$14");
        /* gte_SetTransMatrix(r0) --- inline_c.h :360-369 */
        __asm__ volatile(
            "lw     $12, 20(%0)\n"
            "lw     $13, 24(%0)\n"
            "ctc2   $12, $5\n"
            "lw     $14, 28(%0)\n"
            "ctc2   $13, $6\n"
            "ctc2   $14, $7\n"
            :: "r"(cm) : "$12", "$13", "$14");
        /* gte_ldv3(r0, r1, r2) --- inline_c.h :34-43 */
        __asm__ volatile(
            "lwc2   $0, 0(%0)\n"
            "lwc2   $1, 4(%0)\n"
            "lwc2   $2, 0(%1)\n"
            "lwc2   $3, 4(%1)\n"
            "lwc2   $4, 0(%2)\n"
            "lwc2   $5, 4(%2)\n"
            :: "r"(&D_8009BBE4), "r"(&D_8009BBEC), "r"(&D_8009BBF4));
        /* gte_rtpt() --- inline_c.h :489-492, post-DMPSX command word */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A280030\n");
        /* gte_stsxy3(r0, r1, r2) --- inline_c.h :906-913 */
        __asm__ volatile(
            "swc2   $12, 0(%0)\n"
            "swc2   $13, 0(%1)\n"
            "swc2   $14, 0(%2)\n"
            :: "r"(sxy), "r"(sxy + 1), "r"(sxy + 2) : "memory");
        /* gte_stsz(r0) --- inline_c.h :1042-1046 */
        __asm__ volatile(
            "swc2   $19, 0(%0)\n"
            :: "r"(D_800A34D0) : "memory");
        if (*(s32 *)D_800A34D0 <= 0) {
            continue;
        }
        *(s32 *)D_800A34D0 = func_80052C28(*(s32 *)D_800A34D0 - 50, 0);
        if (*(s32 *)D_800A34D0 >= 0x1005) {
            continue;
        }
        if ((*(s32 *)D_800A34B0 >> 4) >= *(s32 *)D_800A34D0) {
            continue;
        }
        zbuf[(*zn)++] = *(s32 *)D_800A34D0;
        /* gte_ldv0(r0) --- inline_c.h :16-20 (no clobber list) */
        __asm__ volatile(
            "lwc2   $0, 0(%0)\n"
            "lwc2   $1, 4(%0)\n"
            :: "r"(&D_8009BBFC));
        /* gte_rtps() --- inline_c.h :484-487, post-DMPSX command word */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A180001\n");
        /* gte_stsxy(r0) --- inline_c.h :900-904 */
        __asm__ volatile(
            "swc2   $14, 0(%0)\n"
            :: "r"(sxy + 3) : "memory");
        *(s32 *)&prim->x0 = sxy[0];
        *(s32 *)&prim->x1 = sxy[1];
        *(s32 *)&prim->x2 = sxy[2];
        *(s32 *)&prim->x3 = sxy[3];
        if (prim - (POLY_FT4 *)D_800A3720 < 0x1C1) {
            prim++;
        }
    }
    end = prim;
    for (prim = (POLY_FT4 *)D_800A37D4, k = 0; prim < end; prim++, k++) {
        D_800A34E8 = (s32)prim;
        D_800A34E4 = g_gpu_ot_ptr + zbuf[k] * 4;
        *(u32 *)D_800A34E8 = (*(u32 *)D_800A34E8 & 0xFF000000) | (*(u32 *)D_800A34E4 & 0xFFFFFF);
        *(u32 *)D_800A34E4 = (D_800A34E8 & 0xFFFFFF) | (*(u32 *)D_800A34E4 & 0xFF000000);
    }
    D_800A37D4 = (s32)end;
    return 1;
}
