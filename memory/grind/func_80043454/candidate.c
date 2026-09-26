extern s16 D_80095588[];
extern void gpu_OffsetTexPolyFT3();
extern void gpu_OffsetTexPolyFT4();
extern void gpu_OffsetTexPolyGT3();
extern void gpu_OffsetTexPolyGT4();
void func_80043E98(s16 *a0, s16 a1, s16 a2, s16 a3, s16 a4);
void func_80043F0C(s16 *a0, s16 a1, s16 a2, s16 a3, s16 a4);
#define SCRATCH_PTR (*(u16 **)0x1F800000)
void func_80043454(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    s32 count;
    s32 i;
    s32 mode;
    s32 type;
    s32 kind;
    s32 packed;
    s32 blocks;
    u16 *p;
    u8 *b;

    blocks = 0;
    do {
        p = SCRATCH_PTR;
        SCRATCH_PTR = p + 1;
        count = *p;
        if (count & 0x8000) {
            packed = 1;
            mode = 2;
            SCRATCH_PTR = SCRATCH_PTR + (count & 0x7FFF) + 1;
            if ((u32)SCRATCH_PTR & 3) {
                SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + (4 - ((u32)SCRATCH_PTR & 3)));
            }
        } else {
            SCRATCH_PTR = p + 2;
            mode = p[1];
            packed = 0;
            if (mode == 0) {
                SCRATCH_PTR += count * 12;
            } else {
                SCRATCH_PTR += count * 3;
            }
        }
        if (mode == 1) {
            if ((u32)SCRATCH_PTR & 3) {
                SCRATCH_PTR++;
            }
        }
        while ((count = *SCRATCH_PTR++) != 0) {
            type = *SCRATCH_PTR++;
            if ((type & 4) || (u32)(type - 0x26) < 4) {
                if ((u32)(type - 0x26) < 4) {
                    kind = type - 0x26;
                } else {
                    kind = (type >> 3) & 3;
                }
                switch (mode) {
                case 0:
                    i = count;
                    while (--i != -1) {
                        switch (kind) {
                        case 0:
                            gpu_OffsetTexPolyFT3(SCRATCH_PTR, arg0, arg1, arg2, arg3);
                            gpu_OffsetTexPolyFT3((u8 *)SCRATCH_PTR + 0x20, arg0, arg1, arg2, arg3);
                            break;
                        case 1:
                            gpu_OffsetTexPolyFT4(SCRATCH_PTR, arg0, arg1, arg2, arg3);
                            gpu_OffsetTexPolyFT4((u8 *)SCRATCH_PTR + 0x28, arg0, arg1, arg2, arg3);
                            break;
                        case 2:
                            gpu_OffsetTexPolyGT3(SCRATCH_PTR, arg0, arg1, arg2, arg3);
                            gpu_OffsetTexPolyGT3((u8 *)SCRATCH_PTR + 0x28, arg0, arg1, arg2, arg3);
                            break;
                        case 3:
                            gpu_OffsetTexPolyGT4(SCRATCH_PTR, arg0, arg1, arg2, arg3);
                            gpu_OffsetTexPolyGT4((u8 *)SCRATCH_PTR + 0x34, arg0, arg1, arg2, arg3);
                            break;
                        }
                        SCRATCH_PTR += D_80095588[type];
                    }
                    break;
                case 1:
                    i = count;
                    while (--i != -1) {
                        func_80043E98((s16 *)SCRATCH_PTR, arg0, arg1, arg2, arg3);
                        b = (u8 *)SCRATCH_PTR;
                        switch (kind) {
                        case 0:
                            b[7] += arg1;
                            b[9] += arg1;
                            b[11] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x18);
                            break;
                        case 1:
                            b[7] += arg1;
                            b[9] += arg1;
                            b[11] += arg1;
                            b[13] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x18);
                            break;
                        case 2:
                            b[7] += arg1;
                            b[9] += arg1;
                            b[11] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x24);
                            break;
                        case 3:
                            b[7] += arg1;
                            b[9] += arg1;
                            b[11] += arg1;
                            b[13] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x2C);
                            break;
                        default:
                            func_80052C10();
                            break;
                        }
                    }
                    break;
                case 2:
                    i = count;
                    while (--i != -1) {
                        func_80043F0C((s16 *)SCRATCH_PTR, arg0, arg1, arg2, arg3);
                        b = (u8 *)SCRATCH_PTR;
                        switch (kind) {
                        case 0:
                        case 2:
                            b[1] += arg1;
                            b[5] += arg1;
                            b[9] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x14);
                            break;
                        case 1:
                        case 3:
                            b[1] += arg1;
                            b[5] += arg1;
                            b[9] += arg1;
                            b[13] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x18);
                            break;
                        default:
                            func_80052C10();
                            break;
                        }
                    }
                    break;
                }
            } else if (!packed) {
                SCRATCH_PTR += (s16)(count * D_80095588[type]);
            } else {
                switch (type) {
                case 0:
                    SCRATCH_PTR += count * 26;
                    break;
                case 8:
                    SCRATCH_PTR += count * 32;
                    break;
                default:
                    SCRATCH_PTR += (s16)(count * D_80095588[type]);
                    break;
                }
            }
        }
        if (packed) {
            if (blocks == 12) {
                return;
            }
            blocks++;
            while (*SCRATCH_PTR != 0xFFFF) {
                SCRATCH_PTR += 3;
            }
            SCRATCH_PTR++;
        }
    } while (*SCRATCH_PTR++ != 0);
}
