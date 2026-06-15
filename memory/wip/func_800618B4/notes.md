# func_800618B4 — WIP (blocked-lane triage 2026-06-14)

## TL;DR
Text transform-block dispatch (text1b.c:0x800618b4). Carries 2 register pins
(t=$2, mask=$3) + 1 regfix swap ($2<->$3 @42-46). honest pure-C distance 4.
**Tail register-rotation cluster** shared by ~10 text1b siblings. NOT closed
this pass — variable-splitting moves FURTHER from target (extra scheduling).

## The exact gap (tail register rotation)
Target tail (near-sequential):
```
lw v0,0(s0)  ; sw v0,4416  (arg0[0] -> v0)
lw v0,4(s0)  ; sw v0,4420  (arg0[1] -> v0)
lw v1,8(s0)               (arg0[2] -> v1, loaded BEFORE mask)
lui v0,0xff  ; sw v0,920   (mask -> v0)
sw v1,4424                 (store arg0[2])
```
Without pins GCC puts all three loads in v0 (or a0) and REORDERS the schedule
(hoists loads, reorders stores) — the distinct-var split lands arg0[2] in v1
but with the wrong schedule. The pins + swap reproduce target's exact alloc.

## Resume steps (next session)
1. CLUSTER approach: crack the SIMPLEST sibling first (func_800611A4 /
   func_8006156C / func_80061250 — all share this register-rotation infra), then
   transfer the lever. A WIP already exists for several of them
   (memory/wip/func_800611A4 etc.).
2. Instrumented allocator dump (cc1 -da .greg + ALLOCDBG) on the tail: find the
   liveness shape that yields {arg0[2]=v1, mask=v0} WITHOUT pins. Crux: arg0[2]'s
   load must overlap the mask's lui but not take v0, while target keeps a
   near-sequential schedule (no hoist).
3. Source form that keeps arg0[2]'s load textually last (as target) AND
   preserves the sequential schedule — the naive distinct-var split breaks both.

## Ruled out (do NOT re-derive)
- HEAD body, pins+swap removed -> DIFF (load-bearing).
- distinct vars a,b,c,mask (no pins) -> DIFF, different regs + reorder.
- single t + distinct c (no pins) -> DIFF, a0 for loads, reordered.
- (prior card) drop register kw / +inline mask literal -> still 4;
  block-scoped t -> 10; preload arg0[2] -> 8-10; inline all arg0[N] -> 10.

## Pointers
- `.claude/rules/register-alloc-pure-c.md`, `memory/project/register-alloc-deep-dive.md`
- Cluster siblings (shared infra, several have WIP): func_800611A4, func_8006156C,
  func_80061658, func_80061710, func_800617C8, func_80061ACC, func_80061EC0.
