/* func_8002C61C candidate - s1b (recon, 2026-09-06). Loop-3 destinations spelled against the
   record base D_80101EC8 + off + field offset per the 2026-09-06 06:44 Judge ruling (off = i * 0x44C
   is the record stride; +0x174 midpoint, +0x18C centroid). */
typedef struct { Vec3i j[22]; } ProbeScr;               /* scratchpad per-char block, stride 0x108 */
#define SCR ((ProbeScr *)0x1F800078)
void func_8002C61C(void) {
    u8 *s1 = (u8 *)&D_80101EC8;
    u8 *s0 = s1 + 0x44C;
    s32 i;
    u16 mode;

    mode = D_80101F32;

    if (mode == 0xF || mode == 0x1C || mode == 0x1D || mode == 0x1E ||
        mode == 0x1F || mode == 0x20 || mode == 0x21) {
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
        Vec3i *dst_a = (Vec3i *)&D_801020D8;
        Vec3i *dst_b = (Vec3i *)((u8 *)&D_801020D8 + 0x44C);
        Vec3i *src = (Vec3i *)0x1F800000;
        for (i = 0; i < 3; i++) {
            dst_a[i] = src[i];
            dst_b[i] = src[i + 3];
        }
    }

    {
        Vec3i *dst_a = (Vec3i *)&D_801020FC;
        Vec3i *dst_b = (Vec3i *)((u8 *)&D_801020FC + 0x44C);
        Vec3i *src = (Vec3i *)0x1F800000;
        for (i = 0; i < 2; i++) {
            dst_a[i] = src[i + 6];
            dst_b[i] = src[i + 8];
        }
    }

    {
        s32 off;
        for (i = 0; i < 2; i++) {
            off = i * 0x44C;
            *(s32 *)((u8 *)&D_80101EC8 + off + 0x18C) = (SCR[i].j[5].x + SCR[i].j[6].x + SCR[i].j[7].x) / 3;
            *(s32 *)((u8 *)&D_80101EC8 + off + 0x190) = (SCR[i].j[5].y + SCR[i].j[6].y + SCR[i].j[7].y) / 3;
            *(s32 *)((u8 *)&D_80101EC8 + off + 0x194) = (SCR[i].j[5].z + SCR[i].j[6].z + SCR[i].j[7].z) / 3;
            *(s32 *)((u8 *)&D_80101EC8 + off + 0x174) = (SCR[i].j[8].x + SCR[i].j[9].x) / 2;
            *(s32 *)((u8 *)&D_80101EC8 + off + 0x178) = (SCR[i].j[8].y + SCR[i].j[9].y) / 2;
            *(s32 *)((u8 *)&D_80101EC8 + off + 0x17C) = (SCR[i].j[8].z + SCR[i].j[9].z) / 2;
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
