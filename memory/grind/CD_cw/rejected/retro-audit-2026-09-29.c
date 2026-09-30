/* RETRO-AUDIT 2026-09-29 FAIL -- CD_cw reopened (Q37 class C, owner rulings 803d0fea1).
 * Landed on main in bba90442b (src/system.c); this is that landed text, verbatim from main
 * as of the reopen, banked before the body went back to INCLUDE_ASM.
 * FAIL: Alarm_t aggregate merge incomplete and TU-local (old per-word D_800F19B8..C0 / Alarm_plus_0x4/0x8 externs still used in the same TU, prongs (c)/(d)); Intr is a second C object over the bytes the TU still reaches as g_cd_status_a/b/c.
 * Detail: tmp/audit-2026-09-29/review/batch_01.md (gitignored), tmp/audit-2026-09-29/SUMMARY.md;
 * rulings: docs/grind/owner-rulings-2026-09-26.md Q37/Q38 (803d0fea1).
 * Reopen notes: Removed with the body: CD_cw's private static inline helpers set_alarm/get_alarm/callback. LEFT IN PLACE (shared or merge declarations, per the reopen brief): typedef CD_intr + `static volatile CD_intr Intr = {0};` (still used by getintr), typedef Alarm_t + `extern Alarm_t Alarm;` (now UNUSED), the D_800A12FC/D_800A13FC/D_80016254/D_8001625C/D_8001626C externs (now unused), the SOTN `CD_cw(u8, u8 *, u8 *, s32)` prototypes, the asm/data split + bb2.ld system.o(.data) placement, and named_syms.txt `Alarm = 0x800F19B8`.
 * DO NOT resubmit this body as-is. */

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
            ((void (*)(u8, void *))CD_cbready)(Intr.ready, &Result_plus_0x8);
        }
        if ((status & 2) && CD_cbsync != 0) {
            ((void (*)(u8, void *))CD_cbsync)(Intr.sync, &Result);
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

    _memcpy(result, &Result, 8);
    return -(Intr.sync == 5);
}
