# func_80078B04 — WIP checkpoint (sessions 1–3, 2026-06-12)

## TL;DR

Floor 7 -> 2 in session 1 (named `u16 *p` intermediate; reviewer PASS).
Sessions 2+3: 38 manual variants + typed-ptr subscript + 7188 random
permuter iterations — none broke base score 20 (= sandbox floor 2).
Remaining 2-insn diff is a coupled `addu`+`lhu` register-direction
tiebreak driven by the plus's RTL operand order. `D_8009BD6C + v*0x10`
lowers to `(plus (reg lw) (reg sll))`; reversing to shift-first regresses
because it either kills the named-intermediate's preference push or
collapses the if/else trampoline. Random permuter is now exhausted;
next lever is directed PERM macros on the workspace at
`permuter/func_80078B04/`.

## Target asm (14 insns)

```
andi $v1, $a0, 0xFFFF
slti $v0, $v1, 3
beqz $v0, .L80078B30
 sll $v1, $v1, 4
lui  $v0, %hi(D_8009BD6C)
lw   $v0, %lo(D_8009BD6C)($v0)
nop
addu $v1, $v1, $v0           ; candidate emits addu $v0,$v0,$v1
lhu  $v0, 0($v1)             ; candidate emits lhu  $v0,0($v0)
j .L80078B34
 nop
.L80078B30:
addu $v0, $zero, $zero
.L80078B34:
jr $ra
 nop
```

## Mechanism (from `cc1 -da` i.lreg + i.greg, candidate body)

Pseudos at lreg:
- 73 = `v` (zero_extend $a0)         -> pre-allocates $v1 (preference [3,4] push)
- 78 = sll(73, 4)                    -> $v1 (sll dest)
- 79 = mem(D_8009BD6C)               -> $v0 (lw dest)
- 74 = plus(79, 78) = p              -> $v0 (FIRST source's reg)
- 75 = zero_extend(mem(74)) = result -> $v0

Remaining 2 diffs:
- insn 22: `(set 74 (plus 79 78))` — output 74 takes FIRST source's reg
  ($v0). Target needs (plus 78 79) -> output in $v1.
- insn 26: `(set 75 (zero_extend (mem 74)))` — base/dest collapse on $v0
  because 74 and 75 coalesce through the return path.

The plus's commutative operand order in RTL is locked by the C
expression order. Reversing the C order (shift-first) regresses because
named-intermediate preference is lost OR the trampoline collapses.

## Sibling cross-ref (func_80078A68 — matched)

A68 uses `base = (t0 * 0x10) + D_8009BD6C` (shift-first) and matches
target's `addu $v1,$v1,$v0` shape. Structure difference: A68 is FLAT
early-return body with base reused as STORE base multiple times. B04
needs if/else with j-trampoline + single LOAD; the early-return analog
(v64/v65) collapses to 11-13 build_insns. A68's shift-first shape
depends on flat structure B04 can't adopt without losing the trampoline.

## Session 2+3 sweep summary

- 38 manual variants (v37–v74) — pointer-arith spellings, named-
  intermediate placements, mutation styles. ~22 hold floor 2; ~6
  regress (shift-first family).
- Session 3 extra: `u16 *p = (u16 *)D_8009BD6C; result = p[v*8];` —
  score 7.
- Session 3 random permuter (`-j 4 --stop-on-zero`, 7188 iter ~3min):
  base score 20, never dropped below. Random surface exhausted.

## What's been ruled out (do NOT re-derive)

- `& 0xFFFF` order swaps, `(u16)arg0` casts, `u16 v` decls.
- Early-return forms — break trampoline (score 8-12).
- `result = 0;` init outside if — collapses to 8-9.
- Outer-scope `p` initialization — hoists lw out of if-true (7-9).
- Shift-first inline `(v*0x10 + D_8009BD6C)` — score 7
  (named-intermediate preference killed).
- Named accumulators `ofs += / sv += D_8009BD6C` — score 7.
- Typed-ptr subscript `p[v*8]` w/ outer `p = (u16 *)D_8009BD6C;` — 7.
- `register-asm` pin / single-insn `__asm__` w/ hardcoded $N —
  forbidden ([[inline-asm-policy]], [[inline-asm-injection]]).
- Dead stores — would be cheats AND don't improve the score (v74).
- Random permuter randomization repertoire — 7188 iter, no break.

## What worked

Session 1's candidate, unchanged through sessions 2+3:
`u16 *p; ... p = (u16 *)(D_8009BD6C + v * 0x10); result = *p;`
score 2, build_insns = target_insns = 14, cheat-reviewer PASS.

## Next levers (in order)

1. **Directed permuter** — workspace ready at `permuter/func_80078B04/`.
   Add PERM_* macros to base.c, e.g.
   `result = *PERM_TEMP_FOR_EXPR((u16 *)(PERM_GENERAL(D_8009BD6C + v * 0x10)));`
   Re-run `--stop-on-zero` for ≥30k iters. Directed mode explores axes
   random can't hit.
2. **GCC source dive**: `tools/gcc-2.7.2/{expr,combine,fold-const}.c`
   for commutative-PLUS canonicalization. If pseudo number or "simpler"
   classification controls the swap, find a C-shape that creates the
   sll-feeding pseudo at a lower number than the lw-feeding pseudo.
3. **Instrumented dumps** via BB2_ALLOC_DEBUG / BB2_PRIO_DEBUG
   (`tmp/gccdbg/cc1`). Diff candidate vs synthesized A68-body standalone
   at the addu insn; A68 emits the desired order naturally.

## Files

- `tmp/func_80078B04_variants/` (v0–v36) and
  `tmp/func_80078B04_variants2/` (v37–v74) — variant archive.
- `tmp/v11_full.{i.greg,i.lreg,s}` — session 1 RTL dumps.
- `permuter/func_80078B04/` — session 3 directed-permuter workspace.
  Random mode exhausted; ready for PERM macros.
