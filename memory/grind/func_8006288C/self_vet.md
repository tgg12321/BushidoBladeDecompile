# SELF-VET — func_8006288C

Diff under review = `src/text1b.c` working tree vs HEAD (commit `37cd1270`).
Four changes, listed as CONSTRUCTS below. Sandbox `--disable all` = 0 this
session (score 0, build_insns 52 == target_insns 52).

NOTE ON THIS REVISION (session 3): the previous session's self-vet was
DISCARDED by the driver validator on FORMAT only — its SCOPE quotes were
line-wrapped across several physical lines, and the validator's regex
(`tools/grinder/grindlib.py:51`) requires `SCOPE: "<text>"` to sit on ONE
line. The quotes below are the same rule text, de-wrapped onto single lines
(newlines in the rule file replaced by single spaces; no other change), and
each PRECEDENT is a literal file:line. Nothing about the C or the technical
argument changed; the form under review is byte-identical to the one that
measured 0 last session and measures 0 again this session.

CONSTRUCTS: (C1) loop respelled as `do { ... goto out; ... } while (i < 6);`
replacing the `loop_top:` label + `goto loop_top` in an else arm; (C2) the
`D_800F1138 = 1;` store moved from init-block position 7 to position 1; (C3)
the pre-existing constant-holder local (`int new_var;` in HEAD) moved from
init-block position 8 to position 3, renamed `one`, retyped `int` -> `s32`,
and used as the shift base `mask = one << i` (HEAD already had `mask = new_var
<< i`); (C4) the `/* FAKE: ... */` annotation comment on C3's declaration.

## T1 semantic purpose
- **C1** — YES, semantic. `do/while` + early `goto out;` is the ordinary
  spelling of "scan slots 0..5, take the first free one, stop". It is not an
  added construct; it is a different (and more idiomatic) spelling of the same
  control flow HEAD expressed with a backward `goto`. Nothing is added or
  removed from the program.
- **C2** — YES, semantic. `D_800F1138 = 1;` is a real store to a real global;
  moving it earlier in a block with no aliasing or ordering dependency on the
  other seven init statements is an ordinary statement reorder. The store is
  still executed exactly once, unconditionally, with the same value.
- **C3** — NO semantic purpose beyond what `mask = 1 << i;` produces. The
  program is behaviourally identical with a literal. This is the one construct
  in the diff that needs a sanctioned family, and it is claimed as one below.
  Mitigating facts recorded honestly: the holder is NOT dead — it is read on
  every loop iteration as the `sllv` shift base — and it is NOT new in this
  diff; HEAD already carried it as `int new_var;` with the same single use.
  This session moved it, renamed it, and annotated it.
- **C4** — a comment; no codegen effect. Required by the sanction it documents.

## T2 human-programmer test
- **C1** — a human writing a first-free-slot scan writes exactly this loop.
  HEAD's label + backward-goto form is the one a reader would question.
- **C2** — `D_800F1138 = 1;` is a "mark the request as pending" flag store; a
  human writing this allocator would plausibly set it first, before the scan
  bookkeeping. Nothing about its position reads as odd.
- **C3** — a reader COULD ask "why not `1 << i`?". I am not claiming otherwise.
  That is precisely why this is submitted under a sanctioned family with a
  FAKE annotation rather than as ordinary code. Note the shape (`one` holding
  1, used as a shift base) is the exact spelling the frozen SOTN list names.
- **C4** — n/a (annotation).

## T3 GCC-internals justification
- **C1, C2** — the *effect* is explained by GCC internals (C1 permutes
  `global.c:allocno_compare` via `reg_live_length`; C2 changes RTL LUID order
  in the init block and keeps cse from folding the two constant-1s), but each
  construct also stands on its own as program logic: C1 is a loop, C2 is a
  store. A reader needs no GCC knowledge to accept either. Not a cheat signal.
- **C3** — YES, the justification IS a GCC-pass mechanism (loop.c invariant
  hoist placing the `(set reg 1)` at the preheader tail; sched.c first pass
  then emitting it at init slot 6 instead of slot 2). I am not hiding this;
  it is the named mechanism the sanctioned family requires, and it is written
  into the FAKE annotation. Under the checklist this is the cheat SIGNAL for
  C3, which is why C3 is carried by an explicit family claim, not by argument.
- **C4** — n/a.

## T4 permuter/search provenance
No permuter, no auto-search, no directed-PERM macros were run on this function
in any session. C1 came from s1's reasoning about `allocno_compare`. C2 came
from reading s1's H6 result and noticing that H6 held the store fixed at init
position 7, so "store first, holder second" was a cell H6 never covered — a
derivation, not a search hit. It was then confirmed by a 16-variant standalone
cc1 sweep (`tmp/grind/func_8006288C/s2/variants5.py`) and re-confirmed in-tree
by the sandbox. C3 is not a search find either: it is HEAD's own pre-existing
local. No construct in this diff survives only because a detector fails to
spell it.

## T5 family check
- **C1** — no family. It is a loop. The frozen list's `do { } while (0);`
  carve-out is NOT being invoked and is not relevant: this is a real loop with
  a real 6-iteration trip count and a real exit condition, not a
  zero-iteration wrapper. s1 measured `while (1) { ... break; ... }` as
  codegen-identical, i.e. the lever is the loop's block/note layout, not a
  wrapper trick.
- **C2** — no family. Statement reorder of a real store.
- **C3** — matches the sanctioned constant-holder / opaque-arithmetic-variable
  family; claim + citations below. It does NOT match any forbidden family: it
  is not a dead local (it is read every iteration), not an array, not
  address-coerced, not volatile, not an alias rename, not a pin, not asm, not
  a dead store (the value 1 is genuinely consumed by the `sllv`), and not a
  chain-extender (it adds no instruction — the `addiu $t3,$zero,1` exists in
  the target and in the literal form alike; only its slot moves).
- **C4** — no family.

## T6 naming-announces-intent
`one` names the VALUE the variable holds (1), not a coercion role. It is not
`pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`, or
`_frame_pad`. It has a real use (the shift base), so it is not
declaration-only, discard-only, or address-of-only. It is also the literal
spelling the sanction's own text uses (`s32 one = 1;`). HEAD's name was
`new_var`, an m2c artefact; the rule explicitly prefers renaming to something
meaningful, which is what was done.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Constant-holder / dead scalar locals
  SCOPE: "a local variable whose only purpose is codegen influence — holding a constant in a register across calls/statements, or existing as a declaration that biases register allocation — is a sanctioned last-resort matching lever under the prerequisites below."
  PRECEDENT: .claude/rules/named-local-fake-exception.md:12
  (Quote spans lines 12-17 of that file in the wrapped source; de-wrapped to one
  line here for the validator. Cross-listed on the frozen SOTN list at
  .claude/rules/no-new-park-categories.md:229.)

  FAMILY: Opaque arithmetic variables
  SCOPE: "**Opaque arithmetic variables** ([[loop-rotation-two-shift]]): `s32 one = 1;` to prevent compiler bit-test transforms."
  PRECEDENT: .claude/rules/no-new-park-categories.md:175
  (Quote spans lines 175-176 in the wrapped source; de-wrapped to one line.)

  Prerequisite evidence for the above (the rule's four strict prerequisites):
    1. Lever-exhaustion documented — memory/grind/func_8006288C/hypotheses.md
       H6 (s1, 8 holder positions with the store fixed), H7 (s2, 10 literal-1
       init orderings + loop spellings, sweep set 6), H8 (s2, 6 scalar-type
       permutations, sweep set 5 axis B). Every literal-1 form measured leaves
       `addiu $t3,$zero,1` at init slot 6; the target has it at slot 2.
    2. GCC-pass interaction named — loop.c invariant hoist emits the
       `(set reg 1)` at the preheader TAIL; sched.c's first pass then places
       it at init-block slot 6. A holder assigned in the init block gives the
       insn an early LUID instead, and sched emits it at slot 2.
    3. Annotation present — see ANNOTATION-CONFORMANCE below.
    4. Layer-1 + layer-2 cheat-reviewer — layer 1 is the driver's pre-Judge
       reviewer on this diff; layer 2 is the operator's fresh reviewer before
       acceptance. Neither is claimed by me.

ANNOTATION-CONFORMANCE:
  /* FAKE: `one` is an opaque holder for the constant 1 rather than a literal
     `1 << i`; mechanism: with a literal the shift base is loop-invariant and
     loop.c hoists its `(set reg 1)` into the preheader TAIL, so sched.c's
     first pass parks `addiu $t3,$zero,1` at init-block slot 6 instead of the
     target's slot 2. Lever-exhaustion: memory/grind/func_8006288C/
     hypotheses.md H6 (s1) + H7/H8 (s2) — every literal-1 init-block ordering
     and every scalar-type permutation measured, all leave the constant at
     slot 6. */
  Carries all three required elements: WHAT (opaque constant holder instead of
  a literal shift base), MECHANISM (named passes: loop.c invariant hoist +
  sched.c first-pass placement), LEVER-EXHAUSTION (hypotheses.md H6/H7/H8).

## Residual honesty note for the reviewer
The function still carries its 6 regfix rules in `regfix.txt`; this session may
not touch that file and did not. The sandbox score is the cheat-invisible one
(`rules_dropped: 6`), so the 0 is honest pure-C, but the rules must be retired
by the operator (`retire func_8006288C`) before `queue done` — that is the
normal grinder hand-off, not a hidden dependency of this C form.
