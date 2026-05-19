# Targeted permuter plan

**Status:** Planning. Not yet implemented.
**Goal:** Extend `decomp-permuter` with BB2-specific mutation passes that
explore SOTN-accepted pure-C techniques the upstream tool doesn't try, so
we can drive a meaningful subset of the 228 regfix-burdened functions to
zero rules.

## Why this, why now

Investigation on `coli_HitPauseKatana_2` (committed and reverted across
`f7c7379` / `09fb361` / `d9e5b4f`) established that:

1. The current `decomp-permuter` random pass plateaus quickly (~150
   weighted score for coli) because its mutation set doesn't include
   the BB2-relevant techniques: register-pin combinations, inline-move
   aliasing block insertion, or intermediate-variable injection at
   structurally-meaningful points.
2. The original PsyQ compiler also fails to reach target bytes from
   our pure-C body — proving for *that one function* the source is
   unrecoverable. But that's the worst case; cheaper regfix functions
   in the queue have target sources that ARE structurally reachable
   from our C, just not from any C the random permuter happens to
   produce. (Note: cc1psx was historically used as a calibration oracle
   during this investigation; the project has since deprecated cc1psx
   to avoid agent confusion. The targeted permuter scores against
   mips-gcc-2.7.2 only.)
3. The community standard (SOTN/Vagrant/ESA/CTR) explicitly accepts
   the patterns we'd add as mutation passes (see
   `memory/rules/community-standard.md`):
     - `register T x asm("$N");` pins
     - Single-instruction `__asm__ volatile("op %0,%1,$zero" : "=r"(d) : "r"(s));`
       with placeholders + GCC constraints (the libgte.h shape)

A tool that automates exploring these mutations against our 228
regfix-burdened functions could realistically retire ~30-40 of them
cleanly, without re-doing manual coercion per function.

## Architecture choice: extend, don't rebuild

`decomp-permuter` is mature: AST manipulation via `pycparser`, well-defined
randomization passes in `src/randomizer.py`, a scoring pipeline that
diffs the compiled object against `target.o`. Rebuilding that infrastructure
from scratch would take weeks. We extend.

The right extension points:
- **Add new randomizer passes** in `src/randomizer.py` for the BB2 patterns
- **Optionally swap the scoring function** to be "bridge-aware" (penalize
  diffs whose remaining-bridge regfix would require non-uniform-rename rules)
- **Optionally add a per-pass weight knob** so we can tune which patterns
  the random search exploits

No fork of upstream needed initially — patches can land on
`tools/decomp-permuter/` directly. If the patches prove valuable, upstream
them.

## Phase plan

### Phase 1 — auto-aliasing pass (MVP, 2-3 days)

**The single highest-leverage feature.** Insert `__asm__ volatile("op %0,%1,$zero" : "=r"(dst) : "r"(src));` blocks at randomly-chosen C points with randomly-chosen register pin combinations.

Implementation:

1. New randomizer pass: `try_insert_aliasing_block`
2. Walk the AST for a candidate function-body block (between two statements)
3. Pick two local-scope `s32`/`u32` variables already in scope at that
   point — call them `dst` and `src`
4. Emit AST nodes for:
   ```c
   register T dst_pinned asm("$RD");
   register T src_pinned asm("$RS");
   __asm__ volatile("addu %0, %1, $zero" : "=r"(dst_pinned) : "r"(src_pinned));
   ```
   where `$RD` / `$RS` are chosen from {`t0`-`t9`, `s0`-`s7`, `v0`, `v1`}
5. Replace subsequent reads of `dst` with `dst_pinned` (or wrap the alias
   so original variable name still works)

Test on a function we KNOW responds to manual aliasing (one of the
existing matched functions that uses `inline-move-aliasing`, e.g.
`calc_fc_frame_800203B4` per `memory/reference/inline-move-aliasing.md`).
Strip the manual aliasing block, see if the pass reconstructs it. If
yes, validation done.

### Phase 2 — register-pin saturation pass (1-2 days)

Add a randomizer pass that tries `register T x asm("$N")` annotations on
local declarations.

Implementation:
1. New pass: `try_add_register_pin`
2. Walk AST for `Decl` nodes with `s32`/`u32`/pointer type
3. Pick one randomly, add `register T x asm("$N")` annotation
4. `$N` rotates through `t0`-`t9`, `s0`-`s7`, `a0`-`a3`, `v0`-`v1`

Stack with phase 1: the permuter randomly inserts aliasing blocks AND
adds register pins. Together they should cover the existing
`memory/reference/inline-move-aliasing.md` recipes' search space.

### Phase 3 — intermediate variable injection (2-3 days)

Permuter's existing passes don't try inserting `s32 alias = expr; ... alias`
chains, which is the pattern we discovered manually on coli
(`new_var2 = mask` alias trick that closed the mask-routing diffs).

Implementation:
1. New pass: `try_inject_alias_var`
2. Walk AST for expressions used in multiple places
3. Pick one randomly, replace first use with `s32 alias = expr;`,
   subsequent uses with `alias`
4. Variant: same but place the alias declaration in an unreachable
   position (before a `case` label inside a switch) — that's a
   surprisingly load-bearing GCC quirk we hit on coli

### Phase 4 — bridge-aware scoring (3-5 days, optional)

Modify the scoring function so it doesn't just count byte-diff but
estimates "how many auditor-acceptable regfix rules would close this gap."

A variant is in the "auditor-clean" position if:
- All remaining diffs are register renames consistent with a `swap $X <-> $Y`
- OR all remaining diffs are uniform substs that match a uniform pattern
- AND no diffs require `delete`/`insert`/`splice`/structural reorder

Variants in the "auditor-clean" position get score reduction proportional
to how few rules would close them, making them strictly preferred over
equally-byte-diff variants that need cheating rules.

Risk: this is the most complex phase and could be skipped if phases 1-3
prove enough.

## Validation strategy

Before integrating any phase into the project's `dc.sh` pipeline:

1. **Regression test set:** Pick 5-10 functions that are *already*
   matched in pure C with single-insn aliasing (e.g., the
   `calc_fc_frame_800203B4`-family, the text1b.c GTE wrappers). For
   each, strip the aliasing back to a "before" state, run the targeted
   permuter, confirm it reconstructs the working C.
2. **Plateau probe:** Pick 5-10 functions from the regfix-burdened
   list ordered by smallest current rule count (1-3 rules). Run the
   targeted permuter for 10 minutes each. Score the rule-count
   reduction.
3. **Negative test:** Run the targeted permuter on `coli_HitPauseKatana_2`.
   We expect plateau at score 150 (historically confirmed lower bound). If
   the targeted permuter beats that, something's wrong in the scoring;
   if it doesn't, the tool is correctly identifying the unrecoverable
   case.

The regression suite catches the worst failure mode (tool produces
non-compiling C). The plateau probe measures real-world utility. The
negative test confirms the tool doesn't oversell.

## Risk & ROI estimate

| Risk | Likelihood | Mitigation |
|---|---|---|
| Phase 1 produces compile-error C (operand binding wrong) | Med | Regression suite |
| AST manipulation breaks decomp-permuter's existing passes | Low | Keep changes confined to new pass files |
| Tool finds variants but they regress sibling functions | Low (single-function scope) | Existing `verify --all` catches this |
| Tool doesn't help meaningfully (false-negative) | Med | Negative test on coli is honest |

**Estimated investment:** 1-2 weeks for phases 1-3. Phase 4 optional.

**Estimated payoff:**
- Direct: 30-40 of 228 regfix-burdened functions cleanly retired
- Indirect: rules-pattern library to inform manual work on the rest
- Knock-on: pattern recognition might surface "yes target sources of these N functions all share shape X" — useful for the unrecoverable-source group

**Stop condition:** If after Phase 1 the regression suite isn't reliably
reconstructing manual aliasing on the easy cases, abandon (the
representation isn't tractable). If Phase 1 works but Phase 2-3 produce
no plateau-probe improvements, ship Phase 1 alone and move on.

## Out-of-scope

- Patching GCC to match the original PsyQ compiler — separately
  investigated, ruled out (also cc1psx is deprecated per project policy)
- Building a "source archaeology" tool to infer target's original C —
  research problem, not engineering
- Replacing decomp-permuter's core scoring — extension only
- Anything specific to `coli_HitPauseKatana_2` — that function is the
  unrecoverable-source case and a new tool won't help it. Re-bridged and
  left in queue for future agents.

## Answered design questions (2026-05-18)

All locked in via the discussion in this design round:

| Question | Decision |
|---|---|
| Pin set scope | `$t0-$t9` + `$v0-$v1` + `$s0-$s7` (20 regs). All non-parameter caller- and callee-saved. Frame-size shift handled by GCC's standard prologue emission. SOTN-compliant. |
| Output behavior | Save candidates to `permuter/<func>/output-*/` only (current upstream behavior). No auto-integration, no auto-audit. Agent reviews + integrates manually. |
| Time budget | Adaptive — same shape as `dc.sh permute-adaptive`. Easy functions get ~90s, harder ones up to 30 min. |
| Scoring target | mips-gcc only (current build pipeline). cc1psx is deprecated per project policy and not used. |
| Regression suite | Hand-pick 5-7 known-aliasing functions for fast dev cycle, then auto-detect from `INLINE_MOVE_ALIASING:` comment headers for full pre-merge run. |
| Audit integration | Phase 1: no auto-audit. Existing `/cheat-audit` skill remains the audit path; agent invokes after reviewing permuter candidate. |
| Phase 4 commitment | Plan it (kept in design), build Phase 1-3 first, reassess from results. |
| Packaging | New tool `tools/bb2_permuter.py` wrapping `decomp-permuter` and injecting passes at runtime. Decoupled, no fork of upstream. |
| Pass weight | Default 1:1 with existing randomizer passes. Configurable via `--pass-weights` flag. |

## Phase 0 baseline results (commit pending)

Upstream `decomp-permuter` with `-j 6`, 180-second budget per function:

| Function | Blocks | Iters | Base score | Best score | Improvement | Status |
|---|---:|---:|---:|---:|---:|---|
| AddTbpOfst_80047EE8 | 1 | 4516 | 505 | 225 | -280 | regression target |
| func_80019310 | 1 | 4549 | 480 | 370 | -110 | regression target |
| calc_fc_frame_800203B4 | 3 | 4491 | 940 | 490 | -450 | regression target |
| func_8002D320 | 2 | 4501 | 1465 | 965 | -500 | regression target |
| func_8002EA24 | 2 | 4580 | 1670 | 1110 | -560 | regression target |
| saSeInit (stretch) | 4 | 4462 | 590 | 495 | -95 | regression target |
| DispSchoolBG (stretch) | 7 | — | — | — | — | base.c arity mismatch — Phase 1 prep TODO |

**Key finding:** upstream random permuter improves scores by 20-50% on most
functions but never reaches 0 in 4500 iterations. Phase 1's auto-aliasing
pass should close the remaining gap on the core 5 (and ideally saSeInit).
DispSchoolBG fails to compile in the baseline run because m2c's inferred
extern signatures don't match what the stripped C calls (specifically
`func_8007F87C` arity); fix in Phase 1 prep by injecting actual src/
externs into base.c instead of m2c's inferences.

## Phase 1 status: STABLE — framework neutral, pass design needs more work

After v3 (dual-pass design + diagnostic round):

### Diagnosis: the "rate regression" was environment variance, not a wrapper bug

Re-ran controls under current conditions:
- Upstream alone:     1040 iters / 180s → best 425
- bb2 weight=0:        842 iters / 180s → best 425
- bb2 add_pin only:   1138 iters / 180s → best 425
- bb2 both passes:    1170 iters / 180s → best 425

The earlier "upstream gets 225 in 180s" (Phase 0 baseline) was from a
less-contended environment (4516 iters in the same 180s budget — ~4×
more iterations than today). System load / filesystem contention is
the dominant variable, not pass design. **The wrapper is neutral**
relative to bare upstream: same iter rate within noise, same score
floor.

### Current pass design (v3)

Two passes registered in `tools/bb2_permuter.py`:

1. **`perm_bb2_add_pin`** (default weight 10.0):
   - Annotate an existing local Decl with `register ... asm("$N")`
   - Statement-count preserving; minimal mutation
   - Search space: n_decls × 20 pin regs (typically <= 200)
   - This is the lightweight default

2. **`perm_bb2_insert_aliasing`** (default weight 0.0, opt-in via
   `BB2_PERMUTER_HEAVY=1`):
   - Insert `register T pin asm("$N"); __asm__ volatile("move %0, %1" :
     "=r"(pin) : "r"(src));`
   - Redirect one downstream read of src to pin
   - Statement-count +2 (matches what stripped functions are missing)
   - Disabled by default because per-iter cost is higher; opt-in for
     functions where add_pin plateaus

### What's still needed for the targeted permuter to add real value

Pass design has to find improvements upstream can't. Today neither pass
beats upstream. Ideas for Phase 2:

- **Type-aware variable selection** in `perm_bb2_add_pin`: instead of
  random Decl + random reg, look at target.s to see which registers
  hold which values, and bias the pin choice toward registers the
  target actually uses for that variable.
- **Insertion-point heuristic** in `perm_bb2_insert_aliasing`: instead
  of random points, prefer insertion immediately after a Decl whose
  value is read more than once.
- **Combine with PERM_GENERAL macros** in base.c at suspected
  aliasing locations, biasing the search.

### Stop condition met for Phase 1

The framework is committed and stable. The current passes are neutral
relative to upstream. Further investment requires smarter pass design
(Phase 2) or a different architecture (e.g., pre-computed aliasing
candidate library + base.c rewriting before permuter). Neither is
blocking — main is clean, regression suite works, wrapper proven
extensible.

## Phase 0 status: COMPLETE (commit c40abcb)

`tools/bb2_permuter.py` (Phase 1 MVP) successfully wraps decomp-permuter
and injects the `perm_bb2_insert_aliasing` pass at runtime. Mechanically
the integration is sound:

- Monkey-patches `randomizer.RANDOMIZATION_PASSES.append(...)` before
  `src.main.main()` runs
- Injects pass weight into the `[base]` section of the weights TOML via
  `toml.load` monkey-patch (the load happens lazily on first iteration)
- Pass generates SOTN-accepted AST:
  `register T pin asm("$N"); __asm__ volatile("move %0, %1" : "=r"(pin) : "r"(src))`
  and redirects one downstream read of `src` to `pin` so GCC can't DCE
  the pinned local.

**Empirical result on AddTbpOfst_80047EE8 (regression target):**

| Run | Iters / 180s | Best score |
|---|---:|---:|
| Upstream baseline | 4516 | **225** |
| bb2_permuter weight=1.0 | 1872 | 505 (no improvement) |
| bb2_permuter weight=0.1 | ~220 | 505 (no improvement) |

The pass currently HURTS the search rather than helping. Two issues
identified:

1. **Iteration rate drops sharply** (4516 → 1872 → 220). Even at low
   weight, the bb2 wrapper produces fewer iterations than upstream
   alone. Cause not fully diagnosed — may be parallelism interaction
   with the monkey-patch.
2. **Insertion is too random** to land usefully. Adding 2 statements
   (decl + asm) at a random point with random source-var + random
   pin-reg + random downstream-read replacement has a vanishingly small
   chance of producing target-matching codegen.

Phase 1 needs a more principled design before it adds value:

- Profile the iteration-rate regression — is it our pass, our patch,
  or upstream worker overhead?
- Constrain the insertion: use scope/type analysis to pick pin-reg /
  source-var combinations that have a realistic chance of helping.
- Consider a less-aggressive mutation: instead of inserting NEW
  statements, ANNOTATE an existing declaration with a register pin
  asmlabel (no statement-count change, smaller search space).

Suspending Phase 1 here. Framework is committed and tested; the actual
pass implementation needs more thought before continuing.

## Phase 0 status: COMPLETE (commit c40abcb)

Delivered:

- `tools/bb2_permuter_regression.py` — full subcommand surface:
  `show`, `strip`, `strip-all`, `diff`, `load-bearing`, `load-bearing-all`,
  `setup-baseline`, `baseline-record`, `report`
- All 7 suite functions stripped successfully (1+1+3+2+2+4+7 = 20 aliasing
  blocks). Strip mechanics correctly:
  - Identify aliasing blocks (`__asm__ volatile("move %0,%1"...)`)
  - Replace asm with semantic-equivalent C (`dst = src;`)
  - Strip `register asm("$N")` annotations from dst/src locals
  - Drop `INLINE_MOVE_ALIASING:` comment headers
- All 7 functions verified LOAD_BEARING_OK — stripping breaks the SHA1
  match, confirming aliasing is genuinely load-bearing on all suite
  members.
- CRLF-safe I/O via `read_text_lf`/`write_text_lf` helpers (Python's
  default text-mode would corrupt src/ files on Windows; we use byte-mode
  to preserve LF line endings throughout the round-trip).
- Restore-time integrity check guards against silent src/ corruption.
- Initial baseline data point: AddTbpOfst_80047EE8 → upstream decomp-
  permuter plateaus at score 505 after 371 random iterations. Cannot
  reconstruct the aliasing on its own. This is the regression to beat.

What's NOT done (intentionally deferred to Phase 1's validation loop):

- baseline-record runs on the other 6 suite functions. They cost
  ~3 min each = ~18 min total. Will run as part of Phase 1 evaluation
  (before/after comparison). Running them now adds no information — we
  already know the regression set is real per AddTbpOfst.

## Phase 0 — regression suite (FIRST, before any pass code)

Per the answered execution scope: build the regression infrastructure
before any randomizer code. De-risks "tool can't reconstruct anything"
by proving validation works first.

### Hand-picked core set (5 functions)

Pulled from existing `src/` files that have manual `INLINE_MOVE_ALIASING:`
documented aliasing blocks, picked for diversity of:
- File (text1b.c vs code6cac.c vs code6cac_b.c)
- Block count (single block vs multi-block)
- Memory documentation depth (which of these have explicit retrospective notes)

| Function | File | Aliasing blocks | Notes |
|---|---|---|---|
| `AddTbpOfst_80047EE8` | text1b.c | 1 | Smallest single-block case. Good first probe. |
| `func_80019310` | code6cac.c | 1 | Single block, different file. Cross-file generality test. |
| `calc_fc_frame_800203B4` | code6cac.c | 3 | Multi-aliasing case. Per `memory/reference/inline-move-aliasing.md`, this is the canonical "closed the last 21→0 diff jump via aliasing" reference match (commit `a502eb4`). |
| `func_8002D320` | code6cac_b.c | 2 | Documented in `memory/reference/regfix-subst-multi-splice.md`. Has the subst_multi pattern combined with aliasing. |
| `func_8002EA24` | code6cac_b.c | 2 | Documented in `memory/reference/register-asm-pins.md` — the canonical pin-fail-then-aliasing-fix case. |

### Stretch set (advanced, for pre-merge run)

| Function | File | Aliasing blocks | Notes |
|---|---|---|---|
| `saSeInit` | code6cac_b.c | 4 | Heavy multi-aliasing. Stress test. |
| `DispSchoolBG` | code6cac_b.c | 6 | Heaviest aliasing in the codebase. If the tool handles this, it handles everything. |

### Phase 0 deliverables

1. `tools/bb2_permuter_regression.py` — script that:
   - Takes a function name as input
   - Locates its aliasing blocks in src/
   - Strips them (saves stripped C as `regression/<func>/stripped.c`)
   - Saves the matching original C as `regression/<func>/expected.c`
   - Verifies stripped.c doesn't match (sanity check)
   - Runs decomp-permuter on stripped.c
   - Reports whether the permuter reconstructed something equivalent

2. `regression/` directory (gitignored except for golden inputs) with
   subdir per regression function.

3. Validation that decomp-permuter's CURRENT random search reconstructs
   ZERO of these (or some baseline). If it reconstructs some already,
   we re-pick.

Once Phase 0 demonstrates that the validation infrastructure works
AND that current decomp-permuter doesn't already solve these (so we
know there's room for our extension), proceed to Phase 1.
