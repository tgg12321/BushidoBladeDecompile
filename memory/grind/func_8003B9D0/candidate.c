/* candidate.c — func_8003B9D0 — best form after session 2 (structural).
 *
 * FLOOR: sandbox --disable all == 6  (build_insns 188 vs target 185).
 * Session 1 floor was 21 (build_insns 178).  This form CLOSES REGION B
 * entirely — the normalized diff is now 58 equal head insns, three isolated
 * 2-insn-vs-1-insn replacements (region A), and a 113-insn byte-equal tail.
 *
 * It also carries ZERO cheat-asm: the session-1 HEAD's identity-reload barrier
 * on `eda` and the two `__asm__ __volatile__("" ::: "memory")` barriers are all
 * DELETED and are no longer needed — the honest C below reproduces what they
 * were faking.  (The regfix rule `func_8003B9D0: fill_delay @ 49 <- 52`,
 * regfix.txt:1116, is untouched; grind sessions may not edit regfix.txt.)
 *
 * THE ONE CHANGE THAT DID IT (region B, the whole 7-insn shortfall):
 *   writing the two flag-selected argument initialisations as if/ELSE
 *
 *       if (((u8 *)D_800A3878)[3] & 0x1) a3_arg = D_80101EDA; else a3_arg = -1;
 *
 *   instead of the "init to -1, then conditionally overwrite" form
 *
 *       a3_arg = -1;
 *       if (((u8 *)D_800A3878)[3] & 0x1) a3_arg = D_80101EDA;
 *
 *   Mechanism (measured, see hypotheses.md H1): an `else` arm makes GCC emit a
 *   jump around it, so the join label is preceded by a BARRIER and by a real
 *   arm block.  cse.c's `cse_end_of_basic_block` (tools/gcc-2.7.2/cse.c:8039)
 *   ends a CSE basic block at every CODE_LABEL, and only extends past one via
 *   the follow-jumps / skip-blocks branch at :8102-8184.  The plain-`if` form
 *   satisfies that extension (AROUND status), so one CSE table spanned all
 *   three flag reads and cc1 emitted the lui/lw/lbu block ONCE.  The if/else
 *   form ends the block at each join, so each read gets a fresh CSE table and
 *   cc1 re-emits the full lui/lw/nop/lbu/nop reload — exactly target's
 *   tgt[72..77] / tgt[82..87] / tgt[92..96].
 *
 *   Both forms are ordinary, semantically identical C that a human writes
 *   without thinking about the compiler; the if/else is if anything the more
 *   natural spelling of "this argument is either the global or -1".  Nothing
 *   here is dead, nothing is unused, nothing exists only to steer codegen.
 *
 * WHAT REMAINS (region A, score 6 = 3 sites x 2):
 *   ours   `lui $x,%hi(D_80101EDA+0x44C) / lh|sh $r,%lo(...)($x)`   (2 insns)
 *   target `lh|sh $r, 0x44C($s0)`                                   (1 insn)
 *   at the three `eda[0x226]` sites.  See hypotheses.md F1 for the exact cc1
 *   guard (`find_best_addr`, cse.c:2663) and the next probe.
 *
 * SESSION-3 NOTE (2026-08-11) — this body is UNCHANGED and still the floor
 *   (sandbox --disable all == 6, re-measured session 3), but region A is no
 *   longer a mystery: it CLOSES (target's `lh/sh $r,1100($base)` at all three
 *   sites, build_insns 185 == target 185) as soon as a cse basic-block boundary
 *   separates `eda`'s definition from its displaced uses — see
 *   rejected/cse-boundary-diamond-closes-region-a-but-moves-magic-and-la.c,
 *   which scores 11 only because the boundary it uses (spelling the 0x80 test
 *   as if/ELSE) relocates `magic = 0x80190800` out of the prologue (~7 pts) and
 *   forces the `la` out of the qf-block (~4 pts).  Ten access/type spellings
 *   were measured and ALL fold (hypotheses.md K8), and the 30 matched siblings
 *   with this addressing shape use no special spelling either (K9) — so do NOT
 *   spend another session respelling the access.  The open problem is a CHEAPER
 *   BOUNDARY; hypotheses.md session-3 F1 names the exact cse.c conditions to
 *   enumerate.
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
            saved_44c = eda[0x226];
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
