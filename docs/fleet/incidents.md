# Fleet incidents log

Append-only record of circuit-breaker trips and the Overseer's diagnosis + decision.
Trenton reads these on return; be specific about what tripped, what was verified, and why.

---

## 2026-06-15 — DRILL: simulated incident (resume)

- **Tripped by:** drill self-test (`tmp/fleet/state.json.circuit.reason = "self-test reset"`).
- **Reported incident:** "drill: simulated incident (main is actually green)".
- **Ground truth at investigation:**
  - `git rev-parse HEAD` → `d1109cd48aef6d2dc7ea0f716677806179f183b5` (= the supervisor's
    last-known-green commit; no divergence).
  - `& tools/wteng.ps1 main verify-oracle` → `build_matches: true`,
    `build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa` (the locked oracle).
  - `git status` clean except `metrics/events.jsonl` — append-only engine metrics log,
    NOT a build input (absent from the verify-oracle drift corpus), so not contamination.
  - No worktree corruption, no toolchain failure, no bad commit, no quarantine candidate.
- **Diagnosis:** false alarm by design. The drill harness flipped the breaker to exercise
  the Overseer path; `main` was already green throughout.
- **Decision:** `resume`. Supervisor re-arms the fleet; no `main` mutation needed.
- **Notes for Trenton:** drill exercised the read-only investigation flow end-to-end —
  oracle verify, git status, state.json inspection, outcome emission. Nothing for you to
  fix; this entry exists so the incidents log itself is exercised.

---

## 2026-06-16 — DRILL: simulated incident (resume)

- **Tripped by:** drill self-test (`tmp/fleet/state.json.circuit.reason = "self-test reset"`).
- **Reported incident:** "drill: simulated incident (main is actually green)".
- **Last-known-green supplied:** `4e4eda88f8753525fcb7733c86745c5f60ce08fb`.
- **Ground truth at investigation:**
  - `git log --oneline -1` → `4e4eda88` — HEAD equals the last-known-green commit exactly;
    no divergence.
  - `& tools/wteng.ps1 main verify-oracle --rebuild` → `build_matches: true`,
    `build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa` (the locked oracle). `ok: true`.
  - `git status` shows only `metrics/events.jsonl` modified — append-only engine event log,
    not a build-pipeline file, absent from the verify-oracle drift corpus. Not contamination.
  - `tmp/fleet/state.json` circuit block: `"state": "running"`, `"reason": "self-test reset"`.
  - Fleet state shows 7 cheat-reviewer regression entries (DispSamnailWindow,
    camera_InitBoneData, camera_SetMatrix_8001DBE4, camera_Transform, cdrom_ConfigSPU,
    cdrom_GetCmdName, cdrom_GetResultName) — these are normal audit-queue items for rework,
    not oracle breaks. `gate_failed: {}` (empty). Oracle is intact.
  - No worktree corruption, no toolchain failure, no bad commit, no quarantine candidate.
- **Diagnosis:** false alarm by design. Breaker tripped by the drill harness to exercise the
  Overseer path; `main` was green before, during, and after investigation.
- **Decision:** `resume`. Supervisor re-arms fleet; no `main` mutation required.
- **Notes for Trenton:** 7 cheat-reviewer regressions are sitting in state.json awaiting
  rework — DispSamnailWindow (constant-in-local callee-save coercion), camera_InitBoneData
  (do-while wrap + dead scalar + new_var alias), camera_SetMatrix_8001DBE4 (empty-then
  branch-sense coercion), camera_Transform (aggregated codegen-bending), cdrom_ConfigSPU
  (outer-volatile on pointer), cdrom_GetCmdName and cdrom_GetResultName (goto-done
  accumulator). None are oracle breaks; the oracle is `62efab4f` and intact. Fleet can resume
  working those items immediately.

---

## 2026-07-05 — DRILL: simulated incident (resume)

- **Tripped by:** drill self-test (breaker exercise).
- **Reported incident:** "drill: simulated incident (main is actually green)".
- **Last-known-green supplied:** `bdd16448ab475094f81ef0ba5f8a451925d1b1a3`.
- **Ground truth at investigation:**
  - `git log --oneline -6` → HEAD is `bdd16448` — equals the last-known-green commit
    exactly; no divergence (top of log: marionation_Exec session-6i(2) WIP work).
  - `& tools/wteng.ps1 main verify-oracle --rebuild` → `ok: true`, `build_matches: true`,
    `build_sha1 == original_sha1_now == original_sha1_locked == 62efab4f73f992798c43e8c730aa43baa10bb4fa`
    (the locked oracle).
  - `git status` shows two modified files: `memory/wip/marionation_Exec/notes.md` (a WIP
    memory checkpoint doc) and `metrics/events.jsonl` (append-only engine metrics log).
    Neither appears in verify-oracle's drift corpus/config/toolchain lists — neither is a
    build-input file. Not contamination.
  - `tmp/fleet/state.json` is large (558KB); not fully read, but ground-truth build state
    was independently confirmed via the direct `verify-oracle --rebuild` call, which is
    authoritative.
  - No worktree corruption, no toolchain failure, no bad commit, no quarantine candidate.
- **Diagnosis:** false alarm by design. Breaker tripped by the drill harness to exercise the
  Overseer path; `main` was green before, during, and after investigation.
- **Decision:** `resume`. Supervisor re-arms fleet; no `main` mutation required.
- **Notes for Trenton:** nothing to fix. The two dirty files (WIP notes + metrics log) are
  normal ongoing-work artifacts, not build-pipeline files, and pose no threat to the oracle.
