# func_8007C97C — WIP resume notes

## Standing policy — canonical-asm authorization IS REJECTED for this function

> 2026-06-02 user policy (codified after round 1 worker re-proposed it):
> **DO NOT propose canonical-asm authorization for func_8007C97C unless you
> first surface a fresh measured S1/S2/S6/S8 audit demonstrating STRONG-tier
> evidence per `.claude/rules/hand-coded-asm-recognition.md`.** Target has
> ZERO hand-coded signals (S1=0 `/* handwritten instruction */`, S2=none
> hardcoded register signatures, S6=standard `$a0..$a3` ABI, S8=no
> packed-multiply tell); no GTE/BIOS/scratchpad/no-C-form constructs.
> The function is structurally normal C — see `meta.json` rejected_forms[0]
> for the full evidence. Round-1 worker's proposal was the same misguided
> framing as B844's session-3 proposal; both rejected on the same evidence.

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

## Session 2026-06-02 (workflow round 1)

Structural re-analysis of target asm confirmed the 4 dead `sw` stores + 0x10
frame allocation. Considered two alternative coercion shapes (read+write array,
struct-member variant) — both fail human-programmer test and naming-intent test
and were rejected without measurement per default-to-NO_PROGRESS. Worker could
not measure the existing candidate in the isolated worktree. Suggested next
step: escalate for canonical-asm authorization. CAUTION: same caveat applies
as B844's user_policy_note — canonical-asm needs explicit S1/S2/S6/S8 signals,
not evidence-by-absence-of-pure-C-path; user adjudication required.

## Session 2026-06-03 (workflow round 3)

**Floor unchanged at 24 — NO MEASUREMENT POSSIBLE.** Applied WIP candidate.c
and tested a natural-C form; sandbox returned non-scorable due to display.c-wide
cheat-asm-strip pipeline truncation (the same blocker that affected C7A0 and
CBB0 workers in parallel sessions). Worker recommended surfacing as a policy
escalation toward canonical-asm — REJECTED per the **standing policy at the
top of this file** (no fresh S1/S2/S6/S8 audit evidence; absence-of-pure-C-path
is not itself the canonical-asm threshold per `.claude/rules/hand-coded-asm-recognition.md`).

**Unblocker for round 4:** the C86C round-3 worker discovered `--keep-cheat-asm`
restores indices for display.c functions whose own body has no cheat-asm; this
should be tried for C97C next round to get a measurable baseline. Once
baseline measurable, the next-step menu reverts to `meta.json` next_hypotheses:
apply candidate body, measure, re-run cheat-reviewer.

## Session 2026-06-04 (workflow round 4) — FIRST SUCCESSFUL MEASUREMENT

**candidate_floor MEASURED for the first time: 29 (was null).**

The actual unblocker was `pwsh tools/setup_worktree.ps1` — it creates
junctions to main's `.venv`, `build/`, `tools/gcc-2.7.2`, `disc/`. With those
junctions in place, plain `tools/eng.ps1 sandbox func_8007C97C --disable all`
runs cleanly without needing the `--keep-cheat-asm` workaround. Round-3's
pipeline-truncation issue did not reproduce (sandbox returned `scorable=true`,
`score=29`, `rules_dropped=9`, `cheat_asm_stripped=446`, `build_insns=25`,
`target_insns=33`).

**Why 29, not 24 (the prior session's documented head_floor):** the 24 was an
artifact baseline from before the candidate body was clean of the sp[4] coercion;
it was never the legitimate-candidate's measured floor. The true honest pure-C
floor of the candidate body — the closest legitimate form, inverted-null-check +
m2c-shape + r_e2 named intermediate + temp_v0 var-reuse — is **29**.

**Variants measured (all ≥ 29):**
- candidate.c body (preserved): score **29** ✓ base
- Natural-C single-expression body: score **30**

**The un-closable gap (8 insns) is the 4 dead `sw` stores + frame allocation:**
target writes `sp[0..3] = r/g/b1/b2` but never reads them; these dead writes
came from either hand-coded asm or from C constructs in the forbidden cheat
catalog (write-only array, dead self-assign, volatile coercion, address-of-void).
DCE in cc1 removes any local that isn't read — no pure-C source produces 4 dead
stack writes without being itself a cheat-by-spelling.

**No new lever; no candidate.c modification; src reverted; oracle green.**
Outcome: NO_PROGRESS. The candidate remains the legitimate base; the function
remains INCOMPLETE pending either (a) a novel non-cheat C structural lever that
produces dead stack writes (unlikely — DCE catches the class) or (b)
reconsideration of the canonical-asm-authorization standing rejection (also
unlikely — S1/S2/S6/S8 signals remain absent).
