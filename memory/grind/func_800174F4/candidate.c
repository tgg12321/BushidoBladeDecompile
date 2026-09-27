/* func_800174F4 — working candidate, manual session 2026-09-26 (slotP, post-Q17).
 *
 * Sandbox --disable all = 0 (136/136) under the STOCK compiler, with no
 * do-while(0) wraps, no goto loop and no `unsigned short h`. What changed
 * against rejected/compiler-patch-dependent.c (evidence.md "Session 7"):
 *   - the loop limit is an int, so no u16 narrowing chain reaches combine and
 *     `(rand() & 3) + 4` stays an addiu (the old u16 `h` gave combine a
 *     3-insn subreg chain it rewrote to IOR; cc1psx does the same);
 *   - the loop is the natural `for (i = 0; i < n; i++)`;
 *   - the DRAWENV buffer is its real size (0x5C): the for-loop's entry-test
 *     combine leaves one 8-byte reload slot, which is exactly the frame
 *     difference the old 0x68 buffer was absorbing;
 *   - 240 is a literal at both uses (cse keeps it in $s1 by itself).
 * Two locals still hold two values each (`temp`, `sel`): Ruling 11 candidates,
 * proof pending in evidence.md.
 */
void func_800174F4(void) {
    u8 ot[8];
    u8 env[0x5C];
    s32 temp;
    u8 *prim;
    s32 sel;

    prim = &D_800F33D8;
    if (g_disp_enable == DISP_DISABLED) {
        return;
    }
    SetDefDrawEnv(env, 0, (D_800A36AC & 1) ? 0xF0 : 0, 0x280, 0xF0);
    env[0x18] = 0;
    PutDrawEnv(env);
    g_gpu_ot_ptr = ot;
    ClearOTagR(ot, 2);
    sel = g_disp_enable;
    switch (sel) {
    case 1:
    case 2:
        prim = func_8005D46C(prim);
        if (g_disp_fade != 0) {
            s32 i;
            temp = (rand() & 3) + 4;
            for (i = 0; i < temp; i++) {
                prim = func_8005D554(prim, g_disp_enable);
            }
        }
        else if ((rand() & 7) == 0) {
            func_8005D554(prim, g_disp_enable);
        }
        break;
    case 10:
        func_8005E54C(D_800A3784, prim, 0);
        break;
    case 20: {
        u8 cur;
        u32 idx;

        sel = D_800A37A0;
        cur = D_800A38F8;
        idx = cur;
        if (sel < idx) {
            break;
        }
        if (0xF0 / (sel + 1) >= ++D_800A37C0) {
            break;
        }
        if (idx == sel) {
            D_800A38F8 = cur + 1;
        } else {
            u8 next = cur + 1;
            D_800A38F8 = next;
            D_800A37C0 = 0;
            temp = D_800A37A8[idx];
            if (next == sel) {
                temp |= 0x8000;
            }
            func_80060414(temp, prim, 0);
        }
        break;
    }
    }
    DrawOTag((u8 *)(g_gpu_ot_ptr + 4));
    DrawSync(0);
}
