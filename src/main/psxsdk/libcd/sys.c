/* PsyQ 4.0 LIBCD SYS: the CD command layer (CdStatus .. CdPosToInt; SOTN libcd/sys.c). .text
 * 0x8008009C..0x80080828, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106
 * D3. */
#include "common.h"
#include "libcd.h"

/* .rodata 0x80016074..0x8001607C: CdComstr's and CdIntstr's out-of-range name (moved from
 * src/text1a_b_post_rodata.c, Q106 D4: every reader is in this file, in link order). */

/* g_str_none: 1 string(s), 8B @ 0x80016074 (CdComstr / CdIntstr out-of-range name) */
const char g_str_none[8] =
    "none\0\0\0"
    ;

/* Forward declarations */
extern void CD_flush(void);
extern s32 CD_sync(s32, u8 *);
extern s32 CD_ready(s32, u8 *);
extern s32 CD_vol(CdlATV *vol);
extern s32 CD_getsector();
extern s32 CD_getsector2();
extern s32 DMACallback(s32, s32);
extern s32 CD_datasync(s32);

/* Externs for globals */
extern u8 CD_status;
extern u8 CD_pos[4]; /* Sony's u_char CD_pos[4] (SOTN: src/main/psxsdk/libcd/bios.c:42 @aa53500) */
extern u8 CD_mode;
extern u8 CD_com;
extern s32 CD_cbsync;
extern s32 CD_cbready;

u32 CdStatus(void) {
    return CD_status;
}

u32 CdMode(void) {
    return CD_mode;
}

u32 CdLastCom(void) {
    return CD_com;
}

void *CdLastPos(void) {
    return CD_pos;
}

extern void CD_initintr(void);
extern s32 CD_init(void);
extern s32 CD_initvol(void);
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

void CdFlush(void) {
    CD_flush();
}

extern s32 CD_debug;
extern s32 CD_comstr[];
extern s32 CD_intstr[];

s32 CdSetDebug(s32 a0) {
    s32 old = CD_debug;
    CD_debug = a0;
    return old;
}

/* PsyQ 4.0 LIBCD sys: CdComstr — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c */
void *CdComstr(u8 com) {
    if (com > 0x1B) {
        return (void *)g_str_none;
    }
    return (void *)CD_comstr[com];
}

/* PsyQ 4.0 LIBCD sys: CdIntstr — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c */
void *CdIntstr(u8 intr) {
    if (intr > 6) {
        return (void *)g_str_none;
    }
    return (void *)CD_intstr[intr];
}

/* PsyQ 4.0 LIBCD sys: CdSync — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c */
s32 CdSync(s32 mode, u8 *result) {
    return CD_sync(mode, result);
}

/* PsyQ 4.0 LIBCD sys: CdReady — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c */
s32 CdReady(s32 mode, u8 *result) {
    return CD_ready(mode, result);
}

s32 CdSyncCallback(s32 a0) {
    s32 old = CD_cbsync;
    CD_cbsync = a0;
    return old;
}

s32 CdReadyCallback(s32 a0) {
    s32 old = CD_cbready;
    CD_cbready = a0;
    return old;
}

extern s32 g_cd_setloc_flags[];
extern s32 CD_cw(u8, u8 *, u8 *, s32);

s32 CdControl(u8 a0, u8 *a1, u8 *a2) {
    s32 result;
    s32 idx;
    s32 saved;
    s32 count;
    s32 *base;
    s32 *elem;

    idx = a0;
    saved = CD_cbsync;
    count = 3;
    base = g_cd_setloc_flags;
    elem = base + idx;
    result = 0;

loop:
    /* FAKE: do-while(0) wrap -- its loop notes weight the references so that
       count/a1/a2/idx/a0/saved/elem/result seat in s0..s7 (flow.c life
       analysis, reg_n_refs += loop_depth, feeding global.c allocno_compare);
       a real-loop restructure does not reproduce that assignment. */
    do {
    CD_cbsync = 0;

    if (idx != 1) {
        if (CD_status & 0x10) {
            CD_cw(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        if ((*elem) != 0) {
            if (CD_cw(2, a1, a2, 0) != 0) {
                goto next;
            }
        }
    }
    CD_cbsync = saved;
    if (CD_cw(a0, a1, a2, 0) == 0) {
        goto done;
    }
next:
    count--;

    if (count != (-1)) {
        goto loop;
    }
    } while (0);
    CD_cbsync = saved;
    result = -1;
done:
    return result + 1;
}
s32 CdControlF(u8 a0, u8 *a1) {
    s32 result;
    s32 idx;
    s32 saved;
    s32 count;
    s32 *base;
    s32 *elem;

    idx = a0;
    saved = CD_cbsync;
    count = 3;
    base = g_cd_setloc_flags;
    elem = base + idx;
    result = 0;

loop:
    /* FAKE: do-while(0) wrap -- its loop-note ref weighting seats elem in s5
       and result in s6 (flow.c life analysis, reg_n_refs += loop_depth,
       feeding global.c allocno_compare). */
    do {
    CD_cbsync = 0;

    if (idx != 1) {
        if (CD_status & 0x10) {
            CD_cw(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        if ((*elem) != 0) {
            if (CD_cw(2, a1, 0, 0) != 0) {
                goto next;
            }
        }
    }
    CD_cbsync = saved;
    if (CD_cw(a0, a1, 0, 1) == 0) {
        goto done;
    }
next:
    count--;

    if (count != (-1)) {
        goto loop;
    }
    } while (0);
    CD_cbsync = saved;
    result = -1;
done:
    return result + 1;
}
s32 CdControlB(u8 a0, u8 *a1, u8 *a2) {
    s32 count;
    s32 idx;
    s32 saved;
    s32 *elem;
    s32 *base;
    s32 status;

    saved = CD_cbsync;
    count = 3;
    idx = a0;
    base = g_cd_setloc_flags;
    elem = base + idx;

loop:
    CD_cbsync = 0;

    if (idx != 1) {
        if (CD_status & 0x10) {
            CD_cw(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        if ((*elem) != 0) {
            if (CD_cw(2, a1, a2, 0) != 0) {
                goto next;
            }
        }
    }
    CD_cbsync = saved;
    if (CD_cw(a0, a1, a2, 0) == 0) {
        status = 0;
        goto done;
    }
next:
    count--;
    status = -1;
    if (count != (-1)) {
        goto loop;
    }
    CD_cbsync = saved;
done:
    if (status != 0) {
        return 0;
    }
    return CD_sync(0, a2) == 2;
}

s32 CdMix(CdlATV *vol) {
    CD_vol(vol);
    return 1;
}

/* PsyQ 4.0 LIBCD sys: CdGetSector / CdGetSector2 — verbatim-linked Sony
   objects; both forward (madr, size) to the CD_ helper. */
s32 CdGetSector(s32 madr, s32 size) {
    return CD_getsector(madr, size) == 0;
}

s32 CdGetSector2(s32 madr, s32 size) {
    return CD_getsector2(madr, size) == 0;
}

/* PsyQ 4.0 LIBCD sys: CdDataCallback — verbatim-linked Sony object (census
   2026-07-09); returns the previous callback */
s32 CdDataCallback(s32 a0) {
    return DMACallback(3, a0);
}

void CdDataSync(s32 a0) {
    CD_datasync(a0);
}

/* PsyQ 4.0 LIBCD sys: CdIntToPos — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c */
u8 *CdIntToPos(s32 i, u8 *p) {
    inline int ENCODE_BCD(n) { return ((n / 10) << 4) + (n % 10); }

    i += 150;
    p[2] = ENCODE_BCD(i % 75);
    p[1] = ENCODE_BCD(i / 75 % 60);
    p[0] = ENCODE_BCD(i / 75 / 60);
    return p;
}


extern s32 CD_cw(u8, u8 *, u8 *, s32);

s32 CdPosToInt(u8 *a0) {
    u8 b0 = a0[0];
    u8 b1 = a0[1];
    s32 min, sec, frm;
    min = (b0 >> 4) * 10 + (b0 & 0xF);
    sec = min * 60;
    sec += (b1 >> 4) * 10 + (b1 & 0xF);
    {
        s32 total = sec * 75;
        u8 b2 = a0[2];
        frm = (b2 >> 4) * 10 + (b2 & 0xF);
        total += frm;
        return total - 150;
    }
}
