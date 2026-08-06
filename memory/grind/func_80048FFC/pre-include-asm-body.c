/* func_80048FFC — C body removed by Campaign 4 (INCLUDE_ASM conversion).
 * The build DISCARDED this body (asmfix replace_with_asmfile substituted
 * asm/funcs/func_80048FFC.s verbatim), so it never reached the binary.
 * This body is a real prior decomp attempt — archaeology for the C SHAPE when this function is decompiled.
 * NOTE: its signature was never checked by anything — do not trust it.
 * Campaign plan R2.
 */
void func_80048FFC(s32 arg0) {
    s16 sp10, sp12;
    u16 sp14;
    s16 sp16;
    s32 *sp18;
    s32 sp20, sp28, sp30, sp38;
    s32 sp40, sp48, sp50, sp58;
    s32 sp60, sp68;
    s32 sp70;
    s32 *base = (s32 *)((arg0 * 0x134) + (s32)&D_800EF848);
    s32 *cur;
    s32 var_s7;
    s32 a1_1, v0_1;

    var_s7 = *base;
    sp18 = (s32 *)((s32)base + 0x124);
    cur = (s32 *)((s32)base + ((D_800A36AC & 1) * 0x90) + 4);

    {
        u32 v1 = *(u16 *)((s32)base + 0x124);
        u32 v1_2 = *(u16 *)((s32)base + 0x126);
        s32 a0 = (s32)(s16)v1;
        sp20 = (s16)((s32)v1 & 0xFFC0);
        sp28 = (s16)((s32)v1_2 & 0xFF00);
        a1_1 = a0;
        if (a0 < 0) a1_1 = a0 + 0x3F;
        sp30 = (s16)(a0 - ((a1_1 >> 6) << 6));
        {
            s32 v1_3 = (s32)(s16)v1_2;
            v0_1 = v1_3;
            if (v1_3 < 0) v0_1 = v1_3 + 0xFF;
            sp38 = (s16)(v1_3 - ((v0_1 >> 8) << 8));
        }
    }
    sp60 = *(s16 *)((s32)base + 0x128);
    {
        u32 v1 = *(u16 *)((s32)base + 0x12C);
        s16 t2 = *(s16 *)((s32)base + 0x12A);
        s32 a0 = (s32)(s16)v1;
        u32 a2 = *(u16 *)((s32)base + 0x12E);
        sp40 = (s16)((s32)v1 & 0xFFC0);
        sp48 = (s16)((s32)a2 & 0xFF00);
        sp68 = t2;
        a1_1 = a0;
        if (a0 < 0) a1_1 = a0 + 0x3F;
        sp50 = (s16)(a0 - ((a1_1 >> 6) << 6));
        {
            s32 v1_3 = (s32)(s16)a2;
            v0_1 = v1_3;
            if (v1_3 < 0) v0_1 = v1_3 + 0xFF;
            sp58 = (s16)(v1_3 - ((v0_1 >> 8) << 8));
        }
    }
    sp70 = 0;
    do {
        s32 t_w = sp40 + sp50;
        s32 t_h = sp48 + sp58;
        s32 t_y = sp28 + sp38;
        s16 t_diff = sp68 - var_s7;
        sp10 = sp20 + sp30;
        sp12 = (s16)t_y;
        sp14 = (u16)sp60;
        sp16 = t_diff;
        initLoadImage((void *)cur, &sp10, t_w, t_h + var_s7);
        {
            s32 prev_s7 = var_s7;
            var_s7 = var_s7 >> 1;
            sp30 = sp30 >> 1;
            sp38 = sp38 >> 1;
            sp50 = sp50 >> 1;
            sp58 = sp58 >> 1;
            sp60 = sp60 >> 1;
            sp68 = sp68 >> 1;
            sp70 += 1;
            *cur = (*cur & 0xFF000000) | (*(s32 *)((s32)&D_800A378C + 0x3FFC) & 0xFFFFFF);
            *(s32 *)((s32)&D_800A378C + 0x3FFC) = (*(s32 *)((s32)&D_800A378C + 0x3FFC) & 0xFF000000) | ((s32)cur & 0xFFFFFF);
            sp12 = (s16)(t_y + t_diff);
            sp16 = (s16)prev_s7;
            initLoadImage((void *)((s32)cur + 0x18), &sp10, t_w, t_h);
            *(s32 *)((s32)cur + 0x18) = (*(s32 *)((s32)cur + 0x18) & 0xFF000000) | (*(s32 *)((s32)&D_800A378C + 0x3FFC) & 0xFFFFFF);
            *(s32 *)((s32)&D_800A378C + 0x3FFC) = (*(s32 *)((s32)&D_800A378C + 0x3FFC) & 0xFF000000) | (((s32)cur + 0x18) & 0xFFFFFF);
            cur = (s32 *)((s32)cur + 0x30);
        }
    } while (sp70 < 3);
    {
        s32 v = *base + *(s16 *)((s32)sp18 + 0xC);
        s16 mod_by = *(s16 *)((s32)sp18 + 6);
        *base = v;
        if (v >= mod_by) *base = v % mod_by;
    }
}
