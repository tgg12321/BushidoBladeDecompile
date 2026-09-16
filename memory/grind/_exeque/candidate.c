/* _exeque candidate — session 2 (structural), sandbox --disable all score
 * 12/187 (down from the s1-banked floor of 15/187, and 187 raw INCLUDE_ASM
 * floor before s1). Apply this body to src/display.c in place of the
 * `INCLUDE_ASM("asm/funcs", _exeque);` line.
 *
 * Forward-declaration fixups needed (already applied in src/display.c
 * since s1; unchanged this session):
 *   - `void _exeque();` -> `extern s32 _exeque(void);` (correct return type)
 *   - the stray second `extern void _exeque();` redeclaration before _sync
 *     was deleted (wrong type, redundant)
 *   - `extern s32 D_8009BF84;` added near the other D_8009BE7C/D_8009BE80
 *     externs (was completely undeclared before s1 — see s1 evidence)
 *
 * s2 changes (both pure C, no FAKE/cheat constructs, no new declarations
 * needed beyond what's already in scope):
 *
 * 1. The post-call triple-store block (D_8009BF68[0]/D_8009BF6C/D_8009BF70)
 *    now reuses the existing `mask` local (already declared for the saved
 *    interrupt mask, and dead after `D_8009BF84 = mask;`) to hold BOTH the
 *    `.arg` and `.count` field values sequentially instead of using two
 *    freshly-materialized locals. This is the SOTN-sanctioned
 *    "variable reuse for codegen control" family
 *    (.claude/rules/no-new-park-categories.md § SOTN-accepted techniques,
 *    "Variable reuse for codegen control"; .claude/rules/
 *    defeat-licm-hoist-var-reuse.md) — SOTN ships `randy = basePoint.x;
 *    baseX = randy;`-style reuse with a "FAKE but makes register allocation
 *    work" comment for the identical mechanism (single register serializes
 *    the two field loads instead of two independent registers). No FAKE
 *    annotation needed here since the value is real/consumed at each
 *    reassignment (not a dead/no-op copy) — same class as the SOTN
 *    `new_var_temp` citations, [[ordinary-c-judge-decidable]] Ruling 1.
 *    This did not move the sandbox score by itself (see hypotheses.md H4)
 *    but is a genuine structural improvement (matches target's single-
 *    register-per-field-pair reuse instead of a two-register split) and is
 *    kept because it does not regress and documents real progress toward
 *    the remaining residual.
 *
 * 2. The final "clear D_8009BE7C and invoke the D_8009BE80 callback" block
 *    now pre-computes a pointer to D_8009BE7C (`s32 *p = &D_8009BE7C;`)
 *    used for BOTH the guard read (`*p != 0`) and the clear store (`*p =
 *    0;`), instead of reading/writing the global by name twice. This is the
 *    "defeat-combine-symbol-fold — displaced store/load folded into
 *    %lo(sym+K) addressing -> pre-compute a displaced pointer" structural
 *    lever from the codegen-technique-index. Ordinary C, no FAKE needed —
 *    the pointer is genuinely read AND written through, not merely held.
 *    THIS closed the entire final-callback block to a byte-exact match
 *    (only a masked branch-target diff remains there) and dropped the
 *    sandbox floor from 15 to 12. See hypotheses.md H5 (CONFIRMED) for the
 *    measurement and the objdump evidence.
 *
 * Remaining floor-12 residual (2 sites, both scheduling-only — see
 * hypotheses.md frontier for s3):
 *   a. The triple-store block still schedules both field-stores later than
 *      target (target: load-store-load-store per field with its own
 *      %hi/%lo per store; ours: both loads happen, then both stores are
 *      deferred to just before the loop-continuation branch). Confirmed
 *      NOT fixable by pure statement reordering this session (H4a/H4b
 *      KILLED, instance).
 *   b. The final callback's `jalr v0` — target keeps the `D_8009BE7C = 0;`
 *      store BEFORE the call with an unfilled delay-slot nop; our build's
 *      scheduler fills the jalr's delay slot with that same store instead
 *      (functionally equivalent, 1 insn shorter). Empirically, marking the
 *      pointer `volatile s32 *p` prevents the delay-slot fill and closes
 *      this one instruction (score 12 -> 10) — but D_8009BE7C's use-site
 *      here (single-read guard-and-clear, not spin-wait / double-read-
 *      across-sequence-point / IRQ-mutated-loop-bound) does NOT match any
 *      of the three catalogued shapes in
 *      .claude/rules/legitimate-volatile-interrupt-touched.md, so it is
 *      NOT submittable under the current two-prong carve-out even though
 *      _exeque itself IS installed as a DMA IRQ callback (`DMACallback(2,
 *      _exeque);`, src/display.c:915) and would satisfy prong 1. This is a
 *      `ruling-request` candidate for a future session (a new use-site
 *      shape: "IRQ-callback-installed function does its own single-read
 *      test-and-clear on a flag it also writes from non-IRQ context") —
 *      NOT adopted this session; kept OUT of this candidate.
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
            mask = (s32)_que[D_8009BF7C].arg;
            D_8009BF6C = mask;
            mask = _que[D_8009BF7C].count;
            D_8009BF70 = mask;
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
