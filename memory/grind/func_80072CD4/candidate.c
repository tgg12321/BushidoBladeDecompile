/* func_80072CD4 - BYTE MATCH. `sandbox func_80072CD4 --disable all` = 0, build_insns 79 ==
 * target_insns 79, rules_dropped 0. Re-measured on the current chassis this session
 * (2026-08-20, forensics respawn; artifact tmp/grind/func_80072CD4/s5f2/sandbox_rgb_s5f2.json).
 *
 * WHAT THE BODY IS. A POLY_G4 colour setter. arg1's byte offsets are the canonical PSX libgpu
 * POLY_G4 vertex-colour layout: rgb0 = 0x04/0x05/0x06, rgb1 = 0x0C/0x0D/0x0E,
 * rgb2 = 0x14/0x15/0x16, rgb3 = 0x1C/0x1D/0x1E (the +0x24 return is the primitive's size).
 * Each inner branch assigns its OWN complete rgb0 and rgb1 triple in ascending field order;
 * the two unconditional rgb2/rgb3 triples follow. The red component of rgb0/rgb1 happens to be
 * 0xFC on both branches - that is what the game displays on both paths, not a shared temporary.
 * The body declares NO variable of any kind: no local, no holder, no volatile, no __asm__, no
 * pin, no barrier, no do-while, no dead store, no annotation. Twenty-four live field writes.
 *
 * CLASSIFICATION IS SETTLED BY A JUDGE RULING, NOT BY THIS SESSION. docs/grind/decisions.md:8456
 * (2026-08-20 06:09, on main as a8d7ee5f) = **PASS**: "The banned lever is the fc_const-holder
 * mid-arm injection (rejected/dup4_0xc_into_arms.c) and stays banned; that ban does not reach a
 * body with no holder and no hoist." In-repo house-style precedent verified by that ruling and
 * again here: the COMPLETED-C sibling func_80072BC4 (src/text1b.c:5822, zero rules, absent from
 * engine/queue.json) itself writes the cross-arm duplicate `*(u8 *)((s32)(arg1) + 0x1D) = 0xC3;`
 * in BOTH arms at src/text1b.c:5840 and :5843 without hoisting it.
 *
 * FORENSICS (this session, tmp/grind/func_80072CD4/dumps/text1b.{sched2,jump2}) - the source
 * order is NOT the emitted merge order, so the per-arm spelling cannot be a merge-order lever:
 *  - Source order inside each arm is canonical ascending 4,5,6,C,D,E.
 *  - By the END of sched2 each arm has been re-ordered to 5,6,D then 4,C,E: the producer-less
 *    stores (@4/@0xC from the shared `li v1,0xFC`, @0xE from the arm's own li) sink to the arm
 *    tail under sched.c's bottom-up schedule_block, independently and identically in both arms
 *    (THEN insns 40/55/65, ELSE insns 75/90/100).
 *  - jump2 then cross-jumps that identical 3-insn tail, deleting the THEN copies and inserting a
 *    new join label (code_label 223, label 872) ahead of insns 75/90/100 - which is exactly
 *    target's merge head `sb v1,4 / sb v1,0xC / sb v0,0xE`.
 * The merge order is therefore manufactured by sched2 out of canonical field order; no statement
 * ordering in the C selects it. (Prior sessions' complementary result: with the two red writes
 * hoisted into a shared tail behind an `int fc_const` holder - fallback_floor4.c - sched2 sinks
 * them to the MERGE block's tail instead and the score is 4. That holder form is the artificial
 * spelling, and it is the one that had to be searched for.)
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 4) = 0xFC;
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x1E;
            *(u8 *)((s32)(arg1) + 0xC) = 0xFC;
            *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
            *(u8 *)((s32)(arg1) + 0xE) = 0x32;
        } else {
            *(u8 *)((s32)(arg1) + 4) = 0xFC;
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x50;
            *(u8 *)((s32)(arg1) + 0xC) = 0xFC;
            *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
            *(u8 *)((s32)(arg1) + 0xE) = 0x46;
        }
        *(u8 *)((s32)(arg1) + 0x14) = 0xFC;
        *(u8 *)((s32)(arg1) + 0x15) = 0x82;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        *(u8 *)((s32)(arg1) + 0x1C) = 0x32;
        *(u8 *)((s32)(arg1) + 0x1D) = 0x28;
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
