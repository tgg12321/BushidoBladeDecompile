/* s58: Sony struct model with the do{}while(0) FAKE ablated: sandbox 20 / 91 (= 15 real + 5 reloc-addend false points).
 * Residual = the s0/s1/s2 3-cycle callee-save rotation + the two window points, i.e. s56's zero-FAKE chassis result.
 * The do{}while(0) wrap is load-bearing under the struct model exactly as under the scalar model. */
//REPL:extern s32 g_str_cd_timeout; => extern char g_str_cd_timeout[];
//REPL:extern s32 D_800161C8; => extern char D_800161C8[];
//REPL:extern void D_800162C0; => extern char D_800162C0[];
//REPL:extern s32 D_800A11DC[]; => extern char *D_800A11DC[];
//REPL:extern s32 D_800A125C[]; => extern char *D_800A125C[];
//REPL1:extern u8 D_800A1494; => typedef struct { u8 sync; u8 ready; u8 x2; u8 x3; } CD_intr; extern CD_intr D_800A1494;
//DROP:extern u8 D_800A1494;
//REPL1:extern s32 D_800F19B8; => typedef struct { s32 timeout; s32 count; char *func; } CD_alarm; extern CD_alarm D_800F19B8;
//DROP:extern s32 D_800F19B8;
//DROP:extern s32 D_800F19BC;
//DROP:extern void *D_800F19C0;
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    char **tbl_11dc;
    CD_intr *idx_1494;
    char **tbl_125c;

    D_800F19B8.timeout = VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19B8.count = 0;
    D_800F19B8.func = D_800162C0;

loop:
    v0 = VSync(-1);
    if (D_800F19B8.timeout < v0) {
        goto do_timeout;
    }
    cnt = D_800F19B8.count;
    D_800F19B8.count = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    {
        char *arg5;
        s32 i5;
        s32 t0;
        char **pp = &D_800F19B8.func; /* FAKE: pointer-alias staging the D_800F19C0 load early; mechanism: local-alloc.c update_equiv_regs refs-2 sink defeat; lever-exhaustion: memory/grind/CD_datasync/hypotheses.md s50 */
        puts(g_str_cd_timeout);
        i5 = idx_1494->ready;
        t0 = idx_1494->sync;
        t0 *= 4;
        arg5 = tbl_125c[i5];
        t0 = (s32)((u8 *)tbl_125c + t0);
        printf(D_800161C8, *pp, tbl_11dc[D_800A11D5], *(char **)t0, arg5);
        CD_flush();
    }
    v0 = -1;
    goto check;

success:
    v0 = 0;

check:
    if (v0 != 0) {
        return -1;
    }
    if (*D_800A14C0 & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
