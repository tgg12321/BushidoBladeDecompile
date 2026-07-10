/* CD_cw (tslTm2LoadImage, LIBCD BIOS v1.86) — BANKED session 12 (2026-07-10)
 * Floor: 57/263 differing words (link_sim vs EXE, region 0x800812FC+0x41C,
 * prover tmp/closer/cdcw_prove.sh — sed the func/addr/size into
 * triple_prove.sh clone). HEAD splice distance was 262.
 * Remaining clusters + named mechanisms: see phase3-progress.md session 12.
 */
typedef struct {
    u8 sync;   /* +0x0 = 0x800A1494 (g_cd_status_a) */
    u8 ready;  /* +0x1 (g_cd_status_b) */
    u8 c;      /* +0x2 (g_cd_status_c) */
} CdIntr;

s32 tslTm2LoadImage(s32 a0, void *a1, void *a2, s32 a3) {
    extern s32 D_800A11C0;         /* CD_debug */
    extern void D_80016254;        /* "%s...
" */
    extern void D_8001625C;        /* "%s: no param
" */
    extern void D_8001626C;        /* "CD_cw" (alarm name) */
    extern s32 D_800A13FC[];       /* per-com needs-param table */
    extern s32 D_800A12FC[];       /* per-com intr table (+0x40: param count) */
    extern u8 D_800A11D0[];        /* CD_pos */
    extern u8 D_800A11D4;          /* CD_mode */
    extern volatile u8 *D_800A1480;  /* CD register 1 ptr (MMIO pointee) */
    extern volatile u8 *D_800A1484;  /* CD register 2 ptr (MMIO pointee) */
    extern void debug_printf();
    /* Intr status block (IRQ-mutated Sony module state; Sony declares the
       Intr struct volatile — bios.c). Block-scope view; the sibling
       per-byte decls (g_cd_status_a/b/c) stay for the other functions. */
    extern volatile CdIntr g_cd_status_a;
    s32 i;
    s32 v0;
    s32 cnt;
    s32 status;
    u8 saved;
    u8 *src;
    u8 *dst;
    u8 b;
    s32 *tbl;
    volatile u8 *intr;
    volatile u8 *intr1;

    if (!(D_800A11C0 < 2)) {
        debug_printf(&D_80016254, D_800A11DC[a0 & 0xFF]);
    }
    if (D_800A13FC[a0 & 0xFF] != 0 && a1 == 0) {
        if (D_800A11C0 > 0) {
            debug_printf(&D_8001625C, D_800A11DC[a0 & 0xFF]);
        }
        return -2;
    }
    cpu_side_move_dir_4(0, 0);
    if ((a0 & 0xFF) == 2) {
        for (i = 0; i < 4; i++) {
            D_800A11D0[i] = ((u8 *)a1)[i];
        }
    }
    if ((a0 & 0xFF) == 0xE) {
        D_800A11D4 = *(u8 *)a1;
    }
    g_cd_status_a.sync = 0;
    if (D_800A12FC[a0 & 0xFF]) {
        g_cd_status_a.ready = 0;
    }
    *D_800A147C = 0;
    for (i = 0; i < D_800A12FC[(a0 & 0xFF) + 0x40]; i++) {
        *D_800A1484 = ((u8 *)a1)[i];
    }
    D_800A11D5 = a0;
    *D_800A1480 = a0;
    if (a3 == 0) {
    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    D_800F19BC = 0;
    D_800F19C0 = &D_8001626C;
    if (g_cd_status_a.sync != 0) {
        goto cw_done;
    }
    tbl = D_800A125C;
    intr = (volatile u8 *)&g_cd_status_a;
    intr1 = &g_cd_status_a.ready;
loop:
    v0 = sys_VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto alarm_ok;
    }
do_timeout:
    tslTm2LoadImage_2(&D_800161B8);
    {
        s32 arg4, arg5;
        arg4 = tbl[intr[0]];
        arg5 = tbl[intr[1]];
        debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4,
                     arg5);
    }
    cdrom_ClearIrq();
    v0 = -1;
    goto check;
alarm_ok:
    v0 = 0;
check:
    if (v0 != 0) {
        return -1;
    }
    if (sys_GetVblankCount() != 0) {
        saved = (*D_800A147C) & 3;
    poll:
        status = func_80080828();
        if (status != 0) {
            if (status & 4) {
                if (D_800A11B8 != 0) {
                    ((void (*)(u8, void *))D_800A11B8)(*intr1,
                                                       &D_800F19A8);
                }
            }
            if (status & 2) {
                if (D_800A11B4 != 0) {
                    ((void (*)(u8, void *))D_800A11B4)(*intr,
                                                       &D_800F19A0);
                }
            }
            goto poll;
        }
        *D_800A147C = saved;
    }
    if (*intr == 0) {
        goto loop;
    }
cw_done:
    dst = a2;
    src = (u8 *)&D_800F19A0;
    cnt = 7;
    if (a2 != 0) {
        do {
            b = *src;
            src++;
            cnt--;
            *dst = b;
            dst++;
        } while (cnt != -1);
    }
    return -(g_cd_status_a.sync == 5);
    }
    return 0;
}
