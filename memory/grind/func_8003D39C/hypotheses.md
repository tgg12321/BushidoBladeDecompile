# Hypothesis ledger â€” func_8003D39C

## s1 (recon) â€” floor 39 (pinned, pre-migration cheat body) â†’ 16 (pure C, v5 in candidate.c)
CONFIRMED  H-assoc: the array address must be two statements (`p = &D_800A3930[n]; p = (Sprt8Prim*)((u8*)p + (D_800A3218 << 9));`) â€” a single expression (with or without a (u8*) cast) is reassociated by fold to (idx<<9 + base) + n*16. Measured 26 â†’ 27 (association fixed; other diffs opened).
CONFIRMED  H-bitfield-addprim: the OT link must be the OTag bitfield form `((OTag*)p)->addr = ot->addr; ot->addr = (u32)p;` (same as func_8003D330). Measured 25/27 â†’ 16; whole tail byte-identical incl. a1/a2/a3 seats.
KILLED (instance) H-mask-local: `u32 rgb_mask = 0xFFFFFF` local with the manual and/or tail â€” reached 25 but is superseded by the bitfield form (masks come from the expander). Measured on v4 chassis, no FAKE.
KILLED (instance) H-incr-position: moving `D_800A3358 = n + 1;` after the address computation changes nothing (16 â†’ 16, identical sched1 order). Measured on v5/v6, no FAKE.
KILLED (instance) H-luid-head: sched_solver depth-1 over all luid/luid_move atoms on the v5 pass-1 block-1 model â€” no statement reorder yields the target head; only add_dep/cost atoms do (gb_G2.txt, goal_d1.txt).

## Frontier for s2
1. Lengthen the priority chain of the n*16+base computation (needs INSN_PRIORITY(36) â‰¥ 2 so it out-ranks the sw at the stale sort) with an ordinary-C spelling that leaves no extra instruction after RA â€” e.g. the index passing through a coalescable copy (`s32 i = n;` used only in the index), a sub-word/typed index, or the array base taken through a typed pointer variable. Probe: recompile, `extract.py code6cac_c2`, check block-1 head picks; sandbox.
2. Alternative: make the sw of D_800A3358 not ready at the 42-pick sort by giving it a successor among {39,36,34,32}. Distinct plain symbols cannot conflict (sched.c:775), so this needs the count and the buffer-select to share an address base (one struct / one array) â€” only worth probing with base-register evidence per the aggregate-merge rule; otherwise dead.
3. Post-increment spelling `p = &D_800A3930[D_800A3358++]` (no local n at all; CSE keeps the value across the compare) â€” cheap probe, checks whether expand_increment's temp copy adds the missing chain insn (frontier 1 mechanism).
KILLED (instance) H-postinc (frontier 3, measured s1): `if (D_800A3358 == 0x20) return; p = &D_800A3930[D_800A3358++];` with no local n â€” sandbox 16, block-1 head byte-for-byte the same as v5 (expand_increment adds no chain insn; CSE keeps the loaded value). Measured on v7 = v5 chassis, no FAKE. Frontier 3 is closed; s2 starts at frontier 1/2.

## [s1] The array address must be two statements (p = &D_800A3930[n]; p = (Sprt8Prim*)((u8*)p + (D_800A3218 << 9))) because fold reassociates the single-expression form
- mechanism: fold-const STRIP_NOPS + PLUS reassociation puts (idx<<9 + base) first; separate statements keep (n*16 + base) then + idx<<9 as the target does
- probe: v1/v2 vs v3 sandbox
- result: 26/26 -> 27 with association fixed, 0x74000000 lui in lw delay slot, n in v1
- verdict: CONFIRMED

## [s1] The OT link must be spelled with OTag bitfields (((OTag*)p)->addr = ot->addr; ot->addr = (u32)p;) like func_8003D330
- mechanism: store_bit_field/extract_bit_field generate the 0xFF000000/0xFFFFFF masks and load order that reload seats into a1/a2/a3 exactly
- probe: v5 sandbox + objdump diff
- result: 25/27 -> 16; every instruction from sb 3(a0) to jr identical
- verdict: CONFIRMED

## [s1] A u32 rgb_mask = 0xFFFFFF local with the manual and/or tail (v4, this chassis, no FAKE) reaches 25 but not the tail seats
- mechanism: const insn LUID moves the lui/ori to the block head only
- probe: v4 sandbox
- result: 25; superseded by the bitfield form
- verdict: KILLED
- kill_scope: instance
- measured_on: v4 pure-C chassis, no FAKE constructs

## [s1] Moving D_800A3358 = n + 1 after the address computation (v6, this chassis) changes nothing
- mechanism: sched1 head order is fixed by the ready-list stale sort, not by statement LUID
- probe: v6 sandbox + sched model
- result: 16, identical instruction stream
- verdict: KILLED
- kill_scope: instance
- measured_on: v6 = v5 chassis, no FAKE constructs

## [s1] Post-increment index p = &D_800A3930[D_800A3358++] with no local n (v7, this chassis) changes nothing
- mechanism: expand_increment emits no extra chain insn; CSE reuses the compare's load
- probe: v7 sandbox
- result: 16, identical head
- verdict: KILLED
- kill_scope: instance
- measured_on: v7 = v5 chassis, no FAKE constructs

## [s1] On the v5 pass-1 block-1 model no single luid/luid_move atom (statement reorder) produces the target head; sub-goal 'sll n,4 after sw' is satisfied only by add_dep 32/34/36<-28 or cost atoms
- mechanism: sched.c rank_for_schedule stale sort: 36 is a data pred of 42 (class 1) when the list is sorted at the 42 pick, so 28/26 are picked first; only a higher INSN_PRIORITY on the address chain or an unready 28 changes it
- probe: perturb.py --goal-order depth 1 (6047 atoms) + --goal-before 32:28 + simluid.py
- result: NO perturbation at depth 1 for the full order; G2 vectors are all add_dep/cost
- verdict: KILLED
- kill_scope: instance
- measured_on: v3/v5 pure-C chassis sched models, no FAKE constructs

## s2 (structural) â€” floor 16 -> 15 (v10 is the new candidate.c)

## [s2] The array address carried by TWO distinct pointer variables (u8 *q then Sprt8Prim *p) beats the one-variable-assigned-twice spelling
- mechanism: two pointer pseudos survive to sched1 as two separate address computations; the pick
  order of pass-1 block 1 changes from `42,40,28,26,39,142,141,36,34,32` to
  `42,40,36,39,34,32,28,26,142,141`, putting the mask lui/ori, the addiu and the sw at the head
  exactly as the target does. (The dumped scheduler INPUTS are identical between the two spellings,
  so the effect is upstream of the modelled atoms.)
- probe: v10 vs v5 sandbox + `extract.py code6cac_c2` pass-1 block-1 pick dumps
- result: 16 -> 15; the `mask; addiu; sw; sll; la; addu` prefix now matches in order
- verdict: CONFIRMED

## [s2] A single pointer variable doing the whole address arithmetic with a second variable as a pure copy (v22) does not reach 15 on this chassis
- mechanism: `q = (u8 *)&D_800A3930[n] + (D_800A3218 << 9); p = (Sprt8Prim *)q;` leaves one
  address pseudo after copy-coalescing, so the schedule is v5's
- probe: v22 sandbox
- result: 16 (banked rejected/single-pointer-var-one-expression-d16.c)
- verdict: KILLED
- kill_scope: instance
- measured_on: v22 pure-C chassis, no FAKE constructs

## [s2] Hoisting the OT-head load (ot = (OTag *)D_800A374C) into the declaration block costs 19 bytes
- mechanism: the load's LUID moves to the top of the block, the addPrim tail loses the register
  seats that made the whole tail byte-identical since v5
- probe: v11 sandbox
- result: 34 (banked rejected/ot-hoisted-before-guard-d34.c)
- verdict: KILLED
- kill_scope: instance
- measured_on: v11 pure-C chassis, no FAKE constructs

## [s2] On the v10 chassis (floor 15) no statement-order, declaration-order, named-intermediate, operand-order, type-narrowing or block-shape spelling moves the floor
- mechanism: the residual is decided by which pseudo the `n*16` temp is coalesced onto (n's own
  register vs the `n+1` register), which none of these levers touches
- probe: 14 measured spellings v15,v16,v18,v19,v20,v21,v23,v24,v25,v26,v27,v28,v29,v31 â€” all 15
- result: flat 15 across every one
- verdict: KILLED
- kill_scope: instance
- measured_on: v10 (floor-15) pure-C chassis, no FAKE constructs

## [s2] On the v5 chassis no SINGLE scheduler-input atom (6051 atoms: luid, luid_move, add_dep, del_dep, cost, unit) reproduces the target's full pass-1 block-1 pick order
- mechanism: reaching the target head needs both a priority lift on the n*16+base chain AND the
  39/36 rank flip; the depth-1 conjunction search finds 11 vectors satisfying the three key pairwise
  sub-goals (add_dep 34<-39, 34<-40, 36<-39, 36<-40, cost 32:=2/3, cost 34:=2/3/12) but none of
  them yields the full order
- probe: perturb.py --goal-order (full 45-insn target order) --depth 1 over all atoms, plus
  --goal-before 39:28 / 32:28 / 28:142 conjunction (tmp/grind/func_8003D39C/s2/gb_out.txt)
- result: NO single atom reaches the full order; the 11 conjunction vectors all leave 36/39 or 34/32
  transposed
- verdict: KILLED
- kill_scope: instance
- measured_on: v5 (floor-16) pass-1 sched model, no FAKE constructs

## Frontier for s3
1. RA-SOLVER, not scheduling. The residual is one coalescing choice: the target puts the `n*16`
   temp in the SAME hard register as `n + 1` (v0) so the WAR anti-dependence pins `sw v0` before
   `sll v0,v1,4`; ours puts it in n's own register (a0). Classify with
   `tools/ra_solver/inverse_compose.py classify` on the v10 chassis (local-alloc / reload models)
   and ask for the C-lever vector that moves the `n*16` temp off n's allocno.
2. The arg-copy seat rotation (x/y/color -> t0/t1/t2 in the target, t1/t2/t0 in ours) is the same
   allocator's output and should fall out of (1); measure it as a joint goal, not separately.
3. Pass-2 (post-reload) scheduling on the v10 chassis has NOT been modelled this session. The
   pass-1 order is already one transposition from the target while the FINAL order differs more,
   so `extract.py` + `simulate.py --pass 2` on v10 is a cheap unexplored read: it will say whether
   the sw sink is a sched2 decision that a cost/dep atom could flip, or purely the seat.

## [s2] The array address carried by two distinct pointer variables (u8 *q = (u8 *)&D_800A3930[n]; p = (Sprt8Prim *)(q + (D_800A3218 << 9));) beats the one-variable-assigned-twice spelling that s1 banked
- mechanism: two pointer pseudos survive to sched1 as two separate address computations; pass-1 block-1 pick order moves from 42,40,28,26,39,142,141,36,34,32 to 42,40,36,39,34,32,28,26,142,141, which puts the 0xFFFFFF mask lui/ori, the addiu n+1 and the sw D_800A3358 at the head exactly as the target does
- probe: v10 vs v5 sandbox --disable all, plus tools/sched_solver/extract.py code6cac_c2 pass-1 block-1 dumps on both chassis
- result: 16 -> 15; the mask/addiu/sw/sll/la/addu prefix is now in the target's order. NOTE the dumped scheduler INPUTS (dep graph, REG_NOTE kinds, LUIDs, INSN_PRIORITY) are identical between the two spellings, so the win is upstream of every atom perturb.py models.
- verdict: CONFIRMED

## [s2] A single pointer variable doing the whole address arithmetic with a second variable as a pure copy (q = (u8 *)&D_800A3930[n] + (D_800A3218 << 9); p = (Sprt8Prim *)q;) does not reach 15
- mechanism: copy-coalescing leaves one address pseudo, so the schedule is v5's
- probe: v22 sandbox --disable all
- result: 16; banked as memory/grind/func_8003D39C/rejected/single-pointer-var-one-expression-d16.c
- verdict: KILLED
- kill_scope: instance
- measured_on: v22 pure-C chassis, no FAKE constructs

## [s2] Hoisting the OT-head load (ot = (OTag *)D_800A374C) into the declaration block costs 19 bytes
- mechanism: the load's LUID moves to the top of the block and the addPrim tail loses the a1/a2/a3 seats that made it byte-identical from v5 onward
- probe: v11 sandbox --disable all
- result: 34; banked as memory/grind/func_8003D39C/rejected/ot-hoisted-before-guard-d34.c
- verdict: KILLED
- kill_scope: instance
- measured_on: v11 pure-C chassis, no FAKE constructs

## [s2] On the v10 floor-15 chassis, statement order, declaration order, named intermediates, operand order, type narrowing and if-block-vs-early-return all leave the score at 15
- mechanism: the residual is decided by which pseudo the n*16 temp is coalesced onto (n's own register vs the n+1 register); none of these levers touches that decision
- probe: 14 measured spellings v15 v16 v18 v19 v20 v21 v23 v24 v25 v26 v27 v28 v29 v31, each applied to src/code6cac_c2.c and scored with sandbox --disable all
- result: every one scored exactly 15; 15 is a broad plateau for the structural lever set
- verdict: KILLED
- kill_scope: instance
- measured_on: v10 (floor-15) pure-C chassis, no FAKE constructs

## [s2] On the v5 floor-16 chassis no single scheduler-input atom out of 6051 (luid, luid_move, add_dep, del_dep, cost, unit) reproduces the target's full pass-1 block-1 pick order
- mechanism: reaching the target head needs both a priority lift on the n*16+base chain and the 39/36 rank flip; the depth-1 conjunction search finds 11 vectors satisfying the three key pairwise sub-goals (add_dep 34<-39, 34<-40, 36<-39, 36<-40, cost 32:=2/3, cost 34:=2/3/12) but every one of them still leaves 36/39 or 34/32 transposed
- probe: tools/sched_solver/perturb.py --goal-order (full 45-insn target pick order) --depth 1 over all atoms; plus --goal-before 39:28 --goal-before 32:28 --goal-before 28:142 conjunction
- result: NO single atom reaches the full order; the 11 conjunction vectors are enumerated in tmp/grind/func_8003D39C/s2/gb_out.txt
- verdict: KILLED
- kill_scope: instance
- measured_on: v5 (floor-16) pass-1 sched model, no FAKE constructs

## [s3] The `D_800A3218 << 9` buffer displacement is the first subscript of a two-dimensional array `Sprt8Prim D_800A3930[2][32]`, not an addend
- mechanism: 1 << 9 = 512 = 32 * sizeof(Sprt8Prim); the symbol region 0x800A3930 ends exactly at the next symbol D_800A3D30 (+0x400 = 2 * 32 * 16), so the object is a double-buffered array of 32 SPRT_8 primitives and the natural C address is a single two-subscript expression
- probe: v32 â€” `extern Sprt8Prim D_800A3930[2][32]; p = &D_800A3930[D_800A3218][n];` applied to src/code6cac_c2.c, sandbox --disable all
- result: 15 -> 6. The entire address head becomes byte-identical, including the n=v1 seat and the `sw v0,D_800A3358` position that s2 had modelled as an RA coalescing choice; the only residual left was the parameter seat rotation.
- verdict: CONFIRMED

## [s3] The parameter hard-register seat rotation (x/y/color -> t0/t1/t2 vs t1/t2/t0) is decided by the SOURCE ORDER of the field stores that consume the parameters
- mechanism: global.c `allocno_compare` sorts allocnos by floor_log2(n_refs)*n_refs/live_length; x, y and color each have n_refs = 2, so live_length is the only discriminator. With `color` consumed by the packed-colour store two statements before x/y were stored, color had the shortest live range, sorted first, and took the first register in the allocation order (t0). Moving `p->x0 = x; p->y0 = y;` above the colour store makes x and y die first, the three priorities tie, and the qsort tie-break by allocno number restores declaration order 72(x),73(y),75(color) -> t0,t1,t2.
- probe: v36/v37/v39 (x0/y0 stores placed before the packed-colour store, before the clut store, and immediately after the code store) vs v32; the `;; 5 regs to allocate: 76 74 75 72 73` line read from tmp/grind/func_8003D39C/dumps/code6cac_c2.greg
- result: 6 -> 0 for all three placements. v38 (x0/y0 hoisted above `p->code = 0x74;`) is 2, so the placement window is bounded below by the code store.
- verdict: CONFIRMED

## [s3] The s2 mechanism story â€” that the floor-15 residual required coalescing the n*16 temp onto the n+1 allocno â€” described a correlation, not the cause
- mechanism: the sw position, the n=v1 seat and the addu operand order are all emitted correctly once the address is spelled as a two-dimensional subscript, with no coalescing lever present in the C at all; the pointer-variable spellings s1/s2 swept (v5..v31, v33, v34, v35) were all searching inside a wrong object model for D_800A3930
- probe: v32 (2D subscript) vs v33 (2D array but buffer pointer taken first) vs v34/v35 (2D array declared, address still hand-split) â€” sandbox --disable all
- result: v33/v34/v35 all score 15 exactly like the hand-split v10; only the single two-subscript expression moves it. So the lever is the SHAPE OF THE ADDRESS EXPRESSION, and the RA-coalescing framing predicted nothing.
- verdict: KILLED
- kill_scope: instance
- measured_on: v32/v33/v34/v35 pure-C chassis, no FAKE constructs
