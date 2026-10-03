/* PsyQ 4.0 LIBSPU S_SRMP: SpuSetReverbModeParam. .text 0x80089F3C..0x8008A434, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

/* PsyQ 4.0 LIBSPU s_srmp: SpuSetReverbModeParam — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libspu/s_srmp.c.
   4.0 deltas vs the SOTN revision: the DELAYTIME/FEEDBACK gates are range
   compares (mode >= ECHO && mode <= DELAY) with no default-arm clears, and
   the depth/zero split threads off the var_s4 flag. Reverb preset table:
   D_800A2D94 (= Sony rev_param table, 10 entries x 0x44). */
typedef struct {
    /* 0x00 */ u32 flags;
    /* 0x04 */ u16 dAPF1, dAPF2;
    /* 0x08 */ u16 vIIR, vCOMB1, vCOMB2, vCOMB3, vCOMB4;
    /* 0x12 */ u16 vWALL, vAPF1, vAPF2;
    /* 0x18 */ u16 mLSAME, mRSAME, mLCOMB1, mRCOMB1, mLCOMB2, mRCOMB2;
    /* 0x24 */ u16 dLSAME, dRSAME;
    /* 0x28 */ u16 mLDIFF, mRDIFF, mLCOMB3, mRCOMB3, mLCOMB4, mRCOMB4;
    /* 0x34 */ u16 dLDIFF, dRDIFF;
    /* 0x38 */ u16 mLAPF1, mRAPF1, mLAPF2, mRAPF2;
    /* 0x40 */ u16 vLIN, vRIN;
} RevParamEntry;

extern RevParamEntry _spu_rev_param[]; /* rev_param preset table */

s32 SpuClearReverbWorkArea(u32 rev_mode);

static inline void _memcpy(char *dst, char *src, u32 size) {
    while (size--) {
        *dst++ = *src++;
    }
}

s32 SpuSetReverbModeParam(SpuReverbAttr *attr) {
    RevParamEntry entry;
    u32 var_s0;
    u16 cnt;

    s32 var_s7 = 0;
    s32 var_s4 = 0;
    s32 var_s6 = 0;
    s32 sp58 = 0;
    s32 var_fp = 0;

    u32 mask = attr->mask;
    s32 bSetAll = attr->mask == 0;

    entry.flags = 0;
    if (bSetAll || (mask & 0x1)) {
        var_s0 = attr->mode;
        if (attr->mode & 0x100) {
            var_s0 &= ~0x100;
            sp58 = 1;
        }
        if (var_s0 >= 0xA || _SpuIsInAllocateArea_(_spu_rev_startaddr[var_s0])) {
            return -1;
        }
        var_s4 = 1;
        _spu_rev_attr.mode = var_s0;
        _spu_rev_offsetaddr = _spu_rev_startaddr[_spu_rev_attr.mode];
        _memcpy((char *)&entry, (char *)&_spu_rev_param[_spu_rev_attr.mode],
                sizeof(RevParamEntry));
        switch (_spu_rev_attr.mode) {
        case 7: /* SPU_REV_MODE_ECHO */
            _spu_rev_attr.feedback = 0x7F;
            _spu_rev_attr.delay = 0x7F;
            break;
        case 8: /* SPU_REV_MODE_DELAY */
            _spu_rev_attr.feedback = 0;
            _spu_rev_attr.delay = 0x7F;
            break;
        default:
            _spu_rev_attr.feedback = 0;
            _spu_rev_attr.delay = 0;
            break;
        }
    }
    if (bSetAll || (mask & 0x8)) {
        switch (_spu_rev_attr.mode) {
        case 7: /* SPU_REV_MODE_ECHO */
        case 8: /* SPU_REV_MODE_DELAY */
            var_s6 = 1;
            if (!var_s4) {
                _memcpy((char *)&entry, (char *)&_spu_rev_param[_spu_rev_attr.mode],
                        sizeof(RevParamEntry));
                entry.flags = 0x0C011C00;
            }
            _spu_rev_attr.delay = attr->delay;
            entry.mLSAME = ((_spu_rev_attr.delay << 0xD) / 0x7F) - entry.dAPF1;
            entry.mRSAME = ((_spu_rev_attr.delay << 0xC) / 0x7F) - entry.dAPF2;
            entry.mLCOMB1 = ((_spu_rev_attr.delay << 0xC) / 0x7F) + entry.mRCOMB1;
            entry.dLSAME = ((_spu_rev_attr.delay << 0xC) / 0x7F) + entry.dRSAME;
            entry.mLAPF1 = ((_spu_rev_attr.delay << 0xC) / 0x7F) + entry.mLAPF2;
            entry.mRAPF1 = ((_spu_rev_attr.delay << 0xC) / 0x7F) + entry.mRAPF2;
            break;
        default:
            break;
        }
    }
    if (bSetAll || (mask & 0x10)) {
        switch (_spu_rev_attr.mode) {
        case 7: /* SPU_REV_MODE_ECHO */
        case 8: /* SPU_REV_MODE_DELAY */
            var_fp = 1;
            if (!var_s4) {
                if (!var_s6) {
                    _memcpy((char *)&entry, (char *)&_spu_rev_param[_spu_rev_attr.mode],
                            sizeof(RevParamEntry));
                    entry.flags = 0x80;
                } else {
                    entry.flags |= 0x80;
                }
            }
            _spu_rev_attr.feedback = attr->feedback;
            entry.vWALL = (_spu_rev_attr.feedback * 0x8100) / 0x7F;
            break;
        default:
            break;
        }
    }
    if (var_s4) {
        var_s7 = (*(volatile u16 *)(_spu_RXX + 0x1AA) >> 7) & 1;
        if (var_s7) {
            cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
            cnt &= ~0x80;
            *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
        }
    }
    if (!var_s4) {
        if (bSetAll || (mask & 0x2)) {
            *(u16 *)(_spu_RXX + 0x184) = attr->depth.left;
            _spu_rev_attr.depth.left = attr->depth.left;
        }
        if (bSetAll || (mask & 0x4)) {
            *(u16 *)(_spu_RXX + 0x186) = attr->depth.right;
            _spu_rev_attr.depth.right = attr->depth.right;
        }
    } else {
        *(u16 *)(_spu_RXX + 0x184) = 0;
        *(u16 *)(_spu_RXX + 0x186) = 0;
        _spu_rev_attr.depth.left = 0;
        _spu_rev_attr.depth.right = 0;
    }
    if (var_s4 || var_s6 || var_fp) {
        _spu_setReverbAttr((s32 *)&entry);
    }
    if (sp58) {
        SpuClearReverbWorkArea(_spu_rev_attr.mode);
    }
    if (var_s4) {
        _spu_FsetRXX(0xD1, _spu_rev_offsetaddr, 0);
        if (var_s7) {
            cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
            cnt |= 0x80;
            *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
        }
    }
    return 0;
}
