static void D_80082050(u8 intr, u8 *result);
/* External linkage (Sony's cdread.c had cb_data static): byte-identical
   either way, but static linkage bakes the section-local offset into the
   %lo addend, so the sandbox's file-wide cheat strip (which shrinks earlier
   functions) makes the addend diverge from the reference .o — a scorer
   artifact, not a codegen diff (measured s1, 2026-07-18). */
void D_80082320(void);

/* PsyQ 4.0 LIBCD cdread.c module .data block — CD_ReadCallbackFunc followed
   by the volatile cdread state struct (SOTN psxsdk names it D_80032DBC); BB2
   links Sony's CDREAD object verbatim (census 2026-07-09), so
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
   in cdread.c; our per-member externs are the granted §3 view of it).
   func_800827D0 (CdReadSync) declares the one-object CdlREAD view in-body —
   its target bytes address members via displacements off a cached base,
   which only a single C object can produce. saEft00Add / func_800826CC
   declare the CD_sectors scalar view in-body — their target bytes access
   the word as a plain symbol (macro form / pointer-local la). Per-site
   citations at each decl. */

/* PsyQ 4.0 LIBCD cdread.c: cd_read_retry (static) — verbatim-linked Sony
   object (census 2026-07-09). Body below is the HEAD interim form (still
   INCOMPLETE, carries rules); the honest struct respell of this one is
   banked in memory/closer/candidates/ — see phase3-progress.md. These
   per-member externs are the HEAD-era declarations kept only for this
   function; they name the same Sony data block the CdlREAD struct spans. */
/* Per-member view of the same volatile Sony cdread block (CdlREAD above):
   zero-offset symbol accesses are what Sony's cdread.c v1.86 compiles to
   (macro-form lw/sw; the struct+addend spelling la-materializes the first
   access — measured 2026-07-10, cc1psx-confirmed). Volatile pending the
   Ruling-4 block grant (proposal §3, memory/closer/volatile-grant-proposals.md);
   saEft00Add's interim HEAD body compensates with de-volatile casts. */
extern volatile s32 D_800A1500;
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
extern s32 D_800A14CC;   /* CD_ReadCallbackFunc */
extern s32 D_800162D4;   /* "CdRead: sector error\n" */
extern s32 myRobGeneiDraw3(s32, s32); /* CdControlF */

/* PsyQ 4.0 LIBCD cdread: cb_read (static) — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/cdread.c
   cb_read() (v1.86 deltas: saved result ptr D_800A1504, tsl-mode DMA-chain
   split with deferred advance via the cb_data callback below). */
static void D_80082050(u8 intr, u8 *result) {
    s32 pos[3];
    volatile s32 *pp;
    volatile s32 *tsl;

    D_800A1504 = result;
    if (intr == 1) {
        if (D_800A14E4 > 0) {
            if (D_800A14E0 == 0x200) {
                if (D_800A1500 & 1) {
                    tslTmlGetHeda(0);
                    Vu0SetLightColMatrix_80080640((s32)pos, 3);
                    Vu0SetLightColMatrix_80080684(0);
                    tslTmlGetHeda((s32)&D_80082320);
                } else {
                    func_80080620((s32)pos, 3);
                }
                pp = &D_800A14F0; /* target la-form read 0x800820F4+ */
                if (cdrom_BcdToFrames((u8 *)pos) != *pp) {
                    tslTm2LoadImage_2(&D_800162D4);
                    D_800A14E4 = -1;
                }
            }
            tsl = &D_800A1500; /* target la-form read */
            if (*tsl & 1) {
                Vu0SetLightColMatrix_80080640(D_800A14D8, D_800A14E0);
            } else {
                func_80080620(D_800A14D8, D_800A14E0);
                D_800A14D8 += D_800A14E0 * 4;
                D_800A14E4--;
                D_800A14F0++;
            }
        }
    } else {
        D_800A14E4 = -1;
    }
    D_800A14E8 = sys_VSync(-1);
    if (D_800A14E4 < 0) {
        saEft00Add(1);
    }
    if (sys_VSync(-1) > D_800A14EC + 1200) {
        D_800A14E4 = -1;
    }
    if (D_800A14E4 != 0 && sys_VSync(-1) <= D_800A14EC + 1200) {
        return;
    }
    cdrom_SetCallbackA(D_800A14F4);
    cdrom_SetCallbackB(D_800A14F8);
    if (D_800A1500 & 1) {
        tslTmlGetHeda(D_800A14FC);
    }
    myRobGeneiDraw3(9, 0);
    if (D_800A14CC != 0) {
        ((void (*)(u8, u8 *))D_800A14CC)(D_800A14E4 == 0 ? 2 : 5, result);
    }
}

/* PsyQ 4.0 LIBCD cdread: cb_data (static) — the tsl-mode data-DMA-complete
   callback installed by cb_read above; performs the deferred buffer advance. */
void D_80082320(void) {
    D_800A14D8 += D_800A14E0 * 4;
    D_800A14E4--;
    D_800A14F0++;
    if (D_800A14E4 != 0) {
        return;
    }
    cdrom_SetCallbackA(D_800A14F4);
    cdrom_SetCallbackB(D_800A14F8);
    if (D_800A1500 & 1) {
        tslTmlGetHeda(D_800A14FC);
    }
    myRobGeneiDraw3(9, 0);
    if (D_800A14CC != 0) {
        ((void (*)(u8, u8 *))D_800A14CC)(2, D_800A1504);
    }
}

