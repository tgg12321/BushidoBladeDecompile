# func_8007CBB0 — WIP resume notes

## Orchestrator verification 2026-06-03 — round-3 meta-blocker REFUTED

The round-3 worker claimed deploying CBB0's candidate body breaks sibling
`func_8007CE0C`'s splice rules at `regfix.txt:4208/4210/4212` (references to
absolute labels `.L152/.L154/.L158/.L174`). **This claim is empirically
false in current main:**

1. Current `regfix.txt:4208/4210/4212` hold unrelated content (a comment,
   an `exec_game` rule, blank). CE0C's splice rules live at
   `regfix.txt:4184/4186/4188/4189` and already use `{lbl#N}` function-local
   slots (drift-robust per `global-label-drift-sibling-cheat.md` § Preferred).
2. Zero display.c functions have absolute `.L` label cheats in `regfix.txt`
   (audit: `tmp/abs_label_audit.py` — the 7 functions that do are in
   `main.c`, `system.c`, `code6cac_b.c`, `config.c`).
3. **Empirical deployment test:** applied candidate body to `src/display.c` +
   disabled CBB0 asmfix bridge → full build SUCCEEDED, no CE0C link failure,
   final SHA1 `fe6ae5d12406411d21e1815a213ae21dd1e8baff` ≠ oracle (expected;
   candidate doesn't fully match).

The worker was reading a stale-worktree `regfix.txt` from before the
{lbl#N} migration landed in main. Their stated meta-blocker does NOT exist.

## DISCREPANCY found this session

Honest sandbox measurement on current main: **score 55, build_insns 129**
(target 151). The WIP's prior documentation claimed score 52, build_insns
151 EXACT match. Something has drifted between session 2 (2026-06-01) and
now. Possibilities:
- Candidate body in WIP has bug (missing externs / wrong types / etc.)
- Surrounding display.c context changed (new functions, headers, etc.)
- Measurement environment differed (different sandbox flags)

**Round 4's first task: re-derive the actual score-52 candidate** OR confirm
that the WIP's documented measurement was anomalous. Do NOT trust the WIP's
"52/151 exact" claim until reproduced.

Reproducible recipe (verified 2026-06-03):
1. `git checkout HEAD -- src/display.c asmfix.txt`
2. `& tools/eng.ps1 verify-oracle --rebuild` → SHA1 == oracle
3. Apply `memory/wip/func_8007CBB0/candidate.c` body to `src/display.c`
4. `& tools/eng.ps1 sandbox func_8007CBB0 --disable all --keep-cheat-asm`
5. Result: score 55, build_insns 129 (NOT 52/151 as WIP claims)
6. Revert + verify-oracle --rebuild to restore canonical state

## TL;DR (PRE-VERIFICATION — see above section for corrections)

- **Floor 52** (HEAD's measured 149 is a sandbox-strip artifact — the
  function is currently held by an asmfile bridge over a stub body, so
  the strip produces build_insns=2 garbage). The candidate's 52 is the
  first HONEST pure-C measurement. Prior session's reported 76 floor was
  the same artifact-mismeasurement issue.

  **⚠ ORCHESTRATOR 2026-06-03: this 52 measurement is NOT REPRODUCING
  in current main. See "DISCREPANCY found this session" section above.**
- **build_insns 151 == target_insns 151 EXACTLY.** The structural
  decomp is end-to-end correct. Frame size matches (0x40), register
  assignments match ($t0/$t1/$s0/$s1), both packet-path store orders
  match target. Residual 52 = list-scheduler INSN_PRIORITY decisions in
  the GPU packet store sequence (the 0x03FFFFFF constant materialization
  splits in target but lands adjacent in the candidate).

  **⚠ ORCHESTRATOR 2026-06-03: re-measurement shows build_insns 129 (22
  short of target), score 55. The structural-end-to-end-correct claim
  needs re-verification.**
- **Cumulative ~24 negative levers across 2 sessions.** Session 2 went
  149 → 52 via semantic reconstruction + 5 specific levers. Session 3
  tested 9 additional SOTN-allowed variants from the score-52 base —
  none lowered the floor further.
- Cheat-reviewer NOT yet invoked on the candidate. Manual self-audit:
  no obvious cheat constructs (no pins, no register-asm, no volatile
  coercion, no unused-array padding). The `(void)arg2; (void)arg3;` is
  the K&R-style honoring of unused params; ABI takes 4, logic uses 2.

## How to resume in one read

1. Read `meta.json` — sessions[], structural_evidence (build_insns and
   register matches), residual gap description, rejected_forms.
2. Apply `candidate.c`'s body to `src/display.c`. NOTE: the function is
   currently a stub held by an asmfile bridge in `asmfix.txt`. To test
   the candidate you'll need to either:
   - (a) Apply the candidate body to src/display.c AND remove the
     asmfix `replace_with_asmfile` bridge line for func_8007CBB0, then
     measure sandbox + sandbox --disable all
   - (b) Use a different sandbox invocation that bypasses the bridge —
     check `engine sandbox --help` for options
3. Confirm sandbox score 52, build_insns 151.
4. Pick a next_hypothesis. Top candidate: directed permuter from a
   CLEAN single-function target (build `asm/funcs/func_8007CBB0.s`
   standalone) to find a scheduling lever.

## Why this is high-value to checkpoint

Two sessions of work establish:
1. The semantic reconstruction is correct (build_insns match exactly,
   stores emit in target order, registers match).
2. The remaining gap is narrow (pure list-scheduling) — NOT
   register-allocation, NOT cross-jump merge, NOT control-flow
   restructure.
3. ~24 SOTN-allowed levers have been tested and ruled out from this
   base. The next agent should NOT re-derive the m2c semantic
   reconstruction or the lever sweep.

Without this checkpoint, a fresh agent picking up CBB0 from the queue
would face a "149 distance, 1 rule" entry and likely spend most of a
session re-deriving the semantic shape of a 151-instruction GPU DMA
list builder before getting to scheduling work.

## Companion memory note

Full detailed evidence ledger (lever-by-lever, including the 9 session-3
variants and the build_insns/register matching) is at:
`memory/project/func-8007cbb0-pure-c-draft.md`

## What's still off — the residual 52

From the candidate.c header docstring:

> The big-packet path's lui/and/or/sw scheduling differs from target.
> Target interleaves the constant-materialization (lui $a3 0x3FFFFFF
> and ori $a3 0xFFFF) with the BF48 load chain. Mine emits them
> adjacent. The lui $a3 0x3FFFFFF ; ori 0xFFFF pair: target splits them
> by ~20 insns letting BF48 load + scheduling happen in between, mine
> emits them together pre-stores. The small-packet path has similar
> list-scheduler diffs.

This is the same class of gap as cpu_side_move_dir_4 / marionation_Exec
sched.c walls — RTL-level INSN_PRIORITY tiebreaker. The legitimate
levers (combine-foldable chain extender etc.) need careful re-vetting
against the 2026-06-02 cheat catalog; session 8 of cpu_side_move_dir_4
documented one such lever and it was retroactively FORBIDDEN. The next
agent must invoke cheat-reviewer on any closing form found.

## Related

- `memory/project/func-8007cbb0-pure-c-draft.md` — full evidence ledger
- `.claude/rules/cross-jump-store-tail-merge.md` — the sched.c session
  this function is structurally similar to (saEft00Add coupled fixpoint)
- `.claude/rules/register-alloc-pure-c.md` — sessions 5-10 of
  cpu_side_move_dir_4 (the chain extender lever is now FORBIDDEN; same
  catalog applies here)
- `.claude/rules/no-new-park-categories.md` — the standing policy
  governing what permuter output is acceptable

## Session 2026-06-02 (workflow round 1, session 4)

Confirmed sandbox-52 baseline via direct disable=all build; tested 6 additional
SOTN-allowed structural variants — all confirm the 52 floor. Cumulative ~30
negative levers across 4 sessions now. The residual 52 remains pure
list-scheduler INSN_PRIORITY in the GPU packet store sequence — target splits
the 0x03FFFFFF lui/ori materialization across ~36 instructions; mine emits them
adjacent. Same class as cpu_side_move_dir_4 sched.c walls. Top next_hypothesis:
directed permuter from a CLEAN single-function offset-0 target with cheat-reviewer
gating on saved candidates.

## Session 2026-06-03 (workflow round 3)

**Floor unchanged at 52, but NEW META-BLOCKER SURFACED.** Round-3 worker attempted
candidate deployment (commented out asmfix bridge + applied candidate body to
display.c). Build link failed:

```
undefined reference to `.L152'
undefined reference to `.L158'
undefined reference to `.L174'
```

in sibling func_8007CE0C. The candidate body's 80-line replacement shifts cc1's
global `.L` counter, orphaning CE0C's 3 splice rules (regfix.txt:4208, 4210, 4212)
that reference absolute global labels (.L152/.L154/.L156/.L158/.L159/.L160/.L174).

**This is a meta-blocker on the queue item, not just a score gap.** A future
worker cannot deploy ANY pure-C candidate for CBB0 — even a score-0 one — without
first migrating CE0C's splice rules to the new `{lbl#N}` function-local label slot
mechanism (per global-label-drift-sibling-cheat.md as updated 2026-06-02).

Sandbox CBB0 was also unscorable in this session — same display.c-wide
cheat-asm-strip pipeline truncation that blocked C7A0 and C97C workers in
parallel sessions. The C86C round-3 worker's `--keep-cheat-asm` flag is the
likely unblocker for the measurement step (CBB0 candidate has no cheat-asm in
its own body once the bridge is removed).

**Forbidden lever clarification:** The chain-extender lever that worked for
cpu_side_move_dir_4 (a natural fit for CBB0's negative-label-drift scenario)
is FORBIDDEN per global-label-drift-sibling-cheat.md as of 2026-06-02 — both
dead-goto label-pad and DImode chain are codified in the FORBIDDEN section.

New PRIORITIZED next_hypothesis: round 4 should ATTEMPT the {lbl#N} migration
of CE0C's 3 splice rules FIRST as a precondition, THEN deploy candidate +
measure with `--keep-cheat-asm`, THEN run directed permuter from a clean
single-function offset-0 target.
