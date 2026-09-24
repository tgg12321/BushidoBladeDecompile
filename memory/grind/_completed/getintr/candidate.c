/* PsyQ 4.0 LIBCD BIOS: getintr — verbatim-linked Sony object (census 2026-07-09);
 * C ref: SOTN src/main/psxsdk/libcd/bios.c (v1.77; tmp/sotn @8bd7c77). BB2 links
 * v1.86, whose bytes differ in two places: the DiskError report is two
 * CD_debug-gated printf()s (not puts + one gated printf), and the error mask
 * is 0x1D (CdlStatError|SeekError|IdError|ShellOpen = 0x1D, spelled as the
 * value). CD_status is Sony's `int`; this TU declares the libc-side u_char
 * view, hence the s32 accesses (as in CD_initintr / CD_init below). */
typedef char Result_t[8];
extern s32 g_cd_init_flag; /* 0x800A11C8 = Sony CD_status1 */
extern s32 D_800A11CC;     /* Sony CD_nopen */
extern s32 D_800A127C[];   /* per-command "ack is complete" flags */
extern s32 D_800A137C[];   /* per-command "status valid" flags */
extern void D_800F19A0;    /* Result_t result buffers */
extern void D_800F19A8;
extern void D_800F19B0;
extern char D_800161E4[]; /* "DiskError: " */
extern char D_800161F0[]; /* "com=%s,code=(%02x:%02x)\n" */
extern char D_8001620C[]; /* "CDROM: unknown intr" */
extern char D_80016220[]; /* "(%d)\n" */
extern volatile u8 *g_cd_index_reg;
extern volatile u8 *g_cd_param_fifo;
extern volatile u8 *g_cd_req_reg;
extern volatile u8 *g_cd_irq_reg;
extern void puts();
extern void printf();

s32 getintr(void) {
    volatile char nReg;
    volatile Result_t buf;
    s32 i, j;
    s32 bHasError;

    *g_cd_index_reg = 1;

    nReg = *g_cd_irq_reg & 0x7;

    if (nReg == 0) {
        return 0;
    }

    bHasError = 0;

    while (nReg != (*g_cd_irq_reg & 7)) {
        nReg = *g_cd_irq_reg & 0x7;
    }

    for (i = 0; i < 8; i++) {
        if ((*g_cd_index_reg & 0x20) == 0) {
            break;
        }
        buf[i] = *g_cd_param_fifo;
    }
    for (j = i; j < 8; j++) {
        buf[j] = 0;
    }

    *g_cd_index_reg = 1;
    *g_cd_irq_reg = 7;
    *g_cd_req_reg = 7;
    if (nReg != 3 || D_800A137C[CD_com]) {
        if (!(*(s32 *)&CD_status & 0x10) && (buf[0] & 0x10)) {
            D_800A11CC++;
        }
        *(s32 *)&CD_status = buf[0];
        g_cd_init_flag = buf[1];
        bHasError = *(s32 *)&CD_status;
        bHasError &= 0x1D;
    }
    if (nReg == 5) {
        if (CD_debug > 0) {
            printf(D_800161E4);
        }
        if (CD_debug > 0) {
            printf(D_800161F0, CD_comstr[CD_com], *(s32 *)&CD_status, g_cd_init_flag);
        }
    }
    switch (nReg) {
    case 3:
        if (bHasError) {
            Intr.sync = 5;
            _memcpy(&D_800F19A0, &buf, sizeof(Result_t));
            return 2;
        }
        if (D_800A127C[CD_com]) {
            Intr.sync = 3;
            _memcpy(&D_800F19A0, &buf, sizeof(Result_t));
            return 1;
        }
        Intr.sync = 2;
        _memcpy(&D_800F19A0, &buf, sizeof(Result_t));
        return 2;
    case 2:
        Intr.sync = bHasError ? 5 : 2;
        _memcpy(&D_800F19A0, &buf, sizeof(Result_t));
        return 2;
    case 1:
        if (bHasError && i == 1) {
            bHasError = 0;
        }
        Intr.ready = bHasError ? 5 : 1;
        _memcpy(&D_800F19A8, &buf, sizeof(Result_t));
        *g_cd_index_reg = 0;
        *g_cd_irq_reg = 0;
        return 4;
    case 4:
        Intr.ready = Intr.c = 4;
        _memcpy(&D_800F19B0, &buf, sizeof(Result_t));
        _memcpy(&D_800F19A8, &buf, sizeof(Result_t));
        return 4;
    case 5:
        Intr.sync = Intr.ready = 5;
        _memcpy(&D_800F19A0, &buf, sizeof(Result_t));
        _memcpy(&D_800F19A8, &buf, sizeof(Result_t));
        return 6;
    default:
        puts(D_8001620C);
        printf(D_80016220, nReg);
        return 0;
    }
}
