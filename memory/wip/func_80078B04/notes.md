# func_80078B04 — WIP checkpoint (sessions 1–6, 2026-06-12)

## TL;DR

Floor 7 -> 2 in session 1 (`u16 *p` named intermediate; reviewer PASS).
Sessions 2–6 have not broken floor 2 across ~87 manual variants + 7188
random permuter iter + directed PERM_GENERAL + GCC source dive + m2c
cross-ref. Session 5 root-caused at `expr.c:5245-5246` (expand_expr
LHS-first → first PLUS RTL operand fixed by C order). combine.c
swap rule does NOT fire (both operands 'o'=REG by combine time).
`D_8009BD6C + v*0x10` lowers to `(plus (reg lw) (reg sll))`; shift-
first regresses (kills named-int allocno preference OR collapses the
trampoline). Session 6 cross-checked m2c (reconstructs as early-return
shift-first; reproducing collapses bi=13, so the original WAS the
if/else trampoline form, not early-return). Next levers: instrumented
cc1 dump diff vs sibling A68; PERM_RANDOMIZE+PERM_LINESWAP with
typed-pointer family.

## Target asm (14 insns)

```
andi $v1, $a0, 0xFFFF        slti $v0, $v1, 3
beqz $v0, .L80078B30          sll $v1, $v1, 4    ; delay
lui  $v0, %hi(D_8009BD6C)     lw  $v0, %lo(D_8009BD6C)($v0)
nop                           addu $v1, $v1, $v0  ; cand: $v0,$v0,$v1
lhu  $v0, 0($v1)              j   .L80078B34     ; cand: 0($v0)
 nop                          .L80078B30: addu $v0, $0, $0
.L80078B34: jr $ra            nop
```

## Mechanism (cc1 -da i.greg, candidate)

Pseudos: 73=v→$v1, 78=sll(73,4)→$v1, 79=mem(D_…)→$v0,
74=plus(79,78)=p→$v0 (FIRST source's reg), 75=zext(mem(74))→$v0.
Insn 22: `(set 74 (plus 79 78))`. Target needs `(plus 78 79)` so 74
takes $v1. Session-5: expr.c:5245-5246 expands LHS first (lower
pseudo → first PLUS operand); combine.c:2975-2986 swap gated on
second operand non-'o', so by combine both are REG and swap doesn't
fire. C source order is the SOLE knob.

## Sibling A68 cross-ref

A68 uses `(t0 * 0x10) + D_8009BD6C` shift-first AND matches target's
addu sign. A68 is flat early-return with base reused as multiple
STORE bases. B04's trampoline form has single LOAD; early-return
analog collapses to 11-13 bi. **Hypothesis (untested):** A68's
multi-store base-reuse anchors base's allocno toward $v1
independently of plus operand order. Standalone-extract A68 stripped
to just the addu to isolate.

## m2c cross-ref (session 6)

m2c reconstructs as `if (v<3) return *((v*0x10)+D_8009BD6C); return 0U;`
(early-return shift-first). Reproducing collapses bi=13 (loses
j-trampoline), scoring 12. m2c is dataflow-correct but structurally
lossy here; original WAS if/else trampoline.

## Session 6 — 8 fresh variants

Swept `tmp/func_80078B04_s6_variants/`:
- v01 inverted-sense `if (v>=3) {0;} else {...}` → **8** bi=13
- v02 `s32 addr; addr=D_…+v*0x10; p=(u16*)addr` → **2** (addr folds)
- v03 `(u32)D_… + (u32)(v*0x10)` double-cast → **2** (casts fold)
- v04 outer `p=(u16*)D_…` then `p=(u16*)((s32)p+v*0x10)` → **7** bi=13
- v05 m2c early-return shift-first → **12** bi=13
- v06 m2c early-return load-first → **12** bi=13
- v07 `u16 result` (vs s32) → **2** (zext-equivalent)
- v08 shift-first p_named (re-confirm s5_v01) → **7** bi=14 (NEW:
  trampoline preserved, named-int pref still killed)

## Ruled out (do NOT re-derive)

- `& 0xFFFF` order swaps, `(u16)arg0`, `u16 v` decls.
- Early-return forms (both spellings) — collapse 11-13 bi.
- Inverted if/else sense — collapses.
- Outer `p` init — hoists lw out of if-true (7-9).
- Shift-first inline (any spelling, w/ or w/o named-int) — 7.
- Named accumulators (`ofs += / sv += D_…`) — 7.
- Typed-ptr subscript `p[v*8]` outer base — 7.
- Subscript w/ explicit address-of — 2 (no improvement).
- Unary-negate fold tricks — 2 (no improvement).
- Two-step `s32 addr; p=(u16*)addr` — 2 (addr folds back).
- Double `(u32)` casts — 2 (casts fold).
- Return type narrowing (`u16 result`) — 2 (zext-equivalent).
- `register-asm` pin / hardcoded-$N `__asm__` — forbidden.
- Dead stores — cheat + no improvement.
- Random permuter (7188 iter), directed PERM_GENERAL — no break.
- GCC source dive: C source order is the SOLE knob.
- m2c cross-ref: collapses, ruling out early-return as original.

## What worked

Session 1's candidate, unchanged through sessions 2–6:
`u16 *p; ... p = (u16 *)(D_8009BD6C + v * 0x10); result = *p;`
score 2, bi=ti=14, cheat-reviewer PASS.

## Next levers (in order)

1. **Instrumented cc1 dumps vs A68** — build `tmp/gccdbg/cc1` with
   `BB2_ALLOC_DEBUG=1 BB2_PRIO_DEBUG=1`; run on candidate AND a
   stripped A68 standalone (`base=t0*16+D_…; return *(u16*)base;`).
   Diff allocno-preference + conflict graphs at the addu insn to
   isolate the multi-store-base-reuse vs early-return-shape lever.
2. **Directed PERM_RANDOMIZE + PERM_LINESWAP** + typed-ptr axis
   (`u16 *p`, `s32 *p`, `u32 q`, `void *p`).
3. **Read-only GCC mips.c** — `mips_legitimize_address` /
   `ADDRESS_COST` for plus-of-(load, shift) tiebreakers (informational).

## Files

- `tmp/func_80078B04_variants/` (v0–v36), `_variants2/` (v37–v74),
  `_s5_variants/` (v01–v05), `_s6_variants/` (v01–v08).
- `tmp/v11_full.{i.greg,i.lreg,s}` — session 1 RTL dumps.
- `permuter/func_80078B04/` — directed-permuter workspace (base.c
  at candidate; reuse on resume).
