# Evidence bank — func_8007DC9C

- s1 recon (2026-07-22): baseline re-confirmed THIS session — `canonical` verdict C distance 9; `sandbox --disable all` score 9 (target 91 / build 90 insns, rules_dropped 4, cheat_asm_stripped 150). Gap = 1 combine-fold insertion (BF68[0], axis A) + 8-op sched1 reorder cluster (first debug_printf setup, axis B).

- s1 FRESH FINDING: D_8009BF68 has TWO disagreeing block-scope externs in src/display.c — line 872 (func_8007D3F8 scope) `extern s32 (*D_8009BF68)(s32 *, s32);` (function pointer) vs line 965 (func_8007DC9C scope) `extern s32 D_8009BF68[];` (array). func_8007D3F8 does `D_8009BF68 = arg0;` (scalar fn-ptr assign) + fills a fn-ptr table at &D_80103680, so BF68's true type is a scalar function pointer, not an int array. The axis-A fold asymmetry (only BF68 unfolded in target; BF6C/BF70 fold) is plausibly a declaration-shape artifact of the array[0] access. Couples to sibling func_8007D3F8 (queue distance ~43, only other user). NOTE: imported rejected bank already killed the fn-ptr CAST form — but not a consistent scalar fn-ptr DECLARATION+read; that shape is un-tried.

- WIP rejected_form: {'form': 'extern volatile s32 D_8009BF68[]', 'score': 9, 'reason': 'permuter-derived (3-min -j4 run, base 630 -> 515). Volatile coercion is detected as cheat-asm by engine.volatile_cheats and stripped before scoring (cheat_asm_stripped 459 -> 460). Use-site is a single read in printf args — does NOT satisfy [[legitimate-volatile-interrupt-touched]] criterion #2 (not spin-wait, not double-read-across-sequence-point, not IRQ-mutated-loop-bound). IRQ writer cited: func_8007D6D8.s:103 `sw $v0, 0x0($s0)` where `$s0 = &D_8009BF68`. Carve-out criterion #1 (IRQ-mutated) holds, but #2 (use-site) does not.'}

- WIP rejected_form: {'form': 's32 *bf68 = D_8009BF68; ... bf68[0]', 'score': 9, 'reason': 'defeat-combine-symbol-fold pre-compute. Did not move floor because the rule explicitly requires NON-ZERO displacement (target reads `bf68[N]` for N != 0); at offset 0 the (mem (plus sym 0)) simplifies to (mem sym) before combine even runs the substitution. Tried both `bf68 = D_8009BF68` at start of if-body, before the first debug_printf call (an intervening call between def and use), and `bf68 = &D_8009BF68[0]` — same result.'}

- WIP rejected_form: {'form': 'extern s32 D_8009BF68[1]  (sized vs incomplete array)', 'score': 9, 'reason': "Array size doesn't change the combine fold behavior for offset-0 accesses."}

- WIP rejected_form: {'form': 'extern s32 (*D_8009BF68)(...) + (s32)D_8009BF68 cast', 'score': 9, 'reason': 'Function-pointer cast still produces a folded lui+lw($lo) (single scalar load from the symbol address). Same bytes as array fold form.'}

- WIP rejected_form: {'form': '(D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_madr, *g_gpu_dma_chcr  (swap last two args)', 'score': 8, 'reason': 'Score-improvement APPARENT (1 less than baseline 9) but SEMANTICALLY WRONG. Target wants arg4=*chcr and arg5=*madr (D_8009BF54 loaded second, deref into $a3 = arg4); my analysis of the relocations confirms this. Swapping the args in C inverts which value lands in $a3 vs sp+0x10. The masked Levenshtein scores the WRONG-direction swap 1 better than the correct order purely by accident of the instruction-sequence search path — DO NOT commit this form.'}

- WIP rejected_form: {'form': 'status = *g_gpu_stat_reg; debug_printf(..., status, ...);  (single read, status used as arg3)', 'score': 19, 'reason': "Drops the second volatile read entirely — target has TWO reads of *g_gpu_stat_reg at offsets 6E4FC and 6E52C, both through $v1=stat_reg_ptr. Collapsing to one read produces a body 1 insn shorter than target's, scoring much worse."}

- WIP rejected_form: {'form': 'const s32 *fmt_p = &g_str_gpu_timeout; debug_printf(fmt_p, ...)', 'score': 9, 'reason': "Local-fmt-precompute did not reorder the scheduler's fmt-LUI vs BF78-LUI tie-breaker."}

- WIP rejected_form: {'form': '(void)*g_gpu_stat_reg;  (explicit dead read in place of new_var = ...; (void)new_var;)', 'score': 9, 'reason': "Equivalent emit (one volatile load discarded); doesn't change the schedule."}

- WIP rejected_form: {'form': 'block-scoped { s32 new_var = *g_gpu_stat_reg; ... (void)new_var; }', 'score': 9, 'reason': "Tighter scope for new_var did not change the RA's choice of $a0 vs $v0 for the dead load."}

- == imported from memory/wip notes.md ==
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


- [s1] canonical func_8007DC9C -> verdict C, distance 9, total 91 (pure-C target).

- [s1] sandbox --disable all -> score 9; target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150.

- [s1] Axis A (1 op): target keeps 3-insn materialized load for D_8009BF68[0] (asm lines 43-45 lui;addiu;lw 0($v0)); our build combine-folds to 2 insns. Only BF68 unfolds in target; BF6C/BF70 fold normally -> asymmetry is a per-symbol C access-shape difference.

- [s1] Axis B (8 ops): target schedule order fmt-LUI,fmt-ADDIU,*stat-early-lw(->$v0),BF78-LUI,BF78-LW; ours BF78-LUI,BF78-LW,*stat-early-lw(->$a0),fmt-LUI,fmt-ADDIU. Closed by regfix reorder 21,20,19,18 + subst lw $4->$2.

- [s1] FRESH: D_8009BF68 has two disagreeing block-scope externs in src/display.c - line 872 (func_8007D3F8 scope) `extern s32 (*D_8009BF68)(s32*,s32)` fn-ptr vs line 965 (func_8007DC9C scope) `extern s32 D_8009BF68[]` array. func_8007D3F8 assigns D_8009BF68=arg0 (scalar fn-ptr) + fills fn-ptr table at &D_80103680, so BF68's true type is a scalar function pointer, not an int array.

- [s1] Imported rejected bank already killed: fn-ptr CAST, sized array, intervening-call precompute (defeat-combine-symbol-fold needs non-zero displacement, excludes offset 0), arg-order swap (semantically wrong), single-read stat collapse, volatile coercion (cheat, stripped), block-scoped/fmt-local/(void)-read schedule nudges. Do NOT re-run.

- [s1] Instrumented cc1 present at tmp/gccdbg/cc1 (needs WSL) for the axis-B sched-priority dump - not yet run.

- [s1] scan_hand_coded (imported) tier LOW; pure-C match provably exists.
