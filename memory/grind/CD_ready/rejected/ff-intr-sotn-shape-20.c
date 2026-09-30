s32 CD_ready(s32 a0, u8 *a1)
{
    s32 c;
    s32 ready;
    s32 status;
    u8 saved;

    D_800F19B8 = VSync(-1) + 0x3C0;
    Alarm_plus_0x4 = 0;
    Alarm_plus_0x8 = &D_80016248;
    while (1) {
        if (D_800F19B8 < VSync(-1) || Alarm_plus_0x4++ > 0x3C0000) {
            puts(&D_800161B8);
            printf(&D_800161C8, Alarm_plus_0x8, CD_comstr[CD_com], CD_intstr[Intr.sync], CD_intstr[Intr.ready]);
            CD_flush();
            return -1;
        }
        if (CheckCallback()) {
            saved = *D_800A147C & 3;
            while (1) {
                status = getintr();
                if (status == 0) break;
                if ((status & 4) && CD_cbready != 0) {
                    ((void (*)(u8, void *))CD_cbready)(Intr.ready, &Result_plus_0x8);
                }
                if ((status & 2) && CD_cbsync != 0) {
                    ((void (*)(u8, void *))CD_cbsync)(Intr.sync, &Result);
                }
            }
            *D_800A147C = saved;
        }
        c = Intr.c;
        if (c != 0) {
            Intr.c = 0;
            _memcpy(a1, &Result_plus_0x10, 8);
            return c;
        }
        ready = Intr.ready;
        if (ready != 0) {
            Intr.ready = 0;
            _memcpy(a1, &Result_plus_0x8, 8);
            return ready;
        }
        if (a0 != 0) {
            return 0;
        }
    }
}
