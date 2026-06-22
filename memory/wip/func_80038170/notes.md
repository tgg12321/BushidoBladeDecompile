# func_80038170 (code6cac_c_mid.c) — BLOCKED: phantom 8-byte frame local

## TL;DR
Same class as func_8001924C. Target frame -56, cc1 naturally gives -48. The ONLY pure-C
mechanism that gives vars=16/frame=56 without extra body asm is `{ s32 tmp[2]; }` (dead array,
no stores) — **FORBIDDEN** per `dead-vars-local-array` (engine detector refuses completion).
Declaration-order reversal (s3,s2,s1) also **FAILS** cheat-reviewer (prologue-save-order
manipulation). Honest distance = **12, all from +8-byte frame delta**. 141-insn body is
otherwise byte-identical. No recoverable semantic local. Card BLOCKED.

## The gap (precise)
- natural cc1 (clean pure C): `addiu sp,sp,-48`, vars=8, saves at 24/28/32/36/40
- target:                       `addiu sp,sp,-56`, vars=16, saves at 32/36/40/44/48
- Body: NO sp-relative accesses except prologue saves + epilogue restores.
- The 8 extra bytes at sp+16..sp+31 (target) are NEVER accessed. Phantom local.

## What was tried (full lever record)

1. **HEAD cheats stripped** → score 12 (all frame delta). Body byte-identical.
2. **s3,s2,s1 declaration order** → gives correct save ORDER naturally (s0,s3,s2,s1,ra).
   FAILS cheat-reviewer: prologue-save-order manipulation (param-local-alias-prologue-pair-flip).
   Saved under `rejected/decl-order-prologue-flip.c`.
3. **Dead array `{ s32 tmp[2]; }` (no stores, nested end block)** → vars=16/frame=56, correct
   save offsets, NO extra body stores, no register-alloc shift. FORBIDDEN: dead-vars-local-array.
   This IS the compiler mechanism but the technique is forbidden by policy.
4. **Dead array with stores** → vars=16 but adds 2 extra `sw $0,16($sp)` instructions + shifts
   register allocation. Does not match target body even if frame is right.
5. **Dead scalars `s32 tmp1,tmp2`** → vars=8. GCC eliminates scalar dead-stack slots entirely.
6. **Arg-area expansion** → func_80079194 called with only 2 args; expanding prototype doesn't
   change arg-build area (only actual passed args count).
7. **Structural variants** (all s1/s2/s3 orderings, outer j/k/v loops, mask-before-i) → all
   give vars=8/frame=48. No structural variant produces vars=16.

## HEAD cheats (all forbidden)
- `register s32 i asm("a3")`, `register s32 mask asm("a1")` — register pins
- `s32 dummy0, dummy1; __asm__ volatile ("" :: "m"(dummy0), "m"(dummy1));` — dead-vars frame-coercion
- regfix `reorder 10,11,13,12,9 @ 9-13` — save-order reorder
- `prologue_config.json` entry — replaces prologue with hardcoded -56 frame

## Resume guidance
No remaining pure-C levers. The correct disposition is park (unrecoverable phantom frame local,
no semantic local recoverable from body). Surface for user authorization (canonical-asm or accept
as indefinitely parked INCOMPLETE). Do NOT attempt any dead-array or declaration-order approach.
