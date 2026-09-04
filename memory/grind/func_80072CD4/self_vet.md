# SELF-VET — func_80072CD4 (session s14d, structural, 2026-09-03)

Diff under vet: `src/text1b.c` — `INCLUDE_ASM("asm/funcs", func_80072CD4);` replaced by the
body of `memory/grind/func_80072CD4/candidate.c` (identical to
`tmp/grind/func_80072CD4/s14/q0_base.c` plus its three inline FAKE annotations; header
narration stripped by the apply script, per judge_constraints entry 13). No other file in the
build pipeline is touched.

Measurements taken THIS session with this exact body in place in `src/`:
  `sandbox func_80072CD4 --disable all` → **score 0, target_insns 79, build_insns 79,
  rules_dropped 0** (tmp/grind/func_80072CD4/s14d/sandbox_land.txt)
  `verify-oracle` → **ok: true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
  oracle, build_matches true** (tmp/grind/func_80072CD4/s14d/verify_oracle.txt)

CONSTRUCTS: (1) `int red;` — a fresh local holding the shared vertex red channel value,
assigned once and read by the two stores at offsets 4 and 0xC that follow the join;
(2) `u8 blue1;` — a fresh local carrying the vertex-1 blue channel value across the join,
given a different value on each path through the inner conditional and read once afterwards;
(3) three single-level `do { ... } while (0);` wraps, each carrying an inline `/* FAKE: ... */`;
(4) the green-channel store `*(u8 *)((s32)(arg1) + 5) = 0xC3;`, which occurs on each of the
two paths through the inner conditional — discussed in full under T1 and T5 below.

## T1 semantic purpose
(1) `red` carries the vertex-0/vertex-1 red channel value that the offset-4 and offset-0xC
stores receive; it is read twice and its value is in the emitted bytes — the target
materialises it exactly once, `addiu $v1, $zero, 0xFC` at 0x80072D24 (branch delay slot),
feeding `sb $v1, 0x4($s1)` at 0x80072D64 and the matching store at 0xC
(asm/funcs/func_80072CD4.s:22 and :40, read directly this session).
(2) `blue1` carries the vertex-1 blue channel value across the join — the two paths compute
different values (0x32 / 0x46, target 0x80072D44 and 0x80072D60) and the code after the join
consumes it into the offset-0xE store; without a variable the value cannot cross the join.
(3) The three wraps have no observable effect on the function's output values — they are pure
codegen devices, which is precisely what the do-while(0) sanction covers, and each says so in
its own inline FAKE annotation.
(4) The offset-5 store is a real, observable store of 0xC3 to `arg1[5]` on whichever path
executes. It is NOT byte-neutral and no cross-jump removes either copy: the target ships both
`addiu $v0, $zero, 0xC3` / `sb $v0, 0x5($s1)` pairs — 0x80072D28/0x80072D2C on the taken path
and 0x80072D48/0x80072D4C on the fall-through path (asm/funcs/func_80072CD4.s:23-24 and
:32-33, read directly this session). Those are 4 of the target's 79 instructions. Removing the
second source-level occurrence removes exactly those instructions — measured four ways in the
s14 q-probes: hoisted into the post-join do-while group = 12 at 77 insns; hoisted ahead of the
conditional = 7 at 77; placed after the join outside the wrap = 6 at 77; sourced from a holder
local assigned per path = 8 at 78 (tmp/grind/func_80072CD4/s14/sandbox_q1..q4*.txt). PASS.

## T2 human-programmer test
The function paints a 4-vertex gouraud primitive. A programmer writing it from the spec writes
the per-path colour components inside the path that needs them, and the component common to
the two paths once: which is what this body does — each path stores its own green (offset 5),
its own offset-6, its own offset-0xD, and leaves its own vertex-1 blue in a variable; the value
shared by the two paths (`red` = 0xFC) is stored after the join. `red` and `blue1` are ordinary
colour-channel names, not devices. The one thing a reader would ask "why is this here?" about
is the trio of do-while(0) wraps — which is why each carries a FAKE annotation naming its
effect, per that family's prerequisite. Everything else reads as plain primitive setup. PASS.

## T3 GCC-internals justification
The three do-while(0) wraps ARE justified by a named GCC mechanism, and their annotations state
it: the two per-path wraps hold the tail `blue1` constant load at the bottom of its path against
GCC 2.7.2 sched.c first-pass (sched1) hoisting, and the third wrap keeps the offset-4/0xC/0xE
stores at the head of the post-join block against the sched2 ready-store/potential-hazard
tiebreak. That is the required (b) prong of the FAKE template for a sanctioned family, not a
cheat signal — the do-while(0) rule expressly sanctions the device for ANY codegen effect.
For constructs (1), (2) and (4) the explanation is program logic, not a GCC pass: `red` and
`blue1` carry colour-channel values, and the offset-5 store is a store the target itself
performs on each path. PASS.

## T4 permuter / search provenance
No construct here is permuter output. The chassis is the cross-block form derived by s11/s12
from the target's own control flow; each wrap placement was chosen by direct sandbox
measurement of that placement (s14 probes p1-p10) and the offset-5 placement question was
settled by the q1-q4 probes cited under T1, all recorded in the ledger with their scores.
Nothing in the diff exists only because a detector fails to catch its spelling — the offset-5
store's presence on each path is declared explicitly in this vet, and its justification is the
target's own bytes, which I re-read this session rather than inheriting the claim. PASS.

## T5 family check
The one statement appearing at two source sites in this body is
`*(u8 *)((s32)(arg1) + 5) = 0xC3;`, at the tail of each mutually-exclusive path. It does not
fall in the byte-neutral duplicated-statement-into-arms family, whose defining property is that
cross-jump re-merges the copies so the second disappears from the output: here the second copy
IS in the output, at 0x80072D48/0x80072D4C, and the paths do not share a tail (the taken path
exits by `j .L80072D64` at 0x80072D40 with a distinct delay-slot constant). The construct
banned for this function as `dup4_0xc_into_arms` — the offset-4 and offset-0xC stores relocated
into the two paths as a merge-order lever — is absent from this body: each of those two stores
occurs at exactly one source site, after the join, fed by `red`, matching the target's single
`addiu $v1, $zero, 0xFC` at 0x80072D24. I verified that factual premise myself against
asm/funcs/func_80072CD4.s this session rather than resting it on any prior journal entry.
The three wraps are the sanctioned do-while(0) family, claimed below. PASS.

## T6 naming-announces-intent
`red` and `blue1` name the colour channels they carry. Neither is `pad`, `dummy`, `spill`,
`tmp`, `slack` or any other intent-announcing name; both are read (twice and once
respectively) and both values appear in the emitted bytes. No local array, no address-of, no
`(void)` discard, no unused declaration. PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) match device
  SCOPE: "`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation."
  PRECEDENT: .claude/rules/do-while-zero-exception.md:29

ANNOTATION-CONFORMANCE:
  /* FAKE: do-while(0) wrap, mechanism: GCC 2.7.2 sched.c first-pass scheduler (sched1) -
     without it the arm-tail blue1 constant load is hoisted to the arm top, which reseats it
     and cross-jumps the 0xD store out of the arm; lever-exhaustion:
     memory/grind/func_80072CD4/hypotheses.md (s2-s13, 36 banked forms, 15.8k-iteration
     directed permuter). */
  /* FAKE: do-while(0) wrap, mechanism: GCC 2.7.2 sched.c sched1 - keeps the arm-tail blue1
     constant load at the arm bottom (same effect as the then-arm wrap; both arms measured
     necessary, 7/79 and 8/79 with only one present); lever-exhaustion:
     memory/grind/func_80072CD4/hypotheses.md. */
  /* FAKE: do-while(0) wrap, mechanism: GCC 2.7.2 sched.c second-pass scheduler (sched2) - it
     separates these three stores into their own scheduling region so they keep the merge-block
     head instead of being sunk to the block tail by the ready-store/potential-hazard tiebreak;
     lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md (this wrap alone measures
     10/78, the arm wraps alone 4/79). */
  All three are single-level (none nested inside another), each sits at its construct site, and
  each carries what + a named GCC-pass mechanism + a lever-exhaustion pointer.
