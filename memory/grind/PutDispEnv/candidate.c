DISPENV *PutDispEnv(DISPENV *env) {
    s32 h_start, h_end;
    s32 v_start, v_end;
    s32 mode;

    mode = 0x08000000;
    if (g_gpu_debug_level >= 2) {
        GPU_printf(D_80015FF8, env);
    }
    g_gpu_dev_table->ctl(
        g_gpu_type == 1 || g_gpu_type == 2
            ? ((env->disp.y & 0xFFF) << 12) | (get_dx((s16 *)env) & 0xFFF) | 0x05000000
            : ((env->disp.y & 0x3FF) << 10) | (env->disp.x & 0x3FF) |
                  0x05000000);
    /* FAKE: volatile-qualified reads of the saved environment (8 casts, both rect
       compares). The shipped bytes load every one of these fields as lhu + sll 16 +
       sra 16 -- the un-folded extend GCC keeps only for a volatile halfword -- while
       the env-> side of the same compares is a plain lh; the non-volatile spelling
       folds to lh and scores 75 (memory/grind/PutDispEnv/evidence.md).
       SOTN: src/main/psxsdk/libspu/s_m_m.c:48 @aa53500 (use-site
       `*(volatile int *)&` read of the non-IRQ RAM global _spu_memList). */
    if (!(*(volatile s16 *)&g_gpu_disp_env.screen.x == env->screen.x &&
          *(volatile s16 *)&g_gpu_disp_env.screen.y == env->screen.y &&
          *(volatile s16 *)&g_gpu_disp_env.screen.w == env->screen.w &&
          *(volatile s16 *)&g_gpu_disp_env.screen.h == env->screen.h)) {
        env->pad0 = GetVideoMode();
        h_start = env->screen.x * 10 + 0x260;
        v_start = env->screen.y + (env->pad0 ? 0x13 : 0x10);
        h_end = h_start + (env->screen.w ? env->screen.w * 10 : 2560);
        v_end = v_start + (env->screen.h ? env->screen.h : 240);
        h_start = h_start < 500 ? 500 : (h_start > 3290 ? 3290 : h_start);
        h_end = h_end < h_start + 0x50 ? h_start + 0x50
                                       : (h_end > 3290 ? 3290 : h_end);
        v_start = v_start < 0x10 ? 0x10
                : (v_start > (env->pad0 ? 310 : 256) ? (env->pad0 ? 310 : 256)
                                                     : v_start);
        v_end = v_end < v_start + 2 ? v_start + 2
              : (v_end > (env->pad0 ? 312 : 258) ? (env->pad0 ? 312 : 258)
                                                 : v_end);
        g_gpu_dev_table->ctl(
            ((h_end & 0xFFF) << 12) | 0x06000000 | (h_start & 0xFFF));
        g_gpu_dev_table->ctl(
            ((v_end & 0x3FF) << 10) | 0x07000000 | (v_start & 0x3FF));
    }
    /* isinter..pad1 compared as one word: the shipped bytes are a single lw at
       +0x10 on both sides. SOTN: src/main/psxsdk/libgpu/sys.c:367 @aa53500
       (LOW(D_80037EBC.isinter) != LOW(env->isinter), LOW = *(s32 *)&). */
    if (*(s32 *)&g_gpu_disp_env.isinter != *(s32 *)&env->isinter ||
        !(*(volatile s16 *)&g_gpu_disp_env.disp.x == env->disp.x &&
          *(volatile s16 *)&g_gpu_disp_env.disp.y == env->disp.y &&
          *(volatile s16 *)&g_gpu_disp_env.disp.w == env->disp.w &&
          *(volatile s16 *)&g_gpu_disp_env.disp.h == env->disp.h)) {
        env->pad0 = GetVideoMode();
        if (env->pad0 == 1) {
            mode |= 0x8;
        }
        if (env->isrgb24) {
            mode |= 0x10;
        }
        if (env->isinter) {
            mode |= 0x20;
        }
        if (D_8009BE77) {
            mode |= 0x80;
        }
        if (env->disp.w > 280) {
            if (env->disp.w <= 352) {
                mode |= 1;
            } else if (env->disp.w <= 400) {
                mode |= 0x40;
            } else if (env->disp.w <= 560) {
                mode |= 2;
            } else {
                mode |= 3;
            }
        }
        /* FAKE: empty then-arm; the direct `if (env->disp.h > ...) mode |= 0x24;`
           and its respellings add 4 insns (memory/grind/PutDispEnv/evidence.md).
           SOTN: src/main/psxsdk/libgpu/sys.c:394 @aa53500 (same statement, same form). */
        if (env->disp.h <= (env->pad0 ? 288 : 256)) {
        } else {
            mode |= 0x24;
        }
        g_gpu_dev_table->ctl(mode);
    }
    memcpy((s32)&g_gpu_disp_env, env, sizeof(DISPENV));
    return env;
}
