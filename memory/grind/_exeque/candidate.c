/* _exeque candidate — session 3 (structural), sandbox --disable all score
 * 12/187 (unchanged from the s2-banked floor of 12/187). Apply this body to
 * src/display.c in place of the `INCLUDE_ASM("asm/funcs", _exeque);` line.
 *
 * Forward-declaration fixups needed (already applied in src/display.c
 * since s1; unchanged this session):
 *   - `void _exeque();` -> `extern s32 _exeque(void);` (correct return type)
 *   - the stray second `extern void _exeque();` redeclaration before _sync
 *     was deleted (wrong type, redundant)
 *   - `extern s32 D_8009BF84;` added near the other D_8009BE7C/D_8009BE80
 *     externs (was completely undeclared before s1 — see s1 evidence)
 *
 * s3 change (pure C, no FAKE/cheat constructs): the post-call triple-store
 * block's `mask`-reuse spelling (s2's H5a) was REPLACED with a direct,
 * mask-free assignment:
 *
 *     D_8009BF6C = (s32)_que[D_8009BF7C].arg;
 *     D_8009BF70 = _que[D_8009BF7C].count;
 *
 * This session measured THREE variants of this block on the identical
 * surrounding chassis — (a) s2's `mask`-reuse form, (b) two freshly-named
 * locals `arg_val`/`count_val` loaded before either store, (c) this direct
 * assignment with no intermediate local at all — and all three produced
 * BYTE-IDENTICAL object code for the whole function (sandbox score 12,
 * build_insns 185 in every case; objdump of the triple-store region is
 * identical down to register numbers). Kept (c) because it is the
 * simplest of the three byte-equivalent forms (no reused/staged locals to
 * justify) per [[ordinary-c-judge-decidable]] Ruling 1(4) ("simplest-known-
 * form: when multiple byte-exact forms are known, the one with the fewest
 * no-semantic-purpose constructs lands"). See hypotheses.md H7 (CONFIRMED)
 * for the three-way measurement.
 *
 * The final "clear D_8009BE7C and invoke the D_8009BE80 callback" block
 * (s2's H5b) is unchanged: it pre-computes a pointer to D_8009BE7C
 * (`s32 *p = &D_8009BE7C;`) used for BOTH the guard read and the clear
 * store. Ordinary C, no FAKE needed — closes that whole block to a
 * byte-exact match. See hypotheses.md H5b (CONFIRMED, s2).
 *
 * Remaining floor-12 residual (2 sites — see hypotheses.md frontier for
 * s4, updated this session with dump-verified rank_for_schedule evidence):
 *   a. The triple-store block still schedules both field-stores later than
 *      target (target: strict load-store-load-store per field, each with
 *      its own %hi/%lo recompute; ours: both loads/recomputes happen, then
 *      both stores are deferred to just before the loop-continuation
 *      branch). This session traced the EXACT compiler decision with the
 *      instrumented cc1's BB2_RANK_DEBUG hook (tools/gcc-2.7.2/cc1, NOT
 *      tools/gcc-2.7.2/build/cc1 — see [[instrumented-cc1-location]]):
 *      `RANKDBG last=204 y=198 cls=3 x=189 cls2=3 val=0` — the D_8009BF6C
 *      store (insn 189) ties in BOTH priority (8) AND dependency class
 *      (3 = independent of last-scheduled-insn) against insn 198 (part of
 *      the .count field's address recompute), so GCC's rank_for_schedule
 *      (tools/gcc-2.7.2/sched.c:2417-2464) falls through to the final
 *      INSN_LUID tiebreak, which is fixed by RTL-generation (= C
 *      statement) order — and every C-level respelling this session and
 *      s2 tried (mask-reuse, fresh two-locals, direct assignment,
 *      statement-order swap [s2 H4a], increment-position move [s2 H4b])
 *      leaves that LUID relationship unchanged, because the store is
 *      always generated in the same relative position vs. the recompute
 *      chain for the OTHER field. Five independently-measured spellings,
 *      byte-identical every time. See hypotheses.md H7/H8 for the full
 *      evidence chain and the CLASS-scope kill this now supports.
 *   b. The final callback's `jalr v0` — target keeps the `D_8009BE7C = 0;`
 *      store BEFORE the call with an unfilled delay-slot nop; our build's
 *      scheduler fills the jalr's delay slot with that same store instead.
 *      Marking the pointer `volatile s32 *p` closes this one instruction
 *      (score 12 -> 10) but does NOT qualify under the current
 *      `legitimate-volatile-interrupt-touched` two-prong carve-out (s2 H6,
 *      KILLED instance, NOT submittable) — still a `ruling-request`
 *      candidate for a future session, unchanged since s2.
 */
s32 _exeque(void) {
    s32 mask;

    if (*D_8009BF54 & 0x01000000) {
        return 1;
    }
    mask = SetIntrMask(0);
    D_8009BF84 = mask;
    if (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
        do {
            if (((D_8009BF7C + 1) & 0x3F) == D_8009BF78 && D_8009BE80 == 0) {
                DMACallback(2, 0);
            }
            while (!(*D_8009BF48 & 0x04000000)) {
            }
            _que[D_8009BF7C].func(_que[D_8009BF7C].arg, _que[D_8009BF7C].count);
            D_8009BF68[0] = (s32)_que[D_8009BF7C].func;
            D_8009BF6C = (s32)_que[D_8009BF7C].arg;
            D_8009BF70 = _que[D_8009BF7C].count;
            D_8009BF7C = (D_8009BF7C + 1) & 0x3F;
        } while (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000));
    }
    SetIntrMask(D_8009BF84);
    if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
        s32 *p = &D_8009BE7C;
        if (*p != 0 && D_8009BE80 != 0) {
            *p = 0;
            ((s32 (*)(void))D_8009BE80)();
        }
    }
    return (D_8009BF78 - D_8009BF7C) & 0x3F;
}
