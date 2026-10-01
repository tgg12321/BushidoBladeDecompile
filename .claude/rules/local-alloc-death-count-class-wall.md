---
name: local-alloc-death-count-class-wall
description: "Pure $v0<->$v1 swap between a reused multi-load temp and a short constant is NOT a priority tie: local-alloc.c:472 punts reg_n_deaths>1 pseudos to global, the constant wins $v0. Diagnose via .lreg deaths."
paths: [".claude/rules/local-alloc-death-count-class-wall.md"]
metadata:
  type: reference
---

# The local-vs-global allocation CLASS wall (`local-alloc.c:472` death-count gate)

## Symptom

Build and target are structurally identical except two registers are swapped, typically
`$v0`<->`$v1`. One value is a variable **reused across several loads/stores**; the other is a
**short-lived constant or mask** built in place (`lui`/`ori`, `li`). Every reorder, declaration-order
change and block-local split leaves the floor or worsens it. This is **not** a `global.c`
allocno-priority tie — diagnose before spending a session on priority levers
([[register-alloc-pure-c]] Levers A/C).

## Mechanism (`tools/gcc-2.7.2/`)

```c
/* local-alloc.c:472 */
if (reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1 && ...)
    reg_qty[i] = -2;   /* LOCAL — allocated now */
else
    reg_qty[i] = -1;   /* punted to GLOBAL alloc */
```

1. A variable holding N distinct loaded values has `reg_n_deaths == N`; for N > 1 it is ALWAYS
   global-allocated, however the C is spelled.
2. A constant whose sets chain into one quantity has one death in one block: ALWAYS local.

MIPS defines no `REG_ALLOC_ORDER`, so `find_free_reg` / `find_reg` scan ascending regno
(local-alloc.c:2249-2262, global.c:1057-1062): the local constant takes `$v0`, the punted web gets
`$v1`. A constant has no copy source, hence no preference to override the scan.

## The three exits — all measured dead

- **(a)** make the load-temp single-death — impossible (N loads = N deaths; a struct block copy
  still dies N times and loses the interleave).
- **(b)** make the constant multi-death — needs an extra emitted instruction.
- **(c)** a `$v0`-blocking single-death local across the constant's range = a load split; every one
  measured worse (scheduler hoists it, or it steals `$v0` from the web).

The one flip found — a **fresh invented staging local** carving one load out of the web — is a
cheat (no semantic purpose; [[staged-value-reused-variable]] requires reusing an EXISTING variable).
**Do not re-propose it.**

## Diagnosis (one dump)

`cc1 ... -da` → `.lreg`: `Register <N> used K times ... dies in D places` — `D > 1` on one contended
pseudo **is** the wall. `.greg`: `"N regs to allocate: ..."` — a pseudo absent was local-allocated;
if the reused temp is the only entry, you are in this class. cc1psx emits the same wall on the same
C, so the original source was not this C shape.

Examples: func_800611A4, func_80061658, func_80061710, func_80057CC8 (text1b.c; ledgers in
`memory/grind/<func>/evidence.md`); likely siblings func_800617C8, func_800618B4, func_8006133C.

Related: [[register-alloc-pure-c]] · `pre-slim-2026-10-01:memory/project/register-alloc-deep-dive.md` (the global.c
priority wall — distinguish by `.lreg` death count) · [[staged-value-reused-variable]] ·
[[no-compiler-divergence]]
