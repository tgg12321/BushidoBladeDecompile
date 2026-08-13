# SELF-VET — func_800645B0

Session 9 (structural modality, 2026-08-12).  Diff = the body of
func_800645B0 in src/text1b.c, replaced by
memory/grind/func_800645B0/candidate.c.  Measured THIS session with those
edits in place: `sandbox func_800645B0 --disable all` → score 0,
target_insns 78, build_insns 78, scorable true, rules_dropped 1.

CONSTRUCTS: C1 `val` reuse (const-1 shift operand, then the D_800A3444 value and the OR result); C2 read-modify-write spelling of the occupancy-bit set; C3 `last = rand();` named intermediates; C4 `idx` reuse (slot number, then the derived word-element subscript); C5 `val = idx;` staging the sum's addend one line ahead of `idx = idx2 + val;` (FAKE-annotated)

SIX-TEST CHECKLIST:

C1-C4 are unchanged from the session-6/8 "SB" body, which this function's
session-4 layer-1 cheat-reviewer explicitly ruled legitimate ("pins/goto
removed, var-reuse matches the confirmed defeat-licm-hoist-var-reuse
precedent, RMW spelling and named-intermediate are ordinary C").  C5 is the
only new construct in this session's diff and is the one that moved the
honest floor from 1 to 0.

## T1 semantic purpose
- **C1** — real: `val` holds a live value at every point (the shift operand,
  then the occupancy word read/modify/write).  Removing the reuse means
  declaring a second local: a style difference, not a behaviour difference.
- **C2** — real: the OR result is what gets stored back to D_800A3444.
- **C3** — real: the rand() result is consumed (masked into the store
  operand); naming it is ordinary C.
- **C4** — real: both values of `idx` are read (the slot number by the shift
  and the mask; the word subscript by all three word-stride stores).  Neither
  assignment is dead.
- **C5** — the staged value is REAL and is read by the very next statement
  (`idx = idx2 + val;`); it is not a dead store, and DCE does not touch it.
  What removes the extra machine instruction is ordinary copy coalescing, the
  same thing that happens to any C temporary.  Honest answer to the literal
  test: the function's OBSERVABLE behaviour is identical with the one-line
  spelling `idx = idx2 + idx;`, so C5 adds no behaviour — it is a matching
  construct, which is precisely why it is FAKE-annotated and claimed under a
  sanctioned family below rather than defended as ordinary C.  That is the
  disposition .claude/rules/staged-value-reused-variable.md prescribes for
  exactly this shape (its own worked example,
  `v0 = idx_1494[1]; arg5 = tbl_125c[v0];`, is behaviour-identical to the
  one-line spelling too).

## T2 human-programmer
- **C1-C4** — yes.  A programmer implementing "find a free slot, fill three
  parallel word arrays and one halfword array at that slot, then set the
  occupancy bit" writes exactly this shape: a mask built from the slot
  number, a halfword subscript, a word subscript derived from it, and an RMW
  on the occupancy word.
- **C5** — no, not from the specification alone; a reader asks "why stage
  it?".  The in-source annotation answers that question at the site.  Under
  the six-test checklist this is the honest FAIL that the sanctioned-family
  claim below has to carry; I am not claiming C5 is ordinary C.

## T3 GCC-internals justification
- **C1-C4** — no GCC-internals reasoning is needed to explain any of them;
  each follows from what the function does.
- **C5** — yes, explicitly: optabs.c:403-421 (`expand_binop`) swaps a
  commutative operand pair when the expansion target IS op1.  Named in the
  annotation as the mechanism, as bound 4 of the cited rule requires.  This is
  a declared property of the construct, not a concealed one.

## T4 permuter/search provenance
No permuter, no auto-search, no PERM_* macro was involved.  C5 was derived
top-down from the session-8 frontier's item 1 — a named, ledger-recorded
hypothesis about optabs.c's swap condition — and then measured.  The
hand-authored sweeps in tmp/grind/func_800645B0/s9b/ exist only to decide
WHICH spelling of the family to ship (sweep30) and to measure the
no-staging arithmetic alternative, which costs an instruction, 79 vs 78
(sweep31).  The construct is not "whatever passed the detectors": it is the
single C-level consequence of a mechanism the ledger named before any
measurement was taken.

## T5 family check
- **C1-C4** — covered by the frozen SOTN list (variable reuse for codegen
  control; named intermediates), and already reviewed as legitimate for this
  function.
- **C5** — covered by .claude/rules/staged-value-reused-variable.md (owner
  ruling 2026-07-03), claimed formally below.  It is none of:
  register-asm pin, hardcoded-$N asm, regfix/asmfix rule, scheduling barrier,
  volatile coercion, alias rename, unused local or array, dead store (the
  staged value is read on the next line), dead conditional store, `if (1)`
  wrapper, do-while(0) wrapper, dead goto/label pad, DImode chain, redundant
  width cast.
- **Explicit check against this function's four standing bans.**  I have read
  all four; none is present in this diff, in this or any other spelling.
  (a) The banned counter-statement relocation: nothing in this body was moved
  relative to the SB body — the two `for` headers and every statement inside
  them are character-for-character the SB spelling, as `git diff` shows (the
  only hunk is inside the `if` arm).  (b) The banned pair of assignments at
  the top of the nested `for`: the staging here sits inside the `if` arm,
  seven statements below that point, borrows a PRE-EXISTING variable instead
  of inventing one, and takes no part in the enclosing skeleton at all.
  (c) The banned FAKE dead store to a fresh constant-carrier local: there is
  no dead store in this body and no new local was declared.  (d) The banned
  VA-VG sweep of session 9: none of those forms is shipped; that whole family
  stays banked under rejected/.  Most importantly the MECHANISM differs.  All
  three earlier layer-1 FAILs on this function are spellings of one attack —
  steering cc1's first-pass scheduler tie / reorg.c's back-edge delay-slot
  theft at the top of the nested `for`.  C5 does not touch scheduling: it
  changes which of two operands RTL EXPANSION emits first in a single `addu`,
  at a site where the target and the build already agree in every other
  respect, and the emission order at the skeleton top is the one the SB
  chassis already produced for free.  That axis (optabs.c's commutative swap)
  has never been levered in this function's nine sessions.

## T6 naming-announces-intent
No new identifier was introduced by this session's diff.  `val` predates it
and is named for its primary role (the occupancy value); `idx`, `idx2`,
`last`, `mask`, `i`, `j` are all descriptive.  There is no
`pad`/`dummy`/`unused`/`spill`/`tmp`/`fake`-style name anywhere in the body.
Every declared local is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: staged value through a reused variable
  SCOPE: "SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: d9e490c094bc903ca53877dc2fb7887458d18ec6

  FAMILY: Variable reuse for codegen control
  SCOPE: "reusing one C variable for two unrelated values to influence loop-invariant detection or RA."
  PRECEDENT: .claude/rules/no-new-park-categories.md:170

BOUND-BY-BOUND ANSWER for C5 against the six bounds of
.claude/rules/staged-value-reused-variable.md:
  1. *Value real and used* — `val = idx;` is read by the next statement.
  2. *Variable already exists for a real job* — `val` is the function's
     occupancy-value local: it supplies the shift operand for the mask and
     holds the D_800A3444 read and the OR result.  It is multiply assigned in
     the body that predates this session.  No variable was invented; the five
     other measured spellings that DO invent one (sweep30 WB/WD/WE/WF/WG, all
     also 0/78) are deliberately NOT shipped for this reason.
  3. *Borrow provably safe* — `val`'s previous value (the constant 1) is dead
     from `mask = val << idx;` onward, and the staged value is not read after
     `val = D_800A3444;` overwrites it.  The borrow window is three statements
     long and contains no read of `val`.
  4. *Annotated* — the in-source `/* FAKE: ... */` block carries what is
     staged, why the borrow is safe, the named pass (optabs.c `expand_binop`,
     optabs.c:403-421) and the lever-exhaustion pointer.
  5. *Last resort with receipts* — nine sessions of measurements in
     memory/grind/func_800645B0/ (H24/H25 close the expansion-target route,
     H31/H35 close the sum-spelling route, H39 closes the reg_n_sets route,
     sweep31 closes the no-staging arithmetic route at 79 instructions
     against a 78-instruction target).  25 disproven bodies are banked under
     memory/grind/func_800645B0/rejected/.
  6. *Everything else still applies* — no dead stores, no unused variables,
     no pins, no inline asm, no volatile, no regfix/asmfix edit.  The only
     file this session modified under src/ is src/text1b.c, and only the body
     of func_800645B0.

ANNOTATION-CONFORMANCE: one FAKE construct (C5); its annotation, reproduced
from src/text1b.c:
  /* FAKE: the slot index staged through `val` -- `val`'s const-1 value is dead
   * from the mask shift above until `val` is re-loaded from D_800A3444 below,
   * and the staged value is not read after that re-load, so the borrow is
   * safe; the value is real and consumed by the very next statement.
   * Mechanism: optabs.c `expand_binop` (optabs.c:403-421) swaps a commutative
   * operand pair whenever the expansion target IS op1, so writing the *3 sum
   * back into `idx` with `idx` as the addend can only ever emit
   * `addu $s0,$s0,$s1`; staging the addend in another pseudo leaves the pair
   * unswapped and emits the original's `addu $s0,$s1,$s0`.  lever-exhaustion:
   * memory/grind/func_800645B0/hypotheses.md sessions 1-9 (H24/H25/H31/H35/H39
   * close every other route to this operand order; sweep31 measures the
   * no-staging arithmetic spellings at 79 instructions against a
   * 78-instruction target). */
It carries all three required elements: WHAT is staged and why the borrow is
safe, the MECHANISM as a named GCC pass with file:line, and the
LEVER-EXHAUSTION pointer.
