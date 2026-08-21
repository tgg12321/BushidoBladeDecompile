/* func_800858D0 (title_mv_exec2) — BYTES PROVEN (s3, 2026-08-20, permuter
 * modality): permuter score 0 vs target.o (--stack-diffs, honest pipeline),
 * TU-context emission word-identical (72/72, tmp/grind/func_800858D0/s3/
 * ours_tu.dis vs zero.dis — only section-offset branch text differs).
 * NOT candidate-ready: engine/volatile_cheats.py find_empty_if_dead_reads
 * (written 2026-06-02) strips the closing construct before scoring, so
 * `sandbox --disable all` reads 13, not 0 — the detector predates the
 * 2026-08-18 F6 sanction and has no allowlist hook (unlike
 * _SANCTIONED_UNWRITTEN_PADS). Ruling + engine wiring requested; see
 * outcome + docs/grind decision trail.
 *
 * THE CLOSER: `if (D_80101BCC) { }` (empty-if redundant-condition, F6
 * family, .claude/rules/no-new-park-categories.md:371-383; SOTN exhibits
 * docs/reference/sotn-construct-index.md:34 [empty_if family, 17 PSX
 * instances] and :63 [dra/5D5BC.c:769 "!FAKE, permuter found it"]).
 * Mechanism (dump-proven, tmp/grind/func_800858D0/dumps/ regenerated from
 * THIS form): the manufactured limit read becomes reg101/reg102; jump1
 * deletes the empty branch; cse1 rewrites the loop compare to read reg102,
 * leaving `u = reg102` (insn 139) DEAD; loop.c still counts reg75(u) as
 * 2-set (insn 71 `u=24` + insn 139) -> not a movable, li 24 NOT hoisted
 * (main.loop: zero moved lines, "biv discarded"); flow then deletes insn
 * 139 (main.flow NOTE_INSN_DELETED 139) -> at sched1 reg_n_sets[75]==1 ->
 * birthing_insn_p (sched.c:2504-2537) approves -> adjust_priority
 * (sched.c:2584-2590) LAUNCHES li 24 adjacent to its sh (target's mid-loop
 * slot); the a0=sp+16 hard-reg set becomes the lone pri-1 straggler at
 * block top; reorg steals it into preheader + loop-back delay slot
 * (target's rotation identity); hard $a0 live across the sign-extend span
 * puts the extend in $a1 and the 24/limit class in $v1/$v0 — all 13
 * residual diffs flip together, exactly the coupled flip mapped at s3 open.
 * The pass-order window (loop.c counts sets BEFORE flow's DCE, sched counts
 * AFTER) is the only honest resolution of the hoist-vs-launch trilemma
 * proven this session (see hypotheses.md s3 H9-H11).
 *
 * Everything else is the reconstructed floor-13 do-while form (s2):
 * u={0x18, limit-reload} carrier (defeat-licm-hoist-var-reuse),
 * offset={stride, 1} reuse (anti-dep pins li 1 by the jal), direct s16
 * increment, A/B statement moves in the init block. */
void func_800858D0(void) {
    s32 buf[16];
    s16 var_s0;
    s32 offset;
    s16 ff;
    s32 u;

    buf[1] = 0x60093;
    var_s0 = 0;
    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;
    if (D_80101BCC != 0) {
        ff = 0xFF;
        do {
            offset = var_s0 * 54;
            u = 0x18;
            *(s16 *)((u8 *)&D_800F4E1A + offset) = u;
            *(s16 *)((u8 *)&D_800F4E1E + offset) = 0;
            *(s16 *)((u8 *)D_800F4E28 + offset) = ff;
            *(s16 *)((u8 *)&D_800F4E2A + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2C + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2E + offset) = ff;
            offset = 1;
            buf[0] = offset << var_s0;
            func_8008B488(buf);
            D_8010280A = var_s0;
            func_800871D4(1);
            var_s0 = var_s0 + 1;
            if (D_80101BCC) { /* !FAKE: empty-if redundant-condition (F6,
                no-new-park-categories.md:371); mechanism: jump1 branch
                deletion + cse1 load-CSE leaves u's second set dead;
                loop.c sees 2 sets (no hoist), flow DCEs it, sched1 sees
                reg_n_sets==1 and launches the li 24 (birthing_insn_p,
                sched.c:2504-2537); lever-exhaustion: hypotheses.md s3
                H9-H11 (noted-loop trilemma proven analytically) */
            }
            u = D_80101BCC;
        } while (var_s0 < u);
    }
}
