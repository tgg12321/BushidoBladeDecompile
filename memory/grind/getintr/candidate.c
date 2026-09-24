/* MEASUREMENT VARIANT (suffixed names to avoid clashing with later system.c decls).
 * C ref: SOTN src/main/psxsdk/libcd/bios.c getintr (tmp/sotn @8bd7c77), adapted
 * to BB2's bytes (CD_debug-gated printf DiskError path, mask 0x1D). */
typedef char Result_t_m[8];
typedef struct {
    u8 sync;
    u8 ready;
    u8 c;
} CD_intr_m;
extern volatile CD_intr_m Intr_m;
extern Result_t_m D_800F19A0_m, D_800F19A8_m, D_800F19B0_m;
extern s32 CD_status_m;
extern s32 CD_status1_m;
extern s32 CD_nopen_m;
extern s32 D_800A127C_m[];
extern s32 D_800A137C_m[];
extern volatile u8 *g_cd_index_reg;
extern volatile u8 *g_cd_param_fifo;
extern volatile u8 *g_cd_req_reg;
extern volatile u8 *g_cd_irq_reg;
extern const char D_800161E4[], D_800161F0[], D_8001620C[], D_80016220[];
extern void puts();
extern void printf();

static inline void _memcpy_m(void *_dst, void *_src, u32 _size)
{
    char *pDst = (char *)_dst;
    char *pSrc = (char *)_src;
    if (pDst == 0) {
        return;
    }
    while (_size--) {
        *pDst++ = *pSrc++;
    }
}

s32 getintr(void) {
    volatile char nReg;
    volatile Result_t_m buf;
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
    if (nReg != 3 || D_800A137C_m[CD_com]) {
        if (!(CD_status_m & 0x10) && (buf[0] & 0x10)) {
            CD_nopen_m++;
        }
        CD_status_m = buf[0];
        CD_status1_m = buf[1];
        bHasError = CD_status_m;
        bHasError &= 0x1D;
    }
    if (nReg == 5) {
        if (CD_debug > 0) {
            printf(D_800161E4);
        }
        if (CD_debug > 0) {
            printf(D_800161F0, CD_comstr[CD_com], CD_status_m, CD_status1_m);
        }
    }
    switch (nReg) {
    case 3:
        if (bHasError) {
            Intr_m.sync = 5;
            _memcpy_m(&D_800F19A0_m, &buf, sizeof(Result_t_m));
            return 2;
        }
        if (D_800A127C_m[CD_com]) {
            Intr_m.sync = 3;
            _memcpy_m(&D_800F19A0_m, &buf, sizeof(Result_t_m));
            return 1;
        }
        Intr_m.sync = 2;
        _memcpy_m(&D_800F19A0_m, &buf, sizeof(Result_t_m));
        return 2;
    case 2:
        Intr_m.sync = bHasError ? 5 : 2;
        _memcpy_m(&D_800F19A0_m, &buf, sizeof(Result_t_m));
        return 2;
    case 1:
        if (bHasError && i == 1) {
            bHasError = 0;
        }
        Intr_m.ready = bHasError ? 5 : 1;
        _memcpy_m(&D_800F19A8_m, &buf, sizeof(Result_t_m));
        *g_cd_index_reg = 0;
        *g_cd_irq_reg = 0;
        return 4;
    case 4:
        Intr_m.ready = Intr_m.c = 4;
        _memcpy_m(&D_800F19B0_m, &buf, sizeof(Result_t_m));
        _memcpy_m(&D_800F19A8_m, &buf, sizeof(Result_t_m));
        return 4;
    case 5:
        Intr_m.sync = Intr_m.ready = 5;
        _memcpy_m(&D_800F19A0_m, &buf, sizeof(Result_t_m));
        _memcpy_m(&D_800F19A8_m, &buf, sizeof(Result_t_m));
        return 6;
    default:
        puts(D_8001620C);
        printf(D_80016220, nReg);
        return 0;
    }
}
