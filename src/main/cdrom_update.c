/* The CD module's two state-machine steppers, cdrom_UpdateAudio and
 * cdrom_Update. .text 0x80036140 (ROM 0x26940). Start boundary: G8. Compiled
 * -G8 (owner ruling 2026-09-26, Q10): both read g_cd_result (and
 * cdrom_UpdateAudio the ATV/fade state) off $gp. Those globals are tentative
 * definitions (COMMON, zero bytes), as in cdrom_mix.c; gp-relative at their
 * base only (owner ruling Q62, 2026-09-30, global COMMON model). */
#include "common.h"
#include "bb2.h"

CdlATV g_cd_atv;
CdlATV g_cd_atv_target;
u8 g_cd_result[8];

void cdrom_UpdateAudio(void) {
    CdlATV atv;

    if (g_cd_fade_len > 0) {
        atv.val0 = (g_cd_atv_target.val0 * g_cd_fade_step +
                    g_cd_atv.val0 * (g_cd_fade_len - g_cd_fade_step)) /
                   g_cd_fade_len;
        atv.val1 = (g_cd_atv_target.val1 * g_cd_fade_step +
                    g_cd_atv.val1 * (g_cd_fade_len - g_cd_fade_step)) /
                   g_cd_fade_len;
        atv.val2 = (g_cd_atv_target.val2 * g_cd_fade_step +
                    g_cd_atv.val2 * (g_cd_fade_len - g_cd_fade_step)) /
                   g_cd_fade_len;
        atv.val3 = (g_cd_atv_target.val3 * g_cd_fade_step +
                    g_cd_atv.val3 * (g_cd_fade_len - g_cd_fade_step)) /
                   g_cd_fade_len;
        CdMix(&atv);
        if (++g_cd_fade_step >= g_cd_fade_len) {
            g_cd_fade_len = 0;
            g_cd_atv = g_cd_atv_target;
        }
    }

    switch (g_cdrom_state.rec.io_state) {
    case 0x10:
        cdrom_SetMix(0, 0, 0, 0);
        CdControlF(0xE, &g_cdrom_state.rec.audio_mode);
        g_cdrom_state.rec.audio_step_count = 0;
        g_cdrom_state.rec.command_delay_count = 0;
        g_cdrom_state.rec.end_countdown = 0;
        g_cdrom_state.rec.io_state = 0x11;
        break;
    case 0x11: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            CdControlF(2, (u8 *)&g_cdrom_state.rec.pair);
            g_cdrom_state.rec.io_state = 0x12;
        } else if (ret == 5) {
            g_cdrom_state.rec.io_state = 0x17;
        }
        break;
    }
    case 0x12:
        if (++g_cdrom_state.rec.command_delay_count >= 3) {
            s32 ret = CdSync(1, g_cd_result);
            if (ret == 2) {
                CdControlF(0x16, 0);
                g_cdrom_state.rec.io_state = 0x13;
            } else if (ret == 5) {
                g_cdrom_state.rec.io_state = 0x17;
            }
        }
        break;
    case 0x13: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            if (g_cdrom_state.rec.audio_hold == 0) {
                CdControlF(g_cdrom_state.rec.is_xa_audio ? 0x1B : 3, 0);
                g_cdrom_state.rec.io_state = 0x14;
            }
        } else if (ret == 5) {
            g_cdrom_state.rec.io_state = 0x17;
        }
        break;
    }
    case 0x14: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            g_cdrom_state.rec.report_timeout = 0;
            g_cdrom_state.rec.io_state = 0x15;
        } else if (ret == 5) {
            g_cdrom_state.rec.io_state = 0x17;
        }
        break;
    }
    case 0x15:
        if (++g_cdrom_state.rec.audio_step_count == 2) {
            cdrom_SetMix(0xFF, 0, 0xFF, 0);
        }
        if (g_cdrom_state.rec.stop_request != 0 &&
            g_cdrom_state.rec.audio_step_count >= 5) {
            g_cdrom_state.rec.io_state = 0x1C;
            break;
        }
        g_cdrom_state.rec.io_state = 0x16;
        if (g_cdrom_state.rec.is_xa_audio != 0) {
            CdControlF(0x11, g_cd_result);
            g_cdrom_state.rec.getloc_timeout = 0;
            break;
        }
        CdControlF(1, 0);
        {
            s32 ret = CdReady(1, g_cd_result);
            if (ret == 1) {
                if (!(g_cd_result[4] & 0x80)) {
                    g_cdrom_state.rec.report_timeout = 0;
                    if (CdPosToInt((CdlLOC *)&g_cd_result[3]) >=
                        g_cdrom_state.rec.audio_end_pos) {
                        cdrom_SetMix(0, 0, 0, 0);
                        g_cdrom_state.rec.io_state =
                            g_cdrom_state.rec.audio_repeat ? 0x10 : 0x1C;
                    }
                }
            } else if (ret == 5) {
                g_cdrom_state.rec.io_state = 0x17;
            }
        }
        if (g_cdrom_state.rec.report_timeout++ > 0x3C) {
            g_cdrom_state.rec.io_state = 0x17;
        }
        break;
    case 0x16:
        if (g_cdrom_state.rec.end_countdown != 0) {
            g_cdrom_state.rec.end_countdown -= 4;
            if (g_cdrom_state.rec.end_countdown <= 0) {
                cdrom_SetMix(0, 0, 0, 0);
                g_cdrom_state.rec.io_state =
                    g_cdrom_state.rec.audio_repeat ? 0x10 : 0x1C;
                break;
            }
        }
        {
            s32 ret = CdSync(1, g_cd_result);
            if (ret == 2) {
                g_cdrom_state.rec.io_state = 0x15;
                if (g_cdrom_state.rec.is_xa_audio != 0) {
                    s32 pos = CdPosToInt((CdlLOC *)&g_cd_result[5]);
                    if (pos >= g_cdrom_state.rec.audio_end_pos) {
                        cdrom_SetMix(0, 0, 0, 0);
                        g_cdrom_state.rec.io_state =
                            g_cdrom_state.rec.audio_repeat ? 0x10 : 0x1C;
                    } else if (pos >= g_cdrom_state.rec.audio_end_pos - 0x96) {
                        if (g_cdrom_state.rec.end_countdown == 0) {
                            g_cdrom_state.rec.end_countdown =
                                g_cdrom_state.rec.audio_end_pos - pos;
                        }
                    }
                }
            } else if (ret == 5) {
                g_cdrom_state.rec.io_state = 0x17;
            } else if (g_cdrom_state.rec.is_xa_audio != 0) {
                if (++g_cdrom_state.rec.getloc_timeout >= 0x1F) {
                    CdFlush();
                    g_cdrom_state.rec.io_state = 0x15;
                }
            }
        }
        break;
    case 0x1C:
        cdrom_SetMix(0, 0, 0, 0);
        CdControlF(9, 0);
        g_cdrom_state.rec.io_state = 0x1D;
        break;
    case 0x1D: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            g_cdrom_state.rec.audio_step_count = 0;
            g_cdrom_state.rec.io_state = 0x1E;
        } else if (ret == 5) {
            g_cdrom_state.rec.io_state = 0x17;
        }
        break;
    }
    case 0x1E:
        if (++g_cdrom_state.rec.audio_step_count >= 5) {
            g_cdrom_state.rec.io_state = 0;
        }
        break;
    case 0x17:
        cdrom_SetMix(0, 0, 0, 0);
        if (g_cd_result[0] & 0x10) {
            g_cdrom_state.rec.io_state = 0x18;
        } else {
            g_cdrom_state.rec.io_state = 0x1A;
        }
        break;
    case 0x18:
        CdControlF(1, 0);
        g_cdrom_state.rec.io_state = 0x19;
        break;
    case 0x19: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            if (g_cd_result[0] & 0x10) {
                g_cdrom_state.rec.io_state = 0x18;
            } else {
                g_cdrom_state.rec.io_state = 0x1A;
            }
        } else if (ret == 5) {
            g_cdrom_state.rec.io_state = 0x18;
        }
        break;
    }
    case 0x1A:
        CdControlF(0x13, 0);
        g_cdrom_state.rec.io_state = 0x1B;
        break;
    case 0x1B: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            g_cdrom_state.rec.io_state =
                g_cdrom_state.rec.stop_request ? 0 : 0x10;
        } else if (ret == 5) {
            g_cdrom_state.rec.io_state = 0x17;
        }
        break;
    }
    }
}

extern void cdrom_UpdateAudio(void);

void cdrom_Update(void) {
    u8 param[4];

    if (g_cdrom_state.rec.io_state >= 0x10) {
        cdrom_UpdateAudio();
        return;
    }
    switch (g_cdrom_state.rec.io_state) {
    case 0:
        break;
    case 2:
        if (g_cdrom_state.rec.stop_request != 0) {
            g_cdrom_state.rec.io_state = 0;
            break;
        }
        param[0] = 0xA0;
        CdControlF(0xE, param);
        g_cdrom_state.rec.unk06 = 0;
        g_cdrom_state.rec.read_timeout = 0;
        g_cdrom_state.rec.io_state = 3;
        break;
    case 3: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            g_cdrom_state.rec.io_state = 4;
            g_cdrom_state.rec.command_delay_count = 0;
        } else if (ret == 5) {
            g_cdrom_state.rec.io_state = 9;
        } else if (++g_cdrom_state.rec.read_timeout > 0x3C) {
            g_cdrom_state.rec.io_state = 0xA;
        }
        break;
    }
    case 4:
        if (++g_cdrom_state.rec.command_delay_count >= 3) {
            g_cdrom_state.rec.dest_buffer = g_cdrom_state.rec.read_dest_addr;
            g_cdrom_state.rec.cd20 = g_cdrom_state.rec.read_sector_count;
            g_cdrom_state.rec.expected_pos =
                CdPosToInt(&g_cdrom_state.rec.pair.loc);
            CdControl(2, (u8 *)&g_cdrom_state.rec.pair.loc, 0);
            g_cdrom_state.rec.read_timeout = 0;
            g_cdrom_state.rec.io_state = 5;
        }
        break;
    case 5: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            g_cdrom_state.rec.read_timeout = 0;
            CdReadyCallback(cdrom_ReadyCallback);
            CdControlF(6, (u8 *)&g_cdrom_state.rec.pair);
            g_cdrom_state.rec.io_state = 6;
        } else if (ret == 5) {
            g_cdrom_state.rec.io_state = 9;
        } else if (++g_cdrom_state.rec.read_timeout > 0x3C) {
            g_cdrom_state.rec.io_state = 0xA;
        }
        break;
    }
    case 6: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            if (g_cdrom_state.rec.cd20 == 0) {
                g_cdrom_state.rec.io_state = 8;
            } else if (g_cdrom_state.rec.cd20 < 0) {
                g_cdrom_state.rec.io_state = 9;
            } else if (++g_cdrom_state.rec.read_timeout > 0x3C) {
                CdReadyCallback(0);
                g_cdrom_state.rec.io_state = 0xA;
            }
        } else if (ret == 5) {
            CdReadyCallback(0);
            g_cdrom_state.rec.io_state = 9;
        } else if (++g_cdrom_state.rec.read_timeout > 0x3C) {
            CdReadyCallback(0);
            g_cdrom_state.rec.io_state = 0xA;
        }
        break;
    }
    case 8:
        g_cdrom_state.rec.io_state = 0;
        break;
    case 9:
        if (g_cd_result[0] & 0x10) {
            g_cdrom_state.rec.io_state = 0xA;
        } else {
            g_cdrom_state.rec.io_state = 0xC;
        }
        break;
    case 0xA:
        CdControlF(1, 0);
        g_cdrom_state.rec.io_state = 0xB;
        g_cdrom_state.sync_wait_count = 0;
        break;
    case 0xB: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            if (g_cd_result[0] & 0x10) {
                g_cdrom_state.rec.io_state = 0xA;
            } else {
                g_cdrom_state.rec.io_state = 0xC;
            }
        } else if (ret == 5) {
            g_cdrom_state.rec.io_state = 0xA;
        } else if (++g_cdrom_state.sync_wait_count > 0xA) {
            CdFlush();
            g_cdrom_state.rec.io_state = 0xA;
        }
        break;
    }
    case 0xC:
        CdControlF(0x13, 0);
        g_cdrom_state.rec.io_state = 0xD;
        g_cdrom_state.sync_wait_count = 0;
        break;
    case 0xD: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            g_cdrom_state.rec.io_state = 2;
            VSync(4);
            VSync(4);
            VSync(4);
            VSync(4);
        } else if (ret == 5) {
            g_cdrom_state.rec.io_state = 9;
        } else if (++g_cdrom_state.sync_wait_count > 0x1E) {
            CdFlush();
            g_cdrom_state.rec.io_state = 0xA;
        }
        break;
    }
    }
}

/* Tentative definitions (COMMON) of the small data this file reaches
 * gp-relative (Q65). */
CdlATV g_cd_atv_target;
CdlATV g_cd_atv;
u8 g_cd_result[8];
s16 g_cd_fade_step;
s16 g_cd_fade_len;
