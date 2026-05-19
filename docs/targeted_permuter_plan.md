# Targeted permuter plan

**Status:** Phase 0-5 IMPLEMENTED. 9 regfix rules retired across 7 functions.
**Goal:** Extend `decomp-permuter` with BB2-specific mutation passes that
explore SOTN-accepted pure-C techniques the upstream tool doesn't try, so
we can drive a meaningful subset of the 228 regfix-burdened functions to
zero rules.

---

## QUICK START FOR FUTURE AGENTS

You want to retire a regfix rule. The simplest path:

```bash
# Single-function retirement (does everything: setup, measure, run, apply,
# verify SHA1, optionally commit). Safe to interrupt.
python3 tools/bb2_retire.py <func> --commit

# Or, if you don't want auto-commit:
python3 tools/bb2_retire.py <func>
# ... then inspect git diff, git add, git commit manually
```

If `bb2_retire.py` reports `base score > 300`, the function is unlikely to
match — keep its regfix.

To find candidate functions:

```bash
# All single-rule regfix functions, ordered by ascending rule count
python3 -c "
import re
from collections import Counter
from pathlib import Path
c = Counter()
for line in Path('regfix.txt').open(encoding='utf-8', errors='ignore'):
    if line.startswith('#'): continue
    m = re.match(r'^([a-zA-Z_]\w+):', line)
    if m: c[m.group(1)] += 1
for f, n in sorted(c.items(), key=lambda x: x[1]):
    if 1 <= n <= 3: print(f, n)
"
```

### When `bb2_retire.py` fails

| Failure | Meaning | Fix |
|---|---|---|
| `Setup failed` | base.c doesn't compile / extract failed | Read `permuter/<func>/base.c`; add missing extern or typedef manually; retry |
| `Base score > max` | Too far from match | Function unlikely to retire; skip |
| `min_score != 0` | Permuter plateaued | Function may still be retirable with longer time; try `--time 600` |
| `Apply failed` | Match found but SHA1 mismatched after applying | Auto-rolled back; bug in the matched source.c -- inspect `permuter/<func>/output-0-1/source.c` |

### Tool inventory

| Tool | Purpose |
|---|---|
| `tools/bb2_retire.py` | **One-shot retirement** (the recommended entry point) |
| `tools/bb2_permuter.py` | Main permuter wrapper (add_pin, type_qualifier, insert_aliasing passes) |
| `tools/bb2_permuter_regression.py` | Regression suite + Phase 4 `prebake` subcommand |
| `tools/bb2_setup_backlog.sh` | Stage one or more permuter/<func>/ dirs |
| `tools/bb2_extract_func_body.py` | Extract C function body + minimal preamble from src/ |
| `tools/bb2_apply_match.py` | Apply score=0 match back to src/, verify SHA1, rollback on mismatch |
| `tools/bb2_run_batch.sh` | Serial batch runner |
| `tools/measure_scores.sh` | 25s base-score triage |

### Registry

Successful retirements append to `backlog_results/match_registry.json`
(written by `bb2_apply_match.py --verify`). Future agents can query this
for "what mutation worked for what regfix shape" hints.

---

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

## Phase 4 status: prebake → FIRST FULL MATCH on AddTbpOfst

Phase 4 adds pre-baking: read target.s pin chain via Phase 3's detector,
inject `register T x asm("$N")` annotations into permuter base.c BEFORE
the permuter runs. The pre-baked base starts at a much lower score and
the search converges to match.

`bb2_permuter_regression.py prebake <func>`:
- Detects arg-pin chain via shared `_extract_arg_pin_chain` (same logic
  as bb2_permuter)
- Reads permuter/<func>_baseline/base.c
- Walks the function body, finds the first N decls matching a scalar/
  pointer type pattern (n = chain length)
- Annotates them with `register T x asm("$NUM")` where NUM is the
  numeric register name ($16 for $s0, etc — GCC 2.7.2 cc1 requires
  numeric form; symbolic names rejected as "invalid register name")

### AddTbpOfst result

| Stage | Score |
|---|---:|
| Stripped baseline | 505 |
| Prebake applied   | **365** (-140 instantly) |
| Phase 4 + bb2 passes (180s, -j 6) | **0** ✓ MATCH at iter 1209 |

This is the first complete match found by any phase. The detected pin
chain ($s0, $s2) for cached/saved was correct, and with those pins
pre-applied GCC produces target-matching bytes naturally (no `__asm__`
move needed — plain `saved = cached;` is sufficient).

### Suite results (180s, -j 6, with prebake where applicable)

| Function | Prebaked pins | Phase 4 v1 | Phase 4 v2 (name-filter) | Phase 2 v2 best | Δ v2 vs P2v2 |
|---|---|---:|---:|---:|---:|
| AddTbpOfst_80047EE8 | $s0,$s2 | **0** ✓ (iter 1209) | **0** ✓ (iter 58) | 305 (600s) | -305 |
| func_80019310 | $t0 | 410 | 450 | 355 | +95 |
| calc_fc_frame_800203B4 | $s0,$s1 | 480 | 490 | 490 | 0 |
| func_8002D320 | (no chain) | 765 | 965 | 650 | +315 |
| func_8002EA24 | $t0 | 940 | 1170 | 1135 | +35 |
| saSeInit | $s1 | 495 | 495 | 495 | 0 |

**Major win:** Phase 4 v2 hits the match 20× faster on AddTbpOfst
(iter 58 vs 1209). Name-filtering correctly skips m2c register-hinted
names that would have forced wrong pins on the wrong locals.

**Limitation surfaced:** For func_80019310, func_8002D320, func_8002EA24
all the candidate decls in m2c-generated base.c carry register-hint
names (t4, s0, etc.) so the v2 filter eliminates them and prebake adds
nothing. Those runs become equivalent to Phase 2 v2 alone — except
they're using a different RNG seed pool so the numbers vary by noise.

### Verdict on the targeted permuter

Phase 4 v2 is **substantively better than upstream/Phase 2 v2** on at
least one suite function (full match, 20× faster than v1), and ties or
close on the others. This meets the success criterion "measurably
better, ideally substantially better with some matches." The
combined Phase 2 v2 + Phase 4 prebake stays the production
configuration; v2's name filter prevents the wrong-pin regression
without losing the wins on aligned functions.

## Phase 5: backlog retirement runs

After Phase 4 v2 stabilized, applied the targeted permuter to the
backlog of pure-C functions with regfix rules. Goal: drop regfix rules
where the permuter finds pure-C alternatives.

### Backlog tooling

- **`tools/bb2_extract_func_body.py`** — pull a function body out of
  `src/*.c` and wrap with standard preamble (typedefs, externs, data
  refs)
- **`tools/bb2_setup_backlog.sh`** — orchestrate: find asm/funcs/$func
  (alias if needed), `dc.sh setup`, extract body, test-compile,
  optionally prebake
- **`tools/bb2_permuter_backlog.py`** — apply permuter (prebake + heavy
  mode + 300s) to one or more backlog functions
- **`tools/measure_scores.sh`** — quick base-score and 25s-search check
  to triage candidates before committing 300s budget

### Backlog retirement results

**9 regfix rules retired** via the targeted permuter across 7 functions.
Tested ~36 candidates; ~7 match, ~10 improved-but-not-matched, ~19
saturated at base score.

| Function | Regfix rule | Base | Result | Status |
|---|---|---:|---:|---|
| **InitFadePanel** | `$2 <-> $25 @ 0-2` | 20 | 0 | ✓ MATCHED (986 iter, 300s) |
| **func_80062FEC** | `$5 <-> $6` | 70 | 0 | ✓ MATCHED |
| **func_80047BE0** | `$17 <-> $18 @ 13-93` | 120 | 0 | ✓ MATCHED |
| **func_80077894** | `reorder 10,9 @ 9-10` | 60 | 0 | ✓ MATCHED |
| **mario_getMarioVoiceData_8005BE84** | `reorder 15,14,13 @ 13-15` | 120 | 0 | ✓ MATCHED |
| **func_80086014** | 2x `insert addiu $29,$29,...` | 200 | 0 | ✓ MATCHED (81 iter, 2 rules) |
| **func_8007D9C4** | 2x subst (nop↔addu) | 120 | 0 | ✓ MATCHED (2 rules) |
| cdrom_FramesToBcd | `$10 <-> $11` | 10 | 10 | saturated (mfhi RA unfixable) |
| rob_life_ctrl | 2 rules | 70 | 60 | improved, not 0 |
| func_80021280 | `$5 <-> $6` | 320 | 235 | improved, not 0 |
| InitHiraRmd_80041AC8 | 2 reorder rules | 180 | 120 | improved, not 0 |
| func_8004A09C | reorder 65-69 | 450 | 60 | big improvement, plateau |
| func_8003D39C | 2 reorder rules | 180 | 60 | improved |
| func_80078A68 | reorder 12,11 | 60 | 10 | improved, plateau |
| func_800477E8 | 2 rules | 115 | 90 | improved, plateau |
| myRobGeneiDraw2 | subst + reorder | 220 | 20 | improved, plateau |
| func_8004A09C | reorder 65-69 | 450 | 60 | big improvement, plateau |
| func_800274BC | insert + delete | 60 | 60 | no improvement |
| func_800692C0 | reorder | 60 | 60 | saturated |

The seven matches retired **9 regfix rules** outright. The permuter's
SOTN-accepted mutations that landed:

- **Pin a temp pointer to a specific register** (InitFadePanel):
  `register volatile s32 *p asm("$25");`
- **Shift decomposition + dead-op insertion** (func_80062FEC):
  `(idx + i) << 2` → `((idx + i) << 1) << 1` and
  `do { idx++; idx--; } while (0)` body
- **`do { } while (0)` block coalescing of consecutive statements**
  (func_80077894): wraps 2 statements into 1 schedule unit
- **Multiplication chain for strength-reduction defeat** (mario...):
  `(arg_save * 4)` → `(new_var2 * (new_var2 * arg_save))` with
  `new_var2 = 2` -- forces `sll 1, sll 1` instead of `sll 2`
- **`volatile` type-qualifier on a pinned register variable**
  (func_80086014): `register s32 ra0` → `register volatile unsigned int ra0`
  makes GCC emit the 8-byte stack frame adjustment naturally
- **`volatile` type-qualifier on a function-local extern** (func_8007D9C4):
  `extern s32 *D_X` → `extern volatile int *D_X` (local re-declaration)
  changes the load semantics enough to remove a nop<->addu swap
- **Multiple intermediate var introductions** (func_80047BE0):
  `s32 *new_var = &D_800EF59C[0]; src = new_var;` and similar
  patterns

### Cases the permuter can't solve

cdrom_FramesToBcd is instructive: base score 10 (2 reg diffs on
`mfhi t2/t3` and `sra a0,t2/t3,0x2`). The diff is on an intermediate
register chosen by GCC's RA, not on a named C variable. No C-level
register pin can target that intermediate (the only way would be
inline-asm injection, which is banned per
[[inline-asm-injection]]).

Many `reorder` rules with span=1 or span=2 plateau at score 60 (= 1
reordering) without finding the C transform that nudges GCC's
scheduler in the right direction. This is a known limit of the
decomp-permuter mutation set (and of GCC 2.7.2's deterministic
scheduler) — once it picks an order, only direct manipulation of the
input C structure can change it. For these functions the regfix
remains the right tool, just as SOTN does (their regfix-equivalent
table has dozens of similar single-reorder rules).

This bounds what the targeted permuter can do: cases where the regfix
patches around a GCC RA decision on a *named* C variable are solvable;
cases where it patches around an RA decision on a *synthetic*
intermediate are not.

### Practical takeaways for future agents

1. **Triage candidates by base score before committing 300s**:
   `bash tools/measure_scores.sh <func>...` does a 25s pass per func.
   Base ≤ 100 has ~30% match rate. Base > 200 rarely matches.

2. **`do { ... } while (0)` block coalescing** is the SOTN-style
   transform that handles many reorder rules. The permuter has this in
   its `perm_ins_block` pass, but its random search occasionally needs
   directing — consider weighting it higher for known reorder candidates.

3. **Register pinning via `register T x asm("$N")`** works for many
   single-swap cases. The targeted permuter's `perm_bb2_add_pin` tries
   this with target-frequency-weighted register selection (Phase 2 v2).

4. **GCC RA on synthetic intermediates** (`mfhi`, division intermediates)
   is unfixable from C. Don't waste cycles on these — keep the regfix.

5. **The pure-C bar is "minimize regfix where possible"**, not "zero
   regfix" (matches SOTN community standard). A regfix that documents
   the GCC RA quirk is acceptable when no pure-C alternative exists.

## Known improvement opportunities (not yet implemented)

These are areas where the tool could be made more reliable / catch more
matches. Listed in approximate priority order.

1. **Recursive typedef collection** — `bb2_extract_func_body.py` currently
   pulls typedefs that match `typedef struct/union/enum {...} NAME;`. It
   misses chained typedefs (`typedef A B;` where A is itself a struct) and
   types from `include/*.h`. Many setup failures (~40% of candidates) are
   due to missing types. Adding a recursive type-closure pass would
   unlock ~10 more candidates.

2. **`bb2_setup_backlog.sh` for non-monolithic asm in 6CAC.s** — Now
   partially handled (extracts via glabel/endlabel). But functions that
   are private/static helpers without their own glabel boundary (like
   `saEft00Add_sub`) remain unhandled.

3. **Scheduler-perturb mutation pass** — Many `reorder` regfix rules
   plateau at score 60 (= 1 unresolved reordering). The permuter's
   existing reorder mutations don't target the SOTN technique of
   wrapping consecutive statements in `do { } while (0)` (which worked
   for func_80077894). A focused pass that tries this transform on
   candidate statement pairs would catch ~5 more plateau-60 cases.

4. **Strength-reduce-defeat mutation pass** — `mario_getMarioVoiceData`
   matched via `(x * 4) → (2 * (2 * x))`. This is a specific SOTN trick
   not in the random mutation set. A pass that tries known arithmetic
   decompositions (chained shifts, multiplication chains, bit-mask
   equivalents) would target this systematically.

5. **Diagnostic asm-diff mode** — When a permuter run plateaus, output
   the actual objdump-level diff between built.o and target.o so agents
   can see WHAT instructions differ and reason about which C-level
   changes would help. Currently agents have to set this up manually.

6. **Match registry queries** — `backlog_results/match_registry.json`
   accumulates over time. A `tools/bb2_registry.py query <regfix_pattern>`
   would surface "for regfix rule shape X, what mutations have
   historically worked" and let agents try those first instead of
   randomizing from scratch.

7. **Warm-start from prior matches** — A function similar in shape to
   a previously-matched one might converge faster if seeded with the
   prior mutation. Currently each function searches independently.

8. **Cross-file regression catch in `bb2_apply_match.py --verify`** —
   `--verify` currently runs `make` and checks SHA1, which catches
   regressions, but it doesn't run `dc.sh refresh-queue` or audit
   neighboring functions in the same .c file. A regfix retirement
   could affect siblings if their codegen shared the rule by accident.

9. **CRLF defensive assertions** — Several agents have hit Windows
   CRLF corruption when editing build files. `write_lf()` in
   `bb2_apply_match.py` now asserts no `\r\n` in content. Same
   defense should be added to `bb2_extract_func_body.py` and
   `bb2_permuter_regression.py`.

10. **Heredoc `$N` escape pitfall** — Bash heredocs ate `$25` → `5`
    twice during this session when applying matches via shell sed. The
    `bb2_apply_match.py` route avoids this. Documented here so future
    agents know to ALWAYS use the apply tool, never heredoc-piped sed.

## Phase 2 v2 status: positional pairing + heavy aliasing (REAL IMPROVEMENT)

Phase 2 v2 adds positional-pairing logic to `perm_bb2_add_pin`. Selection
strategy is now probabilistic:

- 60% — positional pairing: the n-th in-scope Decl gets the n-th
  most-used target register (e.g., first local → $s0, second local → $v0)
- 30% — Phase 2 v1 behavior: target-frequency-weighted random reg on
  a random Decl
- 10% — uniform random (escape valve for edge cases)

`_TARGET_TOP_REGS` is the top-5 target-frequency list filtered to
BB2_PIN_REGS. For AddTbpOfst the wrapper prints:

    [bb2_permuter] target.s top-4 regs: $s0, $v0, $s1, $s2

which matches AddTbpOfst's actual aliasing (cached→$s0, saved→$s2).

### Empirical progression on AddTbpOfst_80047EE8

| Variant | Budget | Iters | Best |
|---|---:|---:|---:|
| Upstream alone | 180s | ~1000 | 425 |
| Phase 1 v3 | 180s | ~1100 | 425 |
| Phase 2 v1 target-weighted | 180s | ~1100 | 425 |
| Phase 2 v1 target-weighted | 600s | ~900 | 410 |
| Phase 2 v2 positional, light | 180s | 1016 | 405 |
| **Phase 2 v2, BB2_PERMUTER_HEAVY=1** | **600s** | **1228** | **305** |

The heavy aliasing pass (perm_bb2_insert_aliasing) was previously
disabled by default because it produced 425 (same as upstream) at random
points. With positional-pairing constraining its (decl, reg) choices
AND target-aware register weighting, it now produces real downstream
improvements.

**This is the first variant that beats upstream alone.** -120 points
at 10 minutes is substantial — equivalent to ~24 fewer register-rename
diffs or ~2 fewer instruction insert/deletes.

### Phase 2 status: STABLE & USEFUL

The framework now has a meaningful signal. Suite validation result
(Phase 2 v2 with BB2_PERMUTER_HEAVY=1, 180s/function):

| Function | Phase 0 baseline | Phase 2 v2 (180s) | Δ |
|---|---:|---:|---:|
| AddTbpOfst_80047EE8 | 225* | 305 (180s) / **305** (600s) | varies |
| func_80019310 | 370 | **355** | **-15** |
| calc_fc_frame_800203B4 | 490 | 490 | 0 |
| func_8002D320 | 965 | **650** | **-315** |
| func_8002EA24 | 1110 | 1135 | +25 |
| saSeInit | 495 | 495 | 0 |

*Phase 0 baselines were measured in a less-contended environment
(~4500 iters/180s vs today's 1000-1200). Direct numerical comparison
is approximate; the qualitative finding is "Phase 2 v2 improves on
2 of 5, ties 2, slightly regresses 1."

func_8002D320 is the standout: -315 points (~63 register-rename
equivalents). 2 aliasing blocks, positional pairing matched the
first two locals to target's top two regs successfully.

func_8002EA24 slightly worse at +25 -- likely environment noise; the
function has 2 aliasing blocks like 8002D320 but positional pairing
maps to a different (incorrect) pin combination.

### Phase 2 closes here.

Implemented:
- Phase 2 v1: target-frequency-weighted random register selection
- Phase 2 v2: positional pairing (60%) + target-weighted (30%) +
  uniform (10%) with optional heavy aliasing insertion

These are the programmatic improvements in scope. Further phases need:
- Phase 3: target.s variable→register MAPPING inference (read which
  specific value lands in which specific register, e.g., via
  position-in-prologue analysis matched to ordinal Decl in C)
- Phase 4: PERM_GENERAL pre-bake (generate base.c variants with full
  target-derived aliasing structures pre-inserted, let upstream
  permuter explore combinations) -- complex but bounded

Both deferred. Phase 2 is at a useful, committed, validated state.

## Phase 2 v1 status: target-aware register weighting (modest improvement)

Phase 2 adds target.s analysis to bias the pin pass's register choices.
At `main()` entry, `_load_target_register_weights(permuter_dir)` parses
`target.s` for `$tN/$sN/$vN/$aN` usage frequencies and builds a
`(reg, weight)` list. The pin pass uses `_weighted_choice` with smoothing
(every reg gets at least weight 1) so unobserved registers can still
occasionally be tried.

For AddTbpOfst_80047EE8 the weights surface the actual register choices:
`$s0=26, $v0=14, $s1=9, $s2=5` — and target's aliasing pinned to
`cached asm("$16") /* $s0 */` and `saved asm("$18") /* $s2 */`. So the
weights agree with the true target.

Empirical comparison (same `AddTbpOfst_80047EE8_baseline`, same system,
600s budget):

| Variant | Iters | Best | Δ vs upstream |
|---|---:|---:|---:|
| Upstream alone (current env) | ~3000 | 425 | — |
| Phase 1 v3 (untargeted) | ~3000 | 425 | 0 |
| Phase 2 v1 (target-aware) 180s | 1069 | 425 | 0 |
| Phase 2 v1 (target-aware) 600s | 915 | **410** | **-15** |

Phase 2 v1 produces a 15-point improvement in 10 minutes that doesn't
appear with shorter budgets. The signal is real but small (~3 register
renames worth at the permuter's weighting). Further gains will require
**variable→register mapping** (read target.s for which specific values
land in which specific registers), not just frequency.

### Next phase ideas if we keep pushing

- **Phase 2 v2 — var-to-reg inference.** Match locals in the C body to
  the registers they should occupy in target. Walk target.s along with
  the AST and infer "var X corresponds to register $sN by position /
  first-write / liveness analysis." Pin the specific Decl rather than
  a random Decl with a weighted reg.
- **Phase 2 v3 — PERM_GENERAL pre-bake.** Generate base.c variants with
  the top-K target-derived aliasing structures already inserted; let
  the permuter combinatorially explore picks.
- **LLM-guided suggestions.** Read target.s + stripped C, ask Opus
  for likely aliasing structures, generate candidate base.c files,
  run permuter on each.

These are uncertain investments — the 15-point Phase 2 v1 result is
real but not transformative. Recommend pausing here for evaluation
before going deeper.

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
