---
name: split-scalars-hide-aggregate
paths: ["src/*.c", "include/*.h"]
description: "A pointer local carrying the address of one of several ADJACENT splat scalars (`p = &D_X; *p = v; ... f(p - N)`) is the signature of an ARRAY/STRUCT splat never merged. The pointer store is a varying non-struct MEM, so canon_rtx cannot resolve it and it takes a false alias edge against every fixed-address global read in the block — the residual then looks like a scheduler problem and invites a const/volatile coercion. Declare the aggregate instead; the dependence disappears with no qualifier."
metadata:
  type: reference
---

# Split scalars hide an aggregate (and manufacture a false alias edge)

> **Historical framing note (2026-08-30):** this rule predates the removal of the
> regfix/asmfix rule system (retired at zero rules; machinery deleted). Where the
> symptom text says a function "carries a rule", read it as "the honest build shows
> this diff shape vs target". The technique itself is unchanged.

## Symptom

A body that writes 2+ CONSECUTIVE `D_8000xxxx` scalars and passes an address
derived from one of them, e.g.

```c
extern s32 D_8009BF24, D_8009BF28, D_8009BF2C;
p = &D_8009BF24;
*p = a;  D_8009BF28 = b;  D_8009BF2C = c;
return dispatch(..., (s32)p - 8, 0x14, 0);   /* the unexplained "- 8" */
```

and a residual where an unrelated GLOBAL READ in the same block (a vtable /
dev-table pointer load) schedules several slots later than target.

**Two tells that the object model is wrong, not the scheduler:**
1. an address arithmetic constant nobody can motivate (`p - 8`, `p - 2`) —
   it is the compiler reaching a base from an element it materialized;
2. a size argument that equals the byte size of the whole run
   (`0x14` == `sizeof(u_long[5])`).

## Mechanism

`*p = a` is a MEM with a VARYING address and no `MEM_IN_STRUCT_P`. `canon_rtx`
(sched.c:370-377) resolves a pointer pseudo to its symbol only through
`reg_known_value`, installed by `init_alias_analysis` (sched.c:421-433) only for
a REG_EQUAL note at `reg_n_sets == 1` or a REG_EQUIV note. When that lookup
fails, the store conflicts with EVERY fixed-address symbol read — including
reads of unrelated globals — so those loads acquire a true memory dependence on
the packet store and sink below it.

Declaring the run as the array/struct it is makes every write an `ARRAY_REF` /
`COMPONENT_REF` at a fixed address. No varying store exists, the false edge
cannot form, and the loads schedule where target has them.

Note what does NOT help: `p->member` and `p[6]` are the SAME to the scheduler —
`COMPONENT_REF` (expr.c:4888) and `INDIRECT_REF` over a `PLUS` (expr.c:4567)
both set `MEM_IN_STRUCT_P`, and sched.c:834-839 then exempts the ref from
conflicting with fixed-address non-struct refs. Adopting a struct on the READ
side changes nothing; it is the STORE side's varying pointer that does the damage.

## The trap this closes

The false edge is exactly what `const` / `RTX_UNCHANGING_P` deletes at
sched.c:828 (`true_dependence` returns 0 for an unchanging MEM). That makes a
qualifier look like "the closing lever" when it is really compensating for a
wrong declaration — and const on a global is REFUSED
(docs/grind/decisions.md, MoveImage ruling 2026-08-17). Reach for the object
model first.

## Recipe

1. Look at the ADJACENT `.data` words in `asm/data/*.s`: initialized constants
   immediately below the written scalars are the head of the same object
   (they are the array's initializers).
2. Corroborate the shape — matched reference decomp of the same original source
   (`tmp/sotn-decomp/...` for PsyQ library code), the size literal already
   passed at the call, indices used across the codebase.
3. Add a `dlabel` at the object's true base in `asm/data/*.s` (emits no bytes;
   move the preceding `enddlabel` up) + a `symbol_addrs.txt` entry, declare
   `extern T name[N];`, and retire the per-word externs so there is ONE C handle
   per storage (the "per-word splat symbol -> aggregate merge" family,
   .claude/rules/no-new-park-categories.md).
4. Write the elements in TARGET's first-store order: cse.c materializes the
   FIRST store's address into a register and reaches the base by a negative
   offset — that is where the mysterious `- 8` comes from, and it only matches
   when the first store is the right element.

## Confirmed case

`MoveImage` (src/display.c), 2026-08-17: five grind sessions and ~152k permuter
iterations plateaued at floor 2/4 with three scalars + a pointer local, and the
only lever found was `const` (refused on evidence). Declaring
`g_gpu_move_param[5]` at 0x8009BF1C — PsyQ libgpu sys.c's `u_long param[5]`,
whose two initialized head words were already in .data — closed it to 0 and
retired all 19 regfix rules. Every earlier session's construct (packed-store
ordering, walking rect pointer, plain-deref dispatch, the pointer local and its
`- 8`) became unnecessary and was deleted. Full grid:
memory/grind/MoveImage/evidence.md `[s6]`.

Related: [[splat-symbol-names-are-not-evidence]],
[[header-type-correction-from-use-sites]], [[proven-spelling-class-reconstruction]].
