extern s16 D_80095588[];
extern void gpu_OffsetTexPolyFT3();
extern void gpu_OffsetTexPolyFT4();
extern void gpu_OffsetTexPolyGT3();
extern void gpu_OffsetTexPolyGT4();
void func_80043E98(s16 *a0, s16 a1, s16 a2, s16 a3, s16 a4);
void func_80043F0C(s16 *a0, s16 a1, s16 a2, s16 a3, s16 a4);
/* Read cursor into the primitive packet stream, kept in scratchpad word 0. */
#define SCRATCH_PTR (*(u16 **)0x1F800000)
/* Walk a packet stream of primitive groups (the cursor starts at the
 * scratchpad word) and shift every textured primitive's texture source by
 * (arg0, arg1) and its CLUT by (arg2, arg3): via gpu_OffsetTexPoly* for
 * mode-0 groups, via func_80043E98 / func_80043F0C plus a per-vertex v shift
 * for mode-1 / mode-2 groups.  Untextured groups are skipped using the
 * per-type halfword sizes in D_80095588. */
void func_80043454(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    s32 count;
    s32 mode;
    s32 type;
    s32 kind;
    s32 packed;
    s32 blocks;
    u16 *p;
    u8 *b;
    s32 c3;
    u8 d1;

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
                    while (--count != -1) {
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
                    /* The mode-1/2 counted loops are backward-goto loops. As
                     * while/do loops the same code allocates count/kind to
                     * swapped registers (ledger: memory/grind/func_80043454). */
                    if (--count != -1) {
                        c3 = arg3;
                        d1 = arg1;
                    loop1:
                        func_80043E98((s16 *)SCRATCH_PTR, arg0, arg1, arg2, c3);
                        b = (u8 *)SCRATCH_PTR;
                        switch (kind) {
                        case 0:
                            b[7] += d1;
                            b[9] += d1;
                            b[11] += d1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x18);
                            break;
                        case 1:
                            b[7] += d1;
                            b[9] += d1;
                            b[11] += d1;
                            b[13] += d1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x18);
                            break;
                        case 2:
                            b[7] += d1;
                            b[9] += d1;
                            b[11] += d1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x24);
                            break;
                        case 3:
                            b[7] += d1;
                            b[9] += d1;
                            b[11] += d1;
                            b[13] += d1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x2C);
                            break;
                        default:
                            func_80052C10();
                            break;
                        }
                    if (--count != -1) goto loop1;
                    }
                    break;
                case 2:
                    if (--count != -1) {
                        c3 = arg3;
                        d1 = arg1;
                    loop2:
                        func_80043F0C((s16 *)SCRATCH_PTR, arg0, arg1, arg2, c3);
                        b = (u8 *)SCRATCH_PTR;
                        switch (kind) {
                        case 0:
                        case 2:
                            b[1] += d1;
                            b[5] += d1;
                            b[9] += d1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x14);
                            break;
                        case 1:
                        case 3:
                            b[1] += d1;
                            b[5] += d1;
                            b[9] += d1;
                            b[13] += d1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x18);
                            break;
                        default:
                            func_80052C10();
                            break;
                        }
                    if (--count != -1) goto loop2;
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
