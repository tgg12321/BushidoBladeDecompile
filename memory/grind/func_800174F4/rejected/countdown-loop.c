/* REJECTED (grind session 2, structural) - sandbox --disable all
 * score=35, build_insns=133 (base for this session: score 8, insns 136).
 *
 * the loop counts DOWN from `h` to 0 instead of up to `h` - three
 * instructions lost, target's `slt` compare disappears.
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
            if (h != 0) {
                i = h;
                inner_loop:
                prim = (s32)func_8005D554((u8 *)prim, g_disp_enable);
                i--;
                if (i == 0) {
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
