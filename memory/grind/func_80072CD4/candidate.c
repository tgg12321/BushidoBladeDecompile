/* func_80072CD4 - MATCHED. Session s17 (2026-09-04, synthesis) landed this body in
 * src/text1b.c EXACTLY as cleared and measured it in place:
 *   `sandbox func_80072CD4 --disable all` = 0, build_insns 79 == target_insns 79,
 *   rules_dropped 0;  `verify-oracle` = ok true / build_matches true (full-build
 *   SHA1 == the oracle 62efab4f73f992798c43e8c730aa43baa10bb4fa).
 *
 * AUTHORITY. The body is byte-identical (comments aside) to
 * tmp/grind/func_80072CD4/s16/A2_dup_arms_ascending.c, which carries a Judge PASS
 * clearance dated 2026-09-04 01:32 (docs/grind/decisions.md:22248). That ruling
 * narrowed banned_constructs entry 5 / judge_constraints entry 1 (dup4_0xc_into_arms):
 * they do NOT reach a device-free, unannotated, ascending-order per-arm triple body.
 * It also states verbatim that no FAKE annotation is owed here - this is
 * ordinary C, not an exception-family use - and that adding one would be a defect.
 * Six-test vet: memory/grind/func_80072CD4/self_vet.md.
 *
 * WHY THIS IS THE ORIGINAL SOURCE SHAPE. Each arm of the inner `if` writes its own
 * complete vertex-0 and vertex-1 RGB triple in ascending field order (4,5,6,0xC,0xD,0xE)
 * - the same shape as the outer else-arm below it and as the COMPLETED-C sibling
 * func_80072BC4 in this same file. Target's arm 1 ends `j .L80072D64 / addiu $v0,$zero,0x32`,
 * a constant load with no in-block consumer, which GCC 2.7.2's sched1 can only leave at an
 * arm tail if it has a real in-block successor; the only successor that survives sched1 and
 * still costs zero final bytes is a store that jump.c's cross_jump later merges into the
 * join (asm/funcs/func_80072CD4.s emits @4/@0xC/@0xE exactly once, at .L80072D64, lines
 * 40-42). The arms of the original source ended with these stores.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int red;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        red = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 4) = red;
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x1E;
            *(u8 *)((s32)(arg1) + 0xC) = red;
            *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
            *(u8 *)((s32)(arg1) + 0xE) = 0x32;
        } else {
            *(u8 *)((s32)(arg1) + 4) = red;
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x50;
            *(u8 *)((s32)(arg1) + 0xC) = red;
            *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
            *(u8 *)((s32)(arg1) + 0xE) = 0x46;
        }
        *(u8 *)((s32)(arg1) + 0x14) = 0xFC;
        *(u8 *)((s32)(arg1) + 0x15) = 0x82;
        *(u8 *)((s32)(arg1) + 0x1C) = 0x32;
        *(u8 *)((s32)(arg1) + 0x1D) = 0x28;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0xA;
    } else {
        *(u8 *)((s32)(arg1) + 4) = 0x10;
        *(u8 *)((s32)(arg1) + 5) = 0x30;
        *(u8 *)((s32)(arg1) + 6) = 0x60;
        *(u8 *)((s32)(arg1) + 0xC) = 0x18;
        *(u8 *)((s32)(arg1) + 0xD) = 0;
        *(u8 *)((s32)(arg1) + 0xE) = 0x40;
        *(u8 *)((s32)(arg1) + 0x14) = 0x30;
        *(u8 *)((s32)(arg1) + 0x15) = 0;
        *(u8 *)((s32)(arg1) + 0x16) = 0x60;
        *(u8 *)((s32)(arg1) + 0x1C) = 0;
        *(u8 *)((s32)(arg1) + 0x1D) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
