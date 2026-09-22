typedef struct {
    u8 sync;
    u8 ready;
    u8 c;
} CD_intr;
#define Intr (*(volatile CD_intr *)&g_cd_status_a)
extern s32 D_800A12FC[];
extern s32 D_800A13FC[];
extern volatile u8 *g_cd_req_reg;
extern volatile u8 *g_cd_param_fifo;
extern char D_8001626C[];
extern char D_80016254[];
extern char D_8001625C[];

static inline void set_alarm(char *name)
{
    D_800F19B8 = VSync(-1) + 0x3C0;
    D_800F19BC = 0;
    D_800F19C0 = name;
}

static inline s32 get_alarm(void)
{
    if (D_800F19B8 < VSync(-1) || D_800F19BC++ > 0x3C0000) {
        puts(&D_800161B8);
        printf(&D_800161C8, D_800F19C0, CD_comstr[CD_com],
               CD_intstr[Intr.sync], CD_intstr[(&g_cd_status_a)[1]]);
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
            ((void (*)(u8, void *))CD_cbready)((&g_cd_status_a)[1], &D_800F19A8);
        }
        if ((status & 2) && CD_cbsync != 0) {
            ((void (*)(u8, void *))CD_cbsync)(Intr.sync, &D_800F19A0);
        }
    }
    *D_800A147C = saved;
}

s32 CD_cw(com, param, result, async)
u8 com;
void *param;
void *result;
s32 async;
{
    s32 i;
    u8 *src;
    u8 *dst;
    u8 b;

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
            (&CD_pos)[i] = ((u8 *)param)[i];
        }
    }
    if (com == 0xE) {
        CD_mode = ((u8 *)param)[0];
    }
    Intr.sync = 0;
    if (D_800A12FC[com]) {
        Intr.ready = 0;
    }
    *D_800A147C = 0;
    for (i = 0; i < D_800A12FC[com + 0x40]; i++) {
        *g_cd_req_reg = ((u8 *)param)[i];
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

    dst = result;
    src = (u8 *)&D_800F19A0;
    if (dst != 0) {
        i = 7;
        do {
            b = *src;
            src++;
            i--;
            *dst = b;
            dst++;
        } while (i != -1);
    }
    return -(Intr.sync == 5);
}
