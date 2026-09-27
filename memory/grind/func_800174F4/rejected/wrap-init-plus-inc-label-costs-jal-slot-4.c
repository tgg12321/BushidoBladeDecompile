/* REJECTED (grind session 6, forensics).
 * Both wraps, but the second one spans ONLY `i++`. Score 4 / 136.
 * The register half is SOLVED (counter n_refs 7, live_length 13, priority
 * 10769 > `h`'s 9333, counter -> $s0, h -> $s1, and `move s0,zero` is in the
 * `jal rand` delay slot). The residual 4 points are the loop's own delay
 * slot: ours puts `move a0,s2` in the func_8005D554 slot and a nop in the
 * back-edge `j` slot, target puts `addiu s0,s0,1` in the call slot and
 * duplicates `move a0,s2` into the pre-header and the `j` slot. Cause: the
 * wrap's CODE_LABEL sits between the call and `i++`. Fixed by spanning the
 * whole loop body with the wrap - that is the matching form.
 */
void func_800174F4(void) {
    u8 sp18[8];
    u8 sp20[0x68];
    s32 env;
    unsigned short h;
    s32 prim;
    s32 mask;
    s32 mode;

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
    mode = g_disp_enable;
    switch (mode) {
    case 1:
    case 2:
        prim = (s32)func_8005D46C((u8 *)prim);
        if (g_disp_fade != 0) {
            s32 v0;
            s32 i;
            do { i = 0; } while (0);
            v0 = rand();
            v0 &= 3;
            h = v0 + 4;
            if (h == 0) {
                break;
            }
        inner_loop:
            prim = (s32)func_8005D554((u8 *)prim, g_disp_enable);
            do { i++; } while (0);
            if (i >= h) {
                break;
            }
            goto inner_loop;
        }
        else if ((rand() & 7) == 0) {
            func_8005D554((u8 *)prim, g_disp_enable);
        }
        break;
    case 10:
        func_8005E54C(D_800A3784, (u8 *)prim, 0);
        break;
    case 20:
        mode = D_800A37A0;
    {
        u8 a2_val = D_800A38F8;
        s32 a0_val = a2_val & 0xFF;
        s32 div_result;
        s32 counter;
        if (((u32)mode) < (u32)a0_val) {
            break;
        }
        div_result = h / (mode + 1);
        counter = D_800A37C0 + 1;
        D_800A37C0 = counter;
        if (div_result >= counter) {
            break;
        }
        if (a0_val == mode) {
            D_800A38F8 = a2_val + 1;
        } else {
            u8 new_val = a2_val + 1;
            D_800A38F8 = new_val;
            D_800A37C0 = 0;
            h = D_800A37A8[a0_val];
            if ((new_val & 0xFF) == mode) {
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
