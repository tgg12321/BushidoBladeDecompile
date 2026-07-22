# Hypothesis ledger — func_8007DC9C

Floor = 9 (verdict C, target 91 / build 90 insns). Gap decomposes into two
independent axes: Axis A = 1 combine-fold insertion (BF68[0]); Axis B = 8-op
sched1 reorder cluster in the first debug_printf setup.

## Live frontier (mechanism-grounded, built s1 recon)

### H-A1 — offset-0 combine fold on D_8009BF68[0] (Axis A, 1 op)
- statement: target keeps a 3-insn materialized address for D_8009BF68[0]; our
  build combine-folds it to 2 insns. A declaration/access shape exists that
  suppresses the offset-0 fold in pure C.
- mechanism: combine.c folds `(mem (plus symbol_ref 0))` → `(mem symbol_ref)`.
  [[defeat-combine-symbol-fold]] excludes offset 0.
- next_probe: read tools/gcc-2.7.2/combine.c symbol+0 simplification path; test
  a declaration consistent with BF68's real function-pointer type (see H-A2).
- status: OPEN. Fold-defeat family already exhausted in the imported rejected
  bank (sized array / fn-ptr cast / intervening-call precompute all → 9). Do NOT
  re-run those; the un-tried lever is a combine.c-informed decl shape.

### H-A2 — D_8009BF68 real type is a function pointer, not an int array (Axis A)
- statement: the two in-tree block-scope externs disagree (line 872 fn-ptr vs
  line 965 array); func_8007D3F8 assigns `D_8009BF68 = arg0` (scalar fn-ptr) and
  fills a fn-ptr table. Declaring/accessing BF68 consistent with its true scalar
  fn-ptr type may emit the materialized (non-folded) load target wants.
- mechanism: a scalar (non-array) symbol read vs an array[0] read can differ in
  the address RTL combine sees.
- next_probe: after func_8007D3F8 matches (queue ~43, only other user), adopt its
  committed decl shape here; or probe the scalar-read shape directly now.
- status: OPEN, partly a sibling dependency (non-blocking; can probe scalar-read now).

### H-B1 — sched1 fmt-vs-BF78 load-order tie-breaker (Axis B, 8 ops)
- statement: target schedules the fmt address load BEFORE the BF78 load chain, so
  the early *stat_reg read allocates to $v0; our build loads BF78 first, pushing
  the early read to $a0 and fmt later. A C statement-order / liveness lever flips
  the tie-break.
- mechanism: sched.c insn priority / LUID tie-break in the first debug_printf
  argument setup. Closed by regfix `reorder 21,20,19,18` + `subst lw $4→$2`.
- next_probe: run instrumented cc1 (tmp/gccdbg/cc1, WSL) with BB2_SCHED_DEBUG=1
  BB2_PRIO_DEBUG=1 on the sandbox .i; read the priority values for the fmt-LUI vs
  BF78-LUI nodes; derive the C lever (arg-order / local-hoist / liveness). The
  imported block-scope/fmt-local/(void)-read levers did NOT move the schedule —
  the dump is the missing map.
- status: OPEN, highest-value un-run probe (8 of 9 ops).

## Rejected forms — see evidence.md (imported WIP bank). Do NOT re-propose.

## [s1] Honest floor is 9 and the 9-op gap decomposes into 1 combine-fold insertion (D_8009BF68[0]) plus an 8-op sched1 reorder cluster in the first debug_printf setup.
- mechanism: canonical gate + cheat-invisible sandbox score, cross-read against asm/funcs/func_8007DC9C.s: target materializes BF68 address (3 insns) where our build folds (2), and target orders fmt-load before the BF78 chain where ours loads BF78 first.
- probe: canonical func_8007DC9C; sandbox func_8007DC9C --disable all; read asm/funcs/func_8007DC9C.s lines 23-53.
- result: canonical verdict C distance 9; sandbox score 9 (target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150). Axis A + Axis B confirmed as documented.
- verdict: CONFIRMED

## [s2] H-B1: a C statement-order/liveness lever flips the sched1 fmt-vs-deadread order so fmt schedules first (into $a0) and the dead *g_gpu_stat_reg read lands in $v0 (as target does).
- mechanism: .greg dump: dead read (insn 38) is volatile-ordered before the volatile D_8009BF7C read (insn 45), putting it on the critical path dead-read->BF7C->subu(arg1)->call. That gives it high sched1 PRIORITY (critical-path length), so it schedules early and grabs $a0; fmt (a lone `la`, chain length 1) schedules after and reloads $a0. Ordering is priority-driven, not LUID/source-order driven.
- probe: sandbox sweep of 8 structural forms: fmt-precompute-first, fmt-stmt-first, bare-(void)-read+fmt, arg1-temp, madr-temp, bf78-temp, new_var-declared-first, split-subtraction. Cross-checked against the greg dump insn allocation (insn 38 dead read -> reg/v 4 a0, REG_UNUSED; insn 60 fmt la later).
- result: All fmt/arg-precompute/decl-order forms scored 9 (no schedule change); split-subtraction 12; chcr-cache 19. The volatile ordering dead-read<BF7C matches target and is unswappable (observable), so the dead read stays high-priority regardless of source order.
- verdict: KILLED

## [s2] H-A1: a declaration/access shape suppresses the combine offset-0 symbol+0 fold so D_8009BF68[0] emits target's 3-insn materialized-address load (la sym; lw 0(reg)).
- mechanism: Our combine dump represents the access as (mem/s:SI (symbol_ref D_8009BF68)) -> folded to 2-insn lui;lw %lo. Target keeps the address in a register (la=lui+addiu) then lw 0(reg); combine only preserves (set reg sym)(mem reg) unmerged when the address pseudo has MULTIPLE uses (it will not propagate a multi-use reg into the mem). func_8007DC9C has exactly one use of &D_8009BF68, so combine always folds.
- probe: sandbox: array[0] (baseline), sized array[][1] 2D, scalar s32, scalar fn-ptr, s32* pointer-deref, local-base pointer precompute, one-array (BF6C/BF70 as bf68[1]/[2]).
- result: All fold to the 2-insn form (score 9) EXCEPT s32* pointer-deref, which gives build_insns=91 (target count!) but the middle insn is `lw` (load pointer value) where target has `addiu` (materialize address) -> score 12. So BF68 is not a pointer global, and no decl shape materializes the address for a single-use offset-0 access.
- verdict: KILLED

## [s2] H-A2: declaring/accessing D_8009BF68 consistent with its true scalar function-pointer type (per sibling func_8007D3F8, line 872) emits the materialized non-folded load.
- mechanism: A scalar (non-array) symbol read presents a different address RTL to combine than array[0].
- probe: sandbox: extern s32 (*D_8009BF68)(s32*,s32) with the value passed directly as the printf arg; also plain extern s32 D_8009BF68 scalar.
- result: Both scalar forms fold to lui;lw %lo (2 insns, score 9) — a scalar read never materializes the address. Same fold as the array form. H-A2 does not produce the 3-insn shape.
- verdict: KILLED
