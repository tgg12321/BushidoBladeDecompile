# func_80078B04 — WIP checkpoint (sessions 1–5, 2026-06-12)

## TL;DR

Floor 7 -> 2 in session 1 (`u16 *p` named intermediate; reviewer PASS).
Sessions 2–5 have not broken floor 2 across ~79 manual variants + 7188
random permuter iter + directed PERM_GENERAL + GCC source dive. Session
5 root-caused the mechanism at `expr.c:5245-5246` (expand_expr
evaluates LHS of `+` first, fixing RTL plus-operand order from C
order). combine.c:2975-2986 swap rule does NOT fire because both
operands are 'o' (REG) by combine time. `D_8009BD6C + v*0x10` lowers
to `(plus (reg lw) (reg sll))`; reversing to shift-first regresses
because it either kills the named-intermediate's allocno preference
push OR collapses the if/else trampoline. Next levers: instrumented
`BB2_ALLOC_DEBUG`/`BB2_PRIO_DEBUG` cc1 dump diff against matched
sibling A68 (isolate whether A68's STORE-base-reuse is the lever),
then PERM_RANDOMIZE + PERM_LINESWAP with typed-pointer family axes.

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

Pseudos: 73=v (zext $a0, pref [3,4])→$v1; 78=sll(73,4)→$v1;
79=mem(D_…)→$v0; 74=plus(79,78)=p→$v0 (FIRST source's reg);
75=zext(mem(74))=result→$v0. Insn 22: `(set 74 (plus 79 78))` — output
74 takes 79's reg ($v0). Target needs `(plus 78 79)` so 74 takes
78's ($v1). Session-5 root cause: expr.c:5245-5246 expands LHS first
(lower pseudo → first PLUS RTL operand); combine swap rule is gated
on second operand non-'o', so by combine both are REG and the swap
doesn't fire. C source order is the SOLE knob.

## Sibling cross-ref (func_80078A68 — matched)

A68 uses `base = (t0 * 0x10) + D_8009BD6C` (shift-first) and matches
target's `addu $v1,$v1,$v0`. A68 is flat early-return with base
reused as STORE base multiple times. B04 needs if/else with
j-trampoline + single LOAD; the early-return analog collapses to
11-13 bi. **Hypothesis (untested):** A68's repeated STORE-base-reuse
anchors `base`'s allocno preference toward $v1 INDEPENDENT of the
plus-operand order. Standalone-extract A68 without stores to isolate.

## Session 5 (this run) — GCC source dive + 5 fresh variants

Read tools/gcc-2.7.2/{expr.c,combine.c,fold-const.c}. Swept under
tmp/func_80078B04_s5_variants/ (full table in meta.json sessions[1]):
v01 (shiftfirst+named-int inline) **7** — rules out named-int×order
interaction; v02 (outer p + `p+=v*8`) **7** bi=13 collapse; v03
(`&((u16*)D_…)[v*8]`) **2** no improvement; v04 (`ofs=v<<4`) **7**
named-accum regression; v05 (`D_… - (-(v*16))`) **2** unary negate
folds away (useful neg: simple-fold tricks can't change plus's
first operand).

## Ruled out (do NOT re-derive)

- `& 0xFFFF` order swaps, `(u16)arg0` casts, `u16 v` decls.
- Early-return forms — break trampoline (8-12 bi).
- Outer-scope `p` initialization — hoists lw out of if-true (7-9).
- Shift-first inline (any spelling, with or without named-int) — 7.
- Named accumulators (`ofs += / sv += D_…`) — 7.
- Typed-ptr subscript `p[v*8]` w/ outer base — 7.
- Subscript with explicit address-of `&((u16*)D_…)[v*8]` — 2 (no
  improvement).
- Unary-negate fold tricks — 2 (no improvement).
- `register-asm` pin / hardcoded-$N `__asm__` — forbidden
  ([[inline-asm-policy]], [[inline-asm-injection]]).
- Dead stores — cheat AND no improvement.
- Random permuter (7188 iter) — no break.
- Directed PERM_GENERAL (addr/mask/compare axes) — base 25, no break.
- GCC source dive: combine swap rule gated off (both REG operands);
  C source order is the SOLE knob.

## What worked

Session 1's candidate, unchanged through sessions 2–5:
`u16 *p; ... p = (u16 *)(D_8009BD6C + v * 0x10); result = *p;`
score 2, build_insns = target_insns = 14, cheat-reviewer PASS.

## Next levers (in order)

1. **Instrumented cc1 dumps vs A68** (next_hypotheses[0] promoted):
   `BB2_ALLOC_DEBUG=1 BB2_PRIO_DEBUG=1` on candidate AND a synthesized
   standalone of A68's body stripped to just the addu (`base = t0*16
   + D_8009BD6C; return *(u16*)base;`). Diff allocno-preference and
   conflict graphs at the addu insn. Isolate whether A68's multi-store
   base-reuse vs the early-return shape is the lever.
2. **Directed PERM_RANDOMIZE + PERM_LINESWAP** combined with typed-ptr
   axis (`u16 *p`, `s32 *p`, `u32 q`, `void *p`).
3. **Read-only GCC mips.c** — `mips_legitimize_address` / `ADDRESS_COST`
   for plus-of-(load, shift) tiebreakers (toolchain unmodified;
   informational only).

## Files

- `tmp/func_80078B04_variants/` (v0–v36),
  `tmp/func_80078B04_variants2/` (v37–v74),
  `tmp/func_80078B04_s5_variants/` (v01–v05).
- `tmp/v11_full.{i.greg,i.lreg,s}` — session 1 RTL dumps.
- `permuter/func_80078B04/` — directed-permuter workspace.
