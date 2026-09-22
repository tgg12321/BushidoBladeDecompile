/* PsyQ 4.0 LIBCD BIOS: CD_cw — verbatim-linked Sony object (census 2026-07-09);
 * C ref: SOTN src/main/psxsdk/libcd/bios.c (v1.77; BB2 links v1.86, which sets
 * CD_mode before issuing the command and copies the result unconditionally).
 * Identity + object map: memory/closer/libcd-identity.md. */
typedef struct {
    u8 sync;  /* 0x800A1494 */
    u8 ready; /* 0x800A1495 */
    u8 c;     /* 0x800A1496 */
} CD_intr;

typedef struct {
    s32 time;  /* 0x800F19B8 */
    s32 count; /* 0x800F19BC */
    char *name; /* 0x800F19C0 */
} Alarm_t;

/* bios.c's own module state. Intr is DEFINED here, as in Sony's bios.c (SOTN:
 * `static volatile CD_intr Intr = {0};`): it is the IRQ-mutated interrupt
 * status block, and bb2.ld links this object's .data at 0x800A1494, between
 * the two halves of the split data asm. The older per-byte names
 * (g_cd_status_a/b/c, D_800A1494..96) are linker aliases of the same bytes. */
static volatile CD_intr Intr = {0};
extern Alarm_t Alarm;

extern s32 D_800A12FC[];  /* per-command "clears ready" flags; [com + 0x40] = param count */
extern s32 D_800A13FC[];  /* per-command "needs param" flags (= D_800A12FC + 0x40) */
extern volatile u8 *g_cd_req_reg;
extern volatile u8 *g_cd_param_fifo;
extern char D_80016254[]; /* "%s...\n" */
extern char D_8001625C[]; /* "%s: no param\n" */
extern char D_8001626C[]; /* "CD_cw" */

static inline void _memcpy(void *_dst, void *_src, u32 _size)
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

static inline void set_alarm(char *name)
{
    Alarm.time = VSync(-1) + 0x3C0;
    Alarm.count = 0;
    Alarm.name = name;
}

static inline s32 get_alarm(void)
{
    if (Alarm.time < VSync(-1) || Alarm.count++ > 0x3C0000) {
        puts(&D_800161B8);
        printf(&D_800161C8, Alarm.name, CD_comstr[CD_com],
               CD_intstr[Intr.sync], CD_intstr[Intr.ready]);
        CD_flush();
        return -1;
    }
    return 0;
}

static inline void callback(void)
{
    s32 status;
    u8 saved;

    saved = *D_800A147C & 3;
    while (1) {
        status = getintr();
        if (status == 0) {
            break;
        }
        if ((status & 4) && CD_cbready != 0) {
            ((void (*)(u8, void *))CD_cbready)(Intr.ready, &D_800F19A8);
        }
        if ((status & 2) && CD_cbsync != 0) {
            ((void (*)(u8, void *))CD_cbsync)(Intr.sync, &D_800F19A0);
        }
    }
    *D_800A147C = saved;
}

s32 CD_cw(u8 com, u8 *param, u8 *result, s32 async)
{
    s32 i;

    if (CD_debug > 1) {
        printf(D_80016254, CD_comstr[com]);
    }
    if (D_800A13FC[com] != 0 && param == 0) {
        if (CD_debug > 0) {
            printf(D_8001625C, CD_comstr[com]);
        }
        return -2;
    }
    CD_sync(0, 0);
    if (com == 2) {
        for (i = 0; i < 4; i++) {
            (&CD_pos)[i] = param[i];
        }
    }
    if (com == 0xE) {
        CD_mode = param[0];
    }
    Intr.sync = 0;
    if (D_800A12FC[com]) {
        Intr.ready = 0;
    }
    *D_800A147C = 0;
    for (i = 0; i < D_800A12FC[com + 0x40]; i++) {
        *g_cd_req_reg = param[i];
    }
    CD_com = com;
    *g_cd_param_fifo = com;
    if (async != 0) {
        return 0;
    }

    set_alarm(D_8001626C);

    while (Intr.sync == 0) {
        if (get_alarm()) {
            return -1;
        }
        if (CheckCallback()) {
            callback();
        }
    }

    _memcpy(result, &D_800F19A0, 8);
    return -(Intr.sync == 5);
}
