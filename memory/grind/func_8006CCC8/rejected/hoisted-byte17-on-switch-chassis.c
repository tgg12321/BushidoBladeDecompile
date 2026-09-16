/* REJECTED s7 (synthesis) -- hoisted-byte17-on-switch-chassis
 * Re-measure of the s3/s6 hoisted byte17 named-intermediate j-loop form on the s7 SWITCH-dispatch chassis (floor 10 before this probe). Scored 58 (build_insns 187) vs 10 for the split-read form and 0 for the J4 ternary-mask form. Third chassis on which the hoisted-read local loses; killed instance, no FAKE present.
 * Chassis: s7 switch(field) dispatch (floor 10 before the j-loop probe), all ordinary C.
 */
s32 func_8006CCC8(s32 *arg0, s32 *arg1, s16 arg2) {
    s32 i;
    s16 lim;
    s32 shift;
    s32 nib;
    s32 fade;
    s32 j;
    u8 *rec;
    s32 ret;
    s32 t;
    s32 masked;
    s16 field;
    s32 byte17;

    ret = 0;
    if ((*(s32 *)((u8 *)D_800A34FC + 0x28) == 0x50005) && (*arg1 & 0x400040)) {
        func_8005C650(1, 0x7F, 0x7F);
        ret = 1;
    }

    i = 0;
    nib = 0xF;
    fade = 0;
    shift = 0;
    for (; i < 2; shift += 0x10, i++) {
        t = arg2;
        lim = ((t >> i) & 1) ? 4 : 5;

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
                t = i;
                func_8006CBD4(t, *arg1);
            }
            break;
        case 3:
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    masked = *(rec + 0x1A) & (nib << fade);
                    byte17 = *(rec + 0x17);
                    if (i == 0) {
                        *(rec + 0x17) = (u8)((byte17 & 0xF0) + masked);
                    } else {
                        *(rec + 0x17) = (u8)((byte17 & 0xF) + masked);
                    }
                }
            }
            break;
        case 4:
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    masked = *(rec + 0x1D) & (nib << fade);
                    byte17 = *(rec + 0x17);
                    if (i == 0) {
                        *(rec + 0x17) = (u8)((byte17 & 0xF0) + masked);
                    } else {
                        *(rec + 0x17) = (u8)((byte17 & 0xF) + masked);
                    }
                }
            }
            break;
        }
        fade += 4;
    }
    return ret;
}
