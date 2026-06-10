---
name: param-reuse-base-copy-cse-canon
description: Retire an INLINE_MOVE_ALIASING base-copy pin via param reuse (multi-set walker) + early-named call-arg local (flips cse.c make_regs_eqv's longest-lived canonical-reg choice)
paths: ["src/*.c"]
---

# Param-reuse walker + early-named call arg — retire an INLINE_MOVE_ALIASING copy pin

## Symptom

A list/record-walking function carries the INLINE_MOVE_ALIASING cheat cluster:
two register pins (`register s32 cached asm("$16") = arg0;` +
`register s32 new_var asm("$20");`) and an
`__asm__ volatile("move %0, %1" : "=r"(new_var) : "r"(cached))` manufacturing a
base-pointer copy. Target asm has TWO copies the cheat-free build lacks:

```mips
move  s0, a0          # param copied to callee-save at entry (paired sw s0)
...
lw    v0, 0(a1)
move  s4, s0          # base copy, in the lw's load-delay slot
addu  s0, s0, v0      # walker advances IN PLACE (reads s0, not s4)
```

The cheat-free build (48 insns vs target 49) keeps `arg0` in `$a0` until a
single `addu s0,a0,v0` — both copies folded away.

## Lever 1 — the walking pointer must be the PARAMETER itself

Writing the walker as a separate single-set local (`s32 cached = arg0;`) lets
cse copy-propagate the param into all uses; the local's def goes dead and the
copy vanishes (the 48-insn form). Instead REUSE the parameter as the cursor:

```c
void func(s32 arg0, ...) {
    s32 base;
    ...
    base = arg0;        /* move s4,s0 — cannot coalesce: arg0 redefined next */
    arg0 += off;        /* addu s0,s0,v0 — param multi-set */
    count = *(s32 *)arg0;
    arg0 += 4;
    /* loop walks arg0 */
}
```

A multi-set param pseudo lives in a callee-save from entry (forced
`move s0,a0`, save+def paired at the prologue head), and `base = arg0` must
materialize because the param is redefined while `base` stays live.

## Lever 2 — name the call's first arg EARLY so cse keeps the add in-place

With `func_callee(base + entry_off, ...)` computed at the call site, the form
still mismatches by a 3-register rotation (9 diffs): the advance emits
`addu s0,s2,v0` (reads the COPY) and base/sx/sy rotate s2/s3/s4.

Mechanism (cse.c `make_regs_eqv`, GCC 2.7.2): when cse processes
`(set base arg0)`, it inserts the NEW reg at the FRONT of the quantity's
equivalence chain if its last-use cuid is LATER than the chain head's
(`uid_cuid[REGNO_LAST_UID (new)] > uid_cuid[REGNO_LAST_UID (firstr)]` — the
"prefer the longest-lived register as canonical" heuristic). When the call-arg
`base + entry_off` is expanded at the CALL statement, base's last UID falls
after the walker's last advance → base becomes `qty_first_reg` → `canon_reg`
rewrites the advance's source operand to base, adding a 3rd reference that
also flips the allocno priority rotation.

The fix: name the callee's first argument as a loop-body local at the TOP of
the loop body, BEFORE the remaining field reads/advances:

```c
do {
    s32 entry;
    entry = base + *(s32 *)arg0;   /* base's LAST use — early UID */
    arg0 += 8;
    dx_u = *(u16 *)arg0; arg0 += 2;
    dy_u = *(u16 *)arg0; arg0 += 2; /* walker's last use — LATER UID */
    ...
    func_callee(entry, dx + sx, dy + sy);
} while ((count--) != 0);
```

Now the walker's last-use UID exceeds base's, the tie-break fails, the param
stays canonical, and the advance emits in-place (`addu s0,s0,v0`). Combine
folds the single-use `entry` back into the arg-setup insn, so emitted
position/bytes are unchanged — only cse's canonical-register choice flips.
Both moves land naturally (the base copy in the lw's load-delay slot via
sched), and the allocation rotation resolves (base → s4 lowest priority).

## Why this is not a "cheats by any spelling" violation

`base` preserves the list base while the parameter walks — the callee receives
base-relative element pointers; a human would write exactly this. `entry`
NAMES the element pointer being dispatched ("compute the entry, read its
dx/dy, call") — the named-intermediate declaration order family,
SOTN-accepted per [[no-new-park-categories]]. Param reuse as a cursor is
idiomatic 90s C. Cheat-reviewer PASS (2026-06-10).

## Confirmed case — func_800483DC (text1b.c, 2026-06-10)

Queue top, verdict C, distance 6, 0 rules, 2 pins + 1 `__asm__` move. A prior
session had logged 9 failed attempts — all using a separate single-set local
for the walker (the foldable spelling). Param reuse: 7 (insns 49 = target).
Early-named `entry`: 7 → 0. All cheat-asm removed; SHA1 == oracle;
COMPLETED-C.

## Related

- [[hoist-call-arg-local-flips-jal-delay]] — sibling named-intermediate lever
  (there: LUID-bias for reorg's delay-slot fill; here: last-use cuid bias for
  cse's canonical-register choice).
- [[inline-move-aliasing]] — the archived cheat family this retires.
- [[drop-param-alias-local]] — the INVERSE param lever (there you REMOVE the
  alias so the param promotes naturally; here you keep the param as the
  multi-set walker so the copy materializes).
- [[register-alloc-pure-c]] — the parent RA playbook; the s2/s3/s4 rotation
  here was a ref-count artifact of the cse substitution, not a standalone
  allocation problem.
- [[exit-path-return-set-cse-join]] — sibling cse-canonicalization rule (cse
  block boundaries vs pass order); this rule is the qty_first_reg ordering
  within ONE block.
