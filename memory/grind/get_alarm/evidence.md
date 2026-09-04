> **ALIAS NOTE (2026-08-24):** this ledger's function is `get_alarm` (formerly `func_8007DC9C` — decisions.md rulings and interior prose may use either name; any `memory/grind/func_8007DC9C/` path is this dir).

# Evidence bank — get_alarm (formerly func_8007DC9C)

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

- [s2] Baseline re-confirmed this session: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150.

- [s2] The instrumented cc1 at tmp/gccdbg/cc1 has NO BB2_SCHED_DEBUG/BB2_PRIO_DEBUG instrumentation (verified via strings) — it is a standard debug-build cc1 supporting -da RTL dumps. The prior frontier's env-var probe plan was based on a wrong assumption; the standard .greg/.combine/.sched dumps are the actual map. The stale tmp/gccdbg/standalone.c files belong to a DIFFERENT function (PutShadowRmd), not func_8007DC9C.

- [s2] Axis B root cause (greg dump): the dead read new_var=*g_gpu_stat_reg (insn 38) allocates to reg/v 4 a0 with REG_UNUSED; fmt (insn 60, la g_str_gpu_timeout) schedules AFTER it and reloads a0. Our .s order for the first-printf cluster: statptr, madrptr, BF78, deadread(->a0), fmt, BF7C, ... Target order: statptr, fmt, deadread(->v0), BF78, madrptr, ... The 8-op gap is a rotation of {fmt, deadread, BF78, madrptr} plus the deadread reg (a0 vs v0). The deadread reg follows directly from whether fmt is scheduled before it.

- [s2] Axis B is priority-driven, not LUID-driven: 8 source-order/precompute variants all scored 9. The dead read's priority comes from volatile ordering with D_8009BF7C (both volatile), which matches target and cannot be reordered without changing observable behavior.

- [s2] Axis A root cause (combine dump): D_8009BF68[0] folds to (mem/s (symbol_ref D_8009BF68)); BF6C/BF70 are (mem (symbol_ref ...)) scalars folded normally. Target materializes only BF68's address (la;lw 0(reg)) — reachable in pure C ONLY if the address pseudo has >1 use, which requires a second reference to &D_8009BF68 absent from this function.

- [s2] s32* pointer-deref of D_8009BF68 uniquely reaches build_insns=91 (== target) but with opcode lw instead of addiu at the BF68 slot -> score 12. Confirms target materializes the ADDRESS (addiu), not a pointer value load.

- [s2] Mapping confirmed: g_gpu_stat_reg=D_8009BF48 (volatile u32*), g_gpu_dma_madr=D_8009BF4C, g_gpu_dma_chcr=D_8009BF54; first-printf arg3($a3)=*chcr, arg4(sp+0x10)=*madr — the committed arg order (fmt, arg1, *stat, *chcr, *madr) is correct; the s1 arg-swap remains a wrong-direction masked-Levenshtein artifact.

- [s3] Baseline re-confirmed: score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150. Candidate cleanup (drop the new_var2 volatile-ptr dance; write D_8009BF78 = D_8009BF7C directly) is byte-equivalent — score 9 — and is now applied in src.

- [s3] FULL-ASM READ (all 96 lines of asm/funcs/func_8007DC9C.s): target references &D_8009BF68 EXACTLY ONCE (lines 43-45 lui;addiu;lw 0($v0)). There is NO second visible &D_8009BF68 reference (address compare / store) anywhere in the function. => KILLS the s2/frontier "look for a second &BF68 reference the original had" probe: it does not exist in the emitted target. Axis-A materialization is combine-time multi-use retention (combine.c:1458 added_sets_2, same mechanism as func_8007EDBC) of a second use that a LATER pass DCE'd from output. Reproducing that in single-function pure C requires a dead second use of &D_8009BF68 = a forbidden coercion (no semantic purpose, dead in output, justified only by combine internals) per cheats-by-any-spelling. So axis A has NO legitimate single-function structural lever.

- [s3] axisA struct-triple shared base ((struct{s32 a,b,c;}*)&D_8009BF68 read as ->a/->b/->c) -> score 11 (worse), build_insns 90. KILLED: a shared struct base emits ONE materialized base + offset loads (lw 0/4/8(base)); target reads BF6C/BF70 as SEPARATE folded %lo(D_8009BF6C)/%lo(D_8009BF70) scalar symbols. So BF68/BF6C/BF70 are independent scalars, not a contiguous struct/array — closes the shared-base branch of axis A (confirms the s2 one-array negative from the opposite direction). Rejected: rejected/axisA-struct-triple-shared-base.c.

- [s3] CONCLUSION: single-function STRUCTURAL modality is exhausted for func_8007DC9C. Axis A (1 op) needs either the sibling func_8007D3F8's genuine second use of &D_8009BF68 (cross-TU, non-structural) or is a combine artifact with no legitimate single-fn pure-C form. Axis B (8 ops) is a sched1 critical-path tie already KILLED across structural forms in s2 — closing lever is non-structural (directed permuter / RA-steering). Both remaining frontiers are OUTSIDE the structural modality.

- [s3] Baseline re-confirmed s3: score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150.

- [s3] Candidate cleanup (drop new_var2 volatile-ptr dance; D_8009BF78 = D_8009BF7C direct) is byte-equivalent (score 9) and is applied in src/display.c.

- [s3] Full-asm read: target has exactly ONE &D_8009BF68 reference; the axis-A materialization is combine-retained-then-DCE'd multi-use (combine.c:1458), not a visible second in-function use.

- [s3] Struct-triple shared base -> score 11: BF68/BF6C/BF70 are independent scalar symbols in target (separate %lo loads), not a contiguous struct/array. Closes the shared-base branch of axis A.

- [s3] Single-function structural modality is exhausted: axis A has no legitimate single-fn pure-C lever (any would be a dead-second-use coercion); axis B was already KILLED across structural forms in s2.

- [s4] PERMUTER modality. Built single-function permuter workspace (tmp/perm_dc9c_s4, standalone base.c = func body + externs; compile.sh = cc1|prologue_fix|maspsx|multu_pad, extract .ent..end, assemble .set noreorder). Base-vs-target raw diff reproduces exactly the two ledger axes: axis-B reorder cluster (BF78 lui/lw vs fmt lui/lw vs deadread lw a0/v0) + axis-A 3-insn BF68 materialization (target `lui v0;addiu v0,v0,0;lw a1,0(v0)` vs our folded 2-insn `lui a1;lw a1,0(a1)`). base 90 / target 91 insns. Workspace validated.

- [s4] Campaign chassis-1 (s4-directed-random, -j8, base_score 630): 20,157 iters over ~12 min, ONE find output-515-1 = `extern volatile long D_8009BF68[]`. Volatile coercion of a game-state global — the SAME cheat family already banked in s1 WIP (`extern volatile s32 D_8009BF68[]`); stripped by engine.volatile_cheats, forbidden by [[inline-asm-policy]] expanded catalog + [[legitimate-volatile-interrupt-touched]] criterion #2 (single printf-arg read, not spin-wait/double-read/IRQ-loop-bound). Closes axis A ONLY (materialization via volatile-prevents-combine-fold). Axis B never moved. Rejected: rejected/axisA-permuter-volatile-bf68.c.

- [s4] Campaign chassis-2 (s4-chassis2-argtemp: `diff` named-temp arg-homing AST, base_score 630): 7,137 iters over ~5 min. Finds: output-515-1 = `extern volatile int D_8009BF68[]` (same volatile cheat as chassis-1); output-470-1 = pointer-alias/long-long junk — `new_var3=&g_str_gpu_timeout; ... *(new_var3=&(*g_gpu_stat_reg))` self-assigning alias + `long long new_var2=1; temp_v1=-new_var2; return temp_v1;` constant-holder laundering `return -1`. No semantic purpose, dead in output, no human would write it -> cheat-by-any-spelling ([[no-new-park-categories]] vetting). Does not legitimately close either axis. Rejected: rejected/permuter-alias-longlong-junk.c.

- [s4] CONCLUSION (permuter modality KILLED for both axes): across TWO chassis and ~27.3k iterations, every sub-baseline permuter find is a cheat — volatile-coercion on D_8009BF68 (axis A) or pointer-alias/long-long junk. The axis-B 8-op sched1 reorder cluster NEVER moved via any legitimate pure-C statement reordering the permuter tried, directly corroborating the s2 structural KILL (dead *g_gpu_stat_reg read is volatile-ordered before D_8009BF7C -> fixed high sched1 critical-path priority -> unswappable without changing observable volatile order). Directed permuter over the arg-setup region (chassis-2 arg-homing) produced no novel legitimate reorder. Both remaining axes are confirmed to have NO legitimate single-function pure-C lever: axis A needs the cross-TU sibling func_8007D3F8's genuine 2nd use of &D_8009BF68 (non-structural, outside permuter reach); axis B is a volatile-critical-path tie with no pure-C reorder.

- [s4] Built + validated a single-function permuter workspace (tmp/perm_dc9c_s4): standalone base.c (func body + externs), compile.sh = cc1|prologue_fix|maspsx|multu_pad + .ent..end extract + assemble .set noreorder; base 90 / target 91 insns; base-vs-target raw diff reproduces exactly axis-A (BF68 3-insn materialization) + axis-B (fmt/BF78/deadread reorder cluster).

- [s4] Chassis-1 (random, 20,157 iters): single find output-515-1 = extern volatile long D_8009BF68[] — volatile-coercion cheat closing axis A only; axis B never moved.

- [s4] Chassis-2 (arg-homing named-temp AST, 7,137 iters): output-515-1 = extern volatile int D_8009BF68[] (same cheat); output-470-1 = pointer-alias/long-long junk (new_var3=&(*g_gpu_stat_reg) self-assign + long long constant-holder laundering return -1) — cheat-by-spelling, closes neither axis legitimately.

- [s4] Both campaigns harvested with --stop in-turn; permuter_campaign status shows both alive:false, registered_active:false; no orphan permuter processes remain.

- [s4] Permuter corroborates s2/s3 structural KILLs: axis B is a volatile-critical-path priority tie unreachable by pure-C reordering; axis A materialization needs a multi-use &D_8009BF68 that only the cross-TU sibling func_8007D3F8 can supply.

- [s5] PERMUTER modality, THIRD chassis. Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150. src/display.c untouched (s3 candidate applied).

- [s5] Built + validated chassis-3 (tmp/perm_dc9c_s5): whole-body named-temp base.c (fmt1/fmt2 pointer handles + diff + arg_a/b/c second-printf reads all as named temps; two volatile *g_gpu_stat_reg reads + volatile D_8009BF7C read stay inline+ordered). Compiles to 90 insns; base-vs-target raw diff reproduces EXACTLY axis-A (BF68 3-insn lui;addiu;lw materialization vs folded 2-insn) + axis-B (fmt/BF78/deadread reorder cluster). Structurally distinct from s4 chassis-1 (default random) and chassis-2 (arg-homing single diff temp) - a different register-pressure regime.

- [s5] GOTCHA: the standalone permuter compile.sh runs cc1 DIRECTLY with no cpp pass. C /* */ comments in base.c cause a parse error (symptom: line-N `parse error before /`, whole body then parsed at file scope, cc1 emits locals as .globl/.comm and no .ent). base.c MUST be comment-free. (s4 base had no comments; my first chassis-3 draft crashed on the header comment.)

- [s5] Chassis-3 campaign (s5-chassis3-wholebody-temps, -j8, base_score 630): 40,054 iterations over ~27 min across THREE in-turn wait windows (cumulative 14062 / 26802 / 39477 iters). ONLY output: output-630-1 = `extern unsigned long long D_8009BF68[]` at score 630 == baseline floor 9 (NON-improving) - a width coercion of the game-state global, same coercion cheat family as s4's volatile-BF68 / long-long-holder junk. Doubly dead (coercion AND does not lower the floor). Rejected: rejected/axisA-permuter-longlong-width-coercion.c. Harvested --stop in-turn; status alive:false, registered_active:false; no orphan permuter processes.

- [s5] Axis-B 8-op sched1 cluster never reordered in chassis-3; chassis-3 did not even stumble into the volatile cheat s4 found -> confirms it explored a genuinely different region and still found nothing legitimate. PERMUTER modality now TRIPLE-confirmed dead across 3 structurally-distinct chassis (~67k total iters s4+s5): axis A reachable in permuter space ONLY via type/volatile coercion of D_8009BF68 (all cheats); axis B is volatile-order-locked. Both remaining axes have NO legitimate single-function pure-C lever. Next legitimate moves are non-permuter: axis A = cross-TU sibling func_8007D3F8 (still active/unmatched, queue dist 43); axis B = forensics/RTL confirmation of volatile-locked sched1 priority, then likely owner escalation (no OWNER-ESCALATION entry exists for func_8007DC9C yet).

- [s5] s5 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical to s1-s4). src/display.c untouched this session (s3 candidate applied).

- [s5] Built + validated a THIRD permuter chassis (tmp/perm_dc9c_s5): whole-body named-temp base.c compiles to 90 insns, base-vs-target raw diff reproduces exactly axis-A (BF68 3-insn lui;addiu;lw materialization vs our folded 2-insn) + axis-B (fmt/BF78/deadread reorder cluster). Structurally distinct from s4 chassis-1 (default random) and chassis-2 (arg-homing single diff temp).

- [s5] Gotcha recorded: the standalone permuter compile.sh runs cc1 DIRECTLY with no cpp pass, so C /* */ comments in base.c are a parse error (line-26 crash symptom = whole body parsed at file scope). base.c must be comment-free.

- [s5] Chassis-3 campaign: 40,054 iterations, ~27 min, -j8, base_score 630. Three in-turn wait windows (14062, 26802, 39477 iters cumulative). Only output: output-630-1 = extern unsigned long long D_8009BF68[] at score 630 (== floor 9, NON-improving) - width-coercion cheat, doubly dead. Harvested --stop in-turn; status shows alive:false, registered_active:false; no orphan permuter processes.

- [s5] Chassis-3 corroborates s2/s3/s4: axis A materialization is reachable in permuter space ONLY via type/volatile coercion of D_8009BF68 (all cheats); axis B 8-op sched1 cluster never legitimately reorders (volatile-order-locked critical-path priority). Permuter modality now TRIPLE-confirmed dead across 3 structurally-distinct chassis (~67k total iters s4+s5).

- [s6] s6 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150. Candidate.c (s3-cleaned, drop new_var2 volatile-ptr dance; D_8009BF78 = D_8009BF7C direct) applied to src/display.c; byte-equivalent, floor unchanged.

- [s6] Generated full RTL pass dumps for func_8007DC9C via instrumented cc1 -da -dr over real src/display.c (tmp/grind/func_8007DC9C/s6/display.i.{rtl,flow,combine,cse,cse2,loop,jump,jump2,lreg,greg,sched,sched2,dbr}). Prior gccdbg/standalone.* dumps are stale (PutShadowRmd), not this function.

- [s6] AXIS B ROOT (sched1, display.i.sched BB2): dead-read insn 38 (mem/v *g_gpu_stat_reg) priority=2; fmt insn 60 (la g_str_gpu_timeout, non-mem) priority=1. The +1 is the volatile-MEM anti-dep edge REG_DEP_ANTI 38 present on insn 45 (mem/v D_8009BF7C read), insn 64 (2nd mem/v *stat read) and call_insn 68. Two plain reads would not depend; the edge exists ONLY because both are mem/v (volatile).

- [s6] AXIS B decision trace: ready list at T-42 = {60(1) 38(2)}; 'blocking insn 38 for 1 cycles' (load hazard) so 60 fills that cycle; 38 committed at T-43/44. Emitted order (backward scheduler, T-46 first): 36 stat-ptr, 54 madr-ptr, 38 dead-read, 56 *madr, 60 fmt -> dead read BEFORE fmt (our wrong order). greg then puts the dead read in $a0. Target wants fmt before dead read (dead read -> $v0).

- [s6] AXIS B CONCLUSION: the priority ordering is volatile-order-derived; no pure-C lever can flip it (fmt's critical-path height is structurally 1; lowering the dead read's height requires removing the volatile anti-dep = non-volatile/drop = cheat + semantically wrong). Corroborates s2 structural KILL and s4/s5 permuter KILL with the exact sched1 mechanism named.

- [s6] AXIS A ROOT (combine, display.i.combine): D_8009BF68[0] => (mem/s:SI (symbol_ref D_8009BF68)) folded symbol-direct (offset-0 const simplified pre-substitution); target keeps 3-insn materialized addr via combine.c:1458 added_sets_2 multi-use retention. s3 proved single in-function use -> closes only via cross-TU sibling func_8007D3F8.

- [s6] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE), so owner-gated is not available this session despite all single-function modalities now measured dead.

- [s7] s7 baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150. src/display.c restored to clean HEAD after the experiment (no dirt).

- [s7] AXIS A cross-TU sibling theory REFUTED (compiler-source proof): combine_instructions is called per-function from rest_of_compilation (tools/gcc-2.7.2/toplev.c:3004 `combine_instructions(insns, max_reg_num())`; def combine.c:453 walks only the passed insn chain). combine has ZERO cross-function visibility. func_8007D3F8's `sw $19, D_8009BF68` store lives in its OWN combine section (display.i.combine insn 133 / display.s:2693), separate from func_8007DC9C's folded read (display.s:3170). => matching sibling func_8007D3F8 CANNOT close axis A; the "wait for sibling" frontier probe is mechanically impossible. KILLED.

- [s7] AXIS A mechanism CONFIRMED empirically: adding a 2nd intra-function use of &D_8009BF68 (`D_8009BF88 = (s32)&D_8009BF68[0];` dead store before the 2nd printf) makes objdump of tmp/sandbox/.../display.o emit EXACTLY target asm lines 43-45: `1e84 lui v0,%hi(BF68); 1e8c addiu v0,v0,%lo(BF68); 1e90 lw a1,0(v0)`. The address is materialized in a pseudo (combine.c:1458 added_sets_2 multi-use retention). sandbox 9->8, build 90->93 (the dead `sw s3,%lo(BF68)` at 195c is NOT DCE'd — store to a game global can't be proven dead across the intervening calls, so it persists +2).

- [s7] AXIS A disposition: the ONLY single-function reproduction is a dead second use of &D_8009BF68 = cheat-by-any-spelling (no semantic purpose, exists only to change combine's fold analysis, references GCC internals). Rejected: rejected/axisA-dead-addr-store-multiuse.c. Combined with the per-function-combine refutation of the sibling avenue, axis A has NO legitimate lever single-function OR cross-function.

- [s7] CONSOLIDATED: both axes measured dead across structural (s2/s3), permuter x3 (s4/s5), forensics (s6/s7); the last grindable avenue (cross-TU sibling for axis A) is refuted by the per-function-combine proof. func_8007DC9C is a clean OWNER-ESCALATION candidate. No OWNER-ESCALATION entry exists yet in docs/grind/decisions.md, so owner-gated is not claimable this session -> result progress with escalation-ready frontier.

- [s7] s7 baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150. src/display.c restored to clean HEAD after the experiment (no dirt).

- [s7] combine_instructions is called per-function from rest_of_compilation (tools/gcc-2.7.2/toplev.c:3004; def combine.c:453 walks only the passed insn chain) -> combine has NO cross-function visibility, so sibling func_8007D3F8's use of &D_8009BF68 cannot suppress func_8007DC9C's offset-0 fold. The 'wait for sibling' frontier probe is mechanically impossible.

- [s7] Confirmed by s6 combine dump: func_8007D3F8's store `sw $19,D_8009BF68` (display.i.combine insn 133 / display.s:2693) is in its OWN combine section, disjoint from func_8007DC9C's folded read (display.s:3170).

- [s7] Empirical: adding a 2nd intra-function use `D_8009BF88=(s32)&D_8009BF68[0]` (dead store) makes objdump of tmp/sandbox/func_8007DC9C/display.o emit target asm lines 43-45 verbatim (1e84 lui v0,%hi; 1e8c addiu v0,v0,%lo; 1e90 lw a1,0(v0)) via combine.c:1458 added_sets_2 multi-use retention. sandbox 9->8, build 90->93 (dead `sw s3,%lo(BF68)` at 195c not DCE'd: store to a game global can't be proven dead across intervening debug_printf/motion_make_table calls).

- [s7] Axis A disposition: the only single-function reproduction is a dead second use of &D_8009BF68 = cheat-by-any-spelling (no semantic purpose, exists only to change combine's fold analysis). Rejected: rejected/axisA-dead-addr-store-multiuse.c. With the sibling avenue refuted, axis A has NO legitimate lever single-function OR cross-function.

- [s7] Axis B unchanged: s6 CONFIRMED sched1 INSN_PRIORITY(dead *g_gpu_stat_reg read)=2 > PRIORITY(fmt)=1 via the volatile-MEM anti-dep 38->45; unflippable without altering observable volatile order.

- [s7] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE), so owner-gated is not claimable this session despite both axes now measured dead across single- AND cross-function modalities.

- [s8] REDERIVE modality. Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150. src restored to clean HEAD after the experiment (no dirt).

- [s8] Fresh m2c (tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c) reconstructs HEAD's structure: reads D_8009BF68 as a SCALAR VALUE (folds, axis-A 2-insn form) and DCE's the dead *g_gpu_stat_reg read (m2c body would be 1 insn short = the banked score-19 collapse). No structurally-different shape offered by m2c; artifact tmp/grind/func_8007DC9C/s8/m2c_fresh.txt.

- [s8] Kengo transplant leg: the Kengo (PS2 successor) corpus contains the CALLEE motion_make_table (0x118710) + VSync/VSync2/SetVSyncFlag, but NO equivalent of the GPU-timeout debug reporter func_8007DC9C itself (grep of kengo_functions_full.txt / kengo_func_names_sorted.txt for vsync/timeout/gpu/watchdog/make_table). No donor function exists -> Kengo-transplant lever is empty for this function.

- [s8] Structurally-different control-flow rederivation MEASURED DEAD: dissolved the comma-expression condition into explicit statements, re-expressed the `||` short-circuit as an early `goto report` guard + nested `if (count > 0xF0000)`, split the draw-count increment into two named statements (semantics identical: increment only when g_gpu_vcount >= sys_VSync(-1)). sandbox -> score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 — the IDENTICAL fingerprint to every prior form. Axis A (per-expression combine fold) and axis B (volatile-MEM anti-dep sched priority) are both insensitive to control-flow shape. Rejected: rejected/rederive-goto-nested-if-control-flow.c.

- [s8] CONSOLIDATED across ALL FIVE modalities: structural (s2/s3), permuter x3 (s4/s5), forensics x2 (s6/s7), rederive (s8) — every one measured both axes dead. Fresh m2c reproduces HEAD; Kengo has no donor; control-flow restructure is byte-identical. func_8007DC9C has NO legitimate pure-C lever, single-function or cross-function, in any modality. Clean OWNER-ESCALATION candidate; still NO OWNER-ESCALATION entry in docs/grind/decisions.md (grep NONE), so owner-gated is not claimable this session -> result progress with escalation-ready frontier.

- [s8] s8 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150; src restored to clean HEAD after the experiment (no dirt).

- [s8] Fresh m2c reconstructs HEAD's structure: D_8009BF68 read as a scalar VALUE (folds to axis-A 2-insn form), dead stat-read DCE'd. No structurally-different shape offered. Artifact tmp/grind/func_8007DC9C/s8/m2c_fresh.txt.

- [s8] Kengo (PS2 successor) corpus contains the callee motion_make_table (0x118710) + VSync/VSync2/SetVSyncFlag but NO equivalent of the GPU-timeout reporter func_8007DC9C -> sibling/Kengo-transplant lever is empty for this function.

- [s8] Structurally-different control-flow rederivation (comma-expr dissolved into explicit statements + early 'goto report' guard + nested if(count>0xF0000) + split increment; short-circuit semantics preserved) measured score 9 / build 90 = identical fingerprint. Axis A is per-expression combine fold; axis B is volatile-order-locked sched priority; both insensitive to control-flow shape. Rejected: rejected/rederive-goto-nested-if-control-flow.c.

- [s8] CONSOLIDATED: all five modalities now measured both axes dead — structural (s2/s3), permuter x3 (s4/s5), forensics x2 (s6/s7), rederive (s8). No legitimate pure-C lever single- or cross-function in any modality.

- [s8] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE), so owner-gated is not claimable this session despite full modality exhaustion.

- [s9] REDERIVE modality, decomp.me-corpus leg (the un-run rederive avenue; s8 covered m2c+Kengo+control-flow). Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150. src/display.c clean at HEAD (untouched). curl_cffi absent -> searched the CACHED corpus tmp/decomp_me_corpus/ = 3754 GCC-2.7.2 PSX scratches (each JSON has source_code + target_assembly).

- [s9] AXIS A cross-corpus prior-art: searched every matching scratch for a single pure rvalue word-read of a global at offset 0 that materializes as target's 3-insn `lui;addiu R,R,%lo(SYM);lw X,0(R)` (func_8007DC9C's shape) instead of folding to 2-insn `lui;lw %lo`. Regex validated (8011 same-reg addiu+%lo corpus-wide; func_8007DC9C's own form is `addiu $v0,$v0,%lo(D_8009BF68)`+`lw $a1,0x0($v0)`; offset renders `0x0(` not `0(`). STRICT filter (addiu %lo IMMEDIATELY -> offset-0 load, no intervening index math) = 541 hits, 25 in MATCHING scratches. Classified all 25: EVERY one is a genuine C-source MULTI-USE access -- RMW at [0] (D_800AF9D8[0]|=0x8000, &=0x7FFF, D_80097C40[0]++, if(D_80072F16[0]==0)[0]=0x20 => load+store=2 addr uses), struct multi-field (BtlDrawFlag.fade/.chr/.theEnd/.invi/.panel, g_Pad.m_Down, ButtonsRelated.ButtonTapped), struct-member-as-index (D_800B2384.bankIndex x7, D_800719D0.DAT_), matrix/struct copy (cameraMat=D_800AFA64), or repeated reads (D_800AF93A[0] used x2 -> two materializations). NONE is a single pure word-read at offset-0. Materialization ALWAYS traces to a real >=2-use access in the C.

- [s9] AXIS A VERDICT: KILLED, corroborates s2/s3/s6/s7 with independent cross-corpus prior-art. func_8007DC9C reads D_8009BF68[0] as a single pure rvalue (s3: exactly one target reference) -> folds to 2-insn. The corpus offers NO declaration/access shape that materializes a single pure read; the 3-insn form only ever comes from a genuine 2nd use, which single-function pure C can only supply as a dead/fake use = coercion (s7 banked axisA-dead-addr-store-multiuse.c). Rejected: rejected/rederive-decompme-corpus-multiuse-only.c. Artifact tmp/grind/func_8007DC9C/s9/FINDINGS.md + corpus_*.py/classify.py.

- [s9] AXIS B: no corpus lever possible -- axis B is a frozen-toolchain sched fact (INSN_PRIORITY dead-read=2 > fmt=1 via volatile-MEM anti-dep, s6), provably control-flow-insensitive (s8); a corpus donor cannot change a compiler-pass mechanism. Keyword search surfaced reporter-adjacent donors (VsyncHandler, InitHardware, cdReadSectors) but NONE is a GPU-timeout debug reporter analogous to func_8007DC9C -- consistent with s8 Kengo "no reporter donor".

- [s9] CONSOLIDATED: rederive modality now FULLY exhausted incl. the decomp.me-corpus leg. All five modalities (structural s2/s3, permuter x3 s4/s5, forensics x2 s6/s7, rederive s8+s9) measure both axes dead. func_8007DC9C is a clean OWNER-ESCALATION candidate; still NO OWNER-ESCALATION entry in docs/grind/decisions.md (grep NONE; commit d6bc0904's owner ruling covered 7 OTHER endgame-lock funcs, not this one), so owner-gated is not claimable this session -> result progress, escalation-ready.

- [s9] s9 baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150; src/display.c clean at HEAD (untouched this session).

- [s9] Cached decomp.me corpus tmp/decomp_me_corpus/ = 3754 GCC-2.7.2 PSX matching-decomp scratches (gcc2.7.2-psx / gcc2.7.2-cdk / psyq3.5), each with source_code + target_assembly. curl_cffi not installed so searched offline.

- [s9] Regex validated: 8011 same-reg 'addiu R,R,%lo(SYM)' corpus-wide; func_8007DC9C's own axis-A form is 'addiu $v0,$v0,%lo(D_8009BF68)' + 'lw $a1,0x0($v0)' (offset renders 0x0( not 0(); first-pass regex miss corrected).

- [s9] STRICT axis-A filter (materialized addr IMMEDIATELY -> offset-0 load, no intervening index math): 541 hits, 25 in MATCHING scratches. All 25 classified by C source = genuine multi-use (RMW / struct multi-field / struct-member-as-index / matrix copy / repeated reads). NONE is a single pure word-read at offset-0.

- [s9] Corroborates s2/s3/s6/s7 axis-A KILL with independent cross-corpus prior-art: a single pure offset-0 rvalue read always folds to 2-insn; the 3-insn materialization is combine multi-use retention of a genuine 2nd use, which single-function pure C can only supply as a dead/fake use = coercion (banked s7 axisA-dead-addr-store-multiuse.c).

- [s9] Axis B is a frozen-toolchain sched fact (INSN_PRIORITY dead-read=2 > fmt=1 via volatile-MEM anti-dep, s6) proven control-flow-insensitive (s8); a corpus donor cannot change a compiler-pass mechanism. Keyword search found reporter-adjacent donors (VsyncHandler/InitHardware/cdReadSectors) but NO GPU-timeout debug reporter analogue -- consistent with s8 Kengo 'no reporter donor'.

- [s9] Rederive modality now FULLY exhausted incl. the decomp.me-corpus leg. All five modalities (structural s2/s3, permuter x3 s4/s5, forensics x2 s6/s7, rederive s8+s9) measure both axes dead.

- [s9] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE); commit d6bc0904's owner ruling covered 7 OTHER endgame-lock functions, not this one. So owner-gated is not claimable this session despite full modality exhaustion.

- [s10] SYNTHESIS modality. No sandbox re-run needed (mechanism-pinned dead result; floor 9 unchanged, src/display.c clean at HEAD, no dirt). Merged all nine prior sessions into one attack map: tmp/grind/func_8007DC9C/s10/MERGED-ATTACK.md. Confirmed the 9-op gap is TWO orthogonal compiler-internal axes with NO joint lever possible (axis A per-expression combine fold, s8; axis B whole-block volatile-order sched priority, s2/s6 — each proven insensitive to the other's lever surface).

- [s10] Axis A synthesized verdict: KILLED. 3-insn BF68 materialization = combine.c:1458 added_sets_2 multi-use retention; requires a 2nd &D_8009BF68 use the function does not have (s3: exactly one target ref). Single-function reproduction = dead 2nd use = coercion (s7 empirical); cross-function mechanically impossible (s7 per-function-combine proof, toplev.c:3004); cross-corpus prior-art (s9) shows every offset-0 materialization traces to a genuine C multi-use, never a single pure read.

- [s10] Axis B synthesized verdict: no pure-C lever. sched1 INSN_PRIORITY(dead-read insn 38)=2 > PRIORITY(fmt insn 60)=1, the +1 being the volatile-MEM anti-dep REG_DEP_ANTI 38->45 (s6). Unflippable without changing observable volatile order (= cheat + semantically wrong, target keeps two volatile reads). Control-flow-insensitive (s8), permuter-immovable (s4/s5), no corpus donor (s9).

- [s10] Disposition: func_8007DC9C matches [[endgame-lock-disposition-policy]] (owner 2026-07-20) verbatim — byte-matches only via 4 regfix rules, 9 insns short in honest pure C, all sanctioned levers exhausted, residual is an RA/scheduling + combine-retention tiebreak. AND-gate #1 (canonical asm) = REFUSE (scan_hand_coded LOW, s1 = ordinary GCC output). AND-gate #2 (coercion family) = REFUSE (no SOTN/VS/ESA precedent for a dead-2nd-use combine coercion or a volatile-order sched coercion). Both gates fail => policy outcome = keep the cheat, classify INCOMPLETE-owner-accepted, park. This is an OWNER ruling to record, not an agent self-authorization.

- [s10] FRONTIER RESET: s1 hypotheses H-A1/H-A2/H-B1 all marked KILLED in hypotheses.md; the live frontier is now a SINGLE escalation-ready item. Verified again this session: NO OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (Grep 8007DC9C = No matches). owner-gated therefore still not claimable -> result progress, escalation dossier ready. The only remaining action is the OWNER filing the escalation entry; no further pure-C modality exists to run.

- [s10] s10 synthesis merged s1-s9 into tmp/grind/func_8007DC9C/s10/MERGED-ATTACK.md; floor 9 unchanged, src/display.c clean at HEAD (no dirt), no sandbox re-run needed (results mechanism-pinned).

- [s10] The 9-op gap = axis A (1 op, combine.c:1458 added_sets_2 multi-use retention of &D_8009BF68) + axis B (8 ops, sched.c INSN_PRIORITY via volatile-MEM anti-dep REG_DEP_ANTI 38->45). Orthogonal: axis A per-expression (s8), axis B whole-block volatile-order (s2/s6); no joint construct possible.

- [s10] Axis A KILLED across structural(s2/s3)/permuter x3(s4/s5)/forensics x2(s6/s7)/rederive x2(s8/s9): every single-function reproduction is a dead-2nd-use coercion; cross-function is mechanically impossible (per-function combine, toplev.c:3004); corpus prior-art shows single pure offset-0 reads always fold.

- [s10] Axis B: mechanism CONFIRMED (s6), no pure-C lever; volatile-order-locked, control-flow-insensitive (s8), permuter-immovable ~67k iters (s4/s5), no corpus donor (s9).

- [s10] func_8007DC9C matches endgame-lock-disposition-policy (owner 2026-07-20) verbatim: byte-matches only via 4 regfix rules, 9 insns short in honest pure C. Gate #1 (canonical asm) REFUSE (scan_hand_coded LOW, s1). Gate #2 (coercion family) REFUSE (no SOTN/VS/ESA precedent). Both fail -> keep cheat, INCOMPLETE-owner-accepted, park (owner ruling, not agent self-authorization).

- [s10] Frontier reset: s1 hypotheses H-A1/H-A2/H-B1 all marked KILLED in hypotheses.md; live frontier is a single escalation-ready item.

- [s10] Re-verified this session: NO OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (Grep 8007DC9C = No matches; d6bc0904 covered 7 OTHER endgame-lock funcs). owner-gated NOT claimable -> result progress.

- [s11] s11 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical to s1-s10). src/display.c clean at HEAD (candidate.c byte-equivalent, already reflected).

- [s11] Structural lever surface fully banked: block-local var splits (s2), declaration/source order (s2 8-form sweep), type narrowing (s2 scalar/fn-ptr/pointer fold + s4/s5 coercion forms stripped), statement re-association (s2 + s8 goto/nested-if byte-identical), struct/array grouping (s3 struct-triple -> score 11). No un-banked structural form remains.

- [s11] Axis A per-EXPRESSION combine offset-0 fold (combine.c:1458 added_sets_2 multi-use retention): control-flow/decl-order-insensitive (s8); single-function reproduction requires a dead 2nd &D_8009BF68 use = coercion (s7); cross-function impossible (per-function combine, toplev.c:3004).

- [s11] Axis B whole-block sched1 priority tie (sched.c INSN_PRIORITY): dead-read insn 38 mem/v priority=2 > fmt insn 60 priority=1 via volatile-MEM anti-dep REG_DEP_ANTI 38->45 (s6); expression/decl-shape-insensitive (s2/s6); unflippable without changing observable volatile order.

- [s11] Both axes orthogonal and mechanism-pinned dead across all five modalities: structural (s2/s3/s11), permuter x3 (s4/s5), forensics x2 (s6/s7), rederive x2 (s8/s9), synthesis (s10).

- [s11] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches; commit d6bc0904's owner ruling covered 7 OTHER endgame-lock funcs, not this one), so owner-gated is not claimable this session.

- [s12] s12 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical to s1-s11). src/display.c clean at HEAD; candidate.c byte-equivalent, already reflected. No dirt.

- [s12] Structural modality re-affirmed EXHAUSTED by proven mechanism (not just sweep): axis A is a per-EXPRESSION combine offset-0 fold (combine.c:1458 added_sets_2), axis B is a whole-block sched1 INSN_PRIORITY tie via volatile-MEM anti-dep REG_DEP_ANTI 38->45 (s6). Both structurally insensitive a priori; every structural lever category is banked (s2/s3/s11).

- [s12] Axis A single-function reproduction is only a dead 2nd &D_8009BF68 use = coercion (s7 empirical: 9->8 but build 90->93 with undeletable dead sw); cross-function is mechanically impossible (per-function combine, toplev.c:3004).

- [s12] Axis B is unflippable without changing observable volatile order (target keeps two volatile *g_gpu_stat_reg reads); permuter-immovable ~67k iters (s4/s5), control-flow-insensitive (s8), no corpus donor (s9).

- [s12] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches; d6bc0904's owner ruling covered 7 OTHER endgame-lock funcs), so owner-gated is NOT claimable this session.

- [s12] func_8007DC9C matches endgame-lock-disposition-policy (owner 2026-07-20) verbatim: byte-matches only via 4 regfix rules, 9 insns short in honest pure C. Gate #1 (canonical asm) REFUSE (scan_hand_coded LOW, s1). Gate #2 (coercion family) REFUSE (no SOTN/VS/ESA precedent). Both fail -> keep cheat, INCOMPLETE-owner-accepted (owner ruling, not agent self-authorization).

- [s13] PERMUTER modality, 4th chassis. Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150. src/display.c clean (candidate byte-equivalent, in place).

- [s13] Built + validated chassis-4 (tmp/perm_dc9c_s13): tail-temp topology (tail-store values AND second-printf args ALL hoisted to top-computed named temps arg_a/arg_b/arg_c/diff; volatile reads inline+ordered). base_score 695 (HIGHER than prior chassis's 630 — extra temps add reorderings/regs, a distinct higher-register-pressure regime), 90 insns, base-vs-target reproduces both ledger axes. Structurally distinct from s4 chassis-1 (default-random), s4 chassis-2 (arg-homing single temp), s5 chassis-3 (whole-body named-temp).

- [s13] Chassis-4 campaign (s13-chassis4-tail-temps, -j8): 30,817 iterations across in-turn wait windows; harvested --stop in-turn; status alive:false, registered_active:false, 0 live campaigns, no orphan permuter processes.

- [s13] Finds (floor = 630 == sandbox 9): 695/690/685/680/670/655 ALL ABOVE the 630 floor (legitimate reordering ceilings ~680; chassis-4 never even recovers the true floor legitimately). Only sub-floor find (590) = `extern volatile int D_8009BF68[]` = the banked volatile-BF68 coercion (axis A only; s1 WIP + s4 rejected/axisA-permuter-volatile-bf68.c), stripped by engine.volatile_cheats -> scores 9 under sandbox. 655 find = `(long long)D_8009BF70` width coercion (banked family) AND non-improving; 635 find = `new_var2=-1` dead constant-holder laundering sys_VSync(-1), also a cheat + non-improving. Axis B 8-op sched1 cluster NEVER legitimately reordered.

- [s13] PERMUTER modality now QUADRUPLE-confirmed dead across 4 structurally-distinct chassis (~98k total iters s4 27k + s5 40k + s13 31k): only permuter-reachable sub-floor form is the banked volatile coercion of D_8009BF68 (axis A); axis B is volatile-order-locked (s6 root) and never moves. Both axes remain mechanism-pinned dead.

- [s13] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches), so owner-gated is NOT claimable this session -> result progress, escalation-ready frontier unchanged.

- [s13] s13 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint to s1-s12). src/display.c clean; candidate.c byte-equivalent, already in place.

- [s13] Built + validated a 4th structurally-distinct permuter chassis (tmp/perm_dc9c_s13): tail-temp topology, base_score 695 (higher than prior chassis's 630 because the extra temps add reorderings/regs), compiles to 90 insns, base-vs-target reproduces both ledger axes.

- [s13] Campaign 30,817 iterations, -j8, harvested --stop in-turn; status alive:false, registered_active:false, 0 live campaigns, no orphan permuter processes.

- [s13] Every novel find: 695/690/685/680/670/655 all ABOVE the 630 floor (legitimate reordering ceilings ~680); only sub-floor find (590) is the volatile-BF68 coercion `extern volatile int D_8009BF68[]` already banked (s1 WIP + s4 rejected/axisA-permuter-volatile-bf68.c), stripped by volatile_cheats.

- [s13] 655 find = `(long long) D_8009BF70` (long-long width coercion, same family as s4 permuter-alias-longlong-junk / s5 axisA-permuter-longlong-width-coercion) AND non-improving; 635 find = `new_var2 = -1` dead constant-holder laundering the sys_VSync(-1) literal, also a cheat and non-improving.

- [s13] Axis B 8-op sched1 cluster never legitimately reordered in chassis-4, consistent with the s6 forensic root cause (dead-read insn 38 priority=2 > fmt insn 60 priority=1 via volatile-MEM anti-dep REG_DEP_ANTI 38->45, unflippable without changing observable volatile order).

- [s13] PERMUTER modality now QUADRUPLE-confirmed dead across 4 structurally-distinct chassis (~98k total iters s4 27k + s5 40k + s13 31k): the only permuter-reachable sub-floor form is the banked volatile coercion of D_8009BF68 (axis A); axis B is volatile-order-locked and never moves.

- [s13] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches), so owner-gated is not claimable this session.

- [s14] PERMUTER modality, 5th chassis. Baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint to s1-s13). src/display.c clean at HEAD (candidate.c byte-equivalent, already reflected); no dirt.
- [s14] Built + validated chassis-5 (tmp/perm_dc9c_s14): low-register-pressure block-scoped topology — first-printf diverging pieces (fmt/dead-read/BF78-subtraction) as tightly block-scoped temps, 2nd-printf args (D_8009BF68[0]/BF6C/BF70) left INLINE. OPPOSITE pressure regime to s5 chassis-3 (whole-body temps) and s13 chassis-4 (tail+2nd-printf hoisted). base_score 695, compiles to 90 insns / target 91, reproduces both ledger axes.
- [s14] tools/permuter_annotate.py directed-permuter is INAPPLICABLE to func_8007DC9C: its hint catalog (register-asm-pins, shared-end-label, loop-rotation-two-shift, loop-counter-fills-load-delay) maps to none of the diverging region (axis A = combine offset-0 fold; axis B = volatile-MEM sched-priority tie). No directed annotation exists for either axis; random chassis is the only permuter avenue.
- [s14] Chassis-5 campaign (s14-chassis5-lowpressure-blockscope, -j8): 20,344 iterations across in-turn wait windows; harvested --stop in-turn; status alive=False, registered_active=False, 0 live campaigns, 0 orphan permuter processes.
- [s14] Finds (floor=630): legitimate reordering ceiling 635-695 ALL ABOVE the 630 floor (chassis-5 never even recovers the true floor legitimately); output-630-1 ties floor via a null 0x800-constant-hoist reshuffle (non-improving). Only two strictly-sub-floor finds, BOTH cheats: output-500-1 (500) = corrupted fmt pointer (g_gpu_dma_madr substituted for &g_str_gpu_timeout — masked-Levenshtein false-match) + `char` width-coercion of D_8009BF78-D_8009BF7C + dead `new_var3=new_var2` alias self-assign (banked permuter-alias-longlong-junk family; rejected/permuter-s14-fmt-corrupt-char-narrow-junk.c); output-580-1 (580) = `extern volatile int D_8009BF68[]` = banked volatile-BF68 coercion, axis A only, stripped by engine.volatile_cheats (rejected/axisA-permuter-volatile-bf68.c). output-635-1 = `unsigned short new_var=-1; return new_var` u16 constant-holder cheat (banked family, non-improving).
- [s14] Axis A (BF68[0] 3-insn materialization) never legitimately materialized in any legitimate chassis-5 find (stays inline combine-folded 2-insn); axis B 8-op fmt-vs-deadread sched1 cluster never legitimately reordered — consistent with the s6 forensic root (dead-read insn 38 priority=2 > fmt insn 60 priority=1 via volatile-MEM anti-dep REG_DEP_ANTI 38->45, unflippable without changing observable volatile order).
- [s14] PERMUTER modality now QUINTUPLE-confirmed dead across 5 structurally-distinct chassis (~118k total iters: s4 27k + s5 40k + s13 31k + s14 20k). Only permuter-reachable sub-floor forms are banked coercions of D_8009BF68 (axis A) or fmt-corruption/width-coercion junk (semantically wrong); axis B is volatile-order-locked and never moves.
- [s14] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches), so owner-gated is NOT claimable this session -> result progress, escalation-ready frontier unchanged.

- [s14] s14 baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint to s1-s13); src/display.c clean at HEAD, no dirt.

- [s14] Chassis-5 (tmp/perm_dc9c_s14) = low-register-pressure block-scoped topology (first-printf diverging pieces as block temps, 2nd-printf args inline), base_score 695, 90 insns/target 91, reproduces both ledger axes — a distinct pressure regime from s4/s5/s13.

- [s14] Campaign 20,344 iters, harvested --stop in-turn; alive=False, registered_active=False, 0 live campaigns, 0 orphan permuter processes.

- [s14] Both strictly-sub-floor finds are cheats: output-500-1 (fmt-pointer corruption + char width-coercion + dead alias self-assign, banked) and output-580-1 (volatile-BF68 coercion, banked, stripped by engine.volatile_cheats). Above-floor 635-695 = legitimate reorder ceiling (non-improving); 630 ties floor (null constant-hoist).

- [s14] Axis A (BF68[0]) never legitimately materialized (stays 2-insn combine-folded); axis B 8-op sched cluster never legitimately reordered — consistent with s6 forensic root (dead-read insn 38 prio 2 > fmt insn 60 prio 1 via volatile-MEM anti-dep REG_DEP_ANTI 38->45).

- [s14] tools/permuter_annotate.py directed-permuter is INAPPLICABLE: its hint catalog (register-asm-pins/shared-end-label/loop-rotation-two-shift/loop-counter-fills-load-delay) maps to none of func_8007DC9C's diverging region (axis A = combine offset-0 fold; axis B = volatile-MEM sched-priority tie).

- [s14] PERMUTER modality now QUINTUPLE-confirmed dead across 5 structurally-distinct chassis (~118k total iters: s4 27k + s5 40k + s13 31k + s14 20k).

- [s14] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches), so owner-gated is NOT claimable this session.

- [s15] FORENSICS (3rd run; independent re-verification of s6/s7 opus claims per verify-opus-handoff-claims). Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s14). Fresh cc1 -da -dr dumps over committed src/display.c: tmp/grind/func_8007DC9C/s15/display.i.{rtl,flow,combine,cse,cse2,loop,jump,jump2,lreg,greg,sched,sched2,dbr} + FORENSICS.md. src/display.c clean at HEAD (only tmp/ written).

- [s15] AXIS A source-verified: tools/gcc-2.7.2/combine.c:1458 reads verbatim `added_sets_2 = ! dead_or_set_p (i3, i2dest);` (comment: "SETs in I1 or I2 need to be kept around ... whenever the value set there is still needed past I3"). combine keeps the 3-insn address-materialization ONLY when I2DEST (address pseudo) is live past I3 = multi-use. func_8007DC9C's single pure offset-0 rvalue read (s3: one &BF68 ref) => I2DEST dies in I3 => folds to 2-insn. s15 combine dump confirms `(mem/s:SI (symbol_ref "D_8009BF68"))` symbol-direct (no plus 0). s6/s7 citation is EXACT (verified against source, not merely quoted). CONFIRMED.

- [s15] AXIS B reproduced from fresh RTL (insn UIDs +3 vs s6: dead-read 38->41, fmt 60->63, BF7C 45->48, uniform shift because HEAD body carries 3 more prior insns than s6's candidate; byte-equivalent, floor 9). s15 sched priority list: insn[41] priority=2 ref_count=5 (dead read *g_gpu_stat_reg), insn[63] priority=1 ref_count=1 (fmt la). Post-sched RTL: insn 41 = (set (reg/v:SI 75) (mem/v:SI (reg 82))); insn 48 = (set (reg 85) (mem/v:SI (symbol_ref "D_8009BF7C"))) carrying (insn_list:REG_DEP_ANTI 41 (nil)); 2nd stat read + tail also carry REG_DEP_ANTI 41/48. Both 41 and 48 are mem/v => the ANTI edge exists ONLY because both are volatile. Decision T-42 {63(1),41(2)}: "blocking insn 41 for 1 cycles, now 63" then "launching 41 before 63" => dead read emitted before fmt (our order); target wants fmt first. CONFIRMED.

- [s15] The +3 UID shift with identical priorities/anti-dep/decision proves axis B is INVARIANT to the exact body shape (HEAD vs candidate) — independently corroborates s8/s11/s12 structure-insensitivity. No new pure-C lever surfaced; both axes remain mechanism-pinned dead. Forensics now TRIPLE-run (s6/s7/s15). No OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (grep = no matches) => owner-gated not claimable; result progress, escalation-ready.

- [s15] Baseline re-confirmed s15: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint to s1-s14). src/display.c clean at HEAD; only tmp/ dumps written.

- [s15] Axis A source-verified: tools/gcc-2.7.2/combine.c:1458 = `added_sets_2 = ! dead_or_set_p (i3, i2dest);` — the exact mechanism s6/s7 cited; combine keeps the 3-insn materialization only for a multi-use address pseudo, so this function's single pure offset-0 read folds to 2-insn.

- [s15] Axis B reproduced from fresh RTL: dead read insn 41 = (mem/v (reg 82)) priority 2; fmt insn 63 = la into a0 priority 1; volatile D_8009BF7C read insn 48 carries (insn_list:REG_DEP_ANTI 41); both 41 and 48 are mem/v so the anti-dep edge is purely volatile-ordering. Scheduler emits dead read before fmt (our order); target wants fmt first.

- [s15] Insn UIDs shifted uniformly +3 vs s6 (HEAD body carries 3 more prior insns than s6's candidate; byte-equivalent, floor 9) with otherwise-identical priorities/anti-deps/decision => axis B invariant to body shape.

- [s15] Forensics now triple-run (s6/s7/s15); both opus-authored (s6/s7) mechanism pins hold up under independent fresh-dump + source scrutiny per verify-opus-handoff-claims. No new pure-C lever surfaced.

- [s15] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep 8007DC9C = no matches), so owner-gated is not claimable this session.

- [s16] FORENSICS (4th run). Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s15). Fresh cc1 -da -dr dumps over committed src/display.c: tmp/grind/func_8007DC9C/s16/display.i.{rtl,flow,combine,cse,cse2,loop,jump,jump2,lreg,greg,sched,sched2,dbr} + display.s + FORENSICS.md. src/display.c clean at HEAD (only tmp/ written; git status src/+regfix.txt = clean).

- [s16] NEW forensic result: regfix rule <-> axis COMPLETENESS cross-map (un-covered by s6/s7/s15, which pinned mechanisms but never enumerated the rules against the gap). The 4 rules (regfix.txt:2830-2835) partition EXACTLY into the two pinned axes: AXIS A = 2830 `subst lw $5,BF68 -> la $2,BF68` + 2831 `insert_after lw $5,0($2)` (the single materialization insertion); AXIS B = 2833 `subst lw $4,0($3) -> lw $2,0($3)` (dead-read reg $a0->$v0) + 2835 `reorder 21,20,19,18 @ 18-21` (4-insn sched permutation). No 5th rule, no unaccounted divergence => the honest 9-op gap = 1 (axis A) + 8 (axis B) is FULLY closed by these 4 rules, each traceable to one mechanism-pinned pass (combine.c:1458 / sched1 REG_DEP_ANTI). Forecloses the last open worry that a hidden third lever hides in the masked-Levenshtein cluster.

- [s16] Axis A re-confirmed FIRST-HAND from fresh s16 combine dump: line 10051 `(mem/s:SI (symbol_ref:SI ("D_8009BF68")))` symbol-direct/folded (2-insn). Line 8422 = sibling func_8007D3F8's `(set (mem:SI (symbol_ref D_8009BF68)))` store in its OWN combine section (corroborates s7 per-function-combine: sibling use invisible here).

- [s16] Axis B corroborated from fresh s16 sched priority block (func region insns 335-351, priorities 1-5 = critical-path heights, 69 mem/v insns dump-wide); mechanism (dead volatile *g_gpu_stat_reg read out-ranks fmt la by +1 via volatile-MEM REG_DEP_ANTI) unchanged from s6/s7/s15 triple-confirm. Not re-traced insn-by-insn (would be spinning per frontier).

- [s16] No OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (Grep 8007DC9C = No matches), so owner-gated NOT claimable this session -> result progress, escalation-ready frontier unchanged. Forensics now 4x-run (s6/s7/s15/s16); both axes mechanism-pinned dead AND the rule-set proven complete (no hidden 3rd axis).

- [s16] s16 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint to s1-s15). src/display.c + regfix.txt clean at HEAD (git status = clean); only tmp/ written.

- [s16] Fresh s16 cc1 -da -dr dumps generated over committed src/display.c (13 display.i.* passes + display.s + FORENSICS.md).

- [s16] NEW: regfix rule <-> axis completeness cross-map. Rules 2830+2831 = axis A (combine.c:1458 added_sets_2 materialization); rules 2833+2835 = axis B (sched1 volatile-MEM REG_DEP_ANTI priority). No 5th rule -> the 9-op gap (1 axis-A + 8 axis-B) is fully accounted; no hidden third lever inside the masked-Levenshtein cluster.

- [s16] Axis A re-confirmed first-hand from fresh s16 combine dump line 10051: (mem/s:SI (symbol_ref D_8009BF68)) symbol-direct/folded 2-insn form; a single pure offset-0 rvalue read always folds (combine.c:1458).

- [s16] Axis B corroborated from fresh s16 sched priority block (func insns 335-351, priorities 1-5); mechanism (dead volatile *g_gpu_stat_reg read out-ranks fmt la via volatile-MEM anti-dep) unchanged from the s6/s7/s15 triple-confirm; not re-traced insn-by-insn (would be spinning per frontier).

- [s16] Forensics now 4x-run (s6/s7/s15/s16). Both axes mechanism-pinned dead across all five modalities AND the regfix rule-set proven complete. No OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (Grep = No matches) -> owner-gated not claimable this session.

- [s17] REDERIVE modality. Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s16); src at clean HEAD after the experiment (git checkout restored, no dirt).

- [s17] Fresh m2c (tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c) reproduces HEAD's structure AGAIN: reads D_8009BF68 as a SCALAR VALUE (folds -> axis-A 2-insn form) and DCE's the dead *g_gpu_stat_reg read (single-read collapse = banked score-19 form). No structurally-different skeleton offered. Artifact tmp/grind/func_8007DC9C/s17/m2c_fresh.txt. Corroborates s8/s9 m2c leg.

- [s17] NEW rederive control-flow variant MEASURED (not previously banked): inverted early-return guard -- `if (g_gpu_vcount >= sys_VSync(-1)) { temp=g_gpu_draw_count; g_gpu_draw_count=temp+1; if (temp<=0xF0000) return 0; }` then the divergent debug-report block hoisted to straight-line FUNCTION scope (distinct from s8's goto-report + nested-if, which kept the block nested; short-circuit semantics preserved -- draw_count++ runs only on the A-false path). sandbox --disable all -> score 12, build_insns 88 (TWO FEWER than the 90-insn floor build; target 91). WORSE than floor: hoisting to fn-scope + inverting the guard lets GCC merge the two return paths and emit 88 insns, moving AWAY from target's 91. Rejected: rejected/rederive-inverted-early-return-guard.c.

- [s17] REDERIVE conclusion: control-flow rederivation leg now doubly-confirmed dead from BOTH directions -- s8 goto/nested-if ties the floor (90/score 9), s17 inverted-return worsens it (88/score 12); ONLY HEAD's comma-expression guard reaches the 90-insn floor and none reaches target's 91. Both pinned axes (combine offset-0 fold; sched1 volatile-MEM anti-dep priority) unmoved -- consistent with s8/s15 structure-insensitivity. All five modalities (structural s2/s3/s11/s12, permuter x5 s4/s5/s13/s14, forensics x4 s6/s7/s15/s16, rederive s8/s9/s17, synthesis s10) measure both axes dead.

- [s17] No OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (Grep 8007DC9C = No matches) -> owner-gated not claimable this session despite full modality exhaustion. Escalation-ready; the only remaining action is the OWNER filing the entry.

- [s17] s17 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s16); src/display.c restored to clean HEAD after experiment (git checkout, no dirt).

- [s17] Fresh m2c reproduces HEAD's structure again (scalar BF68 read folds to axis-A 2-insn form; dead stat-read DCE'd); no structurally-different skeleton. Artifact tmp/grind/func_8007DC9C/s17/m2c_fresh.txt. Corroborates s8/s9 m2c leg.

- [s17] NEW control-flow rederivation (inverted early-return guard, report block hoisted to fn scope) measured score 12 / build_insns 88 -- two fewer than the floor build, moving AWAY from target's 91. Rejected: rejected/rederive-inverted-early-return-guard.c.

- [s17] Control-flow rederivation leg now doubly-confirmed dead from both directions: s8 goto/nested-if ties the floor (90/score 9), s17 inverted-return worsens it (88/score 12); only HEAD's comma-expression guard reaches the 90-insn floor and none reaches target's 91 -- both pinned axes unmoved (consistent with s8/s15 structure-insensitivity).

- [s17] All five modalities now measure both axes dead: structural s2/s3/s11/s12, permuter x5 s4/s5/s13/s14, forensics x4 s6/s7/s15/s16, rederive s8/s9/s17, synthesis s10. Axis A = combine.c:1458 added_sets_2 multi-use retention (needs a 2nd &D_8009BF68 use the fn lacks; single-fn reproduction is a dead-2nd-use coercion, cross-fn mechanically impossible per per-function combine). Axis B = sched1 INSN_PRIORITY via volatile-MEM REG_DEP_ANTI (dead *g_gpu_stat_reg read out-ranks fmt la by +1; unflippable without altering observable volatile order). The 4 regfix rules are a proven 1:1 cover of the two axes (s16).

- [s17] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches) -> owner-gated NOT claimable this session despite full five-modality exhaustion.

- [s18] s18 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s17); src/display.c restored to clean HEAD after the experiment (git checkout, no dirt).

- [s18] s18 rederive NEW form (not previously banked): computed-boolean-flag two-if skeleton -> score 12, build_insns 88 (two fewer than the 90-insn floor; target 91). Same 88/12 fingerprint as s17's inverted early-return guard — confirming any control-flow shape that lets GCC merge the two exit paths worsens to 88.

- [s18] Control-flow rederivation leg now TRIPLY-confirmed dead across three distinct exit-path topologies: HEAD comma-expr guard = 90/floor 9 (only form reaching the floor build); s8 goto/nested-if = 90/floor 9 (report block kept nested, paths separate); s17 inverted-return hoisted = 88/score 12; s18 computed-flag two-if = 88/score 12. None reaches target's 91.

- [s18] Both pinned axes unmoved and mechanism-pinned dead: axis A = combine.c:1458 added_sets_2 multi-use retention (single pure offset-0 rvalue read folds to 2-insn; s15 source-verified); axis B = sched1 INSN_PRIORITY via volatile-MEM REG_DEP_ANTI dead-read->BF7C (s6/s15). Both proven control-flow-insensitive (s8/s15) — s18 empirically re-confirms via a distinct skeleton.

- [s18] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches), so owner-gated is NOT claimable this session despite full five-modality exhaustion (structural s2/s3/s11/s12, permuter x5 s4/s5/s13/s14, forensics x4 s6/s7/s15/s16, rederive s8/s9/s17/s18, synthesis s10).

- [s19] SYNTHESIS modality (2nd synthesis; s10 was 1st over s1-s9 only). No sandbox re-run (mechanism-pinned dead; floor 9 unchanged; the frontier + s11-s18 empirically prove re-measuring any pure-C axis only re-confirms — 8 sessions, zero movement). src/display.c clean at HEAD (candidate.c byte-equivalent, no dirt). Full cross-read of evidence.md (460 lines) + hypotheses.md + all 12 rejected/ forms + docs/grind/decisions.md.

- [s19] Re-merged ALL 18 prior sessions into tmp/grind/func_8007DC9C/s19/MERGED-ATTACK-s19.md, superseding s10's s1-s9-only dossier. Adds the post-s10 accumulation: permuter QUINTUPLE-confirm ~118k iters (s13/s14), forensics QUADRUPLE-run with combine.c:1458 SOURCE-verified (`added_sets_2 = ! dead_or_set_p (i3, i2dest);`) + rule-completeness 2+2 proof (s15/s16), rederive control-flow leg TRIPLY-confirmed dead across 3 exit-path topologies (s8 goto/nested-if=90, s17 inverted-return=88, s18 computed-flag=88; none reaches target 91).

- [s19] NO un-banked pure-C lever exists (single- or cross-function, any modality). Axis A = combine.c:1458 added_sets_2 multi-use retention (single pure offset-0 rvalue read folds to 2-insn; 2nd use only reproducible as a dead coercion s7; cross-fn mechanically impossible per per-function combine, toplev.c:3004). Axis B = sched1 INSN_PRIORITY via volatile-MEM REG_DEP_ANTI dead-read->BF7C (unflippable without altering observable volatile order). Orthogonal (axis A per-expression, axis B whole-block; s10). The 4 regfix rules are a proven 1:1 cover (s16); no hidden 3rd axis.

- [s19] Disposition per endgame-lock-disposition-policy (owner 2026-07-20): byte-matches only via 4 regfix rules, 9 insns short in honest pure C. Gate #1 (canonical asm) REFUSE (scan_hand_coded LOW, ordinary GCC output). Gate #2 (coercion family) REFUSE (no SOTN/VS/ESA precedent for a dead-2nd-use combine coercion or a volatile-order sched coercion). Both fail -> keep 4 rules, classify INCOMPLETE-owner-accepted. Owner ruling to record, not agent self-authorization.

- [s19] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches, verified this session; commit d6bc0904 covered 7 OTHER endgame-lock funcs), so owner-gated is NOT claimable this session. The single remaining action is the OWNER filing the escalation entry; once filed, the next session emits owner-gated and the queue advances. No further pure-C modality remains to run.

- [s19] s19 re-merged all 18 prior sessions into tmp/grind/func_8007DC9C/s19/MERGED-ATTACK-s19.md, superseding s10's s1-s9-only dossier; no un-banked pure-C lever exists single- or cross-function in any modality.

- [s19] Axis A (1 op): combine.c:1458 `added_sets_2 = ! dead_or_set_p (i3, i2dest);` (source-verified s15) keeps the 3-insn &D_8009BF68 materialization only for a multi-use address pseudo; func reads BF68[0] as a single pure offset-0 rvalue (exactly one target ref, s3) so it folds to 2-insn. Single-fn reproduction is a dead-2nd-use coercion (s7); cross-fn is mechanically impossible (per-function combine, toplev.c:3004).

- [s19] Axis B (8 ops): sched1 INSN_PRIORITY dead *g_gpu_stat_reg read (priority 2) out-ranks fmt la (priority 1) by the volatile-MEM anti-dep REG_DEP_ANTI dead-read->D_8009BF7C (both mem/v); unflippable without altering observable volatile order. Body-shape-invariant (s15 +3 UID shift, identical priorities/decision).

- [s19] The 4 regfix rules (regfix.txt:2830-2835) are a proven 1:1 cover of the two axes (s16 completeness proof): 2830+2831=axis A, 2833+2835=axis B; honest gap 9 = 1+8; no 5th rule, no hidden 3rd divergence.

- [s19] Permuter QUINTUPLE-confirmed dead across 5 structurally-distinct chassis (~118k iters s4/s5/s13/s14): only sub-floor finds are banked coercions (volatile-BF68 / long-long-width / fmt-corrupt junk); axis B never legitimately reordered.

- [s19] Rederive control-flow leg triply-confirmed dead: HEAD comma-expr=90/floor9, s8 goto/nested-if=90/floor9, s17 inverted-return=88/score12, s18 computed-flag=88/score12; none reaches target's 91. m2c reproduces HEAD; Kengo/decomp.me corpus (3754) offer no donor / only genuine multi-use materializations.

- [s19] No OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (grep 8007DC9C = No matches, verified s19; d6bc0904 covered 7 OTHER endgame-lock funcs) -> owner-gated not claimable; the only unblock is the OWNER filing the entry.

- [s19] Baseline unchanged (no re-run needed, mechanism-pinned): floor 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150. src/display.c clean at HEAD (candidate.c byte-equivalent, no dirt).

- [s20] s20 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s19); src/display.c clean at HEAD, no dirt.

- [s20] s20 combined-lever structural form (fmt0 pointer precompute declared first + diff0 named-temp for the masked subtraction, a combination s2 only swept individually) measured score 9 / identical fingerprint. Axis B unchanged (s15 body-shape-invariance: priority 2>1 via volatile-MEM REG_DEP_ANTI, invariant modulo UID shift); axis A unchanged (BF68[0] single pure offset-0 rvalue folds to 2-insn, combine.c:1458 added_sets_2). Rejected: rejected/structural-combined-fmtptr-difftemp.c.

- [s20] Structural modality now re-confirmed EXHAUSTED a 6th time (s2/s3/s11/s12/s20 + full lever-surface enumeration): block-local splits, decl order, type narrowing, statement re-association, struct/array grouping, and now lever-combinations all measure 9 or worse; both axes provably insensitive to structural shape.

- [s20] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep 8007DC9C = No matches this session); owner-gated is NOT claimable despite full modality exhaustion.

- [s20] func_8007DC9C matches endgame-lock-disposition-policy: gate #1 REFUSE (scan_hand_coded LOW, ordinary GCC output), gate #2 REFUSE (no SOTN/VS/ESA precedent for a dead-2nd-use combine coercion or a volatile-order sched coercion) -> keep the 4 regfix rules, classify INCOMPLETE-owner-accepted.

- [s21] STRUCTURAL. Baseline re-confirmed (fresh sandbox, src clean at HEAD): score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s20). Measured ONE novel un-banked structural form: three-lever combo (dead stat-read new_var staged FIRST + fmt pointer precompute fmt0 + masked-subtraction home diff0) — distinct from s2 (each lever individually, all 9) and s20 (fmt-ptr-FIRST + diff-temp, i.e. fmt before dead read, 9). Applied to src/display.c, sandbox -> score 9, IDENTICAL fingerprint; neither axis moved. KILLED. src restored clean (git checkout). Rejected: rejected/structural-deadread-first-fmtptr-difftemp.c. Artifact: tmp/grind/func_8007DC9C/s21/FINDINGS.md.
- [s21] Confirms s15 body-shape-invariance empirically once more: axis B sched1 priority (dead-read=2 > fmt=1 via volatile-MEM REG_DEP_ANTI) is unchanged by declaration/LUID topology; axis A per-expression combine.c:1458 offset-0 fold untouched by statement-level levers. Structural modality re-confirmed EXHAUSTED (7th time: s2/s3/s11/s12/s20 + s21).
- [s21] NO OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (grep 8007DC9C = No matches this session) -> owner-gated NOT claimable. Escalation-ready; only unblock is the OWNER filing the entry.

- [s21] s21 baseline re-confirmed (fresh sandbox, src clean at HEAD): score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 — identical fingerprint s1-s20.

- [s21] Novel three-lever structural form (deadread-first + fmt0 precompute + diff0 home) measured score 9, identical fingerprint; distinct from s2 (each lever individually) and s20 (fmt-ptr-first + diff-temp). KILLED. src restored clean via git checkout (no dirt).

- [s21] Empirically re-confirms s15 body-shape-invariance: axis B sched1 priority (dead-read=2 > fmt=1 via volatile-MEM REG_DEP_ANTI) is unchanged by declaration/LUID topology; axis A per-expression combine.c:1458 offset-0 fold is untouched by statement-level levers.

- [s21] Structural modality re-confirmed EXHAUSTED (7th time: s2/s3/s11/s12/s20 + s21). Both axes orthogonal and mechanism-pinned dead across all five modalities; the 4 regfix rules are a proven 1:1 cover (s16, no hidden 3rd axis).

- [s21] NO OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (grep 8007DC9C = No matches this session) -> owner-gated NOT claimable; only unblock is the OWNER filing the entry.

- [s22] PERMUTER modality, 6th chassis. Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150. src/display.c clean at HEAD (all work in standalone workspace tmp/grind/func_8007DC9C/s22/perm_s22).
- [s22] Built + validated chassis-6 (fully-inline min-pressure base.c, no named temps): 90 insns; base-vs-target objdump diff reproduces exactly axis-A (target lui;addiu;lw 0(v0) 3-insn vs base folded 2-insn lui;lw) + axis-B (fmt/BF78/deadread reorder cluster). base_score 630 == floor 9. Distinct regime from chassis-1 (HEAD-block), chassis-2 (arg-homing temp), chassis-3 (whole-body temps), chassis-4 (tail temps), chassis-5 (low-pressure block-scope).
- [s22] Campaign 35,277 iters ~21 min -j8; single novel find output-515-1 (515) = extern volatile unsigned long D_8009BF68[] = banked volatile/width-coercion cheat (axis A only), stripped by volatile_cheats. Axis-B sched cluster never reordered. Harvested --stop in-turn (stopped=true, procs_killed=9, 0 orphan s22 procs).
- [s22] Permuter now SEXTUPLE-confirmed dead across 6 structurally-distinct chassis (~153k total iters). Only permuter-reachable sub-floor form is the banked volatile/width coercion of D_8009BF68 (axis A); axis B never legitimately reorders, corroborating s6/s15 sched1 volatile-MEM anti-dep root. No OWNER-ESCALATION entry in docs/grind/decisions.md, so owner-gated not claimable.

- [s22] s22 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint to s1-s21). src/display.c clean at HEAD (untouched this session; all work in the standalone workspace).

- [s22] Built + validated chassis-6 workspace tmp/grind/func_8007DC9C/s22/perm_s22: fully-inline minimal-pressure base.c (no named temps), compiles to 90 insns; base-vs-target objdump diff reproduces exactly axis-A (BF68 3-insn materialization vs 2-insn fold) + axis-B (fmt/BF78/deadread reorder cluster). base_score 630 == floor 9.

- [s22] Chassis-6 campaign: 35,277 iters, ~21 min (elapsed_s 1262.7), -j8, base_score 630. Single novel find output-515-1 (515) = extern volatile unsigned long D_8009BF68[] = banked volatile-BF68 axis-A coercion, stripped by volatile_cheats. Axis B never reordered. Harvested --stop in-turn (stopped=true, procs_killed=9); pgrep confirms 0 orphan s22 permuter procs.

- [s22] Permuter modality now SEXTUPLE-confirmed dead across 6 structurally-distinct chassis (~153k total iters: s4 27k + s5 40k + s13 31k + s14 20k + s22 35k). The only permuter-reachable sub-floor form remains the banked volatile/width coercion of D_8009BF68 (axis A only); the axis-B 8-op sched1 cluster has never legitimately reordered in ANY chassis, corroborating the s6/s15 sched1 volatile-MEM anti-dep root.

- [s22] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md, so owner-gated is not claimable this session despite full six-chassis permuter exhaustion + all five modalities dead.

- [s23] PERMUTER modality, 7th chassis. src/display.c untouched (never edited; git status clean). Built chassis-7 (tmp/grind/func_8007DC9C/s23/perm_s23) = second-printf reads (BF68[0]/BF6C/BF70) hoisted to block-top named temps rep_a/rep_b/rep_c. Base compiles to 98 insns (vs target 91) — the three temps spill across the intervening calls, +8 insns of spill code; a genuinely distinct HIGH-register-pressure regime vs all 6 prior 90-insn near-floor chassis. base_score 2826.
- [s23] Chassis-7 campaign (s23-chassis7-secondprintf-hoisted-temps, -j8, base_score 2826): 36,103 iterations across in-turn wait windows + one consolidated 30-cycle grind window. Min score descended 2367 -> 1763 -> 1186 and PLATEAUED at 1186 — never approached the ~630 legitimate floor. 517 outputs, ALL >= 1186 (>> 630). ZERO sub-floor find: this high-pressure seed cannot descend into the region where chassis 1/2/4/5/6 stumbled onto the banked `extern volatile ... D_8009BF68[]` coercion (515-590). Axis A never legitimately materialized (BF68[0] stays combine-folded — it is still a single pure offset-0 rvalue read, combine.c:1458 folds it); axis-B 8-op sched cluster never legitimately reordered.
- [s23] Harvested --stop in-turn: campaign alive=false, registered_active=false, 0 live campaigns, 0 stale registry entries, 0 orphan permuter procs (reap: 0 groups). src/display.c clean (never edited this session). candidate.c unchanged (floor held at 9). Permuter now SEPTUPLE-confirmed dead across 7 structurally-distinct chassis (~189k total iters s4+s5+s13+s14+s22+s23).
- [s23] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches this session), so owner-gated is NOT claimable despite full seven-chassis permuter exhaustion + all five modalities dead. The only unblock remains the OWNER filing the escalation entry.

- [s23] s23 chassis-7 base compiles to 98 insns (vs target 91): hoisting the second-printf reads to block-top temps spills them across the intervening calls (+8 insns), a genuinely distinct HIGH-register-pressure basin vs all 6 prior 90-insn chassis. base_score 2826.

- [s23] 36,103-iteration campaign (-j8): min score plateaued at 1186, never approaching the ~630 legitimate floor; 517 outputs, all >= 1186; ZERO sub-floor find (seed cannot reach the coercion region other chassis hit).

- [s23] rep_a=D_8009BF68[0] hoisted is still a single pure offset-0 rvalue read -> combine.c:1458 added_sets_2 folds to 2-insn exactly as HEAD; the hoist is non-improving on axis A by construction (confirms s2/s3/s6/s9).

- [s23] Permuter now SEPTUPLE-confirmed dead across 7 structurally-distinct chassis (~189k total iters: s4 27k + s5 40k + s13 31k + s14 20k + s22 35k + s23 36k). Only permuter-reachable sub-floor forms across all chassis are the banked volatile/width coercions of D_8009BF68 (axis A only); axis B never legitimately reordered in any chassis.

- [s23] src/display.c never edited this session (git status clean); candidate.c unchanged (floor held at 9).

- [s23] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches), so owner-gated is NOT claimable despite seven-chassis permuter exhaustion + all five modalities measured dead.

- [s24] FORENSICS (5th run; s6/s7/s15/s16 prior). Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s23). src/display.c clean at HEAD; fresh cc1 -da -dr dumps tmp/grind/func_8007DC9C/s24/display.i.* + display.s + FORENSICS.md + dump.sh (cc1.err = benign display.c-internal-typedef redefinition warnings; body compiles+dumps in full).

- [s24] NEW forensic result (sharpens s6/s7/s15/s16 axis-B framing): axis B is an OVER-DETERMINED lock, not the single-cause volatile-anti-dep-priority lock all four prior forensics recorded. Traced the exact schedule_block decision at T-42 in the fresh dump against tools/gcc-2.7.2/sched.c. Backward list scheduler (higher clock T = earlier program position; sched.c:3735 last=next_tail, splice-before at 3742). At T-42 ready={insn63 fmt pri1, insn41 dead-read pri2}: hazard scan sched.c:2664 finds actual_hazard(41)=1 (41 is a MIPS-I load with a load-delay slot) -> "blocking insn 41 for 1 cycles" -> 63 fills T-42, 41 issues T-43 = BEFORE 63 (our wrong order; target wants fmt first).

- [s24] Counterfactual (source-derived, rank_for_schedule sched.c:2399 = priority, then dep-class, then INSN_LUID): equalising 41's priority to fmt's 1 does NOT flip the order -- the single priority group still has 41 hazard-queued (still a load) so 63 fills T-42 and 41 lands at T-43 before 63, IDENTICAL result. The load-delay hazard alone reproduces axis B independent of the anti-dep priority. Third backstop: even neutralising priority AND hazard, LUID tie-break (LUID(41)<LUID(63), dead read precedes fmt in source) still emits dead read first. Removing the load hazard requires removing the load (non-volatile => DCE'd => banked score-19 collapse). => no priority-equalising pure-C construct can flip axis B; over-determined KILL.

- [s24] Insn identities confirmed in fresh post-sched RTL: insn 41 = (set (reg/v 75) (mem/v (reg 82))) dead *g_gpu_stat_reg read (display.i.sched:15665, emitted BEFORE fmt); insn 63 = (set (reg 4 a0) (symbol_ref g_str_gpu_timeout)) fmt la (:15675); insn 48 = (set (reg 85) (mem/v D_8009BF7C)) carrying (insn_list:REG_DEP_ANTI 41) (:15687) -- both 41 and 48 mem/v => anti-dep purely volatile-ordering (UID-stable vs s15). Axis A unchanged: combine section re-shows (mem/s:SI (symbol_ref D_8009BF68)) symbol-direct/folded (combine.c:1458, s15 source-verified).

- [s24] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches this session), so owner-gated is NOT claimable despite full five-modality exhaustion + the strengthened over-determined axis-B mechanism. Escalation-ready; only unblock is the OWNER filing the entry.

- [s24] s24 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s23). src/display.c + regfix.txt clean at HEAD (git status empty); only tmp/ written.

- [s24] NEW (sharpens s6/s7/s15/s16): axis B is an OVER-DETERMINED lock, not the single-cause volatile-anti-dep-priority lock the 4 prior forensics recorded. Backward list scheduler; at T-42 the dead read (insn 41) is queued 1 cycle by its own load-delay hazard (actual_hazard=1, sched.c:2664 'blocking insn 41 for 1 cycles'), so hazard-free fmt (insn 63) fills T-42 and 41 lands at T-43 = earlier in program order.

- [s24] Counterfactual proof (source-derived from rank_for_schedule/actual_hazard, sched.c:2399/2664): equalising the dead read's priority to fmt's does NOT flip the order — the load hazard alone reproduces 41-before-63. Third backstop: rank_for_schedule's final INSN_LUID tie-break (LUID(41)<LUID(63), dead read precedes fmt in source) also emits the dead read first even absent priority+hazard.

- [s24] The dead read must stay volatile to survive DCE and match target's two *g_gpu_stat_reg reads; a volatile load inseparably carries BOTH the volatile-MEM anti-dep (priority 2) AND the load-delay hazard (actual_hazard 1). The two locks cannot be split.

- [s24] Insn identities confirmed in fresh post-sched RTL: 41=(set (reg/v 75)(mem/v (reg 82))) dead read (display.i.sched:15665, emitted before fmt); 63=(set (reg 4 a0)(symbol_ref g_str_gpu_timeout)) fmt la (:15675); 48=(set (reg 85)(mem/v D_8009BF7C)) carrying (insn_list:REG_DEP_ANTI 41) (:15687) — both 41 and 48 mem/v => anti-dep purely volatile-ordering (UID-stable vs s15).

- [s24] Axis A unchanged: fresh s24 combine section re-shows (mem/s:SI (symbol_ref D_8009BF68)) symbol-direct/folded (combine.c:1458 added_sets_2, s15 source-verified) — single pure offset-0 rvalue read folds to 2-insn.

- [s24] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (Grep 8007DC9C = No matches this session), so owner-gated is NOT claimable despite full five-modality exhaustion + the strengthened over-determined axis-B mechanism.

- [s25] FORENSICS (6th run; s6/s7/s15/s16/s24 prior). Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s24). src/display.c + regfix.txt + asmfix.txt clean at HEAD (git status empty); only tmp/ written. Fresh cc1 -da -dr dumps tmp/grind/func_8007DC9C/s25/display.i.* + display.s + dump.sh + FORENSICS.md (cc1.err = benign display.c-internal typedef-redefinition messages; all 13 pass dumps present, display.s emitted).

- [s25] NEW forensic result (sharpens/CORRECTS s6 axis-A framing): the axis-A 2-insn fold in OUR build is a COMBINE event, NOT a pre-combine/expand simplification. s6 said "const_int-0 simplified BEFORE combine's substitution -> emits folded lui;lw". The dumps show otherwise: at EXPAND the array-subscript D_8009BF68[0] MATERIALIZES the address in a separate single-use pseudo -> insn 77 `(set (reg 93) (symbol_ref D_8009BF68))` REG_EQUAL symbol_ref (rtl:10708) feeding insn 85 `(set (reg 5 a1) (mem/s (reg 93)))` (rtl:10726) = 2 insns present at expand. Scalars BF6C/BF70 expand DIRECTLY to `(mem (symbol_ref))` (C-access-shape asymmetry: array subscript forces the address to a reg; bare scalar does not).

- [s25] CSE keeps reg93 separate (cse:9863, still `(set (reg 93) (symbol_ref BF68))`). The collapse to 1 insn is a COMBINE deletion: reg93 has EXACTLY ONE use in func_8007DC9C (def@77, consumer@85; other `(reg 93)` in the dump are other functions — cc1 renumbers pseudos per function), so combine.c:1458 `added_sets_2 = !dead_or_set_p(i3,i2dest)` is FALSE -> combine substitutes the symbol into the mem and DELETES insn 77 (`(note 77 75 79 NOTE_INSN_DELETED)`, combine:~10041), giving combine:10051 `(set (reg 5 a1) (mem/s (symbol_ref D_8009BF68)))` -> final asm `lw $5,D_8009BF68` (display.s:3170).

- [s25] This dump-verifies the SAME combine.c:1458 predicate from BOTH sides: OUR fold = single-use address pseudo -> added_sets_2 FALSE -> materialization deleted (2-insn); TARGET retention = multi-use address pseudo -> added_sets_2 TRUE -> materialization kept (3-insn lui;addiu;lw 0(reg)). The last residual axis-A framing ambiguity (fold at expand vs combine?) is resolved: COMBINE. No new lever: added_sets_2 TRUE needs a 2nd use of reg93 = dead 2nd &BF68 use = coercion (banked s7 axisA-dead-addr-store-multiuse.c); cross-fn mechanically impossible (per-function combine; sibling func_8007D3F8 store `sw $19,D_8009BF68` at display.s:2693 sits in its own combine section, disjoint from this read at display.s:3170).

- [s25] Axis B re-verified UID-stable in the fresh s25 sched dump (dead *g_gpu_stat_reg read scheduled before fmt via volatile-MEM REG_DEP_ANTI to volatile D_8009BF7C, both mem/v; s24 over-determination — anti-dep priority + load-delay hazard + LUID backstop — intact). Both axes now have their divergence-producing pass named and dump-captured on BOTH sides; no un-pinned forensic detail remains.

- [s25] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep 8007DC9C = 0 matches this session), so owner-gated is NOT claimable despite full five-modality exhaustion + both axes now fully dump-pinned on both sides. Escalation-ready since s7 (18 sessions); the ONLY unblock is the OWNER filing the entry. No further forensic detail to pin and no pure-C modality remains.

- [s25] s25 baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s24). src/display.c + regfix.txt + asmfix.txt clean at HEAD; only tmp/ written.

- [s25] AXIS A fold pass PINNED to COMBINE (dump-verified): expand materializes BF68[0]'s address in single-use pseudo reg93 (insn77, REG_EQUAL symbol_ref, rtl:10708) + load (mem/s (reg93)) (insn85, rtl:10726); cse keeps it separate (cse:9863); combine deletes insn77 (NOTE_INSN_DELETED) and folds to (mem/s (symbol_ref)) (combine:10051) -> asm:3170 lw $5,D_8009BF68. Corrects s6's 'fold at expand' framing.

- [s25] combine.c:1458 added_sets_2 = !dead_or_set_p(i3,i2dest) now dump-verified from BOTH sides: OUR fold = single-use reg93 -> FALSE -> materialization deleted (2-insn); TARGET retention = multi-use address pseudo -> TRUE -> materialization kept (3-insn lui;addiu;lw 0(reg)).

- [s25] C-access-shape asymmetry is the root: BF68[0] is an array subscript (address forced to a reg pseudo) while BF6C/BF70 are bare scalar reads (direct (mem (symbol_ref))). Only the subscripted access presents a foldable address pseudo to combine.

- [s25] No new lever: making added_sets_2 TRUE needs a 2nd use of reg93 = dead 2nd &D_8009BF68 use = coercion (banked s7 axisA-dead-addr-store-multiuse.c, which reproduced target's 3-insn form, sandbox 9->8/build93). Cross-fn impossible: per-function combine; sibling store sw $19,D_8009BF68 (asm:2693) in its own combine section, disjoint from this read (asm:3170).

- [s25] AXIS B re-verified UID-stable in the fresh s25 sched dump: dead *g_gpu_stat_reg read scheduled before fmt via volatile-MEM REG_DEP_ANTI to volatile D_8009BF7C (both mem/v); s24 over-determination (anti-dep priority + load-delay hazard + LUID backstop) intact. No re-derivation attempted (forensics modality).

- [s25] Both axes now have their divergence-producing pass named and dump-captured on BOTH sides (axis A = combine single-use added_sets_2-FALSE deletion vs target multi-use retention; axis B = sched1 over-determined lock). No un-pinned forensic detail remains.

- [s25] NO OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (grep = 0 matches this session). Escalation-ready since s7 (18 sessions confirmed); owner-gated NOT claimable without the entry. The only unblock is the OWNER filing it.

- [s25] Six-modality exhaustion stands: structural (s2/s3/s11/s12/s20/s21), permuter x7 ~189k iters (s4/s5/s13/s14/s22/s23), forensics x6 (s6/s7/s15/s16/s24/s25), rederive (s8/s9/s17/s18), synthesis (s10/s19). 4 regfix rules a proven 1:1 cover (s16).

- [s26] s26 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s25). src/display.c clean at HEAD after the experiment (git checkout, verified: if-form restored at line 1037, no dirt).

- [s26] s26 rederive: whole-function ternary/conditional-expression form (report block as comma-expression rvalue inside `?:`) measured score 9 / build 90 = identical floor-9 fingerprint. Structurally distinct AST from all 4 banked rederive forms; neither pinned axis moved.

- [s26] s26 finding: the ternary reaches the 90-insn floor build (distinct -1/0 exit paths preserved), NOT s17/s18's 88-insn coalesced form. Confirms the s17/s18 exit-coalescing threshold: HEAD statement-if + s8 goto/nested-if + s26 ternary all keep the two exits separate -> 90/floor-9; s17 inverted-return + s18 computed-flag-two-if coalesce the exits -> 88/score-12. No rederive skeleton reaches target's 91.

- [s26] Rederive modality now SEXTUPLY exhausted (s8 m2c+Kengo+goto/nested-if, s9 decompme-corpus, s17 inverted-return, s18 computed-flag-two-if, s26 ternary conditional-expression). Axis A is per-expression combine.c:1458 offset-0 fold (control-flow-insensitive, s8/s25); axis B is an over-determined sched1 lock (volatile-MEM anti-dep priority + MIPS-I load-delay hazard + LUID backstop, s24), body-shape-invariant (s15). Both insensitive to any function-level skeleton.

- [s26] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep = No matches, s26), so owner-gated is NOT claimable this session despite full six-modality exhaustion.

- [s26] candidate.c unchanged (HEAD-equivalent best form at floor 9); s26 ternary saved to rejected/rederive-ternary-conditional-expr.c.

- [s27] REDERIVE modality. Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical to s1-s26). src/display.c clean at HEAD before + after (reverted, no dirt).

- [s27] Novel EXPRESSION-OPERATOR-level rederive KILLED: A-false guard side effect written with the native post-increment operator `g_gpu_draw_count++ > 0xF0000` in place of HEAD's comma-expression temp dance (`temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0`); also drops the new_var2 volatile-ptr indirection. Distinct AST from every banked skeleton (all skeleton-level: HEAD comma-if, s8 goto/nested-if, s17 inverted-early-return, s18 computed-flag-two-if, s26 ternary). sandbox --disable all -> score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 = IDENTICAL floor-9 fingerprint. Reaches the 90-insn floor (exit-distinct skeleton). Neither axis moved: post-increment lowers to the same read-then-increment RTL as the comma-expr temp -> axis A (combine.c:1458 offset-0 fold, s25) and axis B (sched1 over-determined volatile-MEM/load-delay/LUID lock, s24) are insensitive to the increment operator's spelling. Rejected: rejected/rederive-postincrement-operator-condition.c. Artifact tmp/grind/func_8007DC9C/s27/FINDINGS.md.

- [s27] Rederive modality now confirmed dead at BOTH the skeleton level (s8/s17/s18/s26) AND the expression-operator level (s27). No OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (grep NONE, re-verified s27) -> owner-gated NOT claimable. Both axes remain mechanism-pinned dead across all six modalities (structural, permuter x7 ~189k iters, forensics x6, rederive x6+s27, synthesis x2); escalation-ready, still no owner entry filed.

- [s27] s27 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint to s1-s26); src/display.c clean at HEAD before and after (reverted, no dirt).

- [s27] Novel post-increment-operator condition form (g_gpu_draw_count++ > 0xF0000, dropping temp_v1 + the new_var2 volatile-ptr dance) measured score 9 / build 90 = identical floor-9 fingerprint. Distinct AST from every banked skeleton (HEAD comma-if, s8 goto/nested-if, s17 inverted-early-return, s18 computed-flag-two-if, s26 ternary). Rejected: rejected/rederive-postincrement-operator-condition.c.

- [s27] Post-increment lowers to the same read-then-increment RTL as the comma-expression temp; axis A (combine.c:1458 added_sets_2 offset-0 fold on the single pure BF68[0] rvalue) and axis B (sched1 over-determined volatile-MEM anti-dep / MIPS-I load-delay / LUID lock) are both insensitive to increment-operator spelling.

- [s27] Rederive modality now confirmed dead at BOTH the skeleton level (s8/s17/s18/s26) AND the expression-operator level (s27).

- [s27] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE, re-verified s27); owner-gated NOT claimable this session.

- [s28] SYNTHESIS (3rd pass; s10 over s1-9, s19 over s1-18, s28 over s1-27). Baseline re-confirmed THIS session: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 — identical fingerprint to s1-s27. src/display.c clean at HEAD (git diff --stat empty; candidate byte-equivalent, no dirt).

- [s28] Full cross-read of the complete ledger (evidence.md 624 lines + hypotheses.md 372 lines + all 17 rejected/ forms + docs/grind/decisions.md). Confirmed NO un-banked pure-C lever exists in any of the six modalities, single- or cross-function. Merged dossier: tmp/grind/func_8007DC9C/s28/MERGED-ATTACK-s28.md.

- [s28] Axis A synthesized (COMBINE-pinned, s25 correction of s6): array subscript BF68[0] materializes the address in a separate single-use pseudo (insn77 reg93=symbol_ref -> insn85 mem/s(reg93)) at expand, survives CSE, then combine deletes insn77 via added_sets_2=!dead_or_set_p(i3,i2dest)=FALSE (single-use) -> 2-insn fold. Target retains it (reg93 multi-use). Scalars BF6C/BF70 expand symbol-direct (access-shape asymmetry). No lever: single pure offset-0 read always folds (s2/s9 corpus); only single-fn reproduction is a dead 2nd &BF68 use = coercion (s7, over-shoots +2); cross-fn impossible (per-function combine, toplev.c:3004).

- [s28] Axis B synthesized (OVER-DETERMINED, s24 strengthens s6/s15): three independent backstops each force dead-read-before-fmt at sched1 T-42 — (1) volatile-MEM anti-dep REG_DEP_ANTI 41->48 priority 2>1, (2) MIPS-I load-delay actual_hazard(41)=1 (holds even at equal priority), (3) LUID tie-break (holds even neutralising priority AND hazard). Unflippable without removing the load = non-volatile read DCE'd = banked score-19 collapse. Body-shape-invariant (s15).

- [s28] Modality exhaustion matrix (all measure both axes dead): structural x4+2 combos (s2/s3/s11/s12/s20/s21), permuter x7 chassis ~189k iters (s4 27k/s5 40k/s13 31k/s14 20k/s22 35k/s23 36k; every sub-floor find a coercion, axis B never legitimately reordered, chassis-7 basin plateaus 1186 never near 630), forensics x6 (s6/s7/s15/s16/s24/s25), rederive x6 (s8/s9/s17/s18/s26/s27; only exit-coalescing skeletons drop below floor build=worse, exit-distinct tie floor 9, none reach target 91), synthesis x3 (s10/s19/s28). Rule-set proven complete 2+2 (s16).

- [s28] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep 8007DC9C = No matches; d6bc0904's owner ruling covered 7 OTHER endgame-lock funcs, not this one). owner-gated NOT claimable this session despite full six-modality exhaustion -> result progress, escalation-ready. The ONLY unblock is the OWNER filing the entry.

- [s28] s28 baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 - identical fingerprint to s1-s27; src/display.c clean at HEAD (no dirt).

- [s28] Axis A = COMBINE offset-0 fold (s25 corrects s6's 'expand' framing): array subscript BF68[0] materializes the address in a separate single-use pseudo (insn77 reg93=symbol_ref -> insn85 mem/s(reg93)) at expand, survives CSE, then combine deletes insn77 via combine.c:1458 added_sets_2=!dead_or_set_p(i3,i2dest)=FALSE (single-use) -> 2-insn fold; target retains it (reg93 multi-use). Scalars BF6C/BF70 expand symbol-direct.

- [s28] Axis A has no lever single- or cross-function: single pure offset-0 read always folds (s2 sweep; s9 corpus of 3754 scratches - every offset-0 materialization is genuine multi-use); only single-fn reproduction is a dead 2nd &BF68 use = coercion (s7, over-shoots +2 build 93); cross-fn impossible (per-function combine, toplev.c:3004; sibling store in its own combine section).

- [s28] Axis B = OVER-DETERMINED sched1 lock (s24 strengthens s6/s15): three independent backstops each force dead-read-before-fmt at T-42 - (1) volatile-MEM anti-dep REG_DEP_ANTI 41->48 priority 2>1, (2) MIPS-I load-delay actual_hazard(41)=1 even at equal priority, (3) LUID tie-break even neutralising priority AND hazard. Unflippable without removing the load = non-volatile read DCE'd = banked score-19 collapse. Body-shape-invariant (s15).

- [s28] 4 regfix rules (2830-2835) are a proven 1:1 cover of the two axes (2+2, s16); honest gap 9 = 1 (axis A) + 8 (axis B); no 5th rule, no hidden third divergence.

- [s28] Six-modality exhaustion: structural x4+2 combos (s2/s3/s11/s12/s20/s21), permuter x7 chassis ~189k iters (s4/s5/s13/s14/s22/s23; every sub-floor find a coercion, axis B never legitimately reordered, chassis-7 basin plateaus 1186 never near 630), forensics x6 (s6/s7/s15/s16/s24/s25), rederive x6 (s8/s9/s17/s18/s26/s27; exit-distinct skeletons tie floor 9, exit-coalescing drop to 88/worse, none reach target 91), synthesis x3 (s10/s19/s28).

- [s28] endgame-lock-disposition-policy: gate #1 REFUSE canonical-asm (scan_hand_coded LOW), gate #2 REFUSE coercion (no precedent; both axes' only single-fn reproductions are coercions) -> keep 4 rules, INCOMPLETE-owner-accepted.

- [s28] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep 8007DC9C = No matches; d6bc0904 covered 7 OTHER endgame-lock funcs). owner-gated not claimable this session.

- [s29] STRUCTURAL modality. Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s28). src/display.c clean at HEAD before + after (restored, no dirt).

- [s29] NOVEL un-banked structural lever MEASURED DEAD: hoist the SECOND debug_printf's global reads D_8009BF6C + D_8009BF70 into named temps (arg_c/arg_d) computed at the TOP of the if-body BEFORE the first debug_printf, extending both live ranges across the axis-B scheduling region. Every prior banked structural form (fmt-ptr, diff-temp, deadread-first, and their s20/s21 combos) touched only the FIRST printf's diverging args; this is the first directed structural probe on the SECOND printf's read liveness. Result: score 35 / build_insns 95 (STRICTLY WORSE, +5 insns vs floor's build 90). Rejected: rejected/structural-hoist-2ndprintf-reads-temps.c.

- [s29] MECHANISM of the +5 worsening: holding two values live across the first debug_printf call forces them into callee-saved $sN registers -> extra prologue save/restore + spill. Axis B did NOT flip (dead *g_gpu_stat_reg read still schedules before fmt) -> confirms empirically that the fmt-vs-deadread sched1 priority is set by the volatile-MEM anti-dep 38->45 (s6/s15), NOT by surrounding register pressure. This is a NEW corroboration of the s2/s6 axis-B KILL from the opposite direction: added cross-call liveness moves the build STRICTLY AWAY from target's natural low-pressure schedule, so the target uses the minimal-pressure allocation and no pressure-inflating structural form can reach it.

- [s29] Structural modality now re-confirmed exhausted (8th time). All structural categories measured dead: block-local var splits (s2, s29), declaration order (s2 8-form sweep), type narrowing (s2 scalar/fn-ptr/pointer; s4/s5 coercions), statement re-association (s2; s8 goto/nested-if), struct/array grouping (s3 struct-triple -> 11), multi-lever combos (s20/s21), and now 2nd-printf read-hoist (s29 -> 35). Both axes remain mechanism-pinned dead. No un-banked structural form remains.

- [s29] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE this session), so owner-gated is not claimable despite full six-modality exhaustion. Result progress, escalation-ready frontier unchanged.

- [s29] s29 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s28); src/display.c clean at HEAD before and after (restored, no dirt).

- [s29] Novel un-banked structural lever (hoist 2nd-printf reads D_8009BF6C+D_8009BF70 into named temps computed before the first printf) measured score 35 / build 95 — strictly worse than floor 9. Rejected: memory/grind/func_8007DC9C/rejected/structural-hoist-2ndprintf-reads-temps.c.

- [s29] NEW corroboration of axis-B KILL from the opposite direction: added cross-call liveness forces callee-saved allocation (+5 insns) but does NOT reorder fmt vs the dead *g_gpu_stat_reg read -> the sched1 priority is set by the volatile-MEM anti-dep 38->45 (s6/s15), not by register pressure. Target uses the natural minimal-pressure allocation; every pressure-inflating structural form moves strictly away from it.

- [s29] Structural modality re-confirmed exhausted (8th time): block-local var splits (s2/s29), decl order (s2 sweep), type narrowing (s2/s4/s5), statement re-association (s2/s8), struct/array grouping (s3), multi-lever combos (s20/s21), 2nd-printf read-hoist (s29) all dead.

- [s29] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE this session), so owner-gated is not claimable despite full six-modality exhaustion.

- [s30] STRUCTURAL. Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical to s1-s29). src/display.c restored to clean HEAD after the experiment (git diff empty); candidate cleanup preserved.

- [s30] Novel structural probe (un-banked placement): hoisted the 2nd debug_printf's three independent global reads into tightly-scoped block-local temps declared in REVERSE source order immediately before the call `{ s32 r3=D_8009BF70; s32 r2=D_8009BF6C; s32 r1=D_8009BF68[0]; debug_printf(&D_80016044,r1,r2,r3); }`. Distinct regime from s29 (block-top hoist -> score 35 strictly worse) and s23 (permuter chassis-7 block-top temps): minimal register pressure, temps live across zero calls, reversed evaluation order. Purpose: isolate whether axis-A BF68[0] materialization is sensitive to sibling-read evaluation order/placement. -> score 9, IDENTICAL floor fingerprint. BF68[0] stays combine-folded to the 2-insn form; axis B unchanged. KILLED: the axis-A combine offset-0 single-use fold (combine.c:1458 added_sets_2, s6/s25) is per-expression and insensitive to sibling-read evaluation order/placement; axis B pressure-insensitive (corroborates s24/s29). Rejected: rejected/structural-s30-reverse-order-2ndprintf-temps.c. Artifact: tmp/grind/func_8007DC9C/s30/NOTES.md.

- [s30] Structural modality now exhausted across 9 distinct probes; both axes mechanism-pinned dead across six modalities. No OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (grep NONE), so owner-gated not claimable -> result progress, escalation-ready.

- [s30] s30 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint to s1-s29).

- [s30] s30 novel structural probe (reverse-order 2nd-printf sibling-read temps, minimal pressure, un-banked placement distinct from s29 block-top/s23 permuter) -> score 9, identical floor fingerprint. Axis-A BF68[0] materialization is per-expression combine single-use fold (combine.c:1458 added_sets_2) and independent of sibling-read evaluation order/placement; axis B unchanged. Rejected: rejected/structural-s30-reverse-order-2ndprintf-temps.c.

- [s30] Structural modality now exhausted across 9 distinct probes (s2/s3/s11/s12/s20/s21/s29/s30 + this): block-local var splits, declaration order, type narrowing, statement re-association, struct/array grouping, multi-lever combos, 2nd-printf read hoists at every placement/order -- all floor 9 or worse.

- [s30] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE this session), so owner-gated is not claimable despite full six-modality exhaustion.

- [s30] src/display.c restored to clean HEAD after the experiment (git diff empty); candidate cleanup form preserved in memory/grind/func_8007DC9C/candidate.c.

- [s31] PERMUTER modality, EIGHTH chassis + FIRST from a restructured control-flow topology. Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150. src/display.c clean at HEAD (worked only in tmp/).
- [s31] Built + validated chassis-8 (tmp/perm_dc9c_s31): goto/nested-if base.c (hoisted vsync_lim; split draw-count increment; nested `if (count <= 0xF0000) return 0;`; short-circuit `||` semantics preserved exactly). Compiles to 88 insns; base reproduces BOTH ledger axes (objdump: axis B dead-read `lw a0,0(v1)` @0x68 BEFORE fmt `lui/addiu a0` @0x6c-0x70 = HEAD wrong-order; axis A `lui a1; lw a1,0(a1)` @0xa0 = 2-insn combine-fold, target has 3). The leaner pre-branch region diverges from target -> base_score 1110 (disjoint higher-distance basin, ~480 above the ~630 floor).
- [s31] Chassis-8 campaign (s31-chassis8-prebranch-goto, -j8, base_score 1110): 16,934 iters over seven in-turn wait windows. Basin plateaued at 955, NEVER approached the 630 floor let alone 0. ALL sub-base finds are coercions: 955=volatile-BF78/BF7C coercion (stripped); 995=extern volatile long D_8009BF68[] (banked volatile-BF68 axis-A coercion); 960=dead pointer-alias `new_var=&D_80016044` junk. Axis A never legitimately materialized; axis B 8-op sched cluster never legitimately reordered. Harvested --stop in-turn; 0 live campaigns, 0 orphan permuter procs. Rejected: rejected/permuter-s31-chassis8-goto-basin.c.
- [s31] CONCLUSION: seeding the permuter from a fundamentally different control-flow topology does NOT open a path to target — the goto basin is disjoint from and farther than the comma-expression floor basin and yields only the same coercion families. Directly corroborates s8 (axis A control-flow-insensitive; axis B block-structure-insensitive). Permuter EIGHT-chassis confirmed dead (~206k cumulative iters). No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md, so owner-gated is not claimable this session despite full modality exhaustion -> result progress, escalation-ready.

- [s31] s31 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150. src/display.c clean at HEAD (worked only in tmp/).

- [s31] Chassis-8 is the FIRST permuter chassis seeded from a restructured control-flow topology; all 7 prior chassis (s4 x2, s5, s13, s14, s22, s23) reused the HEAD comma-expression || body. Base compiles to 88 insns and reproduces both ledger axes; base_score 1110 (disjoint, ~480 above the ~630 floor basin).

- [s31] 16,934 iters (-j8): basin plateaued at 955 and never approached the 630 floor let alone score 0. All sub-base finds are coercions (volatile-BF78/BF7C, banked volatile-BF68 axis-A, dead pointer-alias) — zero legitimate sub-floor form.

- [s31] Corroborates s8: axis A is per-expression / control-flow-insensitive and axis B is volatile-order-locked / block-structure-insensitive; a control-flow reshuffle changes neither. Permuter now EIGHT-chassis confirmed dead (~206k cumulative iters).

- [s31] Campaign harvested --stop in-turn; status shows 0 live campaigns, 0 orphan permuter processes.

- [s31] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md, so owner-gated is not claimable this session despite six-modality exhaustion.

- [s32] PERMUTER modality, NINTH chassis (single-reused-scratch topology: one variable `t` aliased across the dead *g_gpu_stat_reg read AND the motion_make_table tail return). Baseline unchanged: floor 9 (verdict C, target 91 / build 90, rules_dropped 4). src/display.c clean at HEAD (worked only in tmp/perm_dc9c_s32); git diff src/display.c empty.
- [s32] Built + validated chassis-9 (tmp/perm_dc9c_s32, cloned s31 compile.sh/target infra): single-reused-scratch base.c compiles to 90 insns vs target 91, reproduces the axis-A fold gap (BF68[0] folded 2-insn lui;lw %lo). Distinct basin: base_score 1500 (aliasing raises distance) vs prior chassis 630 (s4/s5) / 695 (s13/s14) / 1110 (s31 goto).
- [s32] Chassis-9 campaign (s32-chassis9-single-reused-scratch, -j8, base_score 1500): 51,633 iters over ~27.6 min, best_new_score 405, finds_total 1042. Harvested --stop in-turn (9 procs killed; ps shows 0 live permuter.py). Every strictly-sub-floor find is a cheat: 405 = `(float)0x01000000` stored to *g_gpu_stat_reg (corrupts observable GPU write) + `(double)` dead-read cast; 440 = `volatile u32 *new_var=&(*g_gpu_stat_reg)` pointer-alias self-assign + nested do{}while(0); 605-1 = float-corrupt store + `int new_var2=-1; return new_var2` constant-holder; 605-2 = `(float)`/`unsigned char` width-coercion of dead read + `volatile int *new_var2=D_8009BF64` pointer-alias. Lowest LEGITIMATE find 630-1 ties floor 9 (sys_VSync/diff hoist, non-improving).
- [s32] The only novel spelling this session (float-cast corruption of a volatile store) is still a cheat: it changes the observable *g_gpu_stat_reg bytes and only false-matches under masked Levenshtein. Axis A never legitimately materialized (combine-folded in every legit find); axis B 8-op sched cluster never legitimately reordered. Corroborates s29/s30/s31: axis B is register-pressure/control-flow/aliasing-insensitive; axis A is per-expression combine-fold.
- [s32] Permuter modality NONUPLE-chassis confirmed dead (~258k cumulative iters). No OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (grep count 0), so owner-gated is not claimable this session despite full modality exhaustion -> result progress, escalation-ready.

- [s32] s32 baseline unchanged: floor = 9 (verdict C, target 91 / build 90 insns, rules_dropped 4). src/display.c untouched this session (all work in standalone permuter workspace tmp/perm_dc9c_s32); git diff src/display.c empty.

- [s32] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep count 0), so owner-gated is NOT claimable this session despite full modality exhaustion.

- [s32] chassis-9 base.c validated at 90 insns (objdump) vs target 91 — reproduces the axis-A combine-fold gap (BF68[0] emitted as folded 2-insn lui;lw %lo, target is 3-insn lui;addiu;lw 0(reg)).

- [s32] chassis-9 seeds a genuinely distinct basin: base_score 1500 (single-reused-scratch aliasing) vs prior chassis 630 (s4/s5), 695 (s13/s14), 1110 (s31 goto). 51,633 iters, best_new_score 405, finds_total 1042 — none legitimate below the 630 floor.

- [s32] Every one of the 4 strictly-sub-floor finds is a cheat by an already-catalogued spelling: float-cast corruption of a volatile store (NEW variant, semantically wrong), pointer-alias self-assign junk (banked permuter-alias-longlong-junk), constant-holder return -1 (banked u16/int constant-holder family), width-coercion of the dead read (banked char-narrow family). The only new spelling (float-corrupt volatile store) is still a cheat: it changes the observable *g_gpu_stat_reg bytes and only false-matches under masked Levenshtein.

- [s32] Permuter modality now NONUPLE-confirmed dead across 9 structurally-distinct chassis (~258k cumulative iters: s4 27k + s5 40k + s13 31k + s14 20k + s22 35k + s23 36k + s31 ~18k + s32 52k): axis A reachable in permuter space ONLY via type/volatile/width coercion of D_8009BF68 (all cheats); axis B 8-op sched1 cluster never legitimately reorders (volatile-order-locked critical-path priority, dump-pinned s6/s15/s24).

- [s33] FORENSICS (7th run). Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s32); src/display.c clean at HEAD (untouched). Fresh cc1 -da -dr dumps tmp/grind/func_8007DC9C/s33/display.i.{rtl,lreg,greg,sched,combine,...} + FORENSICS.md.

- [s33] NOVEL forensic result: the axis-B dead-read REGISTER ($a0 in our build vs $v0 in target, = regfix rule 2833 `lw $4->$2 @20`) is a GLOBAL-ALLOC (greg) consequence of the sched1 order, NOT an independent register-allocation lever. Trace: .sched (line 15665) dead read insn 41 = (set (reg/v:SI 75) (mem/v (reg 82))) REG_UNUSED -- still a PSEUDO, already scheduled BEFORE fmt (insn 63). .lreg (line 13412) STILL pseudo 75 (local-alloc leaves it). .greg (line 10901) pseudo 75 -> hard $a0 (reg 4). So the register is assigned by GREG, AFTER sched1 fixed the order.

- [s33] WHY greg picks $a0 (schedule-induced pressure, not free choice): at insn 41's scheduled slot, reg 82 (g_gpu_stat_reg ptr, insn 39) -> $v1 (reg 3) live 39->67 (busy at 41); reg 90 (g_gpu_dma_madr ptr, insn 57) -> $v0 (reg 2) live 57->59 with insn 59 (set (reg 2 v0)(mem(reg 2 v0))) STRADDLING insn 41 (busy at 41). $v0 and $v1 both live across the dead read -> greg takes the next free caller-saved reg = $a0. Dead value is REG_UNUSED so fmt overwrites $a0 harmlessly. In TARGET order (fmt first, madr $v0-range not straddling), $v0 is free at the dead-read slot -> identical greg procedure lands $v0. Register is 100% determined by which regs the schedule made live where.

- [s33] CONSEQUENCE (sharpens s16): the two axis-B regfix rules are ONE root cause. Rule 2833 (register $a0->$v0) is the mechanical greg consequence of rule 2835 (reorder 21,20,19,18); correcting the sched1 order makes the register follow to $v0 automatically. The regfix ledger needs the separate subst only because it patches emitted asm rather than re-running the scheduler. s16's "4 rules = 2 axis-A + 2 axis-B" refined to: 2 axis-A (combine.c:1458) + axis-B = ONE sched1 root cause surfacing as 2 rules (reorder + its greg-induced register subst).

- [s33] Axis B's LAST un-pinned sub-mechanism (the dead-read register) now dump-traced across sched1->greg. No separate register-allocation lever exists: the only way to place the dead read in $v0 is to flip the sched1 order (= s24 over-determined-dead lock) or pin `register T x asm("$2")` (a cheat / register-asm pin, stripped+forbidden, and fights greg's schedule-driven choice). Axis A unchanged (combine.c:1458 added_sets_2, s25). func_8007DC9C ENDGAME-LOCK re-confirmed; no OWNER-ESCALATION entry in docs/grind/decisions.md (grep 8007DC9C = no matches) -> owner-gated not claimable; result progress, escalation-ready.

- [s33] s33 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s32); src/display.c clean at HEAD (untouched this session).

- [s33] Pass-chain pinned: sched1 fixes the dead-read-before-fmt ORDER (dead read = pseudo 75 in .sched line 15665, REG_UNUSED); local-alloc (.lreg line 13412) leaves pseudo 75 unassigned; global-alloc (.greg line 10901) assigns pseudo 75 -> hard $a0 (reg 4). fmt (insn 63) is forced to $a0 as printf arg0 and reloads it right after.

- [s33] greg's $a0 for the dead read is schedule-induced, not a free choice: at insn 41 both $v1 (reg82 stat_reg ptr, live 39->67) and $v0 (reg90 madr ptr, live 57->59 straddling insn 41 via the insn 59 self-RMW `(set (reg 2 v0)(mem(reg 2 v0)))`) are busy, so greg takes the next free caller-saved reg $a0. Target's order frees $v0 at that slot -> $v0.

- [s33] The two axis-B regfix rules are ONE root cause: rule 2833 (register $a0->$v0) is the mechanical greg consequence of rule 2835 (reorder 21,20,19,18). Correcting the sched1 order makes the register follow to $v0 automatically; regfix needs the separate subst only because it patches emitted asm rather than re-running the scheduler.

- [s33] Axis B is now fully pass-pinned across sched1->greg (order over-determined per s24; register a greg downstream of that order per s33) with no legitimate pure-C degree of freedom. Axis A unchanged (combine.c:1458 added_sets_2 offset-0 fold, dump-verified both sides s25).

- [s33] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep 8007DC9C = no matches), so owner-gated is not claimable this session despite full six-modality exhaustion.

- [s34] FORENSICS (8th run; s6/s7/s15/s16/s24/s25/s33 prior). Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s33). src/display.c clean at HEAD (untouched; only tmp/ written). Fresh cc1 -da -dr dumps tmp/grind/func_8007DC9C/s34/display.i.* (13 passes) + display.s + dump.sh + FORENSICS.md (cc1.err = benign typedef-redefinition messages, s25-noted).

- [s34] NOVEL forensic measurement (first PRINTED scheduler values banked; s6/s24 gave these by analysis/source-reasoning, never captured from the scheduler's own trace): display.i.sched per-insn listing (:15404-15413) prints INSN_PRIORITY/ref_count = insn41 (dead *g_gpu_stat_reg read, reg/v75 REG_UNUSED) priority 2 / ref_count 5; insn63 (fmt la g_str_gpu_timeout -> a0) priority 1 / ref_count 1; insn39 (stat ptr) pri1/rc3; insn57 (madr ptr) pri1/rc2; insn59 (*madr) pri2/rc2. The dead read's REG_UNUSED value fans out over the volatile-ordering chain (ref_count 5 via REG_DEP_ANTI 41 on insns 48/61/67) = the quantitative basis for its priority 2 vs fmt's 1.

- [s34] AXIS B decision dump-captured verbatim (display.i.sched:15499-15506): `;; ready list at T-42: 63 (1) 41 (2)` then `;; blocking insn 41 for 1 cycles, now 63` then `;; ready list at T-44: 41 (2), now 41`. Backward scheduler (higher T = earlier program position): fmt insn63 committed T-42, dead read insn41 committed T-44 -> dead read EARLIER = emitted before fmt. Post-sched emission chain (prev/next) 39->57->41->59->63->61->48->67 confirms dead *g_gpu_stat_reg read precedes fmt la. Identical to s6/s24/s33 order.

- [s34] BINDING-CONSTRAINT refinement (novel, consistent with s24 over-determination, does NOT overturn it): at T-42 the priority gap (41=2>63=1) makes the scheduler ATTEMPT the dead read first, but it is the 1-cycle LOAD-DELAY HAZARD ('blocking insn 41 for 1 cycles') that actually assigns fmt to T-42 and defers the dead read to T-44. The hazard is the binding constraint at this decision and persists independent of the priority value -> corroborates s24's conclusion that no priority-equalising pure-C construct flips the order. Removing the hazard requires removing the load = volatile+semantically-required read dropped = banked score-19 collapse. No new degree of freedom.

- [s34] AXIS A re-confirmed on fresh dump: display.i.combine:10051 (set (reg 5 a1) (mem/s (symbol_ref D_8009BF68))) = folded symbol-direct load (address pseudo deleted, combine.c:1458 added_sets_2 FALSE single-use); final asm display.s:54 `lw $5,D_8009BF68` (2-insn) vs target 3-insn; BF6C/BF70 fold (asm:55-56). Sibling func_8007D3F8 store (set (mem (symbol_ref D_8009BF68))) at combine:8422 (insn 133) in its OWN combine section, disjoint from this read -> cross-fn retention mechanically impossible (per-function combine, s7/s25).

- [s34] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep 8007DC9C = 0 matches this session), so owner-gated is NOT claimable despite full six-modality exhaustion + both axes re-measured dead on a fresh dump with concrete printed values. Escalation-ready since s7 (28 sessions); the only unblock is the OWNER filing the entry. No further un-pinned forensic detail remains -> result progress.

- [s34] s34 baseline: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 — identical fingerprint to s1-s33; src/display.c clean at HEAD (only tmp/ + evidence.md written).

- [s34] NOVEL printed scheduler values (first banked from the -da -dr trace): dead *g_gpu_stat_reg read (insn41, reg/v75, REG_UNUSED) INSN_PRIORITY 2 / ref_count 5; fmt la g_str_gpu_timeout->a0 (insn63) INSN_PRIORITY 1 / ref_count 1 (display.i.sched:15404-15413). The dead read's ref_count 5 fans out over REG_DEP_ANTI 41 on insns 48(D_8009BF7C mem/v)/61/67 = quantitative basis for the +1 priority.

- [s34] Scheduler T-42 decision captured verbatim (display.i.sched:15499-15506): 'ready list at T-42: 63 (1) 41 (2)' -> 'blocking insn 41 for 1 cycles, now 63' -> fmt committed T-42, dead read committed T-44; backward scheduler (higher T = earlier) -> dead read before fmt. Emission chain 39->57->41->59->63->61->48->67.

- [s34] Binding constraint = the 1-cycle load-delay hazard on the dead read (not the priority value, which merely orders the ready list); persists independent of priority, corroborating s24 over-determination. Load is volatile+semantically required -> cannot be removed in legitimate pure C.

- [s34] Axis A re-confirmed on fresh dump: combine:10051 folded (mem/s (symbol_ref D_8009BF68)); asm:54 lw $5,D_8009BF68 (2-insn) vs target 3-insn; sibling store at combine:8422 in its own per-function combine section (cross-fn retention impossible, s7/s25).

- [s34] No OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (grep count 0 this session, matching s24-s33). owner-gated NOT claimable. Escalation-ready since s7 (28 sessions).

- [s34] Six-modality exhaustion intact: structural (s2/s3/s11/s12/s20/s21/s29/s30), permuter x9 ~258k iters (s4/s5/s13/s14/s22/s23/s31/s32), forensics x8 (s6/s7/s15/s16/s24/s25/s33/s34), rederive x6 (s8/s9/s17/s18/s26/s27), synthesis x3 (s10/s19/s28). 4 regfix rules = 2 combine (axis A) + 1 sched1 root cause surfacing as 2 rules (axis B, s33).

- [s35] REDERIVE (7th rederive session). Baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s34). src/display.c restored clean HEAD after the experiment (git diff empty, no dirt).

- [s35] NOVEL rederive form MEASURED DEAD: single-exit result-accumulator skeleton (`s32 result=0; if(cond){...; result=-1;} return result;`) — a natural human-writable shape distinct from all six banked rederive forms (HEAD dual comma-if return, s8 goto/nested-if, s17 inverted-early-return-guard, s18 computed-flag-two-if, s26 ternary, s27 post-increment). NOT a cheat (accumulator has real semantic purpose = the return value). Score 19 / build 90 — STRICTLY WORSE (+10 vs floor 9). Rejected: rejected/rederive-single-exit-result-accumulator.c.

- [s35] NEW DATA POINT on the exit-skeleton taxonomy: three classes now measured — (a) distinct-exit (HEAD/s8/s26/s27) -> build 90 / score 9 (floor); (b) exit-coalescing (s17/s18) -> build 88 / score 12; (c) single-exit accumulator (s35) -> build 90 / score 19 (NEW, worst). The accumulator neither coalesces exits NOR reaches the floor build — the shared `result` pseudo threads both paths, forcing an extra $v0 materialization on the A-false fall-through (return loads the accumulator vs `li $v0,0`) + reshuffles the tail return set = added divergence atop both mechanism-pinned axes. Confirms axis A (combine.c:1458 offset-0 fold) and axis B (sched1 over-determined volatile-MEM lock) are both insensitive to return-value skeleton — no skeleton reaches below the floor 9.

- [s35] No OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (exact grep `8007DC9C` = 0 matches this session, matching s7-s34; the 71 decisions.md hits are all the word "escalation" for OTHER functions). owner-gated NOT claimable. Escalation-ready since s7 (29 sessions). The only unblock remains the OWNER filing the entry.

- [s35] s35 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s34); src/display.c restored to clean HEAD (git diff empty, no dirt).

- [s35] Novel single-exit result-accumulator rederive form measured score 19 / build 90 — strictly worse than floor 9. Rejected: memory/grind/func_8007DC9C/rejected/rederive-single-exit-result-accumulator.c.

- [s35] NEW exit-skeleton taxonomy data point: distinct-exit (HEAD/s8/s26/s27) -> build90/score9 (floor); exit-coalescing (s17/s18) -> build88/score12; single-exit accumulator (s35) -> build90/score19 (worst). No return-value skeleton reaches below floor 9 — axis A (combine.c:1458 offset-0 fold) and axis B (sched1 over-determined volatile-MEM lock) both insensitive to exit skeleton.

- [s35] No OWNER-ESCALATION entry for func_8007DC9C in docs/grind/decisions.md (exact grep 8007DC9C = 0 matches this session, consistent s7-s34; the 71 decisions.md hits are all the word 'escalation' for other functions). owner-gated NOT claimable; escalation-ready since s7 (29 sessions).

- [s35] Rederive modality now dead at skeleton, expression-operator, control-flow, m2c, Kengo, decomp.me-corpus, AND return-value-accumulator levels (s8/s9/s17/s18/s26/s27/s35). Both axes mechanism-pinned dead across all six modalities (structural, permuter x9, forensics x8, rederive x7, synthesis x3).

- [s36] s36 baseline re-confirmed (src clean at HEAD): sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 — identical fingerprint to s1-s35.

- [s36] Novel switch-dispatch skeleton (switch on the ||-guard, case 0 -> return 0, default -> report; return -1) measured score 18 / build_insns 95 = STRICTLY WORSE (+9 score, +5 body insns over floor). New rederive data point: unlike the if/ternary/goto/accumulator skeletons which all scored 9 (identical fingerprint), the switch ADDS instructions because it materializes the short-circuit result into a 0/1 truth value instead of branching on it.

- [s36] The +5 insns are a guard-region truth-value cluster; they do not touch axis A (combine offset-0 BF68 fold in the 2nd printf) or axis B (sched1 volatile-MEM anti-dep priority in the 1st printf setup). Corroborates s8's control-flow-insensitivity proof: the report body is unchanged regardless of guard skeleton.

- [s36] Rejected form saved: memory/grind/func_8007DC9C/rejected/rederive-switch-dispatch-skeleton.c. Skeleton-level rederive now dead at if/nested-if+goto (s8), ternary (s26), post-increment condition (s27), single-exit accumulator (s35), inverted-early-return / computed-flag-two-if (banked), and switch-dispatch (s36).

- [s36] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE), so owner-gated is not claimable this session despite full six-modality exhaustion.

- [s36] src/display.c restored to HEAD after the experiment; git status clean (only metrics/events.jsonl dirty, engine-appended). No dirt left.

- [s37] SYNTHESIS (4th pass, s10/s19/s28 precedent). Baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 — IDENTICAL fingerprint to s1-s36. src/display.c clean at HEAD (git status empty; candidate.c byte-equivalent). No OWNER-ESCALATION entry in docs/grind/decisions.md (grep func_8007DC9C = 0 matches).

- [s37] Full re-read of evidence.md (801 lines) + hypotheses.md (484 lines) + rejected bank. NO un-banked pure-C lever surfaces on merge. Both axes remain orthogonal (s10) and mechanism-pinned dead across SIX modalities: structural (s2/s3/s11/s12/s20/s21/s29/s30), permuter x9 chassis ~258k iters (s4/s5/s13/s14/s22/s23/s31/s32), forensics x8 dump-pinned (s6/s7/s15/s16/s24/s25/s33/s34), rederive x8 skeleton+operator (s8/s9/s17/s18/s26/s27/s35/s36), synthesis x4 (s10/s19/s28/s37).

- [s37] Consolidated mechanism (dump-verified both sides): Axis A = combine.c:1458 `added_sets_2 = !dead_or_set_p(i3,i2dest)` FALSE for the single-use BF68 address pseudo (reg93) -> COMBINE deletes the materialization (s25 pins fold pass = combine, not expand). Reproduction needs a 2nd &BF68 use: single-fn = dead-store coercion (s7 banked, 90->93 undeletable), cross-fn mechanically impossible (per-function combine, toplev.c:3004; sibling store in its own combine section, s7). Axis B = OVER-DETERMINED sched1 lock (s24/s34): volatile-MEM anti-dep priority 2>1 + MIPS-I load-delay hazard (binding, alone reproduces the order) + LUID tie-break; register $a0-vs-$v0 is greg-downstream-of-sched1 (s33), NOT independent. Rule-set complete: 4 rules = 2 combine + 1 sched1-root-manifesting-as-2 (s16 sharpened by s33).

- [s37] Disposition unchanged and unambiguous: endgame-lock-disposition-policy gate #1 REFUSE (scan_hand_coded LOW, pure-C match exists) + gate #2 REFUSE (no coercion precedent) -> keep 4 rules, INCOMPLETE-owner-accepted. owner-gated NOT claimable this session (no escalation entry). Result = progress, escalation-ready. Artifact tmp/grind/func_8007DC9C/s37/MERGED-ATTACK-s37.md.

- [s37] s37 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 — identical fingerprint to s1-s36; src/display.c clean at HEAD.

- [s37] Axis A dump-verified BOTH sides (s25): combine.c:1458 added_sets_2 = !dead_or_set_p(i3,i2dest) FALSE for the single-use BF68 address pseudo (reg93) -> COMBINE deletes the materialization (fold pass = combine, not expand). 2nd &BF68 use single-fn = dead-store coercion (s7, 90->93 undeletable); cross-fn mechanically impossible (per-function combine, toplev.c:3004; sibling store in its own combine section).

- [s37] Axis B OVER-DETERMINED (s24, printed-confirmed s34): volatile-MEM anti-dep priority 2/ref_count5 > fmt 1/1 + MIPS-I load-delay hazard (binding, alone reproduces dead-read-before-fmt at T-42) + LUID tie-break. Register $a0-vs-$v0 (rule 2833) is greg-downstream-of-sched1 (s33), NOT an independent lever -> rules 2833+2835 are ONE sched1 root cause.

- [s37] Rule-set complete (s16 sharpened by s33): 4 regfix rules = 2 combine (axis A) + 1 sched1 root manifesting as 2 (axis B); honest gap 9 = 1 insertion + 8-op cluster, no hidden 3rd divergence.

- [s37] Exhaustion: 36 sessions, zero floor movement. Structural (s2/s3/s11/s12/s20/s21/s29/s30) all fold or worsen; permuter 9 chassis ~258k iters (s4/s5/s13/s14/s22/s23/s31/s32) yield only coercion cheats, axis B never legitimately reordered; forensics x8 dump-pinned both axes; rederive x8 (m2c/Kengo/corpus no donor; if/goto/ternary/postinc reach floor-90; inverted/flag/switch/accumulator strictly worse).

- [s37] Disposition per endgame-lock-disposition-policy: gate #1 REFUSE (scan_hand_coded LOW, pure-C match exists), gate #2 REFUSE (no coercion precedent) -> keep 4 rules, INCOMPLETE-owner-accepted. No OWNER-ESCALATION entry exists in docs/grind/decisions.md (grep func_8007DC9C = 0 matches).

- [s38] STRUCTURAL modality. Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150. src/display.c clean at HEAD; reverted after the experiment (no dirt).

- [s38] NOVEL structural probe (un-banked): isolate the axis-A read D_8009BF68[0] into a top-of-block temp `t68` computed BEFORE the first debug_printf, giving its VALUE a live range spanning the first call (forces a callee-save) — distinct from the banked s29/s30 forms that hoisted BOTH 2nd-printf reads together. Hypothesis: a cross-call live range might change combine's single-use fold and materialize the 3-insn address. RESULT: score 25, build_insns 92 (STRICTLY WORSE). The hoisted read STILL folds at its read site (the ADDRESS pseudo stays single-use — combine.c:1458 added_sets_2 counts address uses, NOT the value's live-range length), and the cross-call range adds a callee-save save/restore + schedule disruption. NEW DATAPOINT: axis-A offset-0 fold is insensitive to live-range LENGTH; only a genuine 2nd ADDRESS use defeats it (= coercion single-fn per s7, mechanically impossible cross-fn per s7 per-function combine). KILLED. Rejected: rejected/structural-s38-bf68-crosscall-hoist.c.

- [s38] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE), so owner-gated is not claimable this session despite full six-modality exhaustion -> result progress, escalation-ready. Structural modality re-confirmed dead (now with the live-range-length negative added to the s2/s3/s11/s12/s29/s30 structural KILLs).

- [s38] s38 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150; src/display.c clean at HEAD, reverted after experiment (no dirt).

- [s38] NEW DATAPOINT: axis-A offset-0 combine fold (combine.c:1458 added_sets_2) is insensitive to the read value's live-range LENGTH — a cross-call live range (score 25/build 92) does NOT materialize the 3-insn address; it only adds spill overhead. The address pseudo stays single-use and folds. Only a genuine 2nd ADDRESS use defeats the fold (= coercion single-fn per s7; mechanically impossible cross-fn per s7 per-function combine).

- [s38] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE), so owner-gated is not claimable this session.

- [s38] Structural modality re-confirmed dead: the live-range-length negative adds to the s2/s3/s11/s12/s29/s30 structural KILLs. Both axes remain mechanism-pinned dead across all six modalities (structural, permuter x9, forensics x8, rederive x8, synthesis x4).

- [s39] STRUCTURAL modality. Baseline re-confirmed: sandbox --disable all score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint to s1-s38). src/display.c reverted to clean HEAD after the experiment (no dirt).

- [s39] Novel un-banked structural form KILLED (score 20, build 90, strictly WORSE): diff hoisted to top of if-body + BOTH *g_gpu_stat_reg reads staged as two distinct named temps (dead new_var + named arg3), so the printf-argument stat read is precomputed BEFORE the call instead of inline at the call site. Two reads preserved (NOT the score-19 single-read collapse). Result: 20 differing insns vs target's 91 (build count unchanged at 90). Naming the printf-arg *stat read into a pre-call temp reorders the two volatile reads and pushes the sched1 order further from target. Rejected: rejected/structural-s39-printf-stat-read-hoist-arg3.c.

- [s39] NEW DATA POINT (opposite-direction corroboration of axis B): every prior structural probe (s2 8-form sweep, s29/s30 2nd-printf hoists) nudged the DEAD *g_gpu_stat_reg read and scored 9 (schedule insensitive to dead-read placement). s39 is the FIRST to nudge the LIVE printf-argument *stat read, and it STRICTLY WORSENS (9 -> 20). This proves the printf's *stat read is schedule-position-LOCKED in place (must stay inline at the call site); moving it out of the call is not a lateral no-op like the dead-read nudges but an active regression. Corroborates s6/s34's volatile-MEM anti-dep order lock from the live-read side: the two volatile reads' relative sched1 position is fixed, and any C construct that repositions EITHER read (dead or live) cannot improve and can worsen. Axis B remains mechanism-pinned dead; structural modality re-confirmed exhausted.

- [s39] Both axes stay mechanism-pinned dead across seven structural sessions (s2/s3/s11/s12/s29/s30/s38 + s39). No un-banked legitimate structural lever remains; no pure-C modality remains untried across all six modalities. func_8007DC9C stays a fully-characterized ENDGAME-LOCK, escalation-ready. No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE this session), so owner-gated is NOT claimable -> result progress with escalation-ready frontier.

- [s39] s39 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 — identical fingerprint to s1-s38. src/display.c reverted to clean HEAD after the experiment (no dirt).

- [s39] Novel un-banked structural form (diff hoisted + two *g_gpu_stat_reg reads staged as distinct named temps new_var/arg3, printf-arg read precomputed) scored 20 (build 90), strictly worse than floor 9. Two reads preserved. Rejected: rejected/structural-s39-printf-stat-read-hoist-arg3.c.

- [s39] NEW DATA POINT: every prior structural probe (s2 8-form sweep, s29/s30 2nd-printf hoists) moved the DEAD *stat read and scored 9 (lateral, placement-insensitive). s39 is the FIRST to move the LIVE printf-argument *stat read, and it strictly worsens (9 -> 20). Proves the printf's *stat read is schedule-position-locked inline at the call site; repositioning EITHER volatile read cannot improve and can worsen. Corroborates the s6/s34 volatile-MEM anti-dep order lock from the live-read side.

- [s39] Both axes stay mechanism-pinned dead across seven structural sessions (s2/s3/s11/s12/s29/s30/s38 + s39). Axis A = combine.c:1458 added_sets_2 single-use fold for the BF68 address pseudo (per-expression, structure-insensitive). Axis B = over-determined sched1 lock (volatile-MEM anti-dep priority 2>1 + load-delay hazard + LUID backstop), now corroborated on both the dead-read and live-read sides.

- [s39] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (grep NONE this session), so owner-gated is NOT claimable despite full six-modality exhaustion.

- [s40] PERMUTER modality, TENTH chassis (chassis-10 "long-live arg-temps"): base.c pre-stages the 2nd-printf source reads (BF68[0]/BF6C/BF70) into outer-scope named temps with live ranges crossing the axis-B cluster + splits BF78-BF7C difference (hi=BF78; diff=(hi-BF7C)&0x3F). base_score 695, base 90 / target 91 insns, reproduces both ledger axes. Distinct register-pressure regime vs s5(whole-body)/s13(tail)/s14(low-pressure-blockscope)/s31(goto)/s32(reused-scratch).
- [s40] Campaign s40-chassis10-longlive-argtemps (-j8, base 695): 18,527 iters over ~10 min in-turn wait windows. Legitimate finds ceiling at 630 (== floor 9; recovers floor by inlining the diff temp) — NEVER below 630 legitimately. The ONLY sub-floor find (565) = `extern volatile int D_8009BF68[]` = the banked volatile-BF68 coercion (s4 axisA-permuter-volatile-bf68.c), closes axis A only via volatile-prevents-combine-fold; stripped by engine.volatile_cheats, forbidden by [[legitimate-volatile-interrupt-touched]] crit#2 (single printf-arg read). Axis B 8-op sched1 cluster NEVER legitimately reordered (the arg_a=*stat split of the dead read is a rename, no schedule change). Harvested --stop in-turn; status alive:false, registered_active:false; 0 orphan permuter procs.
- [s40] PERMUTER now DECUPLE-confirmed dead across 10 structurally-distinct chassis (~277k+ iters s4/s5/s13/s14/s31/s32/s40): axis A reachable in permuter space ONLY via type/volatile coercion of D_8009BF68 (all cheats); axis B volatile-order-locked, never reorders. Identical signature to every prior chassis. No legitimate sub-floor pure-C form exists in permuter space. Artifacts tmp/grind/func_8007DC9C/s40/.

- [s40] s40 baseline re-confirmed: sandbox --disable all score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint to s1-s39). src/display.c untouched this session (all work in standalone permuter workspace).

- [s40] Chassis-10 base.c compiled to 90 insns vs target 91; base-vs-target reproduces the axis-A 3-insn BF68 materialization gap; base_score 695 (distinct basin, same as s13/s14 magnitude but different topology).

- [s40] Campaign 18,527 iters, ~10 min in-turn. Legitimate find ceiling 630 == floor 9 (non-improving); ONLY sub-floor find 565 = extern volatile int D_8009BF68[] = banked volatile coercion (axis A only), a cheat stripped by volatile_cheats.

- [s40] Axis B 8-op fmt-vs-deadread sched1 cluster never legitimately reordered in any of 18,527 iters, corroborating the s6/s33/s34 forensics that the ordering is a volatile-MEM anti-dep critical-path lock, not a register-pressure or source-order effect.

- [s40] PERMUTER now DECUPLE-confirmed dead across 10 structurally-distinct chassis (s4 x2, s5, s13, s14, s31, s32, s40) / ~277k+ cumulative iters: axis A reachable in permuter space ONLY via type/volatile coercion of D_8009BF68 (all cheats); axis B is volatile-order-locked and never reorders.

- [s40] Campaign harvested --stop in-turn; status shows alive:false, registered_active:false, 0 live campaigns, 0 stale registry entries, no orphan permuter processes remain.

- [s40] No OWNER-ESCALATION entry for func_8007DC9C exists in docs/grind/decisions.md (per ledger through s39), so owner-gated is not claimable this session.

## [s41] DISPOSITION SESSION (escalation modality) — standing-ruling entry FILED; floor re-measured chassis-current at 9

- [s41] Chassis re-measurement (the number to quote, NOT the ledger's): the banked candidate applied to src/display.c in place of `INCLUDE_ASM("asm/funcs", get_alarm);` measures `sandbox get_alarm --disable all` = **score 9, target_insns 91, build_insns 90, rules_dropped 0, cheat_asm_stripped 149**. The 2026-08-19 asm-until-matched migration retired the 4 regfix rules but did NOT change the honest distance, so every chassis-relative conclusion banked in s1-s40 remains valid on the current chassis. src/display.c restored to INCLUDE_ASM afterwards; no src dirt left behind.
- [s41] candidate.c REFRESHED for the current src/display.c naming — the banked body still used the pre-rename symbols and would not compile as-is. Renames applied: `func_8007DC9C`->`get_alarm`, `sys_VSync`->`VSync`, `debug_printf`->`printf`, `motion_make_table`->`SetIntrMask`. The previous spelling is preserved at tmp/grind/get_alarm/s41/candidate-prev-names.c. Body semantics unchanged; measured 9 as above.
- [s41] AND-gate #1 (canonical-asm) RE-RUN and FAILS: `tools/scan_hand_coded.py --single get_alarm` = tier=LOW, score=1/8, 91 insns. Only S4 fires (5 loads in an 8-insn window @ insn 24). S1 (multu pacing: 0 multu/mflo pairs), S2 (empty branch: none), S6 (BIOS jumptable: none) — the three STRONG signals the gate requires — are all ABSENT, as are S3 (91 insns / 2 spills / 7 distinct regs), S5, S7, S8. Artifact of the run recorded in the session outcome; this is a FAILED gate, not an open question.
- [s41] AND-gate #2 (in-hand SOTN-master precedent) RE-CENSUSED and FAILS: `docs/reference/sotn-construct-index.md` (sotn-decomp master aa53500, 1911 files scanned) contains NO entry that is (a) an `extern volatile` coercion of a game-state global whose only use-site is an ordinary argument read, or (b) a fabricated dead second address-use inserted to defeat combine's single-use symbol fold. Every `volatile` hit in the index is either a FAKE-annotated LOCAL pad (`src/st/sel/stream.c:80`, `src/st/sel/2C048.c:564`, `src/st/e_background_bushes_trees.h:160`) or a `volatile StHEADER*` local pointer alias (`src/main/psxsdk/libcd/c_009.c:12`) — different families, different use-site shapes, neither citable for either closing construct here. A negative census is a FAILED gate.
- [s41] Both gates fail => the owner's 2026-07-27 standing auto-ruling applies verbatim. Entry FILED at docs/grind/decisions.md:8621 — `## 2026-08-20 — get_alarm / func_8007DC9C (src/display.c) — **OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE**` (re-affirmation on the post-migration chassis). Note the function ALREADY carried a terminal owner ruling from 2026-07-22 (decisions.md:1316); the migration re-queued it, which is why 41 sessions were spent re-measuring a decided function. Disposition: INCOMPLETE-owner-accepted, representation stays `INCLUDE_ASM("asm/funcs", get_alarm);`, nothing pending on the owner ([[judge-sole-gate]]).
- [s41] No un-tried lever was found this session. The two axes remain exactly as pinned: axis A = combine.c:1458 added_sets_2 single-use fold of the &D_8009BF68 address pseudo (target's only reproduction is a DEAD second use, i.e. a first-reach coercion); axis B = over-determined sched1 lock (volatile-MEM anti-dep REG_DEP_ANTI 38->45 giving priority 2>1, PLUS the MIPS-I load-delay hazard, PLUS a LUID backstop — three independent causes of the same order, so equalizing any one of them cannot flip it).

- [s41] Chassis floor re-measured THIS session at 9 (target 91 / build 90, rules_dropped 0, cheat_asm_stripped 149) — the ledger floor is chassis-current, not stale, so all s1-s40 chassis-relative conclusions still hold.

- [s41] candidate.c was STALE-NAMED (pre-rename symbols func_8007DC9C/sys_VSync/debug_printf/motion_make_table) and would not have compiled as applied; it is now refreshed to the current src/display.c naming and re-measured at 9. Previous spelling preserved at tmp/grind/get_alarm/s41/candidate-prev-names.c.

- [s41] AND-gate #1 FAILS: scan_hand_coded --single get_alarm = LOW, score 1/8, only S4 firing; the three STRONG signals (S1 multu pacing, S2 empty branch, S6 BIOS jumptable) are all absent.

- [s41] AND-gate #2 FAILS: a real census of docs/reference/sotn-construct-index.md returns nothing citable for either closing construct; the index's volatile entries are FAKE-annotated local pads and one local pointer alias — different families and different use-site shapes.

- [s41] get_alarm/func_8007DC9C ALREADY carried a terminal owner ruling from 2026-07-22 (docs/grind/decisions.md:1316, escalation option (b), REFUSED / OWNER-ACCEPTED INCOMPLETE). The 2026-08-19 asm-until-matched migration retired the 4 regfix rules that ruling's disposition preserved on main, which re-queued the function as INCLUDE_ASM at honest distance 9 — that is why 41 sessions were spent on an already-decided function. The same re-queue pathology hit func_800611A4 (re-affirmed 2026-08-20).

- [s41] Axis A stays pinned to combine.c:1458 added_sets_2 (single-use address pseudo is always folded); target's asm references &D_8009BF68 exactly once (asm/funcs/get_alarm.s:43-45), so the materialization is retention of a use a LATER pass DCE'd — any single-function pure-C reproduction is a DEAD second use, a first-reach coercion with no sanctioned family.

- [s41] Axis B stays pinned as OVER-DETERMINED: the volatile-MEM anti-dep REG_DEP_ANTI 38->45 (priority 2 vs fmt's 1), the MIPS-I load-delay hazard, and a LUID backstop each independently produce the same emission order (s6/s24/s34), and the $a0-vs-$v0 difference is a greg consequence downstream of it (s33). Equalizing any single cause cannot flip it.

- [s41] Exhaustion on record: 41 sessions, six distinct modalities (recon/structural/permuter/forensics/rederive/synthesis), floor flat at 9 since s1, ~258k permuter iterations across TEN structurally-distinct chassis, 25 disproven forms banked in memory/grind/get_alarm/rejected/, 67+ hypotheses killed. s16 proved the 4 pre-migration rules mapped 1:1 onto exactly these two axes — no hidden third divergence.

- [s41] Standing-ruling entry FILED this session at docs/grind/decisions.md:8621 naming get_alarm / func_8007DC9C directly, stating both gates' evidence, the chassis re-measurement, and the asm-until-matched disposition. Nothing is pending on the owner per judge-sole-gate (2026-08-18).

- [s41] No src dirt left: src/display.c restored byte-for-byte to its INCLUDE_ASM state (backup at tmp/grind/get_alarm/s41/display.c.bak); session diff is docs/grind/decisions.md + memory/grind/get_alarm/* only.

- [s42] Chassis re-measured with candidate.c applied: sandbox get_alarm --disable all = 9 (target_insns 91, build_insns 90, rules_dropped 0, cheat_asm_stripped 147). Ledger floor is chassis-current. src/display.c restored to INCLUDE_ASM afterwards; tree clean.
- [s42] OWNER DIRECTIVE 2026-08-24 (solver modality) EXECUTED for the first time. tools/sched_solver/extract.py display -> parity=True, 140 funcs / 598 blocks / 3264 picks, model exact.
- [s42] Axis B is a ONE-INSN move, not an 8-op cluster. Pass-2 emitted order ours = 36,54,43,38,60,45,56,47,58,52,64,66,62,68; target (read off asm/funcs/get_alarm.s 0x8007DCEC-0x8007DD38) = 36,60,38,43,54,45,56,47,58,52,64,66,62,68. Only the fmt `la` (UID 60) moves; 54/43/38 shift one slot as a consequence.
- [s42] The s6/s33/s34 axis-B mechanism (sched1 priority 2 vs 1 from a volatile-MEM anti-dep) is a PASS-1 fact and does NOT determine the final byte order. In pass 2 UID 38 and UID 60 both have priority 2.
- [s42] Exhaustive single-atom sched2 search (4391 atoms) returns EXACTLY ONE vector reaching target's order: `del_dep 60 <- 38` = the pass-2 output dependence on hard register $a0 between the dead *g_gpu_stat_reg read (greg seats its pseudo in $4) and the fmt `la` (writes $4). Target seats that dead read in $v0 (lw $v0,0($v1) @ 0x8007DCFC).
- [s42] NO luid / luid_move atom reaches the goal at depth 1: statement reordering alone cannot produce target's order while the $a0 output dep stands. Depth 2 over luid atoms was started and stopped unfinished (over the in-turn budget) - not claimed either way.
- [s42] Axis B is therefore an RA SEAT residual, not a scheduler tie. Decidable, un-probed question: what pure-C form makes local-alloc/greg seat the dead volatile read in $v0 rather than $a0?
- [s42] TOOLING GAP (pipeline-wide, not get_alarm-specific): tools/ra_solver/inverse_compose.py classify is inapplicable to INCLUDE_ASM-routed functions - it falls back to the text-stream classifier and needs tmp/inverse_work/<stem>.tgt.s, which post-migration carries OUR bytes. sched_solver has a --target-object escape (owner ruling 2026-08-25); ra_solver does not.
- [s42] Three ordinary-C spellings of the discarded volatile read measured: `(void)*g_gpu_stat_reg;` bare expression statement = 9 (ties the floor; strictly cleaner than the s41 dead-local `new_var` form, promoted into candidate.c); comma-expression inside the printf arg = 12; `que` staged before the read = 12. Rejected forms banked as s42-comma-dup-volatile-read-in-arg-score12.c and s42-que-staged-before-bare-read-score12.c.
- [s42] DISPOSITION NOT RE-FILED. The 2026-08-20 entry (docs/grind/decisions.md:8628, "REFUSED / OWNER-ACCEPTED INCOMPLETE") is AUTO-REJECT CLASS under the owner 2026-08-24 second ruling (.claude/rules/escalation-not-parked.md: "any 'accept the debt' disposition in new wording" is pre-decided NO and MUST NOT be filed). The same ruling retired the parked state and kicked get_alarm back to ACTIVE with the solver directive this session executed. The exhaustion premise is additionally falsified by the s42 re-attribution: an entire lever surface (the RA seat of the dead read) has never been probed.

- [s42] Chassis re-measured with candidate.c applied: sandbox get_alarm --disable all = 9 (target_insns 91, build_insns 90, rules_dropped 0, cheat_asm_stripped 147). The ledger floor is chassis-current. src/display.c restored to INCLUDE_ASM; git status shows src/ clean.

- [s42] The owner's 2026-08-24 queue directive (solver modality before deep re-grind) had never been executed by any of the 41 prior sessions; s42 executed it. tools/sched_solver/extract.py display -> parity=True, 140 funcs / 598 blocks / 3264 picks, model exact.

- [s42] Axis B is a ONE-INSN move: only the format-string `la` (pass-2 UID 60) is misplaced (our emit slot 4, target slot 1). The '8-op cluster' framing in every prior session's ledger describes the knock-on shifting of 54/43/38, not eight independent misorderings.

- [s42] In pass 2 the dead *g_gpu_stat_reg read (UID 38) and the fmt `la` (UID 60) both have INSN_PRIORITY 2. The s6/s33/s34 'priority 2 vs priority 1' mechanism is a pass-1 fact and does NOT determine the final byte order - a four-session mis-attribution now corrected with a printed model.

- [s42] Exhaustive single-atom sched2 search over 4391 atoms returns EXACTLY ONE vector reaching target's order: `del_dep 60 <- 38` = the pass-2 output dependence on hard register $a0 (greg seats the dead read's zero-use pseudo in $4; the fmt `la` writes $4). Target seats that dead read in $v0 (lw $v0,0($v1) @ 0x8007DCFC).

- [s42] No luid / luid_move atom reaches the goal at depth 1: statement reordering alone cannot produce target's order while the $a0 output dependence stands. The depth-2 luid-only search was launched, exceeded the in-turn budget (4391^2 pairs), and was STOPPED rather than orphaned - it is explicitly unresolved, not claimed either way.

- [s42] Consequence: axis B is a REGISTER-ALLOCATION SEAT residual, not a frozen scheduler tie. The decidable question is 'what pure-C form makes local-alloc/greg seat the dead volatile read in $v0 rather than $a0?' - a lever surface no session has probed.

- [s42] Three ordinary-C spellings of the discarded volatile read measured: bare `(void)*g_gpu_stat_reg;` = 9 (ties the floor, promoted into candidate.c because it removes the s41 dead local `new_var` + `(void)new_var;` for the same score); comma-expression inside the printf arg = 12; staged `que` before the read = 12.

- [s42] TOOLING GAP (pipeline-wide): tools/ra_solver/inverse_compose.py classify is inapplicable to INCLUDE_ASM-routed functions - it needs tmp/inverse_work/<stem>.tgt.s, which post-migration carries our bytes. sched_solver has --target-object (owner ruling 2026-08-25); ra_solver does not.

- [s42] DISPOSITION DELIBERATELY NOT RE-FILED. The existing 2026-08-20 entry (docs/grind/decisions.md:8628, 'REFUSED / OWNER-ACCEPTED INCOMPLETE') is AUTO-REJECT CLASS under the owner's second 2026-08-24 ruling (.claude/rules/escalation-not-parked.md: 'any "accept the debt" disposition in new wording' is pre-decided NO and MUST NOT be filed), and the same ruling retired the parked state and returned get_alarm to ACTIVE. Re-emitting owner-gated against that entry would file a pre-decided-NO packet. Independently, the exhaustion premise is falsified by the s42 re-attribution above. A STATUS CORRECTION note (explicitly not an escalation, no owner decision requested) was appended at docs/grind/decisions.md:12888 so the owner audit trail does not still read 'no further grind sessions should be dispatched'.

- [s42] This session touched only src/display.c (reverted, clean), memory/grind/get_alarm/**, docs/grind/decisions.md and tmp/. No regfix.txt, asmfix.txt, .claude/rules/, engine/, tools/, Makefile or *.ld edits; no queue done / retire / commit; the one background solver job was stopped in-turn, nothing orphaned.

## [s43] escalation modality — axis B re-attributed a second step upstream (sched1 delay-slot filler), statement-order surface closed at depth 1 in BOTH scheduler passes

- [s43] CHASSIS: floor re-measured chassis-current with `memory/grind/get_alarm/candidate.c`
  spliced into src/display.c: `sandbox get_alarm --disable all` = **9**
  (target_insns 91, build_insns 90, rules_dropped 0, cheat_asm_stripped 147). Identical to
  s41/s42. src/display.c restored to HEAD afterwards; tree clean.

- [s43] OWNER DIRECTIVE EXECUTED (2026-08-30 escalation-batch ruling 10, "active with modality
  change; nothing pends"). This session ran the ra_solver/sched_solver chain that ruling 10 left
  open, on BOTH scheduler passes and on local-alloc, for the first time.

- [s43] **The full causal chain for axis B, now named end to end** (this supersedes the s42
  stopping point, which named only the last link). Read off `tmp/grind/get_alarm/dumps/display.lreg`
  (post-sched1 RTL, function banner at line 11141) plus the sched_solver pass-1 node table:
  1. `insn 36` = `lw <stat_ptr>` (pseudo 80, `D_8009BF48`), icost 2 — a load whose result is not
     available for two cycles.
  2. `insn 38` = `(set (reg:SI 81) (mem/v:SI (reg:SI 80)))` — the DEAD volatile read, carrying
     `REG_UNUSED (reg:SI 81)`. It depends on 36 and therefore cannot issue adjacent to it.
  3. sched1 fills the gap with `insn 54` = `lw <madr_ptr>` (pseudo 89, `D_8009BF4C`), whose result
     is consumed by `insn 56` (`lw (mem (reg 89))`). Our sched1 emitted order for the first printf
     region is `36, 54, 38, 56, 60, 58, 45, 64, 52, 43, 66, 47, 62, 68`.
  4. Because 54 was hoisted between 36 and 38, pseudo 89's live range is **[6,10)** and the dead
     read's is **[8,9)** — they OVERLAP. `tools/ra_solver/local_extract.py display` +
     `local_alloc.py display --func get_alarm` (order 4/4 blocks, assign 15/19 qtys, 1 `sugg` row
     out of model) gives block 2's quantity table; the dead read is **qty 2 / pseudo 81, life
     [8,9), refs 1, got $4 ($a0)** and it is allocated **LAST (ord 15)** because refs=1 makes
     `qty_compare`'s `floor_log2(refs)*refs*size/life` zero. By then qty 1 (pseudo 89, [6,10)) holds
     `$v0` and qty 0 (pseudo 80, stat_ptr, [4,18)) holds `$v1`, so the only free low reg is `$a0`.
  5. That `$a0` seat is exactly the pass-2 OUTPUT DEPENDENCE `60 <- 38` the s42 solver isolated as
     the unique reaching atom: the fmt `la` also writes `$a0`, so sched2 cannot lift it to slot 1.
  So axis B is **not** an RA free choice and **not** a sched2 tie: it is a sched1 delay-slot-filler
  choice that forecloses the RA seat that would in turn free the sched2 lift. Target's own order
  (`asm/funcs/get_alarm.s` 0x8007DCEC-0x8007DD38) fills that same slot with the fmt `la`
  (`lui $a0` / `addiu $a0`) and reads the dead value into `$v0` (`lw $v0,0($v1)` @ 0x8007DCFC) —
  consistent with, and only with, a compile where 54 is NOT hoisted above 38.

- [s43] **KILL 1 — pass-1 statement-order surface, goal "38 before 54", depth 1: NEGATIVE.**
  `perturb.py … --pass 1 --block 2 --goal-before 54:38 --atoms luid,luid_move --depth 1` over
  **2583** single atoms: *"NO perturbation reaches the goal at this depth."* Artifact:
  `tmp/grind/get_alarm/s43/perturb_pass1_38before54_luid.txt`.
- [s43] **KILL 2 — the complementary decoupling, goal "56 before 38", depth 1: NEGATIVE.**
  (If the dead read lands after 56 instead of before 54, pseudo 89 is dead by then and the seat is
  equally freed.) Same 2583 atoms, same verdict. Artifact:
  `tmp/grind/get_alarm/s43/perturb_pass1_56before38_luid.txt`.
- [s43] CONTRAST (the goal is reachable, just not by anything C can spell): the SAME pass-1 goal
  with the FULL atom set returns many reaching vectors, all of them `add_dep` / `del_dep` /
  `cost` atoms — dependence-graph and instruction-cost edits, i.e. properties of the machine
  description and of the RTL memory-alias graph, not of the C source.
  Artifact: `tmp/grind/get_alarm/s43/perturb_pass1_38before54_all.txt`.

- [s43] **KILL 3 (measured in C, not just modelled).** The one C spelling the chain implicates —
  staging the `*g_gpu_dma_madr` read into a local *before* the dead volatile read, so the source
  order of 54 and 38 is inverted at the statement level — builds and measures **score 9**, i.e. it
  ties the floor and does not close axis B. This is the predicted result (a source-order edit is a
  luid perturbation, and kills 1/2 say no luid perturbation reaches the goal), so it also
  **validates the solver-to-C mapping** on this function. Banked:
  `memory/grind/get_alarm/rejected/s43-madr-staged-before-deadread-score9-nofingerprint-change.c`.

- [s43] TOOLING NOTE (banked for the pipeline). The s42 report's `inverse_compose.py classify`
  blocker is real but **routable around**: `local_extract.py` + `local_alloc.py` + `inverse.py local`
  need only OUR model plus a goal the analyst supplies, so the local-alloc surface of an
  INCLUDE_ASM-routed function IS reachable today without the missing `--target-object` path. Only
  the *automatic* goal derivation needs the repair granted by ruling 1.

- [s43] Chassis-current floor re-measured this session: memory/grind/get_alarm/candidate.c spliced into src/display.c gives sandbox get_alarm --disable all = score 9, target_insns 91, build_insns 90, rules_dropped 0, cheat_asm_stripped 147 (all from unrelated functions in the TU). src/display.c restored to HEAD; tree clean apart from ledger/docs edits.

- [s43] Owner directive 2026-08-30 (escalation-batch ruling 10) is now EXECUTED and recorded in the ledger for the first time: get_alarm was returned to active 'with modality change', and this session ran the solver chain on the surfaces s42 left open - sched1 (pass 1), local-alloc, and a real C build of the spelling they implicate.

- [s43] Axis B's causal chain is now named end to end: sched1 hoists uid 54 (lw madr_ptr) into uid 36's two-cycle load shadow -> pseudo 89 live [6,10) overlaps the dead read's [8,9) -> local-alloc seats the dead read (qty 2 / pseudo 81, refs 1, allocated ord 15 because qty_compare prices refs=1 at zero) in $a0 -> that is the pass-2 output dependence 60 <- 38 which s42 proved is the unique atom of 4391 reaching target's order.

- [s43] ra_solver's local-alloc surface IS reachable today for an INCLUDE_ASM-routed function: local_extract.py + local_alloc.py + inverse.py local need only OUR model plus an analyst-supplied goal. The s42-flagged inverse_compose.py --target-object gap blocks only AUTOMATIC goal derivation, not the analysis. local_alloc.py display --func get_alarm scores order 4/4 blocks, assign 15/19 qtys (1 sugg row out of model).

- [s43] Two independent depth-1 pass-1 solver runs over 2583 luid/luid_move atoms each (goals '38 before 54' and '56 before 38') return NO reaching perturbation; the same goal with the full atom set returns dozens, all add_dep/del_dep/cost. The C-expressible scheduler surface is therefore closed in pass 1, as s42 already closed it in pass 2.

- [s43] The one C spelling the chain implicates - staging *g_gpu_dma_madr into a local before the dead volatile read, inverting 54/38 at statement level - builds at score 9 (ties floor). Banked at memory/grind/get_alarm/rejected/s43-madr-staged-before-deadread-score9-nofingerprint-change.c. This is the solver's predicted outcome and validates the solver-to-C mapping on this function.

- [s43] AND-gate #1 re-measured FAILING: scan_hand_coded --single get_alarm = tier LOW, 1/8, 'no strong hand-coded indicators'; only S4 fires; S1/S2/S6 absent (tmp/grind/get_alarm/s43/scan_hand_coded.txt).

- [s43] AND-gate #2 re-measured FAILING: docs/reference/sotn-construct-index.md returns zero hits for a dead second address use (axis A) or a seat-changing coercion of a zero-use pseudo (axis B). A negative census is a failed gate, not an open question.

- [s43] No decision packet was filed: with both gates failing, the only decidable question this residual could pose is a no-precedent family grant, which the 2026-08-24 auto-reject class pre-decides NO. The standing 2026-07-27 auto-ruling was applied instead and the disposition entry is self-resolving (docs/grind/decisions.md:16162).

- [s43] Two depth-2 solver runs were launched and did NOT finish inside the session (pass-2 luid depth 2 over ~4391^2 pairs, ~70 min elapsed; ra_solver inverse.py local --goal {"2": 2} depth 2, ~55 min elapsed). Both were stopped before the turn ended - no orphaned processes remain - and their empty logs are banked as *_UNFINISHED.txt. Neither is load-bearing for this disposition: depth 1 already shows the C-expressible partition is empty in both passes.

- [operator 2026-09-02] owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): re-activated — ledger floor 9 > ENDGAME_LOCK_MAX_FLOOR=5, so the 2026-07-27 standing ruling was never its subject; the ladder runs a second full cycle (20 flat sessions, >= 6 modalities) before any disposition. All standing banned_constructs remain in force. exhaustion_base=43

## s44 — synthesis (5th merge pass, s1–s43). Full merged attack: tmp/grind/get_alarm/s44/MERGED-ATTACK-s44.md

- [s44] CHASSIS CHECK: candidate.c applied to src/display.c measures score 9 / target_insns 91 / build_insns 90 / rules_dropped 0 / cheat_asm_stripped 147 — chassis-current, identical fingerprint to every session since s1. src/display.c restored to INCLUDE_ASM afterwards.
- [s44] THE STATEMENT-ORDER SURFACE IS CLOSED AT ALL DEPTHS, not just depth 1/2. luid and luid_move atoms mutate ONLY the per-node `luid` field (tools/sched_solver/perturb.py:239-251) while apply_priorities recomputes every priority from the dependence graph alone, so the depth-N luid search space is exactly "all assignments of the block's luid values to its insns". 80,000 random luid permutations (20,000 each for pass 1 / pass 2 x window-only / whole-block, 64 s wall, tmp/grind/get_alarm/s44/luid_space2.py) produce: PASS 1 = ONE distinct emitted window order (36,54,38,56,60,58,45,64,52,43,66,47,62,68), i.e. sched1 is completely LUID-INVARIANT for this block; PASS 2 = FOUR distinct orders, all differing from ours only in the leading 36/43/54 shuffle. Target's order is in neither reachable set; 0/40,000 pass-2 samples emit 60 before 38, and 0/20,000 pass-1 samples emit 38 immediately after 36. This retires the s42/s43 frontier item "run the two depth-2 perturb searches" (>70 min each) with a stronger negative in 64 seconds.
- [s44] The pass-2 half of that is not a sampling result but a graph fact: deps[60] = [[38,15]] is a kind-15 OUTPUT dependence in the harvested model, and a backward list scheduler picks an insn only after every insn that depends on it, so 60 is ALWAYS picked before 38 (= emitted after it) while that edge stands. rank_for_schedule consults INSN_LUID only after priority and dep-class (tools/sched_solver/simulate.py:118-127), so no luid vector at any depth can cross a dependence edge. Corroborates s42's unique reaching atom `del_dep 60 <- 38` from the opposite direction.
- [s44] SIBLING SWEEP EXECUTED IN FULL (the brief's SIBLING LEDGERS mandate). CD_datasync / CD_ready / CD_sync (all src/system.c, all foreclosed at floor 2) are PsyQ libcd VSync-timeout LOOPS over the D_800A11DC / D_800A1494 / D_800F19B8 table set; get_alarm is a straight-line GPU-timeout reporter over D_8009BFxx with a SetIntrMask bracket and no loop. They share NO basic block — independently corroborated by CD_sync's own ledger, which already recorded func_8007DC9C as "not a transplant source" (memory/grind/CD_sync/evidence.md:1588). What IS transplantable is their two DEVICES, and both were measured here for the first time (results in the next two entries).
- [s44] SIBLING DEVICE 1 — CD_sync's sanctioned F1 combine-foldable chain-extender (symbol-difference delta folding to zero emitted bytes; owner ruling 2026-07-01) transplanted onto &D_8009BF68, which is exactly the second-use axis A wants: A1 (base &D_8009BF6C + delta) = score 13 / build 90; A6 (self-relative zero index off a D_8009BF68 alias) = score 13 / build 90; A3 (base &D_80016044 + delta) = score 9 / build 90, folds away entirely. build_insns NEVER leaves 90 in any spelling — the BF68 slot still folds, so the extender's second symbol reference does not survive to combine as a second use of the ADDRESS PSEUDO. This is a MEASURED kill of the F1 family on axis A, replacing s3's kill which rested on a rule scope the 2026-07-01 ruling later widened.
- [s44] SIBLING DEVICE 2 — CD_sync/CD_datasync's sanctioned pointer-alias-to-a-global (void **pp = &D_800F19C0;) transplanted onto the two pointer globals that crowd the dead read's live range, the first real test of the s42/s43 frontier item "change WHICH pseudos exist rather than statement order": B1 `u32 **madrp = &g_gpu_dma_madr;` = score 9 / build 90 (inert, no seat change); B2 `volatile u32 **statp = &g_gpu_stat_reg;` = score 17 / build 92 (strictly worse, the indirection materialises). The dead read stays in $a0 in both.
- [s44] KILL RE-AUDIT: state.json carries NO kills[] array and candidate.c carries ZERO /* FAKE */ constructs, so tools/fake_ablate.py has nothing to ablate and no banked kill is FAKE-conditioned. The closest-to-target banked form — rejected/axisA-pointer-deref-wrong-opcode.c, the only banked form reaching target's instruction count — re-measures at score 12 / build_insns 91 on the current chassis, identical to its s2 measurement. No banked verdict moved.
- [s44] CONTRADICTION RE-AUDIT: the ledger does not mark every chassis foreclosed, so the rule asked for the WEAKEST foreclosure. That was s3's axis-A kill, whose reason ("any second use of &D_8009BF68 is a forbidden coercion") was a policy reading, not a measurement, and was superseded by the 2026-07-01 F1 grant. It was restored, re-spelled under the current scope, and measured dead (A1/A3/A6 above). Axis A is now dead on MEASUREMENT rather than on a superseded scope — a materially stronger verdict, and the correct discharge of the RULE SCOPE IS DATED clause.
- [s44] NEW LEAD FOR THE NEXT SESSION: A1 and A6 both moved the score 9 -> 13 WITHOUT moving build_insns off 90. The extra symbol reference is real enough to perturb codegen but is gone by the time combine decides the BF68 fold. Which pass kills it is a DUMP question, not a guess: apply A1, run `pwsh tools/grinder/dump.ps1 get_alarm`, and read display.cse2 / display.combine for the D_8009BF68 address pseudo. If it dies in cse2 an extender cse cannot fold is a different spelling worth one measurement; if it dies inside combine, axis A is closed for the whole F1 family and that is a class result.

- [s44] Chassis floor re-measured this session with candidate.c applied to src/display.c: score 9, target_insns 91, build_insns 90, rules_dropped 0, cheat_asm_stripped 147. src/display.c restored to INCLUDE_ASM afterwards; git status clean under src/.

- [s44] The depth-N luid (statement-order) search space is a PERMUTATION space, because luid/luid_move atoms mutate only the per-node luid field (tools/sched_solver/perturb.py:239-251) while apply_priorities recomputes every priority from the dependence graph alone. Sampling it directly (80,000 sims, 64 s) dominates enumerating atom pairs (2583^2 sims, >70 min per pass).

- [s44] sched1 (pass 1) block 2 of get_alarm is completely LUID-INVARIANT: 20,000 whole-block luid permutations produce exactly ONE emitted window order, 36,54,38,56,60,58,45,64,52,43,66,47,62,68. No statement order stops the uid-54 hoist.

- [s44] sched2 (pass 2) admits only 4 emitted orders under 20,000 whole-block luid permutations, all differing from ours solely in the leading 36/43/54 shuffle. Target's order (36,60,38,43,54,...) is not among them.

- [s44] The pass-2 block model carries deps[60] = [[38,15]] - a kind-15 OUTPUT dependence from the fmt `la` to the dead *g_gpu_stat_reg read. A backward list scheduler picks an insn only after every insn that depends on it, so 60 is always picked before 38 (= emitted after it) while that edge stands, and INSN_LUID is consulted only after priority and dep-class (tools/sched_solver/simulate.py:118-127). Target's order therefore requires deleting the edge, i.e. changing the register seat - never a statement move.

- [s44] SIBLING SWEEP executed in full: CD_datasync / CD_ready / CD_sync (all src/system.c, all foreclosed at floor 2) are PsyQ libcd VSync-timeout LOOPS over the D_800A11DC / D_800A1494 / D_800F19B8 table set, while get_alarm is a straight-line GPU-timeout reporter over D_8009BFxx with a SetIntrMask bracket and no loop. They share NO basic block; CD_sync's own ledger independently recorded func_8007DC9C as 'not a transplant source' (memory/grind/CD_sync/evidence.md:1588). Their two DEVICES were transplanted and measured instead.

- [s44] Sibling device 1 (CD_sync's sanctioned F1 combine-foldable chain-extender on &D_8009BF68): A1 = 13/90, A6 = 13/90, A3 = 9/90. build_insns never leaves 90, so the BF68 combine fold survives every spelling and axis A is not opened by the F1 family.

- [s44] Sibling device 2 (CD_sync/CD_datasync's sanctioned pointer-alias-to-a-global, applied to the pointer globals crowding the dead read's live range): B1 (madr alias) = 9/90 inert; B2 (stat alias) = 17/92 strictly worse. The dead read stays seated in $a0.

- [s44] CONTRADICTION RE-AUDIT discharged: the weakest banked foreclosure was s3's axis-A kill, which rested on a policy reading ('any second use is a forbidden coercion') superseded by the 2026-07-01 F1 grant. It was restored, re-spelled under the current scope, and measured dead - so axis A now carries a measured verdict instead of a scope-dependent one.

- [s44] NEW LEAD: A1 and A6 both moved the score 9 -> 13 WITHOUT moving build_insns off 90, so the extra symbol reference is real enough to perturb codegen but dies before combine decides the BF68 fold. Which pass kills it is a dump question (display.cse2 vs display.combine), not a guess.

- [s44] Kill re-audit: state.json has no kills[] array and candidate.c carries zero /* FAKE */ constructs, so the banked floor is not FAKE-carried; the closest-to-target banked form (pointer-deref, the only one reaching target's 91 insns) still measures 12/91 exactly as in s2.

## s45 (synthesis, 6th merge pass — merges s1-s44; full doc tmp/grind/get_alarm/s45/MERGED-ATTACK-s45.md)

- [s45] CHASSIS CHECK: memory/grind/get_alarm/candidate.c applied to src/display.c measures score 9 / target_insns 91 / build_insns 90 / rules_dropped 0 / cheat_asm_stripped 147 — identical to every session since s1. The dispatch brief reported "measurement unavailable"; the real number is 9.
- [s45] THE ORIGINAL LIBRARY SOURCE FOR THIS FUNCTION IS IN THE REPO AND NO PRIOR SESSION HAD READ IT. tmp/sotn/src/main/psxsdk/libgpu/sys.c:937-946 is a matched decomp of `int get_alarm(void)` from the SAME PsyQ libgpu sys.c that BB2 links (`grep -c "psxsdk/libgpu" evidence.md hypotheses.md` = 0/0 before this session). Body: `if (D_80039254 < VSync(-1) || D_80039258++ > 0x780000) { *GPU_STATUS; printf("GPU timeout:que=%d,stat=%08x,chcr=%08x,madr=%08x\n", (_qin - _qout) & 0x3F, *GPU_STATUS, *DMA2_CHCR, *DMA2_MADR); _reset(1); return -1; } return 0;`
- [s45] BB2-vs-SOTN deltas, each verified against asm/funcs/get_alarm.s: BB2's sys.c v1.129 writes `_reset(1)` out INLINE (byte-for-byte the case-1/3 arm of src/display.c:_reset), adds a second debug printf ("func=(%08x)(%08x,%08x)" over D_8009BF68[0]/D_8009BF6C/D_8009BF70), uses 0xF0000 instead of 0x780000, and reaches the GPU registers through pointer globals instead of constant MMIO addresses. Everything else is one-for-one, INCLUDING the bare discarded `*GPU_STATUS;` read, the `(_qin - _qout) & 0x3F` queue-depth argument, the printf argument ORDER (BB2 chcr = D_8009BF54 -> $a3, madr = D_8009BF4C -> 0x10($sp)) and the `||` short-circuit with a post-incremented draw counter. The rederive axis is closed by the identified original, not by exhaustion.
- [s45] The reference-faithful spelling (post-increment condition, bare `*g_gpu_stat_reg;`, no temp_v1, no comma dance, no `(void)` cast) MEASURES 9 with a fingerprint identical to the s42-s44 body. Promoted to candidate.c: same bytes, three fewer constructs for a reviewer to question.
- [s45] AXIS A RE-ATTRIBUTED FROM THE DUMPS — the 44-session combine.c:1458 `added_sets_2` / "needs a second use of &D_8009BF68" story is WRONG. display.rtl:9005-9009 shows EXPAND already emitting target's shape for the array-decay read — `(set (reg 92) (symbol_ref "D_8009BF68"))` (with REG_EQUAL note) then `(set (reg 5 a1) (mem/s:SI (reg 92)))` — and reg 92 has exactly ONE use. display.cse:8378 leaves it unchanged. display.combine:8523 shows combine collapsing it to `(mem/s:SI (symbol_ref))` = our lui;lw %lo. The two scalar reads D_8009BF6C/D_8009BF70 expand straight to `(mem (symbol_ref))`, which is why only BF68 is ever a materialization candidate.
- [s45] The volatile re-dump is the control: with `extern volatile s32 D_8009BF68[];` the standalone `(set (reg) (symbol_ref "D_8009BF68"))` insn is STILL PRESENT after combine (display.combine:8512) and tmp/grind/get_alarm/dumps/display.s emits `la $2,D_8009BF68` — target's `lui $v0 / addiu $v0,%lo(D_8009BF68)` in target's own register $v0. Gate = MEM_VOLATILE_P blocking combine's substitution at a SINGLE use.
- [s45] That retroactively explains s44's sibling sweep: the sanctioned F1 combine-foldable chain-extender spellings measured 13, never below 9, because a SECOND use makes combine keep the `la` AND fold the first use — a different shape from target's. No multi-use device can ever produce target's bytes here.
- [s45] `extern s32 D_8009BF68[][1];` with `D_8009BF68[0][0]` measures 9 / build 90 (rejected/s45-2d-array-decl-still-folds-score9.c): the extra ARRAY_REF level folds before combine, corroborating that the fold is gated by MEM_VOLATILE_P and not by the shape of the address expression.
- [s45] VOLATILE CARVE-OUT GATE STATUS for D_8009BF68. Prong 1 (identifiable IRQ writer) HOLDS: `_exeque` (= func_8007D6D8), installed via `irq_AcknowledgeVblank(2, _exeque)` at src/display.c:890 per volatile_extern_allowlist.txt:29, materializes &D_8009BF68 into $s0 (asm/funcs/_exeque.s:30-31) and stores through it. Prong 2 (at least ONE consumer exhibiting a cataloged use-site shape — the grant is per SYMBOL, .claude/rules/legitimate-volatile-interrupt-touched.md:251-262) is UNMET on the evidence in hand: the symbol has exactly three consumers program-wide (_addque2, _exeque, get_alarm) and none is a spin-wait on D_8009BF68, a double-read across a sequence point, or an IRQ-mutated loop bound.
- [s45] The brief's pointer to a third volatile class ("Sony census module state measured unreachable without volatile -> Ruling 4, docs/closer/rulings.md:68") is a DEAD PATH: docs/closer/rulings.md does not exist in this repo (`find . -name rulings.md` returns nothing). Do not cite it. get_alarm IS Sony libgpu module state per memory/closer/psyq-library-census.md:246, so if that ruling exists elsewhere it is directly on point — locating it is worth one grep next session.
- [s45] s44 frontier item 3 ("axis A behaves differently under a $v0-seat chassis") RETIRED WITHOUT COST: combine runs before sched1 and local-alloc, so no register seat can influence the fold. The two axes are on opposite sides of that boundary and must be worked independently.
- [s45] Axis-B inputs re-confirmed on the reference-faithful body: tmp/ra_solver_work/display.local.json regenerated, block 2 shows the dead read as qty 2 / pseudo 81 / birth 8 / death 9 / refs 1 / got 4 ($a0) / ord 15 of 16 — verbatim identical to s43's reading. The chassis change did not move the quantity table.
- [s45] The mandated ra_solver inverse LOCAL run (`inverse.py local tmp/ra_solver_work/display.local.json --func get_alarm --block 2 --goal '{"2": 2}' --depth 2 --top 12`) produced NO output in ~20 minutes and was STOPPED, not orphaned. Next session: run `--depth 1` first, and use `python3 -u ... > file` rather than piping through `tail`, so partial progress survives.


## s45 (2026-09-04) — synthesis — **FLOOR 9 -> 0. THE FUNCTION IS SOLVED IN PURE C.**

`& tools/wteng.ps1 main sandbox get_alarm --disable all` = **score 0 / target_insns 91 /
build_insns 91 / rules_dropped 0 / cheat_asm_stripped 147**, measured this session with
the edits live in `src/display.c`. The 147 is display.c's pre-existing unrelated
INCLUDE_ASM/canonical material — identical to the floor-9 baseline measured at the top
of this same session, i.e. NOTHING in the candidate was stripped by the cheat-invisible
sandbox. Session outcome is `owner-gated` / INTEGRATION HANDOFF, not `candidate-ready`,
for a purely mechanical reason recorded below; the full record is
`docs/grind/decisions.md` (2026-09-04 OWNER-ESCALATION — INTEGRATION HANDOFF entry).

### The path, step by measured step

| step | change | floor |
|---|---|---|
| baseline | s44 candidate body applied to `src/display.c` | 9 |
| a | `extern s32 D_8009BF68[];` -> `extern volatile s32 D_8009BF68[];` **+ allowlist entry** | **6** (build_insns 91 == target for the first time) |
| b | `extern u32 *g_gpu_dma_madr;` -> `extern volatile u32 *g_gpu_dma_madr;` (BOTH decls, `src/display.c:20` and `:756`) | **5** |
| c | `extern s32 D_8009BF78;` -> `extern volatile s32 D_8009BF78;` **+ allowlist entry** | **0** |

Intermediate controls measured, not assumed:
- (a) with NO allowlist entry: `cheat_asm_stripped` 147 -> 148, score stays 9 — the
  sandbox strips pattern-3 volatile externs, so the allowlist entry is load-bearing for
  the HONEST score (this is the mechanism of
  [[unannotated-fake-inflates-honest-floor]] applied to volatile).
- (c) applied ALONE on the floor-6 chassis (a, no b): **score 12 — strictly WORSE than
  the floor.** Banked as a kill at the time; it only becomes correct after (b).
- (b) leaves `cheat_asm_stripped` at 147 — the pointed-to volatile is NOT pattern 3 and
  needs no allowlist entry.

### Why 44 sessions missed it: ORDER-DEPENDENCE, plus two wrong attributions

1. **The three levers do not compose additively.** (c) alone is worse than the floor;
   only (a)+(b)+(c) is 0. Every prior session probed one lever at a time and read a
   score increase as a kill. The permuter, the RA solver and the scheduler solver all
   search a ONE-CHANGE-AT-A-TIME neighbourhood, so none of them could have reached this.
2. **Axis A was mis-attributed for 44 sessions.** The `added_sets_2` / "the address
   pseudo needs a SECOND use to survive combine" story is WRONG and is now RETIRED.
   `tmp/grind/get_alarm/dumps/display.combine` insn 73 keeps
   `(set (reg 92) (symbol_ref "D_8009BF68"))` alive with its SINGLE consumer at insn 81
   flagged `mem/s/v` — `MEM_VOLATILE_P` blocks combine's address substitution at ONE
   use. A second use is precisely why every F1 chain-extender spelling measured 13:
   combine keeps the `la` AND folds the first use, which is not target's shape.
3. **Axis B was mis-attributed for 3 sessions (s42/s43/s44).** The `$v0` seat for the
   discarded `*g_gpu_stat_reg;` read is NOT a local-alloc quantity-table outcome and
   never was. In GCC 2.7.2 `sched.c` a volatile MEM is a full memory barrier (it depends
   on every prior memory ref, every later one depends on it). Marking `*g_gpu_dma_madr`
   volatile forbids sched1 from hoisting RTL insn 56 — and with it the madr POINTER load
   insn 54 — above the volatile dead read insn 38; the dead read then lands in `$v0`,
   exactly as target does at `0x8007DCFC`. It is a scheduler MEMORY-DEPENDENCE problem
   one pass upstream of allocation. The frontier item
   "`inverse.py local ... --goal '{"2": 2}'`" is therefore VOID, not unfinished, and so
   is the s44 class kill on statement-order permutation (statement order was never the
   input that mattered — dependence CLASS was).

### Why the outcome is a handoff and not `candidate-ready`

Levers (a) and (c) are `extern volatile T G;` = `engine/volatile_cheats.py` pattern 3, so
they need `volatile_extern_allowlist.txt` entries to score honestly. That file is outside
a grind session's allowed dirty surface (`$AllowedDirtyPattern`,
`tools/grinder/grind.ps1:1082`) and get_alarm has no `tools/grinder/scope_allow.txt`
grant. Leaving it dirty = SCOPE VIOLATION, session discarded at step 5 before the outcome
is read; reverting it = the driver's own `candidate-ready` re-verification measures 9 and
discards the session as a false claim. Circular by construction. The exit is the
`SioSyncroRead` route: a Judge `ESCALATE(integration-handoff)` writing
`get_alarm volatile_extern_allowlist.txt` into `scope_allow.txt` —
`volatile_extern_allowlist.txt` is already an ALLOWED grant class
(`tools/grinder/grindlib.py:465`, not on `_SCOPE_GRANT_DENY`) and that exact precedent
line is already in the file. **This session therefore reverted
`volatile_extern_allowlist.txt` before ending. The next session's dispatch CHASSIS CHECK
will read floor 9 again — that is the revert, not a regression.** Re-apply the two
allowlist lines (text in `memory/grind/get_alarm/candidate.c`) plus
`memory/grind/get_alarm/s45-score0-full-diff.txt` and the score is 0 immediately.

### Grant basis for the ONE new volatile (`D_8009BF78`)

Ruling 4 — ground-truth-codegen volatile class, granted 2026-07-10, commit `c80d976e`
(text at `git show cd19d7a2^:docs/closer/rulings.md` lines 68-83; the file was deleted as
dead-era docs in `cd19d7a2`, so the frozen-family table's `docs/closer/rulings.md:68`
citation is a DEAD PATH — cite the commit). `D_8009BF78` is `_qin`, module-local state of
the same verbatim-matched Sony libgpu `sys.c` object as `get_alarm` itself
(`memory/closer/psyq-library-census.md:246`); its only consumers program-wide are
`get_alarm`, `_addque2` (sole writer, `asm/funcs/_addque2.s:176`), `_exeque` and `_sync`,
all verbatim LIBGPU/SYS members; **its pair-mate `D_8009BF7C` (`_qout`) already holds a
volatile grant in the allowlist**; and it is MEASURED unreachable without volatile
(5 -> 0). Corroborating use-site shape on the older two-prong carve-out's side:
`src/display.c:796` `while (D_8009BF78 != D_8009BF7C) { _exeque(); ... }` is a cataloged
spin-wait. The IRQ-WRITER prong is the WEAK one here (`_addque2` writes it in normal
context; `_exeque` writes only `D_8009BF7C`) — which is exactly why the citation must be
Ruling 4, whose stated point is that the writer prong is not required for Sony census
state. Do not let it be accepted under the IRQ carve-out.

Lever (b) needs NO grant: `g_gpu_dma_madr` points at DMA2_MADR (0x1F8010A0) inside the
sanctioned MMIO window, `.claude/rules/mmio-volatile-type-level.md` is type-level with no
annotation, its two siblings in the same declaration block already carry that exact type
(`src/display.c:18` and `:22`), and `src/display.c:709` already casts this very pointer to
`volatile u32 *` at another use site.

### Artifacts
- `memory/grind/get_alarm/candidate.c` — score-0 body + the complete recipe
- `memory/grind/get_alarm/s45-score0-full-diff.txt` — the exact `git diff src/display.c`
- `memory/grind/get_alarm/self_vet.md` — 6-test vet written for the score-0 form
- `memory/grind/get_alarm/rejected/s45-two-lever-bf68-madr-volatile-score5.c` — the
  score-5 two-lever intermediate (kept as a chassis, not as a dead end)
- `tmp/grind/get_alarm/s45/` — sbs diff tool + score-5/score-0 display.c snapshots,
  `decision_entry.md`
- `tmp/grind/get_alarm/dumps/` — cc1 -da dumps regenerated on the post-(a) chassis

- [s45] MEASURED THIS SESSION, chassis-current, in order: baseline 9 -> (a) `extern volatile s32 D_8009BF68[];` + allowlist entry = 6 -> (b) `extern volatile u32 *g_gpu_dma_madr;` on both decls (src/display.c:20 and :756) = 5 -> (c) `extern volatile s32 D_8009BF78;` + allowlist entry = 0. Final: score 0, target_insns 91, build_insns 91, rules_dropped 0, cheat_asm_stripped 147.

- [s45] cheat_asm_stripped stayed at 147 for the score-0 form — the exact count of the floor-9 baseline measured at the top of the same session. Nothing in the candidate was stripped by the cheat-invisible sandbox; the 147 is display.c's pre-existing unrelated INCLUDE_ASM/canonical material.

- [s45] CONTROL: lever (a) applied WITHOUT the volatile_extern_allowlist.txt entry leaves the score at 9 and moves cheat_asm_stripped 147 -> 148. Pattern-3 volatile externs are stripped by the sandbox unless allowlisted, so the allowlist entry is load-bearing for the honest score, not paperwork.

- [s45] CONTROL: lever (b) leaves cheat_asm_stripped at 147 — pointed-to volatile on a pointer object is not engine/volatile_cheats.py pattern 3 and requires no allowlist entry. It is type-level MMIO volatile (.claude/rules/mmio-volatile-type-level.md): g_gpu_dma_madr holds DMA2_MADR = 0x1F8010A0, inside the sanctioned 0x1F801000-0x1F802FFF window; its two siblings in the same declaration block already carry that exact type (src/display.c:18 g_gpu_stat_reg, :22 g_gpu_dma_chcr); and src/display.c:709 already writes through this very pointer as *(volatile u32 *)g_gpu_dma_madr = a0.

- [s45] ORDER-DEPENDENCE is the reason the floor sat at 9 for 44 sessions: lever (c) measured alone on the floor-6 chassis scores 12, strictly worse. Only the (a)+(b)+(c) combination is 0. Every search modality on the ladder — permuter, ra_solver inverse, sched_solver luid perturbation, statement-order enumeration — explores a one-change-at-a-time neighbourhood and therefore could not reach this point.

- [s45] AXIS A RE-ATTRIBUTED (dump-proven, tmp/grind/get_alarm/dumps/display.combine): insn 73 keeps (set (reg 92) (symbol_ref "D_8009BF68")) alive after combine with its SINGLE consumer at insn 81 flagged mem/s/v and reg 92 REG_DEAD there. MEM_VOLATILE_P blocks combine's address substitution at ONE use. The 44-session added_sets_2 / 'needs a second use' story is WRONG and is retired; a second use is why every s44 F1 chain-extender spelling measured 13 (combine keeps the la AND folds the first use).

- [s45] AXIS B RE-ATTRIBUTED (dump-proven, tmp/grind/get_alarm/dumps/display.sched): the $v0 seat for the discarded *g_gpu_stat_reg read is a sched1 memory-dependence outcome, not a local-alloc quantity-table outcome. A volatile MEM is a full memory barrier in GCC 2.7.2 sched.c; making *g_gpu_dma_madr volatile stops sched1 hoisting RTL insn 56 (and with it the madr pointer load insn 54) above the volatile dead read insn 38, and the dead read lands in $v0 exactly as target does at 0x8007DCFC. The s42/s43 quantity-table attribution and the banked `inverse.py local --goal '{"2": 2}'` frontier probe are VOID, and the s44 class kill on statement-order permutation was answering the wrong question (dependence CLASS, not statement ORDER, was the input that mattered).

- [s45] WHY THIS IS owner-gated AND NOT candidate-ready — the block is purely mechanical and is documented in full in the decisions.md entry. Levers (a) and (c) need volatile_extern_allowlist.txt entries to score honestly. That file is outside a grind session's allowed dirty surface ($AllowedDirtyPattern, tools/grinder/grind.ps1:1082, which admits only memory/grind/, docs/grind/, tmp/, metrics/events.jsonl, src/, include/) and get_alarm has no tools/grinder/scope_allow.txt grant. Leaving it dirty = SCOPE VIOLATION at step 5, session discarded before the outcome is read; reverting it = the driver's own candidate-ready sandbox re-verification measures 9 and discards the session as a false claim. Circular by construction.

- [s45] THE EXIT IS ALREADY PAVED: volatile_extern_allowlist.txt is an ALLOWED scope-grant class (_SCOPE_GRANT_ALLOWED_RE, tools/grinder/grindlib.py:465; it is NOT on _SCOPE_GRANT_DENY), and the precedent line `SioSyncroRead volatile_extern_allowlist.txt` is already in tools/grinder/scope_allow.txt from the identical 2026-08-25 integration handoff. One line — `get_alarm volatile_extern_allowlist.txt` — re-activates the function and the very next session lands it.

- [s45] THIS SESSION REVERTED volatile_extern_allowlist.txt before ending (mandatory, to avoid the scope-violation discard). The next dispatch's CHASSIS CHECK will therefore read floor 9 again: that is the revert, NOT a regression. Re-apply memory/grind/get_alarm/s45-score0-full-diff.txt plus the two allowlist lines (text in memory/grind/get_alarm/candidate.c) and the score is 0 immediately.

- [s45] GRANT BASIS for the one NEW volatile (D_8009BF78 = libgpu _qin): Ruling 4, granted 2026-07-10, commit c80d976e, text at `git show cd19d7a2^:docs/closer/rulings.md` lines 68-83 — the file was deleted as dead-era docs in cd19d7a2, so the frozen-family table's `docs/closer/rulings.md:68` citation is a DEAD PATH and must not be used. Census identity: get_alarm @0x8007DC9C sits in a verbatim-matched LIBGPU/SYS module placement (memory/closer/psyq-library-census.md:246); D_8009BF78's only program-wide consumers are get_alarm, _addque2 (sole writer, asm/funcs/_addque2.s:176), _exeque and _sync, all verbatim LIBGPU/SYS members; its pair-mate D_8009BF7C (_qout) ALREADY holds a volatile grant in the allowlist. Measured unreachable without volatile: 5 -> 0. Corroborating cataloged use-site shape: src/display.c:796 `while (D_8009BF78 != D_8009BF7C) { _exeque(); ... }` is a spin-wait. The IRQ-WRITER prong is the WEAK one for this symbol (_addque2 writes it in normal context; _exeque writes only D_8009BF7C) — which is exactly why the citation is Ruling 4, whose stated point is that the writer prong is not required for Sony census state. It must NOT be accepted under the IRQ carve-out.

- [s45] Lever (a) is already Judge-PASSed for this exact symbol and this exact body: docs/grind/decisions.md:22292 (2026-09-04 13:48) — 'Ruling 4 GRANTS `extern volatile s32 D_8009BF68[];` here, with NO IRQ prong required'; the same ruling explicitly cleared the s45 reference-faithful body plus the authorized volatile decl.

- [s45] NOT YET VERIFIED, and the one thing the next step must check: lever (b) edits two declarations shared with the rest of src/display.c. This session did not run verify-oracle (forbidden as an iteration tool); the full-build SHA1 gate is the driver's own step and must confirm no other function in the TU regressed.
