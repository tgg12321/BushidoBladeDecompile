# func_80049718 (text1b.c) — WIP, floor 6 (HEAD 11), BLOCKED

## TL;DR (2026-06-14)
HEAD carries 2 regfix reorder rules (16-insn prologue @3-18 + 3-insn store
@167-169), honest floor 11. **Removing two semantically-empty constructs — a
`do { ... } while (0)` wrapper and an empty `if (0) { }` — drops the floor to
6** (they were cheat-by-spelling scheduling perturbers). The candidate (clean,
0 cheat-asm) is saved. Residual 6 is an operand-order + store-scheduling
coupling; not byte-identical (retire SHA1 mismatch 3fd1338c).

## Residual gap (floor 6)
1. `p_anim = &D_800EF980[arg0]`: target computes lui/addiu (BASE) then
   sll (index), `addu s0,v1(index),v0(base)`. Mine does sll first → index in
   $v0, base in $v1 (operands swapped). Need base computed first.
2. One `sh v0,4(s2)` store reordered (the 2nd regfix `reorder 168,169,167`).

## Resume steps
1. Paste candidate.c; confirm sandbox 6.
2. Work the s0 operand order (base-first) and the sh store position. Verify via
   retire (full SHA1) — the operand swap may be masked, so trust retire.
3. Likely modality: decomp-permuter from the floor-6 base.

## Ruled out (do not re-derive)
- `p_anim = D_800EF980 + arg0;` → floor 11 (worse). Keep `&D_800EF980[arg0]`.
- `(s16*)((s32)&D_800EF980[0] + arg0*2)` → no change (6).

## Finding worth noting
The `do{...}while(0)` + empty `if(0){}` here were INFLATING the floor (11→6
when removed) — a reminder that these no-op constructs are not free; they can
perturb sched1. Worth a cheat-by-spelling detector note (empty-if already has a
detector per the 2026-06-02 audit; the do-while(0)-around-real-statements
wrapper is the perturber here).

## Pointers
- `.claude/rules/prologue-fix-redundant-reorder.md`, `compare-operand-order-register.md`
- `.claude/rules/register-alloc-pure-c.md`
