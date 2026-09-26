extern s32 CdSync(s32, u8 *);
extern void CdControl(s32, s32, s32);
extern u8 g_cd_result;
extern void func_80036140(void);
void func_80036940(void) {
    u8 param[4];

    if (D_80101E58.rec.unk02 >= 0x10) {
        func_80036140();
        return;
    }
    switch (D_80101E58.rec.unk02) {
    case 0:
        break;
    case 2:
        if (D_80101E58.rec.unk08 != 0) {
            D_80101E58.rec.unk02 = 0;
            break;
        }
        param[0] = 0xA0;
        CdControlF(0xE, (s32)param);
        D_80101E58.rec.unk06 = 0;
        D_80101E58.rec.unk38 = 0;
        D_80101E58.rec.unk02 = 3;
        break;
    case 3: {
        s32 ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            D_80101E58.rec.unk02 = 4;
            D_80101E58.rec.unk2C = 0;
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 9;
        } else if (++D_80101E58.rec.unk38 > 0x3C) {
            D_80101E58.rec.unk02 = 0xA;
        }
        break;
    }
    case 4:
        if (++D_80101E58.rec.unk2C >= 3) {
            D_80101E58.rec.dest_buffer = D_80101E58.rec.unk1C;
            D_80101E58.rec.sectors_remaining = D_80101E58.rec.unk18;
            g_cdread_expected_pos = CdPosToInt((s32)&D_80101E58.rec.pair);
            CdControl(2, (s32)&D_80101E58.rec.pair, 0);
            D_80101E58.rec.unk38 = 0;
            D_80101E58.rec.unk02 = 5;
        }
        break;
    case 5: {
        s32 ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            D_80101E58.rec.unk38 = 0;
            CdReadyCallback((s32)cdrom_ReadyCallback);
            CdControlF(6, (s32)&D_80101E58.rec.pair);
            D_80101E58.rec.unk02 = 6;
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 9;
        } else if (++D_80101E58.rec.unk38 > 0x3C) {
            D_80101E58.rec.unk02 = 0xA;
        }
        break;
    }
    case 6: {
        s32 ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            if (D_80101E58.rec.sectors_remaining == 0) {
                D_80101E58.rec.unk02 = 8;
            } else if (D_80101E58.rec.sectors_remaining < 0) {
                D_80101E58.rec.unk02 = 9;
            } else if (++D_80101E58.rec.unk38 > 0x3C) {
                CdReadyCallback(0);
                D_80101E58.rec.unk02 = 0xA;
            }
        } else if (ret == 5) {
            CdReadyCallback(0);
            D_80101E58.rec.unk02 = 9;
        } else if (++D_80101E58.rec.unk38 > 0x3C) {
            CdReadyCallback(0);
            D_80101E58.rec.unk02 = 0xA;
        }
        break;
    }
    case 8:
        D_80101E58.rec.unk02 = 0;
        break;
    case 9:
        if (g_cd_result & 0x10) {
            D_80101E58.rec.unk02 = 0xA;
        } else {
            D_80101E58.rec.unk02 = 0xC;
        }
        break;
    case 0xA:
        CdControlF(1, 0);
        D_80101E58.rec.unk02 = 0xB;
        D_80101E58.unk04 = 0;
        break;
    case 0xB: {
        s32 ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            if (g_cd_result & 0x10) {
                D_80101E58.rec.unk02 = 0xA;
            } else {
                D_80101E58.rec.unk02 = 0xC;
            }
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 0xA;
        } else if (++D_80101E58.unk04 > 0xA) {
            CdFlush();
            D_80101E58.rec.unk02 = 0xA;
        }
        break;
    }
    case 0xC:
        CdControlF(0x13, 0);
        D_80101E58.rec.unk02 = 0xD;
        D_80101E58.unk04 = 0;
        break;
    case 0xD: {
        s32 ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            D_80101E58.rec.unk02 = 2;
            VSync(4);
            VSync(4);
            VSync(4);
            VSync(4);
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 9;
        } else if (++D_80101E58.unk04 > 0x1E) {
            CdFlush();
            D_80101E58.rec.unk02 = 0xA;
        }
        break;
    }
    }
}
