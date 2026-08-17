# SELF-VET — func_80049584

CONSTRUCTS: single-variable reuse — `i` carries both loop counters AND the computed total (`i = func_8004954C(...)`, `i = 0x24` / `i = 0x88` on the sentinel path, `i` read at the `end:` join), carrying a `/* FAKE: ... */` annotation. NO other non-obvious construct: the diff otherwise DELETES five `register asm("$N")` pins, an unused `s32 dummy[2]` array and a `(void) var_s2;` discard, and replaces the m2c transcription with plain C (two `do/while` pointer walks, `>= 0` sign comparison, `goto end` sentinel short-circuit, three distinct named pointer variables, one call).

## T1 semantic purpose
The reuse: the variable holds a real, read value at every point — the loop
index while counting, then the total that is compared against `D_800A324C`,
stored to it, and passed as `func_80045B68`'s second argument. There is no dead
store, no write that is never read, and no statement whose removal leaves the
program's behaviour unchanged. What the construct changes is which SINGLE
variable holds the total; the observable behaviour of the function is identical
either way. So the honest answer is: the reuse has full semantic purpose as
CODE (nothing dead), but the CHOICE to share the variable rather than declare a
second one is codegen-motivated, and I am declaring it as such rather than
dressing it up. Every other construct in the diff has plain semantic purpose:
the pointers walk arrays, the flag records whether the copy changed anything,
the `goto end` is the sentinel short-circuit that target's `j` encodes.

## T2 human-programmer
The clean C — pointer walks, sign comparison, ordered (lo,hi) normalization,
sentinel short-circuit — is exactly what a human would write from the spec, and
it is strictly MORE natural than the body it replaces. The reuse of one scratch
variable for a counter and then a result is the one thing a reader could ask
"why?" about. It is not unheard of in 1998 game C (a generic `i`/`n` scratch
int reused down a function is common in that era), but I am not claiming it
reads as inevitable — I am claiming it under a sanctioned family, below.

## T3 GCC-internals justification
Yes, and I state it plainly rather than hiding it: the mechanism is `global.c`
allocno allocation. A pseudo is only eligible for a call-saved hard reg when it
is live across a CALL. Neither `0..0x3A` counter crosses a call, so no separate
counter variable can be allocated `$s0` under any ordering — yet target holds
the counters in `$s0`, the same register that later holds the call's result.
The single-variable form is therefore not one lever among several; it is the
only C-level shape that can produce target's allocation. Measured both ways:
merging 23 -> 16, re-splitting 0 -> 12. This IS a GCC-internals mechanism, which
is why the construct is being declared under a sanctioned family with a
`/* FAKE */` annotation naming that mechanism, not offered as ordinary program
logic. For every other construct in the diff the answer is NO — they are
explained purely by what the function does.

## T4 permuter/search provenance
No permuter, no directed search, no auto-search tool of any kind ran on this
function. Every form was hand-derived from the target disassembly and confirmed
by `sandbox --disable all`. The reuse was predicted from the register
assignment BEFORE it was measured (target's counters and result share `$s0`),
then confirmed; it is not a spelling that happened to slip past detectors.

## T5 family check
Checked against the forbidden-family catalog line by line. The construct is NOT:
a register-asm pin, hardcoded-`$N` asm, a lost-codegen regfix, a scheduling
barrier, INLINE_MOVE_ALIASING, volatile coercion in any spelling, an unused
local array or scalar (nothing is unused), a dead-param-assign or
dead-conditional-store or empty-body dead read (nothing is dead — every write
to `i` is read), an `if (1)` wrapper, a dead-goto label pad, a DImode chain, a
param-local alias, an opaque `s32 one = 1;`, a lowercase `asm()` block, an
`asm("sym")` alias rename, a redundant width cast, or a rodata reorder. It is
also NOT the goto-end-with-ret-val accumulator + shared label family: the
`goto end` here encodes target's own sentinel short-circuit jump and exists in
the control form too (the rejected separate-`total` variant has the identical
`goto end` and still measures 12), so the label is not what is doing the work.
It DOES match a sanctioned family — declared below.
Note this session also REMOVED five forbidden constructs (the `asm("$N")` pins)
and one more (`s32 dummy[2]`, frame coercion) from the tree.

## T6 naming-announces-intent
The variable is `i` — an ordinary loop-index name. The other locals are `dst`,
`src`, `p`, `unchanged`, `rank`, `step`, `lo`, `hi`. None is named `pad`,
`_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`, or
`_frame_pad`, and none exists only to be discarded, left unused, or
address-taken: every declared local is both written and read on a live path.
The `s32 dummy[2]` that DID announce coercion intent is deleted by this diff.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Variable reuse for codegen control
  SCOPE: "reusing one C variable for two unrelated values to influence loop-invariant detection or RA"
  PRECEDENT: .claude/rules/no-new-park-categories.md:171

  (The quoted scope sentence is the rule file's own wording at
  `.claude/rules/no-new-park-categories.md:170-172`, joined across its two
  wrapped source lines; that entry resolves the family as ALLOWED on direct
  SOTN master-branch evidence — SOTN ships `idxSub = idxSub;` and
  `randy = basePoint.x; baseX = randy;` with "FAKE but makes register
  allocation work" comments. It is on the FROZEN sanctioned list, so no
  category extension is being requested here.)

ANNOTATION-CONFORMANCE:
  /* FAKE: `i` carries both the two loop counters and the computed total,
     mechanism: global.c allocno allocation — only a pseudo that crosses a
     CALL is eligible for a call-saved hard reg, so sharing one variable is
     what puts the loop counter in $s0 (target); with a separate `total` the
     counter takes a call-clobbered reg and 12 insns diverge.
     lever-exhaustion: memory/grind/func_80049584/hypotheses.md (H1/H3). */
  Carries all three required parts: WHAT (the shared variable and its two
  roles), MECHANISM (a named GCC pass — `global.c` allocno allocation and the
  crosses-a-CALL eligibility test), and LEVER-EXHAUSTION (a pointer to the
  ledger section recording the measured control form and why declaration-order
  permutations cannot substitute). The exhaustion record is in the ledger's
  hypotheses.md, not merely asserted here: it names the rejected control form
  (`rejected/separate-total-var.c`, floor 12) and the liveness argument that
  rules out the ordering alternatives.
