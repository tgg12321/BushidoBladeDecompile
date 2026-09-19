# func_80017A44 — WIP checkpoint (2026-09-19)

Status: INCOMPLETE pure-C target; honest floor reduced from 208 to 89.

## Resume

Replace `INCLUDE_ASM("asm/funcs", func_80017A44);` in `src/ings.c` with
`candidate.c`, and add the four declarations currently adjacent to the stub:

```c
extern void func_80017848(u8 *, s32, s32, s32);
extern void SetRotMatrix(u8 *);
extern void SetTransMatrix(u8 *);
extern void RotTrans(s16 *, s32 *, s32 *);
```

`& tools/wteng.ps1 main sandbox func_80017A44 --disable all --diff` should
report score 89, 211 candidate instructions versus 208 target instructions.

## Semantics recovered

- Apply the descriptor's rotation/translation matrix.
- Transform each 8-byte point into a 0x40-byte output record and accumulate
  the transformed positions whose source index is nonnegative.
- Compute the centroid, then store each record's clamped centroid distance.
- Walk variable-length index groups and call `func_80017848` for every pair,
  ordering the pair by centroid distance.

The independent m2c reconstruction agrees with this control flow and field
usage. The local Kengo name lead (`coli_MakeKatanaVec`) is only a size match;
no public source body was found, so it was not treated as evidence.

## Live gap

Only eight diff regions are source-level. The first and largest is allocation
coupling: target spills the long-lived record-base value at `sp+0x40` and uses
one record pointer; GCC keeps the candidate's base in a saved register and
rebases the first-loop pointer. That changes the frame by 16 bytes and rotates
most saved-register assignments. The nested group loop also folds
`groups + i + 1` to `outer + 1`, while target retains base-plus-index setup.

Do not close either gap with `volatile`, explicit `register`, fake locals,
inline assembly, stack-layout structs invented for codegen, or aliasing
coercions. The next useful step is allocator instrumentation followed by a
natural source-shape change, not another blind spelling sweep.

## Ruled out

- Literal m2c temporary split: score 115.
- Typed point/record caching: score 100.
- Explicit saved-outer local: score 95 and spills the wrong value.
- Public/Kengo source search: no usable source body.
