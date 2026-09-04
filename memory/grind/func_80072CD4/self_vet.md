# SELF-VET — func_80072CD4  (session s14, rederive modality, 2026-09-03; re-measured and re-vetted 2026-09-03 after the prior run was discarded on self-vet citation FORMAT only — two FAMILY blocks with one SCOPE sentence; collapsed to the single family actually claimed)

Diff under vet: `src/text1b.c` — `INCLUDE_ASM("asm/funcs", func_80072CD4);` replaced by the body in
`memory/grind/func_80072CD4/candidate.c`. Measured THIS session with this exact body in place:
`sandbox func_80072CD4 --disable all` = **0**, build_insns 79 == target_insns 79, rules_dropped 0
(tmp/grind/func_80072CD4/s14/s14b_sandbox_final.txt); full-build `verify-oracle` = `"ok": true`,
build_sha1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle, `build_matches: true` (tmp/grind/func_80072CD4/s14/s14b_verify_oracle.txt).

CONSTRUCTS: (1) three sibling `do { ... } while (0);` wraps — one per inner arm around the `blue1`
assignment, one around the three merge-head stores @4/@0xC/@0xE; (2) `int red;` — a local holding the
0xFC value stored to @4 and @0xC; (3) `u8 blue1;` — a local holding the branch-selected value stored
to @0xE.

## T1 semantic purpose
(1) The wraps have no observable effect on the function's output — `do { X } while (0)` executes X
exactly once, which is what the unwrapped statements do. This is the defining property of the
do-while(0) family and the reason the family requires the FAKE annotation; it is not a claim that the
wraps are semantically load-bearing. Their codegen effect is measured and is stated below.
(2) `red` carries a value that lands in two of the function's output bytes (@4 and @0xC = 0xFC), so
it has semantic purpose: it is program data, not a codegen holder. (3) `blue1` carries the
branch-selected value that lands in output byte @0xE (0x32 / 0x46); it is the only variable through
which the two arms communicate their result to the code after the join, so it is load-bearing for
the program's meaning, not only for its bytes.

## T2 human-programmer
(1) No — a human writing this from a specification would not add the wraps; they are a match device.
That is exactly why they are annotated `/* FAKE: ... */` at each site and why the family sanction,
not naturalness, is what admits them. (2)/(3) Yes — naming a colour component that is written twice
(`red`) and a value chosen in one branch and consumed after the join (`blue1`) is how a human writes
a GPU-primitive setter. The COMPLETED-C sibling in this very file, `func_80072BC4`, uses precisely
this shape (`int fc_const = 0xFC;` plus a cross-block per-arm value).

## T3 GCC-internals justification
Yes for (1), and this is stated openly rather than hidden, because the do-while(0) sanction is
explicitly construct-level and mechanism-independent: `.claude/rules/do-while-zero-exception.md:29`
sanctions the wrap for ANY codegen effect and its prerequisite 1 asks for an effect-level annotation.
The observed effects, measured this session, are: the two arm wraps stop GCC 2.7.2's first-pass
scheduler (sched.c / sched1) from hoisting the arm-tail constant load to the arm top — the hoist
reseats that value and makes the arm tails identical, so jump2's cross_jump then lifts the `sb …,0xD`
out of the arms and the build lands at 78 insns; the merge wrap keeps the three merge-head stores in
their own scheduling region, so the second-pass scheduler's ready-store / potential-hazard tiebreak
(the "law" banked in s9) cannot sink them to the block tail. No (1)-style mechanism claim is made for
(2) or (3): those are ordinary program values.

## T4 permuter/search provenance
No permuter, solver or automated search produced this body. It was derived by hand this session from
the s9 forensics law (chain-independent stores in a scheduling region are always emitted in its tail)
plus the direct inference that the fix is to make the three stores their own region. Nine hand-written
variants were then measured to isolate which wraps are necessary (2/79, 2/79, 4/79, 7/79, 8/79,
10/78, 10/78, 0/79, 0/79); all are banked under `memory/grind/func_80072CD4/rejected/s14_*.c`.

## T5 family check
(1) is the do-while(0) family, which is on the FROZEN SOTN-accepted list and is separately ruled by
`.claude/rules/do-while-zero-exception.md` (owner ruling 2026-07-06). It is NOT the construct banned
for this function: `state.json` banned_constructs entry 5 bans identical @4/@0xC (and @5) stores
written into BOTH inner arms, and the standing 2026-07-24 judge constraint bans respelling the
@4/@0xC common-tail stores as a duplicated-into-arms store-schedule lever. This body duplicates
nothing: @4, @0xC and @0xE appear exactly once each, in the merge region, and the inner arms contain
only their own distinct per-arm values. The mechanism is also different in kind — the match here comes
from a scheduling-region split, not from a jump2 cross-jump of a duplicated arm tail. (2)/(3) are
ordinary C locals holding real, byte-materialised values, so no exception family is claimed for them.
Note on scope currency, per the brief's RULE SCOPE IS DATED clause: this function's s1 ledger records
a reviewer FAIL of an EMPTY `do { } while (0)` barrier form (`rejected/plus4_first_dowhile.c`) under
the 2026-06-04 mechanism-scoping. That scoping was abolished by the owner ruling of 2026-07-06, which
the rule file records at line 25-33 as superseding it; the wraps here are additionally non-empty.

## T6 naming-announces-intent
No name in the diff is `pad`, `dummy`, `unused`, `spill`, `tail`, `slack` or similar. `red` and
`blue1` name the POLY_G4 colour components they are stored into (@4/@0xC are r0/r1, @0xE is b1). The
earlier register-derived name `var_v0` was deliberately dropped before submission because it names a
machine register rather than a program value. Both locals are read; neither is dead, address-taken or
discarded.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) match device
  SCOPE: "`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation."
  PRECEDENT: .claude/rules/do-while-zero-exception.md:29 (rule text; shipped in-project application of the same family: commit cf3e6ce7)

ANNOTATION-CONFORMANCE: three inline annotations, one at each wrap site (not file-header prose), each
naming what / mechanism / lever-exhaustion:
  /* FAKE: do-while(0) wrap, mechanism: GCC 2.7.2 sched.c first-pass scheduler (sched1) - without it the arm-tail blue1 constant load is hoisted to the arm top, which reseats it and cross-jumps the 0xD store out of the arm; lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md (s2-s13, 36 banked forms, 15.8k-iteration directed permuter). */
  /* FAKE: do-while(0) wrap, mechanism: GCC 2.7.2 sched.c sched1 - keeps the arm-tail blue1 constant load at the arm bottom (same effect as the then-arm wrap; both arms measured necessary, 7/79 and 8/79 with only one present); lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md. */
  /* FAKE: do-while(0) wrap, mechanism: GCC 2.7.2 sched.c second-pass scheduler (sched2) - it separates these three stores into their own scheduling region so they keep the merge-block head instead of being sunk to the block tail by the ready-store/potential-hazard tiebreak; lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md (this wrap alone measures 10/78, the arm wraps alone 4/79). */
No wrap is nested inside another, so prerequisite 3 of the rule (nested-wrap justification) does not
apply; prerequisite 2 (prefer natural geometry first) is satisfied by thirteen prior sessions and 36
banked natural-geometry forms with a flat honest floor of 4.
