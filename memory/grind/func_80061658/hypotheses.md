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
- [s2] Grouping-preserving reorders of the single-temp form (mask mid, decl-order
  swap) — all floor 9. Pure reorder/decl-order cannot flip the RA tiebreak.
- [s2] Outer/multi load splits (load1, loads1&3, 3-scoped, t0/t1/t2) — floor 11.
  Multiple simultaneously-live single-use temps scatter to v0/v1/a0/a1 and break the
  interleave. rejected/split-outer-loads-scatter.c.

## s2 CONFIRMED / PROGRESS
- [s2] Lever A (block-local split of the MIDDLE load) lowers floor 9 -> 7.
  mechanism: mask's RMW-chained sets make it a LOCAL quantity that grabs v0 before the
  DISJOINT-range global load-temp t; a block-local `u` for load2 is born before mask,
  takes v0, and pushes mask to v1 (target-correct) — but the shared t (loads 1&3) then
  scatters to a0 and the scheduler hoists load2, distorting the interleave. Net 7.
  verdict: CONFIRMED (floor improved; best 7-form saved to candidate.c).

## s2 frontier (for next session)
1. Directed permuter over the tail RA (frontier #2, PERMUTER modality): base from the
   floor-9 pure-swap form (correct schedule, only RA differs) — cleaner base than the
   7-form. Build clean single-fn target from asm/funcs/func_80061658.s at offset 0.
   The specific unmet need: force the DISJOINT-range shared load-temp to win v0 over the
   RMW-chained LOCAL mask while keeping the single-temp interleave.
2. If permuter also plateaus, the residual is a local-vs-global allocation-class wall
   (register-rotation family); escalate per policy (NOT owner-gated yet — floor still
   dropping, still grindable).

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

## [s2] The s1 v0<->v1 swap is caused by mask winning v0 in local_alloc (which runs before global_alloc) because its two sets (li+ori) are RMW-chained into one contiguous quantity, while the load-temp t's three disjoint load ranges force it to global_alloc where it gets leftover v1.
- mechanism: GCC 2.7.2 local-alloc combines only chained/contiguous sets into a quantity; local_alloc runs first and seizes v0. t's 3 independent loads are disjoint ranges -> not combinable -> deferred to global_alloc -> v1.
- probe: Clean cc1 -da dump from current src (tmp/grind/func_80061658/s2/dumps/cur.i.lreg + cur.i.greg): pseudo 76=mask (li insn105 + ior insn106, RMW), pseudo 75=t (loads insn76/81/92, disjoint); disposition + '6 regs to allocate' list show t global, mask local.
- result: mask local->v0, t global->v1; exactly the 9-insn pure swap.
- verdict: CONFIRMED

## [s2] Lever A (block-local split of the middle load) forces mask off v0 onto v1 (target-correct) and lowers the honest floor from 9 to 7.
- mechanism: A block-local `u` for load2 is a local pseudo born before mask; local_alloc gives u v0 and mask the next free reg (v1). But the shared t (loads 1&3) stays a disjoint-range global pseudo -> scatters to a0, and u (no anti-dep) lets the scheduler hoist load2 -> interleave distorted.
- probe: Edited src to `{ s32 u = arg0[1]; D_800F1144 = u; }`, sandbox --disable all = 7; emitted tail (cur.s): load1@a0, load2(u)@v0, mask@v1, load3@a0, load2 hoisted.
- result: Floor 9 -> 7; mask now correctly in v1; best 7-form saved to candidate.c.
- verdict: CONFIRMED

## [s2] Grouping-preserving reorders and declaration-order changes of the single-temp form cannot flip the RA tiebreak.
- mechanism: Pseudo birth follows statement/use order, not decl order; the constant mask's live range is scheduler-pinned, so reordering its store is inert; the local/global classification is unchanged.
- probe: Sandbox: mask between load1/load2 = 9; decl-order swap (mask decl first) = 9; [s1 banked] mask-early=9, mask-late=9, mask-before-call=9.
- result: All floor 9 - no movement.
- verdict: KILLED

## [s2] Splitting loads other than the middle one reaches the match.
- mechanism: Multiple simultaneously-live single-use load temps have no anti-deps, so the scheduler hoists all loads and the temps scatter across v0/v1/a0/a1, breaking the target interleave.
- probe: Sandbox: split load1 = 11; split loads 1&3 = 11; 3 scoped block-locals = 11; 3 named temps t0/t1/t2 = 11; split load3 = 8; only split load2 = 7.
- result: All non-middle splits WORSE (8-11); only the middle-load split improves.
- verdict: KILLED
