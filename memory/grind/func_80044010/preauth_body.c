/* func_80044010 -- pre-authorization C body, banked 2026-10-01 at de-authorization (owner ruling,
 * docs/grind/decisions.md "inline-asm audit"). Measured: sandbox func_80044010 --disable all
 * --candidate <this file> -> honest floor in migration_pin.json. Volatile frame pads removed;
 * any asm barrier is stripped by the sandbox. Starting context only, not a vetted candidate.
 * MEASUREMENT SHIM: src/text1a_c.c declares `extern void func_80044010(s32, s32);`, so the
 * parameters arrive as raw s32 and are narrowed to the original `s32 *a0` / `s16 a1` locals --
 * a measurement aid matching the current prototype, not a proposed body. Reproduces floor 3
 * (target 34, build 34). */
void func_80044010(s32 a0_raw, s32 a1_raw) {
    s32 *a0 = (s32 *)a0_raw;
    s16 a1 = a1_raw;
    s32 new_var;
    s32 *a2 = a0;
    s32 v1 = *a0;
    unsigned short v0;
    *a0 = (v1 | 0x8000) & 0xFFFF;
    a0 = a0 + 1;
    D_80103608[a1] = a0;
    D_80103658[a1] = v1 & 0x7FFF;
    v0 = v1;
    if (!(v1 & 0x8000)) {
        v1 = v0 & 0xFFFF;
        new_var = v1;
        if (new_var > 0) {
            s32 i = 0;
            do {
                *a0 += (s32)a2;
                i++;
                a0++;
            } while (i < new_var);
        }
    }
}
