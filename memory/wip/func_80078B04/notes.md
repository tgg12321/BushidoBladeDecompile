# func_80078B04 — WIP checkpoint (sessions 1–4, 2026-06-12)

## TL;DR

Floor 7 -> 2 in session 1 (`u16 *p` named intermediate; reviewer PASS).
Sessions 2–4: ~74 manual variants + 7188 random permuter iter + directed
PERM_GENERAL across the address-expr/mask/compare axes — none broke
base score 20 (= sandbox floor 2). Remaining 2-insn diff is a coupled
`addu`+`lhu` register-direction tiebreak driven by the plus's RTL
operand order. `D_8009BD6C + v*0x10` lowers to `(plus (reg lw) (reg
sll))`; reversing to shift-first regresses because it either kills the
named-intermediate's preference push or collapses the if/else
trampoline. Random and directed permuter surfaces both exhausted; next
lever is GCC source dive (`combine.c`/`expr.c`/`mips.c` for
commutative-plus canonicalization) or instrumented
`BB2_ALLOC_DEBUG`/`BB2_PRIO_DEBUG` cc1 dump diff against matched sibling
func_80078A68.

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
.L80078B30: addu $v0, $zero, $zero
.L80078B34: jr $ra
 nop
```

## Mechanism (cc1 -da i.greg, candidate body)

Pseudos: 73=v (zext $a0, pref [3,4])→$v1; 78=sll(73,4)→$v1; 79=mem(D_…)→$v0;
74=plus(79,78)=p→$v0 (FIRST source's reg); 75=zext(mem(74))=result→$v0.
Insn 22: `(set 74 (plus 79 78))` — output 74 takes 79's reg ($v0). Target
needs `(plus 78 79)` so 74 takes 78's ($v1). Reversing the C order
(shift-first) regresses: named-intermediate preference is lost OR
trampoline collapses.

## Sibling cross-ref (func_80078A68 — matched)

A68 uses `base = (t0 * 0x10) + D_8009BD6C` (shift-first) and matches
target's `addu $v1,$v1,$v0`. A68 is FLAT early-return with base reused
as STORE base multiple times. B04 needs if/else with j-trampoline +
single LOAD; the early-return analog (v64/v65) collapses to 11-13
build_insns.

## Session 4 (this run) — directed-PERM probe

Setup: PERM_GENERAL on address-expr (4 alt: lw+sll, sll+lw, (v<<4)+lw,
lw+(v<<4)), mask (arg0&0xFFFF vs (s32)(u16)arg0), compare (v<3 vs
(u32)v<3U vs v<=2). Result: base score 25 (HIGHER than session-3's 20 —
the (u32)v<3U alt adds a typing insn). 240s 4-thread run; best iter
score 25, no break. Address-order alternatives are the same forms
v40/v45/v50 ruled out manually.

## Ruled out (do NOT re-derive)

- `& 0xFFFF` order swaps, `(u16)arg0` casts, `u16 v` decls.
- Early-return forms — break trampoline (8-12).
- `result = 0;` init outside if — collapses (8-9).
- Outer-scope `p` initialization — hoists lw out of if-true (7-9).
- Shift-first inline `(v*0x10 + D_8009BD6C)` — 7 (named-int pref killed).
- Named accumulators `ofs += / sv += D_8009BD6C` — 7.
- Typed-ptr subscript `p[v*8]` w/ outer `p = (u16 *)D_8009BD6C;` — 7.
- `register-asm` pin / hardcoded-$N `__asm__` — forbidden
  ([[inline-asm-policy]], [[inline-asm-injection]]).
- Dead stores (v74) — cheat AND no score improvement.
- Random permuter (7188 iter) — no break.
- Directed PERM_GENERAL across addr/mask/compare axes — base 25, no break.

## What worked

Session 1's candidate, unchanged through sessions 2–4:
`u16 *p; ... p = (u16 *)(D_8009BD6C + v * 0x10); result = *p;`
score 2, build_insns = target_insns = 14, cheat-reviewer PASS.

## Next levers (in order)

1. **GCC source dive**: `tools/gcc-2.7.2/{expr,combine,fold-const,mips}.c`
   for commutative-PLUS canonicalization during RTL expand. Especially
   `mips_legitimize_address` / `expand_binop` for plus-canonicalization
   on the target side. If pseudo number or "simpler" classification
   drives the swap, find a C-shape with the sll-feeding pseudo at a
   lower number than the lw-feeding pseudo.
2. **Instrumented cc1 dumps** via BB2_ALLOC_DEBUG / BB2_PRIO_DEBUG
   (`tmp/gccdbg/cc1`). Diff candidate vs synthesized A68-body standalone
   at the addu insn — A68 emits the desired order naturally; isolate why.
3. **Directed PERM_RANDOMIZE + PERM_LINESWAP** combined with a typed-ptr
   family axis (`u16 *p`, `s32 *p`, `u32 q`, `void *p`) — PERM_LINESWAP
   forces line-reorder permutations random doesn't reach.

## Files

- `tmp/func_80078B04_variants/` (v0–v36),
  `tmp/func_80078B04_variants2/` (v37–v74) — variant archive.
- `tmp/v11_full.{i.greg,i.lreg,s}` — session 1 RTL dumps.
- `permuter/func_80078B04/` — directed-permuter workspace. Restored to
  candidate base.c; settings.toml unchanged.
