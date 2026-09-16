/* REJECTED -- func_8006CCC8 -- s7 (solver), 2026-09-16
 * WHY DEAD: score 29 (184/189). Replaced the shift counter with a walking mask (`nib <<= 4` instead of `fade += 4`, `& nib` instead of `& (nib << fade)`). Target has `sllv $a2,$s6,$s5` inside each j-loop iteration with $s6 = 0xF constant and $s5 = 0/4, so the original really shifted a constant mask by a counter; the walking-mask spelling loses 5 insns and 29 points.
 * measured_on: src/text1b.c working tree, s7 switch + ternary-mask chassis (candidate.c = 0), no FAKE construct present.
 */
s32 func_8006CCC8(s32 *arg0, s32 *arg1, s16 arg2) {
    s32 i;
    s16 lim;
    s32 shift;
    s32 nib;
    s32 j;
    u8 *rec;
    s32 ret;
    s32 masked;
    s16 field;

    ret = 0;
    if ((*(s32 *)((u8 *)D_800A34FC + 0x28) == 0x50005) && (*arg1 & 0x400040)) {
        func_8005C650(1, 0x7F, 0x7F);
        ret = 1;
    }

    i = 0;
    nib = 0xF;
    shift = 0;
    for (; i < 2; shift += 0x10, i++) {
        lim = ((arg2 >> i) & 1) ? 4 : 5;

        if (*arg1 & (0x1000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (((s16 *)((u8 *)D_800A34FC + 0x28))[i] <= 0) {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = lim;
            } else {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = (s16)(((s16 *)((u8 *)D_800A34FC + 0x28))[i] - 1);
            }
        } else if (*arg1 & (0x4000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (((s16 *)((u8 *)D_800A34FC + 0x28))[i] >= lim) {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = 0;
            } else {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = (s16)(((s16 *)((u8 *)D_800A34FC + 0x28))[i] + 1);
            }
        }

        field = ((s16 *)((u8 *)D_800A34FC + 0x28))[i];
        switch (field) {
        case 0:
        case 1:
        case 2:
            if (*arg1 & (0xF0 << shift)) {
                func_8005C650(0, 0x7F, 0x7F);
                func_8006CBD4(i, *arg1);
            }
            break;
        case 3:
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    masked = *(rec + 0x1A) & nib;
                    *(rec + 0x17) = (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + masked);
                }
            }
            break;
        case 4:
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    masked = *(rec + 0x1D) & nib;
                    *(rec + 0x17) = (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + masked);
                }
            }
            break;
        }
        nib <<= 4;
    }
    return ret;
}
