/*
 * REJECTED (session 5b, permuter) — `volatile unsigned int new_var; new_var = 0;`
 * added to the floor-13 chassis. Campaign wsA best find, `output-259-1`,
 * permuter weighted score 259 against that workspace's base of 413 — the
 * largest permuter drop of the session.
 *
 * MEASURED WITH THE HONEST METRIC: `sandbox motion_Close --disable all` prints
 * score 13, build_insns 27 — i.e. IDENTICAL distance to the chassis it was
 * mutated from (13, build_insns 25) while emitting two MORE instructions (the
 * volatile store plus its frame slot). It buys nothing at all on the engine
 * gradient. This is the FOURTH independent confirmation on this function that
 * the permuter's weighted metric (regs x5, reorderings x60, ins/del x100) does
 * not track engine distance here: s3's chassis scored 835 permuter / 16 engine
 * against the clean chassis' 468 / 17; s4's 383 find measured 18; this 259 find
 * measures 13. On motion_Close, permuter score is not usable as a proxy and
 * every find must be re-measured with the sandbox before it means anything.
 *
 * IT IS ALSO A CHEAT, INDEPENDENT OF THE MEASUREMENT. A written-never-read
 * volatile scalar local whose only effect is to reserve a frame slot and
 * perturb allocation is the unused-local frame-coercion family
 * (`.claude/rules/inline-asm-policy.md` expanded catalog: "Unused fixed-size
 * local arrays"; `.claude/rules/dead-vars-local-array.md`). The 2026-07-01
 * written-never-read carve-out does NOT reach it: that carve-out is gated on
 * the TARGET BYTES CONTAINING the corresponding dead stores, and motion_Close's
 * target contains no store to any frame slot other than the three register
 * saves. It fails checklist T1 (no observable effect), T2 (no human writes it)
 * and T6 (`new_var` is a permuter-generated placeholder name announcing exactly
 * that it holds nothing).
 *
 * Recorded per `.claude/rules/no-new-park-categories.md` § "Auto-search tools
 * (permuter, etc.) — output is PROPOSALS, not winners": the find is banked as
 * evidence about the closing-form space and was NOT surfaced as a candidate.
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);
    volatile unsigned int new_var;

    new_var = 0;
    if (D_800A2668 != 0) {
        do { do { do { p = &D_8008D070; } while (0); } while (0); } while (0);
        count = (s32)&D_00000000;
        if (count != 0) {
            do {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            } while (count != 0);
        }
    }
}
