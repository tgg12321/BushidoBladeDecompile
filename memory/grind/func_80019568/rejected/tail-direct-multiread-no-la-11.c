void func_80019568(s32 arg0) {
    struct {
        s16 output[4];
        s32 voice_mask;
        s32 unk_1C;
        s32 unk_20;
        s32 unk_24;
        s32 packets[4];
    } sp;
    u8 *packets;
    s16 *output;
    s32 i;
    s32 voice_mask;
    s32 old_mask;
    s16 *base_addr;
    s16 *dst1;
    s16 *dst0;
    s16 *src;

    voice_mask = 0;
    i = 0;
    packets = (u8 *)&sp.packets[0];
    sp.packets[0] = D_800FF580;
    sp.packets[1] = D_800FF584;
    sp.packets[2] = D_800FF5A4;
    sp.packets[3] = D_800FF5A8;
    do {
        u8 *p = &packets[i * 8];
        s16 *o = &sp.output[i];
        s32 bits;

        s32 enable;

        if (p[0] == 0) {
            s32 voice2;

            o[0] = p[1] >> 4;
            enable = 1;
            o[2] = enable;
            voice2 = (s16)((u16)o[0] - 1);

            if ((u32)voice2 < 8) {
                switch (voice2) {
                case 4:
                case 6:
                    o[0] = 4;
                case 1:
                case 2:
                case 3:
                    bits = ~((p[2] << 8) | p[3]);
                    break;
                case 0:
                case 5:
                case 7:
                default:
                    bits = 0;
                    break;
                }
            } else {
                bits = 0;
            }
        } else {
            o[0] = 4;
            enable = 0;
            o[2] = enable;
            bits = 0;
        }

        voice_mask = ((u32)voice_mask >> 16) | (bits << 16);
        i++;
    } while (i < 2);

    sp.voice_mask = voice_mask;
    func_8001B138(&sp.voice_mask);

    if (D_800A3834 == 1 && arg0 == 0) {
        s32 voice_state = D_800A38DC;

        if ((u32)voice_state < 7) {
            switch (voice_state) {
            case 4:
            case 5:
                if (D_8010278E == 0) {
                    sp.voice_mask |= 0x08000800;
                }
            case 0:
            case 1:
            case 2:
            case 3:
            case 6:
                if (D_8010278C == 0) {
                    sp.voice_mask |= 0x08000800;
                }
                break;
            }
        }
    }

    func_8003A728((s32)&sp.output[0]);

    i = 0;
    base_addr = &D_80102788;
    dst1 = base_addr + 2;
    dst0 = base_addr;
    src = &sp.output[0];

    do {
        dst0[0] = src[0];
        dst0++;
        dst1[0] = src[2];
        src++;
        i++;
        dst1++;
    } while (i < 2);

    D_80102794 = sp.voice_mask & ~D_80102790;
    D_80102798 = ~sp.voice_mask & D_80102790;
    D_8010279C = ~sp.voice_mask;
    D_80102790 = sp.voice_mask;
}
