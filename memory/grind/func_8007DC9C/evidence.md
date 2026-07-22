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
