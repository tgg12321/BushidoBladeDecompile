/* REJECTED (s3b, 2026-08-19) — score 10, 68 insns (candidate scores 0 at 66).
 * This is the zero-scoring candidate with construct (4) removed: temp2 still
 * carries copy 2's source pointer, but the 0x1A halfword read is written back
 * inline instead of through temp2. Two extra instructions and a 10-point
 * distance. Banked because it is EVIDENCE, not just a dead end: it proves
 * temp2's named-intermediate job is independently load-bearing, which is what
 * bound 2 of staged-value-reused-variable requires of a borrowed local.
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
    s32 temp2;
    s32 temp_a1;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    /* FAKE: copy 1's source pointer, and then the word it points at, are staged
       through the function's existing `result` local (its previous value is dead
       here — nothing reads `result` until the dispatch call below overwrites it),
       mechanism: GCC 2.7.2 sched.c adjust_priority -> birthing_insn_p, whose
       LAUNCH_PRIORITY bump is gated on reg_n_sets[regno] == 1, so a fresh
       single-set destination is picked early in the backward list schedule and
       therefore EMITTED late; a multiply-set destination keeps its honest
       priority, lever-exhaustion: memory/grind/func_80060A68/evidence.md
       (s1 v3-v13, s2 v20-v33, s3 v40-v46, s3-permuter w1-w5) */
    result = *(s32 *)(outer + 0xC);
    result = *(s32 *)(result + 0);
    *(s32 *)(outer + 0x20) = result;
    /* FAKE: copy 2's source pointer is carried by the existing `temp2` local
       (widened to s32; it is written again below with the 0x1A halfword and is
       dead in between), so that copy 2's address load also loses the
       birthing_insn_p LAUNCH_PRIORITY bump — same sched.c mechanism as above.
       With both it and the staged 0x10 load unbumped, sched2's LUID tie-break
       (sched.c:2464) emits them in source order, which is target order,
       lever-exhaustion: evidence.md s3-permuter w1-w5 (w2 kills the
       both-bumped alternative; K11 kills the `idx` borrow) */
    temp2 = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x24) = *(s32 *)(temp2 + 4);
    /* FAKE: the 0x10 pointer is staged through temp_a1, which its own next read
       (`temp_a1 = *(u16 *)(temp_a1 + 4)`) consumes and overwrites; same reg_n_sets
       mechanism, and the statement sits AFTER copy 2 so that its RTL LUID is above
       copy 2's address load, lever-exhaustion: evidence.md s1 K2 / s2 K10 / s3 */
    temp_a1 = *(s32 *)(outer + 0x10);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    /* FAKE: the 0x1A halfword is read into `temp2` here, above the D_800A3478
       store, and stored below it, mechanism: GCC 2.7.2 sched.c cannot
       disambiguate a %gp_rel symbol store from a base-register load, so no load
       written after that store can schedule above it; target's store order
       requires this read to precede it, lever-exhaustion: evidence.md s1 v8/v9 */
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
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
