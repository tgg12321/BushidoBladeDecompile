extern s32 CdSync(s32, u8 *);
extern s32 CdReady(s32, u8 *);
extern u8 g_cd_result;
extern u8 g_cd_result_plus_0x3;
extern u8 g_cd_result_plus_0x5;
void func_80036140(void) {
    u8 atv[4];
    s32 ret;

    if (D_800A3854 > 0) {
        atv[0] = (D_800A36B8 * D_800A3840 + g_cd_atv * (D_800A3854 - D_800A3840)) / D_800A3854;
        atv[1] = (D_800A36B9 * D_800A3840 + g_cd_atv_plus_0x1 * (D_800A3854 - D_800A3840)) / D_800A3854;
        atv[2] = (D_800A36BA * D_800A3840 + g_cd_atv_plus_0x2 * (D_800A3854 - D_800A3840)) / D_800A3854;
        atv[3] = (D_800A36BB * D_800A3840 + g_cd_atv_plus_0x3 * (D_800A3854 - D_800A3840)) / D_800A3854;
        CdMix(atv);
        if (++D_800A3840 >= D_800A3854) {
            D_800A3854 = 0;
            g_cd_atv = D_800A36B8;
            g_cd_atv_plus_0x1 = D_800A36B9;
            g_cd_atv_plus_0x2 = D_800A36BA;
            g_cd_atv_plus_0x3 = D_800A36BB;
        }
    }

    switch (D_80101E60.unk02) {
    case 0x10:
        cdrom_SetMix(0, 0, 0, 0);
        CdControlF(0xE, (s32)&D_80101E90);
        D_80101E88 = 0;
        D_80101E8C = 0;
        D_80101EA4 = 0;
        D_80101E60.unk02 = 0x11;
        break;
    case 0x11:
        ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            CdControlF(2, (s32)&D_80101E60.pair);
            D_80101E60.unk02 = 0x12;
        } else if (ret == 5) {
            D_80101E60.unk02 = 0x17;
        }
        break;
    case 0x12:
        if (++D_80101E8C >= 3) {
            ret = CdSync(1, &g_cd_result);
            if (ret == 2) {
                CdControlF(0x16, 0);
                D_80101E60.unk02 = 0x13;
            } else if (ret == 5) {
                D_80101E60.unk02 = 0x17;
            }
        }
        break;
    case 0x13:
        ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            if (D_80101E60.unk04 == 0) {
                CdControlF(D_80101E94 ? 0x1B : 3, 0);
                D_80101E60.unk02 = 0x14;
            }
        } else if (ret == 5) {
            D_80101E60.unk02 = 0x17;
        }
        break;
    case 0x14:
        ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            D_80101E9C = 0;
            D_80101E60.unk02 = 0x15;
        } else if (ret == 5) {
            D_80101E60.unk02 = 0x17;
        }
        break;
    case 0x15:
        if (++D_80101E88 == 2) {
            cdrom_SetMix(0xFF, 0, 0xFF, 0);
        }
        if (D_80101E60.unk08 != 0 && D_80101E88 >= 5) {
            D_80101E60.unk02 = 0x1C;
            break;
        }
        D_80101E60.unk02 = 0x16;
        if (D_80101E94 != 0) {
            CdControlF(0x11, (s32)&g_cd_result);
            D_80101E9A = 0;
            break;
        }
        CdControlF(1, 0);
        ret = CdReady(1, &g_cd_result);
        if (ret == 1) {
            if (!(g_cd_result_plus_0x4 & 0x80)) {
                D_80101E9C = 0;
                if (CdPosToInt((s32)&g_cd_result_plus_0x3) >= D_80101E60.unk14) {
                    cdrom_SetMix(0, 0, 0, 0);
                    D_80101E60.unk02 = D_80101E60.unk0A ? 0x10 : 0x1C;
                }
            }
        } else if (ret == 5) {
            D_80101E60.unk02 = 0x17;
        }
        if (D_80101E9C++ > 0x3C) {
            D_80101E60.unk02 = 0x17;
        }
        break;
    case 0x16:
        if (D_80101EA4 != 0) {
            D_80101EA4 -= 4;
            if (D_80101EA4 <= 0) {
                cdrom_SetMix(0, 0, 0, 0);
                D_80101E60.unk02 = D_80101E60.unk0A ? 0x10 : 0x1C;
                break;
            }
        }
        ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            D_80101E60.unk02 = 0x15;
            if (D_80101E94 != 0) {
                s32 pos = CdPosToInt((s32)&g_cd_result_plus_0x5);
                if (pos >= D_80101E60.unk14) {
                    cdrom_SetMix(0, 0, 0, 0);
                    D_80101E60.unk02 = D_80101E60.unk0A ? 0x10 : 0x1C;
                } else if (pos >= D_80101E60.unk14 - 0x96) {
                    if (D_80101EA4 == 0) {
                        D_80101EA4 = D_80101E60.unk14 - pos;
                    }
                }
            }
        } else if (ret == 5) {
            D_80101E60.unk02 = 0x17;
        } else if (D_80101E94 != 0) {
            if (++D_80101E9A >= 0x1F) {
                CdFlush();
                D_80101E60.unk02 = 0x15;
            }
        }
        break;
    case 0x1C:
        cdrom_SetMix(0, 0, 0, 0);
        CdControlF(9, 0);
        D_80101E60.unk02 = 0x1D;
        break;
    case 0x1D:
        ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            D_80101E88 = 0;
            D_80101E60.unk02 = 0x1E;
        } else if (ret == 5) {
            D_80101E60.unk02 = 0x17;
        }
        break;
    case 0x1E:
        if (++D_80101E88 >= 5) {
            D_80101E60.unk02 = 0;
        }
        break;
    case 0x17:
        cdrom_SetMix(0, 0, 0, 0);
        if (g_cd_result & 0x10) {
            D_80101E60.unk02 = 0x18;
        } else {
            D_80101E60.unk02 = 0x1A;
        }
        break;
    case 0x18:
        CdControlF(1, 0);
        D_80101E60.unk02 = 0x19;
        break;
    case 0x19:
        ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            if (g_cd_result & 0x10) {
                D_80101E60.unk02 = 0x18;
            } else {
                D_80101E60.unk02 = 0x1A;
            }
        } else if (ret == 5) {
            D_80101E60.unk02 = 0x18;
        }
        break;
    case 0x1A:
        CdControlF(0x13, 0);
        D_80101E60.unk02 = 0x1B;
        break;
    case 0x1B:
        ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            D_80101E60.unk02 = D_80101E60.unk08 ? 0 : 0x10;
        } else if (ret == 5) {
            D_80101E60.unk02 = 0x17;
        }
        break;
    }
}
