/* PsyQ 4.0 LIBGPU SYS: PutDispEnv — verbatim-linked Sony object (census 2026-07-09); C ref: Xeeynamo/psyz decomp/src/libgpu/sys.c */
typedef struct {
    s16 x, y, w, h;
} _dispenv_rect;

typedef struct {
    _dispenv_rect disp;   /* +0x00 */
    _dispenv_rect screen; /* +0x08 */
    u8 isinter;           /* +0x10 */
    u8 isrgb24;           /* +0x11 */
    u8 pad0;              /* +0x12 */
    u8 pad1;              /* +0x13 */
} _dispenv;

extern u8 D_8009BE77;
extern const char D_80015FF8[];
extern s32 GetVideoMode(void);
s32 get_dx(s16 *arg0);

#define DISP_RECT_EQ(r1, r2)                                                   \
    (((volatile _dispenv_rect *)r1)->x == r2.x &&                              \
     ((volatile _dispenv_rect *)r1)->y == r2.y &&                              \
     ((volatile _dispenv_rect *)r1)->w == r2.w &&                              \
     ((volatile _dispenv_rect *)r1)->h == r2.h)

#define CLAMP(value, low, high)                                                \
    value < low ? low : (value > high ? high : value)

#define info (*(_dispenv *)&g_gpu_disp_env)

_dispenv *PutDispEnv(_dispenv *env) {
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
    if (!DISP_RECT_EQ(&info.screen, env->screen)) {
        env->pad0 = GetVideoMode();
        h_start = env->screen.x * 10 + 0x260;
        v_start = env->screen.y + (env->pad0 ? 0x13 : 0x10);
        h_end = h_start + (env->screen.w ? env->screen.w * 10 : 2560);
        v_end = v_start + (env->screen.h ? env->screen.h : 240);
        h_start = CLAMP(h_start, 500, 3290);
        h_end = CLAMP(h_end, h_start + 0x50, 3290);
        v_start = CLAMP(v_start, 0x10, (env->pad0 ? 310 : 256));
        v_end = CLAMP(v_end, v_start + 2, (env->pad0 ? 312 : 258));
        g_gpu_dev_table->ctl(
            ((h_end & 0xFFF) << 12) | 0x06000000 | (h_start & 0xFFF));
        g_gpu_dev_table->ctl(
            ((v_end & 0x3FF) << 10) | 0x07000000 | (v_start & 0x3FF));
    }
    if (*(s32 *)&info.isinter != *(s32 *)&env->isinter ||
        !DISP_RECT_EQ(&info.disp, env->disp)) {
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
    memcpy((s32)&info, env, sizeof(_dispenv));
    return env;
}
#undef info
#undef CLAMP
#undef DISP_RECT_EQ
