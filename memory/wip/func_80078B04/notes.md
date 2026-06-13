# func_80078B04 — WIP checkpoint (sessions 1–7, 2026-06-12)

## TL;DR

Floor 7 -> 2 in session 1 (`u16 *p` named intermediate; reviewer PASS).
Sessions 2–7 have not broken floor 2 across ~100 manual variants + 7188
random permuter iter + directed PERM_GENERAL + GCC source dive + m2c
cross-ref. Session 5 root-caused at `expr.c:5245-5246` (expand_expr
LHS-first → first PLUS RTL operand fixed by C order). combine.c
swap rule does NOT fire (both operands 'o'=REG by combine time).
`D_8009BD6C + v*0x10` lowers to `(plus (reg lw) (reg sll))`; shift-
first regresses (kills named-int allocno preference OR collapses the
trampoline). Session 6 cross-checked m2c (reconstructs as early-return
shift-first; reproducing collapses bi=13, so the original WAS the
if/else trampoline form, not early-return). Session 7 swept 13 fresh
probes across orthogonal axes (ternary, switch, void*/char*, unsigned-
mask, paren grouping, inner-block, two-locals) — 11 held at 2, ternary
inline regressed to 7 (no named-int), switch regressed to 4 bi=16
(jump-table). Cheat-reviewer re-PASS on unchanged candidate.

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

## m2c cross-ref

m2c reconstructs as `if (v<3) return *((v*0x10)+D_8009BD6C); return 0U;`
(early-return shift-first). Reproducing collapses bi=13 (loses
j-trampoline), scoring 12. m2c is dataflow-correct but structurally
lossy here; original WAS if/else trampoline.

## Session 7 — 13 fresh probe variants (tmp/func_80078B04_s7_variants/)

- v01 ternary inline → **7** (no named-int, HEAD-equivalent)
- v09 `switch(v)` → **4** bi=16 (jump-table)
- v02-v08, v10-v11, v13-v14 (ternary+comma, comma-in-assign, p-first
  decl, void*/char* recast, `0xFFFFu` mask, paren-group, inner-block
  init/decl-assign, two-locals base+ofs, mutate-base chain) → **2**.

11/13 hold; 1 regresses to HEAD-floor; 1 dispatch-shape regression.

## Ruled out (do NOT re-derive)

- `& 0xFFFF` order swaps, `(u16)arg0`, `u16 v` decls, `0xFFFFu` mask.
- Early-return forms (both spellings), inverted if/else — collapse 11-13 bi.
- Ternary inline (no named-int), switch dispatch (jump-table) — regress.
- Outer `p` init — hoists lw out of if-true (7-9).
- Shift-first inline (any spelling, w/ or w/o named-int) — 7.
- Named accumulators (`ofs += / sv += D_…`), typed-ptr `p[v*8]` outer — 7.
- Subscript w/ explicit address-of, unary-negate fold, two-step
  `s32 addr;`, two-named-locals add, mutate-base compound chain,
  double `(u32)` casts, void*/char* recast, p-first decl, paren group,
  inner-block scope, comma operator, `u16 result` — **all hold at 2**.
- `register-asm` pin / hardcoded-$N `__asm__` — forbidden.
- Dead stores — cheat + no improvement.
- Random permuter (7188 iter), directed PERM_GENERAL — no break.
- GCC source dive: C source order is the SOLE knob.
- m2c cross-ref: collapses, ruling out early-return as original.

## What worked

Session 1's candidate, unchanged through sessions 2–7:
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
