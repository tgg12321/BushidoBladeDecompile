/* PsyQ 4.0 LIBCD SYS: the CD command layer (CdStatus .. CdPosToInt; SOTN
 * libcd/sys.c). .text 0x8008009C..0x80080828, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libcd_internal.h"

/* .rodata 0x80016074..0x8001607C: CdComstr's and CdIntstr's out-of-range
 * name (Q106 D4: every reader is in this file). */
const char g_str_none[8] = "none\0\0\0";

extern s32 DMACallback(s32, s32);

inline u32 CdStatus(void) { return (u8)CD_status; }

u32 CdMode(void) { return CD_mode; }

u32 CdLastCom(void) { return CD_com; }

void *CdLastPos(void) { return CD_pos; }

s32 CdReset(s32 a0) {
    if (a0 == 2) {
        CD_initintr();
        return 1;
    }
    if (CD_init() != 0) {
        return 0;
    }
    if (a0 == 1) {
        if (CD_initvol() != 0) {
            return 0;
        }
    }
    return 1;
}

void CdFlush(void) { CD_flush(); }

s32 CdSetDebug(s32 a0) {
    s32 old = CD_debug;
    CD_debug = a0;
    return old;
}

void *CdComstr(u8 com) {
    if (com > 0x1B) {
        return (void *)g_str_none;
    }
    return (void *)CD_comstr[com];
}

void *CdIntstr(u8 intr) {
    if (intr > 6) {
        return (void *)g_str_none;
    }
    return (void *)CD_intstr[intr];
}

s32 CdSync(s32 mode, u8 *result) { return CD_sync(mode, result); }

s32 CdReady(s32 mode, u8 *result) { return CD_ready(mode, result); }

CdlCB CdSyncCallback(CdlCB func) {
    CdlCB old = CD_cbsync;
    CD_cbsync = func;
    return old;
}

CdlCB CdReadyCallback(CdlCB func) {
    CdlCB old = CD_cbready;
    CD_cbready = func;
    return old;
}

/* .data 0x800A112C..0x800A11AC, the module's 128-byte .data in LIBCD.LIB:
 * per command, whether CdControl* issues CdlSetloc first (SOTN D_80032A24). */
static s32 g_cd_setloc_flags[32] = {
    0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0};

/* Form of psyz decomp/src/libcd/sys.c (Xeeynamo/psyz @ 5d29a0f8e): the shared
 * retry loop of CdControl / CdControlF / CdControlB, four tries. Two changes:
 * the counted for header (psyz's `count = 4; while (count--)` tests before
 * the decrement: score 8) and BB2's CdStatus() accessor for CD_status. */
static inline s32 cd_cw(u8 com, u8 *param, u8 *result, s32 arg3) {
    CdlCB old = CD_cbsync;
    s32 count;

    for (count = 3; count != -1; count--) {
        CD_cbsync = 0;
        if (com != 1 && (CdStatus() & 0x10)) {
            CD_cw(1, 0, 0, 0);
        }
        if (param == 0 || g_cd_setloc_flags[com] == 0 ||
            !CD_cw(2, param, result, 0)) {
            CD_cbsync = old;
            if (!CD_cw(com, param, result, arg3)) {
                return 0;
            }
        }
    }

    CD_cbsync = old;
    return -1;
}

s32 CdControl(u8 com, u8 *param, u8 *result) {
    return cd_cw(com, param, result, 0) == 0;
}

s32 CdControlF(u8 com, u8 *param) { return cd_cw(com, param, 0, 1) == 0; }

s32 CdControlB(u8 com, u8 *param, u8 *result) {
    if (cd_cw(com, param, result, 0)) {
        return 0;
    }
    return CD_sync(0, result) == 2;
}

s32 CdMix(CdlATV *vol) {
    CD_vol(vol);
    return 1;
}

s32 CdGetSector(s32 madr, s32 size) { return CD_getsector(madr, size) == 0; }

s32 CdGetSector2(s32 madr, s32 size) { return CD_getsector2(madr, size) == 0; }

/* returns the previous callback */
s32 CdDataCallback(s32 a0) { return DMACallback(3, a0); }

void CdDataSync(s32 a0) { CD_datasync(a0); }

CdlLOC *CdIntToPos(s32 i, CdlLOC *p) {
    inline int ENCODE_BCD(n) { return ((n / 10) << 4) + (n % 10); }

    i += 150;
    p->sector = ENCODE_BCD(i % 75);
    p->second = ENCODE_BCD(i / 75 % 60);
    p->minute = ENCODE_BCD(i / 75 / 60);
    return p;
}

s32 CdPosToInt(CdlLOC *p) {
    u8 b0 = p->minute;
    u8 b1 = p->second;
    s32 min, sec, frm;
    min = (b0 >> 4) * 10 + (b0 & 0xF);
    sec = min * 60;
    sec += (b1 >> 4) * 10 + (b1 & 0xF);
    {
        s32 total = sec * 75;
        u8 b2 = p->sector;
        frm = (b2 >> 4) * 10 + (b2 & 0xF);
        total += frm;
        return total - 150;
    }
}
