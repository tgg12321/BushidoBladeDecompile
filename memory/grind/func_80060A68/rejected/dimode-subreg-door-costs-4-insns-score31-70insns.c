/* REJECTED (s8, 2026-08-19, forensics) — the DImode/SUBREG exit of birthing_insn_p is
 * measured DEAD on BYTES, not merely on rules.
 *
 * MECHANISM UNDER TEST.  tools/gcc-2.7.2/sched.c:2504-2535 (birthing_insn_p) has exactly
 * three ways to return 0 for a load insn: (a) reload_completed == 1 (sched2 only, not the
 * pass that orders this block); (b) GET_CODE (SET_DEST (pat)) != REG — the SUBREG door,
 * i.e. the load writes only part of a multi-word pseudo; (c) the dest not being in
 * bb_live_regs (only true for a dead dest, which DCE removes). Every earlier session
 * attacked door (a)'s successor `reg_n_sets[i] == 1` (the multiply-assigned carrier axis,
 * now banned for this function by the 2026-08-19 10:21 / 10:48 / 11:07 Judge rulings) and
 * dismissed door (b) on RULES grounds ("the forbidden DImode family") without measuring
 * it. This body measures it.
 *
 * THE BODY.  candidate.c with copy 2's source pointer routed through a `long long`:
 *     cpq = (long long)(u32)*(s32 *)(outer + 0xC);
 *     *(s32 *)(outer + 0x24) = *(s32 *)((s32)cpq + 4);
 * so the pointer load becomes (set (subreg:SI (reg:DI cpq) 0) (mem ...)) and cannot be
 * bumped by LAUNCH_PRIORITY.
 *
 * MEASURED (sandbox func_80060A68 --disable all, 2026-08-19 chassis): score 31,
 * build 70 / target 66.
 *
 * WHAT IT PROVES.  The door FIRES — the disassembly
 * (tmp/grind/func_80060A68/s3/d1_disasm.txt) shows copy 2's `lw a0,12(a2)` hoisted to
 * slot 10, ahead of the stage load `lw a1,16(a2)`, which is exactly the reordering the
 * carrier axis was chasing. But the DImode local costs FOUR instructions that no later
 * pass folds away: `move v0,a0` (low half), `move v1,zero` (high half — flow does NOT
 * delete it even though the high half is never read), `addiu v0,v0,4`, and a reloaded
 * `lw a0,12(a2)`. 70 instructions against a 66-instruction target.
 *
 * CONSEQUENCE.  Door (b) is not a way to close this function at ALL — not "closed by
 * policy", closed by instruction count. Combined with door (c) being unreachable for a
 * live dest and door (a) being sched2-only, the ONLY exit that can close func_80060A68
 * is `reg_n_sets[i] == 1`, i.e. a multiply-assigned carrier — the axis the Judge has now
 * banned under every spelling. Do not re-propose a wide-typed carrier in any spelling
 * (long long, double, a two-word struct or union): the extra materialisation insns are
 * the point, and they are unconditional.
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
    long long cpq;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    cpq = (long long)(u32)*(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x24) = *(s32 *)((s32)cpq + 4);
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
