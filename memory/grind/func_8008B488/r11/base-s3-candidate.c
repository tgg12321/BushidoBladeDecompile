/* CANDIDATE - func_8008B488 (src/main.c), best LANDABLE form (manual s1, 2026-09-25).
 * One once-per-block rate local per ADSR block. sandbox --disable all = 12/387
 * with the landing chassis, all operand-only (SR rate/smode seats, SL rate a0 vs a1).
 * The closing form (0/387, full-build SHA1 == oracle) is
 * rejected/sotn-shared-rate-ruling5-0.c: SOTN's shared `u16 rate`, ruled
 * INADMISSIBLE by layer-2 on 2026-09-25 (settled, not an owner question).
 * s2 allocator dump: no one-pseudo-per-block form can close SL or SR (evidence.md).
 *
 * LANDING CHASSIS (needed by either form; see evidence.md):
 *  1. src/main.c: replace INCLUDE_ASM with this file (typedef + extern + body);
 *     DELETE the hand-transcribed const jtbl_80016460[8] / jtbl_80016480[7]
 *     at the end of main.c (the compiler now emits both ADDR_VECs).
 *  2. Makefile:136: drop `main` from RODATA_ALIGN2_FILES.
 *  3. engine/buildconfig.py RODATA_ALIGN2_FILES: drop "main" (mirror).
 */
typedef struct {
    /* 0x00 */ u32 voice;
    /* 0x04 */ u32 mask;
    /* 0x08 */ SpuVolume volume;
    /* 0x0C */ SpuVolume volmode;
    /* 0x10 */ SpuVolume volumex;
    /* 0x14 */ u16 pitch;
    /* 0x16 */ u16 note;
    /* 0x18 */ u16 sample_note;
    /* 0x1A */ s16 envx;
    /* 0x1C */ u32 addr;
    /* 0x20 */ u32 loop_addr;
    /* 0x24 */ s32 a_mode;
    /* 0x28 */ s32 s_mode;
    /* 0x2C */ s32 r_mode;
    /* 0x30 */ u16 ar;
    /* 0x32 */ u16 dr;
    /* 0x34 */ u16 sr;
    /* 0x36 */ u16 rr;
    /* 0x38 */ u16 sl;
    /* 0x3A */ u16 adsr1;
    /* 0x3C */ u16 adsr2;
} SpuVoiceAttr;

extern u16 D_800A28A4[];

void func_8008B488(SpuVoiceAttr *attr) {
    volatile s32 sp10;
    volatile s32 sp14;
    s32 voice;
    s32 pos;
    u32 mask;
    s32 bSetAll;
    u16 ar_rate;
    u16 dr_rate;
    u16 sr_rate;
    u16 rr_rate;
    u16 sl_rate;

    mask = attr->mask;
    bSetAll = mask == 0;
    for (voice = 0; voice < 24; voice++) {
        if ((attr->voice & (1 << voice)) == 0) {
            continue;
        }
        pos = voice * 8;

        if (bSetAll || (mask & 0x10)) {
            *(volatile u16 *)(_spu_RXX + (pos + 2) * 2) = attr->pitch;
        }
        if (bSetAll || (mask & 0x40)) {
            D_800A28A4[voice] = attr->sample_note;
        }
        if (bSetAll || (mask & 0x20)) {
            u16 center;
            u16 note;

            center = D_800A28A4[voice];
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
                switch ((s16)attr->volmode.left) {
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
                switch ((s16)attr->volmode.right) {
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

            sr_rate = attr->sr;
            if (sr_rate >= 0x80) {
                sr_rate = 0x7F;
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
            *(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | ((sr_rate | smode) << 6);
        }
        if (bSetAll || (mask & 0x4000)) {
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

            sl_rate = attr->sl;
            if (sl_rate >= 0x10) {
                sl_rate = 0xF;
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = (adsr & 0xFFF0) | sl_rate;
        }
    }
    sp14 = 1;
    for (sp10 = 0; sp10 < 2; sp10++) {
        sp14 *= 13;
    }
}
