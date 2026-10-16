# Hypothesis ledger — func_80052C10

## s1 frontier (recon)

1. **Register-seat recovery ($v0 -> $t9).** Mechanism: global register allocator
   (global.c) allocno priority / preference order — find what in a plausible
   fuller original body (more locals/calls before this store) would push the
   address pseudo past $v0..$t8 to $t9, without a pin or dead store. Probe:
   dump `.greg`/`.lreg` for the minimal body (`cc1 -da`, `tools/gcc-2.7.2/cc1`)
   and compare against a synthetic body with extra live scratch values: does
   adding genuine (non-dead) prior computation shift the allocation? If no
   natural-C body of any shape reaches $t9, that's evidence for the hand-written
   side despite the LOW scan_hand_coded tier.
2. **Trailing double-nop.** Mechanism unknown — not inter-object padding (next
   function starts immediately after, no gap), not GTE/cop2 latency (no cop2 op
   here). Probe: check sibling single-store scratchpad-poke functions elsewhere
   in the codebase (grep other `lui+ori+sw+jr+nop+nop` 6-insn leaf shapes) for
   a matched C analog; check `-da` `.sched`/reorg dumps for why a second nop
   would ever follow `jr $ra` in a leaf void function (delay-slot filling never
   fills past the one slot — a second nop has to come from source, e.g. two
   statements where the second is fully optimized away, which is itself a
   likely FAKE/dead-store situation and needs scrutiny under
   [[dead-store-fake-exception]] if pursued).
3. **Re-weigh the canonical-asm grant itself.** The grant's stated mechanism
   (combine.c always folds the offset) is measurably false for this address;
   scan_hand_coded is LOW/0. If (1) and (2) cannot be closed without a cheat
   construct after a real attempt, the right move is a `ruling-request` citing
   this evidence (not a unilateral revert of the 2026-06-07 grant) — the
   reopen_reason explicitly asks the Grinder to decide afresh, but a grant
   revocation is itself owner-relevant.

## s1 — kill + refined frontier (this session)

**KILLED (instance):** "A single-statement body `*(T *)0x1F800400 = 0;` (volatile
or plain) with no other live state reaches the target's `$t9` register seat."
Measured: `.greg` dump (`tmp/grind/func_80052C10/s1/3AB48.greg.txt`) shows the
volatile form compiles to exactly one pseudo (reg 72, zero conflicts), which
global.c allocates to `$v0` (lowest free scratch reg) every time, per
[[register-alloc-pure-c]]'s stated allocation order. This is an INSTANCE kill
on the single-statement chassis (both `vu32` and plain `s32` pointer forms
tried, no FAKE construct present) — it says nothing about whether SOME larger
body with real competing live state could reach `$t9`; that remains open
(frontier item 1 below).

Revised frontier for session 2:
1. **Register-seat recovery via genuine multi-value pressure.** To reach `$t9`
   needs ~13 other simultaneously-live scratch pseudos in-function — no
   caller or sibling evidence (checked this session: every call site is a
   true parameterless `void(void)` sink) suggests the original body had any
   such content. Next probe: construct a synthetic body with that much real
   pressure purely to test the allocator's behavior (diagnostic only, never
   landed) and confirm/deny $t9 is reachable at all before concluding "no
   natural single-store body reaches it" more broadly than this session's
   single-statement instance.
2. **Trailing double-nop** — unchanged from s1, still unexplained, still
   flagged source-level (no register lever reaches it).
3. **Re-weigh the canonical-asm grant.** New data this session CUTS BOTH WAYS:
   the grant's "combine.c always folds" is right for the non-volatile reading
   (score-4, folded-displacement form) and wrong only for the volatile
   reading; AND the immediately-next sibling function
   (`func_80052C28.s`) carries an independent, already-annotated
   `/* handwritten instruction */` marker, which is affirmative evidence FOR
   hand-written asm in this cluster, cutting against reopening the grant
   on scan_hand_coded's LOW/0 tier alone. Recommend NOT escalating a
   ruling-request yet — gather the item-1 probe first; if $t9 proves
   unreachable even under heavy synthetic pressure (a class-level finding,
   citable against [[register-alloc-pure-c]]'s ordering predicate), that is
   the point to ask.

## s2 (structural) — frontier item 1 resolved: $t9 IS reachable, but only at prohibitive cost

**CONFIRMED:** "$t9 is reachable by genuine (non-pinned, non-dead) C register
pressure for the scratchpad-address pseudo in func_80052C10's chassis."
- mechanism: global.c's register allocator has no `REG_ALLOC_ORDER` override
  for MIPS (confirmed: `grep REG_ALLOC_ORDER tools/gcc-2.7.2/config/mips/*` —
  no hits), so `global.c:1079`'s `#ifndef REG_ALLOC_ORDER` default path applies:
  ascending hard-register-number preference, i.e. $v0(2),$v1(3),$a0-$a3(4-7),
  $t0-$t7(8-15),$s0-$s7(16-23), THEN $t8(24),$t9(25). Reaching $t9 needs 22
  other hard regs already claimed by simultaneously-live pseudos — not ~13 as
  the s1 frontier estimated (that number omitted $s0-$s7, which sit BEFORE
  $t8/$t9 in the default order despite being callee-saved).
- probe: synthetic diagnostic chassis (never landed, tmp/grind/func_80052C10/s2/gen2.py):
  `target` pointer declared first (so its live range spans the whole body),
  N other `vu32 *pK = (vu32*)0x1F80....` pointers declared after it, all N
  dereferenced-and-stored BEFORE `*target = 0;` (forces target's live range to
  overlap all N). Swept N = 5,9,13,15,20,22,23 via
  `tools/grinder/dump.ps1 func_80052C10 -Candidate <file>` + `.greg` dispositions.
  Dumps saved: tmp/grind/func_80052C10/s2/{p5,p9,p13,q13,q15,q20,q22,q23}.greg.
- result: reg72 (the `target` pseudo, pseudo-id of the first-declared, longest-lived
  local) tracked across N: N=13→hard reg 16 ($s0); N=15→18 ($s2); N=20→23
  ($s7, last of the s-bank); N=22→**25 ($t9)**, confirmed via `sandbox
  --candidate tmp/grind/func_80052C10/s2/q22.c --diff`: score 106, 110 build
  insns vs target's 6, with a `addiu $sp,$sp,-32` prologue (saved in
  tmp/grind/func_80052C10/s2/q22_sandbox_diff.txt) — the s0-s7 saves this
  pressure level requires. N=23→30 (an even higher reg, past t9, since by then
  t9 itself is also claimed by an earlier-order competitor).
- verdict: CONFIRMED (not a kill — this is the positive mechanism finding the
  s1 frontier asked for).

**Practical corollary (separate hypothesis, instance-scope):** "A body whose
total instruction count stays close to the target's 6 insns can reach $t9 via
this genuine-pressure mechanism."
- measured_on: the gen2.py pointer-pressure chassis, N=5..23, no FAKE construct,
  no register pin; swept exhaustively over the only mechanism that reaches $t9
  (ascending default order forces $s0-$s7 saturation first).
- result: KILLED on this chassis — every N that reaches $t9 (N>=22) necessarily
  also claims all of $s0-$s7, which under MIPS/PsyQ calling convention forces a
  non-trivial save/restore prologue+epilogue (measured: 110 insns at N=22,
  `addiu $sp,$sp,-32` + corresponding restores, vs the target's 0-insn
  prologue). No N in the swept range reaches $t9 without that overhead — the
  mechanism that reaches $t9 and the mechanism that keeps the body near 6 insns
  are mutually exclusive on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: gen2.py pointer-pressure chassis (declare-target-first,
  dereference-others-before-target shape), N=5,9,13,15,20,22,23, no FAKE
  construct, no register pin, measured via `.greg` dispositions +
  `sandbox --candidate --diff`.

This sharpens s1's open register-seat-anomaly gap: the target's $t9 choice is
mechanically *possible* in GCC 2.7.2 but demands the kind of register pressure
(22+ live scratch values, s0-s7 saved) this function's actual asm shows no
sign of (no save/restore insns at all in the 6-insn target). That is evidence
*against* a small natural-C original reaching $t9 here, and so *for* the
canonical-asm (hand-written) reading already supported by the sibling
func_80052C28 marker (s1 evidence) — not conclusive (only this one chassis
shape was swept; a differently-shaped body might reach pressure without using
$s0-$s7, e.g. one with 22 live values that are all call-clobbered temporaries
with no calls — untested), but it meaningfully narrows the frontier.

## [s1] A single-statement body `*(T *)0x1F800400 = 0;` (volatile vu32 or plain s32 pointer form) reaches the target's $t9 register seat for the scratchpad address pseudo.
- mechanism: global.c register allocation order: lowest-numbered free scratch register is preferred absent competing live pseudos.
- probe: .greg dump for func_80052C10 (tmp/grind/func_80052C10/s1/3AB48.greg.txt) on the vu32 candidate body; confirmed only one pseudo (reg 72, zero conflicts) exists, allocated to $v0.
- result: Both the vu32 and plain s32 single-statement forms allocate the address pseudo to $v0 (lowest free scratch reg), never $t9. Reaching $t9 would need ~13 other simultaneously-live scratch pseudos occupying $v0..$t8 first, which no caller or sibling evidence suggests ever existed in this body.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main/3AB48.c func_80052C10, single-statement chassis `*(vu32 *)0x1F800400 = 0;` and `*(s32 *)0x1F800400 = 0;`, no FAKE construct present, no register pin used

## [s2] $t9 is reachable by genuine (non-pinned, non-dead) C register pressure for the scratchpad-address pseudo in func_80052C10's chassis.
- mechanism: global.c's register allocator has no REG_ALLOC_ORDER override for MIPS (grep confirmed), so the #ifndef REG_ALLOC_ORDER default path (tools/gcc-2.7.2/global.c:1079) applies: ascending hard-register preference $v0,$v1,$a0-$a3,$t0-$t7,$s0-$s7,$t8,$t9 -- 22 registers must be claimed by other simultaneously-live pseudos before $t9 is chosen.
- probe: gen2.py pointer-pressure chassis (target pointer declared first so its live range spans the body, N other vu32* pointers declared after and dereferenced before target's own use to force overlap); swept N=5,9,13,15,20,22,23 via tools/grinder/dump.ps1 -Candidate + .greg dispositions.
- result: CONFIRMED: at N=22 other live pointers, the target pseudo (reg72) is allocated hard reg 25 ($t9), matching the target asm's register choice. Verified via sandbox --candidate q22.c --diff (score 106, 110 build insns, addiu $sp,$sp,-32 prologue for the s0-s7 saves this pressure level requires).
- verdict: CONFIRMED

## [s2] A body whose total instruction count stays close to the target's 6 insns can reach $t9 via this genuine-pressure mechanism.
- mechanism: n/a -- this is the practical corollary, not a new codegen mechanism: the same ascending-order allocator that reaches $t9 at N>=22 also forces $s0-$s7 into use, which the PsyQ/MIPS calling convention requires saving/restoring across the function body.
- probe: Same gen2.py sweep, N=5..23, reading build-insn counts from sandbox --candidate --diff at each reachability threshold.
- result: KILLED on this chassis: every N that reaches $t9 (N>=22) also claims all of $s0-$s7 and incurs a non-trivial save/restore prologue+epilogue (110 insns at N=22 vs the target's 6, with addiu $sp,$sp,-32 -- target has zero prologue insns). No swept N reaches $t9 without that overhead; the mechanism that reaches $t9 and the mechanism that keeps a body near 6 insns are mutually exclusive on this chassis shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: gen2.py pointer-pressure chassis (declare-target-first, dereference-others-before-target shape), N=5,9,13,15,20,22,23, no FAKE construct, no register pin, measured via .greg dispositions + sandbox --candidate --diff
