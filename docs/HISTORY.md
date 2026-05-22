# Project History

A condensed timeline of the BB2 matching decompilation, from the initial commit to the present. Compiled from `git log`, the two `CODEX_HANDOFF_*.md` files, and the project's internal status memory.

For day-to-day commits, run `git log --oneline`. This document focuses on inflection points — milestones, infrastructure landings, and rule changes.

## Phase 1 — Bringing-up (2026-03-23 to 2026-03-30)

**The project starts.** Initial commit on **2026-03-23** (b4c9c2d). First week is the bringing-up sprint: 96 commits on 2026-03-25 alone.

| Date | Milestone |
|---|---|
| 2026-03-23 | Initial commit. Splat config, Makefile skeleton, first decompiled functions in `sound.c` (33 functions, 7.5% of total). |
| 2026-03-24 | `config.c`, `gpu.c`, `ings.c`, `ings2.c`, `display.c`, `system.c` segments scaffolded and partially decompiled. `text1a`, `text1b`, `6CAC` segments scaffolded (`asm/text1a.s`, `asm/text1b.s`, `asm/6CAC.s` containing per-function `#include`s). |
| 2026-03-24 | **GP-relative addressing solved** (commit 33747f6). `maspsx.patch` fixes the `_uses_gp()` early-out bug that was skipping nop insertion for GP loads. Unblocks 326 functions across `text1b.c`, `text1a.c`, `code6cac_c2.c`. |
| 2026-03-24 | First major matching push — 196 functions decompiled by end of day (13.9% of total). |
| 2026-03-25 | 96 commits — peak throughput. Many functions matched via early m2c-based workflow. |
| 2026-03-28 | **`--expand-lb` / `--expand-lh` per-function control added to maspsx** (FIX). ASPSX expands `lb sym` to `lbu+nop+sll+sra`; GCC combine pass folds it back to `lb`. Per-function expansion control via `expand_lb_funcs.txt`. |
| 2026-03-29 | First multi-agent attempts with subagents / parallel orchestration. Later archived in 2026-05-12 in favor of solo end-to-end workflow. |
| 2026-03-30 | **Kengo rename complete** (commit 185370e). 220 renames across 7 batches. ~355 of 1,410 functions named (combining Kengo + manual + regfix renames). The Kengo PS2 game (same studio, same engine) provided ~2,500 debug-named functions as a Rosetta stone. |

## Phase 2 — Foundation tooling (2026-04-01 to 2026-04-14)

The decomp process matures. Tools that the project still uses today are built.

| Date | Milestone |
|---|---|
| 2026-04-07 | gccdumps captured for reverse-engineering the optimizer's behavior on specific functions (`gccdump.combine`, `gccdump.cse`, `gccdump.flow`, `gccdump.greg`, `gccdump.sched`, etc.). |
| 2026-04-10 | **Readability pass complete.** ~40 of 64 `GameObj` struct fields named (in `tools/gen_m2c_context.py` → `include/m2c_context.h`). ~240 `D_XXXXXXXX` globals renamed to `g_*` semantic names (linker aliases in `named_syms.txt`). 5 subsystem headers (`gpu.h`, `sound.h`, `game.h`, `system.h`, `code6cac.h`) — removed 2,508 duplicated externs. 105 function renames. 42 named magic constants in `psx.h` + `bb2_const.h`. |
| 2026-04-10 | 66-commit day; major naming + header reorganization. |
| 2026-04-17 | **First Codex session handoff** (CODEX_HANDOFF_2026-04-17.md, no longer present — never tracked in git). Documents the shift from m2c-heavy / permuter-heavy workflow to a more disciplined per-function lab process. Introduces `codex_lab/<func>/` as the isolation pattern for hard functions. New tooling: `check_func.py`, `dump_func_pipeline.py`, `compare_lab_vs_live.py`, `new_match_lab.py`, `promote_regfix_lab.py`. **asmfix.py is introduced** as the next post-pass after regfix, with `replace_with_asmfile` as the bridge mechanism for split jump-table / shared-rodata cases. |
| 2026-04-13–14 | First wave of "endgame" functions matched: `tslPrintScreen`, `replay_camera_rob_back_loose2`, `func_800826CC` (via lab → regfix → asmfix workflow). |
| 2026-04-18 | maspsx vendored as plain files (commit a1ec1fc). Previously a gitlink with no `.gitmodules` pointing at local-only commits — fresh clones got an empty dir and broken build. Local patches saved in `tmp/maspsx_backup/`. |

## Phase 3 — The zero-stub sweep (2026-04-20 to 2026-04-27)

The project reaches a major milestone: **every `INCLUDE_ASM` stub cleared**. All 1,410 functions have entries in C source files, even if many are still implemented as inline `__asm__()` or bridged via `replace_with_asmfile`.

| Date | Milestone |
|---|---|
| 2026-04-20–26 | Steady sweep through remaining stubs. ~470 functions converted to inline-asm or asmfix bridges. Late-stage strategy from the handoff: replace each `INCLUDE_ASM` with the smallest safe C definition + `name: replace_with_asmfile` in asmfix.txt. |
| 2026-04-27 | **ZERO-STUB SWEEP COMPLETE.** All 1,410 functions have C source entries; `wsl make clean-check` passes; build SHA1 matches. Documented in [`CODEX_HANDOFF_2026-04-27.md`](handoffs/2026-04-27-codex.md). 68-commit day. |
| 2026-04-27 | Stabilizer functions identified: `single_game_VoiceContorol`, `camera_set_zoom`, plus pinned helpers (`saTan4GaugeInit`, `saTan0GaugeDraw`, etc.) — fragile due to label coupling in regfix_stage2 / asmfix. |
| 2026-04-27 | **Goal shift.** Reaching zero-stub clarified that "stub clearing" wasn't the right metric. The new goal is **pure C**: eliminate the ~376 inline-asm + asmfix functions that produce the right bytes but aren't really decompiled. |

## Phase 4 — Attempt-first pipeline + permanent blockers (2026-04-27 to 2026-05-01)

| Date | Milestone |
|---|---|
| 2026-04-27 | **Foundation rebuild.** Attempt-first pipeline introduced: `dc.sh classify`, `dc.sh attempt`, `dc.sh smart`, `dc.sh permute`, `dc.sh add-regfix`, `dc.sh recipes`, `dc.sh gte`, `psyq_stdlib_scan`. `dc.sh inline-*` commands for inline-asm-aware setup. |
| 2026-04-28 | **`known_blocked.txt` introduced.** Permanent blocker categories (BIOS jumptables, overflow-trapping `add`/`sub`/`addi`, `$sp` swap, data-as-code, no-`jr-ra`) auto-detected by `tools/classify_func.py` and filtered out of the active decomp queue. Initial list: 11 functions. |
| 2026-04-28 | **Workflow tightening.** `dc.sh verify` checks symbol address (catches `regfix reorder @ 0` and `delete @ 0` traps). `regfix.py` warns on `reorder@0` spans, `insert_label` op added, loud warnings on no-match substs. `dc.sh dump-text --post-regfix`. `gen-regfix` early-exit-alias and varargs detection. |
| 2026-04-29 | **More tools.** `build_queue` filters matched, regfix warns on `.L<N>` inserts, `gen_regfix` frame-pad hint, `tools/siblings.py` (fingerprint matcher), `tools/check_gp_layout.py`, `gen_regfix` packed-args-in-saved-regs detector. |
| 2026-04-29 | Heavy matching day — 12+ functions matched including the call-loop family (47EE8, 47FBC, 480C0, 481E8, 483DC, 644FC), the nested-bool memcard family (60412..., 60156C, 6017C8, 61EC0, 6018B4), display.c clamping siblings (7C7A0, 7C86C), gpu_MoveImage-style packet builder (7B6C8 — 22-rule regfix). |

## Phase 5 — Bridge debt awareness + THE HARD RULE (2026-05-02 to 2026-05-12)

The project notices that 209 functions are bridged via `replace_with_asmfile` after rounds of "this one's too hard" — and pushes back. THE HARD RULE is added and enforced via the active-marker hook.

| Date | Milestone |
|---|---|
| 2026-05-02 | **Status snapshot.** 1,034 real C / 304 inline asm / 72 asmfix bridges / 6 INCLUDE_RODATA jump tables. Still 376 functions of asm in disguise. |
| 2026-05-02 | Recipes accumulate: LICM unhoist, call-loop family, early-exit alias breaker, varargs prologue, nested-bool memcard. Named recipes saved as JSON in `tools/recipes/`. |
| 2026-05-04 | 60-commit day — heavy matching push. |
| 2026-05-10 | **Inline-asm audit.** 18 committed/audited functions with suspect non-canonical inline asm. `tools/gen_work_queue.py` now live-scans `src/*.c` for inline-asm debt and injects into `WORK_QUEUE.md` as `inline_asm_debt` tags. |
| 2026-05-10 | 135-commit day — most active day in project history. Mostly small mechanical matches + tooling fixes. |
| 2026-05-11 | **Stop-event hook (`tools/hooks/grind_check.sh`)** added. Rejects wrap-up language ("next session can continue", "diminishing returns", "I've made substantial progress") while a function is unmatched and active. The Claude Code agent doing the matching cannot voluntarily stop until the function is matched + committed. |
| 2026-05-12 | **Active-marker hook (`tools/hooks/active_func_guard.sh`)** added/refined. Enforces THE HARD RULE: `git commit` blocked unless `dc.sh verify <active>` returns MATCH; `git checkout`/`restore`/`reset` blocked on src/ files; `dc.sh next*` blocked while a function is in progress. `dc.sh release` is the only escape hatch (user-driven, typed confirmation). |
| 2026-05-12 | **Subagent orchestration archived.** Prior parallel-worker / orchestrator tooling moved to `archive/`. Solo end-to-end becomes the canonical working mode. |
| 2026-05-12 | **Subsystem map + naming triage** captured ([`SUBSYSTEM_MAP_2026-05-12.md`](handoffs/2026-05-12-subsystem-map.md), [`AUDIT_EXISTING_C_2026-05-12.md`](handoffs/2026-05-12-audit.md), [`NAMING_TRIAGE_2026-05-12.md`](naming/2026-05-12-triage.md), [`KENGO_RENAME_QUEUE_2026-05-12.md`](naming/2026-05-12-rename-queue.md)). Address ranges and content map for every `src/*.c` file. 17 score-4 Kengo names triaged into keep / demote / rename decisions. |
| 2026-05-12 | **Quick reference consolidation.** ~11 historical memory files merged into `feedback_quick_reference.md` (the symptom-indexed playbook). |

## Phase 6 — Bridge retirement + canonical inline-asm formalization (2026-05-13 to present)

The current phase: retiring bridges one at a time, formalizing the small canonical-asm category for functions that genuinely can't be C, and continuing to deepen the matching toolbox.

| Date | Milestone |
|---|---|
| 2026-05-13 | **`inline_asm_canonical.txt` formalized.** Functions whose ORIGINAL implementation was hand-written assembly (custom calling conventions, GTE primitives, BIOS trampolines, hand-coded math kernels) get authorized inline-asm form. Six initial entries: `func_8004A76C` (custom `$s0` ABI), `func_8007F87C` + cluster (sin/cos rotation kernels, scanner STRONG 5/5), `func_8007F5EC` + sibling (3-axis Euler rotation, hand-scheduled multu pacing). |
| 2026-05-13 | **`scan_hand_coded.py` and `memory_check.py` added.** Data-driven hand-coded-asm signal detection (5 signals: uniform multu pacing, empty-body branch, front-loaded loads, INT_MIN guard, tight register packing). `memory_check.py` surfaces project memory mentions + sibling memory hits before launching the matching pipeline. |
| 2026-05-13 | **`subst_multi` and `splice` regfix ops** added (K-to-N instruction transformations). Closes the gap where `subst`+`insert_after` chains race with maspsx debug nops. Unblocks `func_8002D320` (6→0 diffs). |
| 2026-05-13 | Eight functions matched in one day including major bridge retirements: `exec_game` (10ee50c), `func_800325E0` (spatial-audio), `func_80031890` (motion + rotation). |
| 2026-05-15 | **45 BIOS-trampoline primitives authorized en masse** in `inline_asm_canonical.txt` after manual review of all 39 candidates against `asm/funcs/<name>.s`. Pattern: 3-instruction sequences with the function number in `$t1` in the `jr` delay slot — no C analog. |
| 2026-05-15 | **CU sandwich-split technique** used for `func_80077B30` (text1b_b.c) and `func_8006B578` (text1b_a.c) — large-function isolation via second CU split + splice cascade fixes. Both were reverted same-day after sibling regressions; further work continues. |
| 2026-05-15 | **`saTan2KabutoWareMove` bridge retired** via regfix splice — proof that previously "permanently bridged" functions can be matched with deeper regfix work. |
| 2026-05-15 | **Asm-cheat detector (`tools/audit_asm_cheats.py`)** integrated into active_func_guard hook. Detects: large splice rules (regfix.txt force-rewrites), file-scope `__asm__("glabel ...")` function bodies, wildcard `subst .*` patterns, multi-insn C-body `__asm__` smuggling. Surfaces existing cheats in the session briefing. |
| 2026-05-16 | **Auto-drift-repair (`tools/auto_drift_repair.py`)** integrated into `dc.sh build-active`. Auto-detects cascade-drift symptoms (asmfix `did not match` warnings, doubly-defined `.L<N>` labels, regfix subst no-match) and runs `fix-asmfix-drift` + `fix-label-drift` then rebuilds. Modifies asmfix.txt / regfix.txt; included in commits. Includes SHA1-silent pair-shift drift detection via MISMATCH line. Rolls back on no-improvement, detects maspsx `.set noreorder` stripping. |
| 2026-05-16 | Six more matches today (~2026-05-16) including `func_8004C388` (xyz+packed-rgb midpoint averager, trapping adds via single-insn `__asm__`), `func_80052720` (GTE sqr wrapper), `func_80052C28` (sub/addi trapping), `func_80052788` (GTE gpf/gpl LERP wrapper, pure C with hardcoded `$reg` asm). |
| 2026-05-22 | **Tooling-error forced-fix system added** (`tools/hooks/tooling_error_guard.py` PostToolUse + `tooling_incident_stop_guard.sh` Stop/SubagentStop, wired in committed `.claude/settings.json`). Hardens the `debugging-discipline` rule into enforcement: a known tooling failure (CRLF, WSL-unavailable, broken worktree symlink, missing dep, core tool off PATH) raises a `.bb2_tooling_incident.json` marker that blocks turn-end until fixed permanently via `dc.sh fix-tooling-incident`. Signatures in `tools/hooks/tooling_error_signatures.json`; ledger in `docs/tooling_incidents.md`. See `docs/TOOLING_ERROR_GUARD.md`. |
| 2026-05-22 | **`active_func_guard.sh` PreToolUse hook deprecated** (unwired from `.claude/settings.local.json`). It blocked `git commit` until match, blocked `git checkout`/revert of in-progress build files, and blocked `dc.sh next*` while a function was active+unmatched — obstructing the natural "try an approach, revert if it fails" loop. Staying-on-task is now enforced solely by the `grind_check` Stop hook. The programmatic commit cheat-audit it ran (`audit_asm_cheats.py --check-new`, "Rule 1a") was **relocated** to `tools/hooks/commit_audit_guard.sh`, so anti-cheat enforcement is unchanged. Script kept (unwired) for reference. |

## Standing items

- **Bridged functions:** ~148 active `replace_with_asmfile` lines in asmfix.txt. The asmfix retirement queue is the primary work stream pulling these down toward zero.
- **Inline-asm debt:** live-scanned by `tools/gen_work_queue.py` from `src/*.c`; suspect patterns are surfaced as `inline_asm_debt` in the active queue.
- **Permanent blockers:** 7 functions in `known_blocked.txt` (BIOS jumptables, overflow-trapping ops, `$sp` swap, data-as-code). Filtered out of the active queue.
- **Authorized canonical inline-asm:** ~80 functions in `inline_asm_canonical.txt` (BIOS trampolines, GTE primitives, hand-coded math kernels). Tracked as the "1.0 final form" for genuinely non-C code.

## Major handoff documents

| Document | Purpose |
|---|---|
| CODEX_HANDOFF_2026-04-17.md (no longer present) | First Codex session handoff. Documents the shift from m2c/permuter-heavy workflow to the codex_lab + regfix/asmfix discipline. Introduced asmfix and the lab pattern. Never tracked in git; deleted during 2026-05-18 repo cleanup. |
| [`CODEX_HANDOFF_2026-04-27.md`](handoffs/2026-04-27-codex.md) | Zero-stub sweep complete. Documents the late-stage zero-stub strategy (smallest-safe-C + asmfix bridge) and the stabilizers (`single_game_VoiceContorol`, `camera_set_zoom`, pinned helpers). |
| [`AUDIT_EXISTING_C_2026-05-12.md`](handoffs/2026-05-12-audit.md) | Audit of the finished C surface as of 2026-05-12. Bridge-signature cleanup, naming-suspect list, subsystem map provenance. |
| [`NAMING_TRIAGE_2026-05-12.md`](naming/2026-05-12-triage.md) | Per-symbol triage of 17 score-4 Kengo-derived names. Keep / demote / rename decisions with evidence. |
| [`SUBSYSTEM_MAP_2026-05-12.md`](handoffs/2026-05-12-subsystem-map.md) | Object-level map: what lives in each `src/*.c` and at what address range. Plus a line/address landmark map for `text1b.c`. |
| [`KENGO_RENAME_QUEUE_2026-05-12.md`](naming/2026-05-12-rename-queue.md) | Reviewed action queue for Kengo-derived renames, machine-readable in `kengo_name_decisions.csv`. |
| [`CLAIMS.md`](../CLAIMS.md) | Active session claims and stabilizer-fragility tracking (kept up-to-date when significant cross-session coordination is needed). |
