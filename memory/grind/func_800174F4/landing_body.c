/* PsyQ libgpu DRAWENV (libgpu.h): 0x5C bytes, isbg at +0x18. */
typedef struct {
    Rect clip;
    s16 ofs[2];
    Rect tw;
    u16 tpage;
    u8 dtd;
    u8 dfe;
    u8 isbg;
    u8 r0;
    u8 g0;
    u8 b0;
    u32 dr_env[16];
} DrawEnv;

void func_800174F4(void) {
    u32 ot[2];
    DrawEnv env;
    /* temp: holds two values, the case-1/2 fade loop's iteration count and
     * the case-20 D_800A37A8[] code passed to func_80060414. Ruling 11
     * (ordinary-c-judge-decidable.md); (D) proof in
     * memory/grind/func_800174F4/evidence.md "Ruling 11 proof". */
    s32 temp;
    u8 *prim;
    /* temp2: holds two values, the g_disp_enable switch selector and the
     * case-20 D_800A37A0 limit. Ruling 11; (D) proof in
     * memory/grind/func_800174F4/evidence.md "Ruling 11 proof". */
    s32 temp2;

    prim = &D_800F33D8;
    if (g_disp_enable == DISP_DISABLED) {
        return;
    }
    SetDefDrawEnv((u8 *)&env, 0, (D_800A36AC & 1) ? 0xF0 : 0, 0x280, 0xF0);
    env.isbg = 0;
    PutDrawEnv((u8 *)&env);
    g_gpu_ot_ptr = (u8 *)ot;
    ClearOTagR((u8 *)ot, 2);
    temp2 = g_disp_enable;
    switch (temp2) {
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

        temp2 = D_800A37A0;
        cur = D_800A38F8;
        if ((u32)temp2 < cur) {
            break;
        }
        if (0xF0 / (temp2 + 1) >= ++D_800A37C0) {
            break;
        }
        /* FAKE: the common `D_800A38F8 = cur + 1` store is written in both
         * arms (unconditional-common-store duplication, F7, no-new-park-
         * categories.md 2026-08-18). Target computes `addiu v0,a2,1` in each
         * arm; one store hoisted above the `if` measures 6 (131 insns),
         * `next` hoisted with a store per arm measures 1 (135). */
        if (cur == temp2) {
            D_800A38F8 = cur + 1;
        } else {
            u8 next = cur + 1;
            D_800A38F8 = next;
            D_800A37C0 = 0;
            temp = D_800A37A8[cur];
            if (next == temp2) {
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
