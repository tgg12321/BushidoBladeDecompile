/* PsyQ 4.0 LIBCD CDREAD: CdRead and its callbacks (cb_read .. CdReadMode; SOTN libcd/cdread.c).
 * .text 0x80082050..0x800828CC, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json),
 * Q106 D3. One file across the old system.c|ings2.c cut at 0x8008289C, which was mid-module (Q106
 * D3). */
#include "common.h"
#include "libcd.h"

/* .rodata 0x800162D4..0x80016318: cb_read's and cd_read_retry's messages (moved from
 * src/text1a_b_tail_rodata.c, Q106 D4: every reader is in this file, in link order). */

/* D_800162D4: 1 string(s), 24B @ 0x800162D4 */
const char D_800162D4[24] =
    "CdRead: sector error\n\0\0\0"
    ;

/* D_800162EC: 1 string(s), 24B @ 0x800162EC */
const char D_800162EC[24] =
    "CdRead: Shell open...\n\0\0"
    ;

/* D_80016304: 1 string(s), 20B @ 0x80016304 */
const char D_80016304[20] =
    "CdRead: retry...\n\0\0\0"
    ;

/* Declarations from the file this module was split from (src/main/psxsdk/libcd/bios.c, ex system.c): the
 * LIBCD SYS and BIOS functions CDREAD calls, declared as their definitions declare them (the old file
 * defined them above this module). */
extern s32 VSync(s32);
extern void puts(void *);
u32 CdStatus(void);
u32 CdMode(void);
void *CdLastPos(void);
void CdFlush(void);
s32 CdReady(s32 mode, u8 *result);
s32 CdSyncCallback(s32 a0);
s32 CdReadyCallback(s32 a0);
s32 CdGetSector(s32 madr, s32 size);
s32 CdGetSector2(s32 madr, s32 size);
s32 CdDataCallback(s32 a0);
void CdDataSync(s32 a0);
s32 CdPosToInt(u8 *a0);


/* External linkage (Sony's cdread.c had cb_data static): the linked bytes
   are identical either way. */
void cb_data(void);

/* The cdread module state is D_800A14D0 (CdlREAD, include/libcd.h). */
extern u8 *D_800A1504;   /* cdread.c v1.86: saved result ptr for cb dispatch */
extern s32 g_CdReadCallback_func;   /* CD_ReadCallbackFunc */

/* PsyQ 4.0 LIBCD cdread: cb_read (static) — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libcd/cdread.c
   cb_read() (v1.86 deltas: saved result ptr D_800A1504, tsl-mode DMA-chain
   split with deferred advance via the cb_data callback below). */
static void cb_read(u8 intr, u8 *result) {
    /* FAKE: second handles for D_800A14D0.cnt / .size / .tslmode (0x800A14E4 / E0 / 1500),
     * each read once in the first block below; every other access is through the struct.
     * The target reads all three as %hi/%lo of their own address (0x80082078, 0x8008208C,
     * 0x800820A0). Through the struct, the block's first member read is expanded as
     * `la D_800A14D0+off` and cse's related-value addressing (use_related_value) bases the
     * block's later members on that register, so the first read keeps the la. Measured:
     * all through the struct 2; per-member cnt only 3; size only 2; tslmode only 2;
     * cnt+size 2; cnt+tslmode 3; size+tslmode 2; these three 0. Admitted for cb_read only:
     * owner ruling Q99 (aggregate-merge-family.md). */
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

/* PsyQ 4.0 LIBCD cdread: cb_data (static) — the tsl-mode data-DMA-complete
   callback installed by cb_read above; performs the deferred buffer advance. */
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
        ((void (*)(u8, u8 *))g_CdReadCallback_func)(2, D_800A1504);
    }
}

/* PsyQ 4.0 LIBCD cdread.c: cd_read_retry (static) - verbatim-linked Sony object. */
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
            puts(&D_800162EC);
        }
        CdControlF(1, 0);
        D_800A14D0.t1 = VSync(-1);
        D_800A14D0.cnt = -1;
        return D_800A14D0.cnt;
    }
    if (arg0 != 0) {
        puts(&D_80016304);
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
    CdReadyCallback((s32)&cb_read);
    if (D_800A14D0.tslmode & 1) {
        CdDataCallback((s32)&cb_data);
    }
    D_800A14D0.p = D_800A14D0.buf;
    CdControlF(6, 0);
    D_800A14D0.cnt = D_800A14D0.sectors;
    D_800A14D0.t2 = VSync(-1);
    return D_800A14D0.cnt;
}

/* PsyQ 4.0 LIBCD cdread.c: CdReadBreak — verbatim-linked Sony object;
   C ref: sotn-decomp psxsdk shape + v1.86 hooks */
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

/* PsyQ 4.0 LIBCD cdread.c: CdRead — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/cdread.c */
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

/* PsyQ 4.0 LIBCD cdread.c: CdReadSync — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/cdread.c */
s32 CdReadSync(s32 mode, s32 result) {
    s32 var_s0;

    while (1) {
        var_s0 = -1;
        if (VSync(-1) <= D_800A14D0.t1 + 1200) {
            if (D_800A14D0.cnt < 0 ||
                VSync(-1) > D_800A14D0.t2 + 60) {
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

s32 CdReadCallback(s32 a0) {
    s32 old = g_CdReadCallback_func;
    g_CdReadCallback_func = a0;
    return old;
}

/* PsyQ 4.0 LIBCD cdread.c: CdReadMode — verbatim-linked Sony object (census
   2026-07-09); the "timer" word (0x800A1500) is the +0x30 mode-flag member
   of the volatile cdread module state block (CdlREAD, include/libcd.h). */

s32 CdReadMode(s32 a0) {
    s32 old = D_800A14D0.tslmode;
    D_800A14D0.tslmode = a0;
    return old;
}
