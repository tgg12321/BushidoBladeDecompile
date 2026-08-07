/* candidate.c — motion_LoadPreCalcData_80037F08 (s2, structural)
 * APPLIED IN src/ AND MEASURED s2 (2026-07-14) — sandbox 0 for callee (14/14)
 * AND caller pad_FuncAnalog (173/173), with the edits live in the working
 * tree this session. (A prior attempt's src/ edits had been reverted by
 * grinder hygiene before scoring; this session re-applied and re-measured
 * with the edits verifiably in place — grep confirmed the alias was back
 * at :180/:189-194/:555 at session start, then re-edited and scored 0.)
 *
 * The 4-part retype from the s1 frontier:
 *   1. src/code6cac_c_mid.c — deleted `extern s32 func_80037F08_ret(s32, s32);`
 *      and the file-scope __asm__ alias block (.globl + equate).
 *   2. src/code6cac_c_mid.c — function retyped void -> s32 with `return` on the
 *      final BIOS call (body below).
 *   3. src/code6cac_c_mid.c — call site in pad_FuncAnalog state_7 arm now calls
 *      the canonical name directly: var_v0 = motion_LoadPreCalcData_80037F08(0, 0);
 *   4. include/code6cac.h — extern void -> extern s32 bios_FormatDevice_B(s32 *);
 *      (single call site project-wide; PSX BIOS B-table returns status in $v0,
 *      so s32 is the TRUE signature).
 *
 * Measured s2: sandbox motion_LoadPreCalcData_80037F08 --disable all => 0 (14/14)
 *              sandbox pad_FuncAnalog                  --disable all => 0 (173/173)
 * Zero regfix/asmfix rules; zero cheat-asm; the judge constraint (alias
 * removal) is cleared. Driver re-verifies full-build bytes.
 * Artifacts: tmp/grind/motion_LoadPreCalcData_80037F08/s2/
 * Rejected cheat form: rejected/asm-alias-rename-func_80037F08_ret.c
 */

s32 motion_LoadPreCalcData_80037F08(s32 a0, s32 a1) {
    s32 buf[2];
    func_80079A30(buf, &D_800109C8, a0, a1);
    return bios_FormatDevice_B(buf);
}
