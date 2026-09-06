void func_8002C61C(void) {
    u8 *s1 = (u8 *)&D_80101EC8;
    u8 *s0 = s1 + 0x44C;
    s32 i;

    switch (D_80101F32) {
    case 0xF:
    case 0x1C:
    case 0x1D:
    case 0x1E:
    case 0x1F:
    case 0x20:
    case 0x21:
        func_80026DA4();
        break;
    case 0x11:
        func_8002C0DC();
        break;
    default:
        func_8002872C();
        func_800288C8();
        D_800A3824 = func_80029454();
        if (D_800A3824 < 0) goto do_calc;
        func_8002C22C();
        if (D_800A3824 < 0) goto do_calc;
        if (D_80101F75 != 0 || D_801023C1 != 0) {
            func_800283D0(s1, (u8 *)0x1F8003F4);
            func_800283D0(s0, (u8 *)0x1F8003F4);
            D_801023C1 = 0;
            D_80101F75 = 0;
            goto after_calc;
        }
    do_calc:
        func_8002AB08(0);
    after_calc:

        if (*(s32 *)(s1 + 0x3C) >= 3 && *(s32 *)(s0 + 0x3C) >= 3 &&
            D_800A38A8 != 0 && *(s16 *)(s1 + 0x286) == -1 &&
            *(s16 *)(s0 + 0x286) == -1 && *(s16 *)(s1 + 0xC) != 0x1F &&
            *(s16 *)(s0 + 0xC) != 0x1F) {
            s32 diff = *(s32 *)(s1 + 0xF8) - *(s32 *)(s0 + 0xF8);
            if (diff < 0) diff = -diff;
            if (diff < 0x3E8) {
                *(s16 *)(s1 + 0x286) = 0xA;
                *(s16 *)(s0 + 0x286) = 0xA;
                *(s32 *)(s0 + 0x28C) = 0;
                *(s32 *)(s1 + 0x28C) = 0;
                D_800A3910 = 0;
                D_800A389C = 0;
            }
        }

    }

    if (D_80101F32 == 5) {
        D_800A3748 = 1;
        D_800A3834 = 0x1C;
    } else if (D_8010237E == 5) {
        D_800A3748 = 0;
        D_800A3834 = 0x1C;
    }

    {
        Vec3i *dst_a = (Vec3i *)&D_801020D8;
        Vec3i *dst_b = (Vec3i *)((u8 *)&D_801020D8 + 0x44C);
        Vec3i *sp_src = (Vec3i *)0x1F800000;
        i = 0;
        do {
            *dst_a = sp_src[0];
            *dst_b = sp_src[3];
            dst_b++;
            dst_a++;
            sp_src++;
            i++;
        } while (i < 3);
    }

    {
        Vec3i *dst_a = (Vec3i *)&D_801020FC;
        Vec3i *dst_b = (Vec3i *)((u8 *)&D_801020FC + 0x44C);
        Vec3i *sp_src = (Vec3i *)0x1F800000;
        i = 0;
        do {
            *dst_a = sp_src[6];
            *dst_b = sp_src[8];
            dst_b++;
            dst_a++;
            sp_src++;
            i++;
        } while (i < 2);
    }

    {
        s32 *a1 = (s32 *)0x1F8000EC;
        s32 a2_off = 0;
        i = 0;
        do {
            *(s32 *)((u8 *)&D_80102054 + a2_off) = (a1[-14] + a1[-11] + a1[-8]) / 3;
            *(s32 *)((u8 *)&D_80102058 + a2_off) = (a1[-13] + a1[-10] + a1[-7]) / 3;
            *(s32 *)((u8 *)&D_8010205C + a2_off) = (a1[-12] + a1[-9] + a1[-6]) / 3;
            *(s32 *)((u8 *)&D_8010203C + a2_off) = (a1[-5] + a1[-2]) / 2;
            i++;
            *(s32 *)((u8 *)&D_80102040 + a2_off) = (a1[-4] + a1[-1]) / 2;
            *(s32 *)((u8 *)&D_80102044 + a2_off) = (a1[-3] + a1[0]) / 2;
            a1 += 66;
            a2_off += 0x44C;
        } while (i < 2);
    }

    {
        s16 saved = *(s16 *)(s1 + 0x286);
        if (saved == -1) {
            func_80031B24();
            if (*(s16 *)(s1 + 0x286) == saved) {
                func_80032314();
            }
        }
    }
}
