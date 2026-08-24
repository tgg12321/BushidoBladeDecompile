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
| 2026-05-26 | **Named-recipe library archived.** With the `dc.sh` workflow retired in favor of the engine, the 16 `tools/recipes/*.json` "fingerprinted technique" files — plus their `capture_recipe.py` registrar and `recipes.py` matcher — were moved to `archive/dcsh_workflow_2026-05-26/recipes/` (the engine never consumed recipes, leaving them orphaned and misleadingly live-looking). Technique *knowledge* survives in `docs/MATCHING.md` and the path-scoped `.claude/rules/*.md` docs. A formalized **"Register findings"** loop step (CLAUDE.md) + a `retire` nudge replaced the old `capture-recipe` flow: agents now record reusable patterns directly into `.claude/rules/`, which the metrics layer fingerprints as technique slugs. |
| 2026-05-22 | **`active_func_guard.sh` PreToolUse hook deprecated** (unwired from `.claude/settings.local.json`). It blocked `git commit` until match, blocked `git checkout`/revert of in-progress build files, and blocked `dc.sh next*` while a function was active+unmatched — obstructing the natural "try an approach, revert if it fails" loop. Staying-on-task is now enforced solely by the `grind_check` Stop hook. The programmatic commit cheat-audit it ran (`audit_asm_cheats.py --check-new`, "Rule 1a") was **relocated** to `tools/hooks/commit_audit_guard.sh`, so anti-cheat enforcement is unchanged. Script kept (unwired) for reference. |

## Phase 7 — Engine + rodata cleanup + SOTN-standard alignment (2026-05-27 to 2026-07-05)

The `dc.sh` workflow retired; the deterministic **engine** (`engine/`,
`python3 -m engine.cli`) became the standing per-function loop. Big
infrastructure retirement + rule-catalog crystallization phase.

| Date | Milestone |
|---|---|
| 2026-06-01 | **"No new cheat-tolerant park categories" user policy codified** (`.claude/rules/no-new-park-categories.md`). Register-rotation, cross-jump-merge, prologue-order walls are pure-C-reachable — not new infrastructure carve-outs. The bar is SOTN's. |
| 2026-06-02 | **Techniques audit**: 3 catalog rules retroactively identified as cheats-by-any-spelling. Adversarial `cheat-reviewer` process becomes MANDATORY layer-2 gate for every completion commit. Sanctioned families crystallized after SOTN-master-branch evidence census. |
| 2026-06-08 | **No-new-regfix-rules policy** (`.claude/rules/no-new-regfix-rules.md`). `commit-msg` hook enforces net-zero rule additions by default; escape-hatch category tags for genuinely-new infrastructure only. |
| 2026-06-09 | **Rodata cleanup Phase A COMPLETE** (`docs/rodata-cleanup-project.md`). All 12 `asm/data/*.rodata*` blocks retired from `bb2.ld`. Later that day: **26 misrouted ASM-STRUCTURAL items parked** with hand-coded-signal audit; the canonical gate hardened to require BOTH distance>500 AND `scan_hand_coded` tier ≥ POSSIBLE. |
| 2026-06-10 | **Layer-2 mandatory cheat-reviewer** ratified as ACCEPTANCE gate. First-layer (in-session) reviewer is provisional; a fresh default-FAIL reviewer confirms. |
| 2026-06-24 | **No-park-permanently** user directive. Every function reaches either COMPLETED-C, COMPLETED-INLINE-ASM-CANONICAL, or stays INCOMPLETE in queue. No "permanent park" bucket. |
| 2026-07-01 | **SOTN-family research 2026-07-01** ratified 4 new narrow last-resort carve-outs: `dead-store-fake-exception`, `named-local-fake-exception`, `pointer-alias-fake-exception`, `mmio-volatile-type-level`, plus `duplicated-statement-into-arms`. Each requires `/* FAKE */` annotation + documented lever-exhaustion + layer-1/2 review. |
| 2026-07-06 | **The Grinder** (`tools/grinder/`) — new default autonomous pipeline. Single-lane deterministic driver, per-function persistent ledger (`memory/grind/`), driver-enforced modality ladder, default-FAIL Judge. Supersedes the multi-agent fleet (which is now retired). Spec: `docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md`; skill: `decomp-grind`. |

## Phase 8 — Closer mission (Sony PsyQ library adoption) (2026-07-09 to 2026-07-13)

Manual close-out mission to adopt SOTN-matched Sony PsyQ 4.0 psxsdk C source
for BB2's census-proven verbatim-linked library functions. Ran in
parallel with the Grinder on unrelated queue items. **Retired 2026-07-13**:
all 7 legitimately-banked candidates map to functions still active in
`engine/queue.json`, so the Grinder inherits every remaining item by
construction — no Class-D (novel-lever) work was left that a dedicated Closer
session would do better.

| Date | Milestone |
|---|---|
| 2026-07-09 | **PsyQ census landed** (`memory/closer/psyq-library-census.md`, `psyq-queue-hits.json`). 177 verbatim Sony PsyQ 4.0 module placements identified in the EXE (68,208 bytes = 11.3% of the image is bit-verbatim Sony library code). 92 queue items map to Sony library functions and become the closer's work list. |
| 2026-07-10 | **Sessions 3-11**: ~20 functions closed via SOTN psxsdk transcription. 4 owner-gated volatile-grant proposals filed (§1 SIO, §2 _spu_RQ, §3 cdread per-member, §4 canonical-asm content edit) — all ratified within days. Session 5 discovers the LIBGTE cluster is canonical-asm territory, not pure-C adoptable. |
| 2026-07-10 | **LIBSND ground-truth hunt** (`memory/closer/libsnd-hunt-report.md`). BB2 links an interim 4.0-lineage sound lib not in public archives (Jun-Sep 1997 build); most LIBSND-gap items remain excluded from the work list. Only `_SsSndStop` gets confirmed and added. |
| 2026-07-11 | **LIBGTE canonical-asm authorization pass**: 10 functions retired as COMPLETED-INLINE-ASM-CANONICAL in one commit (`9eba9a3e`). 27 forbidden `dead-branch-scheduling` regfix rules cleared alongside. Infrastructure finding: `.set reorder/at` at end of file-scope `__asm__` block combined with a subsequent `.section .text`-opening block causes maspsx to insert a stray load-delay nop — fix documented in `canonical-asm-authorization-recipe.md`. |
| 2026-07-12 | **`hoist-shared-arm-computation-defeats-copy-pref` sanctioned** as a new pure-C RA lever. Confirmed case: `saTan2Main` (banked at floor 5 since 2026-07-10). Rule doc in `.claude/rules/`. |

## Phase 9 — Toolchain fidelity, asmfix-to-zero, naming (2026-07-14 to present)

The phase that moved the project's leverage from per-function grinding to
*config fidelity* (make our compiler behave like Sony's), *class-wide debt
retirement* (delete an entire cheat category at once), and *evidence-backed
naming*. The Grinder ran continuously underneath it all.

| Date | Milestone |
|---|---|
| 2026-08-04 | **`-mel` adopted** into canonical `CC_FLAGS`. The prebuilt cc1's `mips-mips-gnu` triple defaulted to BIG-endian on a little-endian target, corrupting spill-slot layout, bitfield direction and lwl/lwr offsets; `-mel` fixes it at the source and **retires the `fix_lwl` pipeline stage** entirely. 20 `-mel`-obsoleted offset rules retired the same day. Both `-mel` and the empty `FIX_LWL_FILES` are now load-bearing for the oracle ([[mel-endianness-adoption]]). |
| 2026-08-04 | **`tools/ra_solver`** — GCC 2.7.2's *entire* allocation stack modelled and ground-truth validated: global.c allocation, local-alloc, reload/`retry_global_alloc`, and the suggested-register pass. Turns "which register will GCC pick" from intuition into arithmetic ([[ra-solver-campaign-2026-08-04]]). |
| 2026-08-05 | **`tools/sched_solver`** — sched.c's list scheduler modelled EXACTLY: 6,978/6,978 blocks reproduced across both passes, plus a perturbation layer and a goal mapper (target order → RTL UIDs) ([[sched-solver-campaign-2026-08-05]]). |
| 2026-08-05 | **Per-file `-G8` adopted for `text1a`** (owner-approved, flag-evidence backed), enabling the top-level-asm extraction class. |
| 2026-08-06 | **Owner ruling: ALL `asmfix.txt` entries are debt** — including the 65 canonical-extraction wirings. The end state is zero regfix + zero asmfix ([[asmfix-all-debt-end-state]]). |
| 2026-08-06 | **Campaign 4 — asmfix-to-zero** (Waves 0-7). Scoped as 205× in-place `INCLUDE_ASM` conversion + 1 true TU re-split; executed to completion. `asmfix.txt` fell from 153 rule-carrying functions to 14, and `replace_with_asmfile` bridges from 140 to 3. The CANON-EXTRACT queue category ended EMPTY. |
| 2026-08-06 | **65 canonical text1b bodies extracted** to `asm/funcs/` (owner-approved, `[infra-rule: canonical-asm-extraction]` — sanctioned with mechanical narrowness: every added rule must be `replace_with_asmfile` against a function already in `inline_asm_canonical.txt`). `text1b.c` shrank 17,743 → 7,440 lines. The wave regressed 66 canonical completions to one wiring rule apiece, which Campaign 4 then retired ([[canonical-extraction-resurrection-wave]]). |
| 2026-08-07 | **Scorer fix: section-relative `R_MIPS_LO16` addends masked.** A function taking the address of a `.text` symbol read as distance 1 forever — a scorer artifact, not a gap ([[sandbox-lo16-text-addend-false-distance]]). `saEft00Add` had been reverted over it. |
| 2026-08-07 | **`tools/spotcheck/`** — standing guards against silent COMPLETED-C regressions (four-mode per-function check + whole-corpus queue-regen diff). A completion had previously been verified exactly once, at completion time. |
| 2026-08-07 | **COMPLETED-C count made definitional**: 12 data-as-code symbols (`.include`d asm bodies, `.aent` alternate entries, an instruction-less glabel marker) excluded from the pool by owner ruling, applied to both counters in one commit so they can never disagree. |
| 2026-08-07 | **Function-naming census + waves.** Phase 1 tiered all 1,436 functions by name evidence; the libscan verbatim wave applied **334 Sony names**; the phase-2 reset wave applied **300 RESET + 2 RENAME**. Every wave oracle-verified byte-neutral, applied only via `tools/naming_wave.py` (names are pipeline keys — [[naming-wave-tool]], [[names-require-evidence]]). |
| 2026-08-10 | **XDEF boundary fixes**: `note2pitch`, `_spu_FiDMA` + `_spu_Fr_`, and `_spu_2pitch` split out of the functions they had been glued into, plus an addendum wave of 12 renames. |
| 2026-08-11 | **Canonical-sweep decision packet** (A=1 / B=4 / C=81) + library xref (25 census-matched) + reference-body measurements. `_SsSeqPlay` closed by reference adoption; owner-ruled ternary grant closed the `get_cs`/`get_ce` twins; `func_80052930` authorized as canonical, completing the gte-3x3 cluster. |
| 2026-08-14 | **WSL bridge on by default** in `tools/wteng.ps1`, with grinder state reporting — cutting `wsl.exe` invocations (each leaks a kernel Job object — [[wsl-kernel-object-leak-audio]]). |
| 2026-08-24 | **Parked-set review + migration sweep 2 + doc/hygiene audit.** All 33 parks audited against the two-gate standard (28 stand). The 2026-08-19 migration's rodata-refusal heuristic was found counting quotes inside cheat constructs as `.rodata` evidence — every pin-carrying parked body had false-failed its own eligibility check. Detector fixed (`strip_asm_constructs`), 9 parked pin-carriers migrated to `INCLUDE_ASM` (30 pins, 4 asm blocks/barriers, 1 phantom array, 7 rules removed from main; oracle SHA1 MATCH), deferred set 68 → 56. Metadata repaired (stale floors incl. func_80057CC8 3→30, park prose, inert prologue_config entry). Repo-wide documentation audit refreshed every stale surface (STATUS/README/CLAUDE/AGENTS/conventions, cheat-reviewer brief drift, retired-skill banners, nonpaged-pool advisory removal per the 2026-08-19 directive). |
| 2026-08-19 | **The self-serve + asm-until-matched day.** (1) Three overnight bytes-proven INTEGRATION HANDOFFs (SioSyncroWrite, func_8001B748, func_8002D518) operator-integrated to COMPLETED-C under two-layer review — then made pipeline-executable forever ([[integration-handoff-self-serve]]: driver widens scope / clears Judge-superseded bans on Judge verdicts; `grindlib unban`, `queue unpark` added). (2) Stale-park re-audit: 6 of 33 parks reclaimed under post-park family rulings. (3) Modality-effectiveness report (`docs/grind/modality-effectiveness-2026-08-19.md`) → ladder retune (synthesis to s6, permuter cap 2, closing-modality instrumentation) + paperwork-FAIL fixes (family-selection table, citation prechecks, CITATION fix-up ground). (4) **asm-until-matched migration** ([[asm-until-matched]]): 191 INCOMPLETE functions converted to `INCLUDE_ASM`, 865 rules retired, oracle-verified per batch; 68 byte-coupling deferred (wave-2 mechanical retirement measured a dead end). Queue distances now pin to ledger honest floors. |


## Standing items (2026-08-24)

- **COMPLETED-C**: 1,051 functions (per `tools/check_completion_integrity.py`, the authority).
- **COMPLETED-INLINE-ASM-CANONICAL**: 179 functions in `inline_asm_canonical.txt` (BIOS trampolines, GTE primitives, hand-coded math kernels, LIBGTE modules).
- **INCOMPLETE queue**: 244 items (211 active, 33 parked). Verdict breakdown: 208 C / 36 ASM-PARTIAL. INCOMPLETE = committed as `INCLUDE_ASM("asm/funcs", <func>);` except the 56 byte-coupling deferred ([[asm-until-matched]]).
- **Rules**: 689 outstanding across 37 functions (all inside the 56 deferred) — down from 1,573 on 2026-08-17; all debt per the 2026-08-06/2026-08-19 rulings, retiring per function at COMPLETED-C.
- **Retired work streams**: `dc.sh` workflow (2026-05-26), named-recipe library (2026-05-26), multi-agent fleet (2026-07-06), `dc.sh active_func_guard` hook (2026-05-22), Closer Phase 3 (2026-07-13).
- **Owner escalation shelf**: `docs/escalations/` — incl. the cc1 fork-divergence ruling request (`_spu_FiDMA`: our decompals fork segfaults on Sony's faithful volatile-MMIO wait loop that original cc1psx compiles to target bytes).

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
