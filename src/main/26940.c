/* The CD module's two state-machine steppers, func_80036140 and func_80036940.
 * .text 0x80036140 (ROM 0x26940). Start boundary: G8. Compiled -G8 (owner
 * ruling 2026-09-26, Q10): both read g_cd_result (and func_80036140 the
 * ATV/fade state) off $gp. Those globals are tentative definitions (COMMON,
 * zero bytes), as in 26730.c; gp-relative at their base only (owner ruling
 * Q62, 2026-09-30, global COMMON model). */
#include "common.h"
#include "bb2.h"

CdlATV g_cd_atv;
CdlATV D_800A36B8;
u8 g_cd_result[8];

void func_80036140(void) {
    CdlATV atv;

    if (D_800A3854 > 0) {
        atv.val0 = (D_800A36B8.val0 * D_800A3840 +
                    g_cd_atv.val0 * (D_800A3854 - D_800A3840)) /
                   D_800A3854;
        atv.val1 = (D_800A36B8.val1 * D_800A3840 +
                    g_cd_atv.val1 * (D_800A3854 - D_800A3840)) /
                   D_800A3854;
        atv.val2 = (D_800A36B8.val2 * D_800A3840 +
                    g_cd_atv.val2 * (D_800A3854 - D_800A3840)) /
                   D_800A3854;
        atv.val3 = (D_800A36B8.val3 * D_800A3840 +
                    g_cd_atv.val3 * (D_800A3854 - D_800A3840)) /
                   D_800A3854;
        CdMix(&atv);
        if (++D_800A3840 >= D_800A3854) {
            D_800A3854 = 0;
            g_cd_atv = D_800A36B8;
        }
    }

    switch (D_80101E58.rec.unk02) {
    case 0x10:
        cdrom_SetMix(0, 0, 0, 0);
        CdControlF(0xE, &D_80101E58.rec.unk30);
        D_80101E58.rec.unk28 = 0;
        D_80101E58.rec.unk2C = 0;
        D_80101E58.rec.unk44 = 0;
        D_80101E58.rec.unk02 = 0x11;
        break;
    case 0x11: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            CdControlF(2, (u8 *)&D_80101E58.rec.pair);
            D_80101E58.rec.unk02 = 0x12;
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 0x17;
        }
        break;
    }
    case 0x12:
        if (++D_80101E58.rec.unk2C >= 3) {
            s32 ret = CdSync(1, g_cd_result);
            if (ret == 2) {
                CdControlF(0x16, 0);
                D_80101E58.rec.unk02 = 0x13;
            } else if (ret == 5) {
                D_80101E58.rec.unk02 = 0x17;
            }
        }
        break;
    case 0x13: {
        s32 ret = CdSync(1, g_cd_result);
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
        s32 ret = CdSync(1, g_cd_result);
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
            CdControlF(0x11, g_cd_result);
            D_80101E58.rec.unk3A = 0;
            break;
        }
        CdControlF(1, 0);
        {
            s32 ret = CdReady(1, g_cd_result);
            if (ret == 1) {
                if (!(g_cd_result[4] & 0x80)) {
                    D_80101E58.rec.unk3C = 0;
                    if (CdPosToInt((CdlLOC *)&g_cd_result[3]) >=
                        D_80101E58.rec.unk14) {
                        cdrom_SetMix(0, 0, 0, 0);
                        D_80101E58.rec.unk02 =
                            D_80101E58.rec.unk0A ? 0x10 : 0x1C;
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
            s32 ret = CdSync(1, g_cd_result);
            if (ret == 2) {
                D_80101E58.rec.unk02 = 0x15;
                if (D_80101E58.rec.unk34 != 0) {
                    s32 pos = CdPosToInt((CdlLOC *)&g_cd_result[5]);
                    if (pos >= D_80101E58.rec.unk14) {
                        cdrom_SetMix(0, 0, 0, 0);
                        D_80101E58.rec.unk02 =
                            D_80101E58.rec.unk0A ? 0x10 : 0x1C;
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
        s32 ret = CdSync(1, g_cd_result);
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
        if (g_cd_result[0] & 0x10) {
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
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            if (g_cd_result[0] & 0x10) {
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
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            D_80101E58.rec.unk02 = D_80101E58.rec.unk08 ? 0 : 0x10;
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 0x17;
        }
        break;
    }
    }
}

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
        CdControlF(0xE, param);
        D_80101E58.rec.unk06 = 0;
        D_80101E58.rec.unk38 = 0;
        D_80101E58.rec.unk02 = 3;
        break;
    case 3: {
        s32 ret = CdSync(1, g_cd_result);
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
            D_80101E58.rec.expected_pos = CdPosToInt(&D_80101E58.rec.pair.loc);
            CdControl(2, (u8 *)&D_80101E58.rec.pair.loc, 0);
            D_80101E58.rec.unk38 = 0;
            D_80101E58.rec.unk02 = 5;
        }
        break;
    case 5: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            D_80101E58.rec.unk38 = 0;
            CdReadyCallback(cdrom_ReadyCallback);
            CdControlF(6, (u8 *)&D_80101E58.rec.pair);
            D_80101E58.rec.unk02 = 6;
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 9;
        } else if (++D_80101E58.rec.unk38 > 0x3C) {
            D_80101E58.rec.unk02 = 0xA;
        }
        break;
    }
    case 6: {
        s32 ret = CdSync(1, g_cd_result);
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
        if (g_cd_result[0] & 0x10) {
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
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            if (g_cd_result[0] & 0x10) {
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
        s32 ret = CdSync(1, g_cd_result);
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

/* Tentative definitions (COMMON) of the small data this file reaches
 * gp-relative (Q65). */
CdlATV D_800A36B8;
CdlATV g_cd_atv;
u8 g_cd_result[8];
s16 D_800A3840;
s16 D_800A3854;
