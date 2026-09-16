/* _exeque candidate — session 4 (permuter), sandbox --disable all score
 * 2/187 (dropped from the s2/s3-banked floor of 12/187). Apply this body to
 * src/display.c in place of the `INCLUDE_ASM("asm/funcs", _exeque);` line.
 *
 * s5 (permuter) re-confirmed this exact body at floor 2/187, unchanged. Two
 * hand-derived structural variants of the final-callback block (do-while(0)
 * wrap: no effect; cb-local hoist: WORSE, 2->11) and a second fresh-seed
 * permuter campaign on the jalr-delay-slot residual (15356 iters, 0 novel
 * finds; cumulative with s4's campaign-3 now >=20k on this exact chassis)
 * all failed to improve on this body. See hypotheses.md [s5]. This body is
 * still the best known chassis — next session should NOT re-seed the same
 * permuter search space; either try a structurally different rewrite of the
 * final block, or resolve the frontier's ruling-request.
 *
 * Forward-declaration fixups (unchanged since s1/s3):
 *   - `void _exeque();` -> `extern s32 _exeque(void);` (correct return type)
 *   - `extern s32 D_8009BF84;` added near the other D_8009BE7C/D_8009BE80
 *     externs (was completely undeclared before s1 — see s1 evidence)
 *
 * s4 change (pure C, TWO do-while(0) wraps, both /* FAKE * / annotated per
 * [[do-while-zero-exception]]): a directed decomp-permuter campaign
 * (memory/grind/_exeque/evidence.md [s4]) found that wrapping the post-call
 * triple-store block in a single `do { ... } while (0);` drops floor 12 -> 7,
 * and NESTING a second do-while(0) around just the first two stores
 * (D_8009BF68[0]/D_8009BF6C) — leaving D_8009BF70's store and the
 * D_8009BF7C increment outside — drops floor 7 -> 2. Both wraps are
 * sanctioned per the frozen SOTN family (ANY codegen effect including
 * register/scheduling, owner ruling 2026-07-06); the nested wrap's
 * mandatory "single-level-insufficient" justification is the direct A/B
 * measurement (single-level: floor 7; nested: floor 2) on the identical
 * surrounding chassis.
 *
 * Everything else (s2's H5b final-callback pointer, s3's direct-assignment
 * field spellings now living INSIDE the do-while bodies) is unchanged.
 *
 * s6 (structural, 2026-09-16): re-confirmed floor 2/187 unchanged. Ran the
 * instrumented cc1 .dbr (reorg.c) dump for pass-attribution due diligence,
 * confirming the exact mechanism (RTL insn 311, the `*p=0` store with its
 * address cached in hard reg v1, is reorg.c's delay-slot fill for the
 * jalr call_insn). Four fresh structural probes around the residual (final
 * -block guard && order, outer-guard && order, SetIntrMask relocation into
 * both if/else arms, outer loop spelled as plain `while` instead of
 * if+do-while) all measured WORSE or NO-EFFECT, never better -- see
 * hypotheses.md [s6] for all four. This body (unchanged from s4/s5) is
 * still the best known chassis.
 *
 * s7 (enumerate, 2026-09-16): re-confirmed floor 2/187 unchanged (src had
 * drifted back to INCLUDE_ASM between sessions; re-applied this body
 * verbatim). Ran the systematic spelling-enumeration protocol on the final
 * -callback block, naming the guard read and callback pointer as
 * additional locals (`flag`, `cb`) alongside the existing `p`: 19 spellings
 * (every inline/keep subset x valid order x commutative swap) swept
 * against the real sandbox. Floor 2 reached ONLY by the two spellings that
 * collapse back to this exact chassis; every spelling that names `flag` or
 * `cb` as a real local scores 5-27 (worse). Class-killed (instance scope)
 * this enumeration space -- see hypotheses.md [s7]. This body unchanged.
 *
 * s8 (synthesis, 2026-09-16): re-confirmed floor 2/187 unchanged (src had
 * drifted back to INCLUDE_ASM; re-applied this body verbatim). Ran the
 * mandatory kill re-audit via tools/fake_ablate.py on this exact body:
 * keep-both-wraps=2, drop-either-wrap=7, drop-both=12 -- both FAKE wraps
 * independently load-bearing, no inert-carrier/false-kill pattern (the
 * func_8002EA24 s8 failure mode). Synthesized the frontier down to two
 * untried STRUCTURAL axes (see hypotheses.md [s8] synthesis entry): the
 * H6 volatile ruling-request, and forcing D_8009BE7C's address into a
 * call-clobbered register via the *address computation* itself (not the
 * callback pointer -- s7 already enumerated and killed naming the
 * callback pointer as a fresh local in all 19 combinations). This body
 * unchanged.
 *
 * Remaining floor-2 residual (1 site, unchanged mechanism from s2's H6):
 *   The final "clear D_8009BE7C and invoke the D_8009BE80 callback" block's
 *   `jalr $v0` — target keeps `sw $zero,0($v1)` (D_8009BE7C = 0;) BEFORE the
 *   jalr with an explicit unfilled delay-slot nop; our build's reorg.c
 *   delay-slot filler moves that store INTO the jalr's delay slot instead.
 *   s2 already identified this and found that marking the pointer
 *   `volatile s32 *p` closes it (floor -> 0 in that session's numbering)
 *   but is NOT submittable: the guard-and-clear single-read-test-and-clear
 *   use-site shape is not one of legitimate-volatile-interrupt-touched's
 *   three catalogued shapes (H6, KILLED instance, rejected form banked at
 *   memory/grind/_exeque/rejected/volatile-D_8009BE7C-guard-clear.c).
 *   s4's campaign 3 (this session) re-targeted this exact residual with a
 *   directed permuter run on the now much-smaller floor-2 chassis; see
 *   hypotheses.md for the outcome.
 *
 * s9 (solver, 2026-09-16): re-confirmed floor 2/187 unchanged (src had
 * drifted back to INCLUDE_ASM; re-applied this body verbatim). Ran
 * tools/ra_solver/inverse_compose.py classify (object-level path) and
 * tools/sched_solver/perturb.py (both sched1/sched2 passes,
 * --goal-from-target via the object-level --target-object escape --
 * the asm/funcs/_exeque.s text escape is unusable, its parser skips every
 * `/* offset addr bytes */`-prefixed disassembly line). classify confirms
 * SCHED/nop-only (no RA component anywhere in the function); perturb.py
 * then found ZERO blocks in either pass where target's required pre-reorg
 * order differs from our own pick order -- the search space for a
 * scheduling-order lever is EMPTY, not merely exhausted. This formally
 * upgrades s6's dump-based diagnosis: the floor-2 residual is entirely a
 * reorg.c fill_simple_delay_slots decision, a pass neither ra_solver nor
 * sched_solver models (documented out-of-scope in both tools' READMEs).
 * See hypotheses.md [s9] for the full evidence chain. This body unchanged.
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
            /* FAKE: do-while(0) wrap, mechanism: reorg.c list-scheduler
             * ordering of the post-call debug-record triple-store
             * (D_8009BF68[0]/D_8009BF6C/D_8009BF70), lever-exhaustion:
             * memory/grind/_exeque/hypotheses.md H7/H8 (five independent
             * statement/variable respellings of this block measured
             * byte-identical, s2-s3) */
            do {
                _que[D_8009BF7C].func(_que[D_8009BF7C].arg, _que[D_8009BF7C].count);
                /* FAKE: NESTED do-while(0) wrap, mechanism: same
                 * reorg.c/rank_for_schedule ordering as the outer wrap, on
                 * a narrower sub-block; lever-exhaustion: single-level
                 * wrap around the whole triple-store (this file's outer
                 * do-while) measurably left floor at 7/187 -- nesting a
                 * second level around just the first two stores measured
                 * floor 2/187 on the identical surrounding chassis
                 * (memory/grind/_exeque/evidence.md [s4] campaign 1 vs 2),
                 * satisfying do-while-zero-exception's
                 * single-level-insufficient prerequisite for nested wraps */
                do {
                    D_8009BF68[0] = (s32)_que[D_8009BF7C].func;
                    D_8009BF6C = (s32)_que[D_8009BF7C].arg;
                } while (0);
                D_8009BF70 = _que[D_8009BF7C].count;
            } while (0);
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
