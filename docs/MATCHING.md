# Matching Techniques

How to get GCC 2.7.2 (PsyQ-era, through this project's cc1 → maspsx → as pipeline) to
emit byte-identical code **from honest C**. The bar is COMPLETED-C: pure C, zero cheats,
full-build SHA1 == oracle ([`DECOMP_WORKFLOW.md`](DECOMP_WORKFLOW.md)). The living,
symptom-indexed technique catalog is `.claude/rules/` — start from
`.claude/rules/codegen-technique-index.md`, which maps diff symptoms to rule files and
marks which constructs are sanctioned, sanctioned-with-prerequisites (FAKE-annotated
SOTN families), or forbidden. This page is the short primer.

## The matching mindset

Matching is not "translate the asm into clean C"; it is writing the C a 1998 programmer
plausibly wrote that this compiler turns into the target bytes. Most effort goes into
understanding which GCC pass (CSE, combine, loop/LICM, local/global alloc, sched, reorg)
produced a difference, then finding the *source* spelling that reproduces it.

## Diagnose before you edit

All through the engine (`& tools/wteng.ps1 main <cmd>`):

| Command | Tells you |
|---|---|
| `canonical <func>` | whether the function is C at all (run first) |
| `sandbox <func> --disable all` | the honest, cheat-stripped distance |
| `sandbox <func> --disable all --diff` | **where** it differs, per hunk, classed source-level / operand-only / not-scored |
| `diagnose <func>` | matchable / control-flow / canonical / plateau |
| `verify-oracle --rebuild` | the only truth: full build SHA1 == oracle |

Rough routing: register-only renames → allocation order (declaration order, live
ranges, which value is the call return); reorders → statement order, where values are
born, loop shape; extra/missing instructions → what the target caches vs reloads, CSE
spans, types; frame-size differences → locals the original declared (see the
phantom-frame rules). For pass-level evidence use the instrumented cc1
(`tools/gcc-2.7.2/cc1`, `-da` dumps).

## Core pure-C techniques

- **Declaration order** steers callee-saved register assignment; try it before anything else.
- **Destination type picks the load**: `s32 x = s16arr[i]` → `lh`; mismatched `extern` types
  across files (`s16` vs `u16`) make one file wrong — fix the header from the use sites
  (`header-type-correction-from-use-sites`).
- **Signedness** picks opcodes: `slti` vs `sltiu`, `(s8)` vs `(u8)` constants, `divu` (no
  `INT_MIN % -1` break block) for an unsigned `%`.
- **`/ (1 << N)` vs `>> N`**: a `bgez + addiu` rounding pattern in the target means division.
- **Truncation on little-endian**: `(s16)v` of a memory operand can read `lh +2` (the high
  half); write `v & 0xFFFF` for the low half.
- **Loop shape**: `do { } while (--i >= 0)` count-down is common in PsyQ-era code; pre-test
  `while` adds a top branch.
- **Branch sense**: `if (c) goto L;` vs `if (c) { body }` are not byte-equivalent; match the
  target's sense (`switch-vs-ifchain-branch-sense`).
- **Statement and expression order** drive scheduling and register birth order; reorder
  real statements rather than adding barriers.
- **Data model**: many "unmatchable" functions are a wrong declaration — an aggregate split
  into scalars, a wrong width, a wrong struct layout (`split-scalars-hide-aggregate`).

## Forbidden — never a completion

`register T x asm("$N")` pins, inline `__asm__` carrying general-purpose opcodes or
hardcoded `$N`, scheduling/memory barriers (`do {} while (0)` used as a fence,
`asm("" ::: "memory")`), strength-reduction/LICM-defeat asm, typed coercions whose only
purpose is to steer codegen, regfix/asmfix post-pass rules (retired), and compiler
patches. Sanctioned exceptions (GTE/cop2 islands per `inline-asm-policy`, FAKE-annotated
SOTN families per `no-new-park-categories`) apply only with their prerequisites met.

## Permuter vs full build

The permuter and `sandbox` compile ONE function in isolation; the full build compiles
whole TUs and links. Differences that matter:

- `.L<N>` labels are numbered per file, so neighbouring functions shift each other.
- A change in one function can cascade into later functions of the same large TU
  (`text1b*.c`, `main.c`, `code6cac*.c`) — always re-run `verify-oracle --rebuild`.
- The permuter checks `.text` only; a wrong `case` order shows up only in `.rodata` (jump tables).
- Build the permuter target from `asm/funcs/<func>.s` so the function sits at offset 0.

A permuter `min_score=0` or `sandbox == 0` is a hypothesis; the full-build SHA1 is the
test, and the closing form still has to pass the cheat review.

## Gotchas

- **Stale build cache**: only the edited `.o` recompiles; confirm with `verify-oracle --rebuild`.
- **Caller arity**: a declaration narrower than real calls makes callers dead-code argument
  computations — widen to the maximum observed arity.
- **Stack args**: PsyQ reads a `u16` stack slot from offset+2; changing a callee's `s32` stack
  argument to `u16` shifts both caller and callee (`narrow-stack-param-subword-offset`).
- **Shell nesting**: `$N` inside `wsl bash -c '…'` strings is expanded by the outer shell —
  write scripts to files; build files must stay LF.

## Stuck?

Change modality, never target: read the `--diff` hunks, pull pass dumps, read the
matching rule in the technique index, sweep spellings (`tools/spelling_enum.py`), run a
permuter campaign (`tools/permuter_campaign.py`), re-derive from m2c. Stuck is never a
reason to skip the function or to leave asm or a cheat as the answer
(`difficult-is-not-impossible`).
