# func_8007DC9C WIP — GPU-timeout debug reporter, two debug_printf calls

## TL;DR (2026-06-13, HEAD bfdc89c3, floor 9 == HEAD)

Honest pure-C distance is stuck at 9. The 4 regfix rules close exactly these 9
ops; without them the C produces a 90-insn body where target has 91. **No
floor lowering this session** — committed structural levers (block-scoped
new_var, fmt-local pre-compute, defeat-combine-symbol-fold for offset-0,
sized-array decl, function-pointer cast, `(void)*g_gpu_stat_reg` form, scoped
`status`-as-arg) all measure 9 or worse. Permuter (3-min -j4) reached 515 only
via `extern volatile s32 D_8009BF68[]` — a cheat per
[[legitimate-volatile-interrupt-touched]] criterion #2 (single-read in printf
args is NOT a sanctioned use-site shape) and stripped by `volatile_cheats`.

## The 9-op gap (Levenshtein over masked normalized insns)

1. **One insertion** — `D_8009BF68[0]` access:
   - Target: `lui $v0, %hi(D_8009BF68); addiu $v0, $v0, %lo(D_8009BF68); lw $a1, 0($v0)` (3 insns)
   - Current: `lui $a1, %hi(D_8009BF68); lw $a1, %lo(D_8009BF68)($a1)` (2 insns, combine-folded)
   - Root cause: cc1's combine.c folds `(mem (plus symbol_ref 0))` → `(mem symbol_ref)` for offset-0
     array accesses, so even `s32 *bf68 = D_8009BF68; bf68[0]` with an intervening call still folds.
     [[defeat-combine-symbol-fold]] explicitly requires non-zero displacement; the rule does not
     cover offset 0.

2. **Eight reorder ops** — schedule cluster of 5 instructions:
   - Target order: `fmt-LUI, fmt-ADDIU, *stat_reg-early-lw (→$v0), BF78-LUI, BF78-LW`
   - Current order: `BF78-LUI, BF78-LW, *stat_reg-early-lw (→$a0), fmt-LUI, fmt-ADDIU`
   - The reorder cluster reflects GCC's sched1 priority tie-breaker: in target, fmt is loaded
     before the BF78 chain (so the *stat_reg early read goes to $v0, since $a0 is already busy
     with fmt). In current, BF78 is loaded first (so the early read goes to $a0 and fmt comes
     later, clobbering $a0).
   - The regfix `reorder 21,20,19,18 @ 18-21` rule fixes a 4-instruction permutation in this
     cluster; the `subst @ 20 lw $4,0($3) -> lw $2,0($3)` fixes the early-read going to the wrong
     register.

## Why each lever in `rejected_forms` was needed

- `defeat-combine-symbol-fold` — the obvious lever for #1; ruled out because the rule's
  recipe requires non-zero displacement.
- block-scoped `new_var` / fmt-local-precompute / `(void)*g_gpu_stat_reg` — attempts to nudge
  the scheduler tie-breaker for #2; none changed the schedule.
- arg-order swap — apparent score win was a masked-Levenshtein artifact, the swap is
  semantically wrong (target wants `*chcr` in $a3, `*madr` at sp+10, which matches the
  HEAD/m2c-reconstructed C order `(fmt, X, *stat, *chcr, *madr)`).
- volatile coercion — the only permuter find; explicitly forbidden by
  [[inline-asm-policy]] expanded catalog + [[legitimate-volatile-interrupt-touched]] criterion
  #2; detected and stripped by `volatile_cheats`.

## Confirmed evidence (against current main, bfdc89c3)

- `scan_hand_coded --single func_8007DC9C` → tier LOW (no hand-coded signals, S1-S8 all
  false). Pure-C target confirmed.
- `m2c.py --valid-syntax --target mipsel-gcc-c -f func_8007DC9C asm/funcs/func_8007DC9C.s`
  reconstructs essentially HEAD's body modulo the cleanup (`D_8009BF7C = D_8009BF7C` not
  `*new_var2`); arg order is `(fmt, X, *D_8009BF48, *D_8009BF54, *D_8009BF4C)` i.e.
  `(fmt, X, *stat, *chcr, *madr)` — matches HEAD's arg order, confirms the swap is wrong.
- D_8009BF68 IRQ-writer: `asm/funcs/func_8007D6D8.s:103 sw $v0, 0x0($s0)` where
  `$s0 = &D_8009BF68` (set up at lines 30-31); handler installed via
  `irq_AcknowledgeVblank(2, func_8007D6D8)` in func_8007D3F8 (line 890 of src/display.c).
  Criterion #1 of [[legitimate-volatile-interrupt-touched]] holds. Criterion #2 (use-site
  shape: spin-wait / double-read-across-sequence-point / IRQ-mutated-loop-bound) does NOT —
  use-site is a single read passed as a printf arg.

## Next-session resume points (in priority order)

1. **Instrumented cc1 dumps via `tmp/gccdbg/cc1`** with `BB2_SCHED_DEBUG=1` and
   `BB2_PRIO_DEBUG=1` — read the sched1 priority decisions for the fmt vs BF78
   tie-breaker. Per [[register-alloc-pure-c]] Step-0, this is the map for finding the
   structural lever; the current session ran out of budget before attempting it.
2. **Read `tools/gcc-2.7.2/combine.c`** at the `(mem (plus sym 0))` simplification path
   to confirm zero-offset combine fold is unavoidable in pure C — or find a documented
   carve-out (e.g. flag bits on the symbol that suppress fold) that's reachable from
   declaration shape.
3. **Wait for func_8007D3F8 to be matched** (currently in queue at distance 43). It's the
   only other user of D_8009BF68; if its match commits a particular declaration shape for
   D_8009BF68 (struct, sized array with non-trivial element type, etc.) that survives the
   fold globally, propagate that to func_8007DC9C.

## Related rules

- [[defeat-combine-symbol-fold]] — applicable lever, but excludes offset 0.
- [[legitimate-volatile-interrupt-touched]] — narrow carve-out, criterion #2 fails here.
- [[inline-asm-policy]] — volatile coercion forbidden by default.
- [[no-compiler-divergence]] — the toolchain is frozen; the C is the only variable.
- [[difficult-is-not-impossible]] — keep grinding; the matching C exists.
- [[register-alloc-pure-c]] — Step-0 instrumented cc1 dump methodology.
