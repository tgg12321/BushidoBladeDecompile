# func_8007C97C — WIP resume notes

## TL;DR

- A `Match:` commit (`0e845d2`, 2026-06-01) was **reverted** (`45a7bb0`,
  2026-06-02) because the independent cheat-reviewer agent flagged a
  `s32 sp[4]; sp[0..3] = ...;` write-only frame coercion as a cheat-by-
  spelling — same family as `s32 buf[N];` in dead-vars-local-array, but
  spelled with writes-but-no-reads instead of declared-but-unreferenced.
- **The inverted-null-check structural insight IS a real, legitimate
  lever** — it's the SOTN-clean replacement for the now-archived
  `goto-end-prologue-delay-slot` technique. The reviewer FAILED ONLY on
  the sp[4] part. The candidate body in `candidate.c` preserves the
  legitimate parts (inverted null-check + m2c body + r_e2 + var-reuse).
- **NOT yet measured.** The next session's first task: apply
  `candidate.c` to `src/display.c`, run `sandbox func_8007C97C
  --disable all`, record the floor in `meta.json.scores.candidate_floor`.
- The rejected form is preserved at `rejected/sp4_frame_coercion.c` with
  the reviewer's evidence inline — DO NOT re-derive it.

## How to resume in one read

1. Read `meta.json` — note `reviewer.verdict` is `FAIL_on_prior_form` for
   the rejected sp[4] form, and the candidate's verdict is NOT yet run.
2. Read `rejected/sp4_frame_coercion.c` — understand what the cheat
   was, so you don't re-introduce it under a different name (`spill`,
   `stk`, `_pad`, `buf` — same family).
3. Apply `candidate.c`'s body to `src/display.c` (replace HEAD's
   func_8007C97C body).
4. `& tools/eng.ps1 sandbox func_8007C97C --disable all` — record the
   floor. Update `meta.json` accordingly.
5. **Re-run cheat-reviewer** on the candidate as a sanity check. The
   removal of the sp[4] writes should clear the prior FAIL.
6. From there, the work is finding a SOTN-clean lever (likely
   register-allocation or frame-layout structural) that closes whatever
   gap remains.

## Lessons preserved here

1. **The inverted-null-check pattern is legitimate.** Don't lose that
   in the panic of the revert. It's the right replacement for the
   forbidden goto-end accumulator that the archived
   `goto-end-prologue-delay-slot` rule was trying to encode.

2. **Mechanical detectors are necessary but not sufficient.** The
   find_unused_local_arrays detector missed sp[4] because it checks
   reference COUNT, not reference KIND (read vs write). A write-only-
   array detector is needed; that's filed as a separate engine work
   item in the revert commit.

3. **Workflow workers self-administering the cheat-reviewer checklist
   is unreliable.** The session-0e845d2 worker self-administered the
   6-test checklist and claimed PASS. The independent reviewer caught
   the cheat. Future workflow runs must invoke the reviewer subagent
   from the orchestrator side, not delegate self-review to the worker.

## What to AVOID re-deriving

- Any `T name[N];` (any name, any type) where the array is declared,
  written to, but never read. Same family as the rejected form. The
  rejected/ file documents the specific evidence.
- Any other frame-coercion construct: `volatile T pad;`, `(void)&local;`,
  declared-but-unused structures, etc. See
  `.claude/rules/inline-asm-policy.md` § expanded cheat catalog.
- The earlier `goto end; ... end: return ret_val;` accumulator pattern
  (now archived in `goto-end-prologue-delay-slot.md`). The inverted-
  null-check in candidate.c is the legitimate replacement.

## Related

- `.claude/rules/goto-end-prologue-delay-slot.md` — the archived rule
  this WIP's candidate cleanly replaces
- `.claude/rules/dead-vars-local-array.md` — the catalog rule the
  rejected form violates
- `.claude/rules/review-discipline-before-commit.md` — the gate that
  caught the cheat (independent reviewer invocation, post-fact)
- Revert commit `45a7bb0` — the full reviewer evidence + the
  next-detector-gap filing
