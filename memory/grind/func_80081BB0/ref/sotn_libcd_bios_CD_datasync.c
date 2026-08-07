/* ===========================================================================
 * GROUND-TRUTH REFERENCE SOURCE for saEft01Init  (session 7, 2026-08-01)
 *
 * saEft01Init @ 0x80081BB0 IS Sony PsyQ LIBCD `CD_datasync`.  Verbatim excerpt
 * of the matched C from:
 *
 *     Xeeynamo/sotn-decomp  ::  src/main/psxsdk/libcd/bios.c
 *     (fetched 2026-08-01 via `gh api repos/Xeeynamo/sotn-decomp/contents/...`;
 *      full file cached at tmp/grind/saEft01Init/s7/ref/sotn_bios.c)
 *
 * Other independent decomps of the same library object, if a second opinion is
 * ever wanted (found via `gh api search/code -f q='CD_datasync in:file language:c'`):
 *     ladysilverberg/xenogears-decomp :: src/slus_006.64/psyq/libcd/bios.c
 *     hansbonini/psx_tomba            :: src/scus_942.36/psyq/libcd/bios.c
 *     celophi/lom-decomp              :: src/psyq/libcd/BIOS.c
 *     ser-pounce/rood-reverse         :: src/SLUS_010.40/libcd/BIOS.c
 *     Xeeynamo/psyz                   :: decomp/src/libcd/bios.c
 *
 * ---------------------------------------------------------------------------
 * SYMBOL MAPPING (BB2 name -> Sony name).  Every one of these was confirmed
 * against the target disassembly, not guessed:
 *
 *   saEft01Init            CD_datasync(int mode)
 *   a0                     mode
 *   D_800F19B8             Alarm.unk0    (the VSync deadline)
 *   D_800F19BC             Alarm.unk4    (the spin counter)
 *   D_800F19C0             Alarm.unk8    (the caller's name string pointer)
 *   D_800162C0             the string "CD_datasync"
 *   D_800161B8             the string "CD timeout: "   (arg of puts)
 *   D_800161C8             the printf format string
 *   D_800A11DC[]           D_80032AC8[] — the CdlCom name table
 *   D_800A11D5             CD_com       — the current command byte
 *   D_800A125C[]           D_80032B48[] — the interrupt-state name table
 *   D_800A1494             Intr         — struct { u8 sync; u8 ready; u8 c; }
 *                                         so idx_1494[0] = Intr.sync
 *                                            idx_1494[1] = Intr.ready
 *   *D_800A14C0            the DMA3 (CD-ROM) CHCR register; 0x01000000 is the
 *                          channel-busy bit — this is what proves the identity
 *                          independently of the census
 *   sys_VSync              VSync
 *   tslTm2LoadImage_2      puts        (BB2's splat name is a misnomer)
 *   cdrom_ClearIrq         CD_flush    (BB2's splat name is a misnomer)
 *   debug_printf           printf
 *
 * NOTE the BB2 queue name `saEft01Init` and the two `tslTm2LoadImage_2` /
 * `cdrom_ClearIrq` callee names are all auto-generated MISNOMERS.  Do not
 * rename them (queue keys, regfix anchors and the ledger reference them).
 *
 * SIBLINGS: `set_alarm`/`get_alarm` are inlined into three more functions in
 * this same TU.  src/system.c already carries two of them in the same
 * hand-derived goto shape — `cpu_side_move_dir_4` (= CD_sync, its name string
 * is D_80016240) at ~line 366 and the one at ~line 480 (= CD_ready, string
 * D_80016248).  Anything learned here transfers to them verbatim.
 * =========================================================================== */

/* ---- verbatim from sotn-decomp bios.c, lines 95-113 ---------------------- */

static inline void set_alarm(char* name) {
    // schedule timeout for 960 vblanks from now
    ((Alarm_t*)&Alarm)->unk0 = VSync(-1) + 960;
    ((Alarm_t*)&Alarm)->unk4 = 0;
    ((Alarm_t*)&Alarm)->unk8 = name;
}

static inline int get_alarm(void) {
    if (((Alarm_t*)&Alarm)->unk0 < VSync(-1) ||
        ((Alarm_t*)&Alarm)->unk4++ > 0x3C0000) {
        puts("CD timeout: ");
        printf("%s:(%s) Sync=%s, Ready=%s\n", ((Alarm_t*)&Alarm)->unk8,
               D_80032AC8[CD_com], D_80032B48[Intr.sync],
               D_80032B48[Intr.ready]);
        CD_flush();
        return -1;
    }
    return 0;
}

/* ---- verbatim from sotn-decomp bios.c, lines 459-478 --------------------- */

int CD_datasync(int mode) {
    int ret;

    set_alarm("CD_datasync");
    while (true) {
        if (get_alarm()) {
            ret = -1;
            break;
        }
        if (!(*D_80032DAC & 0x01000000)) {
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

/* ---- relevant declarations from the same file ---------------------------- */

typedef struct Alarm_t {
    int unk0;
    int unk4;
    char* unk8;
} Alarm_t;

volatile Alarm_t Alarm;                       /* -> BB2 D_800F19B8 */
static volatile CD_intr Intr = {0};           /* -> BB2 D_800A1494 */
static volatile int* D_80032DAC = (int*)0x1F8010BC;   /* DMA3 CHCR         */
char* D_80032AC8[] = { "CdlSync", "CdlNop", ... };     /* -> BB2 D_800A11DC */
char* D_80032B48[] = { "NoIntr", "DataReady", ... };   /* -> BB2 D_800A125C */
unsigned char CD_com = 0;                              /* -> BB2 D_800A11D5 */

/* ===========================================================================
 * WHAT SESSION 7 MEASURED WHEN THIS WAS TRANSCRIBED INTO BB2 (all with
 * `sandbox saEft01Init --disable all`; the inherited candidate is 8 / 91):
 *
 *   r0   reference hand-inlined verbatim, no helper locals ......  35 / 91
 *   r1   r0 + the three explicit table-base locals ..............  37 / 93
 *   r2   set_alarm/get_alarm as real `static __inline__` .........  31 / 94
 *   +vol Sony's `volatile` on Alarm and Intr, candidate chassis ..  16 / 91
 *        (same declarations applied to r0: no change at all, 35 / 91)
 *   c1   candidate + ONLY the `||` short-circuit timeout test ....  27 / 92
 *   clean  proven chassis + reference statements, zero levers ....  32 / 96
 *   cleank clean + ONLY the k double-set LICM defeat .............  21 / 90
 *
 * So the reference does NOT hand us a lower score directly.  Its value is that
 * it settles WHICH constructs are original — and the answer indicts the
 * inherited 8: the original has NO named `arg4` intermediate and NO `k`.
 * See evidence.md "Session 7" for the full reading.
 * =========================================================================== */
