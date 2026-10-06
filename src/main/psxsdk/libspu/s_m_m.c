/* PsyQ 4.0 LIBSPU S_M_M: SpuMalloc. .text 0x800893D8..0x800896A0, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

/* PsyQ 4.0 LIBSPU s_m_m: SpuMalloc — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libspu/s_m_m.c */
s32 SpuMalloc(s32 size) {
    s32 var_s2;
    s32 var_s3;
    s32 i;

    i = 0;
    var_s2 = -1;

    if (_spu_rev_reserve_wa == 0) {
        var_s3 = 0;
    } else {
        var_s3 = (0x10000 - _spu_rev_offsetaddr) << _spu_mem_mode_plus;
    }

    size += (size & ~_spu_mem_mode_unitM) ? _spu_mem_mode_unitM : 0;
    size >>= _spu_mem_mode_plus;
    size <<= _spu_mem_mode_plus;

    if (_spu_memList[0].addr & 0x40000000) {
        var_s2 = 0;
    } else {
        _spu_gcSPU();

        for (; i < _spu_AllocBlockNum; i++) {
            if (_spu_memList[i].addr & 0x40000000 ||
                (_spu_memList[i].addr & 0x80000000 &&
                 _spu_memList[i].size >= size)) {
                var_s2 = i;
                break;
            }
        }
    }

    if (var_s2 == -1)
        return -1;

    if (_spu_memList[var_s2].addr & 0x40000000) {
        if (var_s2 < _spu_AllocBlockNum &&
            _spu_memList[var_s2].size - var_s3 >= size) {
            s32 next = var_s2 + 1;

            /* FAKE: volatile re-read of the block's addr word, admitted on
               SOTN precedent (owner rulings Q50/Q55, Q53): the target reloads
               the word here; without the cast GCC reuses the register loaded
               for the 0x40000000 test (score 15). SOTN carries the same cast
               at the same statement, marked "Why the volatile?".
               SOTN: src/main/psxsdk/libspu/s_m_m.c:48 @db41b28 */
            _spu_memList[next].addr =
                (*(volatile u32 *)&_spu_memList[var_s2].addr & 0x0FFFFFFF) +
                    size |
                0x40000000;
            _spu_memList[next].size = _spu_memList[var_s2].size - size;

            _spu_AllocLastNum = next;
            _spu_memList[var_s2].size = size;
            _spu_memList[var_s2].addr &= 0x0FFFFFFF;

            _spu_gcSPU();

            return _spu_memList[var_s2].addr;
        }
    } else {
        if (size < _spu_memList[var_s2].size &&
            _spu_AllocLastNum < _spu_AllocBlockNum) {
            u32 _addr = _spu_memList[var_s2].addr + size;
            u32 _size = _spu_memList[var_s2].size - size;
            /* FAKE: record address as integer arithmetic (index first); &_spu_memList[n]
               adds base first (addu operand order, score 1). */
            SpuMemRec *kb =
                (SpuMemRec *)((_spu_AllocLastNum << 3) + (s32)_spu_memList);
            u32 swapAddr = kb->addr;
            u32 swapSize = kb->size;

            kb->addr = _addr | 0x80000000;
            kb->size = _size;
            _spu_AllocLastNum++;
            kb[1].addr = swapAddr;
            kb[1].size = swapSize;
        }

        _spu_memList[var_s2].size = size;
        _spu_memList[var_s2].addr &= 0x0FFFFFFF;
        _spu_gcSPU();

        return _spu_memList[var_s2].addr;
    }
    return -1;
}
