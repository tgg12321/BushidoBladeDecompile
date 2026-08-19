/* func_80060A68 — RULING-REQUEST BODY (session s7, forensics, 2026-08-19).
 * MEASURED: `sandbox func_80060A68 --disable all` => score 0, build 66 / target 66,
 * on the 2026-08-19 chassis (measured twice; disassembly in
 * tmp/grind/func_80060A68/s3/z3_disasm.txt, cc1 -da dump set in
 * tmp/grind/func_80060A68/s3/z3dumps/).
 *
 * WHY THIS IS NOT A RESPELLING OF THE BANNED CARRIER.  Every previously measured
 * score-0 body closed copy 2's address load by reloading the SAME pointer twice from
 * `outer + 0xC` with no intervening write (banned construct 1: `src`, `temp2`, and
 * y3's `idx`).  This body loads `outer + 0xC` exactly ONCE for copy 2 and then does
 * ordinary pointer arithmetic on it:
 *
 *     cp = *(s32 *)(outer + 0xC);
 *     cp += 4;
 *     *(s32 *)(outer + 0x24) = *(s32 *)cp;
 *
 * i.e. the same-variable SPLIT-INIT ACCUMULATION shape (`var = a; var += b;` instead
 * of `var = a + b;`) that the owner sanctioned provisionally on 2026-06-13
 * ([[split-init-accumulation-sanctioned]], which closed func_80049C24 in this same
 * file, commit ad11a8c8).  GCC's `combine` folds the `+= 4` back into the load's
 * displacement — the emitted insn is `lw $v0,4($a0)`, the instruction count stays at
 * 66, and NOTHING about the split is visible in the object code.  Confirmed in the RTL:
 * tmp/grind/func_80060A68/s3/z3dumps/text1b.sched shows exactly one set of reg/v 77
 * (insn 34) and the consumer as `(mem (plus (reg/v 77) 4))` (insn 40).
 *
 * THE CONTROL THAT ISOLATES THE LEVER.  z6 keeps the `cp` local and its single load
 * but writes `*(s32 *)(cp + 4)` in one statement — score 2, 66 insns
 * (rejected/z6-control-cp-without-split-init-single-set-still-bumped-score2.c).
 * The `cp += 4;` line, folded away by combine, is worth exactly the last two
 * instructions.  The mechanism is the one this ledger has documented since s3:
 * GCC 2.7.2's `adjust_priority`/`birthing_insn_p` (tools/gcc-2.7.2/sched.c:2504-2592)
 * bumps a ready insn to LAUNCH_PRIORITY when its destination pseudo has
 * `reg_n_sets == 1`; the split-init leaves that count at 2 (the same stale-count
 * behaviour the 2026-06-13 sanction documents for `reg_n_refs`), so copy 2's address
 * load is not bumped and the three unbumped loads emit in source/LUID order —
 * target's order.
 *
 * THE OTHER CONSTRUCT.  Copy 1 is staged through the pre-existing `result`
 * (`result = ptr; result = *ptr; store result;`).  That is the staged-value-reused-
 * variable form already present, unbanned, in candidate.c (where `temp_a1` does the
 * identical thing for the 0x10 pointer), hosted on a pre-existing local whose real job
 * is the dispatch call's return value.  It is also target's own register flow: `$v0`
 * is loaded with copy 1's source pointer and then overwritten by the loaded word
 * (`lw v0,12(v1)` / `lw v0,0(v0)`).
 *
 * WHY IT IS A RULING REQUEST AND NOT A SUBMISSION.  `cp` is a FRESH local that is
 * written twice, and the 2026-08-19 10:21 Judge ruling calls "fresh AND multi-write"
 * an excluded quadrant.  The split-init sanction is a 2026-06-13 owner directive that
 * does NOT appear on the frozen SOTN family list, and its own text says new adjacent
 * spellings need their own ruling — here the SHAPE is identical but the GCC mechanism
 * it exploits is the scheduler's reg_n_sets rather than global.c's reg_n_refs.  A
 * grind session may not self-approve that.  Question filed in docs/grind/decisions.md.
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
    s32 cp;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    result = *(s32 *)(outer + 0xC);
    result = *(s32 *)(result + 0);
    *(s32 *)(outer + 0x20) = result;
    cp = *(s32 *)(outer + 0xC);
    cp += 4;
    *(s32 *)(outer + 0x24) = *(s32 *)cp;
    temp_a1 = *(s32 *)(outer + 0x10);
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
