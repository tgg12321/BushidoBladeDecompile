# SELF-VET -- func_800645B0

Session s17 (2026-09-02, rederive modality).  Honest measurement with this diff
in place in src/text1b.c: `sandbox func_800645B0 --disable all` = score 0,
target_insns 78, build_insns 78, rules_dropped 0, zero non-canonical cheat-asm
in the body.  Re-measured from scratch THIS session after re-applying
memory/grind/func_800645B0/candidate.c over the `INCLUDE_ASM` line; raw engine
output banked at tmp/grind/func_800645B0/s17/sandbox.json.

CONSTRUCTS: (C1) a single-level do-while-zero wrap placed around the slot-index
computation; (C2) reuse of one scalar local as both the shift-constant carrier
and the occupancy-word read-modify-write scratch; (C3) a fresh named local
holding the tripled word offset that the three word writes consume; (C4)
ordinary C -- the two counted iterations, the halfword offset shift, the
`(s32)(&SYM) + off` addressing, the `break`, and the constant return.

## T1 semantic purpose
C1: NONE -- the wrap has no observable effect on the output; its body executes
exactly once with or without it.  It is a pure match device, which is precisely
what the do-while(0) family sanctions, and it is FAKE-annotated at the construct
site as that rule requires.
C2: none beyond a simpler form (a separate `one` local would behave
identically); it is the standing LICM-defeat carrier of this chassis, present in
the honest floor form since session 1 and FAKE-annotated.
C3: YES.  `wid` holds a real, consumed value -- the word index (slot * 3) --
read by all three word writes.  Removing it forces either recomputation or
clobbering the slot index, which is a different program, not a simpler spelling.
C4: yes, these are the actual semantics.

## T2 human-programmer
C1: NO -- a reader would ask why the computation is wrapped.  That is exactly
why the family requires the inline FAKE annotation, which this construct
carries; it is the SOTN-idiomatic marking for a wrap that exists for codegen,
and it is indistinguishable from an original PsyQ-era macro expansion (the
rule's "macro-idiom plausibility" argument).
C2: yes, borderline -- one scratch variable serving the mask constant and then
the occupancy word is ordinary period C; annotated anyway because its real
purpose here is defeating loop.c's hoist.
C3: YES.  Naming the word offset separately from the slot index is what a human
would write; clobbering the slot index with its own triple (the previous floor
form) is the less natural spelling of the two.
C4: yes.

## T3 GCC-internals justification
C1: YES -- the mechanism is stated as sched.c `adjust_priority` /
`birthing_insn_p` and is dump-proven (tmp/grind/func_800645B0/s17/wd.scheddbg.txt
vs dw.scheddbg.txt, instrumented cc1 with BB2_SCHED_DEBUG=1).  Under the general
checklist this would be the cheat signal; under
.claude/rules/do-while-zero-exception.md a named mechanism is a REQUIREMENT of
the sanctioned family (the FAKE template's "mechanism:" field), not a
disqualifier, and the rule states outright that "the mechanism a wrap influences
is invisible at source level anyway".  I am claiming the family, not arguing
around the test.
C2: YES, same posture -- loop.c `scan_loop`/`move_movables`, the
`n_times_set == 1` movable gate (tools/gcc-2.7.2/loop.c:705), which is the
documented mechanism OF the defeat-licm-hoist-var-reuse family.
C3: NO -- `wid` is justified by program logic (a distinct named quantity), not
by a pass.  Its codegen effect (expand_binop not swapping a commutative pair
when `target != op1`) is a consequence, not the reason a reader needs.
C4: no.

## T4 permuter/search provenance
C1: the wrap was originally FOUND by the session-5 decomp-permuter campaign
(2026-08-12, `tmp/grind/func_800645B0/s5/ws3/output-0-2`).  That provenance is
disclosed, and it is not what justifies it: the construct is a member of an
explicitly sanctioned family with a quoted scope and an in-hand PSX SOTN-master
precedent, it was re-applied and re-measured independently on the current tree
this session, and its mechanism was attributed from cc1 dumps rather than taken
on the permuter's word.  It passes because the family covers it, not because a
detector missed this spelling.
C2/C3: hand-authored; C2 predates this session as the standing floor form, C3
was measured and certified ordinary C by session s12.
C4: n/a.

## T5 family check
C1 matches the `do { ... } while (0);` family and nothing else; it is
SINGLE-LEVEL, so the nested-wrap justification duty does not apply.  Checked
against every standing ban on this record: it relocates no statement, it routes
no value through a second variable, and it introduces no dead write.  The
2026-08-12 rejection of this exact wrap
(rejected/permuter-bare-do-while0-wrapper-outside-carveout.c) rested entirely on
a reorg.c-only scoping that the owner had already abolished six weeks earlier on
2026-07-06; the brief's CURRENT SCOPE block states the live scope, so under the
driver's "RULE SCOPE IS DATED" clause the form was restored and re-measured.
Under the live scope, "any codegen effect" is the sanction, so an objection that
reduces to "this wrap steers a scheduling decision" cannot distinguish it from
every wrap the family permits.
C2 matches defeat-licm-hoist-var-reuse (already ruled legitimate for this
record by the 2026-08-12 layer-1 review, which listed the var-reuse construct
among "constructs 1-4 are legitimate").
C3 is ordinary C, no family claimed.  C4 ordinary C.

## T6 naming-announces-intent
No construct is named `pad`, `dummy`, `unused`, `spill`, `tail`, `slack`, or any
coercion-announcing name.  `idx` = slot index, `idx2` = halfword offset,
`wid` = word index, `mask` = occupancy bit, `val` = scratch, `last` = last
rand() result.  Every local is written and read; there are no address-of,
discard, or declaration-only uses.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) match device
  SCOPE: "`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation."
  PRECEDENT: docs/reference/sotn-construct-index.md:599
  FAMILY: variable reuse to defeat a loop.c invariant hoist
  SCOPE: "A movable must be set **once** in the loop (loop.c line ~705: `n_times_set==1 || consec_sets_invariant_p`). If the invariant lands in a pseudo that is **also assigned a *used* loop-variant value earlier** (non-consecutively), `n_times_set > 1` and it is **not a movable → not hoisted → recomputed inline.**"
  PRECEDENT: .claude/rules/defeat-licm-hoist-var-reuse.md:48

ANNOTATION-CONFORMANCE:
  C1 (src/text1b.c, at the construct site) -- inline block comment opening
  `/* FAKE: single-level do-while(0) wrap around the slot-index assignment.
  Mechanism: cc1's first-pass scheduler -- sched.c adjust_priority /
  birthing_insn_p (tools/gcc-2.7.2/sched.c:2505,2543). ... Lever-exhaustion: 16
  sessions, memory/grind/func_800645B0/hypotheses.md H24/H58/H63-H72 and
  evidence.md s9-s16 (49 banked rejected forms, floor flat at 1/78; the
  natural-geometry close-out was searched first and every spelling of it is
  banked dead). */` -- carries WHAT, MECHANISM (named GCC pass) and
  LEVER-EXHAUSTION (ledger pointer).
  C2 (src/text1b.c, immediately above `val = 1;`) -- inline block comment
  opening `/* FAKE: 'val' carries BOTH the const-1 shift source and the
  D_800A3444 read-modify-write. Mechanism: loop.c scan_loop / move_movables --
  a multi-set pseudo is not a movable (the n_times_set == 1 gate,
  tools/gcc-2.7.2/loop.c:705), so the constant 1 is recomputed inline each
  iteration instead of being hoisted into a callee-saved seat (+2 insns). */`
  -- carries WHAT and MECHANISM; its lever-exhaustion is the same 16-session
  ledger cited by C1.
