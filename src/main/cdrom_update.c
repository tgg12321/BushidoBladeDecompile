/* The CD module's two state-machine steppers, cdrom_UpdateAudio and
 * cdrom_Update. .text 0x80036140 (ROM 0x26940). Start boundary: G8. Compiled
 * -G8 (owner ruling 2026-09-26, Q10): both read g_cd_result (and
 * cdrom_UpdateAudio the ATV/fade state) off $gp. Those globals are tentative
 * definitions (COMMON, zero bytes), as in cdrom_mix.c; gp-relative at their
 * base only (owner ruling Q62, 2026-09-30, global COMMON model). */
#include "common.h"
#include "bb2.h"
#include "bb2_const.h"

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
    case CDROM_IO_AUDIO_SETMODE:
        cdrom_SetMix(0, 0, 0, 0);
        CdControlF(CdlSetmode, &g_cdrom_state.rec.audio_mode);
        g_cdrom_state.rec.audio_step_count = 0;
        g_cdrom_state.rec.command_delay_count = 0;
        g_cdrom_state.rec.end_countdown = 0;
        g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_SETMODE_SYNC;
        break;
    case CDROM_IO_AUDIO_SETMODE_SYNC: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == CdlComplete) {
            CdControlF(CdlSetloc, (u8 *)&g_cdrom_state.rec.pair);
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_SETLOC_SYNC;
        } else if (ret == CdlDiskError) {
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_ERROR;
        }
        break;
    }
    case CDROM_IO_AUDIO_SETLOC_SYNC:
        if (++g_cdrom_state.rec.command_delay_count >= 3) {
            s32 ret = CdSync(1, g_cd_result);
            if (ret == CdlComplete) {
                CdControlF(CdlSeekP, 0);
                g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_SEEK_SYNC;
            } else if (ret == CdlDiskError) {
                g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_ERROR;
            }
        }
        break;
    case CDROM_IO_AUDIO_SEEK_SYNC: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == CdlComplete) {
            if (g_cdrom_state.rec.audio_hold == 0) {
                CdControlF(
                    g_cdrom_state.rec.is_xa_audio ? CdlReadS : CdlPlay, 0);
                g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_PLAY_SYNC;
            }
        } else if (ret == CdlDiskError) {
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_ERROR;
        }
        break;
    }
    case CDROM_IO_AUDIO_PLAY_SYNC: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == CdlComplete) {
            g_cdrom_state.rec.report_timeout = 0;
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_POLL;
        } else if (ret == CdlDiskError) {
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_ERROR;
        }
        break;
    }
    case CDROM_IO_AUDIO_POLL:
        if (++g_cdrom_state.rec.audio_step_count == 2) {
            cdrom_SetMix(0xFF, 0, 0xFF, 0);
        }
        if (g_cdrom_state.rec.stop_request != 0 &&
            g_cdrom_state.rec.audio_step_count >= 5) {
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_PAUSE;
            break;
        }
        g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_POLL_SYNC;
        if (g_cdrom_state.rec.is_xa_audio != 0) {
            CdControlF(CdlGetlocP, g_cd_result);
            g_cdrom_state.rec.getloc_timeout = 0;
            break;
        }
        CdControlF(CdlNop, 0);
        {
            s32 ret = CdReady(1, g_cd_result);
            if (ret == CdlDataReady) {
                if (!(g_cd_result[4] & 0x80)) {
                    g_cdrom_state.rec.report_timeout = 0;
                    if (CdPosToInt((CdlLOC *)&g_cd_result[3]) >=
                        g_cdrom_state.rec.audio_end_pos) {
                        cdrom_SetMix(0, 0, 0, 0);
                        g_cdrom_state.rec.io_state =
                            g_cdrom_state.rec.audio_repeat
                                ? CDROM_IO_AUDIO_SETMODE
                                : CDROM_IO_AUDIO_PAUSE;
                    }
                }
            } else if (ret == CdlDiskError) {
                g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_ERROR;
            }
        }
        if (g_cdrom_state.rec.report_timeout++ > 0x3C) {
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_ERROR;
        }
        break;
    case CDROM_IO_AUDIO_POLL_SYNC:
        if (g_cdrom_state.rec.end_countdown != 0) {
            g_cdrom_state.rec.end_countdown -= 4;
            if (g_cdrom_state.rec.end_countdown <= 0) {
                cdrom_SetMix(0, 0, 0, 0);
                g_cdrom_state.rec.io_state =
                    g_cdrom_state.rec.audio_repeat ? CDROM_IO_AUDIO_SETMODE
                                                   : CDROM_IO_AUDIO_PAUSE;
                break;
            }
        }
        {
            s32 ret = CdSync(1, g_cd_result);
            if (ret == CdlComplete) {
                g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_POLL;
                if (g_cdrom_state.rec.is_xa_audio != 0) {
                    s32 pos = CdPosToInt((CdlLOC *)&g_cd_result[5]);
                    if (pos >= g_cdrom_state.rec.audio_end_pos) {
                        cdrom_SetMix(0, 0, 0, 0);
                        g_cdrom_state.rec.io_state =
                            g_cdrom_state.rec.audio_repeat
                                ? CDROM_IO_AUDIO_SETMODE
                                : CDROM_IO_AUDIO_PAUSE;
                    } else if (pos >= g_cdrom_state.rec.audio_end_pos - 0x96) {
                        if (g_cdrom_state.rec.end_countdown == 0) {
                            g_cdrom_state.rec.end_countdown =
                                g_cdrom_state.rec.audio_end_pos - pos;
                        }
                    }
                }
            } else if (ret == CdlDiskError) {
                g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_ERROR;
            } else if (g_cdrom_state.rec.is_xa_audio != 0) {
                if (++g_cdrom_state.rec.getloc_timeout >= 0x1F) {
                    CdFlush();
                    g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_POLL;
                }
            }
        }
        break;
    case CDROM_IO_AUDIO_PAUSE:
        cdrom_SetMix(0, 0, 0, 0);
        CdControlF(CdlPause, 0);
        g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_PAUSE_SYNC;
        break;
    case CDROM_IO_AUDIO_PAUSE_SYNC: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == CdlComplete) {
            g_cdrom_state.rec.audio_step_count = 0;
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_STOP_DELAY;
        } else if (ret == CdlDiskError) {
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_ERROR;
        }
        break;
    }
    case CDROM_IO_AUDIO_STOP_DELAY:
        if (++g_cdrom_state.rec.audio_step_count >= 5) {
            g_cdrom_state.rec.io_state = CDROM_IO_IDLE;
        }
        break;
    case CDROM_IO_AUDIO_ERROR:
        cdrom_SetMix(0, 0, 0, 0);
        if (g_cd_result[0] & CdlStatShellOpen) {
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_RECOVER_NOP;
        } else {
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_RECOVER_GETTN;
        }
        break;
    case CDROM_IO_AUDIO_RECOVER_NOP:
        CdControlF(CdlNop, 0);
        g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_RECOVER_SYNC;
        break;
    case CDROM_IO_AUDIO_RECOVER_SYNC: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == CdlComplete) {
            if (g_cd_result[0] & CdlStatShellOpen) {
                g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_RECOVER_NOP;
            } else {
                g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_RECOVER_GETTN;
            }
        } else if (ret == CdlDiskError) {
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_RECOVER_NOP;
        }
        break;
    }
    case CDROM_IO_AUDIO_RECOVER_GETTN:
        CdControlF(CdlGetTN, 0);
        g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_RECOVER_GETTN_SYNC;
        break;
    case CDROM_IO_AUDIO_RECOVER_GETTN_SYNC: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == CdlComplete) {
            g_cdrom_state.rec.io_state =
                g_cdrom_state.rec.stop_request ? CDROM_IO_IDLE
                                               : CDROM_IO_AUDIO_SETMODE;
        } else if (ret == CdlDiskError) {
            g_cdrom_state.rec.io_state = CDROM_IO_AUDIO_ERROR;
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
    case CDROM_IO_IDLE:
        break;
    case CDROM_IO_READ_SETMODE:
        if (g_cdrom_state.rec.stop_request != 0) {
            g_cdrom_state.rec.io_state = CDROM_IO_IDLE;
            break;
        }
        param[0] = 0xA0;
        CdControlF(CdlSetmode, param);
        g_cdrom_state.rec.unk06 = 0;
        g_cdrom_state.rec.read_timeout = 0;
        g_cdrom_state.rec.io_state = CDROM_IO_READ_SETMODE_SYNC;
        break;
    case CDROM_IO_READ_SETMODE_SYNC: {
        s32 sync_status = CdSync(1, g_cd_result);
        if (sync_status == CdlComplete) {
            g_cdrom_state.rec.io_state = CDROM_IO_READ_SETLOC;
            g_cdrom_state.rec.command_delay_count = 0;
        } else if (sync_status == CdlDiskError) {
            g_cdrom_state.rec.io_state = CDROM_IO_ERROR;
        } else if (++g_cdrom_state.rec.read_timeout > 0x3C) {
            g_cdrom_state.rec.io_state = CDROM_IO_NOP;
        }
        break;
    }
    case CDROM_IO_READ_SETLOC:
        if (++g_cdrom_state.rec.command_delay_count >= 3) {
            g_cdrom_state.rec.dest_buffer = g_cdrom_state.rec.read_dest_addr;
            g_cdrom_state.rec.cd20 = g_cdrom_state.rec.read_sector_count;
            g_cdrom_state.rec.expected_pos =
                CdPosToInt(&g_cdrom_state.rec.pair.loc);
            CdControl(CdlSetloc, (u8 *)&g_cdrom_state.rec.pair.loc, 0);
            g_cdrom_state.rec.read_timeout = 0;
            g_cdrom_state.rec.io_state = CDROM_IO_READ_SETLOC_SYNC;
        }
        break;
    case CDROM_IO_READ_SETLOC_SYNC: {
        s32 sync_status = CdSync(1, g_cd_result);
        if (sync_status == CdlComplete) {
            g_cdrom_state.rec.read_timeout = 0;
            CdReadyCallback(cdrom_ReadyCallback);
            CdControlF(CdlReadN, (u8 *)&g_cdrom_state.rec.pair);
            g_cdrom_state.rec.io_state = CDROM_IO_READ_SECTORS;
        } else if (sync_status == CdlDiskError) {
            g_cdrom_state.rec.io_state = CDROM_IO_ERROR;
        } else if (++g_cdrom_state.rec.read_timeout > 0x3C) {
            g_cdrom_state.rec.io_state = CDROM_IO_NOP;
        }
        break;
    }
    case CDROM_IO_READ_SECTORS: {
        s32 sync_status = CdSync(1, g_cd_result);
        if (sync_status == CdlComplete) {
            if (g_cdrom_state.rec.cd20 == 0) {
                g_cdrom_state.rec.io_state = CDROM_IO_READ_DONE;
            } else if (g_cdrom_state.rec.cd20 < 0) {
                g_cdrom_state.rec.io_state = CDROM_IO_ERROR;
            } else if (++g_cdrom_state.rec.read_timeout > 0x3C) {
                CdReadyCallback(0);
                g_cdrom_state.rec.io_state = CDROM_IO_NOP;
            }
        } else if (sync_status == CdlDiskError) {
            CdReadyCallback(0);
            g_cdrom_state.rec.io_state = CDROM_IO_ERROR;
        } else if (++g_cdrom_state.rec.read_timeout > 0x3C) {
            CdReadyCallback(0);
            g_cdrom_state.rec.io_state = CDROM_IO_NOP;
        }
        break;
    }
    case CDROM_IO_READ_DONE:
        g_cdrom_state.rec.io_state = CDROM_IO_IDLE;
        break;
    case CDROM_IO_ERROR:
        if (g_cd_result[0] & CdlStatShellOpen) {
            g_cdrom_state.rec.io_state = CDROM_IO_NOP;
        } else {
            g_cdrom_state.rec.io_state = CDROM_IO_GETTN;
        }
        break;
    case CDROM_IO_NOP:
        CdControlF(CdlNop, 0);
        g_cdrom_state.rec.io_state = CDROM_IO_CMD_SYNC;
        g_cdrom_state.sync_wait_count = 0;
        break;
    case CDROM_IO_CMD_SYNC: {
        s32 sync_status = CdSync(1, g_cd_result);
        if (sync_status == CdlComplete) {
            if (g_cd_result[0] & CdlStatShellOpen) {
                g_cdrom_state.rec.io_state = CDROM_IO_NOP;
            } else {
                g_cdrom_state.rec.io_state = CDROM_IO_GETTN;
            }
        } else if (sync_status == CdlDiskError) {
            g_cdrom_state.rec.io_state = CDROM_IO_NOP;
        } else if (++g_cdrom_state.sync_wait_count > 0xA) {
            CdFlush();
            g_cdrom_state.rec.io_state = CDROM_IO_NOP;
        }
        break;
    }
    case CDROM_IO_GETTN:
        CdControlF(CdlGetTN, 0);
        g_cdrom_state.rec.io_state = CDROM_IO_GETTN_SYNC;
        g_cdrom_state.sync_wait_count = 0;
        break;
    case CDROM_IO_GETTN_SYNC: {
        s32 sync_status = CdSync(1, g_cd_result);
        if (sync_status == CdlComplete) {
            g_cdrom_state.rec.io_state = CDROM_IO_READ_SETMODE;
            VSync(4);
            VSync(4);
            VSync(4);
            VSync(4);
        } else if (sync_status == CdlDiskError) {
            g_cdrom_state.rec.io_state = CDROM_IO_ERROR;
        } else if (++g_cdrom_state.sync_wait_count > 0x1E) {
            CdFlush();
            g_cdrom_state.rec.io_state = CDROM_IO_NOP;
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
