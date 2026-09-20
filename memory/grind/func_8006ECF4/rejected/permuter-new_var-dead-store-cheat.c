/* func_8006ECF4 — s4 (permuter modality) — REJECTED closing-form family.
 *
 * The directed permuter campaign (tmp/perm_ecf4/, 15k+ iterations, base
 * score 725, best find 470) converged repeatedly on ONE family of
 * mutation: introduce an unused `s32`/`unsigned long new_var;` local and
 * fold a real constant into it as a side effect of an existing comparison
 * or arithmetic expression, e.g.:
 *
 *   if (sel < (new_var = 15)) { ... }                       // output-525-1, permuter score 525
 *   for (i = 0; i < D_800A3554 + (new_var = 1) + D_800A35B0; i++)   // output-470-1, permuter score 470
 *   s.p0 = (void *)(s0 + (0x12C & 0xFFFFu));                 // output-470-1, redundant width mask
 *
 * VERIFIED AGAINST THE REAL SANDBOX (not just the permuter's own scorer):
 * applying the `if (sel < (new_var = 15))` form alone to src/text1b.c and
 * running `sandbox func_8006ECF4 --disable all` measured 9 (down from the
 * honest floor of 11) — so this family DOES move the honest metric, not
 * just the permuter's proxy score. It was reverted immediately after
 * measurement; it is NOT in the banked candidate.c.
 *
 * REJECTED per the 6-test cheat checklist:
 *   T1 (semantic purpose) — FAIL. `new_var` is written once, in the middle
 *     of an unrelated comparison/arithmetic expression, and never read
 *     anywhere. It carries no value the function's behavior depends on;
 *     `sel < 15` and `sel < (new_var = 15)` are behaviorally identical for
 *     every input.
 *   T2 (human-programmer test) — FAIL. No one writes
 *     `if (sel < (new_var = 15))` from a specification of this function;
 *     it reads as exactly what it is, a compiler-steering side effect
 *     smuggled into a comparison.
 *   T3 (GCC-internals justification) — the ONLY reason this construct
 *     "works" is that materializing the literal into a fresh pseudo changes
 *     local-alloc's register-choice / cse.c's constant handling for the
 *     surrounding code — i.e. the justification is GCC-internal, not
 *     program logic. FAIL.
 *   T4 (permuter provenance) — this is exactly and only a permuter find;
 *     every one of the ~10 harvested outputs across 15k+ iterations that
 *     beat the 725 baseline used this same `new_var` dead-write pattern
 *     (see output-525-1, output-470-1, output-625-1, output-559-1, plus
 *     the redundant `& 0xFFFFu` mask in output-470-1, itself a separate
 *     forbidden family — "redundant width casts (F2)", explicitly listed
 *     as NOT extended by the 2026-07-01/2026-08-18 SOTN research in
 *     .claude/rules/no-new-park-categories.md).
 *   T5 (family check) — this is the dead-store / constant-holder family
 *     ([[dead-store-fake-exception]] / [[named-local-fake-exception]],
 *     .claude/rules/no-new-park-categories.md "2026-07-01 additions").
 *     Both are LAST-RESORT sanctions requiring (a) documented lever
 *     exhaustion, (b) a named GCC-pass mechanism, (c) a mandatory
 *     `/* FAKE */` annotation. NONE of the three are met this session:
 *     only two honest structural levers were tried this session (removing
 *     the `if (sel<15)` guard entirely — measured WORSE at 46; reordering
 *     the `i` local's declaration position — measured flat at 11) before
 *     this dead-store family was found by the permuter. That is not
 *     lever-exhaustion by the ladder's own standard, so even though the
 *     construct sits inside a nominally-sanctioned family, its
 *     prerequisites are unmet — NOT submittable this session.
 *   T6 (naming) — `new_var` is the permuter's own auto-generated name
 *     (decomp-permuter's default fresh-variable naming), not a project
 *     name; if this family is ever properly exhausted and re-attempted, it
 *     would need a real semantic-sounding name AND the `/* FAKE */`
 *     annotation with mechanism + exhaustion pointer, per rule.
 *
 * DISPOSITION: not proposed as candidate-ready. Recorded here as a banked
 * PROPOSAL for a FUTURE session that first exhausts honest structural
 * alternatives for hunks 4/5 (the b2+c*12 addu operand-only tie — already
 * KILLED for naive associativity swap, s3) and hunk 10 (the `sel < 15`
 * extra slti/beqz insert) — if those are truly exhausted with no honest
 * lever found, THIS is the next thing to re-evaluate under
 * [[named-local-fake-exception]] / [[dead-store-fake-exception]] with a
 * full exhaustion ledger and a `/* FAKE */` annotation, not before.
 */
