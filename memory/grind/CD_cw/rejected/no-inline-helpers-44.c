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
            CD_pos[i] = param[i];
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
    /* FAKE: the parameter count D_800A13FC[com] read through the preceding
     * table's base, verbatim SOTN (Q50/Q55, Q53); mechanism: cse keeps
     * &D_800A12FC from the ready-flag read above live and forms the count's
     * address as that base + 0x100 (asm/funcs/CD_cw.s: `addiu $v0, $v1, 0x100`
     * at 0x80081460); lever-exhaustion: D_800A13FC[com] = 31/263,
     * memory/grind/CD_cw/evidence.md */
    for (i = 0; i < D_800A12FC[com + 0x40]; i++) { /* SOTN: src/main/psxsdk/libcd/bios.c:314 @aa53500 */
        *g_cd_req_reg = param[i];
    }
    CD_com = com;
    *g_cd_param_fifo = com;
    if (async != 0) {
        return 0;
    }

    Alarm.time = VSync(-1) + 0x3C0;
    Alarm.count = 0;
    Alarm.name = D_8001626C;

    while (Intr.sync == 0) {
        if (Alarm.time < VSync(-1) || Alarm.count++ > 0x3C0000) {
            puts(&D_800161B8);
            printf(&D_800161C8, Alarm.name, CD_comstr[CD_com],
                   CD_intstr[Intr.sync], CD_intstr[Intr.ready]);
            CD_flush();
            return -1;
        }
        if (CheckCallback()) {
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
    }

    if (result != 0) {
        u8 *src = (u8 *)&Result;
        u32 n = 8;
        while (n--) {
            *result++ = *src++;
        }
    }
    return -(Intr.sync == 5);
}
