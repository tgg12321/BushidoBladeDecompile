/* LIBSPU code between SR_GAKS and S_N2P: func_8008B488. .text 0x8008B488..0x8008BA94. By link order
 * and size it is probably LIBSPU S_SVA (SpuSetVoiceAttr; memory/closer/libsnd-hunt-report.md,
 * PROBABLE), but no libscan xref or near-tier evidence identifies it, so the region stays one gap file
 * (owner rulings Q106 D3, Q109), named by its ROM offset. */
#include "common.h"
#include "libspu_internal.h"

/* func_8008B488: per-voice SPU attribute setter with the shape of PsyQ
 * LIBSPU's SpuSetVoiceAttr (C ref: sotn-decomp src/main/psxsdk/libspu/s_sva.c
 * and psyz decomp/src/libspu/sr_sv.c). BB2 links an older build: no min/max
 * voice range, a different block order, and the SR mode defaulting to 0x100.
 * The name stays auto (near-tier-ruling-2026-09-07: no verbatim caller pins
 * it). SpuVoiceAttr is PsyQ libspu.h's (include/psxsdk/libspu.h; sizeof =
 * 0x40, the callers' s32[16]). */

void func_8008B488(SpuVoiceAttr *attr) {
    /* FAKE: volatile locals admitted on SOTN precedent (owner rulings Q50
       route A, Q53) -- the closing settle loop (v = 1; 2 x v *= 13) runs on
       $sp slots, as in the target; plain locals score 35. SOTN's
       _SpuSetVoiceAttr ends with the same loop on the same volatile pair
       (src/main/psxsdk/libspu/s_sva.c:279-283). */
    volatile s32 i; /* SOTN: src/main/psxsdk/libspu/s_sva.c:14 @db41b28 */
    volatile s32 v; /* SOTN: src/main/psxsdk/libspu/s_sva.c:15 @db41b28 */
    s32 voice;
    s32 pos;
    u32 mask;
    s32 bSetAll;

    mask = attr->mask;
    bSetAll = mask == 0;
    for (voice = 0; voice < 24; voice++) {
        u16 temp; /* two values: the clamped sustain rate (SR block), then the
                   * clamped sustain level (SL block); Ruling 11 */

        if ((attr->voice & (1 << voice)) == 0) {
            continue;
        }
        pos = voice * 8;

        if (bSetAll || (mask & 0x10)) {
            *(volatile u16 *)(_spu_RXX + (pos + 2) * 2) = attr->pitch;
        }
        if (bSetAll || (mask & 0x40)) {
            _spu_voice_centerNote[voice] = attr->sample_note;
        }
        if (bSetAll || (mask & 0x20)) {
            u16 center;
            u16 note;

            center = _spu_voice_centerNote[voice];
            note = attr->note;
            *(volatile u16 *)(_spu_RXX + (pos + 2) * 2) =
                _spu_note2pitch(center >> 8, center & 0xFF, note >> 8, note & 0xFF);
        }
        if (bSetAll || (mask & 0x1)) {
            u16 volmode_left;
            u16 vol_left;

            vol_left = attr->volume.left & 0x7FFF;
            volmode_left = 0;
            if (bSetAll || (mask & 0x4)) {
                switch (attr->volmode.left) {
                case 1:
                    volmode_left = 0x8000;
                    break;
                case 2:
                    volmode_left = 0x9000;
                    break;
                case 3:
                    volmode_left = 0xA000;
                    break;
                case 4:
                    volmode_left = 0xB000;
                    break;
                case 5:
                    volmode_left = 0xC000;
                    break;
                case 6:
                    volmode_left = 0xD000;
                    break;
                case 7:
                    volmode_left = 0xE000;
                    break;
                }
            }
            if (volmode_left != 0) {
                if (attr->volume.left >= 0x80) {
                    vol_left = 0x7F;
                } else if (attr->volume.left < 0) {
                    vol_left = 0;
                }
            }
            *(volatile u16 *)(_spu_RXX + pos * 2) = vol_left | volmode_left;
        }
        if (bSetAll || (mask & 0x2)) {
            u16 volmode_right;
            u16 vol_right;

            vol_right = attr->volume.right & 0x7FFF;
            volmode_right = 0;
            if (bSetAll || (mask & 0x8)) {
                switch (attr->volmode.right) {
                case 1:
                    volmode_right = 0x8000;
                    break;
                case 2:
                    volmode_right = 0x9000;
                    break;
                case 3:
                    volmode_right = 0xA000;
                    break;
                case 4:
                    volmode_right = 0xB000;
                    break;
                case 5:
                    volmode_right = 0xC000;
                    break;
                case 6:
                    volmode_right = 0xD000;
                    break;
                case 7:
                    volmode_right = 0xE000;
                    break;
                }
            }
            if (volmode_right != 0) {
                if (attr->volume.right >= 0x80) {
                    vol_right = 0x7F;
                } else if (attr->volume.right < 0) {
                    vol_right = 0;
                }
            }
            *(volatile u16 *)(_spu_RXX + (pos + 1) * 2) = vol_right | volmode_right;
        }
        if (bSetAll || (mask & 0x80)) {
            _spu_FsetRXXa(pos | 3, attr->addr);
        }
        if (bSetAll || (mask & 0x10000)) {
            _spu_FsetRXXa(pos | 7, attr->loop_addr);
        }
        if (bSetAll || (mask & 0x20000)) {
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = attr->adsr1;
        }
        if (bSetAll || (mask & 0x40000)) {
            *(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = attr->adsr2;
        }
        if (bSetAll || (mask & 0x800)) {
            u16 ar_rate;
            s32 amode;
            s32 adsr;

            ar_rate = attr->ar;
            if (ar_rate >= 0x80) {
                ar_rate = 0x7F;
            }
            amode = 0;
            if (bSetAll || (mask & 0x100)) {
                if (attr->a_mode == 5) {
                    amode = 0x80;
                }
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            adsr &= 0xFF;
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = adsr | ((ar_rate | amode) << 8);
        }
        if (bSetAll || (mask & 0x1000)) {
            u16 dr_rate;
            s32 adsr;

            dr_rate = attr->dr;
            if (dr_rate >= 0x10) {
                dr_rate = 0xF;
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            adsr &= 0xFF0F;
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = adsr | (dr_rate << 4);
        }
        if (bSetAll || (mask & 0x2000)) {
            s32 smode;
            s32 adsr;

            temp = attr->sr;
            if (temp >= 0x80) {
                temp = 0x7F;
            }
            smode = 0x100;
            if (bSetAll || (mask & 0x200)) {
                switch (attr->s_mode) {
                case 1:
                    smode = 0;
                    break;
                case 5:
                    smode = 0x200;
                    break;
                case 7:
                    smode = 0x300;
                    break;
                }
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 5) * 2);
            adsr &= 0x3F;
            *(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | ((temp | smode) << 6);
        }
        if (bSetAll || (mask & 0x4000)) {
            u16 rr_rate;
            s32 rmode;
            s32 adsr;

            rr_rate = attr->rr;
            if (rr_rate >= 0x20) {
                rr_rate = 0x1F;
            }
            rmode = 0;
            if (bSetAll || (mask & 0x400)) {
                switch (attr->r_mode) {
                case 3:
                    break;
                case 7:
                    rmode = 0x20;
                    break;
                }
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 5) * 2);
            adsr &= 0xFFC0;
            *(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (rr_rate | rmode);
        }
        if (bSetAll || (mask & 0x8000)) {
            s32 adsr;

            temp = attr->sl;
            if (temp >= 0x10) {
                temp = 0xF;
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = (adsr & 0xFFF0) | temp;
        }
    }
    v = 1;
    for (i = 0; i < 2; i++) {
        v *= 13;
    }
}
