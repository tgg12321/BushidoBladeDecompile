/* REJECTED (s1, 2026-07-30) — KILLED as a lever for the D_80101E70 reload.
 *
 * Idea: target stores D_80101E62 through a register-held address
 * (`sh $a0, 0($t0)`) rather than `%lo(sym)($at)`. If that store's address were
 * opaque to CSE, cse.c:7539 `note_mem_written` would set `writes_ptr->all = 1`
 * (a varying address that is neither MEM_IN_STRUCT_P nor a PLUS-form
 * array-element address invalidates ALL memory equivalences), which would kill
 * the recorded D_80101E70 store equivalence and force the genuine reload that
 * target emits — WITHOUT needing the volatile qualifier.
 *
 * MEASURED: sandbox --disable all -> score 17, build_insns 36. Identical to the
 * alias-free candidate. No reload appears anywhere in the object.
 *
 * WHY IT CANNOT WORK: cse_insn canonicalizes and fold_rtx-folds the destination
 * MEM's address BEFORE note_mem_written inspects it. The address pseudo carries
 * `(expr_list:REG_EQUIV (symbol_ref:SI ("D_80101E62")))` — visible as insn 13 in
 * this session's .greg dump — so the address folds to a constant symbol_ref and
 * `cse_rtx_addr_varies_p` returns false. This closes the ENTIRE family, not just
 * this spelling: any C-level pointer whose initializer GCC can constant-fold
 * will be folded away for aliasing purposes and will never invalidate memory.
 *
 * SEPARATELY (do not confuse the two results): this alias IS worth -1 on the
 * register/scheduling axis once the reload is present by other means
 * (--keep-cheat-asm: 14 without it, 13 with it, both at 38 insns), because it
 * reproduces target's single materialized address reused across the branch
 * ($t0 in target). It remains non-committable in this bare form — s0's
 * cheat-reviewer FAILed it as an unannotated pointer-alias-fake-exception.
 */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 sval;
    s16 *s0 = &D_80101E62;

    if (*s0 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    {
        extern u8 SpecialCam;
        s32 cam_val;
        s32 ec_val;
        cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
        D_80101E6C = cam_val;
        D_80101E7C = a1;
        ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E70 = ec_val;
    }
    D_80101E68 = 0;
    *s0 = 2;
    D_80101E9E = 0;
    D_80101E78 = (u32)(D_80101E70 + 0x7FF) >> 11;
    return 1;
}
