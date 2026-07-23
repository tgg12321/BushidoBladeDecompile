# Hypothesis ledger — func_80061658

Gap = pure v0<->v1 swap: unpinned GCC puts load-temp `t`->v1 and mask const->v0;
target wants t->v0, mask->v1. Scheduling is byte-identical. Root cause: global.c
allocno priority — mask's short scheduler-fixed live range outranks t's one long
reused-variable live range, so mask grabs v0 first. Goal: make `t` win v0 (raise
t's priority / shorten its live range) WITHOUT touching the mask (mask is a
constant; its placement is inert, KILLED s1).

## Frontier (mechanism-grounded, for drill)
1. **Shorten t's live range so its allocno beats mask's for v0.** t is one reused
   variable => one long allocno. Splitting into t0/t1/t2 was measured 11 (worse)
   — but WHY 11 was never captured in RTL. Probe: split t, dump greg, check
   whether the 3 short webs reuse v0 (target) or scatter to v1/a0 (the likely
   cause of the +2). If they scatter because the scheduler makes two loads
   simultaneously live, try ordering that keeps each load dead before the next.
2. **Directed permuter on the tail with a CLEAN single-function target**
   (asm/funcs/func_80061658.s + prelude, offset 0 per difficult-is-not-impossible
   §3). Search RA-steering structures for the tail 3-load + const-store block.
   Vet any closing form against the cheat catalog before proposing.
3. **Verify the winning form against a sibling** (func_80061710/617C8/618B4/
   611A4/6133C share the pattern) — a real pure-C fix should generalize.

## KILLED
- mask-before-call (cross-call to force mask->v1): const rematerialized after
  call; floor 9. rejected/mask-before-call.c.

## [s1] The honest floor-9 residual is a pure v0<->v1 register swap in the tail, not a scheduling/interleaving difference as the prior WIP claimed.
- mechanism: objdump of pin-stripped build vs asm/funcs shows identical instruction order/interleaving; only the register names differ: target puts load-temp t in v0 and mask 0x10FFFF in v1, unpinned GCC does the reverse. All 9 differing insns are the v0/v1 tail insns.
- probe: mipsel objdump of tmp/sandbox .o tail vs asm/funcs/func_80061658.s; RTL greg dump (np.i.greg) shows load-temp in reg3(v1), mask const in reg2(v0).
- result: 9 differing insns = the swap; scheduling identical.
- verdict: CONFIRMED

## [s1] Unpinned GCC swaps because global.c allocno priority ranks the mask const above the reused load-temp t for v0.
- mechanism: reg_alloc_order puts $2(v0) before $3(v1); priority=log2(n_refs)*freq/live_length. t is one reused C variable => single long allocno (6 refs, ~9-insn span) with LOWER priority than mask's short scheduler-fixed live range (~3 refs, ~5-insn span), so mask is allocated first and grabs v0.
- probe: greg conflict/disposition dump np.i.greg (5-6 regs to allocate; load-temp->reg3, mask->reg2).
- result: Priority ordering explains the observed swap.
- verdict: CONFIRMED

## [s1] Placing mask=0x10FFFF before the func_80060A68() call forces mask to cross the call (call clobbers v0), pushing mask off v0 into v1 and freeing v0 for t (= target allocation).
- mechanism: A value live across a call conflicts with the call-clobbered v0, so global.c must avoid v0 for it.
- probe: Edited src (pins removed, mask assign moved before call), sandbox --disable all; disassembled tail.
- result: Floor still 9; disasm shows GCC rematerializes the constant (lui+ori) AFTER the call, so mask never crosses it; tail still lw v1 (t->v1). Corollary: any source reorder of a constant mask is inert (scheduler pins its live range to just before its single use) -- explains prior WIP's 3 mask-move variants all being 9.
- verdict: KILLED
