/* REJECTED (grind session 2, structural) - sandbox --disable all
 * score=14, build_insns=136 (base for this session: score 8, insns 136).
 *
 * `i = 0;` placed in the loop PRE-HEADER (after `h = v0 + 4;`, before the
 * `if (h != 0)` guard) - i.e. the placement that WOULD let reorg fill the
 * rand() jal delay slot the way target does. Measured 14 (vs 8 for the
 * in-guard placement): the counter loses $s0 to the `h` web and the whole
 * 6-point callee-save cluster comes back. Same result for `i = 0;` before
 * the rand() call (base, 14) and between rand() and the mask (b2, 14).
 * NB the cc1 BB2_ALLOC_DEBUG table is IDENTICAL for this form and for the
 * floor-8 form (same allocno order, same n_refs/live_length/priority), so
 * the $s0/$s1 flip is NOT decided in global.c - it is decided downstream
 * (local-alloc quantities / reload). Any further work on this axis must
 * instrument local-alloc (BB2_QTY_DEBUG / BB2_SUGG_DEBUG), not global.
 */
void func_800174F4(void) {
    u8 sp18[8];
    u8 sp20[0x68];
    s32 env;
    unsigned short h;
    s32 prim;
    s32 mask;

    prim = (s32)(&D_800F33D8);
    if (g_disp_enable == DISP_DISABLED) {
        return;
    }
    env = (s32)sp20;
    h = 0xF0;
    mask = D_800A36AC & 1;
    mask = -mask;
    SetDefDrawEnv((u8 *)env, 0, mask & 0xF0, 0x280, h);
    sp20[0x18] = 0;
    PutDrawEnv((u8 *)env);
    D_800A374C = sp18;
    ClearOTagR(sp18, 2);
    switch (g_disp_enable) {
    case 1:
    case 2:
        prim = (s32)func_8005D46C((u8 *)prim);
        if (g_disp_fade != 0) {
            s32 v0;
            s32 i;
            v0 = rand();
            v0 &= 3;
            h = v0 + 4;
            i = 0;
            if (h != 0) {
                inner_loop:
                prim = (s32)func_8005D554((u8 *)prim, g_disp_enable);
                i++;
                if (i >= h) {
                    break;
                }
                goto inner_loop;
            }
        }
        else if ((rand() & 7) == 0) {
            func_8005D554((u8 *)prim, g_disp_enable);
        }
        break;
    case 10:
        func_8005E54C(D_800A3784, (u8 *)prim, 0);
        break;
    case 20:
    {
        u8 a2_val = D_800A38F8;
        s32 a1_val = D_800A37A0;
        s32 a0_val = a2_val & 0xFF;
        s32 div_result;
        s32 counter;
        if (((u32)a1_val) < (u32)a0_val) {
            break;
        }
        div_result = h / (a1_val + 1);
        counter = D_800A37C0 + 1;
        D_800A37C0 = counter;
        if (div_result >= counter) {
            break;
        }
        if (a0_val == a1_val) {
            D_800A38F8 = a2_val + 1;
        } else {
            u8 new_val = a2_val + 1;
            D_800A38F8 = new_val;
            D_800A37C0 = 0;
            h = D_800A37A8[a0_val];
            if ((new_val & 0xFF) == a1_val) {
                h |= 0x8000;
            }
            func_80060414(h, (u8 *)prim, 0);
        }
        break;
    }
    }
    DrawOTag((u8 *)(D_800A374C + 4));
    DrawSync(0);
}
