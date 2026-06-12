---
name: switch-break-shared-return-sched-hoist
description: Per-case `return 0;` in a switch makes sched1 hoist the v0-set into a load-delay slot (RA conflict → wrong RMW register) and flips a case's branch polarity; write `break;` + one shared trailing `return 0;` instead
paths: ["regfix.txt"]
# broad src/*.c glob removed 2026-06-11: surfaced via codegen-technique-index
---

# Per-case `return 0;` in a switch — sched1 hoists the v0-set into a load-delay slot; write `break;` + shared trailing `return 0;`

## Symptom

A switch-dispatch function carries a small regfix cluster with TWO seemingly
independent diff families:

1. **Branch polarity flip** on a case's call-result test (4 diffs: `bne`↔`beq`
   + swapped delay-slot fills + swapped j targets):
   ```
   sand:  bnez v0,done;  li a0,3;       j end;  move v0,zero
   canon: beqz v0,end;   move v0,zero;  j done; li a0,3
   ```
2. **Register choice on a global RMW** (`lbu/addiu/sb` cluster lands in `$v1`,
   target uses `$v0`; one `$2 <-> $3` rename rule).

The C body ends every "success" case with its own `return 0;` and there is
(or could be) a trailing post-switch `return 0;`. Target's tail shape is the
tell:

```mips
.LA:  addu $v0, $zero, $zero     # ONE shared return-0
.LB:  lw $ra, 16(sp); addiu $sp; jr $ra
```
with every case ending `j .LB; move v0,zero` (or `beqz ..., .LB; move v0,zero`)
— the `move v0,zero` sits in the DELAY SLOT and the jump targets the epilogue
PAST the shared `addu v0,zero,zero`.

## Mechanism (verified via instrumented cc1, func_8003AB44 session 15)

That target tail shape is reorg.c's **steal-from-jump-target-thread** pattern:
at RTL time each case ended with a bare `jump .LA` (no v0-set in the case
block), .LA held the ONE shared `(set v0 0)`, and reorg's delay-slot filler
duplicated it into each jump's delay slot, retargeting the jump past it to .LB.
That only happens when the C source uses **`break;` + one shared trailing
`return 0;`**.

With per-case `return 0;` instead, cc1 expands a per-case `(set (reg v0) 0)`
inside EVERY case block. Two cascades follow:

1. **The RMW register loss.** In a case whose body is a global byte RMW
   (`D_xxx++` → `lbu; addiu; sb`), sched1 fills the lbu's MIPS1 load-delay
   hazard slot with the only independent insn in the block — the `v0 = 0`.
   That makes hard `$v0` live ACROSS the RMW pseudos at global.c time →
   conflict → the RMW lands in `$v1`. (In the final asm the v0-set migrates
   back down into the j's delay slot, so the emitted ORDER looks identical to
   target — the damage is invisible except as the register rename. The
   conflict happens at RA time, in the sched1 ordering.)
2. **The polarity flip.** For `if (call() == 0) return 0; goto done;`, jump.c
   makes the per-case return-0 block the fall-through and inverts the branch
   (`bnez done`). With `break`, the test compiles to a conditional jump
   straight to the shared post-switch label (`beqz .Lpost`) — no inversion —
   and reorg steals the shared `move v0,zero` into ITS delay slot too,
   producing target's exact `beqz ...,.LB; move v0,zero`.

One `return 0;` → `break;` substitution fixes BOTH diff families at once.

## The fix

```c
switch (state) {
case 0:  ...; break;                 /* was: return 0; */
case 1:  ...; break;                 /* was: return 0; */
case 2:  if (call() == 0) break;     /* was: return 0; */
         goto done;
...
case 7:  ...; return 1;              /* non-zero returns stay early returns */
fail:    ...; return -1;
}
return 0;                            /* the ONE shared return-0 */
```

Early returns with NON-zero constants (`return 1;` / `return -1;`) stay as-is
— only the paths returning the same value as the trailing return become
`break`.

## Diagnosis recipe (when you suspect this)

- Target tail has the `.LA: addu v0,zero,zero / .LB: epilogue` shared pair
  with per-case `j .LB; move v0,zero` delay slots → strong signal.
- Decisive: `tmp/gccdbg/cc1` (instrumented GCC 2.7.2) with `-da` on an
  isolated `.i`; read the `.sched` dump — if the per-case `(set v0 0)` insn
  sits BETWEEN the RMW's load and its consumer, this is the mechanism.
  `BB2_FLOW_DEBUG=1` (flow.c hook, added 2026-06-10 alongside ALLOCDBG /
  SCHEDDBG / PRIODBG) dumps converged `basic_block_live_at_start/end` + CFG
  edges per block — useful to RULE OUT cross-block live-in theories quickly.

## Confirmed case — func_8003AB44 (code6cac_c_ab.c, 2026-06-10)

15 sessions, floor stuck at 6 (29 rejected structural forms, 33k permuter
iters, ALLOCDBG diagnostics). Sessions 4–14 attributed the case 5/6 register
diff to flow.c live-set propagation at the multi-entry jtbl join — **that
theory was WRONG**: the BB2_FLOW_DEBUG dump showed `$v0` live-in ONLY at the
shared return block, nowhere near the case blocks, and "pseudo 94" from the
session-4 ALLOCDBG was actually the switch dispatch index, not the increment.
The real conflict was sched1's load-delay fill (above). `return 0;` → `break;`
on the seven return-0 cases: sandbox 6 → 0 first try, `retire` dropped all 5
rules, SHA1 == oracle, COMPLETED-C.

Lesson for long WIP histories: a multi-session mechanism theory that has only
INDIRECT evidence (inference from one allocno's ord) can misdirect a dozen
sessions of levers. The direct dump (sched1 insn order + per-block live sets)
falsified it in one run. Prefer direct instrumentation early.

## When this does NOT apply

- The switch's cases return DIFFERENT values with no shared trailing return —
  nothing to break to.
- Target does NOT show the steal pattern (per-case `move v0,zero` BEFORE the
  jump, not in its delay slot, or no shared `.LA` v0-set) — then per-case
  `return 0;` may be the correct source form.
- The inverse problem — target HAS per-case v0-sets that your build
  constant-folds away — is [[shared-end-label]] (read both to keep the two
  straight).

## Related

- [[shared-end-label]] — the INVERSE recipe (defeat per-case constant-fold by
  unifying returns); this rule is "defeat per-case v0-materialization by
  removing per-case returns".
- [[switch-vs-ifchain-branch-sense]] — sibling "restore the natural switch
  form and codegen falls out" case (there: if-goto chain → switch; here:
  per-case return → break).
- [[cross-jump-store-tail-merge]] — the goto-label mixed exit forms that
  coexist with this pattern (`goto done`/`goto fail` inside the switch are
  fine and stay).
- [[register-alloc-pure-c]] — the RA-levers playbook; this rule's conflict is
  created by sched1 ordering, so none of the RA levers (live-range shaping)
  could ever fix it — the v0-set had to leave the block entirely.
