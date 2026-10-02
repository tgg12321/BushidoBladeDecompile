s32 func_800693CC(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    typedef struct { u8 state; u8 mode; } MenuOption;
    extern s32 D_8009BC08; extern MenuOption D_8009BC0C[8];
    extern s16 D_8010278C, D_8010278E;
    extern void func_80069AE4(s32 *, s32, s32);
    extern void func_8006A880(u8 *, u16 *, s32);
    extern void func_80069F80(s32 *, s32);
    extern void func_8006A1A0(s32 *, s32);
    s32 context[15];
    s32 result;
    s32 render_base;
    render_base = ((D_800A36AC & 1) * 0x4090) + (s32)&g_gpu_db;
    func_8006E390(context, &D_800A3518);
    context[12] = (arg1 & 0xFFFF) | ((u32)arg1 >> 16);
    result = func_800692C0((u32 *)&context[12], 0,
                           (s16 *)(D_800A34FC + 0xC), &D_800A350C);
    if ((result >> 16) == 1) goto increment;
    if ((result >> 16) == 2) goto decrement;
    goto after_move;
increment:
    {
        s32 clear_mask = -0x10;
        do {
            s32 flags = D_800A34F8;
            s32 index;
            D_800A34F8 = (flags & clear_mask) | (index = ((flags & 0xF) + 1) & 0xF);
            if (index >= 8) {
                D_800A34F8 &= clear_mask;
            }
        } while (!((D_8009BC04 >> (D_800A34F8 & 0xF)) & 1));
    }
    goto moved;
decrement:
    {
        u32 *available = &D_8009BC04;
        do {
            s32 flags = D_800A34F8;
            s32 index = flags & 0xF;
            if (index == 0) {
                D_800A34F8 = (flags & ~0xF) | 7;
            } else {
                D_800A34F8 = (flags & ~0xF) | ((index - 1) & 0xF);
            }
        } while (!((*available >> (D_800A34F8 & 0xF)) & 1));
    }
moved:
    D_800A3514 = 0;
    ((s32 *)D_800A3524)[8] &= ~8;
after_move:
    if (((result & 0xFF) < 3) && ((result & 0xFF) != 0) &&
        (D_8009BC0C[D_800A34F8 & 0xF].mode == 3)) {
        ((s32 *)D_800A3524)[8] =
            (((s32 *)D_800A3524)[8] & ~8) |
            (((((u32)((s32 *)D_800A3524)[8] >> 3) & 1) ^ 1) << 3);
        func_8005C650(0, 0x7F, 0x7F);
    }
    if (D_8009BC0C[D_800A34F8 & 0xF].mode == 2) {
        ((s32 *)D_800A3524)[8] |= 8;
    } else if (D_8009BC0C[D_800A34F8 & 0xF].mode == 1) {
        ((s32 *)D_800A3524)[8] &= ~8;
    }
    if ((D_800A34F8 & 0xF) != 7) {
        if (((D_800A34F8 & 0xF) - 6) >= 0) {
            s32 *mask_ptr = &D_8009BC08;
            context[14] = *mask_ptr & 0x80;
            *mask_ptr = context[14] | (0x1F << ((D_800A34F8 & 0xF) - 5));
        } else {
            D_8009BC08 = 0x9F;
        }
    }
    func_80069AE4(context, 0, render_base);
    func_8006A880((u8 *)context, (u16 *)render_base,
                  D_8009BC0C[D_800A34F8 & 0xF].mode);
    func_80069F80(context, D_8009BC0C[D_800A34F8 & 0xF].mode);
    func_8006A1A0(context, D_8009BC0C[D_800A34F8 & 0xF].mode);
    if (arg1 & 0x40) {
        s32 state = D_8009BC0C[D_800A34F8 & 0xF].state;
        if (state >= 6) goto accept;
        if (state < 4) goto accept;
        {
            if (D_8010278E == 0) {
                func_8005C650(2, 0x7F, 0x7F);
                func_8005C6D0();
                goto cancel;
            }
            goto accept;
        }
    }
    if (arg1 & 0x400000) {
        s32 state = D_8009BC0C[D_800A34F8 & 0xF].state;
        if (state >= 0) {
            if (state >= 4) {
                if (state != 6) {
                    goto check_locked;
                }
                goto choose;
            }
            goto choose;
        }
check_locked:
        if (D_8010278C == 0) {
choose:
            func_8005C650(2, 0x7F, 0x7F);
            goto cancel;
        }
accept:
        func_8005C650(1, 0x7F, 0x7F);
        if (D_8009BC0C[D_800A34F8 & 0xF].state != 7) {
            func_8005B6FC();
        }
        return D_8009BC0C[D_800A34F8 & 0xF].state;
    }
    if (arg1 & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        return -2;
    }
cancel:
    if (arg1 & 0x50005000) {
        func_8005C650(0, 0x7F, 0x7F);
    }
    return -1;
}
