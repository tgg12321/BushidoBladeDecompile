/* func_80060A68 — y3: the ROLE-PERMUTATION form.  MEASURED score 0 (build 66 /
 * target 66) under `sandbox func_80060A68 --disable all` on the 2026-08-19 chassis.
 *
 * ***** DO NOT SUBMIT THIS BODY UNTIL A RULING EXISTS.  IT IS UNCLASSIFIED. *****
 *
 * WHAT IT IS.  The function declares the same five locals it has always declared
 * (`outer`, `idx`, `temp2`, `temp_a1`, `result`).  No local is added, none is removed,
 * and every one of them is written and read.  What changed relative to the ban-free
 * floor-2 candidate is only WHICH local carries WHICH of the function's values:
 *
 *     local      floor-2 candidate                  y3
 *     outer      the D_800A3468 base                unchanged
 *     result     the dispatch call's return         copy 1's source pointer, then the
 *                                                   word loaded through it, then the
 *                                                   dispatch call's return
 *     idx        the character index (late read)    copies 2 and 3's source pointer
 *                                                   (loaded twice, once per copy)
 *     temp2      the 0x1A halfword                  the 0x1A halfword, then the
 *                                                   character index (late read)
 *     temp_a1    the 0x10 pointer, then the         unchanged
 *                halfword read through it
 *
 * WHY IT CLOSES.  Documented in evidence.md [s6].  Copy 2's address load must lose
 * GCC 2.7.2's birthing_insn_p LAUNCH_PRIORITY bump (sched.c:2504-2535, whose only live
 * exit is `reg_n_sets[i] == 1`), which requires its destination pseudo to be assigned
 * twice; and that pseudo must ALSO be local-allocated to $a0, which (measured this
 * session, y1 vs y2) requires its live range not to reach the late-index job.  `idx`
 * satisfies both once the late-index job is moved off it, and the only local that can
 * absorb the late-index job without breaking its own register is `temp2`, whose 0x1A
 * halfword also lives in $a0 and dies immediately before the late index is read.
 *
 * WHY IT IS UNCLASSIFIED RATHER THAN BANKED.  The `idx` block here is textually the
 * banned construct 1 with the identifier changed:
 *     idx = *(s32 *)(outer + 0xC);  ...  idx = *(s32 *)(outer + 0xC);
 *     *(s32 *)(outer + 0x28) = *(s32 *)(idx + 8);
 * Ban 1 forbids exactly that shape spelled `src`, and the Judge's 2026-08-19 ruling
 * says "no further respelling of the fresh multiply-assigned carrier under any name or
 * family".  The countervailing fact is that nothing here is FRESH: the local count is
 * unchanged, no variable was invented, and the layer-1 FAIL's own remedy (a) was "find
 * an honest pre-existing local the function already uses for a real job".  Whether a
 * pure permutation of jobs across the existing locals is that remedy or is the banned
 * construct wearing an existing name is a question this session cannot answer for
 * itself, so it is asked as a ruling-request instead of submitted.
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
    result = *(s32 *)(outer + 0xC);
    result = *(s32 *)(result + 0);
    *(s32 *)(outer + 0x20) = result;
    idx = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x24) = *(s32 *)(idx + 4);
    temp_a1 = *(s32 *)(outer + 0x10);
    idx = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x28) = *(s32 *)(idx + 8);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(temp_a1 + 4);
    temp2 = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + temp2) + *(s32 *)((s32)&D_800F10D0 + temp2 * 4)) * 4))(temp2, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
