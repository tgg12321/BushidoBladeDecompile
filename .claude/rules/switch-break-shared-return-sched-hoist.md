---
name: switch-break-shared-return-sched-hoist
paths: [".claude/rules/switch-break-shared-return-sched-hoist.md"]
description: "Per-case `return 0;` makes sched1 hoist the v0-set into a load-delay slot (wrong RMW register) and flips a case's branch polarity: write `break;` + one shared trailing `return 0;`."
metadata:
  type: reference
---

# Per-case `return 0;` in a switch → `break;` + one shared `return 0;`

## Symptom

Two diff families in a switch-dispatch function: (1) a case's call-result test has flipped polarity
(`bnez v0,done` vs target `beqz v0,end; move v0,zero`); (2) a global byte RMW (`lbu/addiu/sb`) lands in `$v1`
where target uses `$v0`. Target's tail is the tell:

```mips
.LA:  addu $v0, $zero, $zero     # ONE shared return-0
.LB:  lw $ra, 16(sp); addiu $sp; jr $ra
```

with each case ending `j .LB; move v0,zero` (the v0-set in the delay slot, jump past `.LA`) — reorg's
steal-from-target-thread pattern, produced only by `break;` + a shared trailing `return 0;`.

## Mechanism

Per-case `return 0;` expands a `(set v0 0)` in every case block. sched1 fills the RMW's `lbu` load-delay slot
with it, making `$v0` live across the RMW pseudos at global-alloc time → the RMW goes to `$v1` (the final asm
order looks identical; only the register betrays it). And jump.c inverts `if (call() == 0) return 0;` so the
return block falls through.

## Fix

```c
switch (state) {
case 0:  ...; break;                 /* was: return 0; */
case 2:  if (call() == 0) break;     /* was: return 0; */
         goto done;
case 7:  ...; return 1;              /* non-zero returns stay */
}
return 0;                            /* the ONE shared return-0 */
```

Only paths returning the trailing value become `break`. func_8003AB44: 6 → 0 after 15 sessions of RA levers that
could never work (the conflict came from sched1 ordering).

Diagnose with the `.sched` dump: a per-case `(set v0 0)` sitting between the RMW's load and its consumer.
`BB2_FLOW_DEBUG=1` live sets rule out cross-block theories fast — prefer direct dumps over inference.

Does NOT apply when cases return different values with no shared trailing return, or the target lacks the
steal pattern. The inverse problem (target keeps per-case v0-sets we fold) is [[shared-end-label]].

## Related

[[shared-end-label]] · [[switch-vs-ifchain-branch-sense]] · [[cross-jump-store-tail-merge]] ·
[[register-alloc-pure-c]]
