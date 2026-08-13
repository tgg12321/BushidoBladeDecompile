/* func_800470B0 (src/sound.c) — MATCHED, pure C, session 1 (2026-08-12).
 *
 * Sandbox `--disable all` distance 0; full build SHA1 == oracle
 * (62efab4f73f992798c43e8c730aa43baa10bb4fa). Zero regfix/asmfix rules,
 * zero cheat-asm. The prior src body carried `register void *var_s1 asm("s1")`,
 * `register s32 var_s2 asm("s2")` and `volatile s32 _sp_pad[2]` (honest floor 22
 * with those stripped); all three are gone.
 *
 * Requires `#include "gte.h"` in src/sound.c (added by this session; the file
 * previously included common.h / include_asm.h / sound.h / game.h / code6cac.h
 * only). gte.h supplies `typedef struct MATRIX { s16 m[3][3]; u16 pad; s32 t[3]; }`.
 *
 * Two residuals were closed, in this order:
 *
 * 1) FRAME SIZE (56 -> 64, worth 10 of the 17 remaining points). The local is a
 *    PsyQ MATRIX (32 bytes: 18 bytes of m[3][3] + 2 bytes pad + 12 bytes of t[3]),
 *    not the `s16 sp10[9]` (18 -> 24 bytes rounded) the decompilation had. The
 *    target frame reserves exactly 0x20 bytes of locals at sp+0x10..sp+0x2F while
 *    only m[][] is ever written, and func_80052930 is a matrix-transform leaf that
 *    takes a MATRIX* — so the unwritten `pad`/`t[3]` tail is genuine dead space in
 *    the caller's own local, not a phantom slot. This is a type correction, not a
 *    frame-coercion construct: no dead stores, no unused array, no `(void)&x`.
 *
 * 2) s1/s2 SWAP (the last 7 points). GCC gave arg3 -> $s1 and arg2 -> $s2; target
 *    wants arg2 -> $s1, arg3 -> $s2. Diagnosed from the .greg dump (pseudo 74 =
 *    arg2 in hard reg 18, pseudo 75 = arg3 in hard reg 17). Both pseudos carried
 *    n_refs == 4, but arg3 dies six insns earlier than arg2 (its last use is the
 *    second `subu`, arg2's is the final `sw`), so global.c's allocno_compare —
 *    priority ~ floor_log2(n_refs)*n_refs*size / live_length — ranked arg3 first
 *    and handed it the lower-numbered callee-save.
 *
 *    THE FIX IS A CORRECTNESS FIX, NOT A REF-COUNT TRICK: func_80052930 takes
 *    THREE parameters. src/sound.c:46 already declares
 *    `extern void func_80052930(void *, void *, void *);`, the sibling
 *    camera_Transform in this same file calls it `func_80052930(&g_cam_matrix,
 *    a0, a1)` (matrix, src, dst), src/text1a_post.c:166 calls it with three
 *    arguments, and include/m2c_context.h:738 prototypes it with three. The
 *    decompilation was passing only two, dropping the destination. Restoring the
 *    destination argument (`func_80052930(&sp10, var_s0, arg2)`) makes the call
 *    agree with its own prototype and with every other call site in the tree.
 *    It costs zero instructions because arg2 is already live in $a2 from the
 *    incoming ABI at the call point, so GCC coalesces the copy away — the emitted
 *    bytes are unchanged except that arg2 now wins the allocation.
 *
 * Store order 5,6,7 is load-bearing: moving `arg2[6] = arg3;` to first or to last
 * both regress to 8 (see rejected/).
 */
void func_800470B0(s32 arg0, s32 *arg1, s32 *arg2, s32 arg3) {
    MATRIX sp10;
    s32 temp_v1;
    s32 *var_s0;

    var_s0 = arg1;
    temp_v1 = arg0 * 0x60;
    sp10.m[0][0] = 0x1000;
    sp10.m[0][1] = (s16) -((s32)(*(s16 *)((s8 *)&g_cam_fov_x + temp_v1) << 12) / *(s16 *)((s8 *)&g_cam_fov_div + temp_v1));
    sp10.m[0][2] = 0;
    sp10.m[1][0] = 0;
    sp10.m[1][1] = 0;
    sp10.m[1][2] = 0;
    sp10.m[2][0] = 0;
    sp10.m[2][1] = (s16) -((s32)(*(s16 *)((s8 *)&g_cam_fov_z + temp_v1) << 12) / *(s16 *)((s8 *)&g_cam_fov_div + temp_v1));
    sp10.m[2][2] = 0x1000;
    func_80052930(&sp10, var_s0, arg2);
    arg2[5] = var_s0[5] + (((var_s0[6] - arg3) * sp10.m[0][1]) >> 12);
    arg2[6] = arg3;
    arg2[7] = var_s0[7] + (((var_s0[6] - arg3) * sp10.m[2][1]) >> 12);
}
