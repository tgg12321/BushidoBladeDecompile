# func_80078B04 — WIP checkpoint (2026-06-12, session 2)

## TL;DR

Floor 7 → 2 from session 1 (named `u16 *p` intermediate; reviewer PASS).
Session 2 explored 38 more variant forms — all converge at 2 or regress.
Remaining 2-insn diff is a coupled `addu`+`lhu` register-direction
tiebreak driven by the plus's RTL operand order. The C expression
`D_8009BD6C + v * 0x10` lowers to `(plus (reg lw) (reg sll))`; reversing
to shift-first breaks either the named-intermediate's preference push
or the if/else trampoline. Next lever: directed permuter (not yet set up).

## Target asm (14 insns)

```
andi $v1, $a0, 0xFFFF
slti $v0, $v1, 3
beqz $v0, .L80078B30
 sll $v1, $v1, 4
lui  $v0, %hi(D_8009BD6C)
lw   $v0, %lo(D_8009BD6C)($v0)
nop
addu $v1, $v1, $v0           ; <-- candidate emits addu $v0,$v0,$v1
lhu  $v0, 0($v1)             ; <-- candidate emits lhu  $v0,0($v0)
j .L80078B34
 nop
.L80078B30:
addu $v0, $zero, $zero
.L80078B34:
jr $ra
 nop
```

## Mechanism (from `cc1 -da` i.lreg + i.greg)

Candidate body's pseudos at lreg:
- 73 = `v` (zero_extend $a0)
- 78 = sll(73, 4)         — pre-allocated to $v1
- 79 = mem(D_8009BD6C)    — pre-allocated to $v0
- 74 = plus(79, 78) = p   — pre-allocated to $v0 (FIRST source's reg)
- 75 = zero_extend(mem(74)) = result — pre-allocated to $v0

Allocno preferences for pseudo 73: `[3 4]` (named-intermediate push: $v1
wins over $v0) — this gave us the andi-into-$v1 match in session 1.

The remaining 2 diffs are:
- insn 22: `(set 74 (plus 79 78))` — output pseudo 74 takes its FIRST
  source's register ($v0). Target needs (plus 78 79) → output in $v1.
- insn 26: `(set 75 (zero_extend (mem 74)))` — base and dest both end up
  $v0 because 74 and 75 are coalesced through the return path.

The plus's commutative operand order in RTL is locked by the C
expression order. Reversing the C order (shift-first) regresses score
by either pushing the addu out of the if-block (named-intermediate
preference lost → back to 7) or collapsing the trampoline (build_insns
falls 14→11-13, score 8-12).

## Session 2 sweep summary (38 variants)

Plateau at 2 (~22 of them): all preserve the candidate's
named-intermediate shape (`u16 *p` declared outer, assigned inside if),
varying the pointer-arith spelling: byte-ptr, s32-ptr, char-ptr,
subscript, struct-typed, separate s32 intermediate, named offset,
u32 cast — none flip the plus's RTL operand order.

Regress to 5-12 (~6 of them): shift-first family (`v*0x10 + D_8009BD6C`,
`ofs += D_8009BD6C`, `sv += D_8009BD6C`, m2c-form early-return) — break
either the named-intermediate effect or the if/else trampoline structure.

## What's been ruled out (do NOT re-derive)

- All `& 0xFFFF` order swaps, `(u16)arg0` casts, `u16 v` decls.
- Early-return forms `if (cond) return *expr; return 0;` (build_insns 13,
  score 12 — target's structure has explicit `j; nop` trampoline + else).
- `result = 0;` init outside if — collapses to score 8-9.
- Outer-scope `p` initialization (hoists lw out of if-true branch — 7-9).
- Intermediate variables for the addition (`ofs`, `sv`, `addr`):
  named-after-shift but added to D_... still gives 2; reversing to
  D_... first regresses.
- `register-asm` pin / single-insn `__asm__` with hardcoded $N —
  forbidden cheat ([[inline-asm-policy]], [[inline-asm-injection]]).
- Dead stores (`p = 0;` before if, etc.) — would be cheats and don't
  even improve the score (verified v74).

## What worked (session 1, unchanged)

`u16 *p; ... p = (u16 *)(D_8009BD6C + v * 0x10); result = *p;` — score 2,
build_insns = target_insns = 14, cheat-reviewer PASS.

## Next levers (in order of measured/plausibility)

1. **Directed permuter** (NOT YET SET UP). Setup:
   `permuter/func_80078B04/` with base.c (candidate), target.s (from
   `asm/funcs/func_80078B04.s` + `tools/decomp-permuter/prelude.inc`
   stripped of `.set gp=64`), settings.toml, compile.sh. Run
   `tools/decomp-permuter/permuter.py -j N --stop-on-zero permuter/func_80078B04`.
   The random RTL-mutating search may find a non-obvious shape that
   flips the plus operand order while preserving the named-intermediate
   preference push.
2. **GCC source dive**: `tools/gcc-2.7.2/{expr,combine,fold-const}.c`
   for the commutative-operand canonicalization heuristic. If pseudo
   number ordering or operand "simpler" classification controls the
   swap, we may find a C-shape that creates the sll-feeding pseudo at
   a lower number than the lw-feeding pseudo.
3. **Sibling cross-ref**: `func_80078A68` (matched, in same file)
   uses `base = (t0 * 0x10) + D_8009BD6C` (mult-first) and works. Diff
   its greg dump against this one — A68 is a STORE not a LOAD, so its
   constraints are different, but the operand-order normalization
   should be visible.

## Files

- `tmp/func_80078B04_variants/` (session 1, v0–v36) and
  `tmp/func_80078B04_variants2/` (session 2, v37–v74) — full variant
  archive. Winners (score 2) all share the named-intermediate `p`
  shape with `D_8009BD6C + v*0x10` ordering.
- `tmp/func_80078B04_standalone.i.greg`, `tmp/v11_full.i.greg`,
  `tmp/v11_full.i.lreg`, `tmp/v11_full.s` — RTL dumps from session 1.
