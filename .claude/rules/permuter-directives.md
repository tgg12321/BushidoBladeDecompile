---
name: permuter-directives
paths: [".claude/rules/permuter-directives.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "Translate a rule-recommended maneuver (register-asm-pins, shared-end-label, loop-rotation-two-shift, etc.) into PERM_* macros so decomp-permuter can exhaustively test the variations. Output is PROPOSALS — every closing form must clear the cheat-by-any-spelling vetting in [[no-new-park-categories]] AND the layer-2 [[review-discipline-before-commit]] gate before commit."
metadata:
  type: reference
---

# Permuter manual-mutation directives — the bridge from "rule says try X" to "permuter validated X"

## Why this exists

Workers currently invoke `tools/decomp-permuter/` rarely and as a black box.
When [[codegen-technique-index]] points at a rule (e.g. "try the
[[shared-end-label]] restructure"), the worker rewrites C by hand and
re-runs `sandbox`. That loop loses two affordances permuter already
ships:

1. **Test ALL the combinations of a directed maneuver in one run** — the
   `PERM_*` macros take user-specified alternatives and the permuter tries
   every cross-product, scoring each. Hand-iteration tests one form per
   sandbox run.
2. **Layer directed mutations onto random** — `PERM_RANDOMIZE(...)` opens
   a region for random codegen exploration around your directed seeds.

The bridge is the helper `tools/permuter_annotate.py` (`--hint <slug>`)
and the conventions below — write the function with PERM_* directives
that encode the rule's recommended maneuver, then `./tools/decomp-
permuter/permuter.py <dir>` exhaustively scores every combination.

## When to reach for permuter at all

Default posture today: workers under-use permuter. Reach for it when:

- You measurably **lowered the sandbox floor** with a manual restructure
  but plateau at honest distance **2 ≤ d ≤ ~30** — too close for hand
  search, too noisy for a single guess.
- A technique-index rule **lists 2+ alternative spellings** of the same
  maneuver (e.g. shared-end-label has `goto end; ... end: return v;`
  vs. accumulator var vs. switch-with-break-and-shared-return) — encode
  them as `PERM_GENERAL(...)` choices and let the permuter pick.
- You have a **register pin** ([[register-asm-pins]]) you suspect is
  not the lever — `PERM_VAR(reg, "$t0", "$t1", ...)` sweeps the
  candidate registers so the pin's "did it take?" question is mechanical.
- You're considering a **statement reorder** (e.g.
  [[loop-rotation-two-shift]], [[walking-pointer-serializes-parallel-loads]],
  [[defer-store-past-later-compute-into-jal-delay]]) — `PERM_LINESWAP`
  enumerates orderings.

Do NOT reach for permuter to:
- **Find a cheat.** Permuter's scorer is cheat-blind: it accepts any
  form that produces target bytes, including the entire catalog in
  [[no-new-park-categories]] ("cheats by any spelling"). The 2026-06-01
  `func_8007B844` cautionary tale is recorded there — 36k iters returned
  a dead-conditional-store form. The closing form MUST clear the
  vetting checklist before you surface it.
- **Replace the sandbox honest-distance metric.** The permuter's
  weighted score and the engine's `sandbox --disable all` masked
  distance are different metrics; don't cross-compare.

## The macros

Source of truth: `tools/decomp-permuter/README.md` §"the .c file may be
modified with any of the following macros". Reproduced + annotated:

### `PERM_GENERAL(a, b, ...)`
Expands to any one of `a`, `b`, .... The permuter cross-products with
every other PERM site. Commas inside parens are NOT separators; `(,)`
is the literal-comma escape.

```c
sum = PERM_GENERAL(a + b, ({ s32 t = a; t += b; t; }));
```

**Pairs naturally with:** rules that present 2+ legal C spellings of
the same operation. Examples:

- [[shared-end-label]] vs per-case return — `PERM_GENERAL(return v;, goto end;)`
- [[switch-vs-ifchain-branch-sense]] — `PERM_GENERAL(switch (x) { ... }, if (x == K) ...)`
- accumulator forms — `PERM_GENERAL(sum = a + b;, sum = a(,) sum += b;)`

### `PERM_VAR(name, value)` / `PERM_VAR(name)`
First form sets a meta-variable; second expands it. Combined with
`PERM_GENERAL` for the value, it sweeps a value used in many places.

```c
PERM_VAR(reg, "$t0")
register s32 cached asm(PERM_VAR(reg)) = ...;
```

**Pairs naturally with:** [[register-asm-pins]] — sweep candidate
registers (`PERM_GENERAL("\"$t0\"", "\"$t1\"", "\"$a3\"", "\"$v1\"")`)
without rewriting the file per candidate. **Vetting reminder:** a
sandbox-0 / SHA1-match closing form that requires keeping the pin in
the committed source is cheat-asm per [[register-asm-pins]] — the pin
is DIAGNOSTIC-ONLY; the finished match drops it and relies on C
structure. Surface the pin as evidence; do NOT commit it.

### `PERM_RANDOMIZE(code)`
Marks a region for the random pass. By default permuter only randomizes
when there are NO multi-choice PERM macros; if you provide any
`PERM_GENERAL`/`PERM_LINESWAP`, randomization stops unless you opt back
in with `PERM_RANDOMIZE`. Use to combine directed maneuvers with random
mutation around them.

### `PERM_FORCE_SAMELINE(code)`
Joins `code` to a single line after the C parser round-trip. IDO-flavoured
codegen is sensitive to same-lineness; observed effect on our
GCC 2.7.2 fork is small, but keep it for cases where the index rule
specifically points at "GCC's source-line tracking" (e.g.
[[loop-note-fixes-delay-slot-steal]]).

### `PERM_LINESWAP(lines)`
Permutes the ordered set of non-whitespace lines (split by `\n`); each
line must be zero-or-more complete C statements. Use `PERM_LINESWAP_TEXT`
for incomplete statements (slower; re-parses).

```c
PERM_LINESWAP(
    s32 val = *(s32 *)(p + off);
    off += 0x10;
    i += 1;
    sum += val;
)
```

**Pairs naturally with:** scheduling-sensitive rules where the lever
is the order of independent statements —
loop-counter-fills-load-delay (hint),
[[walking-pointer-serializes-parallel-loads]],
[[defer-store-past-later-compute-into-jal-delay]],
[[hoist-call-arg-local-flips-jal-delay]].

### `PERM_INT(lo, hi)`
Expands to an integer in `[lo, hi]` (both constants). Used to sweep
small constants — array sizes, mask values, shift counts.

### `PERM_IGNORE(code)`
Wraps code the C parser (pycparser) shouldn't touch — GCC extensions,
inline asm with unusual syntax, etc. The permuter passes it through
verbatim.

### `PERM_PRETEND(code)`
Expands for the parser but is removed afterwards. Used together with
`PERM_IGNORE` so the parser sees a stand-in declaration while the actual
non-standard construct is preserved as text.

**Pairs naturally with:** wrapping a function-scope construct that
pycparser chokes on (e.g. a macro that expands to a label) while you
direct the permuter to mutate the surrounding C.

### `PERM_ONCE([key,] code)`
Expands to either `code` or to nothing; each unique key gets expanded
exactly once across the cross-product. Default `key` = `code`. Useful
for "either-or" placement of a single statement:

```c
PERM_ONCE(x = y;) z = w; PERM_ONCE(x = y;)
```
yields `x = y; z = w;` or `z = w; x = y;`.

**Pairs naturally with:** rules where the lever is "this statement goes
HERE or there, but only once" — e.g.
[[hoist-call-arg-local-flips-jal-delay]] (where the arg-load goes
before vs. inside the call's block).

### Nesting + commas

Nested macros are allowed; the README example builds `PERM_ONCE`
equivalents from `PERM_VAR` + `PERM_GENERAL`. Commas inside parens are
NOT argument separators; use `(,)` as the literal-comma escape inside
a `PERM_GENERAL` alternative.

## Rule → macro cheatsheet

| Rule slug | Recommended macro | Notes |
|---|---|---|
| [[register-asm-pins]] | `PERM_VAR(reg, ...)` | Diagnostic sweep of pin register; DROP pin before commit. |
| [[shared-end-label]] | `PERM_GENERAL(goto end;, return v;)` | Test per-case vs. joined-return shapes. |
| [[loop-rotation-two-shift]] | `PERM_LINESWAP(...)` over loop body | Reorder shifts; SOTN-sanctioned `s32 one = 1;` opaque can also live in `PERM_GENERAL`. |
| [[switch-vs-ifchain-branch-sense]] | `PERM_GENERAL(switch (...){...}, if (...) ...)` | Sweep dispatch shape. |
| [[cross-jump-store-tail-merge]] | `PERM_GENERAL(...)` over per-exit form | Mix `goto endK` + inline `return`; SOTN-sanctioned per the index. |
| [[walking-pointer-serializes-parallel-loads]] | `PERM_LINESWAP` over the pointer-walk block | Reorder loads / stores; preserve dataflow. |
| [[defer-store-past-later-compute-into-jal-delay]] | `PERM_ONCE(GLOBAL = val;)` placement | Either-side-of-compute store placement. |
| [[hoist-call-arg-local-flips-jal-delay]] | `PERM_ONCE` for arg load placement | First-in-block vs. just-before-call. |
| loop-counter-fills-load-delay (hint, no rule file) | `PERM_LINESWAP` over load / inc / consume | Test load / address-advance / counter / use orderings. |
| [[narrow-byte-args-packed-call]] | `PERM_GENERAL(u8, s32)` for param type | Sweep narrow vs. word param decls. |

This list is starter coverage; extend as the helper grows.

## Invocation (the BB2 flow)

The helper writes a PROPOSED annotated copy to
`tmp/permuter_candidates/<func>.c` — it does NOT modify `src/`. From
there the standard permuter flow:

```bash
# Inside WSL, repo root:
source .venv/bin/activate
# 1. Stand up a permuter dir for the function (import.py wires compile.sh,
#    target.s/.o, settings.toml from the project's makefile).
python3 tools/decomp-permuter/import.py src/<file>.c asm/funcs/<func>.s
# 2. Replace the auto-generated base.c with your PERM_*-annotated version.
cp tmp/permuter_candidates/<func>.c tools/decomp-permuter/<dir>/base.c
# 3. Launch via the campaign wrapper (NOT raw permuter.py — see §Campaign
#    discipline below; the wrapper records telemetry + enables harvest, and
#    passes --stack-diffs by DEFAULT: without it the scorer normalizes sp
#    offsets to addr(sp)/imm, so frame-size/stack-offset gaps FALSE-MATCH at
#    score 0 — measured on func_80037540, 2026-07-13. Raw permuter.py runs
#    without --stack-diffs are blind to that whole gap class.)
python3 tools/permuter_campaign.py launch --func <func> --dir tools/decomp-permuter/<dir> \
    --label <chassis-slug> -j 8 --stop-on-zero
# 4. At the no-novel-find window (or session end), harvest + stop:
python3 tools/permuter_campaign.py harvest --dir tools/decomp-permuter/<dir> \
    --stop --reason "<why>"
```

The helper prints this command tail on stdout for every run.

## Campaign discipline — fresh-seed windows + telemetry (owner directive 2026-07-07)

**Every campaign goes through `tools/permuter_campaign.py`** (`launch` /
`harvest` / `status`) — never raw `permuter.py` for anything longer than an
interactive foreground run. The wrapper logs `permuter-launch` and
`permuter-harvest` events into `metrics/events.jsonl` (base score, iteration
count, every find with seconds-since-seed), which is what makes permuter
yield a data question instead of a gut question.

**The stopping rule.** The permuter is a hill-climber: a freshly seeded
basin yields its finds early or not at all. If **~20–30 minutes after a
fresh seed** no NOVEL find has landed (novel = outside the already-known
attractor classes for this function, not a re-find), `harvest --stop
--reason "no-novel-find window elapsed"` and either:

- reseed a **structurally different chassis** (a genuinely new CFG/statement
  geometry — reseeding restarts the clock and is the productive move), or
- switch modality (forensics, rederive, structural).

**Never leave campaigns simmering.** Evidence (marionation_Exec,
2026-07-07): campaigns left idle 10.5 h produced 0 new finds; a 5-basin
15 h+ portfolio harvested 0 sub-floor finds; the finds that DID land came
within ~1 h of fresh seeds and were all re-finds of known attractors; a
process sweep found ~100 orphaned permuter workers, some 21 h old, none of
which would ever have been harvested again. The single deep find on record
(func_8007B844, ~36k iters) was a cheat-form, rejected on sight. Long
tails re-find known attractors; they do not search.

**Harvest everything, always.** A 0-find harvest is the data point — run
`harvest` on every campaign you launched before your session ends, then
stop it. Campaigns must not outlive the session that seeded them.
(Grinder backstop: the driver reaps any surviving permuter process at
every session boundary — but a reaped campaign is LOST telemetry; the
harvest is your job.)

This is a measured-yield stopping criterion, not budget framing
([[no-budget-caps]] is untouched): the same search effort goes into MORE
fresh basins instead of dead tails.

**Hand-written prelude.inc note** (per [[difficult-is-not-impossible]]
§3): build `target.o` from `asm/funcs/<func>.s` so the function sits at
offset 0 — otherwise internal branch addresses mismatch and add ~340k
constant score noise that drowns the signal.

## Vetting permuter output — MANDATORY before surfacing

This rule does not relax any policy. Every closing form permuter
produces is a **proposal**; the worker must:

1. Re-read [[no-new-park-categories]] § "Auto-search tools (permuter,
   etc.) — output is PROPOSALS, not winners" and walk its 4-test
   checklist.
2. Run `python3 tools/check_completion_integrity.py` if rules were
   dropped.
3. Invoke `cheat-reviewer` per [[review-discipline-before-commit]] — the
   in-session reviewer is layer 1; the orchestrator's fresh adversarial
   `cheat-reviewer` is the MANDATORY layer 2.
4. Confirm the form is not in the EXPANDED CHEAT CATALOG of
   [[inline-asm-policy]] (alias renames, inline volatile casts, dead
   self-assignments, unused arrays, macro-hidden `__asm__`, register-asm
   pins left in committed source).

A permuter-found form that requires keeping ANY cheat construct in
committed source is REJECTED, regardless of `sandbox == 0` /
`SHA1 == oracle`. The work continues until the same byte-match falls
out of cheat-free C structure.

## Related

- [[codegen-technique-index]] — the index; this rule is a bridge layer.
- [[no-new-park-categories]] — the cheat-by-any-spelling policy that
  vets permuter output.
- [[review-discipline-before-commit]] — the mandatory layer-2 cheat-
  reviewer gate; nothing ships without it.
- [[inline-asm-policy]] — the expanded cheat catalog the closing form
  must clear.
- [[register-alloc-pure-c]] — the lever playbook the macros encode.
- [[difficult-is-not-impossible]] — the `target.o`-at-offset-0 gotcha
  and the directed-PERM macros suggestion (§ "Permuter — but with a
  CLEAN single-function target").
