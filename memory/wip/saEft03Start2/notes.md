# saEft03Start2 — volatile MMIO closes most of it; +0x1AA displacement is the residual

## TL;DR (2026-06-14)
Floor 15 -> **7** with a CLEAN candidate (no cheats). The 4 regfix rules
(2x $2<->$3 swap, 1 nop insert, 1 reorder) are all caused by the SPU register
access (`*(u16*)((u8*)D_800A2CDC + 0x1AA)`) being non-volatile. Making it a
volatile MMIO access (`*(volatile u16 *)(D_800A2CDC + 0x1AA)`, candidate.c)
reproduces the target's load-delay nop and the $v0/$v1 register assignment —
this is the SAME legitimate MMIO category as the spu_WriteReg match committed
this session (D_800A2CDC is the SPU hardware base, register at +0x1AA).
Residual: candidate is 54 insns vs target 52 — GCC materializes the full
address `BASE + 0x1AA` (an extra `addiu` per case) where target keeps BASE in
$v0 and uses displacement (`lhu v1,426(v0)` / `sh v1,426(v0)`).

## Resume steps
1. Apply candidate.c to src/main.c. `sandbox saEft03Start2 --disable all` ~ 7.
2. Close the last ~3 insns: get volatile-base-WITH-displacement that does NOT
   CSE-collapse the per-case base loads.

## The core tension (GCC 2.7.2, measured)
- `(volatile u16*)(BASE + 0x1AA)` (candidate): per-case loads kept (54), volatile
  works (nop + regs correct), BUT `addiu v0,v0,426` penalty.
- `(volatile u16*)BASE` + `[0xD5]`/`*(p+0xD5)`: CSE-hoists base load across the
  3 cases -> collapses to 42 (score 20). WRONG structure.
- `*(volatile u16*)(p + 0x1AA)` deref-cast on non-volatile base: volatile does
  NOT take (score 15, identical to baseline). The volatile must be on the
  dereferenced POINTER TYPE of a pointer VARIABLE.
Target wants per-case base loads + displacement lhu/sh + volatile nop — a combo
none of the above produces.

## Live hypotheses
- split-read-defeats-hoist: keep the 3 D_800A2CDC (non-volatile global) reads
  per-case WITHOUT the +0x1AA full-address materialization.
- A per-case distinct base expression that GCC can't CSE but still folds the
  +0x1AA into the lhu/sh displacement.
- If genuinely impossible in GCC 2.7.2, accept the 7-floor WIP or a tiny
  canonical residual.

## Ruled out (do not re-derive)
- volatile base ptr + index/arith/reload-local -> 20 (CSE collapse to 42).
- deref-cast volatile on non-volatile base -> 15 (volatile ignored).

## Pointers
- spu_WriteReg (src/main.c, COMPLETED-C this session) — established the
  volatile-MMIO-for-SPU-register pattern + precedent (src/main.c:2078).
- `.claude/rules/split-read-defeats-hoist.md`, `register-alloc-pure-c.md`,
  codegen-technique-index MMIO-volatile note.
