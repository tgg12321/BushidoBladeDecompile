/* s58 (2026-09-04, object-model).  THE 2/91 FLOOR BODY UNDER SONY'S OBJECT MODEL,
 * WITH THE `void **pp` POINTER-ALIAS FAKE REMOVED.  Link-level bytes are IDENTICAL
 * to memory/grind/CD_datasync/candidate.c (tmp/grind/CD_datasync/s58/diff_base_G.txt,
 * norm_G2_nopp.txt vs norm_base.txt: the ONLY object-level differences are the five
 * R_MIPS_LO16 addends D_800F19B8+4 / D_800F19B8+8, which the linker resolves to the
 * same addresses as D_800F19BC / D_800F19C0).  The engine sandbox reports 7 for this
 * body because engine/score.py does not mask named-symbol reloc addends (memory
 * sandbox-lo16-text-addend-false-distance) - 2 real points + 5 false points.
 *
 * Declaration surface (header-level; the //REPL1 / //DROP directives below are
 * replayed by tmp/grind/CD_datasync/s58/apply.py against a src/system.c whose
 * CD_datasync is INCLUDE_ASM):
 *   typedef struct { s32 timeout; s32 count; char *func; } CD_alarm;  extern CD_alarm D_800F19B8;
 *   typedef struct { u8 sync; u8 ready; u8 x2; u8 x3; } CD_intr;      extern CD_intr D_800A1494;
 *   extern char *D_800A11DC[]; extern char *D_800A125C[];
 *   extern char g_str_cd_timeout[]; extern char D_800161C8[]; extern char D_800162C0[];
 * and the per-word externs D_800F19BC / D_800F19C0 removed from C (prong (c) as
 * amended 2026-09-03: their undefined_syms_auto.txt rows stay, suffixed
 * `alias of D_800F19B8+N; retire with CD_cw`, while CD_cw/CD_sync/CD_ready .s bodies
 * reference them).  Evidence for the object model: Sony BIOS.OBJ XDEF `Alarm` in
 * .bss+0x18 == 0x800F19B8 (memory/closer/libcd-groundtruth.md:42), i.e. ONE object.
 */
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
    /* FAKE: do{}while(0) - loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of.
     * mechanism: flow.c loop_depth ref-weighting -> global.c allocno_compare.
     * lever-exhaustion: memory/grind/CD_datasync/hypotheses.md (s9 H37/H38). */
    do {
        char *arg5;
        s32 i5;
        s32 t0;
        puts(g_str_cd_timeout);
        i5 = idx_1494->ready;
        t0 = idx_1494->sync;
        t0 *= 4;
        arg5 = tbl_125c[i5];
        t0 = (s32)((u8 *)tbl_125c + t0);
        printf(D_800161C8, D_800F19B8.func, tbl_11dc[D_800A11D5], *(char **)t0, arg5);
        CD_flush();
    } while (0);
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
