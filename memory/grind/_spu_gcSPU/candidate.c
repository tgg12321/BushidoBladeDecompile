/* ===========================================================================
 * _spu_gcSPU @0x800896A0 (src/main.c) -- BYTES PROVEN, session 1 (2026-09-11).
 *
 * sandbox --disable all == 0 (194/194 insns) AND full-build SHA1 ==
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa (verify-oracle --rebuild, this
 * session). ZERO cheat constructs: no asm, no volatile, no FAKE, no dead
 * locals. Ordinary C.
 *
 * REQUIRES ONE OUT-OF-SCOPE BUILD INPUT (integration handoff):
 *   maspsx_label_nop_funcs.txt  <- append the line `_spu_gcSPU`
 * Without it the build is 192/194: two real load-delay `nop`s are dropped by
 * the maspsx `.L`-label blind spot (maspsx-label-nop-gate.md -- the sanctioned
 * pure-C retirement path; siblings SpuFree and _spu_init in this same TU are
 * already listed). With it: exactly 0.
 *
 * ALSO REMOVED from src/main.c (byte-neutral, measured 22 before and after):
 *   - `typedef struct Entry { s32 w0; s32 w1; } Entry;`  (dead residue of the
 *     retired 2026-08 pointer chassis; no other user in the TU)
 *   - the duplicate `extern s32 _spu_AllocLastNum; extern s32 _spu_memList;`
 *     pair that sat immediately before the INCLUDE_ASM. They are below the
 *     `#define _spu_memList ((SpuMemRec *)_spu_memList)` at main.c:2107, so
 *     the second one macro-expanded to
 *     `extern s32 ((SpuMemRec *)_spu_memList);` -- a cc1 parse error that the
 *     front end error-recovered past. Canonical declarations live at
 *     main.c:57-58, above the macro.
 *
 * SHAPE NOTE (the whole s1 delta, 22 -> 2): phase 1's inner scan must exit by
 * `goto`, not `break`. stmt.c:expand_end_loop rolls a *leading conditional
 * exit* to the bottom of the loop only when that exit jumps to the loop's own
 * end_label/alt_end_label (tools/gcc-2.7.2/stmt.c, the `last_test_insn` scan).
 * A `break` qualifies, so the loop gets rotated, and jump.c then applies
 * duplicate_loop_exit_test to the resulting LOOP_BEG+simplejump, producing a
 * peeled guard copy (+8 insns, and a re-materialized base + 0x2FFFFFFF).
 * A `goto` to a user label after the loop does NOT target end_label, so
 * last_test_insn stays 0, no rotation happens, and the emitted loop is the
 * target's shape: test at top, unconditional `j` back-edge, `j++` in its delay
 * slot.
 * ========================================================================= */
/* PsyQ 4.0 LIBSPU s_m_int.c: _spu_gcSPU -- verbatim-linked Sony object
   (census 2026-07-09); C ref: Xeeynamo/psyz decomp/src/libspu/s_m_int.c */
void _spu_gcSPU(void) {
    s32 i;
    s32 j;

    for (i = 0; i <= _spu_AllocLastNum;) {
        if (_spu_memList[i].addr & 0x80000000) {
            for (j = i + 1;; j++) {
                if (_spu_memList[j].addr != 0x2FFFFFFF) {
                    goto scanned;
                }
            }
        scanned:
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

    for (i = 0; i <= _spu_AllocLastNum; i++) {
        if (_spu_memList[i].size == 0) {
            _spu_memList[i].addr = 0x2FFFFFFF;
        }
    }

    for (i = 0; i <= _spu_AllocLastNum; i++) {
        if (_spu_memList[i].addr & 0x40000000) {
            break;
        }
        for (j = i + 1; j <= _spu_AllocLastNum; j++) {
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

    for (i = 0; i <= _spu_AllocLastNum; i++) {
        if (_spu_memList[i].addr & 0x40000000) {
            break;
        }
        if (_spu_memList[i].addr == 0x2FFFFFFF) {
            _spu_memList[i].addr = _spu_memList[_spu_AllocLastNum].addr;
            _spu_memList[i].size = _spu_memList[_spu_AllocLastNum].size;
            _spu_AllocLastNum = i;
            break;
        }
    }

    for (i = _spu_AllocLastNum - 1; i >= 0; i--) {
        if (!(_spu_memList[i].addr & 0x80000000)) {
            break;
        }
        _spu_memList[i].addr &= 0x0FFFFFFF;
        _spu_memList[i].addr |= 0x40000000;
        _spu_memList[i].size += _spu_memList[_spu_AllocLastNum].size;
        _spu_AllocLastNum = i;
    }
}
