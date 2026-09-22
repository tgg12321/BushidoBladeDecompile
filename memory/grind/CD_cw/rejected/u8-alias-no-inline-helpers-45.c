extern volatile u8 g_cd_status_b;
extern s32 D_800A12FC[2][64];
extern volatile u8 *g_cd_req_reg;
extern volatile u8 *g_cd_param_fifo;
extern void D_8001626C;
extern char D_80016254[];
extern char D_8001625C[];

s32 CD_cw(com, param, result, async)
u8 com;
void *param;
void *result;
s32 async;
{
    u8 *prm = param;
    u8 *res = result;
    s32 i;
    u8 *src;
    u8 *dst;
    u8 b;
    s32 v0;
    s32 cnt;
    u8 saved;
    s32 status;
    volatile u8 *idx_1494;
    volatile u8 *idx_1495;
    s32 *tbl_125c;

    if (CD_debug >= 2) {
        printf(D_80016254, CD_comstr[com]);
    }
    if (D_800A12FC[1][com] != 0 && param == 0) {
        if (CD_debug > 0) {
            printf(D_8001625C, CD_comstr[com]);
        }
        return -2;
    }
    CD_sync(0, 0);
    if (com == 2) {
        for (i = 0; i < 4; i++) {
            (&CD_pos)[i] = prm[i];
        }
    }
    if (com == 0xE) {
        CD_mode = prm[0];
    }
    g_cd_status_a = 0;
    if (D_800A12FC[0][com] != 0) {
        g_cd_status_b = 0;
    }
    *D_800A147C = 0;
    for (i = 0; i < D_800A12FC[1][com]; i++) {
        *g_cd_req_reg = prm[i];
    }
    CD_com = com;
    *g_cd_param_fifo = com;
    if (async != 0) {
        return 0;
    }
    D_800F19B8 = VSync(-1) + 0x3C0;
    D_800F19BC = 0;
    D_800F19C0 = &D_8001626C;
    idx_1494 = &g_cd_status_a;
    if (*idx_1494 == 0) {
        tbl_125c = CD_intstr;
        idx_1495 = idx_1494 + 1;
        do {
            v0 = VSync(-1);
            if (D_800F19B8 < v0) {
                goto do_timeout;
            }
            cnt = D_800F19BC;
            D_800F19BC = cnt + 1;
            if (!(0x3C0000 < cnt)) {
                goto success;
            }
        do_timeout:
            puts(&D_800161B8);
            printf(&D_800161C8, D_800F19C0, CD_comstr[CD_com],
                   tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]);
            CD_flush();
            v0 = -1;
            goto check;
        success:
            v0 = 0;
        check:
            if (v0 != 0) {
                return -1;
            }
            if (CheckCallback() != 0) {
                saved = *D_800A147C & 3;
                while ((status = getintr()) != 0) {
                    if (status & 4) {
                        if (CD_cbready != 0) {
                            ((void (*)(u8, void *))CD_cbready)(*idx_1495, &D_800F19A8);
                        }
                    }
                    if (status & 2) {
                        if (CD_cbsync != 0) {
                            ((void (*)(u8, void *))CD_cbsync)(*idx_1494, &D_800F19A0);
                        }
                    }
                }
                *D_800A147C = saved;
            }
        } while (*idx_1494 == 0);
    }
    dst = res;
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
    return -(g_cd_status_a == 5);
}
