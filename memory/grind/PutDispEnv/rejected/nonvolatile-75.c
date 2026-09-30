typedef struct {
    s16 x, y, w, h;
} DispRect;

typedef struct {
    DispRect disp;   /* +0x00 */
    DispRect screen; /* +0x08 */
    u8 isinter;      /* +0x10 */
    u8 isrgb24;      /* +0x11 */
    u8 pad0;         /* +0x12 */
    u8 pad1;         /* +0x13 */
} DispEnv;

extern DispEnv g_gpu_disp_env_x;

DispEnv *PutDispEnv(DispEnv *env) {
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
    if (!(g_gpu_disp_env_x.screen.x == env->screen.x && g_gpu_disp_env_x.screen.y == env->screen.y &&
          g_gpu_disp_env_x.screen.w == env->screen.w && g_gpu_disp_env_x.screen.h == env->screen.h)) {
        env->pad0 = GetVideoMode();
        h_start = env->screen.x * 10 + 0x260;
        v_start = env->screen.y + (env->pad0 ? 0x13 : 0x10);
        h_end = h_start + (env->screen.w ? env->screen.w * 10 : 2560);
        v_end = v_start + (env->screen.h ? env->screen.h : 240);
        h_start = h_start < 500 ? 500 : (h_start > 3290 ? 3290 : h_start);
        h_end = h_end < h_start + 0x50 ? h_start + 0x50 : (h_end > 3290 ? 3290 : h_end);
        v_start = v_start < 0x10 ? 0x10 : (v_start > (env->pad0 ? 310 : 256) ? (env->pad0 ? 310 : 256) : v_start);
        v_end = v_end < v_start + 2 ? v_start + 2 : (v_end > (env->pad0 ? 312 : 258) ? (env->pad0 ? 312 : 258) : v_end);
        g_gpu_dev_table->ctl(
            ((h_end & 0xFFF) << 12) | 0x06000000 | (h_start & 0xFFF));
        g_gpu_dev_table->ctl(
            ((v_end & 0x3FF) << 10) | 0x07000000 | (v_start & 0x3FF));
    }
    if (*(s32 *)&g_gpu_disp_env_x.isinter != *(s32 *)&env->isinter ||
        !(g_gpu_disp_env_x.disp.x == env->disp.x && g_gpu_disp_env_x.disp.y == env->disp.y &&
          g_gpu_disp_env_x.disp.w == env->disp.w && g_gpu_disp_env_x.disp.h == env->disp.h)) {
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
        if (env->disp.w <= 280) {
        } else if (env->disp.w <= 352) {
            mode |= 1;
        } else if (env->disp.w <= 400) {
            mode |= 0x40;
        } else if (env->disp.w <= 560) {
            mode |= 2;
        } else {
            mode |= 3;
        }
        if (env->disp.h <= (!env->pad0 ? 256 : 288)) {
        } else {
            mode |= 0x24;
        }
        g_gpu_dev_table->ctl(mode);
    }
    memcpy((s32)&g_gpu_disp_env_x, env, sizeof(DispEnv));
    return env;
}
