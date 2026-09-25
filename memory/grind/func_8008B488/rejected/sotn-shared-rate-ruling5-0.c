/* CLOSING FORM (not landable: Ruling 5) - func_8008B488 (src/main.c)  sandbox --disable all = 0/387,
 * full-build SHA1 == oracle with the landing chassis below (manual s1, 2026-09-25).
 * NOT LANDABLE YET: the function-scope `u16 rate` is written in five sibling
 * ADSR blocks (AR/DR/SR/RR/SL), different fields, plus clamp constants.
 * That is SOTN's verbatim `var_a2` reuse in the same Sony function
 * (sotn-decomp src/main/psxsdk/libspu/s_sva.c), but it fails Ruling 5
 * 1(a)/1(b)/1(e), and Ruling 8 covers vmNoiseOn only. Owner question pending:
 * docs/grind/borderline.md 2026-09-25 func_8008B488. See evidence.md.
 *
 * LANDING CHASSIS (all three required; the patch is in evidence.md):
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
    s32 amode;
    s32 smode;
    s32 rmode;
    u16 volmode_left;
    u16 volmode_right;
    u16 center;
    u16 note;
    u16 rate;
    u16 vol_right;
    u16 vol_left;
    s32 adsr;

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
            center = D_800A28A4[voice];
            note = attr->note;
            *(volatile u16 *)(_spu_RXX + (pos + 2) * 2) =
                _spu_note2pitch(center >> 8, center & 0xFF, note >> 8, note & 0xFF);
        }
        if (bSetAll || (mask & 0x1)) {
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
            rate = attr->ar;
            if (rate >= 0x80) {
                rate = 0x7F;
            }
            amode = 0;
            if (bSetAll || (mask & 0x100)) {
                if (attr->a_mode == 5) {
                    amode = 0x80;
                }
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            adsr &= 0xFF;
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = adsr | ((rate | amode) << 8);
        }
        if (bSetAll || (mask & 0x1000)) {
            rate = attr->dr;
            if (rate >= 0x10) {
                rate = 0xF;
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            adsr &= 0xFF0F;
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = adsr | (rate << 4);
        }
        if (bSetAll || (mask & 0x2000)) {
            rate = attr->sr;
            if (rate >= 0x80) {
                rate = 0x7F;
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
            *(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | ((rate | smode) << 6);
        }
        if (bSetAll || (mask & 0x4000)) {
            rate = attr->rr;
            if (rate >= 0x20) {
                rate = 0x1F;
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
            *(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (rate | rmode);
        }
        if (bSetAll || (mask & 0x8000)) {
            rate = attr->sl;
            if (rate >= 0x10) {
                rate = 0xF;
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = (adsr & 0xFFF0) | rate;
        }
    }
    sp14 = 1;
    for (sp10 = 0; sp10 < 2; sp10++) {
        sp14 *= 13;
    }
}
