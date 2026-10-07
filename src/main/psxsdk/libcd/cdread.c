/* PsyQ 4.0 LIBCD CDREAD: CdRead and its callbacks (cb_read .. CdReadMode; SOTN
 * libcd/cdread.c). .text 0x80082050..0x800828CC, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3; one file across an old cut at
 * 0x8008289C, which was mid-module (Q106 D3). */
#include "common.h"
#include "libcd_internal.h"
#include <psxsdk/libc.h>
#include <psxsdk/libetc.h>

/* cb_read's and cd_read_retry's messages (here by Q106 D4: every reader is
 * in this file). */
const char D_800162D4[24] = "CdRead: sector error\n\0\0\0";

const char D_800162EC[24] = "CdRead: Shell open...\n\0\0";

const char D_80016304[20] = "CdRead: retry...\n\0\0\0";

/* External linkage (Sony's cdread.c had cb_data static): the bytes are
   identical either way. */
void cb_data(void);

/* The cdread module state is D_800A14D0 (CdlREAD, include/psxsdk/libcd.h). */
extern u8 *D_800A1504; /* v1.86: saved result ptr for cb dispatch */
extern CdlCB g_CdReadCallback_func; /* CD_ReadCallbackFunc */

/* cb_read (static in Sony's source); v1.86 deltas from SOTN's: the saved
   result ptr D_800A1504 and the tsl-mode deferred advance via cb_data. */
static void cb_read(u8 intr, u8 *result) {
    /* FAKE: second handles for D_800A14D0.cnt / .size / .tslmode, each read
     * once below, so each is addressed as %hi/%lo of its own symbol instead
     * of off one `la` base; all through the struct: score 2 (Q99,
     * aggregate-merge-family) */
    extern volatile s32 D_800A14E4, D_800A14E0, g_CdReadMode_value;
    s32 pos[3];

    D_800A1504 = result;
    if (intr == 1) {
        if (D_800A14E4 > 0) {
            if (D_800A14E0 == 0x200) {
                if (g_CdReadMode_value & 1) {
                    CdDataCallback(0);
                    CdGetSector2((s32)pos, 3);
                    CdDataSync(0);
                    CdDataCallback((s32)&cb_data);
                } else {
                    CdGetSector((s32)pos, 3);
                }
                if (CdPosToInt((CdlLOC *)pos) != D_800A14D0.pos) {
                    puts(D_800162D4);
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
        g_CdReadCallback_func(D_800A14D0.cnt == 0 ? 2 : 5, result);
    }
}

/* The tsl-mode data-DMA-complete callback cb_read installs; performs the
   deferred buffer advance. */
void cb_data(void) {
    D_800A14D0.p += D_800A14D0.size * 4;
    D_800A14D0.cnt--;
    D_800A14D0.pos++;
    if (D_800A14D0.cnt != 0) {
        return;
    }
    CdSyncCallback(D_800A14D0.cbsync);
    CdReadyCallback(D_800A14D0.cbready);
    if (D_800A14D0.tslmode & 1) {
        CdDataCallback(D_800A14D0.cbdata);
    }
    CdControlF(9, 0);
    if (g_CdReadCallback_func != 0) {
        g_CdReadCallback_func(2, D_800A1504);
    }
}

s32 cd_read_retry(s32 arg0) {
    u8 sp10;
    s32 temp_s0;

    CdSyncCallback(0);
    CdReadyCallback(0);
    if (D_800A14D0.tslmode & 1) {
        CdDataCallback(0);
    }
    if (CdStatus() & 0x10) {
        if (!(VSync(-1) & 0x3F)) {
            puts(D_800162EC);
        }
        CdControlF(1, 0);
        D_800A14D0.t1 = VSync(-1);
        D_800A14D0.cnt = -1;
        return D_800A14D0.cnt;
    }
    if (arg0 != 0) {
        puts(D_80016304);
        CdControl(9, 0, 0);
        if (CdControl(2, CdLastPos(), 0) == 0) {
            return D_800A14D0.cnt = -1;
        }
    }
    CdFlush();
    temp_s0 = D_800A14D0.mode;
    sp10 = temp_s0;
    temp_s0 = temp_s0 & 0xFF;
    if (temp_s0 != CdMode() || arg0 != 0) {
        if (CdControl(0xE, &sp10, 0) == 0) {
            D_800A14D0.cnt = -1;
            return D_800A14D0.cnt;
        }
    }
    D_800A14D0.pos = CdPosToInt(CdLastPos());
    CdReadyCallback(cb_read);
    if (D_800A14D0.tslmode & 1) {
        CdDataCallback((s32)&cb_data);
    }
    D_800A14D0.p = D_800A14D0.buf;
    CdControlF(6, 0);
    D_800A14D0.cnt = D_800A14D0.sectors;
    D_800A14D0.t2 = VSync(-1);
    return D_800A14D0.cnt;
}

void CdReadBreak(void) {
    if (D_800A14D0.tslmode & 1) {
        CdDataSync(0);
    }
    D_800A14D0.cnt = 0;
    CdSyncCallback(D_800A14D0.cbsync);
    CdReadyCallback(D_800A14D0.cbready);
    if (D_800A14D0.tslmode & 1) {
        CdDataCallback(D_800A14D0.cbdata);
    }
    CdControlF(9, 0);
}

s32 CdRead(s32 sectors, s32 buf, s32 mode) {
    D_800A14D0.mode = mode;
    switch (D_800A14D0.mode & 0x30) {
    case 0:
        D_800A14D0.size = 0x200;
        break;
    case 0x20:
        D_800A14D0.size = 0x249;
        break;
    default:
        D_800A14D0.size = 0x246;
        break;
    }
    D_800A14D0.mode |= 0x20;
    D_800A14D0.buf = buf;
    D_800A14D0.sectors = sectors;
    D_800A14D0.cbsync = CdSyncCallback(0);
    D_800A14D0.cbready = CdReadyCallback(0);
    if (D_800A14D0.tslmode & 1) {
        D_800A14D0.cbdata = CdDataCallback(0);
    }
    D_800A14D0.t1 = VSync(-1);
    if (CdStatus() & 0xE0) {
        CdControlB(9, 0, 0);
    }
    return cd_read_retry(0) > 0;
}

s32 CdReadSync(s32 mode, s32 result) {
    s32 var_s0;

    while (1) {
        var_s0 = -1;
        if (VSync(-1) <= D_800A14D0.t1 + 1200) {
            if (D_800A14D0.cnt < 0 || VSync(-1) > D_800A14D0.t2 + 60) {
                cd_read_retry(1);
                var_s0 = D_800A14D0.sectors;
            } else {
                var_s0 = D_800A14D0.cnt;
            }
        }
        if (mode != 0 || var_s0 <= 0) {
            CdReady(1, (u8 *)result);
            return var_s0;
        }
    }
}

CdlCB CdReadCallback(CdlCB func) {
    CdlCB old = g_CdReadCallback_func;
    g_CdReadCallback_func = func;
    return old;
}

s32 CdReadMode(s32 a0) {
    s32 old = D_800A14D0.tslmode;
    D_800A14D0.tslmode = a0;
    return old;
}
