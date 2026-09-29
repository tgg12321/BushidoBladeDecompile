/* PsyQ 4.0 LIBCOMB COMB (link-cable SIO driver) — verbatim-linked Sony object
 * (census 2026-07-09). This file is that one object: its .text
 * (0x8008BE04-0x8008D050: AddCOMB .. __nulldev, in Sony's order) and its
 * .rdata (0x8001649C-0x800164F8: the two driver strings, then
 * _comb_control's three switch tables), placed after main.o by bb2.ld.
 * Boundaries and symbol offsets: the COMB module of PsyQ 4.0 LIBCOMB.LIB;
 * evidence and measurements: memory/grind/_comb_control/evidence.md [s2].
 * No published C reference (psyz decomp/src/libcomb/comb.c is INCLUDE_ASM). */
#include "common.h"

/* SIO port registers (0x1F801050, hardware I/O: volatile is type-level). */
typedef struct {
    u8 data;
    u8 unk1[3];
    u16 stat;
    u16 unk6;
    u16 mode;
    u16 ctrl;
    u16 misc;
    u16 baud;
} SioRegs;

/* One asynchronous transfer request: `sen` (write) and `rec` (read). */
typedef struct {
    s32 flag;
    u8 *buf;
    s32 len;
    s32 dsr;
} SioReq;

extern volatile SioRegs *D_800A3044;  /* SIO registers */
extern volatile u32 *D_800A3048;      /* SIO_DATA as a word */
extern s32 D_800A304C;                /* interrupt handler record */
extern volatile u32 *D_800A305C;      /* I_STAT (I_MASK at [1]) */
extern s16 D_800A3060[];              /* packet size -> SIO_CTRL rx bits */
extern s16 D_800A3074[];              /* SIO_CTRL rx bits -> packet size */
extern s32 D_800A307C;                /* "sio" device control block */

/* Module state (.bss): shadow SIO registers, wait callback, requests. */
extern volatile SioRegs regs;
extern s32 (*CombWaitCallback)(s32, s32);
extern volatile SioReq sen;
extern volatile SioReq D_800F1AFC;    /* rec */

extern s32 EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern s32 ResetGraph(s32);
extern void DeliverEvent(u32, u32);
extern void AddDrv(s32 *);
extern void DelDrv(const char *);
extern void FlushCache(void);
/* Not declared by any PsyQ 4.0 header (KERNEL.H lists neither), so COMB
   called them undeclared: int-returning calls with no prototype. */
extern s32 SysEnqIntRP();
extern s32 SysDeqIntRP();

const char D_8001649C[12] = "SIO console";
const char D_800164A8[4] = "sio";

void AddCOMB(void) {
    s32 intr;

    intr = EnterCriticalSection();
    AddDrv(&D_800A307C);
    if (intr == 1) {
        ExitCriticalSection();
    }
}

void DelCOMB(void) {
    s32 intr;

    intr = EnterCriticalSection();
    DelDrv(D_800164A8);
    FlushCache();
    if (intr == 1) {
        ExitCriticalSection();
    }
}

void ChangeClearSIO(s32 val) {
}

static s32 SioAnsyncRead(u8 *buf, s32 len) {
    if (D_800F1AFC.flag) {
        return -1;
    }
    D_800F1AFC.len = len;
    D_800F1AFC.buf = buf;
    D_800F1AFC.flag = 1;
    D_800A3044->ctrl |= 0x800;
    D_800A3044->ctrl |= 0x20;
    return 0;
}

static s32 SioSyncroRead(u8 *buf, s32 len) {
    s32 i;
    s32 cnt = 0;
    s32 size;
    u16 ctrl;

    if (D_800F1AFC.flag) {
        return -1;
    }
    size = D_800A3074[(regs.ctrl & 0x300) >> 8];
    D_800F1AFC.len = len;
    D_800F1AFC.buf = buf;
    D_800F1AFC.flag = 0;
    D_800A3044->ctrl |= 0x20;
    i = 0;
    while (D_800F1AFC.len) {
        if (D_800A3044->stat & 0x38) {
            ctrl = D_800A3044->ctrl;
            D_800A3044->ctrl = 0x50;
            D_800A3044->mode = regs.mode;
            D_800A3044->baud = regs.baud;
            D_800A3044->ctrl |= 0x10;
            D_800A3044->ctrl = ctrl;
            D_800A3044->ctrl &= ~0x20;
            DeliverEvent(0xF000000B, 0x8000);
            return len - D_800F1AFC.len;
        }
        while (!(D_800A3044->stat & 2)) {
            if (CombWaitCallback && (*CombWaitCallback)(1, cnt++) == 0) {
                D_800A3044->ctrl &= ~0x20;
                DeliverEvent(0xF000000B, 0x100);
                return len - D_800F1AFC.len;
            }
        }
        *D_800F1AFC.buf = D_800A3044->data;
        D_800F1AFC.buf++;
        D_800F1AFC.len--;
        if (++i == size) {
            i = 0;
            D_800A3044->ctrl ^= 2;
        }
    }
    D_800A3044->ctrl &= ~0x20;
    return len - D_800F1AFC.len;
}

static s32 SioAnsyncWrite(u8 *buf, s32 len) {
    if (sen.flag) {
        return -1;
    }
    sen.len = len;
    sen.buf = buf;
    sen.flag = 1;
    sen.dsr = D_800A3044->stat & 0x80;
    D_800A3044->ctrl |= 0x400;
    return 0;
}

static s32 SioSyncroWrite(u8 *buf, s32 len) {
    s32 i;
    s32 cnt = 0;
    s32 size;

    if (sen.flag) {
        return -1;
    }
    size = D_800A3074[(regs.ctrl & 0x300) >> 8];
    sen.len = len;
    sen.buf = buf;
    i = 0;
    while (sen.len) {
        while ((D_800A3044->stat & 5) != 5) {
            if (CombWaitCallback && (*CombWaitCallback)(2, cnt++) == 0) {
                DeliverEvent(0xF000000B, 0x100);
                return len - sen.len;
            }
        }
        if (i == 0) {
            sen.dsr = D_800A3044->stat & 0x80;
        }
        D_800A3044->data = *sen.buf;
        sen.buf++;
        sen.len--;
        if (++i == size) {
            while ((D_800A3044->stat & 0x80) == sen.dsr) {
                if (CombWaitCallback && (*CombWaitCallback)(2, cnt++) == 0) {
                    DeliverEvent(0xF000000B, 0x100);
                    return len - sen.len - 1;
                }
            }
            i = 0;
        }
    }
    return len - sen.len;
}

s32 _comb_control(u32 cmd, u32 arg, u32 param) {
    s32 ret = 0;

    switch (cmd) {
    case 0:
        switch (arg) {
        case 0:
            ret = D_800A3044->stat;
            break;
        case 1:
            ret = (D_800A3044->ctrl >> 1) & 1;
            if (D_800A3044->ctrl & 0x20) {
                ret |= 2;
            }
            break;
        case 2:
            ret = regs.mode;
            break;
        case 3:
            ret = 2073600 / regs.baud;
            break;
        case 4:
            ret = 1;
            break;
        case 5:
            if (param == 0) {
                ret = sen.len;
            } else {
                ret = D_800F1AFC.len;
            }
            break;
        case 6:
            ret = (param == 0 ? sen.flag : D_800F1AFC.flag) != 0;
            break;
        }
        break;
    case 1:
        switch (arg) {
        case 0:
            break;
        case 1:
            regs.ctrl = D_800A3044->ctrl & ~0x22;
            regs.ctrl |= ((param & 1) ? 2 : 0) | ((param & 2) ? 0x20 : 0);
            D_800A3044->ctrl = regs.ctrl;
            break;
        case 2:
            regs.mode = param;
            D_800A3044->mode = regs.mode;
            break;
        case 3:
            if (2073600 % param) {
                ret = -1;
                break;
            }
            regs.baud = 2073600 / param;
            D_800A3044->baud = regs.baud;
            D_800A3044->ctrl |= 0x10;
            break;
        case 4:
            if (param != 0 && param < 9) {
                ret = D_800A3060[param];
                if (ret >= 0) {
                    regs.ctrl &= ~0x300;
                    regs.ctrl |= ret;
                    D_800A3044->ctrl = regs.ctrl;
                }
            }
            break;
        }
        break;
    case 2:
        switch (arg) {
        case 0: {
            s32 intr = EnterCriticalSection();

            D_800A3044->ctrl |= 0x50;
            D_800A3044->mode = regs.mode;
            D_800A3044->ctrl = regs.ctrl;
            D_800A3044->baud = regs.baud;
            D_800A3044->ctrl |= 0x10;
            ret = 0;
            if (intr == 1) {
                ExitCriticalSection();
            }
            break;
        }
        case 1:
            D_800A3044->ctrl |= 0x10;
            ret = 0;
            break;
        case 2: {
            s32 intr = EnterCriticalSection();

            D_800A3044->ctrl &= ~0x400;
            sen.flag = 0;
            sen.len = 0;
            if (intr == 1) {
                ExitCriticalSection();
            }
            break;
        }
        case 3: {
            s32 intr = EnterCriticalSection();

            D_800A3044->ctrl &= ~0x820;
            D_800F1AFC.flag = 0;
            D_800F1AFC.len = 0;
            if (intr == 1) {
                ExitCriticalSection();
            }
            break;
        }
        }
        break;
    case 3:
        switch (arg) {
        case 0:
            if (param) {
                D_800A3044->ctrl |= 0x20;
            } else {
                D_800A3044->ctrl &= ~0x20;
            }
            break;
        case 1:
            if (D_800A3044->stat & 0x100) {
                ret = 1;
            }
            break;
        }
        break;
    case 4:
        if (arg == 0) {
            /* FAKE: pointer alias to CombWaitCallback, mechanism: expand_expr
               keeps a plain static's constant address (gcc expr.c:4222), so
               the direct form folds both accesses to %lo(CombWaitCallback)
               (our cc1 and cc1psx alike); through one pointer the address
               is a single pseudo shared by the load and the store, as in
               the target. lever-exhaustion: memory/grind/_comb_control/evidence.md [s2] */
            s32 (**slot)(s32, s32) = &CombWaitCallback;

            ret = (s32)*slot;
            *slot = (s32 (*)(s32, s32))param;
        }
        break;
    case 5:
        if (arg == 0) {
            ResetGraph(5);
        }
        break;
    }
    return ret;
}

static s32 EvalpSio(void) {
    if (!(D_800A305C[0] & D_800A305C[1] & 0x100)) {
        return 0;
    }
    if (sen.flag && (D_800A3044->stat & 1)) {
        D_800A3044->ctrl &= ~0x400;
    }
    return 1;
}

static s32 HandleSio(void) {
    s32 size;

    if (*D_800A305C & 0x100) {
        *D_800A305C = ~0x100;
    }
    if (D_800A3044->stat & 0x38) {
        D_800A3044->ctrl &= ~0x820;
        D_800F1AFC.len = 0;
        D_800F1AFC.flag = 0;
        D_800A3044->ctrl;
        D_800A3044->ctrl = 0x50;
        D_800A3044->mode = regs.mode;
        D_800A3044->baud = regs.baud;
        D_800A3044->ctrl |= 0x10;
        DeliverEvent(0xF000000B, 0x8000);
        return 0;
    }
    if (D_800F1AFC.flag && D_800F1AFC.len && (D_800A3044->stat & 2)) {
        size = D_800A3074[(regs.ctrl & 0x300) >> 8];
        switch (size) {
        case 1:
            D_800F1AFC.buf[0] = D_800A3044->data;
            break;
        case 2:
            D_800F1AFC.buf[0] = D_800A3044->data;
            D_800F1AFC.buf[1] = D_800A3044->data;
            break;
        case 4:
            ((u32 *)D_800F1AFC.buf)[0] = *D_800A3048;
            break;
        case 8:
            ((u32 *)D_800F1AFC.buf)[0] = *D_800A3048;
            ((u32 *)D_800F1AFC.buf)[1] = *D_800A3048;
            break;
        }
        D_800F1AFC.buf += size;
        D_800F1AFC.len -= size;
        if (D_800F1AFC.len == 0) {
            D_800F1AFC.flag = 0;
            D_800A3044->ctrl &= ~0x820;
            DeliverEvent(0xF000000B, 0x400);
        }
        D_800A3044->ctrl ^= 2;
    }
    if (sen.flag && (D_800A3044->stat & 1) && (D_800A3044->stat & 0x80) == sen.dsr) {
        if (sen.len == 0) {
            sen.flag = 0;
            DeliverEvent(0xF000000B, 0x800);
        } else {
            sen.dsr ^= 0x80;
            D_800A3044->data = *sen.buf;
            sen.buf++;
            sen.len--;
            D_800A3044->ctrl |= 0x400;
        }
    }
    D_800A3044->ctrl |= 0x10;
    return 0;
}

static s32 r_sioinit(void) {
    regs.ctrl = 5;
    regs.mode = 0xCE;
    regs.baud = 0xD8;
    D_800A3044->ctrl = 0x50;
    D_800A3044->mode = regs.mode;
    D_800A3044->baud = regs.baud;
    D_800A3044->ctrl |= 0x10;
    D_800A3044->ctrl = regs.ctrl;
    SysDeqIntRP(3, &D_800A304C);
    SysEnqIntRP(3, &D_800A304C);
    D_800A305C[1] |= 0x100;
    sen.flag = D_800F1AFC.flag = 0;
    sen.buf = D_800F1AFC.buf = 0;
    CombWaitCallback = 0;
    sen.len = D_800F1AFC.len = 0;
    return 0;
}

static s32 r_sioopen(void) {
    return 0;
}

static s32 r_sioclose(void) {
    return 0;
}

static s32 r_sioremove(void) {
    D_800A305C[1] &= ~0x100;
    D_800A3044->ctrl |= 0x50;
    D_800A3044->ctrl = regs.ctrl;
    SysDeqIntRP(3, &D_800A304C);
    sen.flag = D_800F1AFC.flag = 0;
    sen.len = D_800F1AFC.len = 0;
    return 0;
}

/* PS1 kernel file control block handed to a device strategy routine. */
typedef struct {
    s32 flags;
    s32 unit;
    u8 *addr;
    s32 count;
    s32 pos;
    s32 dev_flags;
    s32 error;
} Fcb;

static s32 r_siostrategy(Fcb *fcb, s32 mode) {
    fcb->error = 0;
    if (mode == 1) {
        s32 n;

        if (fcb->flags & 0x8000) {
            if (SioAnsyncRead(fcb->addr, fcb->count) != 0) {
                fcb->error = 0x10;
                return -1;
            }
            return 0;
        }
        n = SioSyncroRead(fcb->addr, fcb->count);
        if (n < 0) {
            fcb->error = 5;
        }
        return n;
    }
    if (mode == 2) {
        s32 n;

        if (fcb->flags & 0x8000) {
            if (SioAnsyncWrite(fcb->addr, fcb->count) != 0) {
                fcb->error = 0x10;
                return -1;
            }
            return 0;
        }
        n = SioSyncroWrite(fcb->addr, fcb->count);
        if (n < 0) {
            fcb->error = 5;
        }
        return n;
    }
    fcb->error = 0x16;
    return -1;
}

static s32 __nulldev(void) {
    return 0;
}
