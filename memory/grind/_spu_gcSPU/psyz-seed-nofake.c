/* ===========================================================================
 * DRAFT TRANSPLANT — _spu_gcSPU  (0x800896A0, src/main.c ~line 2009)
 * VARIANT A: NO-FAKE  ***TRY THIS ONE FIRST*** (policy: fake constructs are a
 * last resort and must clear cheat-reviewer; see variant B).
 *
 * (a) psyz source: tmp/psyq_prov/psyz/decomp/src/libspu/s_m_int.c
 *     (PsyQ 4.0, MIT, function `_spu_gcSPU`, 69 lines) — the same Sony object
 *     BB2 links verbatim (memory/closer/psyq-library-census.md §a).
 *
 * (b) MAPPING DECISIONS  (all verified against src/main.c + symbol_addrs.txt,
 *     NOT taken from tmp/psyq_provenance.md, which mis-stated two of them):
 *
 *     psyz identifier      BB2 spelling                    addr        evidence
 *     -------------------  ------------------------------  ----------  --------
 *     SPU_MALLOC           SpuMemRec {u32 addr; u32 size;}  —           src/main.c:59-62 (already declared)
 *     _spu_memList         _spu_memList  (macro)            0x800A2D40  src/main.c:1922
 *                            = ((SpuMemRec *)g_spu_voice_key_c)
 *     _spu_AllocLastNum    g_spu_voice_key_b                0x800A2D3C  see below
 *     _spu_AllocBlockNum   g_spu_voice_key_a                0x800A2D38  see below
 *
 *     *** CORRECTION to tmp/psyq_provenance.md ***  That report says
 *     "_spu_memList <- D_800A2D40, _spu_AllocLastNum <- D_800A2D3C" and the
 *     addresses are right, but BB2 ALREADY HAS NAMES for all three words and
 *     the current _spu_gcSPU body re-declares them as `extern s32 D_800A2D3C;
 *     extern s32 D_800A2D40;` — a second C handle for memory that already has
 *     one.  Use the existing names.  Proof of the a/b split, from the two
 *     already-matched siblings in the same file:
 *       - SpuInitMalloc (src/main.c:1905) writes top[0], g_spu_voice_key_c=top,
 *         g_spu_voice_key_b=0, g_spu_voice_key_a=num, top[1]=size-0x1010.
 *         psyz s_m_init.c writes _spu_memList=top, [0].addr, [0].size,
 *         _spu_AllocLastNum=0, _spu_AllocBlockNum=num.
 *         => key_c=_spu_memList, key_b=_spu_AllocLastNum, key_a=_spu_AllocBlockNum.
 *       - SpuMalloc (src/main.c:1986) spells `(g_spu_voice_key_b << 3) +
 *         (s32)_spu_memList` = _spu_memList[_spu_AllocLastNum]. Confirms key_b.
 *       - spu_DmaTransfer (=SpuFree, src/main.c:2164) loops `i <
 *         g_spu_voice_key_a`; psyz SpuFree loops `i < _spu_AllocBlockNum`.
 *         Confirms key_a.
 *
 * (c) LOAD-BEARING SHAPE (per tmp/psyq_provenance.md, verified vs asm):
 *     - plain `for` loops, NOT hand-rotated do/while + goto
 *     - array indexing `_spu_memList[i]`, NOT a hand-written pointer chassis
 *       (target rebuilds the outer address each phase: sll $v0,$a2,3;
 *        addu $a0,$v0,$t5 — that is strength reduction ON an array index)
 *     - the loop bound is a LIVE GLOBAL READ in every `for` condition; do NOT
 *       cache it in a local (target re-loads %hi/%lo(D_800A2D3C) at each phase
 *       head and at each outer back-edge, e.g. .L8008987C)
 *     - phase 3 compare is UNSIGNED (target uses sltu) — keep u32 operands
 *     - phase 5 reads _spu_memList[bound].size BEFORE assigning the bound
 *
 * (d) OPEN QUESTIONS FOR THE MEASURING SESSION:
 *     1. cc1 ICE.  memory/closer/phase3-progress.md records that a structured
 *        pointer-walk scan SIGSEGVs cc1 in this function.  This form is
 *        array-indexed, so it probably sidesteps it — UNMEASURED.  If cc1
 *        segfaults, that is the known catalogued failure, not a new bug.
 *     2. Phase 1's inner `for (j = i + 1;; j++)` has no exit test in the
 *        condition; if GCC turns it into an infinite loop + break the shape
 *        should still match (target: unconditional `j .L800896F0`).  If the
 *        distance stalls in phase 1 only, try variant B (__fake.c).
 *     3. This body replaces src/main.c:2005-2155 INCLUDING the `typedef struct
 *        Entry` and the two `extern s32 D_800A2D3C/D_800A2D40` lines — delete
 *        them, they exist nowhere else.
 *     4. Retire all 104 regfix rules for _spu_gcSPU only after `sandbox
 *        --disable all` reads 0; the rules are score-inert so they cannot help.
 * ======================================================================== */

/* --- context already present in src/main.c; shown for compilability only ---
 * typedef struct { u32 addr; u32 size; } SpuMemRec;   (line 59)
 * extern s32 g_spu_voice_key_b;                       (line 56)
 * extern s32 g_spu_voice_key_c;                       (line 57)
 * #define _spu_memList ((SpuMemRec *)g_spu_voice_key_c)   (line 1922)
 * ------------------------------------------------------------------------ */

void _spu_gcSPU(void) {
    s32 i;
    s32 j;

    /* phase 1 — merge each flagged block with the next flagged block that is
       physically adjacent; `continue` (no i++) so the merged block is retried */
    for (i = 0; i <= g_spu_voice_key_b;) {
        if (_spu_memList[i].addr & 0x80000000) {
            for (j = i + 1;; j++) {
                if (_spu_memList[j].addr != 0x2FFFFFFF) {
                    break;
                }
            }
            if ((_spu_memList[j].addr & 0x80000000) &&
                ((_spu_memList[j].addr & 0x0FFFFFFF) ==
                 (_spu_memList[i].addr & 0x0FFFFFFF) + _spu_memList[i].size)) {
                _spu_memList[j].addr = 0x2FFFFFFF;
                _spu_memList[i].size += _spu_memList[j].size;
                continue;
            }
        }
        i++;
    }

    /* phase 2 — zero-size entries become sentinels */
    for (i = 0; i <= g_spu_voice_key_b; i++) {
        if (_spu_memList[i].size == 0) {
            _spu_memList[i].addr = 0x2FFFFFFF;
        }
    }

    /* phase 3 — bubble sort by (addr & 0x0FFFFFFF), UNSIGNED compare */
    for (i = 0; i <= g_spu_voice_key_b; i++) {
        if (_spu_memList[i].addr & 0x40000000) {
            break;
        }
        for (j = i + 1; j <= g_spu_voice_key_b; j++) {
            if (_spu_memList[j].addr & 0x40000000) {
                break;
            }
            if ((_spu_memList[j].addr & 0x0FFFFFFF) <
                (_spu_memList[i].addr & 0x0FFFFFFF)) {
                u32 swapAddr = _spu_memList[i].addr;
                u32 swapSize = _spu_memList[i].size;
                _spu_memList[i].addr = _spu_memList[j].addr;
                _spu_memList[i].size = _spu_memList[j].size;
                _spu_memList[j].addr = swapAddr;
                _spu_memList[j].size = swapSize;
            }
        }
    }

    /* phase 4 — first sentinel is overwritten by the last entry, list shrinks */
    for (i = 0; i <= g_spu_voice_key_b; i++) {
        if (_spu_memList[i].addr & 0x40000000) {
            break;
        }
        if (_spu_memList[i].addr == 0x2FFFFFFF) {
            _spu_memList[i].addr = _spu_memList[g_spu_voice_key_b].addr;
            _spu_memList[i].size = _spu_memList[g_spu_voice_key_b].size;
            g_spu_voice_key_b = i;
            break;
        }
    }

    /* phase 5 — backward tail coalesce */
    for (i = g_spu_voice_key_b - 1; i >= 0; i--) {
        if (!(_spu_memList[i].addr & 0x80000000)) {
            break;
        }
        _spu_memList[i].addr &= 0x0FFFFFFF;
        _spu_memList[i].addr |= 0x40000000;
        _spu_memList[i].size += _spu_memList[g_spu_voice_key_b].size;
        g_spu_voice_key_b = i;
    }
}
