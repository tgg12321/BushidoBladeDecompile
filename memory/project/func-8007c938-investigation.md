---
name: func-8007c938-investigation
description: func_8007C938 (display.c GPU draw-area / coord-packing leaf) — pin-only function; honest pure-C floor 4 (was 5); the gap is GCC's `arg0 & MASK` → in-place $a0 coalescing on var_v0's pseudo, which conflicts with hard reg $v0 occupied by the const-OR's return-reg-propagated chain. No single pure-C lever in the explored search space flips both the mask-result register and the const-load register to target's allocation.
date: 2026-06-01
status: parked
metadata:
  type: project
---

# func_8007C938 — pin-only register-rotation plateau

`src/display.c:606`. Tiny 17-insn GPU coord-packing leaf:

```c
extern u8 g_gpu_type;
s32 func_8007C938(s32 arg0, s32 arg1) {
    /* return = (mask(arg1) << shift) | (arg0 & arg_mask) | 0xE5000000
     * shift  = 11, arg_mask = 0x7FF if g_gpu_type in {0,3+} (else 12, 0xFFF). */
}
```

`canonical`: verdict C, distance 5. **0 regfix/asmfix rules** — the only
cheats are two `register asm("$2")` / `register asm("$3")` pins on
var_v0 / var_v1. Sandbox `--disable all` (strips pins): **5** baseline
(this session lowered to **4**).

## What the gap is (instrumented .greg dump from canonical cc1)

The 4 remaining diffs are pure register allocation in the merge tail:

```
Build (score 4):           Target:
andi $a0, $a0, 0x7FF       andi $v0, $a0, 0x7FF     ← reg diff: $a0 vs $v0
sll  $v1, $v1, 12          sll  $v1, $v1, 12
andi $a0, $a0, 0xFFF       andi $v0, $a0, 0xFFF     ← reg diff
lui  $v0, 0xE500           lui  $a0, 0xE500         ← reg diff
or   $v0, $a0, $v0         or   $v0, $v0, $a0       ← operand swap
jr   $ra                   jr   $ra
or   $v0, $v1, $v0         or   $v0, $v1, $v0
```

All four diffs are downstream consequences of ONE allocation decision:
GCC puts **`var_v0` (= arg0 & MASK) in $a0** via in-place modification.
Target puts it in **$v0** (fresh register). Once that flips, the const
moves to $a0, the OR operand order matches, and the chain to return
matches.

## RTL root cause (from `tmp/c938_standalone.c.greg`)

```
;; 4 regs to allocate: 75 74 73 72
;; 72 in 4   (= arg0)
;; 73 in 5   (= arg1)
;; 74 in 4   (= var_v0 — COALESCED with arg0's pseudo)
;; 75 in 3   (= var_v1)
;; 76 in 2   (= 0xE5000000 const, hard-allocated to $v0 via return propagation)

;; 74 conflicts: 74 75 2 29
;; 74 preferences: 4    ← prefers $a0!
```

The decisive RTL insn (after coalescing):

```
(insn 28 (set (reg/v:SI 4 a0) (and:SI (reg/v:SI 4 a0) (const_int 2047))))
```

Both operands of the AND are `(reg/v:SI 4 a0)` — GCC's tree → RTL pass
saw `var_v0 = arg0 & MASK`, and since arg0 dies at this point and var_v0
starts here, it COALESCED var_v0's pseudo (74) with arg0's pseudo (72).
Both end up as the same "virtual register" assigned to $a0.

Pseudo 76 (the 0xE5000000 const) is hard-pinned to $v0 because the
const-OR's result (`r_e5 = var_v0 | const`) flows directly to the
return value, and GCC's return-reg propagation places the precursor's
destination in $v0. So pseudo 74 ($a0 mask) conflicts with hard reg 2
($v0, holding the const) at the merge point.

For target's allocation, pseudo 74 would need to land in $v0 INSTEAD of
$a0. The conflict says it can't — the const is in $v0. Reversing
requires the const to land in $a0 instead, which depends on which side
of the IOR is the destination pseudo, which depends on which order
combine merges the OR chain, which depends on the C source's expression
shape AND on var_v0's pseudo not being coalesced with arg0.

## Levers tested this session (all but the score-4 baseline regress or no-op)

| Lever | Score | Notes |
|---|---|---|
| Remove pins, original body shape | 5 | starting baseline |
| Inline final OR `return v1 \| v0 \| const` | 4 | left-assoc chain |
| `r_e5 = var_v0 \| const; return v1 \| r_e5;` | 4 | same score, named intermediate |
| `(var_v0 \| const) \| var_v1` parens form | 5 | reassociates back |
| Pre-init `var_v0 = const` at top + OR-in mask | 4 / +1 insn | hoists `lui` → extra insn |
| `hi = 0xE5000000` named local at top | 4 | put mask in $v0 BUT const went to $a2 (delay slot), not $a0 |
| `mask_res` separate local, post-if const-init | 4 | coalesced back |
| `a0_copy = arg0; var_v0 = a0_copy & MASK` | 4 | coalesced back |
| `(u32)arg0 & MASK` cast | 4 | coalesced back |
| Block-local `m0 = arg0 & MASK` per arm (Lever A) | 4 | coalesced back |
| s16/u16/u32 arg types (Lever B) | 4 | no effect |
| m2c shape `(arg1 & MASK) << SHIFT` single-expr | 7-10 | breaks var_v1 delay-slot pre-compute |
| Inverted `if` branch sense | 9 | regresses |
| Goto-form control flow | 9 | regresses |
| Per-arm `return` (no merge) | 12 / +2 insn | duplicates the const |
| Const-OR folded into each arm `(arg0 & MASK) \| const` | 9-10 | duplicates the const |
| Hi assigned in each arm (defeat hoist) | 5 | duplicates |
| Hi declared at top, assigned after `if` | 4 | inlined to chain |

## Forbidden levers (skipped per project policy)

- `(volatile s32 *)&arg0` — volatile coercion on a parameter (cheat-asm
  per [[inline-asm-policy]] expanded catalog).
- `*(volatile u8 *)&g_gpu_type` — volatile coercion on game-state global
  (forbidden per [[split-read-defeats-hoist]] update).
- `arg0 = arg0;` / dead self-assignments — forbidden Lever D per
  [[register-alloc-pure-c]] update.
- `s32 pad[N];` frame coercion — forbidden per [[dead-vars-local-array]].
- `__asm__ volatile("move %0, %1" ...)` — forbidden per
  [[inline-move-aliasing]].
- Compiler patches — forbidden per [[no-compiler-divergence]].

## What I think the new goto-end-prologue-delay-slot lever applied to

Briefed as today's main new lever. The target's entry pattern is the
ordinary `lui/lbu g_gpu_type → addiu → sltiu → bnez` — NOT the
4-insn `bnez+sp-adjust-delay+j+zero-delay` entry pattern that
goto-end-prologue-delay-slot addresses. This leaf function has no
frame (no sp adjust), no early-exit goto-end shape, no return-zero. So
goto-end-prologue-delay-slot doesn't apply here. The 4-insn entry
shape exists only in functions with stack frames AND a NULL-check
early exit.

## The score-4 form (best legitimate, NOT committed since rules still present)

```c
s32 func_8007C938(s32 arg0, s32 arg1) {
    u32 var_v0;
    u32 var_v1;
    u32 r_e5;
    var_v1 = arg1 & 0xFFF;
    if ((u32) (g_gpu_type - 1) >= 2U) {
        var_v1 = arg1 & 0x7FF;
        var_v1 = var_v1 << 0xB;
        var_v0 = arg0 & 0x7FF;
    } else {
        var_v1 = var_v1 << 0xC;
        var_v0 = arg0 & 0xFFF;
    }
    r_e5 = var_v0 | 0xE5000000;
    return var_v1 | r_e5;
}
```

Distance 5 → 4 (sandbox `--disable all`). Identical structure to the
pinned source; only difference is pins removed and named intermediate
`r_e5` added. **Not committed** (the cheat-asm pins are the only thing
keeping the source byte-matching, and this form still has 4 diffs,
which means `retire` would roll back).

## Resume avenues for the next session

1. **BB2_ALLOC_DEBUG instrumented cc1** — pseudo 74's preferences are 4
   (`$a0`) due to coalescing with pseudo 72 (arg0). The `priority` /
   `assign_one_reg` walks would show whether any alternative
   allocation (74 → $v0, 76 → $a0) is in the cost ballpark or
   structurally ruled out by the conflict graph.
2. **Decomp-permuter from the score-4 base** with `target.o` built
   from `asm/funcs/func_8007C938.s` + prelude.inc (clean offset-0
   target) for honest weighted scoring. Run with `--stop-on-zero`.
   Tiny function (17 insns); permuter may find a structural mutation
   the manual search missed.
3. **Force pseudo 76 (const) NOT to land in $v0.** The const goes to
   $v0 via return-reg-propagation. Any C-source structure that breaks
   the direct flow `const-OR-result → return` would push the const
   off $v0. Examples to try:
   - Insert a no-op transformation between the const-OR and the
     return (e.g. `r_e5 = (var_v0 | 0xE5000000) ^ 0; return var_v1 | r_e5;`)
     — needs to survive combine.
   - Return through a wider type that requires a cast.
4. **Examine the just-completed siblings' source structure**
   (`func_8007C97C`, `func_8007C748`) — both went COMPLETED-C from
   similar leaf shapes. `func_8007C748` has the same `(u32)(g-1) < 2U`
   dispatch and an `arg2 & MASK` packing — note that target there has
   `andi $v0, $a2, 0x27FF` (mask in $v0, not in $a2). The C for that
   function is COMPLETED-C, so the C-level lever exists. Diff its `.greg`
   against this function's to identify what makes 8007C748's mask
   pseudo land in $v0 — likely a different conflict graph due to its
   extra args / branch structure.

## Status

Honest pure-C floor lowered from **5 → 4** this session via the chained-OR
`r_e5` form. Reverted to committed (pinned) form to keep oracle clean.
Stays INCOMPLETE in queue. Not parking — the queue policy is no new
infrastructure categories ([[no-new-park-categories]]).

## Related

- [[register-alloc-pure-c]] — the playbook this fits into (4-pseudo
  allocation tie, in-place coalescing)
- [[register-asm-pins]] — the pins ARE diagnostic; the C-source lever
  that flips them is still to find
- [[difficult-is-not-impossible]] — the cardinal rule; the matching
  C exists, the levers explored this session aren't it
- [[no-new-park-categories]] — explicit user policy that
  register-rotation-infrastructure is NOT a new park category
