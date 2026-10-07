/* PsyQ 4.0 LIBSPU S_M_INT: _spu_gcSPU. .text 0x800896A0..0x800899A8, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

/* _spu_gcSPU. C ref: Xeeynamo/psyz decomp/src/libspu/s_m_int.c. Phase 1's inner
   scan exits by `goto`, not `break`: a break gets the loop rotated with a
   peeled guard copy (+8 insns); the goto keeps test-at-top with `j++` in the
   back-edge delay slot. */
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
