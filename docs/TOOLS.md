# Tools

This document catalogs the tooling shipped in `tools/`. Most decomp operations go through `tools/dc.sh`, which dispatches to roughly 60 named subcommands. A smaller set of standalone Python tools handles tasks not covered by `dc.sh` (disc extraction, splat preprocessing, etc.).

For matching techniques themselves see [`MATCHING.md`](MATCHING.md). For the overall build pipeline see [`ARCHITECTURE.md`](ARCHITECTURE.md).

## `tools/dc.sh` — the central wrapper

`tools/dc.sh` is the entry point for almost all decomp operations. It dispatches subcommands to underlying Python scripts, sources the Python virtualenv automatically, and handles working directory and config conventions for you. **Never construct raw WSL pipelines for compile/score/build** — use `dc.sh`.

Run all commands from the project root inside WSL:

```bash
bash tools/dc.sh <subcommand> [args]
```

The subcommands are grouped below by workflow phase.

### Workflow drivers

These are the commands you'll run at the start and end of every session.

| Subcommand | Purpose |
|---|---|
| `dc.sh start` | Session startup briefing. Reports build status, active-marker state, queue freshness, top of queue, and the rules summary. Run as your first action of every session — also auto-run by the SessionStart hook in Claude Code sessions. |
| `dc.sh next [N\|--with-context]` | Pull the next function from the active decomp queue. With no argument, claims the top-1 function and writes `.bb2_active_func`. With `N`, previews the top N without claiming. With `--with-context`, also runs `agent-brief` for the claimed function. |
| `dc.sh next-structural [N]` | Same as `next`, but pulls from the structural split queue (functions needing rodata splits or function-boundary splits). |
| `dc.sh next-asmfix [N]` | Pull from the asmfix retirement queue (bridged functions to retire). |
| `dc.sh next-cheat [N]` | Pull from the active queue filtered to `*_debt` tags (inline-asm debt cleanup). |
| `dc.sh release` | **User-only escape hatch.** Requires typing the function name to confirm. Releases the active marker without a successful match. |
| `dc.sh refresh-queue` | Regenerate `WORK_QUEUE.md` from `tmp/batch_attempt.csv` + live scans. Run after a batch of matches to drop matched functions from the queue. |
| `dc.sh classify <func>` | One-line pre-dive report: size, blockers, BIOS/syscall/PsyQ-stdlib tags, recommendation. |
| `dc.sh agent-brief <func>` | Full context dump for a function: classify + asm + base.c + gen_regfix + recipe suggestions + Kengo + neighbor functions. The fastest way to get oriented on a new function. |
| `dc.sh memory-check <func>` | Surface project-memory mentions of `<func>`, sibling memory hits, hand-coded-asm signal tier (STRONG / POSSIBLE / TIGHT_C / LOW), canonical-asm status. **Run before manually attacking pure C** — catches functions that should be in `inline_asm_canonical.txt`. |
| `dc.sh attempt <func>` | Full mechanical pipeline: classify → setup → smart_match → permute_capped → gen_regfix. Reports MATCHED / NEAR_MISS / HARD / SKIPPED. The default first-touch on a new function. |

### Per-function diagnosis

When `attempt` doesn't immediately match (most cases), these tools tell you why.

| Subcommand | Purpose |
|---|---|
| `dc.sh preflight <func>` | One-screen brief: bridge state, source location, classification, size, blockers, Kengo equivalent, existing regfix/asmfix rule counts, sibling commits to clone, one-line recommendation. Best replacement for a chain of `classify` + `agent-brief` + manual greps. |
| `dc.sh preflight-cascade <func>` | Read-only impact report before integrating. Lists how many sibling rules in the function's `.c` file use literal `.L<N>` labels — those rules can silently break when integration shifts GCC's file-wide label counter. |
| `dc.sh diff <func>` | Side-by-side diff: original asm (target) vs build pipeline output (mine), per-instruction. Index-aligned; cascades on length differences. |
| `dc.sh diff-align <func>` | **Recommended first diagnostic.** Sequence-aligned binary diff with relocation masking and recipe detection. Collapses cascade (a 1-instruction shift shows as ONE diff, not 145) and suggests fix recipes (delay-slot fill, label-shift, hoist removal). |
| `dc.sh side-by-side <func>` | Shape-aligned binary diff with per-row category (MATCH / REG-RENAME / BRANCH-OFFSET / STRUCTURAL). Cascade-immune by construction. Use to triage which diffs are register renames vs structural changes. |
| `dc.sh diff-summary <func>` | Categorized verdict with one suggested next action. ~200B output. The fastest way to decide what to do next. |
| `dc.sh verify <func>` | Binary-level verification one function against original. **Bridge-blind** — always reports MATCH for bridged functions. |
| `dc.sh verify-c <func>` | Bridge-aware verify — refuses if `asmfix.txt` has an active `replace_with_asmfile` for `<func>`. Use during retirement work. |
| `dc.sh verify --all` | Verify the whole binary. SHA1 short-circuits on match (~0.2s). |
| `dc.sh verify --clean` | Wipe `build/`, rebuild from scratch, then `verify --all`. The trustworthy long-form. Use before commits with complex regfix or register-asm pins. |
| `dc.sh score <func_dir>` | Permuter score for a `permuter/<func>/` directory (just the number). |
| `dc.sh debug <func_dir> [--full]` | Permuter penalty list + score. Default omits the ~200KB side-by-side asm; `--full` includes it. The penalty list alone tells you whether the remaining diff is structural (Ins/Del/Reord) or register-only — and that's the routing decision you want. |
| `dc.sh compile <func_dir>` | Compile a permuter base.c via `compile.sh` and dump the disassembly. |
| `dc.sh analysis <func>` | Run `tools/asm_analysis.py` on the function's asm. |
| `dc.sh siblings <func> \| --all` | Find structural siblings (other functions with similar shape). Useful when matching one function might unlock several. |
| `dc.sh dump-text <func> [--post-regfix]` | Numbered TEXT indices from the build pipeline. Use before writing regfix rules. `--post-regfix` shows indices AFTER regfix passes. |
| `dc.sh post-match-validate <func>` | After per-function MATCH: SHA1 short-circuit; if fails, walks siblings, surfaces suspicious regfix rules. Run before `git commit`. |
| `dc.sh caller-audit <func>` | Scan callers' max arg arity. Mismatch = cascade-regression risk. Auto-runs as part of `dc.sh inline-replace`. |
| `dc.sh diagnose-hoist <func>` | Compare mine vs target callee-save constant hoisting; recommend inline-asm-defeat / register-asm-force recipes. |
| `dc.sh find-label-at <func> <hex_addr>` | Resolve a target byte address to the right `.L<N>` label in mine. Eliminates trial-and-error label hunting. |

### Matching automation

Tools that try to find a match (or close part of one) automatically.

| Subcommand | Purpose |
|---|---|
| `dc.sh smart <func>` | `smart_match.py`: 16 automated transformation strategies (declaration reorder, cast variations, do-while barriers, register hints, etc). Pure-C exploration before reaching for the permuter. |
| `dc.sh permute <func> [--max-time N] [--max-flat N]` | `permute_capped.py`: bounded permuter run with flat-score early termination. |
| `dc.sh permute-adaptive <func> [--dry-run]` | Permuter with budget scaled to penalty count: 0 ins/del → skip, 1-2 → 90s, 3-5 → 5min, 6-10 → 15min, >10 → 30min. |
| `dc.sh near-miss <func> [--apply]` | Auto-detect `byte_arith_fix` / `drain_delay` / `plain_reg_substs` patterns and apply them with try-and-revert. |
| `dc.sh recipes [<func>]` | List recipes (no arg) / suggest recipes for `<func>`. |
| `dc.sh apply-recipe <recipe> <func>` | Print concrete `add-regfix` commands for a named recipe. |
| `dc.sh capture-recipe [<commit>]` | After committing a match, classify the patterns used and report whether it matches an existing recipe or is novel. `--write` saves a draft JSON. |

### regfix tooling

Tools for authoring and maintaining `regfix.txt`.

| Subcommand | Purpose |
|---|---|
| `dc.sh add-regfix <op> <func> <args>` | Append a validated regfix rule. **Pre-validates** against live `dump_text_indices` before append (catches `$0`-vs-`$zero` patterns and out-of-bounds idx). **Auto-rolls back** on live-build validation failure. Ops: swap, subst, delete, insert, insert_after, reorder, fill_delay, drain_delay, insert_label. |
| `dc.sh regfix-suggest <func> [--apply] [--max-rules N]` | **Run BEFORE writing any regfix rule by hand.** Diffs target.s vs the live post-regfix build pipeline and emits `delete`/`insert_after`/`subst` rules that close the gap. Catches gp-rel pseudo expansions (emits `sdata_exclude.txt` hints) and label drift. Default max 40 rules — anything more usually means structural mismatch (reach for permuter instead). |
| `dc.sh gen-regfix <func> [src]` | Older auto-generator. Useful for varargs/early-exit/frame patterns; for most cases, `regfix-suggest` supersedes. Don't run on pure register-cycle diffs (floods 200+ "structural diff" lines). |
| `dc.sh validate-regfix [--func F] [--live]` | Validate `regfix.txt` rules. `--live` checks against current pipeline output. |
| `dc.sh fix-label-drift [--apply] [--also-verify]` | Auto-fix `.L<N>` drift in regfix rules. Default is dry-run. Linker-driven by default (catches undefined labels only); `--also-verify` runs `verify --all` to catch wrong-but-existing label drift (slower, ~3-5 min). |
| `dc.sh fix-asmfix-drift [--apply]` | Auto-fix `.L<N>` drift in `asmfix.txt` rename rules via address-based lookup. |
| `dc.sh regfix-drift-immune [--apply]` | Audit `regfix.txt` for `subst` rules with hardcoded `.L<N>` labels in patterns; rewrite to `\.L\d+` regex where the replacement is project-custom (not itself a GCC label). |
| `dc.sh frame-shift <func> [--delta N] [--apply]` | Auto-generate frame-cascade regfix rules when GCC's frame size differs from target by N bytes. Emits prologue/epilogue substs + all sw/lw stack-offset shifts as one batch. |
| `dc.sh asmfix-slice <func> <start_label> <end_label> [--apply]` | Lift a target asm slice into asmfix.txt as `delete_between` + `insert_before` rules. Last-resort tool when C + permuter + regfix can't reproduce target's scheduler decisions. Stay strictly inside the function body. |
| `dc.sh fix-branch-drift <func> [--apply]` | Auto-emit `.word`-encoded subst rules for branch-offset diffs (the residual end-game when STRUCTURAL/REG-RENAME diffs are gone but branches still differ in immediate only). |
| `dc.sh gen-substs <func> [--apply]` | Auto-generate regfix `subst` rules for REG-RENAME diff rows from the shape-aligned binary diff. |
| `dc.sh binary-diff-count <func>` | JSON: `{structural, rename, branch_offset, total}` for one function. Used by `build_active.py` to drive REGRESSION detection. |
| `dc.sh branch-offsets <func>` | JSON dump of BRANCH-OFFSET rows (branch instructions whose opcode+registers match but immediate differs). |
| `dc.sh purge-retirements [--dry-run]` | Manually purge `# RETIRE: <func>: replace_with_asmfile` lines from `asmfix.txt`, only for functions that pass `verify-c`. |

### Inline-asm-aware setup

For functions still implemented as inline `__asm__()` blocks in C files.

| Subcommand | Purpose |
|---|---|
| `dc.sh inline-locate <func>` | Find the inline `__asm__()` block for `<func>` in `src/`. |
| `dc.sh inline-verify <func>` | Verify the inline asm matches `asm/funcs/<func>.s`. |
| `dc.sh inline-setup <func>` | Stage `permuter/<func>/` for an inline-asm function (m2c base.c + target.s). |
| `dc.sh inline-replace <func> <c_file>` | After matching, swap the inline block for matched C. Auto-runs `caller-audit` and blocks if signature arity is wrong. |

### GTE helpers

For functions using the PS1 GTE (Geometry Transformation Engine) coprocessor 2.

| Subcommand | Purpose |
|---|---|
| `dc.sh gte <func>` | `gte_classifier.py`: which `gte_*()` macros from `include/gte.h` would the function need? |
| `dc.sh gte-migrate <func> [--stdout\|--setup]` | `gte_migrate.py`: convert an inline-asm GTE function into a C scaffold using `gte_*()` macros + properly-constrained `__asm__` for remaining mtc2/mfc2 ops. Writes `permuter/<func>/base.c`. |

### Bridge and retirement tooling

For working with the asmfix `replace_with_asmfile` bridge mechanism.

| Subcommand | Purpose |
|---|---|
| `dc.sh retire <func>` | Start retirement of a bridged (`replace_with_asmfile`) function: comments out the asmfix rule with `# RETIRE:`, sets the active marker, prints C-body location. Refuses if not bridged or if build is mismatched. |
| `dc.sh audit-bridges` | Caller-signature audit on all bridged functions. Reports ACTIVE / latent / informational issues. JSON written to `tmp/bridge_signature_audit.json`. |
| `dc.sh build-active <func>` | Incremental rebuild for one function: nuke the `.o` for the containing `.c` file + link products, run make (~30s vs ~2 min for full clean), then run bridge-aware `verify-c`. Use during the edit-iterate loop. Auto-detects cascade-drift symptoms and auto-runs `fix-asmfix-drift` + `fix-label-drift` then rebuilds. |
| `dc.sh iter-log <func>` | Per-function build trajectory: shows recent rounds + plateau advice. Auto-fed by `build-active`. Use to check if you're stuck on the same diff count and should switch technique. |
| `dc.sh auto-repair` | Build wrapper that detects cascade-drift symptoms (asmfix `did not match` warnings, doubly-defined `.L<N>` labels, regfix subst no-match) and auto-runs `fix-asmfix-drift` + `fix-label-drift` then rebuilds. Used by `build-active` by default; can also be called directly to retry a stuck build. |

### General build & utility

| Subcommand | Purpose |
|---|---|
| `dc.sh build` | Incremental `make` + SHA1 check, tail output. |
| `dc.sh replace <src_file> <func_name> <c_file>` | Replace `INCLUDE_ASM` stub with C code from a file (LF-safe). **Don't use for final integration** — copies the standalone permuter base.c (with duplicate typedefs/externs). Use `inline-replace` instead. |
| `dc.sh setup <func> <src>` | Set up a permuter directory (`INCLUDE_ASM` stub form). |
| `dc.sh suggest <func>` | Lean per-function rollup: state + top 3 techniques + don't-tries + next single command, plus (post-build) diff routing and recipe candidates. |
| `dc.sh check-gp-layout` | Verify build's symbol layout matches canonical addresses. Catches silent gp-rel miscompiles. |
| `dc.sh check-permuter-parity` | Verify `tools/permuter_compile.sh`'s cc1 + maspsx flags match the Makefile's. |
| `dc.sh classify-batch <funcs...>` | Classify a list of functions; flag stale (already pure C) entries. |
| `dc.sh queue-structural [N]` / `queue-asmfix [N]` | Aliases for `next-structural --no-pull N` / `next-asmfix --no-pull N`. |
| `dc.sh cheats [--summary\|--list\|<func>]` | Surface existing asm-cheat patterns (large splice rules, inline `__asm__("glabel ...")` function bodies). |
| `dc.sh scan-hand-coded [--single <func>\|--all\|--json]` | Bulk scanner for hand-coded-asm indicators across `asm/funcs/*.s`. Reports STRONG / POSSIBLE / TIGHT_C tiers. |

## Standalone Python tools

These tools either run independently of the `dc.sh` flow, are invoked indirectly by `dc.sh`, or serve as one-off utilities. Most are in `tools/`; some live in `tools/recipes/` or `tools/hooks/`.

### Build pipeline (called by Makefile)

| Tool | Purpose |
|---|---|
| `tools/prologue_fix.py` | Per-function prologue rewrites (delay-slot `sw $ra`, varargs prologue fixups). Driven by `tools/prologue_config.json` and `tools/delay_slot_ra_funcs.txt`. |
| `tools/fix_lwl.py` | XOR `lwl`/`lwr`/`swl`/`swr` byte offsets with 3 to convert from GCC's big-endian byte ordering to PS1's little-endian. |
| `tools/multu_pad.py` | Per-function `multu` padding for hand-coded-asm patterns. Driven by `multu_pad_funcs.txt`. |
| `tools/regfix.py` | The regfix.txt processor. Phase-ordered application of swaps, substs, deletes, inserts, reorders, fill_delay, drain_delay. |
| `tools/asmfix.py` | The asmfix.txt processor. Label renames, regex substs, delete_between, insert_before, `replace_with_asmfile`. |
| `tools/maspsx/maspsx.py` | ASPSX compatibility layer (vendored from [mkst/maspsx](https://github.com/mkst/maspsx) with project-local patches in `tools/maspsx.patch`). |
| `tools/make_psexe.py` | Prepend the original PS-EXE 0x800-byte header to the linker's raw binary output. |

### Disc / asset handling

| Tool | Purpose |
|---|---|
| `tools/extract_iso.py` | Extract PS1 disc filesystem from BIN/CUE into `disc/`. Run once after first setup. |

### Splat preprocessing / postprocessing

| Tool | Purpose |
|---|---|
| `tools/canonical_funcname.py` | Resolve queue name → renamed canonical name from `asm/funcs/`. The queue can carry stale `func_<addr>` names from before a rename; the active marker must use the renamed form so `verify` succeeds. |
| `tools/sync_undefined_syms.py` | Sync `undefined_syms_auto.txt` between splat output and named overrides. |
| `tools/audit_named_syms_orphans.py [--apply]` | Strip orphan `named_syms.txt` assignments (entries that shadow a real text-section symbol now defined by a C `.o`). |
| `tools/rename_funcs.py` | Rename functions across asm/funcs/, named_syms.txt, and src/. |
| `tools/apply_kengo_names.py` | Apply Kengo PS2 sister-engine names to BB2 functions via `kengo_name_decisions.csv`. |
| `tools/audit_kengo_renames.py [--decided-only] [--csv]` | Audit applied Kengo renames; flag score-4 names as likely false positives. |
| `tools/audit_bridge_signatures.py` | Caller-signature audit on all bridged functions. |

### Matching / scoring

| Tool | Purpose |
|---|---|
| `tools/smart_match.py` | 16-strategy automated C-variation sweep. |
| `tools/permute_capped.py` | Bounded permuter wrapper with flat-score termination. |
| `tools/permute_adaptive.py` | Permuter with budget scaled to penalty count. |
| `tools/attempt_func.py` | Full mechanical pipeline: classify → setup → smart → permute → gen_regfix. |
| `tools/gen_regfix.py` | Auto-generate regfix rules from pipeline-vs-target diff. |
| `tools/regfix_suggest.py` | Newer regfix-rule generator. Knows maspsx idx conventions; emits gp-rel and label-drift hints. |
| `tools/near_miss_attempt.py` | Auto-detect common patterns (byte_arith_fix, drain_delay, plain_reg_substs); apply with try-and-revert. |
| `tools/score_match.py` | Compute raw objdump-style differing-instruction-words count. |
| `tools/recipes.py` | Named-recipe matcher (list / suggest / apply). Recipes live in `tools/recipes/*.json`. |
| `tools/capture_recipe.py` | Classify patterns used in a commit; offer to save as a draft recipe JSON. |
| `tools/decomp-permuter/` | Vendored copy of [`decomp-permuter`](https://github.com/simonlindholm/decomp-permuter) for randomized C variation search. |
| `tools/permuter_compile.sh` / `permuter_setup.sh` | Wrappers for the permuter to use BB2's pipeline. |

### Verification

| Tool | Purpose |
|---|---|
| `tools/regfix_verify.py [--all\|<func>] [--force]` | Binary-level verification against original. Used by `dc.sh verify`. |
| `tools/validate_regfix.py [--live]` | Static (`regfix.txt` lint) or live (rule application) validation. |
| `tools/verify_labels.py [--all\|--func F]` | Resolve every regfix label subst to actual target instruction. Run after substs that reference `.L<N>` in replacement. |
| `tools/check_gp_layout.py` | Verify symbol layout matches canonical addresses; catches silent gp-rel miscompiles. |
| `tools/check_permuter_parity.py` | Verify `permuter_compile.sh` cc1 + maspsx flags match Makefile's. |
| `tools/post_match_validate.py` | After per-function match: detect sibling regressions caused by `.L<N>` numbering shifts. |
| `tools/binary_diff.py` | Shape-aligned binary diff with per-row category. Backend for `dc.sh side-by-side` and `dc.sh gen-substs`. |
| `tools/diff_align.py` | Sequence-aligned diff with relocation masking and recipe detection. Backend for `dc.sh diff-align`. |
| `tools/diff_build.py` | Index-aligned diff (cascades on length difference). Backend for `dc.sh diff`. |
| `tools/diff_summary.py` | Categorized verdict + suggested next action. |
| `tools/diagnose_mismatch.py` | Map every differing byte to its function via linker map; flag drift-prone regfix rules. |
| `tools/make_check.py` | Incremental make + SHA1 check; tail output; surface silently no-op'd regfix rules. |
| `tools/caller_audit.py [--quiet] [--json]` | Scan callers' max arg arity; block if too narrow. |
| `tools/binary_diff.py` | Shape-aligned binary diff with cascade-immune category-per-row output. |

### Analysis / classification

| Tool | Purpose |
|---|---|
| `tools/classify_func.py` | Pre-dive classifier: size, blockers, BIOS/syscall/PsyQ-stdlib tags, recommendation. |
| `tools/classify_asmfix.py` | Classify `replace_with_asmfile` entries by recipe potential. |
| `tools/asm_analysis.py` | Per-function asm analysis (instruction counts, jal targets, structural hints). |
| `tools/scan_hand_coded.py` | Bulk hand-coded-asm scanner: 5 signals from `feedback_hand_coded_asm_recognition.md` (uniform multu pacing, empty-body branch, front-loaded loads, INT_MIN guard, tight register packing). |
| `tools/scan_handasm.py` | Older variant of the same. |
| `tools/audit_asm_cheats.py` | Surface asm-cheat patterns (large splice rules in regfix.txt, inline `__asm__("glabel ...")` function bodies in src/*.c). Used by `dc.sh start` to surface existing cheat debt. |
| `tools/extract_inline_asm.py [locate\|verify\|setup\|replace]` | Find / verify / set up / replace inline `__asm__()` blocks in C files. Backend for `dc.sh inline-*`. |
| `tools/extract_prologues.py` | Statistical extraction of common prologue shapes from asm/funcs/. |
| `tools/find_label_at.py` | Resolve a target byte address to the right `.L<N>` label in mine. |
| `tools/diagnose_hoist.py` | Compare callee-save constant hoisting between mine and target. |
| `tools/struct_copy_fix.py` | Detect and fix struct-copy patterns (lwl/lwr unaligned reads). |
| `tools/word_branches.py` | Auto-emit `.word`-encoded subst rules for branch-offset drift. |
| `tools/check_func.py` / `check_cluster.py` / `check_jtbl.py` | Per-function / per-cluster / per-jtbl verification helpers (from the codex_lab era). |
| `tools/census.py` | Count C definitions, INCLUDE_ASM stubs, asmfix bridges, etc. |
| `tools/build_queue.py` | Build the work queue CSV; filter matched / permanently blocked. |
| `tools/build_active.py` | Incremental rebuild for one function + bridge-aware verify-c + auto-repair on cascade drift. Backend for `dc.sh build-active`. |
| `tools/iter_log.py` | Per-function build trajectory log + plateau advice. |
| `tools/preflight.py` | One-screen pre-work brief. |
| `tools/preflight_cascade.py` | Cascade impact report before integrating. |
| `tools/memory_check.py` | Surface project-memory mentions of a function + sibling memory hits. |
| `tools/suggest_func.py` | Lean per-function rollup. |

### regfix / asmfix authoring

| Tool | Purpose |
|---|---|
| `tools/add_regfix.py` | Append a validated regfix rule. Pre-validates and auto-rolls back on failure. |
| `tools/dump_text_indices.py [--post-regfix]` | Numbered TEXT indices from build pipeline. |
| `tools/asmfix_slice.py` | Lift a target asm slice into asmfix.txt. |
| `tools/frame_shift.py` | Auto-generate frame-cascade regfix rules. |
| `tools/fix_label_drift.py` | Auto-fix `.L<N>` drift in regfix rules. |
| `tools/fix_asmfix_drift.py` | Auto-fix `.L<N>` drift in asmfix rename rules. |
| `tools/fix_gte_asm.py` | GTE-specific asm fixups. |
| `tools/regfix_drift_immune.py` | Audit regfix.txt for drift-fragile literal `.L<N>` rules; rewrite to `\.L\d+`. |
| `tools/regfix_preview.py` | Preview the effect of delete/insert/reorder/subst on slot numbering. |
| `tools/regfix_debug.py` | Phase-by-phase debug dump for regfix.py. |
| `tools/regfix_verify.py` | Backend for `dc.sh verify`. |
| `tools/regasm_explorer.py` | Interactive exploration of register-asm pin effects. |
| `tools/promote_regfix_lab.py` | Translate lab regfix configs onto live TU label/index space. |
| `tools/auto_drift_repair.py` | Build wrapper that detects cascade-drift symptoms and auto-runs fix tools. Backend for `dc.sh auto-repair` and `build-active`. |
| `tools/drift_surface_summary.py` | Report literal-`.LN` rule surface area across asmfix+regfix. Trend metric for `dc.sh start`. |
| `tools/dc_retire.py` | Backend for `dc.sh retire`: comments out asmfix bridge, sets active marker, prints C body location. |

### Naming / Kengo reference

The Kengo PS2 game uses the same Marionation engine and has ~2,500 debug-named functions extracted via `ccc`/`stdump`. These tools cross-reference Kengo names against BB2 functions.

| Tool | Purpose |
|---|---|
| `tools/kengo_match.py` | Match BB2 functions to Kengo PS2 equivalents by structure/size. |
| `tools/kengo_ref.py` | Resolve a BB2 function to its Kengo source-path / file / line. |
| `tools/kengo_globals.py` | Match data globals to Kengo equivalents. |
| `tools/apply_kengo_names.py` | Apply Kengo-derived names to a batch of BB2 functions. |
| `tools/audit_kengo_renames.py` | Audit Kengo-derived renames; flag weak matches. |

### m2c (assembly-to-pseudo-C decompiler)

`m2c` is a vendored decompiler that produces pseudo-C from MIPS assembly. Used as a starting point for new functions.

| Tool | Purpose |
|---|---|
| `tools/m2c/` | Vendored [m2c](https://github.com/matt-kempster/m2c) source tree. |
| `tools/m2c_post.py` | Post-process m2c output (fix common quirks, restore PsyQ types). |
| `tools/gen_m2c_context.py` | Generate `include/m2c_context.h` from PsyQ headers. |

### Queue generation

| Tool | Purpose |
|---|---|
| `tools/gen_work_queue.py` | Regenerate `WORK_QUEUE.md` from `tmp/batch_attempt.csv` + live scans for asmfix replacements and suspect inline-asm debt. |
| `tools/batch_attempt.py [--classify-only]` | Run `attempt_func.py` over a batch of functions; populate `tmp/batch_attempt.csv` for the queue generator. |
| `tools/batch_gte.py` | Batch-classify GTE functions. |
| `tools/psyq_stdlib_scan.py` | Detect functions that match known PsyQ stdlib idioms (memcpy, strcpy, etc). |

### Shared infrastructure

| Tool | Purpose |
|---|---|
| `tools/func_tooling.py` | Shared helpers: stage compilation, asm extraction, symbol lookup, map lookup, object comparison, WSL fallback. |
| `tools/active_func_scope.py` | Helpers for the active-marker hook. |
| `tools/hooks/active_func_guard.sh` | **Deprecated 2026-05-22** (unwired). Formerly blocked `git commit`/`git checkout`/queue pulls while a function was active+unmatched. Staying on-task is now the `grind_check` Stop hook's job; commit cheat-auditing moved to `commit_audit_guard.sh`. Script kept for reference / easy re-enable. |
| `tools/hooks/grind_check.sh` | Stop-event hook that rejects wrap-up language while a function is unmatched. |
| `tools/recipes/*.json` | Named recipe definitions (LICM unhoist, call-loop family, GTE 3x3 wrapper, etc). |

### Tests

| Tool | Purpose |
|---|---|
| `tools/test_auto_drift_rollback.py` | Test the auto-drift-repair rollback path. |
| `tools/test_maspsx_stripping_scanner.py` | Test detection of maspsx `.set noreorder` stripping. |
| `tools/test_regfix_reorder_idx0.py` | Test regfix's idx-0 reorder handling (function symbol attachment). |

## Conventions

A few rules that apply to all the tooling:

- **All decomp commands go through `dc.sh`.** Don't construct raw WSL pipelines; the wrapper handles working-directory, venv-activation, and config conventions.
- **Build files (`src/*.c`, `*.h`, `Makefile`, `regfix.txt`, `asmfix.txt`, `*.s`) must be written through WSL** to preserve LF line endings. CRLF silently corrupts GCC/maspsx output. Non-build files (`README.md`, `docs/*.md`, `tools/*.py`) tolerate CRLF.
- **Always use `2>&1` on WSL commands** to capture stderr. Many tools write important diagnostics there.
- **The systemd warning is cosmetic.** "Failed to start systemd user session" doesn't mean the command failed.
- **`dc.sh verify --clean` short-circuits.** SHA1 match → ~0.2s. Don't fear it; run it whenever you suspect a cached `.o` is masking a real regression.
- **Don't use `run_in_background` for compile/score/build commands.** They take < 15 seconds in normal use.
