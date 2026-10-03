/* PsyQ 4.0 LIBCD BIOS: the CD-ROM controller driver (getintr .. cdrom_IrqHandler, the module's
 * static `callback`; $Id: bios.c,v 1.86; SOTN libcd/bios.c). .text 0x80080828..0x80082000, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "system.h"
#include "psx.h"
#include "libcd.h"

/* .rodata 0x8001607C..0x8001622C: the module's strings in front of getintr's jump table
 * (0x8001622C..0x80016240, emitted with getintr below): the CD_comstr / CD_intstr command and
 * interrupt names (read through those .data tables, asm/data/7D920.data.s), then get_alarm's and
 * getintr's messages (moved from src/text1a_b_post_rodata.c, Q106 D4: every C reader is in this
 * file, in link order). */

/* D_8001607C: 29 string(s), 316B @ 0x8001607C (the CD_comstr / CD_intstr names) */
const char D_8001607C[316] =
    "CdlReadS\0\0\0\0CdlSeekP\0\0\0\0"
    "CdlSeekL\0\0\0\0CdlGetTD\0\0\0\0CdlGetTN"
    "\0\0\0\0CdlGetlocP\0\0CdlGetlocL\0\0?\0\0\0"
    "CdlSetmode\0\0CdlSetfilter\0\0\0\0CdlD"
    "emute\0\0\0CdlMute\0CdlReset\0\0\0\0CdlP"
    "ause\0\0\0\0CdlStop\0CdlStandby\0\0CdlR"
    "eadN\0\0\0\0CdlBackward\0CdlForward\0\0"
    "CdlPlay\0CdlSetloc\0\0\0CdlNop\0\0CdlS"
    "ync\0DiskError\0\0\0DataEnd\0Acknowle"
    "dge\0Complete\0\0\0\0DataReady\0\0\0NoIn"
    "tr\0\0"
    ;

/* D_800161B8: 1 string(s), 16B @ 0x800161B8 */
const char D_800161B8[16] =
    "CD timeout: \0\0\0\0"
    ;

/* D_800161C8: 1 string(s), 28B @ 0x800161C8 */
const char D_800161C8[28] =
    "%s:(%s) Sync=%s, Ready=%s\n\0\0"
    ;

/* D_800161E4: 1 string(s), 12B @ 0x800161E4 */
const char D_800161E4[12] =
    "DiskError: \0"
    ;

/* D_800161F0: 1 string(s), 28B @ 0x800161F0 */
const char D_800161F0[28] =
    "com=%s,code=(%02x:%02x)\n\0\0\0\0"
    ;

/* D_8001620C: 1 string(s), 20B @ 0x8001620C */
const char D_8001620C[20] =
    "CDROM: unknown intr\0"
    ;

/* D_80016220: 2 string(s), 12B @ 0x80016220 */
const char D_80016220[12] =
    "(%d)\n\0\0\0\0\0\0\0"
    ;

/* Declarations from the head of the old system.c (now in libcd/sys.c) that this module uses. */
extern void CD_flush(void);
extern u8 CD_status;
extern u8 CD_pos[4]; /* Sony's u_char CD_pos[4] (SOTN: src/main/psxsdk/libcd/bios.c:42 @aa53500) */
extern u8 CD_mode;
extern u8 CD_com;
extern s32 CD_cbsync;
extern s32 CD_cbready;
extern s32 CD_debug;
extern s32 CD_comstr[];
extern s32 CD_intstr[];

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

/* .rodata 0x80016240..0x800162CC: the rest of the module's strings, after getintr's jump table:
 * CD_sync, CD_ready, CD_cw (with the rcsid "$Id: bios.c,v 1.86 ...", which the module's .data block
 * D_800A1498 points at), CD_init and CD_datasync's (moved from src/text1a_b_tail_rodata.c, Q106 D4:
 * every C reader is in this file, in link order). */

/* D_80016240: 1 string(s), 8B @ 0x80016240 */
const char D_80016240[8] =
    "CD_sync\0"
    ;

/* D_80016248: 1 string(s), 12B @ 0x80016248 */
const char D_80016248[12] =
    "CD_ready\0\0\0\0"
    ;

/* D_80016254: 1 string(s), 8B @ 0x80016254 */
const char D_80016254[8] =
    "%s...\n\0\0"
    ;

/* D_8001625C: 1 string(s), 16B @ 0x8001625C */
const char D_8001625C[16] =
    "%s: no param\n\0\0\0"
    ;

/* D_8001626C: 2 string(s), 60B @ 0x8001626C */
const char D_8001626C[60] =
    "CD_cw\0\0\0$Id: bios.c,v 1.86 1997/"
    "03/28 07:42:42 makoto Exp $\0"
    ;

/* D_800162A8: 1 string(s), 12B @ 0x800162A8 */
const char D_800162A8[12] =
    "CD_init:\0\0\0\0"
    ;

/* D_800162B4: 1 string(s), 12B @ 0x800162B4 */
const char D_800162B4[12] =
    "addr=%08x\n\0\0"
    ;

/* D_800162C0: 1 string(s), 12B @ 0x800162C0 */
const char D_800162C0[12] =
    "CD_datasync\0"
    ;

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
extern u8 CD_com;
extern s32 CD_comstr[];
extern s32 CD_intstr[];



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
