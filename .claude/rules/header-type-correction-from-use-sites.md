---
name: header-type-correction-from-use-sites
paths: [".claude/rules/header-type-correction-from-use-sites.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "OWNER RULING 2026-07-13 (evidence: func_8001B138) — a global's declared type may be corrected at its single canonical extern declaration when independent use-site evidence proves the header mistyped. FOUR HARD PRONGS (a-d); all mandatory, layer-2 verifies against grep+cast-necessity output, not author summary. Same GCC-fold-escape as a banned local coercion is NOT disqualifying — the test is the prongs, not the mechanism."
metadata:
  type: rule
---

# Header type correction from use-site evidence

**Owner ruling 2026-07-13** (confirmed case: func_8001B138, commit
`4598a98e`, layer-2 CONFIRM). A global's declared type may be corrected
at its `extern` declaration in a shared header when independent use-site
evidence proves the header was mistyped from the start. The correction
subsumes and removes compensating casts that had been silently doing
semantic work under the wrong type.

This is a **narrow, prereq-gated** sanction — a header type flip has the
same GCC-fold-escape mechanism as the local-holder and pointer-pun cheats
the Judge correctly bans (all three defeat `convert_for_assignment` at
`c-typeck.c:3987`), so the rule cannot be "does it defeat a fold?" — that
proves too much (it would ban every legitimate `s16` variable). The test
is the four prongs below, applied against the actual codebase state.

## The four prongs (all mandatory)

**(a) All use sites codebase-wide are independently consistent with the
new type.** Verified by `grep -n <symbol>` over `src/**/*.c` and
`include/**/*.h`, not by author claim. AND — as a tightening for
sparsely-used globals — at least ONE use site must exhibit signed-specific
(or width-specific, for width flips) semantics that would be dead or
wrong under the OLD type. Examples of qualifying signed semantics:

- A comparison against a negative literal (`x < -N`).
- A read into a wider signed lvalue with the implicit expectation of
  sign extension.
- A signed arithmetic shift (`x >> k` where the shift's sign extension
  is depended on by downstream code).
- A `if (x < 0) x += C; x >>= k` round-toward-zero-before-arithmetic-shift
  idiom.

An "all sites are also valid under the new type but none of them
positively require it" configuration DOES NOT clear this prong; the
absence of contradiction is not evidence of correctness.

**(b) The OLD type required functionally necessary compensating casts to
produce correct behavior, and those casts predate the residual-chasing
session.** "Functionally necessary" is defined strictly: **removing the
cast under the OLD type changes runtime behavior on the target ABI.**
Casts that only silence a warning, that are stylistic, or that are
defensive-but-behavior-neutral do NOT satisfy this prong. The
cast-necessity analysis is mechanical: for each cast, describe what the
generated expression evaluates to WITH and WITHOUT the cast under the
old type's promotion rules; if the two are behaviorally identical the
cast is not evidence.

The "predates the residual-chasing session" clause is falsifiable via
`git blame` — if the compensating casts were added in the same session
that discovered the residual, that is a signal the casts themselves are
the coercion, not evidence of one.

**(c) The fix is a single canonical `extern` declaration edit in a shared
header.** Never an alias-rename (`extern T x asm("Y");`), never a per-use
pointer-cast pun (`*(T*)&x = ...;`), never a local-scope typedef
override, never a macro that hides the coercion. These forms all leave
the header lying — the correction has to be to the ground-truth
declaration itself, not a per-use spelling that contradicts it.

**(d) The correction eliminates casts and special-casing at ALL use
sites — no residual site still needs the old type's behavior.** If the
codebase-wide diff shows use sites that KEEP their compensating casts
under the new type, the "correction" is really a per-site coercion in
disguise and MUST be rejected. A legitimate type correction *simplifies*
the code by removing casts consistently; it does not merely rearrange
where the coercion sits.

## Why "same GCC-fold escape as a banned local coercion" is NOT
## disqualifying

The Judge correctly bans within-function coercion families — local
`{s16 tmp = -0x1C00; g = tmp;}` holders, `*(s16*)&g = -0x1C00` pointer
puns — because those constructs have no purpose beyond changing what
GCC's front-end folds at one call site while leaving the declared type
lying. Their existence is inexplicable if the reader asks "what's the
natural way to write this?"

A header type correction has the same *low-level mechanism* (both defeat
`convert_for_assignment` at `c-typeck.c:3987`) but a categorically
different *high-level structure*:

| | Local holder / pointer pun | Header type flip |
|---|---|---|
| What the natural reader writes | Not this shape | Exactly this shape (given the new type) |
| Header declaration after | Still lying (`u16` when meant `s16`) | Corrected |
| Number of edited sites | 1 (the residual) | 1 (the declaration) |
| Number of eliminated casts | 0 | ≥1 (the corrected type subsumes them) |
| Consistent with `bitfield-direction-divergence` precedent | No | Yes |
| Consistent with `mmio-volatile-type-level` precedent | No | Yes |

Under the four-prong test, the local/pointer forms fail (c) and (d);
the header correction passes all four. **A commit cannot be rejected on
the "same GCC fold" ground alone — that argument proves too much (it
would ban every legitimate signed variable). Layer-2 reviewers must
apply the prongs, not the mechanism.**

## Confirmed case — func_8001B138 (2026-07-13, commit `4598a98e`)

`include/system.h:10`: `extern u16 g_file_vram_timer;` → `extern s16 g_file_vram_timer;`

Prong-by-prong at time of ruling:

- **(a) codebase-wide consistency + positive signed semantics** — grep
  finds exactly two real use sites: `src/code6cac.c:func_8001B138` and
  `src/ings.c:231` (`g_file_vram_timer = 0;`, behavior-identical under
  either type). The func_8001B138 body contains a signed clamp
  (`if (g < -0x1C00) g = -0x1C00;`), a signed upper-bound compare
  (`if (g >= 0x7401) g = 0x7400;`), AND a signed round-toward-zero
  idiom on a captured value (`v = g; if (v < 0) v += 0xF; D_800FF5E0 = v >> 4;`).
  Three independent qualifying signed semantics. PASS.
- **(b) functionally necessary compensating casts predating session** —
  five `(s16)` casts existed in the function body before the residual
  investigation. Cast-necessity analysis: on this ABI `u16` promotes to
  signed `int` fully representing 0-65535, so `g_file_vram_timer < -0x1C00`
  without the cast is unconditionally false and the entire lower-clamp
  branch is dead code; the same reasoning applies to the `>= 0x7401`
  compare being trivially true for the upper half of u16 range. Removing
  the casts under u16 provably changes runtime behavior at the clamp.
  `git blame` confirms the casts predate this session. PASS.
- **(c) single canonical extern edit** — one line changed in
  `include/system.h`; no alias-rename, no pointer-pun, no per-use
  typedef, no macro-hidden coercion. PASS.
- **(d) casts eliminated everywhere, no residual sites** — after the
  flip, all five `(s16)` casts + both `(u16)(g +/- 0x4CC)` truncation
  casts in the function are removed and the code compiles/runs
  identically at every site. The `ings.c` site needed no change (a
  zero-assign is type-agnostic). Zero use sites remain under the new
  type that still need old-type behavior. PASS.

Result: sandbox 5 → 0; `retire` dropped one fill_delay rule; full clean
build SHA1 == oracle.

## Precedent hazard + abuse signals

**The strongest abuse signal is failing prong (a)'s positive-evidence
clause:** an author who wants to close a byte via header flip on a
global that appears in one site with NO signed-specific semantics is
almost certainly redirecting a within-function coercion into the
header. Reviewers reject on prong (a) even if the flip happens to
produce SHA1 == oracle; the residual is not evidence of correctness,
the use-site semantics are.

**Second-strongest signal is failing prong (b)'s cast-necessity
analysis:** if the compensating casts under the OLD type produce
behavior identical to the uncast expression, the casts were not
evidence — they were noise. Author must not skip the mechanical
per-cast WITH/WITHOUT comparison; if they can't produce that analysis,
the prong fails.

**Third signal is a git-blame timing collision:** if the compensating
casts were introduced in the same session that discovered the residual,
prong (b) fails as a matter of definition — the casts and the flip are
the same coercion, not evidence for it.

**Fourth signal is failing prong (d):** if the "corrected" body still
carries any cast that would be redundant under a correct declaration
of THAT type, the correction is incomplete. A legitimate type flip is
a whole-tree simplification, not a targeted one-site adjustment.

**Layer-2 verification is mandatory and specific to this rule:** the
reviewer runs the grep themselves, produces the cast-necessity table
themselves, and applies each prong to the ACTUAL codebase state — not
to the author's summary of it. The author's four-prong claim is a
starting point, not a verdict.

## Not a wand (scope limits)

- **Does NOT sanction width flips (`u16` → `u32`, `s8` → `s32`, etc.).**
  Width changes affect ABI-visible layout (struct offsets, array
  strides, va_arg promotion). This rule covers signedness only; a width
  flip needs its own evidence bar including layout consistency across
  every consumer, and is out of this rule's scope.
- **Does NOT sanction changing a type from a struct/union/pointer to a
  scalar or vice versa.** Ground-truth structural claims about the
  binary need their own SOTN-evidence pathway.
- **Does NOT create a general "flip a header type to close a byte"
  license.** The four prongs are the gate; the residual is not
  evidence.
- **Does NOT license unilateral edits to `include/*.h` during grind
  sessions** unless the four-prong evidence pack is on record. The
  Grinder's default-FAIL Judge treats a header edit without a documented
  four-prong pack as a coercion by default.

## Related

- [[bitfield-direction-divergence]] — sibling "header truth" pattern:
  target codegen reveals the true bitfield direction, fix at the header
  declaration order.
- [[mmio-volatile-type-level]] — sibling "header truth" pattern: MMIO
  addresses are volatile at declaration as ordinary hardware semantics.
- [[inline-asm-injection]] — the within-function coercion families this
  rule sits BESIDE (alias-rename, pointer-pun, local-holder). Those
  remain forbidden; this rule does not relax them, and they cannot be
  reintroduced through prong-(c)/(d) failures.
- [[no-new-park-categories]] — the standing "cheats by any spelling"
  posture. This rule is a *narrowly-scoped exception* under the four
  prongs, not a general relaxation of the posture.
- [[review-discipline-before-commit]] — layer-2 review is mandatory for
  commits invoking this rule; the six-test checklist plus the
  prong-by-prong verification.
- [[completion-standard]] — a header-corrected match still needs zero
  rules + SHA1 == oracle to reach COMPLETED-C.
