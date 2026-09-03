# Hypothesis ledger — func_8003D39C

## s1 (recon) — floor 39 (pinned, pre-migration cheat body) → 16 (pure C, v5 in candidate.c)
CONFIRMED  H-assoc: the array address must be two statements (`p = &D_800A3930[n]; p = (Sprt8Prim*)((u8*)p + (D_800A3218 << 9));`) — a single expression (with or without a (u8*) cast) is reassociated by fold to (idx<<9 + base) + n*16. Measured 26 → 27 (association fixed; other diffs opened).
CONFIRMED  H-bitfield-addprim: the OT link must be the OTag bitfield form `((OTag*)p)->addr = ot->addr; ot->addr = (u32)p;` (same as func_8003D330). Measured 25/27 → 16; whole tail byte-identical incl. a1/a2/a3 seats.
KILLED (instance) H-mask-local: `u32 rgb_mask = 0xFFFFFF` local with the manual and/or tail — reached 25 but is superseded by the bitfield form (masks come from the expander). Measured on v4 chassis, no FAKE.
KILLED (instance) H-incr-position: moving `D_800A3358 = n + 1;` after the address computation changes nothing (16 → 16, identical sched1 order). Measured on v5/v6, no FAKE.
KILLED (instance) H-luid-head: sched_solver depth-1 over all luid/luid_move atoms on the v5 pass-1 block-1 model — no statement reorder yields the target head; only add_dep/cost atoms do (gb_G2.txt, goal_d1.txt).

## Frontier for s2
1. Lengthen the priority chain of the n*16+base computation (needs INSN_PRIORITY(36) ≥ 2 so it out-ranks the sw at the stale sort) with an ordinary-C spelling that leaves no extra instruction after RA — e.g. the index passing through a coalescable copy (`s32 i = n;` used only in the index), a sub-word/typed index, or the array base taken through a typed pointer variable. Probe: recompile, `extract.py code6cac_c2`, check block-1 head picks; sandbox.
2. Alternative: make the sw of D_800A3358 not ready at the 42-pick sort by giving it a successor among {39,36,34,32}. Distinct plain symbols cannot conflict (sched.c:775), so this needs the count and the buffer-select to share an address base (one struct / one array) — only worth probing with base-register evidence per the aggregate-merge rule; otherwise dead.
3. Post-increment spelling `p = &D_800A3930[D_800A3358++]` (no local n at all; CSE keeps the value across the compare) — cheap probe, checks whether expand_increment's temp copy adds the missing chain insn (frontier 1 mechanism).
KILLED (instance) H-postinc (frontier 3, measured s1): `if (D_800A3358 == 0x20) return; p = &D_800A3930[D_800A3358++];` with no local n — sandbox 16, block-1 head byte-for-byte the same as v5 (expand_increment adds no chain insn; CSE keeps the loaded value). Measured on v7 = v5 chassis, no FAKE. Frontier 3 is closed; s2 starts at frontier 1/2.

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
