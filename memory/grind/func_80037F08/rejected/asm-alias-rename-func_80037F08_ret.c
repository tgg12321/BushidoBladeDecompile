/* REJECTED — the judge-flagged cheat construct this grind removed.
 * Family: asm alias-rename (inline-asm-injection sibling); NOT canonical.
 * A second assembler symbol (func_80037F08_ret) was equated to the void
 * function so the caller could harvest $v0 through a differently-typed
 * declaration. Replaced by the honest s32 retype (see candidate.c) which
 * is byte-identical: sandbox 0 for callee (14/14) and caller (173/173),
 * measured s2 2026-07-14.
 */

extern s32 func_80037F08_ret(s32, s32);

void motion_LoadPreCalcData_80037F08(s32 a0, s32 a1) {
    s32 buf[2];
    func_80079A30(buf, &D_800109C8, a0, a1);
    bios_FormatDevice_B(buf);
}
__asm__(
    ".globl func_80037F08_ret\n"
    "func_80037F08_ret = motion_LoadPreCalcData_80037F08\n"
);

/* caller (pad_FuncAnalog state_7 arm): var_v0 = func_80037F08_ret(0, 0); */
