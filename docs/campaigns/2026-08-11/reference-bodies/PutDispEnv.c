/* BEST ADAPTED BODY - src/display.c - PutDispEnv
 * Reference: sotn-decomp src/main/psxsdk/libgpu/sys.c:336-402.
 * Adaptation: D_8002C260->ctl -> D_8009BE6C[4]; D_8002C268 -> g_gpu_debug_level;
 * D_8002C26C -> D_8009BE74 (range test (u32)(x-1) < 2U); D_8002C270 -> D_8009BE77;
 * get_dx() -> func_8007CA00(); GetVideoMode() -> sys_GetVideoMode(); the shadow env
 * global is D_8009BEE0 (SOTN D_80037EBC).
 * BB2's revision differs from SOTN's in four measurable ways, all read off the target
 * asm: it compares all FOUR halfword fields of screen/disp (SOTN's LOW() macro is a
 * 32-bit reinterpret, so SOTN compares word PAIRS); it clamps v_start from 0x10 rather
 * than 0; it uses v_start+2 and 0x102/0x138 for the v_end clamp (SOTN v_start+1,
 * 0x100/0x138); and it adds 0x13 rather than 0x10 to screen.y when the video mode is
 * PAL.
 *
 * MEASURED: 77   (baseline 298 = INCLUDE_ASM).  build 293 / target 298.
 * RESIDUAL: the eight shadow-field compares - the target loads the shadow field with
 * `lhu` and then sign-extends it with sll 16 / sra 16, while our build folds the
 * u16-read + (s16)-cast into a single `lh`. Plus one callee-save register choice
 * ($s3 vs $s2) for `cmd`.
 *
 * Replaces:  INCLUDE_ASM("asm/funcs", PutDispEnv);
 */

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} _DispRect;

typedef struct {
    _DispRect disp;   /* +0x00 */
    _DispRect screen; /* +0x08 */
    u8 isinter;       /* +0x10 */
    u8 isrgb24;       /* +0x11 */
    u8 pad0;          /* +0x12 */
    u8 pad1;          /* +0x13 */
} _DISPENV;

typedef struct {
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} _DispRectU;

typedef struct {
    _DispRectU disp;
    _DispRectU screen;
    s32 flags;
} _DISPENVU;

#define DCLAMP(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))

extern _DISPENVU D_8009BEE0;
extern u32 D_80015FF8;
extern u8 D_8009BE77;
extern u32 *D_8009BE6C;
extern s32 sys_GetVideoMode(void);
extern s32 func_8007CA00(_DISPENV *);

_DISPENV *PutDispEnv(_DISPENV *env) {
    s32 h_start;
    s32 v_start;
    s32 cmd;
    s32 v_end;
    s32 h_end;

    cmd = 0x08000000;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015FF8, env);
    }
    ((void (*)(s32))D_8009BE6C[4])(
        0x05000000 |
        ((u32)(D_8009BE74 - 1) < 2U
             ? (((env->disp.y & 0xFFF) << 12) | (func_8007CA00(env) & 0xFFF))
             : (((env->disp.y & 0x3FF) << 10) | (env->disp.x & 0x3FF))));

    if (!((s16)D_8009BEE0.screen.x == env->screen.x &&
          (s16)D_8009BEE0.screen.y == env->screen.y &&
          (s16)D_8009BEE0.screen.w == env->screen.w &&
          (s16)D_8009BEE0.screen.h == env->screen.h)) {
        env->pad0 = sys_GetVideoMode();
        h_start = env->screen.x * 0xA + 0x260;
        v_start = env->screen.y + (env->pad0 ? 0x13 : 0x10);
        h_end = h_start + (env->screen.w ? env->screen.w * 0xA : 0xA00);
        v_end = v_start + (env->screen.h ? env->screen.h : 0xF0);
        h_start = DCLAMP(h_start, 0x1F4, 0xCDA);
        h_end = DCLAMP(h_end, h_start + 0x50, 0xCDA);
        v_start = DCLAMP(v_start, 0x10, (env->pad0 ? 0x136 : 0x100));
        v_end = DCLAMP(v_end, v_start + 2, (env->pad0 ? 0x138 : 0x102));
        ((void (*)(s32))D_8009BE6C[4])(0x06000000 | ((h_end & 0xFFF) << 12) |
                                       (h_start & 0xFFF));
        ((void (*)(s32))D_8009BE6C[4])(0x07000000 | ((v_end & 0x3FF) << 10) |
                                       (v_start & 0x3FF));
    }
    if (D_8009BEE0.flags != *(s32 *)&env->isinter ||
        !((s16)D_8009BEE0.disp.x == env->disp.x &&
          (s16)D_8009BEE0.disp.y == env->disp.y &&
          (s16)D_8009BEE0.disp.w == env->disp.w &&
          (s16)D_8009BEE0.disp.h == env->disp.h)) {
        env->pad0 = sys_GetVideoMode();
        if (env->pad0 == 1) {
            cmd |= 0x8;
        }
        if (env->isrgb24) {
            cmd |= 0x10;
        }
        if (env->isinter) {
            cmd |= 0x20;
        }
        if (D_8009BE77 != 0) {
            cmd |= 0x80;
        }
        if (env->disp.w > 0x118) {
            if (env->disp.w <= 0x160) {
                cmd |= 1;
            } else if (env->disp.w <= 0x190) {
                cmd |= 0x40;
            } else if (env->disp.w <= 0x230) {
                cmd |= 2;
            } else {
                cmd |= 3;
            }
        }
        if (env->disp.h > (env->pad0 ? 0x120 : 0x100)) {
            cmd |= 0x24;
        }
        ((void (*)(s32))D_8009BE6C[4])(cmd);
    }
    memcpy((s32)&D_8009BEE0, env, 0x14);
    return env;
}
