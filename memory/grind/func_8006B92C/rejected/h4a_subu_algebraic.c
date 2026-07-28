s32 func_8006B92C(s32 *unused, u32 *arg1) {
    s32 sp10;
    s32 ret;
    s32 idx;
    s32 var_v0;
    u32 var_v1;
    s32 var_s0 = 0;
    u32 v;
    u32 a0;
    v = *arg1;
    sp10 = (v & 0xFFFF) | (v >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC + 0xC, &D_800A350C);
    ret >>= 16;
    switch (ret) {
    case 1:
        a0 = D_800A34F8;
        if ((a0 & 0xE000) == 0x4000) {
            D_800A34F8 = a0 & 0xFFFF1FFF;
        } else {
            var_v1 = a0 - (a0 & 0xE000);
            var_v0 = ((a0 >> 13) & 7) + 1;
            goto complete_store;
        }
        goto do_call;
    case 2:
        a0 = D_800A34F8;
        if ((a0 & 0xE000) == 0) {
            D_800A34F8 = (a0 & 0xFFFF1FFF) | 0x4000;
        } else {
            var_v1 = a0 - (a0 & 0xE000);
            var_v0 = ((a0 >> 13) & 7) - 1;
        complete_store:
            var_v1 |= ((var_v0 & 7) << 13);
            D_800A34F8 = var_v1;
        }
    do_call:
        func_8005C650(0, 0x7F, 0x7F);
        break;
    }

    idx = (D_800A34F8 >> 13) & 7;
    switch (idx) {
    case 0:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 2;
        }
        break;
    case 1:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 3;
        }
        break;
    case 2:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 1;
            D_800A34F8 = (D_800A34F8 & ~0x1C00) | (((((D_800A34F8 >> 10) & 7) + 1) & 7) << 10);
        }
        break;
    }

    if (*arg1 & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        var_s0 = 1;
    }
