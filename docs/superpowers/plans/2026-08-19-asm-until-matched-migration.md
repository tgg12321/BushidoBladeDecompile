# ASM-Until-Matched Migration + Modality Ladder Tuning — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove every committed cheat from main — all 1,573 regfix/asmfix rules and all cheat-asm bodies for the 259 INCOMPLETE functions — by representing unsolved functions as `INCLUDE_ASM("asm/funcs", <func>);` until they reach COMPLETED-C, and retune the grinder ladder per the 2026-08-19 modality-effectiveness report (R1/R2/R3/R5).

**Architecture:** Owner ruling 2026-08-19 (this plan's Task 0 records it): main carries C only for solved functions; in-progress candidates live in `memory/grind/<func>/`. The migration uses the repo's existing INCLUDE_ASM mechanism (oracle-proven, 341 uses), one function at a time with full-build SHA1 verification and rollback. Queue ordering survives via ledger-floor distance pinning. Ladder: stop cycle-2 (1 drop/285 transitions), synthesis to s6, permuter hard cap 2, record closing modality.

**Tech Stack:** Python (engine/, tools/), PowerShell driver (tools/grinder/grind.ps1), WSL build (make → SHA1 oracle 62efab4f73f992798c43e8c730aa43baa10bb4fa).

**Invariants that hold through every task:**
- Oracle green after every commit (`engine build` / `verify-oracle`).
- `engine test` green after every engine/ edit; grinder unittest green after every grindlib edit; drill GO before relaunch.
- Grinder STOPPED for the whole execution (stop confirmed = gate for any tracked edit).
- Nothing is lost: every retired chassis + rule stack is banked in the function's ledger before deletion.
- Parked items migrate too but KEEP their parked status and reasons.
- COMPLETED-C (1,036) and COMPLETED-INLINE-ASM-CANONICAL (179) functions are untouched. jtbl-infra and CANON-EXTRACT authorize-bucket wirings are untouched (separate campaigns).

---

### Task 0: Record the owner ruling BEFORE any code spends it

**Files:** Create `.claude/rules/asm-until-matched.md`; modify `docs/grind/borderline.md` (append note).

- [ ] Write `.claude/rules/asm-until-matched.md`: owner ruling 2026-08-19 — (a) no cheat may be committed on main in any form (rules, cheat-asm, coercion spellings) for INCOMPLETE functions; the committed representation of not-yet-decompiled is `INCLUDE_ASM("asm/funcs", <func>);` verbatim; (b) C lands on main exactly once, at COMPLETED-C, through the unchanged gates; (c) candidates/chassis live in `memory/grind/<func>/` only; (d) `no_new_regfix_guard` unchanged (rule count only goes DOWN; migration commits are pure deletions from regfix/asmfix); (e) ladder retuning per `docs/grind/modality-effectiveness-2026-08-19.md` R1/R2/R3/R5 adopted; (f) supersedes the never-implemented chassis-refresh proposal.
- [ ] `paths:` glob: `["src/*.c", "regfix.txt", "asmfix.txt", "tools/grinder/**", "engine/queue.py"]`.
- [ ] Commit: `rules: asm-until-matched — owner ruling 2026-08-19 (no committed cheats; INCLUDE_ASM until COMPLETED-C; ladder retune)`.

### Task 1: R5 — record the closing modality (measurement first, it validates everything later)

**Files:** Modify `tools/grinder/grind.ps1` (the COMPLETED-C merge path, currently `Journal "$func COMPLETED-C after $sessionsTaken sessions."`).

- [ ] Change to `Journal "$func COMPLETED-C after $sessionsTaken sessions (closer: s$sessionN [$modality] floor->0)."` using the in-scope `$modality`/`$sessionN` of the closing session.
- [ ] Also append the closing modality into the Match commit message body (one line), so completions are analyzable from git alone.

### Task 2: R3 — permuter hard cap at 2 sessions per function

**Files:** Modify `tools/grinder/grindlib.py` (the permuter gate in `assign_modality`, ~line 610); test in `tools/grinder/tests/test_grindlib.py`.

- [ ] Read the existing gate (skips permuter after a zero-yield first). Extend: count prior permuter entries in `floor_history`; if >= 2, skip the rung unconditionally (advance to next rung). Evidence: 3rd+ permuter = 0 drops / 64 sessions.
- [ ] Add a unittest: state with two permuter floor_history entries → `assign_modality` never returns "permuter".
- [ ] Run `python3 -m unittest tools.grinder.tests.test_grindlib` → OK.

### Task 3: R2 + R1 — ladder reorder and cycle stop

**Files:** Modify `tools/grinder/grindlib.py` (`LADDER` constant ~line 20; `_exhaustion_ready` / `assign_modality`); tests.

- [ ] R2: `LADDER = ["structural","structural","permuter","permuter","synthesis","forensics","forensics","rederive","rederive"]` (synthesis s6, was s10). Revert trigger recorded in the rule file: if ~15 early-synthesis sessions yield no drop, restore.
- [ ] R1: in `assign_modality`, after one full ladder cycle (session_count > 10) with NO floor drop across the last full cycle (compare `floor_history` tail), return "escalation" instead of cycling. Evidence: cycle-2+ = 1 drop / 285. This reaches the existing `_exhaustion_ready` escalation path sooner; no-deferral is preserved (escalation IS the modality change; target never rotates).
- [ ] Unittests: (a) 10 flat sessions → "escalation"; (b) a drop inside the last cycle → ladder continues.
- [ ] Run grinder unittest suite → OK. Commit Tasks 1–3 together: `grinder: ladder retune per modality-effectiveness report (R1 cycle-stop, R2 synthesis->s6, R3 permuter cap 2, R5 closing-modality record)`.

### Task 4: Migration tool

**Files:** Create `tools/migrate_include_asm.py` (+ `tmp/` dry-run outputs). LF-safe (WSL python only).

- [ ] Implement per-function migration:
  1. Locate the function's C body span in `src/<stem>.c` (reuse `engine/inlineasm._func_body_span`; refuse if absent unless already INCLUDE_ASM → no-op "already migrated").
  2. Bank to ledger: `memory/grind/<func>/retired-chassis-2026-08/body.c` (the exact span text incl. any file-scope decls ONLY if unused elsewhere — conservative: body span only) and `rules.txt` (every regfix/regfix_stage2/asmfix line keyed to the function, with original line numbers). Create ledger dir if the function never had one.
  3. Replace the body span with `INCLUDE_ASM("asm/funcs", <func>);` (verify `asm/funcs/<func>.s` exists first; refuse otherwise).
  4. Delete the function's rule lines from all three rule files.
  5. Record the pre-migration honest floor: run `sandbox <func> --disable all` BEFORE the edit; write `{"floor": N, "measured": "<date>", "source": "pre-migration chassis"}` to `memory/grind/<func>/migration_pin.json`. If the ledger's `floor_history` has a lower banked floor, record both.
  6. Rebuild (`make` via WSL) → SHA1 must equal the oracle; on mismatch, restore the body + rules verbatim (in-memory originals) and report FAIL.
- [ ] Batch mode: `--funcs a,b,c` or `--batch N` (take N from queue order); `--dry-run` prints the plan without editing.
- [ ] Compile-failure handling: if the TU fails to compile after body removal (orphaned static helpers/decls), the tool reports the exact error and rolls back — such functions go to a `migration-deferred.txt` list for manual handling; NEVER auto-delete adjacent declarations.

### Task 5: Queue distance pinning

**Files:** Modify `engine/queue.py` (`generate()`, the `_no_c_body` branch ~line 320 and entry construction ~line 405); `engine/tests/` (wherever regen tests live — follow existing test file).

- [ ] In `generate()`: when `_no_c_body(stem, func)` and `memory/grind/<func>/migration_pin.json` exists, use the pinned floor as `distance` and set `"distance_source": "migration-pin"` on the entry (whole-function length remains the fallback). Ledger `floor_history` last value wins if lower (a banked candidate is the true frontier).
- [ ] Engine test for the pin path (fixture queue regen with a pin file).
- [ ] Run `engine test` → 332+ green. Commit Tasks 4–5: `engine+tools: INCLUDE_ASM migration tool + ledger-floor distance pinning`.

### Task 6: Prototype on 5 functions (diverse, oracle-verified each)

Candidates: `CD_datasync` (15 rules, banked floor-7 candidate, parked — stays parked), `func_80034F88` (30 rules, parked), one 16+-rule active item from the queue top half, one 2–5-rule active item, one 0-rule cheat-asm active item.

- [ ] `--dry-run` all five; review banking plan output.
- [ ] Migrate one at a time; after each: full build SHA1 == oracle, `check_completion_integrity` OK.
- [ ] `queue regen` → verify: all five present, ACTIVE ones still active, parked ones still parked, distances == pins (not whole-length), ordering sane.
- [ ] Cheat-audit spot check: `volatile_cheats`/`func_cheat_asm_count` on the five migrated TUs attribute nothing to the migrated functions.
- [ ] `pwsh tools/grinder/grind.ps1 -Once` (one supervised iteration) on the new queue top IF it is one of the migrated functions — otherwise temporarily verify session flow by confirming the brief's CHASSIS CHECK reports the INCLUDE_ASM state sanely and the session can write a candidate body over the INCLUDE_ASM line (the driver's Revert-SessionEdits already restores it). Fix whatever breaks BEFORE the sweep.
- [ ] Commit: `migrate: asm-until-matched prototype — 5 functions to INCLUDE_ASM (oracle green)`.

### Task 7: The sweep — remaining ~254 in batches of 25

- [ ] For each batch: migrate (tool, one-at-a-time internally), full build SHA1, integrity audit, commit `migrate: asm-until-matched batch K/11 (M functions, N rules retired)`.
- [ ] Deferred list (compile failures) handled manually at the end; each gets its orphaned-decl cleanup reviewed individually, then the same tool path.
- [ ] After the last batch: `queue regen`; verify counts (259 items, distances pinned, parked preserved); `regfix.txt`/`asmfix.txt` contain ONLY jtbl-infra + canonical-extraction wirings (verify with `engine/cheats.py` helpers); `fixtures-verify` green.

### Task 8: Docs, memory, relaunch

- [ ] Update `CLAUDE.md` (queue section: one line — INCOMPLETE functions are committed as INCLUDE_ASM; candidates live in ledgers) and `memory/project/greenfield-engine-v2.md` (same). Regen memory index.
- [ ] Append the migration record to `docs/grind/borderline.md` (what moved, counts, pins).
- [ ] `pwsh tools/grinder/drill.ps1 -WithJudge` → GO.
- [ ] Relaunch detached; watch the FIRST full session on a migrated function end-to-end (brief sanity, candidate flow, revert works); then report.

**Rollback story (whole migration):** every step is a commit with oracle verification; any regression = `git revert` of the offending batch commit (bodies+rules restore verbatim; ledgers keep the banked copies regardless).
