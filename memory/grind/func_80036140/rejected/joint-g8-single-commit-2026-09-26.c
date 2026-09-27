extern void CdMix(CdlATV *);
extern s16 D_800A3854;
extern s16 D_800A3840;
extern void cdrom_SetMix(s32, s32, s32, s32);
extern s32 CdSync(s32, u8 *);
extern s32 CdReady(s32, u8 *);
extern u8 g_cd_result;
extern u8 g_cd_result_plus_0x3;
extern u8 g_cd_result_plus_0x5;
void func_80036140(void) {
    CdlATV atv;

    if (D_800A3854 > 0) {
        atv.val0 = (D_800A36B8.val0 * D_800A3840 + g_cd_atv.val0 * (D_800A3854 - D_800A3840)) / D_800A3854;
        atv.val1 = (D_800A36B8.val1 * D_800A3840 + g_cd_atv.val1 * (D_800A3854 - D_800A3840)) / D_800A3854;
        atv.val2 = (D_800A36B8.val2 * D_800A3840 + g_cd_atv.val2 * (D_800A3854 - D_800A3840)) / D_800A3854;
        atv.val3 = (D_800A36B8.val3 * D_800A3840 + g_cd_atv.val3 * (D_800A3854 - D_800A3840)) / D_800A3854;
        CdMix(&atv);
        if (++D_800A3840 >= D_800A3854) {
            D_800A3854 = 0;
            g_cd_atv = D_800A36B8;
        }
    }

    switch (D_80101E58.rec.unk02) {
    case 0x10:
        cdrom_SetMix(0, 0, 0, 0);
        CdControlF(0xE, (s32)&D_80101E58.rec.unk30);
        D_80101E58.rec.unk28 = 0;
        D_80101E58.rec.unk2C = 0;
        D_80101E58.rec.unk44 = 0;
        D_80101E58.rec.unk02 = 0x11;
        break;
    case 0x11: {
        s32 ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            CdControlF(2, (s32)&D_80101E58.rec.pair);
            D_80101E58.rec.unk02 = 0x12;
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 0x17;
        }
        break;
    }
    case 0x12:
        if (++D_80101E58.rec.unk2C >= 3) {
            s32 ret = CdSync(1, &g_cd_result);
            if (ret == 2) {
                CdControlF(0x16, 0);
                D_80101E58.rec.unk02 = 0x13;
            } else if (ret == 5) {
                D_80101E58.rec.unk02 = 0x17;
            }
        }
        break;
    case 0x13: {
        s32 ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            if (D_80101E58.rec.unk04 == 0) {
                CdControlF(D_80101E58.rec.unk34 ? 0x1B : 3, 0);
                D_80101E58.rec.unk02 = 0x14;
            }
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 0x17;
        }
        break;
    }
    case 0x14: {
        s32 ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            D_80101E58.rec.unk3C = 0;
            D_80101E58.rec.unk02 = 0x15;
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 0x17;
        }
        break;
    }
    case 0x15:
        if (++D_80101E58.rec.unk28 == 2) {
            cdrom_SetMix(0xFF, 0, 0xFF, 0);
        }
        if (D_80101E58.rec.unk08 != 0 && D_80101E58.rec.unk28 >= 5) {
            D_80101E58.rec.unk02 = 0x1C;
            break;
        }
        D_80101E58.rec.unk02 = 0x16;
        if (D_80101E58.rec.unk34 != 0) {
            CdControlF(0x11, (s32)&g_cd_result);
            D_80101E58.rec.unk3A = 0;
            break;
        }
        CdControlF(1, 0);
        {
            s32 ret = CdReady(1, &g_cd_result);
            if (ret == 1) {
                if (!(g_cd_result_plus_0x4 & 0x80)) {
                    D_80101E58.rec.unk3C = 0;
                    if (CdPosToInt((s32)&g_cd_result_plus_0x3) >= D_80101E58.rec.unk14) {
                        cdrom_SetMix(0, 0, 0, 0);
                        D_80101E58.rec.unk02 = D_80101E58.rec.unk0A ? 0x10 : 0x1C;
                    }
                }
            } else if (ret == 5) {
                D_80101E58.rec.unk02 = 0x17;
            }
        }
        if (D_80101E58.rec.unk3C++ > 0x3C) {
            D_80101E58.rec.unk02 = 0x17;
        }
        break;
    case 0x16:
        if (D_80101E58.rec.unk44 != 0) {
            D_80101E58.rec.unk44 -= 4;
            if (D_80101E58.rec.unk44 <= 0) {
                cdrom_SetMix(0, 0, 0, 0);
                D_80101E58.rec.unk02 = D_80101E58.rec.unk0A ? 0x10 : 0x1C;
                break;
            }
        }
        {
            s32 ret = CdSync(1, &g_cd_result);
            if (ret == 2) {
                D_80101E58.rec.unk02 = 0x15;
                if (D_80101E58.rec.unk34 != 0) {
                    s32 pos = CdPosToInt((s32)&g_cd_result_plus_0x5);
                    if (pos >= D_80101E58.rec.unk14) {
                        cdrom_SetMix(0, 0, 0, 0);
                        D_80101E58.rec.unk02 = D_80101E58.rec.unk0A ? 0x10 : 0x1C;
                    } else if (pos >= D_80101E58.rec.unk14 - 0x96) {
                        if (D_80101E58.rec.unk44 == 0) {
                            D_80101E58.rec.unk44 = D_80101E58.rec.unk14 - pos;
                        }
                    }
                }
            } else if (ret == 5) {
                D_80101E58.rec.unk02 = 0x17;
            } else if (D_80101E58.rec.unk34 != 0) {
                if (++D_80101E58.rec.unk3A >= 0x1F) {
                    CdFlush();
                    D_80101E58.rec.unk02 = 0x15;
                }
            }
        }
        break;
    case 0x1C:
        cdrom_SetMix(0, 0, 0, 0);
        CdControlF(9, 0);
        D_80101E58.rec.unk02 = 0x1D;
        break;
    case 0x1D: {
        s32 ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            D_80101E58.rec.unk28 = 0;
            D_80101E58.rec.unk02 = 0x1E;
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 0x17;
        }
        break;
    }
    case 0x1E:
        if (++D_80101E58.rec.unk28 >= 5) {
            D_80101E58.rec.unk02 = 0;
        }
        break;
    case 0x17:
        cdrom_SetMix(0, 0, 0, 0);
        if (g_cd_result & 0x10) {
            D_80101E58.rec.unk02 = 0x18;
        } else {
            D_80101E58.rec.unk02 = 0x1A;
        }
        break;
    case 0x18:
        CdControlF(1, 0);
        D_80101E58.rec.unk02 = 0x19;
        break;
    case 0x19: {
        s32 ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            if (g_cd_result & 0x10) {
                D_80101E58.rec.unk02 = 0x18;
            } else {
                D_80101E58.rec.unk02 = 0x1A;
            }
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 0x18;
        }
        break;
    }
    case 0x1A:
        CdControlF(0x13, 0);
        D_80101E58.rec.unk02 = 0x1B;
        break;
    case 0x1B: {
        s32 ret = CdSync(1, &g_cd_result);
        if (ret == 2) {
            D_80101E58.rec.unk02 = D_80101E58.rec.unk08 ? 0 : 0x10;
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 0x17;
        }
        break;
    }
    }
}
