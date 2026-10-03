#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"
#include "system.h"
#include "psx.h"
#include "libcd.h"

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

/* --- Functions 0x8008008C - 0x800807A8 --- */

BIOS_B_FUNCTION(DeliverEvent, 0x7);

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
extern const char g_str_none[];

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

/* --- text3 segment functions (0x800807A8-0x800827D0, 17 funcs) --- */

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
/* libcd bios.c module types/helpers, hoisted above getintr (their first
 * user). */
typedef struct {
    u8 sync;  /* 0x800A1494 */
    u8 ready; /* 0x800A1495 */
    u8 c;     /* 0x800A1496 */
} CD_intr;

/* bios.c's own module state, restated from Sony's source (owner ruling Q42):
 * the IRQ-mutated interrupt status block (written by getintr from the CD IRQ
 * callback). bb2.ld links this object's .data at 0x800A1494, between the two
 * halves of the split data asm. It is the only C handle to those three bytes. */
static volatile CD_intr Intr = {0}; /* SOTN: src/main/psxsdk/libcd/bios.c:80 @db41b28 */

static inline void _memcpy(void *_dst, void *_src, u32 _size)
{
    char *pDst = (char *)_dst;
    char *pSrc = (char *)_src;
    if (pDst == 0) {
        return;
    }
    while (_size--) {
        *pDst++ = *pSrc++;
    }
}

/* PsyQ 4.0 LIBCD BIOS: getintr — verbatim-linked Sony object;
 * C ref: SOTN src/main/psxsdk/libcd/bios.c (v1.77; @8bd7c77). BB2 links
 * v1.86, whose bytes differ in two places: the DiskError report is two
 * CD_debug-gated printf()s (not puts + one gated printf), and the error mask
 * is 0x1D (CdlStatError|SeekError|IdError|ShellOpen = 0x1D, spelled as the
 * value). CD_status is Sony's `int`; this TU declares the libc-side u_char
 * view, hence the s32 accesses (as in CD_initintr / CD_init below). */
typedef char Result_t[8];
extern s32 CD_status1; /* 0x800A11C8 = Sony CD_status1 */
extern s32 CD_nopen;     /* Sony CD_nopen */
extern s32 D_800A127C[];   /* per-command flags: INT3 is only the acknowledge, completion follows as INT2 (SOTN D_80032B68) */
extern s32 D_800A137C[];   /* per-command "status valid" flags */
extern void Result;    /* Result_t result buffers */
extern void Result_plus_0x8;
extern void Result_plus_0x10;
extern char D_800161E4[]; /* "DiskError: " */
extern char D_800161F0[]; /* "com=%s,code=(%02x:%02x)\n" */
extern char D_8001620C[]; /* "CDROM: unknown intr" */
extern char D_80016220[]; /* "(%d)\n" */
extern volatile u8 *g_cd_reg0;
extern volatile u8 *g_cd_reg1;
extern volatile u8 *g_cd_reg2;
extern volatile u8 *g_cd_reg3;
extern void puts();
extern void printf();

s32 getintr(void) {
    /* FAKE: volatile locals admitted on SOTN precedent (owner rulings Q50 route A, Q53) -- every access becomes a $sp-slot memory round-trip instead of a register, as in the target. */
    volatile char nReg; /* SOTN: src/main/psxsdk/libcd/bios.c:116 @db41b28 */
    volatile Result_t buf; /* SOTN: src/main/psxsdk/libcd/bios.c:117 @db41b28 */
    s32 i, j;
    s32 bHasError;

    *g_cd_reg0 = 1;

    nReg = *g_cd_reg3 & 0x7;

    if (nReg == 0) {
        return 0;
    }

    bHasError = 0;

    while (nReg != (*g_cd_reg3 & 7)) {
        nReg = *g_cd_reg3 & 0x7;
    }

    for (i = 0; i < 8; i++) {
        if ((*g_cd_reg0 & 0x20) == 0) {
            break;
        }
        buf[i] = *g_cd_reg1;
    }
    for (j = i; j < 8; j++) {
        buf[j] = 0;
    }

    *g_cd_reg0 = 1;
    *g_cd_reg3 = 7;
    *g_cd_reg2 = 7;
    if (nReg != 3 || D_800A137C[CD_com]) {
        if (!(*(s32 *)&CD_status & 0x10) && (buf[0] & 0x10)) {
            CD_nopen++;
        }
        *(s32 *)&CD_status = buf[0];
        CD_status1 = buf[1];
        bHasError = *(s32 *)&CD_status;
        bHasError &= 0x1D;
    }
    if (nReg == 5) {
        if (CD_debug > 0) {
            printf(D_800161E4);
        }
        if (CD_debug > 0) {
            printf(D_800161F0, CD_comstr[CD_com], *(s32 *)&CD_status, CD_status1);
        }
    }
    switch (nReg) {
    case 3:
        if (bHasError) {
            Intr.sync = 5;
            _memcpy(&Result, &buf, sizeof(Result_t));
            return 2;
        }
        if (D_800A127C[CD_com]) {
            Intr.sync = 3;
            _memcpy(&Result, &buf, sizeof(Result_t));
            return 1;
        }
        Intr.sync = 2;
        _memcpy(&Result, &buf, sizeof(Result_t));
        return 2;
    case 2:
        Intr.sync = bHasError ? 5 : 2;
        _memcpy(&Result, &buf, sizeof(Result_t));
        return 2;
    case 1:
        if (bHasError && i == 1) {
            bHasError = 0;
        }
        Intr.ready = bHasError ? 5 : 1;
        _memcpy(&Result_plus_0x8, &buf, sizeof(Result_t));
        *g_cd_reg0 = 0;
        *g_cd_reg3 = 0;
        return 4;
    case 4:
        Intr.ready = Intr.c = 4;
        _memcpy(&Result_plus_0x10, &buf, sizeof(Result_t));
        _memcpy(&Result_plus_0x8, &buf, sizeof(Result_t));
        return 4;
    case 5:
        Intr.sync = Intr.ready = 5;
        _memcpy(&Result, &buf, sizeof(Result_t));
        _memcpy(&Result_plus_0x8, &buf, sizeof(Result_t));
        return 6;
    default:
        puts(D_8001620C);
        printf(D_80016220, nReg);
        return 0;
    }
}
extern s32 VSync(s32);
extern void puts(void *);
extern void printf();
extern s32 CheckCallback(void);
extern s32 getintr(void);
extern s32 CD_cbsync;
extern s32 CD_cbready;
extern void Result;
extern void Result_plus_0x8;
extern void Result_plus_0x10;
extern char D_80016240[]; /* "CD_sync" */
extern s32 D_800161B8;
extern s32 D_800161C8;
extern u8 CD_com;
extern s32 CD_comstr[];
extern s32 CD_intstr[];


extern char D_80016248[]; /* "CD_ready" */

/* bios.c's alarm helpers, as in Sony's source (SOTN: src/main/psxsdk/libcd/bios.c:95 @aa53500).
 * SOTN reaches its `volatile Alarm_t Alarm` only through the non-volatile view
 * `((Alarm_t *)&Alarm)->`; include/system.h declares Alarm non-volatile, which is
 * that view without the cast. */
static inline void set_alarm(char *name)
{
    Alarm.time = VSync(-1) + 0x3C0;
    Alarm.count = 0;
    Alarm.name = name;
}

/* SOTN: src/main/psxsdk/libcd/bios.c:102 @aa53500 */
static inline s32 get_alarm(void)
{
    if (Alarm.time < VSync(-1) || Alarm.count++ > 0x3C0000) {
        puts(&D_800161B8);
        printf(&D_800161C8, Alarm.name, CD_comstr[CD_com],
               CD_intstr[Intr.sync], CD_intstr[Intr.ready]);
        CD_flush();
        return -1;
    }
    return 0;
}

/* SOTN: src/main/psxsdk/libcd/bios.c:210 @aa53500 */
static inline void callback(void)
{
    s32 status;
    u8 saved;

    saved = *g_cd_reg0 & 3;
    while (1) {
        status = getintr();
        if (status == 0) {
            break;
        }
        if ((status & 4) && CD_cbready != 0) {
            ((void (*)(u8, void *))CD_cbready)(Intr.ready, &Result_plus_0x8);
        }
        if ((status & 2) && CD_cbsync != 0) {
            ((void (*)(u8, void *))CD_cbsync)(Intr.sync, &Result);
        }
    }
    *g_cd_reg0 = saved;
}

/* SOTN: src/main/psxsdk/libcd/bios.c:232 @aa53500 */
s32 CD_sync(s32 mode, u8 *result)
{
    s32 sync;

    set_alarm(D_80016240);
    while (1) {
        if (get_alarm()) {
            return -1;
        }
        if (CheckCallback()) {
            callback();
        }
        sync = Intr.sync;
        if (sync == 2 || sync == 5) {
            Intr.sync = 2;
            _memcpy(result, &Result, 8);
            return sync;
        }
        if (mode != 0) {
            return 0;
        }
    }
}
/* SOTN: src/main/psxsdk/libcd/bios.c:260 @aa53500 */
s32 CD_ready(s32 mode, u8 *result)
{
    s32 c;
    s32 ready;

    set_alarm(D_80016248);
    while (1) {
        if (get_alarm()) {
            return -1;
        }
        if (CheckCallback()) {
            callback();
        }
        c = Intr.c;
        if (c != 0) {
            Intr.c = 0;
            _memcpy(result, &Result_plus_0x10, 8);
            return c;
        }
        ready = Intr.ready;
        if (ready != 0) {
            Intr.ready = 0;
            _memcpy(result, &Result_plus_0x8, 8);
            return ready;
        }
        if (mode != 0) {
            return 0;
        }
    }
}
/* PsyQ 4.0 LIBCD BIOS: CD_cw — verbatim-linked Sony object;
 * C ref: SOTN src/main/psxsdk/libcd/bios.c (v1.77; BB2 links v1.86, which sets
 * CD_mode before issuing the command and copies the result unconditionally). */

extern s32 D_800A12FC[];  /* per-command "clears ready" flags; [com + 0x40] = param count */
extern s32 D_800A13FC[];  /* per-command "needs param" flags (= D_800A12FC + 0x40) */
extern char D_80016254[]; /* "%s...\n" */
extern char D_8001625C[]; /* "%s: no param\n" */
extern char D_8001626C[]; /* "CD_cw" */

s32 CD_cw(u8 com, u8 *param, u8 *result, s32 async)
{
    /* FAKE: one counter for both loops (the CD_pos copy and the parameter
     * write), reused exactly as SOTN's CD_cw reuses its i (owner rulings
     * Q51, Q53); separate counters do not reproduce the target's allocation. */
    s32 i; /* SOTN: src/main/psxsdk/libcd/bios.c:292 @aa53500 */

    if (CD_debug > 1) {
        printf(D_80016254, CD_comstr[com]);
    }
    if (D_800A13FC[com] != 0 && param == 0) {
        if (CD_debug > 0) {
            printf(D_8001625C, CD_comstr[com]);
        }
        return -2;
    }
    CD_sync(0, 0);
    if (com == 2) {
        for (i = 0; i < 4; i++) {
            CD_pos[i] = param[i];
        }
    }
    if (com == 0xE) {
        CD_mode = param[0];
    }
    Intr.sync = 0;
    if (D_800A12FC[com]) {
        Intr.ready = 0;
    }
    *g_cd_reg0 = 0;
    /* FAKE: the parameter count D_800A13FC[com] read through the preceding
     * table's base, verbatim SOTN (owner rulings Q50/Q55, Q53) -- cse keeps
     * &D_800A12FC from the ready-flag read above live and forms the count's
     * address as that base + 0x100 (asm/funcs/CD_cw.s: `addiu $v0, $v1, 0x100`
     * at 0x80081460). */
    for (i = 0; i < D_800A12FC[com + 0x40]; i++) { /* SOTN: src/main/psxsdk/libcd/bios.c:314 @aa53500 */
        *g_cd_reg2 = param[i];
    }
    CD_com = com;
    *g_cd_reg1 = com;
    if (async != 0) {
        return 0;
    }

    set_alarm(D_8001626C);

    while (Intr.sync == 0) {
        if (get_alarm()) {
            return -1;
        }
        if (CheckCallback()) {
            callback();
        }
    }

    _memcpy(result, &Result, 8);
    return -(Intr.sync == 5);
}

s32 CD_vol(CdlATV *vol) {
    *g_cd_reg0 = 2;
    *g_cd_reg2 = vol->val0;
    *g_cd_reg3 = vol->val1;
    *g_cd_reg0 = 3;
    *g_cd_reg1 = vol->val2;
    *g_cd_reg2 = vol->val3;
    *g_cd_reg3 = 0x20;
    return 0;
}
extern volatile u32 *g_com_delay_reg;

void CD_flush(void) {
    u8 v0;
    *g_cd_reg0 = 1;
    v0 = *g_cd_reg3 & 7;
    if (v0 != 0) {
        do {
            *g_cd_reg0 = 1;
            *g_cd_reg3 = 7;
            *g_cd_reg2 = 7;
            v0 = *g_cd_reg3 & 7;
        } while (v0 != 0);
    }
    Intr.ready = Intr.c = 0;
    Intr.sync = 2;
    *g_cd_reg0 = 0;
    *g_cd_reg3 = 0;
    *g_com_delay_reg = 0x1325;
}
extern volatile u16 *g_cd_spu_voice;
/* PsyQ 4.0 LIBCD bios.c v1.86: CD_initvol — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libcd/bios.c */
s32 CD_initvol(void) {
    CdlATV vol;

    if (g_cd_spu_voice[0xDC] == 0 && g_cd_spu_voice[0xDD] == 0) {
        g_cd_spu_voice[0xC0] = 0x3FFF;
        g_cd_spu_voice[0xC1] = 0x3FFF;
    }

    g_cd_spu_voice[0xD8] = 0x3FFF;
    g_cd_spu_voice[0xD9] = 0x3FFF;
    g_cd_spu_voice[0xD5] = 0xC001;
    vol.val0 = vol.val2 = 0x80;
    vol.val1 = vol.val3 = 0;
    *g_cd_reg0 = 2;
    *g_cd_reg2 = vol.val0;
    *g_cd_reg3 = vol.val1;
    *g_cd_reg0 = 3;
    *g_cd_reg1 = vol.val2;
    *g_cd_reg2 = vol.val3;
    *g_cd_reg3 = 0x20;
    return 0;
}
extern s32 CD_status1;
extern void InterruptCallback(s32, void *);
void cdrom_IrqHandler(void);
void CD_initintr(void) {
    CD_cbready = 0;
    CD_cbsync = 0;
    CD_status1 = 0;
    *(s32 *)&CD_status = 0;
    ResetCallback();
    InterruptCallback(2, cdrom_IrqHandler);
}
extern void D_800162A8;
extern void D_800162B4;
extern void D_800A1498;

s32 CD_init(void) {
    u8 v0;

    puts(&D_800162A8);
    printf(&D_800162B4, &D_800A1498);

    CD_com = 0;
    CD_mode = 0;
    CD_cbready = 0;
    CD_cbsync = 0;
    CD_status1 = 0;
    *(s32 *)&CD_status = 0;

    ResetCallback();
    InterruptCallback(2, cdrom_IrqHandler);

    *g_cd_reg0 = 1;
    v0 = *g_cd_reg3 & 7;
    if (v0 != 0) {
        do {
            *g_cd_reg0 = 1;
            *g_cd_reg3 = 7;
            *g_cd_reg2 = 7;
            v0 = *g_cd_reg3 & 7;
        } while (v0 != 0);
    }

    Intr.ready = Intr.c = 0;
    Intr.sync = 2;
    *g_cd_reg0 = 0;
    *g_cd_reg3 = 0;
    *g_com_delay_reg = 0x1325;

    CD_cw(1, 0, 0, 0);

    if (*(s32 *)&CD_status & 0x10) {
        CD_cw(1, 0, 0, 0);
    }

    if (CD_cw(0xA, 0, 0, 0) != 0) {
        return -1;
    }
    if (CD_cw(0xC, 0, 0, 0) != 0) {
        return -1;
    }
    if (CD_sync(0, 0) != 2) {
        return -1;
    }
    return 0;
}
extern s32 VSync(s32);
extern void puts(void *);
extern void printf();
extern s32 D_800161C8;
extern char D_800162C0[]; /* "CD_datasync" */
extern u8 CD_com;
extern s32 CD_comstr[];
extern s32 CD_intstr[];



extern volatile u32 *g_cd_dma_ctrl;

/* SOTN: src/main/psxsdk/libcd/bios.c:459 @aa53500 */
s32 CD_datasync(s32 mode)
{
    /* FAKE: one return value written on each of the three exits, reused exactly
     * as SOTN's CD_datasync reuses its ret (owner rulings Q51, Q53); a direct
     * return on each exit compiles to 94 insns instead of the target's 91. */
    s32 ret; /* SOTN: src/main/psxsdk/libcd/bios.c:460 @aa53500 */

    set_alarm(D_800162C0);
    while (1) {
        if (get_alarm()) {
            ret = -1;
            break;
        }
        if (!(*g_cd_dma_ctrl & 0x1000000)) {
            ret = 0;
            break;
        }
        if (mode != 0) {
            ret = 1;
            break;
        }
    }
    return ret;
}

extern volatile u32 *g_com_delay_reg;
extern volatile u32 *g_cdrom_delay_reg;
extern volatile u32 *g_cd_dma_ctrl_b4;
extern volatile u32 *g_cd_dma_dest;
extern volatile u32 *g_cd_dma_size;

s32 CD_getsector(s32 a0, s32 a1) {
    *g_cd_reg0 = 0;
    *g_cd_reg3 = CD_REQ_WANT_DATA;
    *g_cdrom_delay_reg = 0x20943;
    *g_com_delay_reg = 0x1323;
    *g_cd_dma_ctrl_b4 |= DMA_CD_ENABLE;
    *g_cd_dma_dest = a0;
    *g_cd_dma_size = a1 | 0x10000;
    while (!(*g_cd_reg0 & CD_STAT_DATA_REQ)) {
    }
    *g_cd_dma_ctrl = DMA_CD_TO_RAM;
    while (*g_cd_dma_ctrl & DMA_BUSY) {
    }
    *g_com_delay_reg = 0x1325;
    return 0;
}
s32 CD_getsector2(s32 a0, s32 a1) {
    *g_cd_reg0 = 0;
    *g_cd_reg3 = CD_REQ_WANT_DATA;
    *g_cdrom_delay_reg = 0x21020843;
    *g_com_delay_reg = 0x1325;
    *g_cd_dma_ctrl_b4 |= DMA_CD_ENABLE;
    *g_cd_dma_dest = a0;
    *g_cd_dma_size = a1 | 0x10000;
    while (!(*g_cd_reg0 & CD_STAT_DATA_REQ)) {
    }
    *g_cd_dma_ctrl = DMA_CD_TO_RAM_CHOPPED;
    {
        /* FAKE: volatile dummy local (Route B, Q48) -- the target stores the CHCR read-back to
         * its own $sp slot (sw $v0,0($sp) at 0x80081EF8, 8-byte frame); a non-volatile local or
         * a bare `*g_cd_dma_ctrl;` drops the store and the frame. */
        volatile s32 tmp;
        tmp = *g_cd_dma_ctrl;
    }
    return 0;
}

extern s32 D_800A1460;
void CD_set_test_parmnum(s32 a0) {
    D_800A1460 = a0;
}


extern s32 CD_cbsync;
extern s32 CD_cbready;
extern void Result_plus_0x8;
extern void Result;
extern s32 getintr(void);

void cdrom_IrqHandler(void) {
    u8 s2;
    s32 s0;
    s2 = *g_cd_reg0 & 3;
    do {
        s0 = getintr();
        if (s0 == 0) break;
        if (s0 & 4) {
            if (CD_cbready != 0) {
                ((void (*)(u8, void *))CD_cbready)(Intr.ready, &Result_plus_0x8);
            }
        }
        if (!(s0 & 2)) continue;
        if (CD_cbsync == 0) continue;
        ((void (*)(u8, void *))CD_cbsync)(Intr.sync, &Result);
    } while (1);
    *g_cd_reg0 = s2;
}
/* PsyQ 4.0 LIBC2 puts: puts — verbatim-linked Sony object (census
   2026-07-09); no public C ref (absent from sotn psxsdk tree); transcribed
   from the ground-truth object: putchar loop with "<NULL>" fallback. */
extern s32 D_800162CC;
extern void putchar();
void puts(void *a0) {
    char *s = a0;
    char c;

    if (s == NULL) {
        s = (char *)&D_800162CC;
    }
    while ((c = *s++) != 0) {
        putchar(c);
    }
}
extern s32 D_800162EC;
extern s32 D_80016304;

/* External linkage (Sony's cdread.c had cb_data static): the linked bytes
   are identical either way. */
void cb_data(void);

/* PsyQ 4.0 LIBCD cdread.c module .data block — CD_ReadCallbackFunc followed
   by the volatile cdread state struct (SOTN psxsdk names it D_80032DBC); BB2
   links Sony's CDREAD object verbatim, so
   D_800A14D0..D_800A1500 are one Sony data block (preceded by
   CD_ReadCallbackFunc at D_800A14CC), not separate globals. Member map
   recorded in memory/closer/sony-naming-map.md. */
typedef struct {
    /* 0x00 */ s32 sectors; /* D_800A14D0 */
    /* 0x04 */ s32 buf;     /* D_800A14D4 */
    /* 0x08 */ s32 p;       /* D_800A14D8 */
    /* 0x0C */ s32 mode;    /* D_800A14DC */
    /* 0x10 */ s32 size;    /* D_800A14E0 */
    /* 0x14 */ s32 cnt;     /* D_800A14E4 */
    /* 0x18 */ s32 t2;      /* D_800A14E8 */
    /* 0x1C */ s32 t1;      /* D_800A14EC */
    /* 0x20 */ s32 pos;     /* D_800A14F0 */
    /* 0x24 */ s32 cbsync;  /* D_800A14F4 */
    /* 0x28 */ s32 cbready; /* D_800A14F8 */
    /* 0x2C */ s32 cbdata;  /* D_800A14FC */
    /* 0x30 */ s32 tslmode; /* D_800A1500 */
} CdlREAD;
/* No file-scope decl for D_800A14D0: the symbol is CD_sectors AND the block
   base simultaneously (Sony CDREAD.OBJ ground truth: every member access
   relocates against the module's own .data section — the state was static
   in cdread.c; the per-member externs below are a view of it).
   CdReadSync declares the one-object CdlREAD view in-body —
   its target bytes address members via displacements off a cached base,
   which only a single C object can produce. cd_read_retry / CdRead
   declare the CD_sectors scalar view in-body — their target bytes access
   the word as a plain symbol (macro form / pointer-local la). Per-site
   citations at each decl. */

/* PsyQ 4.0 LIBCD cdread.c: cd_read_retry (static) — verbatim-linked Sony
   object. The per-member externs below name the same
   Sony data block the CdlREAD struct spans. */
/* Per-member view of the same volatile Sony cdread block (CdlREAD above):
   zero-offset symbol accesses are what Sony's cdread.c v1.86 compiles to
   (macro-form lw/sw; the struct+addend spelling la-materializes the first
   access — cc1psx-confirmed). */
extern volatile s32 g_CdReadMode_value;
extern volatile s32 D_800A14EC;
extern volatile s32 D_800A14E8;
extern volatile s32 D_800A14E4;
extern volatile s32 D_800A14E0;
extern volatile s32 D_800A14DC;
extern volatile s32 D_800A14D4;
extern volatile s32 D_800A14D8;
extern volatile s32 D_800A14F0;
extern volatile s32 D_800A14F4;
extern volatile s32 D_800A14F8;
extern volatile s32 D_800A14FC;

extern u8 *D_800A1504;   /* cdread.c v1.86: saved result ptr for cb dispatch */
extern s32 g_CdReadCallback_func;   /* CD_ReadCallbackFunc */
extern s32 D_800162D4;   /* "CdRead: sector error\n" */

/* PsyQ 4.0 LIBCD cdread: cb_read (static) — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libcd/cdread.c
   cb_read() (v1.86 deltas: saved result ptr D_800A1504, tsl-mode DMA-chain
   split with deferred advance via the cb_data callback below). */
static void cb_read(u8 intr, u8 *result) {
    s32 pos[3];
    volatile s32 *pp;
    volatile s32 *tsl;

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
                pp = &D_800A14F0; /* target la-form read 0x800820F4+ */
                if (CdPosToInt((u8 *)pos) != *pp) {
                    puts(&D_800162D4);
                    D_800A14E4 = -1;
                }
            }
            tsl = &g_CdReadMode_value; /* target la-form read */
            if (*tsl & 1) {
                CdGetSector2(D_800A14D8, D_800A14E0);
            } else {
                CdGetSector(D_800A14D8, D_800A14E0);
                D_800A14D8 += D_800A14E0 * 4;
                D_800A14E4--;
                D_800A14F0++;
            }
        }
    } else {
        D_800A14E4 = -1;
    }
    D_800A14E8 = VSync(-1);
    if (D_800A14E4 < 0) {
        cd_read_retry(1);
    }
    if (VSync(-1) > D_800A14EC + 1200) {
        D_800A14E4 = -1;
    }
    if (D_800A14E4 != 0 && VSync(-1) <= D_800A14EC + 1200) {
        return;
    }
    CdSyncCallback(D_800A14F4);
    CdReadyCallback(D_800A14F8);
    if (g_CdReadMode_value & 1) {
        CdDataCallback(D_800A14FC);
    }
    CdControlF(9, 0);
    if (g_CdReadCallback_func != 0) {
        ((void (*)(u8, u8 *))g_CdReadCallback_func)(D_800A14E4 == 0 ? 2 : 5, result);
    }
}

/* PsyQ 4.0 LIBCD cdread: cb_data (static) — the tsl-mode data-DMA-complete
   callback installed by cb_read above; performs the deferred buffer advance. */
void cb_data(void) {
    D_800A14D8 += D_800A14E0 * 4;
    D_800A14E4--;
    D_800A14F0++;
    if (D_800A14E4 != 0) {
        return;
    }
    CdSyncCallback(D_800A14F4);
    CdReadyCallback(D_800A14F8);
    if (g_CdReadMode_value & 1) {
        CdDataCallback(D_800A14FC);
    }
    CdControlF(9, 0);
    if (g_CdReadCallback_func != 0) {
        ((void (*)(u8, u8 *))g_CdReadCallback_func)(2, D_800A1504);
    }
}

s32 cd_read_retry(s32 arg0) {
    u8 sp10;
    s32 temp_s0;
    /* FAKE: second C handle for D_800A1500 / D_800A14DC -- target materializes
       each address into its own register (0x80082440 and 0x8008252C: lui/addiu
       then lw 0(reg)) instead of the 2-insn %hi/%lo macro form the direct
       global read compiles to. Reading the globals directly leaves the address
       as a bare (mem (symbol_ref)), which aspsx expands to lui/lw and drops
       both addiu (131 insns vs the target's 133). The alias gives the symbol
       address its own pseudo, which survives to the emitted la-form. NB the
       third read of the SAME word at the end of this function stays a direct
       global read, matching target's macro form there. Same shape as
       CdReadBreak and CdRead below, which alias this same Sony cdread block. */
    volatile s32 *tsl;
    /* FAKE: see above — the mode member's address, same mechanism. */
    volatile s32 *md;

    CdSyncCallback(0);
    CdReadyCallback(0);
    tsl = &g_CdReadMode_value;
    if (*tsl & 1) {
        CdDataCallback(0);
    }
    if (CdStatus() & 0x10) {
        if (!(VSync(-1) & 0x3F)) {
            puts(&D_800162EC);
        }
        CdControlF(1, 0);
        D_800A14EC = VSync(-1);
        D_800A14E4 = -1;
        return D_800A14E4;
    }
    if (arg0 != 0) {
        puts(&D_80016304);
        CdControl(9, 0, 0);
        if (CdControl(2, CdLastPos(), 0) == 0) {
            return D_800A14E4 = -1;
        }
    }
    CdFlush();
    md = &D_800A14DC;
    temp_s0 = *md;
    sp10 = temp_s0;
    temp_s0 = temp_s0 & 0xFF;
    if (temp_s0 != CdMode() || arg0 != 0) {
        if (CdControl(0xE, &sp10, 0) == 0) {
            D_800A14E4 = -1;
            return D_800A14E4;
        }
    }
    D_800A14F0 = CdPosToInt(CdLastPos());
    CdReadyCallback((s32)&cb_read);
    if (g_CdReadMode_value & 1) {
        CdDataCallback((s32)&cb_data);
    }
    D_800A14D8 = D_800A14D4;
    CdControlF(6, 0);
    {
        extern volatile s32 D_800A14D0; /* CD_sectors scalar view: target reads
            sectors as 2-insn macro-form (0x800825EC lui/lw) */
        D_800A14E4 = D_800A14D0;
    }
    D_800A14E8 = VSync(-1);
    return D_800A14E4;
}

/* PsyQ 4.0 LIBCD cdread.c: CdReadBreak — verbatim-linked Sony object;
   C ref: sotn-decomp psxsdk shape + v1.86 hooks */
void CdReadBreak(void) {
    volatile s32 *tsl = &g_CdReadMode_value; /* target caches &tslmode in $s0
        (0x80082638 lui/addiu) and re-reads 0($s0) twice */
    if (*tsl & 1) {
        CdDataSync(0);
    }
    D_800A14E4 = 0;
    CdSyncCallback(D_800A14F4);
    CdReadyCallback(D_800A14F8);
    if (*tsl & 1) {
        CdDataCallback(D_800A14FC);
    }
    CdControlF(9, 0);
}

/* PsyQ 4.0 LIBCD cdread.c: CdRead — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/cdread.c */
s32 CdRead(s32 sectors, s32 buf, s32 mode) {
    extern volatile s32 D_800A14D0; /* CD_sectors scalar view; the store below
        goes through a pointer local: target materializes the address
        (0x80082728 lui/addiu) and stores sw $a3,0($v0) */
    volatile s32 *ps;
    D_800A14DC = mode;
    switch (D_800A14DC & 0x30) {
        case 0:
            D_800A14E0 = 0x200;
            break;
        case 0x20:
            D_800A14E0 = 0x249;
            break;
        default:
            D_800A14E0 = 0x246;
            break;
    }
    D_800A14DC |= 0x20;
    ps = &D_800A14D0;
    D_800A14D4 = buf;
    *ps = sectors;
    D_800A14F4 = CdSyncCallback(0);
    D_800A14F8 = CdReadyCallback(0);
    if (g_CdReadMode_value & 1) {
        D_800A14FC = CdDataCallback(0);
    }
    D_800A14EC = VSync(-1);
    if (CdStatus() & 0xE0) {
        CdControlB(9, 0, 0);
    }
    return cd_read_retry(0) > 0;
}

/* PsyQ 4.0 LIBCD cdread.c: CdReadSync — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/cdread.c */
s32 CdReadSync(s32 mode, s32 result) {
    extern volatile CdlREAD D_800A14D0; /* one-object view REQUIRED by target
        bytes: 0x800827E8 caches &t1 in $s1 and addresses the other members
        via displacements off it (lw -0x8($s1)=cnt, -0x4($s1)=t2,
        -0x1C($s1)=sectors) — cross-member addressing only a single C object
        can produce; Sony's cdreadStruct (SOTN cdread.c D_80032DBC) */
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
