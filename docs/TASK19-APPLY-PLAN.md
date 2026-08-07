# TASK19 APPLY PLAN — main-tree session steps

Companion to `docs/TASK19-PROTO-ANALYSIS.md`. The work was prepared on the
isolated worktree branch `worktree-agent-acbdbab88162d8f4b` (base `9fb39395`)
in three commits:

1. `cleanup: unify text1b prototype contradictions ...` — src/text1b.c +
   src/text1b_b.c edits (**GRINDER-CONFLICT-RISK — see below**)
2. `tools: promote w6_canon_check as canon_payoff_check ...` — new
   tools/canon_payoff_check.py (no conflict risk)
3. `docs: task #19 analysis + apply plan` — the two docs (no conflict risk)

NOTHING here was build-verified (the worktree has no toolchain/.venv/disc).
Every src edit is asm-evidence-argued byte-neutral, but the oracle is the
only truth.

## GRINDER-CONFLICT-RISK

Commit 1 edits `src/text1b.c` (and `src/text1b_b.c`). The Grinder is actively
working `func_80048AD0` in **src/text1b.c**. Per
[[grinder-clobbers-uncommitted-edits]], foreign dirt in tracked files makes
the driver discard sessions. **Apply commit 1 ONLY when the Grinder is
stopped (`pwsh tools/grinder/grind.ps1 -Stop`) or has moved past text1b.c.**
Commits 2 and 3 touch only tools/ + docs/ and are safe to apply any time the
tree is otherwise clean (still prefer a stopped moment; the scope check
tolerates no foreign dirt mid-session).

## Ordered steps (main tree, WSL for builds)

0. **Preconditions:** Grinder stopped; `git status` clean;
   `verify-oracle` green (`& tools/wteng.ps1 main verify-oracle`).
   Optional but recommended diagnostic capture (pre-fix baseline of the
   swallowed cc1 errors for text1b): run the cpp|cc1 stage for
   src/text1b.c and src/text1b_b.c redirecting cc1 stderr to a tmp file;
   expect "conflicting types for `func_80052754`" (and data-decl siblings).
1. Fetch/cherry-pick the three commits from the worktree branch (or apply
   the equivalent patch). Order: docs + tools first if the Grinder window is
   tight; the src commit last.
2. **Oracle gate for the src commit:** full `build` →
   SHA1 must equal `62efab4f73f992798c43e8c730aa43baa10bb4fa`.
   * Expected result: byte-identical (all edits are decl unifications,
     value-preserving casts, and the func_8005344C 4th-arg drop whose
     byte-neutrality is argued from the instruction-identical sibling
     func_80053304 call block).
   * **If the oracle goes RED:** the only plausible culprit is the
     func_8005344C call-arg drop. Recovery: revert JUST that hunk (restore
     the 4th argument `*(s32 *)((u8 *)D_800A33F4 + 0x1C)` in the call) and
     change the 1502-region decl to unprototyped
     `extern s32 func_80052754();` (legal with both call shapes); rebuild.
     If STILL red, revert the src commit entirely (`git revert`), re-verify
     green, and record the diff bytes in the analysis doc — the caller diff
     tells which unification was not codegen-neutral.
3. Re-run the pre-step-0 diagnostic capture (optional): the
   "conflicting types" errors for func_80052754/func_80052D00 should be gone.
4. Delete `tmp/w6_canon_check.py` on main (superseded by
   tools/canon_payoff_check.py). First run of the promoted tool should be
   observed: it must REFUSE when build/bb2.exe is missing, and behave
   identically to the tmp original when build/ is fresh.
5. Census refresh (can ride any later docs commit): update
   `docs/naming/README.md` derivation + AGENTS.md "1,436 functions" per
   analysis §6 (1,437 files − blob − 7 stale duplicates = 1,429 unique).
   OPTIONAL cleanup (build-neutral, verified unreferenced): delete the 7
   stale duplicate files
   `func_800806A4.s func_8003F274.s func_8003032C.s func_80030A2C.s
   func_8003339C.s func_80036D98.s func_80037348.s` — then the census
   becomes 1,430 files − blob = 1,429 functions. If deleted, re-run
   `verify-oracle` anyway (they SHOULD be inert; trust nothing).
6. Commit-msg conventions: the src commit is `cleanup:`-class (not `Match:`)
   — no completion state changes, no rules touched. Manual-path review
   discipline: these are not completion-class commits, but given they touch
   matched bodies in src/, a quick layer-2 look is cheap insurance if any
   doubt remains after green oracle.

## What could NOT be determined without a build

* Byte-neutrality of every edit (argued, not proven; step 2 is the proof).
* Whether cc1 actually emits the expected "conflicting types" diagnostics on
  main today (step 0 capture confirms).
* The promoted canon_payoff_check.py's runtime behavior (engine imports
  unavailable in the worktree).
