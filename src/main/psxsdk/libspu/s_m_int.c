/* PsyQ 4.0 LIBSPU S_M_INT: _spu_gcSPU. .text 0x800896A0..0x800899A8, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

/* Self-referential on purpose: the object `_spu_memList` (Sony's SPU_MALLOC list pointer,
   declared s32 in libspu_internal.h) is viewed as SpuMemRec* through this macro; a macro
   name inside its own replacement list is not re-expanded (C90 6.8.3.4). */
#define _spu_memList ((SpuMemRec *)_spu_memList)

/* Shape note: phase 1's inner scan exits by `goto`, not `break`.
   stmt.c:expand_end_loop rolls a leading conditional exit to the bottom of the
   loop only when that exit jumps to the loop's own end_label/alt_end_label
   (the `last_test_insn` scan). A `break` qualifies, so the loop gets rotated
   and jump.c:duplicate_loop_exit_test then peels a guard copy (+8 insns). A
   `goto` to a user label after the loop does not target end_label, so
   last_test_insn stays 0, no rotation happens, and the emitted loop has the
   target's shape: test at top, unconditional `j` back-edge, `j++` in its delay
   slot.
   Two load-delay hazard nops across .L merge labels come from maspsx's
   .L-label nop handling, as for siblings SpuFree (s_m_f.c) and _spu_init
   (spu.c). */
/* PsyQ 4.0 LIBSPU s_m_int.c: _spu_gcSPU -- verbatim-linked Sony object;
   C ref: Xeeynamo/psyz decomp/src/libspu/s_m_int.c */
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
