typedef struct { Vec3i j[22]; } ProbeScr;               /* scratchpad per-char block, stride 0x108 */
typedef struct {
    u8 pad0[0x174];
    Vec3i mid;                                            /* +0x174 */
    u8 pad1[0x18C - 0x180];
    Vec3i cen;                                            /* +0x18C */
    u8 pad2[0x44C - 0x198];
} ProbeRec;                                               /* stride 0x44C */
void func_8002C61C(void) {
    u8 *s1 = (u8 *)&D_80101EC8;
    u8 *s0 = s1 + 0x44C;
    s32 i;
    u16 mode;

    mode = D_80101F32;

    if (mode == 0xF || (u32)(mode - 0x1C) < 2 ||
        (u32)(mode - 0x1E) < 2 || (u32)(mode - 0x20) < 2) {
        func_80026DA4();
    } else if (mode == 0x11) {
        func_8002C0DC();
    } else {
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
        Vec3i *dst = (Vec3i *)&D_801020D8;
        Vec3i *src = (Vec3i *)0x1F800000;
        for (i = 0; i < 3; i++) {
            dst[i] = src[i];
            ((Vec3i *)((u8 *)dst + 0x44C))[i] = src[i + 3];
        }
    }

    {
        Vec3i *dst = (Vec3i *)&D_801020FC;
        Vec3i *src = (Vec3i *)0x1F800048;
        for (i = 0; i < 2; i++) {
            dst[i] = src[i];
            ((Vec3i *)((u8 *)dst + 0x44C))[i] = src[i + 2];
        }
    }

    {
        ProbeScr *sp = (ProbeScr *)0x1F800078;
        ProbeRec *ch = (ProbeRec *)&D_80101EC8;
        for (i = 0; i < 2; i++) {
            ch[i].cen.x = (sp[i].j[5].x + sp[i].j[6].x + sp[i].j[7].x) / 3;
            ch[i].cen.y = (sp[i].j[5].y + sp[i].j[6].y + sp[i].j[7].y) / 3;
            ch[i].cen.z = (sp[i].j[5].z + sp[i].j[6].z + sp[i].j[7].z) / 3;
            ch[i].mid.x = (sp[i].j[8].x + sp[i].j[9].x) / 2;
            ch[i].mid.y = (sp[i].j[8].y + sp[i].j[9].y) / 2;
            ch[i].mid.z = (sp[i].j[8].z + sp[i].j[9].z) / 2;
        }
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
