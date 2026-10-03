static void cb_read(u8 intr, u8 *result) {
    extern volatile s32 D_800A14E0;
    s32 pos[3];

    D_800A1504 = result;
    if (intr == 1) {
        if (D_800A14D0.cnt > 0) {
            if (D_800A14E0 == 0x200) {
                if (D_800A14D0.tslmode & 1) {
                    CdDataCallback(0);
                    CdGetSector2((s32)pos, 3);
                    CdDataSync(0);
                    CdDataCallback((s32)&cb_data);
                } else {
                    CdGetSector((s32)pos, 3);
                }
                if (CdPosToInt((u8 *)pos) != D_800A14D0.pos) {
                    puts(&D_800162D4);
                    D_800A14D0.cnt = -1;
                }
            }
            if (D_800A14D0.tslmode & 1) {
                CdGetSector2(D_800A14D0.p, D_800A14D0.size);
            } else {
                CdGetSector(D_800A14D0.p, D_800A14D0.size);
                D_800A14D0.p += D_800A14D0.size * 4;
                D_800A14D0.cnt--;
                D_800A14D0.pos++;
            }
        }
    } else {
        D_800A14D0.cnt = -1;
    }
    D_800A14D0.t2 = VSync(-1);
    if (D_800A14D0.cnt < 0) {
        cd_read_retry(1);
    }
    if (VSync(-1) > D_800A14D0.t1 + 1200) {
        D_800A14D0.cnt = -1;
    }
    if (D_800A14D0.cnt != 0 && VSync(-1) <= D_800A14D0.t1 + 1200) {
        return;
    }
    CdSyncCallback(D_800A14D0.cbsync);
    CdReadyCallback(D_800A14D0.cbready);
    if (D_800A14D0.tslmode & 1) {
        CdDataCallback(D_800A14D0.cbdata);
    }
    CdControlF(9, 0);
    if (g_CdReadCallback_func != 0) {
        ((void (*)(u8, u8 *))g_CdReadCallback_func)(D_800A14D0.cnt == 0 ? 2 : 5, result);
    }
}
