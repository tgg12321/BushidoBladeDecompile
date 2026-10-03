/* PsyQ 4.0 LIBSND VS_VH: SsVabOpenHead, SsVabOpenHeadSticky, SsVabFakeHead and
 * SsVabOpenHeadWithMode. .text 0x80088058..0x800884C4, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "sound.h"
#include <psxsdk/libspu.h>

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s16 SsVabOpenHeadWithMode(u8 *, s16, s16, u32);
extern s16 kMaxPrograms;
extern s32 _svm_vab_vh[];
extern s32 _svm_vab_pg[];
extern s32 _svm_vab_tn[];
extern u8 _svm_vab_used[];

s16 SsVabOpenHead(s32 a0, s16 a1) {
    return SsVabOpenHeadWithMode((u8 *)a0, a1, 0, 0);
}

s16 SsVabOpenHeadSticky(s32 a0, s16 a1, s32 a2) {
    return SsVabOpenHeadWithMode((u8 *)a0, a1, 1, (u32)a2);
}

s16 SsVabFakeHead(s32 a0, s16 a1, s32 a2) {
    return SsVabOpenHeadWithMode((u8 *)a0, a1, 1, (u32)a2);
}
extern u16 _svm_vab_count;
extern s32 _svm_vab_start[];
extern s32 _svm_vab_total[];
/* PsyQ 4.0 LIBSND vs_vh: SsVabOpenHeadWithMode — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libsnd/vs_vh.c */
s16 SsVabOpenHeadWithMode(u8 *addr, s16 vabid, s16 arg2, u32 sbaddr) {
    int vagLens[256];
    s32 i;
    s32 var_s0;
    s16 vabId_2;
    u16 temp_v1;
    u16 *ptr_vag_off_table;
    u32 magic;
    u32 spuAllocMem;
    u8 num_vags;
    ProgAtr *pProgTable;
    u8 *var_a2;
    VabHdr *vab_hdr_2;
    u32 sum;
    vabId_2 = 0x10;
    if (_spu_getInTransfer() == 1) {
        return -1;
    }
    _spu_setInTransfer(1);
    if (vabid >= 0x10) {
        _spu_setInTransfer(0);
        return -1;
    }
    if (vabid == -1) {
        for (i = 0; i < 16; i++) {
            if (_svm_vab_used[i] == 0) {
                _svm_vab_used[i] = 1;
                vabId_2 = i;
                _svm_vab_count++;
                break;
            }
        }
    } else {
        var_a2 = _svm_vab_used;
        if (var_a2[vabid] == 0) {
            _svm_vab_used[vabid] = 1;
            vabId_2 = vabid;
            _svm_vab_count++;
        }
    }
    if (vabId_2 >= 0x10) {
        _spu_setInTransfer(0);
        return -1;
    }
    var_a2 = addr;
    _svm_vab_vh[vabId_2] = (s32)var_a2;

    var_a2 = var_a2 + 0x20;
    vab_hdr_2 = (VabHdr *)addr;
    magic = vab_hdr_2->form;
    if ((magic >> 8) != ('V' << 0x10 | 'A' << 0x8 | 'B')) {
        _svm_vab_used[vabId_2] = 0;
        _spu_setInTransfer(0);
        _svm_vab_count -= 1;
        return -1;
    }
    if ((magic & 0xFF) == 'p') {
        if (vab_hdr_2->ver >= 5) {
            kMaxPrograms = 0x80;
        } else {
            kMaxPrograms = 0x40;
        }
    } else {
        kMaxPrograms = 0x40;
    }
    if (vab_hdr_2->ps <= kMaxPrograms) {
        _svm_vab_pg[vabId_2] = (s32)var_a2;
        pProgTable = (ProgAtr *)var_a2;
        var_a2 = var_a2 + (kMaxPrograms * 0x10);
        var_s0 = 0;
        for (i = 0; i < kMaxPrograms; i++) {
            pProgTable[i].reserved1 = var_s0;
            if (pProgTable[i].tones != 0) {
                var_s0++;
            }
        }
        var_s0 = 0;
        _svm_vab_tn[vabId_2] = (s32)var_a2;
        ptr_vag_off_table = (u16 *)(var_a2 + (vab_hdr_2->ps << 9));
        num_vags = vab_hdr_2->vs;
        for (i = 0; i < 256; i++) {
            if (num_vags >= i) {
                temp_v1 = *ptr_vag_off_table;
                if (vab_hdr_2->ver >= 5) {
                    vagLens[i] = temp_v1 * 8;
                } else {
                    vagLens[i] = temp_v1 * 4;
                }
                var_s0 += vagLens[i];
            }
            ptr_vag_off_table++;
        }
        if (arg2 == 0) {
            spuAllocMem = SpuMalloc(var_s0);
            if (spuAllocMem == -1) {
                _svm_vab_used[vabId_2] = 0;
                _spu_setInTransfer(0);
                _svm_vab_count -= 1;
                return -1;
            }
        } else {
            spuAllocMem = sbaddr;
        }
        sum = spuAllocMem + var_s0;
        if (sum > 0x80000U) {
        end:
            _svm_vab_used[vabId_2] = 0;

            _spu_setInTransfer(0);
            _svm_vab_count -= 1;
            return -1;
        }
        _svm_vab_start[vabId_2] = spuAllocMem;
        var_s0 = 0;
        for (i = 0; i <= num_vags; i++) {
            var_s0 += vagLens[i];
            if (!(i & 1)) {
                pProgTable[i / 2].reserved2 = (spuAllocMem + var_s0) >> 3;
            } else {
                pProgTable[i / 2].reserved3 = (spuAllocMem + var_s0) >> 3;
            }
        }

        _svm_vab_total[vabId_2] = var_s0;
        _svm_vab_used[vabId_2] = 2;
    } else {
        goto end;
    }
    return vabId_2;
}
