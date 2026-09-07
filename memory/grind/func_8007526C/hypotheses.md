# Hypothesis ledger — func_8007526C

## Settled

- **H1 (s1) CONFIRMED — index-derived cursor removes the loop.c giv bias.**
  `p = base + i * 2` computed inside the loop (rather than `p += 2` at the loop bottom)
  stops `loop.c` strength reduction from creating a `reg = p + 16` biv, so all field
  offsets stay absolute and the pre-header is a bare `lw a0,0(gp)`. Measured 48 -> 13.
  This form is banked as memory/grind/func_8007526C/candidate.c and is ordinary C.

- **H2 (s1) KILLED (instance) — re-reading `D_800A36A0` at every access.**
  Hypothesis was that source-level repetition of the global load would inflate the loop's
  RTL insn_count past loop.c's move_movables threshold. `cse` collapses the repeated loads
  before `loop.c` runs: insn_count went 91 -> 95 only, the four constants were still
  hoisted, and the score got WORSE (13 -> 25, 98 insns). Measured on HEAD 2026-09-07
  chassis with no FAKE constructs. rejected/global-reload-per-access-score25.c

- **H3 (s1) KILLED (instance) — `for (i=0;i<2;i++, p+=2)` vs `do {...} while`.**
  Both spellings of the pointer-increment loop measure exactly 48; the loop form is not
  the lever, the addressing form is. Measured on HEAD 2026-09-07, no FAKE constructs.

## Live frontier (in priority order)

- **F1 — inflate the loop's `scan_loop` insn_count above 122.**
  Mechanism: `loop.c:1631` moves an invariant iff
  `(threshold * savings * m->lifetime) >= insn_count`, with threshold = 2*(1+60) = 122
  for this (call-free) loop, savings = lifetime = 1 for each switch comparison constant.
  Our loop is 91 RTL insns at `scan_loop` time, so the four constants are hoisted; the
  target's are not. Any ordinary-C spelling that leaves >= 123 RTL insns inside the loop
  *at loop.c time* (i.e. surviving `jump` and `cse`, but freely collapsible afterwards by
  `combine` / `jump2` cross-jumping) would flip the predicate.
  Next probe: write the four case arms in a form that produces more pre-`combine` RTL —
  e.g. arms that duplicate a real statement (the sanctioned duplicated-statement-into-arms
  family, `.claude/rules/duplicated-statement-into-arms.md`) and are re-merged by `jump2`,
  or field accesses whose address arithmetic is not yet folded into the MEM at `cse` time.
  Read `Loop from N to M: K real insns` at the head of the per-function .loop dump after
  every variant — that number IS the predicate's insn_count and gives a direct gradient.

- **F2 — make the case comparisons not become movables at all.**
  Mechanism: the movable is only registered if the set's dest survives the gates at
  `loop.c:695-700`. For compiler-generated constant pseudos the second disjunct
  (`! REG_USERVAR_P (dest) && ! REG_LOOP_TEST_P (dest)`) is unconditionally true, so no
  ordinary switch spelling escapes there. The escape would have to change WHAT RTL the
  case comparison produces: a spelling where the comparison constant is folded into the
  instruction as an immediate (`sltiu`/`xori`/`slti`) leaves no pseudo to hoist.
  Next probe: measure an if/else-if chain and a range-shifted switch (e.g. switching on
  `x - 1`) and diff the resulting decision tree against the target's
  `beq $v1,$v0 / slti $v0,$v1,3 / beqz` bisection. NOTE: the target itself uses
  `addiu $v0,$zero,N; beq $v1,$v0`, so the constants ARE in registers there — this
  frontier can only win if some spelling both keeps the bisection tree and makes the
  constant pseudos non-invariant (e.g. one pseudo set more than once, which fails
  `n_times_set[regno] == 1` at loop.c:707).

- **F3 — check the sibling `func_80074B18`.**
  It is the other consumer of `D_800A36A0` (memory/grind/func_80074B18/retired-chassis-2026-08/body.c)
  and has the same 2-slot / stride-2 shape. If it has ever been matched, or if its ledger
  records a switch-in-loop constant-hoist finding, the spelling transfers directly
  (see [[sibling-ledger-propagation]] — a foreclosed sibling holding the shared window's
  fix is a known 41-session failure mode in this repo).

## [s2] Deriving the loop cursor from the index inside the loop (p = base + i*2) instead of bumping a pointer (p += 2) stops loop.c strength reduction from creating a base induction variable biased by +0x10, restoring the target's absolute field offsets and bare 'lw a0,0(gp)' preamble.
- mechanism: GCC 2.7.2 loop.c strength_reduce/combine_givs. With an explicit pointer bump, reg 72 is the biv and the first in-loop address (p+0x10, the lbu feeding the switch) is chosen as the combined giv base, emitting (set (reg 130) (plus (reg 72) (const_int 16))) in the pre-header and shifting every MEM offset by -0x10. With an index-derived cursor the giv over biv i has add_val 0, so the pre-header initialiser is the plain global load.
- probe: Wrote both spellings into src/text1b.c and measured with `wteng main sandbox func_8007526C --disable all`; read tmp/grind/func_8007526C/dumps/text1b.loop ('Insn 219: dest address src reg 72 ... add 16' and insn 281) for pass attribution; objdump-diffed both builds against asm/funcs/func_8007526C.s.
- result: Pointer-bump form measures 48 (build 94 insns) with every offset biased by -0x10; index-derived form measures 13 (build 93 insns) with every offset, block order, register seat and delay slot matching the target. Banked as memory/grind/func_8007526C/candidate.c.
- verdict: CONFIRMED

## [s2] Re-reading the global D_800A36A0 at every field access, with no local cursor, inflates the loop's RTL insn_count past loop.c's move_movables threshold and keeps the switch comparison constants in the loop.
- mechanism: loop.c:1631 moves an invariant iff (threshold * savings * lifetime) >= insn_count; the idea was to raise insn_count above threshold = 2*(1+n_non_fixed_regs) = 122 by multiplying the source-level global loads.
- probe: Rewrote all 24 accesses as *(u16 *)(D_800A36A0 + i * 2 + OFFS), measured the sandbox score, and read the insn_count line at the head of the regenerated .loop dump.
- result: cse collapses the repeated global loads before loop.c runs: 'Loop from 11 to 622: 95 real insns' (up from 91, still far below 122), all four constants still 'moved to' the pre-header, and the score regressed from 13 to 25 (98 insns). Saved to rejected/global-reload-per-access-score25.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD f43e0467 chassis 2026-09-07, pure C, no FAKE constructs present, score 25

## [s2] The loop form itself (for (i=0;i<2;i++, p+=2) versus i=0; do {...} i++; p+=2; while (i<2)) is the lever behind the 48-point residual.
- mechanism: Front-end loop-note placement and where the increment lands relative to NOTE_INSN_LOOP_CONT could change which register loop.c picks as the biv.
- probe: Measured both spellings of the pointer-bump body with `wteng main sandbox func_8007526C --disable all`.
- result: Both measure exactly 48 with build 94 insns and identical output; the loop syntax is neutral here. The addressing form, not the loop form, carries the residual.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD f43e0467 chassis 2026-09-07, pure C, no FAKE constructs present, score 48 for both spellings
