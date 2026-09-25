void func_8008B488(SpuVoiceAttr *attr) {
    volatile s32 sp10;
    volatile s32 sp14;
    s32 voice;
    s32 pos;
    u32 mask;
    s32 bSetAll;
    u16 adsr1_rate;
    u16 adsr2_rate;

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

            adsr1_rate = attr->ar;
            if (adsr1_rate >= 0x80) {
                adsr1_rate = 0x7F;
            }
            amode = 0;
            if (bSetAll || (mask & 0x100)) {
                if (attr->a_mode == 5) {
                    amode = 0x80;
                }
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            adsr &= 0xFF;
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = adsr | ((adsr1_rate | amode) << 8);
        }
        if (bSetAll || (mask & 0x1000)) {
            s32 adsr;

            adsr1_rate = attr->dr;
            if (adsr1_rate >= 0x10) {
                adsr1_rate = 0xF;
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            adsr &= 0xFF0F;
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = adsr | (adsr1_rate << 4);
        }
        if (bSetAll || (mask & 0x2000)) {
            s32 smode;
            s32 adsr;

            adsr2_rate = attr->sr;
            if (adsr2_rate >= 0x80) {
                adsr2_rate = 0x7F;
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
            *(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | ((adsr2_rate | smode) << 6);
        }
        if (bSetAll || (mask & 0x4000)) {
            s32 rmode;
            s32 adsr;

            adsr2_rate = attr->rr;
            if (adsr2_rate >= 0x20) {
                adsr2_rate = 0x1F;
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
            *(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (adsr2_rate | rmode);
        }
        if (bSetAll || (mask & 0x8000)) {
            s32 adsr;

            adsr1_rate = attr->sl;
            if (adsr1_rate >= 0x10) {
                adsr1_rate = 0xF;
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = (adsr & 0xFFF0) | adsr1_rate;
        }
    }
    sp14 = 1;
    for (sp10 = 0; sp10 < 2; sp10++) {
        sp14 *= 13;
    }
}
