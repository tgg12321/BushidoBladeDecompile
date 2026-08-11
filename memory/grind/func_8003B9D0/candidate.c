/* candidate.c — func_8003B9D0 — session 4 (permuter).  HONEST DISTANCE 0.
 *
 * FLOOR: `sandbox func_8003B9D0 --disable all` == **0**
 *        (target_insns 185, build_insns 185, rules_dropped 1, cheat-asm
 *        stripping ON), measured this session with this body in
 *        src/code6cac_c2.c.  Session-3 floor was 6; sessions 1/2/3 were
 *        21 -> 6 -> 6.
 *
 * The function's body carries ZERO cheat-asm (session 2 deleted the three
 * `__asm__` constructs and they stayed deleted).  ONE regfix rule still exists
 * in regfix.txt for this function (`func_8003B9D0: fill_delay @ 49 <- 52`,
 * regfix.txt:1116); grind sessions may not touch regfix.txt, and the sandbox
 * drops it (rules_dropped 1) — so the 0 above is the honest, cheat-free
 * distance and the rule is now dead weight for the operator to retire.
 *
 * WHAT CLOSED REGION A (the last 6 points, three `+0x44C` sites where ours
 * emitted `lui $x,%hi(D_80101EDA+1100)` + `lh|sh $r,%lo(...)($x)` and target
 * emits `lh|sh $r,1100($s0)`):
 *
 *     p = (u8 *)&eda[0x226];        <- the far halfword's address staged in the
 *     saved_44c = *(s16 *)p;           function's existing scratch pointer
 *
 * instead of `saved_44c = eda[0x226];`.  With that ONE statement pair, all
 * THREE displaced sites (the read and both `sh`s, which still spell
 * `eda[0x226]`) keep target's register+displacement addressing, and
 * build_insns drops 188 -> 185 == target.
 *
 * Provenance: the shape came out of the session-4 permuter campaign on the
 * floor-6 chassis (tmp/grind/func_8003B9D0/s4/wsA, output-200-2, permuter
 * score 330 -> 200); it was a PROPOSAL — the permuter's own form read a BYTE
 * through `p` (semantically wrong) and its sibling output-200-1 was outright
 * UB (uninitialised `eda`).  The halfword-correct form above was derived from
 * it and re-measured (tmp/grind/func_8003B9D0/s4/sweep2.py, sweep3.py).
 *
 * Mechanism: cse's `find_best_addr` folds any non-REG address unconditionally
 * (cse.c:2663) by substituting the base pseudo's `qty_const` in `fold_rtx`
 * (cse.c:5171-5176).  Staging the derived address through the longer-lived
 * scratch pointer keeps `(plus (reg eda) 1100)` alive as a register-equivalent
 * value, and the fold no longer materialises the symbol at any of the three
 * sites.  A DEDICATED pointer local does not do it: a block-scope `s16 *far`
 * (init-at-decl, split decl/assign, assigned before or after the zero-offset
 * read, `u8 *` + cast) all still fold or cost 3 extra insns — see
 * hypotheses.md session-4 K10/H4 for the measured table.
 *
 * The construct is `/* FAKE *\/`-annotated in src (sanctioned family: variable
 * reuse for codegen control, .claude/rules/no-new-park-categories.md:170); the
 * self-vet is memory/grind/func_8003B9D0/self_vet.md.
 *
 * Region B (the earlier 7-insn shortfall) is closed by the session-2 if/else
 * spelling of the two flag-selected argument initialisations, unchanged here.
 */

void func_8003B9D0(void) {
    s32 saved_first;
    s32 saved_44c;
    s32 a3_arg;
    s32 a0_arg;
    s32 magic;
    s32 v0;
    u8 *p;
    u8 flags;

    magic = 0x80190800;
    func_8001DA2C();
    game_Cleanup();
    if (D_800A3768 != 0x14) gpu_InitDisplay();
    if (D_800A3768 != 0xFF) gpu_DisableDisplay();
    func_800174F4();
    gpu_EnableDisplay();
    func_80020D38();
    disp_SetFramebufferMode(1, 0, 0, 0);
    if (((u8 *)D_800A3878)[3] & 0x80) {
        func_80020CDC();
        magic = 0x80118800;
    }
    {
        u8 *q = (u8 *)D_800A3878;
        u8 qf = q[3];
        if (qf & 0x30) {
            s16 *eda = &D_80101EDA;
            saved_first = eda[0];
            /* FAKE: the +0x44C halfword is reached through the scratch pointer
               already declared above instead of directly as eda[0x226];
               mechanism: cse.c find_best_addr's unconditional address fold
               (cse.c:2663) substituting the base pseudo's qty_const in fold_rtx
               (cse.c:5171-5176), which the staged pointer defeats;
               lever-exhaustion: memory/grind/func_8003B9D0/hypotheses.md
               K1/K2/K5/K8/K9 (10 access/type spellings + 30-sibling census). */
            p = (u8 *)&eda[0x226];
            saved_44c = *(s16 *)p;
            if (qf & 0x10) eda[0] = 0x32;
            if (q[3] & 0x20) eda[0x226] = 0x32;
            func_8003AFFC();
            eda[0] = saved_first;
            eda[0x226] = saved_44c;
        }
    }
    if (((u8 *)D_800A3878)[3] & 0x1) a3_arg = D_80101EDA; else a3_arg = -1;
    if (((u8 *)D_800A3878)[3] & 0x2) a0_arg = D_80102326; else a0_arg = -1;
    p = (u8 *)D_800A3878;
    flags = p[3];
    if (flags & 0x10) a3_arg = 0x32;
    if (flags & 0x20) a0_arg = 0x32;
    D_800A390F = 0;
    func_80054884(D_800A376C, p[0], 0, a3_arg, a0_arg, -1, -1, magic);
    func_80041688(0, 0);
    func_80041688(1, 0);
    if (((u8 *)D_800A3878)[3] & 0x40) func_8004659C(-1);
    if (D_8010277D == 0xE || D_8010277D == 0x1D) {
        func_80041BF4(D_800A37B4, D_800A37B5, D_800A37B6);
    }
    func_8001DBE4();
    D_800A3768 = 0xFF;
    D_800A36A8 = 0;
    func_80035FA8();
    v0 = func_80036EA8(5, ((u8 *)D_800A3878)[1]);
    func_80036FD4(v0, ((u8 *)D_800A3878)[2]);
    func_80037260();
    D_800A37B8 = 0;
    D_800A3834 = 7;
    gpu_DisableDisplay();
}
