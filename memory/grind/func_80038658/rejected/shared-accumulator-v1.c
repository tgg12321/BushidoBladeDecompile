/* REJECTED — shared single-accumulator goto form (m2c-shape, de-pinned).
 * Measured: sandbox --disable all = 9 (draft with direct D_800A379E=5) /
 * 10 (with sense-corrected branches). 55/55 insns, structure fully matched,
 * but the ONE multi-def accumulator pseudo (defs 1,2,3,4,6,15 + uses at the
 * shared sh labels) CONFLICTS WITH HARD REG 2 in .greg ("73 conflicts: 73 74
 * 2 29") and is punted to $v1 -> 7 li + 3 sh rename diffs. The
 * call-return-init lever (var_v0 = func(); var_s0 = var_v0;) does NOT fix it:
 * cse collapses the staging copy and the conflict remains.
 * The winning fix is NOT to have a shared accumulator at all: per-arm direct
 * stores give independent one-shot temps that each take $v0 (see candidate.c).
 *
 * Also REJECTED (worse, =27): the old-chassis m2c spelling that re-reads the
 * accumulator as the compare constant (`var_v0 = 1; if (var_s0 != var_v0)`)
 * — reproduces the pre-migration floor 27 exactly; the compare-reuse extends
 * the accumulator across close() into callee-saves.
 */
void func_80038658_rejected(void) {
    s32 var_v1;
    s32 var_v0;
    s32 var_s0;

    var_v1 = D_800A31F4;
    if (var_v1 == 4) goto state_4;
    if (var_v1 == 6) goto state_6;
    return;

state_4:
    var_s0 = func_800378A8();
    var_v0 = 1;
    if (var_s0 == 0) goto store_only;
    close(D_800A3794);
    if (var_s0 != 1) {
        var_v0 = 3;
        goto finish;
    }
    var_v0 = 2;
    goto finish;

state_6:
    var_s0 = func_800378A8();
    var_v0 = 4;
    if (var_s0 == 0) goto store_only;
    close(D_800A3794);
    if (var_s0 != 1) {
        var_v0 = 6;
        goto finish;
    }
    D_800A379E = 5;
    if (func_8003800C(&D_800F34D8) != 0) goto clear;
    var_v0 = 0xF;
finish:
    D_800A379E = var_v0;
clear:
    D_800A31F4 = 0;
    return;
store_only:
    D_800A379E = var_v0;
}
