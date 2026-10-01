---
name: split-scalars-hide-aggregate
paths: [".claude/rules/split-scalars-hide-aggregate.md"]
description: "Adjacent splat scalars written through a pointer local (`p = &D_X; *p = v; f(p - N)`) or fed from one base = an unmerged array/struct. Declare the aggregate; the false alias edge / wrong pointer home disappears."
metadata:
  type: reference
---

# Split scalars hide an aggregate

## Symptoms

- **Scheduling face:** a body writes 2+ consecutive `D_8000xxxx` scalars and passes an address derived from
  one (`p = &D_8009BF24; *p = a; D_8009BF28 = b; ... dispatch((s32)p - 8, 0x14)`), and an unrelated global
  read in the block schedules later than target. Tells: an unmotivated address constant (`p - 8`) and a size
  argument equal to the whole run's byte size.
- **Register face:** a pointer homed in the wrong register feeds N loads at consecutive offsets into N adjacent
  scalars (or N stores from one base). As one record copy (`table[i] = *p;`) the block-move expansion keeps
  the pointer live across the pattern and it homes where target has it. The grind brief's DATA MODEL section
  (engine/datamodel.py) flags SPLIT-AGGREGATE / INDEXED-ACCESS / CENSUS-VS-DECL — then the declaration is
  hypothesis #1.

## Mechanism

`*p = a` is a varying-address MEM without `MEM_IN_STRUCT_P`; when `canon_rtx` (sched.c:370-377) can't resolve
`p` via `reg_known_value`, the store conflicts with every fixed-address read. Declared as an array/struct,
every write is a fixed-address `ARRAY_REF`/`COMPONENT_REF` and the false edge never forms. (Read-side struct
spellings change nothing.) `const` would also delete the edge (sched.c:828) but const on a global is REFUSED —
reach for the object model first.

## Recipe

1. Check the adjacent `.data` words: initialized constants just below the written scalars are the object's head.
2. Corroborate the shape (matched reference decomp of the same library source, the size literal at the call,
   indices across the codebase).
3. Declare `extern T name[N];` at the true base and retire the per-word externs — one C handle per storage
   (the aggregate-merge family in [[no-new-park-categories]], all its prongs).
4. Write elements in TARGET's first-store order (cse reaches the base by a negative offset from the first
   store's address).

Example: `MoveImage` — `g_gpu_move_param[5]` at 0x8009BF1C (PsyQ libgpu `u_long param[5]`) closed it to 0.

Related: [[header-type-correction-from-use-sites]] ·
[[proven-spelling-class-reconstruction]]
