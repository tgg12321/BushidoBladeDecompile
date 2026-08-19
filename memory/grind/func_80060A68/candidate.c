/* func_80060A68 candidate — session s1 (2026-08-19), honest sandbox floor 2
 * (from 39 at session start). Apply this body over src/text1b.c:3321.
 *
 * MEASURED THIS SESSION: sandbox func_80060A68 --disable all => score 2,
 * build 66 / target 66 insns, frame 0x20 matches, all registers match.
 * The residual 2 is a single adjacent swap of two independent loads:
 *   ours:   lw $2,12($3); lw $5,16($3); lw $4,12($3)   (slots 10/11/12)
 *   target: lw $2,12($3); lw $4,12($3); lw $5,16($3)
 * i.e. the staged pointer load (insn "25", priority 3) places one slot
 * before copy2's address load (insn "35", priority 3 — equal priority,
 * rank_for_schedule tie). Source-position sweep of the staged statement
 * (before copy1 / after copy1 / after copy2) measured IDENTICAL emit.
 *
 * INTEGRATION WARNING (FRAGILE ANCHOR): this body changes the first body
 * instruction from lhu $4,0($3) (old pinned C) to lhu $2,0($3). The two
 * asmfix rules for this function anchor delete_between on
 * "^lhu\t\$4,0\(\$3\)$" — applying this candidate to src WITHOUT retiring
 * or re-anchoring the asmfix rules in the same change silently mis-fires
 * the splice and duplicates the body in the full build (the func_800393C8
 * +452-byte failure mode). Session s1 therefore reverted src to HEAD after
 * measuring; re-apply from here.
 *
 * [s2 2026-08-19, structural] RE-MEASURED on today's chassis: still score 2,
 * build 66 / target 66. This body remains the best known form; s2 measured 14
 * structural respellings (copy-triple spelling x5, copy1 data-load split, stage
 * statement position x2 more, tail-block statement order x3, stage read hoisted
 * above the D_800F10D0 store) and NONE beat it. The residual is now localised to
 * one line of GCC: rank_for_schedule's LUID fall-through, sched.c:2464 � insns 25
 * (stage load) and 35 (copy2 addr load) are structurally identical, tie on both
 * INSN_PRIORITY (3) and dependence class (3), and the LUID that breaks the tie is
 * the chain order sched1 already emitted (28->25->30->35), not source order. See
 * evidence.md [s2]. Do NOT re-spend the position or copy-spelling axes.
 *
 * Contains ONE sanctioned-family construct: the staged-value load through
 * temp_a1 ([[staged-value-reused-variable]], SANCTIONED 2026-07-03), FAKE-
 * annotated below; exhaustion grid in evidence.md.
 */
void func_80060A68(void) {
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    s32 outer;
    s32 idx;
    u16 temp2;
    s32 temp_a1;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    /* FAKE: pointer ctx->unk10 staged through temp_a1 (dead until its real
       assignment below reads through and overwrites it), mechanism: sched.c
       load-late launch priority for reg_n_sets==1 destinations sinks a fresh
       single-set local's load to slot 23/26 (measured) or floats it into the
       slot-5 delay slot when hoisted above the D_800F10D0 store; target has it
       at slot 12, lever-exhaustion: memory/grind/func_80060A68/evidence.md s1 */
    temp_a1 = *(s32 *)(outer + 0x10);
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(temp_a1 + 4);
    idx = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
