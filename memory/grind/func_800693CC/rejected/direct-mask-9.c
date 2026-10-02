/* measurement only: stands in for the game.h declaration */
typedef struct { u8 state; u8 mode; } MenuOption;
extern MenuOption D_8009BC0C[8];
s32 func_800693CC(s32 held, s32 pressed, s32 arg2, s32 arg3) {
    extern s32 D_8009BC08;
    extern void func_80069AE4(s32 *, s32, s32);
    extern void func_8006A880(u8 *, u16 *, s32);
    extern void func_80069F80(s32 *, s32);
    extern void func_8006A1A0(s32 *, s32);
    /* FAKE: frame layout. The draw context func_8006E390 fills is ten words
     * (siblings: s32 sp10[10]); the decoded input sits at word 12 and the
     * saved row mask at word 14 only to reproduce the target's 0x60 frame
     * (sp+0x40 / sp+0x48). Separate locals: score 11-21, wrong frame. */
    s32 context[15];
    s32 result;
    s32 render_base;
    render_base = ((D_800A36AC & 1) * 0x4090) + (s32)&g_gpu_db;
    func_8006E390(context, &D_800A3518);
    context[12] = (pressed & 0xFFFF) | ((u32)pressed >> 16);
    result = func_800692C0((u32 *)&context[12], 0,
                           (s16 *)(D_800A34FC + 0xC), D_800A350C);
    if ((result >> 16) == 1) goto increment;
    if ((result >> 16) == 2) goto decrement;
    goto after_move;
increment:
    {
        /* FAKE: the index-field clear mask, named so its load sits ahead of
         * the alias (the literal moves `li -16` two slots; score 2). */
        s32 clear_mask = ~0xF;
        /* FAKE: keep the availability address across the loop so GCC reloads
         * its word each iteration; direct global reads hoist the word (score 5). */
        s32 *available = &D_8009BC04;
        do {
            s32 flags = D_800A34F8;
            s32 index;
            D_800A34F8 = (flags & clear_mask) | (index = ((flags & 0xF) + 1) & 0xF);
            if (index >= 8) {
                D_800A34F8 &= clear_mask;
            }
        } while (!(((u32)*available >> (D_800A34F8 & 0xF)) & 1));
    }
    goto moved;
decrement:
    {
        /* FAKE: keep the availability address across the loop so GCC reloads
         * its word each iteration; direct global reads hoist the word (score 5). */
        s32 *available = &D_8009BC04;
        do {
            s32 flags = D_800A34F8;
            s32 index = flags & 0xF;
            if (index == 0) {
                D_800A34F8 = (flags & ~0xF) | 7;
            } else {
                D_800A34F8 = (flags & ~0xF) | ((index - 1) & 0xF);
            }
        } while (!(((u32)*available >> (D_800A34F8 & 0xF)) & 1));
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
            context[14] = D_8009BC08 & 0x80;
            D_8009BC08 = context[14] | (0x1F << ((D_800A34F8 & 0xF) - 5));
        } else {
            D_8009BC08 = 0x9F;
        }
    }
    func_80069AE4(context, 0, render_base);
    func_8006A880((u8 *)context, (u16 *)render_base,
                  D_8009BC0C[D_800A34F8 & 0xF].mode);
    func_80069F80(context, D_8009BC0C[D_800A34F8 & 0xF].mode);
    func_8006A1A0(context, D_8009BC0C[D_800A34F8 & 0xF].mode);
    if (pressed & 0x40) {
        switch (D_8009BC0C[D_800A34F8 & 0xF].state) {
        case 4:
        case 5:
            if (D_80102788.unk_00[3] == 0) {
                func_8005C650(2, 0x7F, 0x7F);
                func_8005C6D0();
                goto cancel;
            }
        }
        goto accept;
    }
    if (pressed & 0x400000) {
        switch (D_8009BC0C[D_800A34F8 & 0xF].state) {
        case 0: case 1: case 2: case 3: case 6:
            goto reject;
        }
        if (D_80102788.unk_00[2] == 0) {
reject:
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
    if (pressed & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        return -2;
    }
cancel:
    if (pressed & 0x50005000) {
        func_8005C650(0, 0x7F, 0x7F);
    }
    return -1;
}
