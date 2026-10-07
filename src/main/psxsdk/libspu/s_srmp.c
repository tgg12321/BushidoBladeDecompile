/* PsyQ 4.0 LIBSPU S_SRMP: SpuSetReverbModeParam. .text 0x80089F3C..0x8008A434,
 * a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3.
 */
#include "common.h"
#include "libspu_internal.h"

/* C ref: sotn-decomp src/main/psxsdk/libspu/s_srmp.c. 4.0 deltas: the
   DELAYTIME/FEEDBACK gates are range compares (ECHO..DELAY) with no default-arm
   clears, and the depth/zero split keys off var_s4. Preset table:
   _spu_rev_param (10 entries x 0x44). */
extern RevParamEntry _spu_rev_param[]; /* rev_param preset table */

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
        if (var_s0 >= 0xA ||
            _SpuIsInAllocateArea_(_spu_rev_startaddr[var_s0])) {
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
                _memcpy((char *)&entry,
                        (char *)&_spu_rev_param[_spu_rev_attr.mode],
                        sizeof(RevParamEntry));
                entry.flags = 0x0C011C00;
            }
            _spu_rev_attr.delay = attr->delay;
            entry.mLSAME = ((_spu_rev_attr.delay << 0xD) / 0x7F) - entry.dAPF1;
            entry.mRSAME = ((_spu_rev_attr.delay << 0xC) / 0x7F) - entry.dAPF2;
            entry.mLCOMB1 =
                ((_spu_rev_attr.delay << 0xC) / 0x7F) + entry.mRCOMB1;
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
                    _memcpy((char *)&entry,
                            (char *)&_spu_rev_param[_spu_rev_attr.mode],
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
        var_s7 = (_spu_RXX->rxx.spucnt >> 7) & 1;
        if (var_s7) {
            cnt = _spu_RXX->rxx.spucnt;
            cnt &= ~0x80;
            _spu_RXX->rxx.spucnt = cnt;
        }
    }
    if (!var_s4) {
        if (bSetAll || (mask & 0x2)) {
            _spu_RXX->rxx.rev_vol.left = attr->depth.left;
            _spu_rev_attr.depth.left = attr->depth.left;
        }
        if (bSetAll || (mask & 0x4)) {
            _spu_RXX->rxx.rev_vol.right = attr->depth.right;
            _spu_rev_attr.depth.right = attr->depth.right;
        }
    } else {
        _spu_RXX->rxx.rev_vol.left = 0;
        _spu_RXX->rxx.rev_vol.right = 0;
        _spu_rev_attr.depth.left = 0;
        _spu_rev_attr.depth.right = 0;
    }
    if (var_s4 || var_s6 || var_fp) {
        _spu_setReverbAttr(&entry);
    }
    if (sp58) {
        SpuClearReverbWorkArea(_spu_rev_attr.mode);
    }
    if (var_s4) {
        _spu_FsetRXX(0xD1, _spu_rev_offsetaddr, 0);
        if (var_s7) {
            cnt = _spu_RXX->rxx.spucnt;
            cnt |= 0x80;
            _spu_RXX->rxx.spucnt = cnt;
        }
    }
    return 0;
}
