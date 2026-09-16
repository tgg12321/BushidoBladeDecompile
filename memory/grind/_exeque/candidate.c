/* _exeque candidate — session 4 (permuter), sandbox --disable all score
 * 2/187 (dropped from the s2/s3-banked floor of 12/187). Apply this body to
 * src/display.c in place of the `INCLUDE_ASM("asm/funcs", _exeque);` line.
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
