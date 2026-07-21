# Evidence bank — cpu_get_dist

- WIP rejected_form: {'form': 'v1_clean (cos+vx+sin+vz+rx+rz+v48 single-statement decls in source order)', 'score': 21, 'reason': 'baseline natural form; sched1 hoists lw vx into cos_val region'}

- WIP rejected_form: {'form': 'v2_earlyloads (vx,vz,v48 loaded BEFORE cos/sin)', 'score': 21, 'reason': "LUID reordering doesn't change sched1's class-3 preference"}

- WIP rejected_form: {'form': 'v3_inline_vx (precomputed cross-product locals p_vxc/p_vzs/...)', 'score': 21, 'reason': "intermediate naming doesn't restructure live ranges enough"}

- WIP rejected_form: {'form': 'v4_vx_first (vx declared before cos_val)', 'score': 21, 'reason': "vx's lower LUID doesn't override class-tie-break"}

- WIP rejected_form: {'form': 'v5_struct_access (K&R top-of-block decls, identical to HEAD layout minus the cheats)', 'score': 21, 'reason': 'K&R-style decl alone gives no coercion effect'}

- WIP rejected_form: {'form': 'v6_cross (named partial products before the rx/rz sums)', 'score': 21, 'reason': "explicit naming doesn't change combined-mult shape"}

- WIP rejected_form: {'form': 'v7_callret_v0 (K&R + clean assignments)', 'score': 21, 'reason': 'same as v5; no behavior change'}

- WIP rejected_form: {'form': 'v8_pre_negate (s32 neg_vx = -vx; precomputed)', 'score': 21, 'reason': "neg_vx alias doesn't affect sched1's decision on lw vx"}

- WIP rejected_form: {'form': 'v9_judge_local (s16 *judge = &Judge; precomputed)', 'score': 21, 'reason': 'Judge pointer materialization is folded back by GCC; identical RTL'}

- WIP rejected_form: {'form': 'v10_inline_call (separate cos_idx and sin_idx locals)', 'score': 21, 'reason': "split scratch locals don't affect the lh-vs-lw choice"}

- WIP rejected_form: {'form': 'v11_cossin_first (cos+sin declared together, then vx, vz)', 'score': 21, 'reason': 'later-LUID vx still wins class-3 over class-1 lh'}

- WIP rejected_form: {'form': 'v12_array (a0[17] instead of (s32*)((u8*)a0+0x44))', 'score': 21, 'reason': 'array-vs-cast emits identical RTL'}

- WIP rejected_form: {'form': 'v13_macro (u8 *base = (u8*)a0; reused)', 'score': 21, 'reason': 'base alias folded by GCC; identical RTL'}

- WIP rejected_form: {'form': 'v15_subtract_const (cos_idx = (angle - 0xC00) & 0xFFF; math-equivalent)', 'score': 22, 'reason': 'addiu -0xC00 changes RTL constant; combine.c folds back to addiu +0x400 OR sched1 differs; either way regresses one insn'}

- WIP rejected_form: {'form': 'v16_late_v48 (v48 in inner scope after both stores)', 'score': 21, 'reason': 'scope wrap does not affect sched1 (no LOOP_BEG note generated)'}

- WIP rejected_form: {'form': 'v17_cos_addr_local (s16 *cos_addr = ...; cos_val = *cos_addr;)', 'score': 21, 'reason': 'explicit address-pointer materialization GCC-folded back into single combined load'}

- WIP rejected_form: {'form': 'v18_cos_first_use (vx/vz declared at top but assigned later)', 'score': 21, 'reason': "decl-before-assign doesn't change LUID of assignments"}

- WIP rejected_form: {'form': 'v19_judge_indexed (s16 *judge = &Judge; judge[idx])', 'score': 21, 'reason': 'pointer alias folds back to global address; identical RTL to direct &Judge+idx'}

- WIP rejected_form: {'form': 'v20_vx_byte_offset (u8 *base = (u8*)a0; reused for all offsets)', 'score': 21, 'reason': 'duplicate of v13 essentially; base ptr folded'}

- WIP rejected_form: {'form': 'v21_split_vx_rx (vx_cos/vz_sin/vz_cos/nvx_sin named locals)', 'score': 27, 'reason': 'explicit named partial products force separate pseudos; regresses RA'}

- WIP rejected_form: {'form': 'v22_call_inline (top-of-block decls, no init; assigned later)', 'score': 21, 'reason': 'K&R top decls (no init) same as v5_struct_access'}

- WIP rejected_form: {'form': 'v23_neg_vx_local (s32 nvx = -vx; used in rz comp)', 'score': 21, 'reason': 'named -vx as separate local; combine.c folds negu into surrounding mult anyway'}

- WIP rejected_form: {'form': 'v24_baseline (literal candidate.c, sanity check)', 'score': 21, 'reason': 'baseline confirmation — no semantic change'}

- == imported from memory/wip notes.md ==
# cpu_get_dist (code6cac_b.c) — pure-C wall analysis

## TL;DR (re-confirmed 2026-06-14 floor 15; breakthrough 2026-06-13 floor 21 -> 15)

> 2026-06-14 (orch0614b-4): resumed candidate.c, re-verified clean floor **15**
> (build_insns 63 vs target 62). 2 fresh load-ordering levers (v48-early,
> vx-after-sin) both 15 — statement order does NOT break the sched1 tie (consistent
> with the INSN_PRIORITY deep-dive below). Next modality unchanged: directed
> permuter RE-SEEDED from the score-15 candidate.c. Resume from candidate.c, not HEAD.


- **BREAKTHROUGH 2026-06-13: clean floor lowered 21 -> 15** via the "fundamentally different algebraic factoring" the prior notes flagged as the ONE untried clean direction. candidate.c is now the score-15 form: **rz computed BEFORE rx, with rz's inner sum written vz*cos-first `((vz*cos)+((-vx)*sin))`** (rx unchanged). Pure reassociation + statement order; cheat-reviewer PASS. So the prior "clean floor = 21 / matching C outside every structural dimension" conclusion was WRONG — statement-emission order of the products was the untested dimension.
- **Decomposition** (tmp/cgd_alg + tmp/cgd2 sweeps): rz-first-with-vz*cos-lead = 15. Swapping rx's addends too regresses (24-28). Store-operand swaps regress (24-26). `(-vx)*sin -> -(vx*sin)` sub form = 61 insns / score 17. Best clean = 15 (== tmp/cgd2/r6_base15.c).
- **Remaining gap (masked 15, build_insns 63 vs target 62):** unmasked diff (tmp/cgd_diff.py) shows target's mult order is **vx*cos, vz*sin, (-vx)*sin, vz*cos with vz loaded LATE**; my 15-form front-loads vz*cos. Residual is sched1 register allocation, NOT instruction set. **Next modality: directed permuter RE-SEEDED from this 15-floor candidate.c** (the prior 2 campaigns seeded from the 21 base and found only cheat-forms). PARKED 2026-06-13 for the 50-item-goal session (set-aside-walls directive); resume here.
- **HEAD's body uses two forbidden coercion cheats** (do-while(0) wrapper + dead `rx = ... uninit vz ... >> 12;` read-before-assignment). Both are
  [[no-new-park-categories]] cheats-by-spelling. With them, cheat-stripped sandbox = 8 (closed by 4 regfix rules). The HONEST clean floor is now 15 (was 21).
- **24-variant sweep total** (14 prior session + 10 this session) documents the rejected lever space (see `meta.json.rejected_forms`). Best non-baseline: v15_subtract_const = 22 (regression). All structural/decl-order/ptr-alias/named-product perturbations converge to 21.
- The wall is sched.c:2399 `rank_for_schedule`: at the choice point after `sll cos_idx`, sched1 picks `lw vx` (class 3, indep of last_scheduled) over `lh cos_val` (class 1, data-dep on sll). This places lw before lh, prevents vx from reusing the freshly-dead $v1, and shifts the entire downstream allocation by one register position. Confirmed by direct read of sched.c:2399-2443 this session.
- `scan_hand_coded.py --single cpu_get_dist` → tier=LOW (`no strong hand-coded indicators`). Function is NOT canonical-asm material; matching C must exist.
- candidate.c is the honest pure-C baseline (score 21). Next session should resume from THIS, not from HEAD's cheated body.
- **2026-06-13 directed-permuter campaign (43k iters, 4 workers): found a permuter-score-0 byte-match but it is a CHEAT-FORM — REJECTED.** Closing constructs: `rz++; rz--;` dead no-op in the `if(v48<0)` block (rz liveness extension, zero semantic purpose), unused `s32 vx;`, asymmetric `new_var=(u8*)a0;` alias on only the 2nd store. Same intent as HEAD's original cheats, permuter-spelled. Clean forms plateaued ~permuter-310; sandbox floor unchanged at 21. **Takeaway: the target allocation IS reachable but every 0-form in the explored mutation space needs the rz-liveness coercion (no natural C use of rz exists in the v48 block). The clean lever is NOT in the permuter's statement-mutation space.** Untried clean levers remain: maspsx label-nop opt-out (low ROI — only matters if sched1 already produced target order, which it doesn't), re-seed permuter from a permuter-310 cleaner base. Permuter workspace persists at permuter/cpu_get_dist/ (output-0-1/ is the rejected cheat-form — do NOT apply).

## 2026-06-13 INSN_PRIORITY deep-dive — equal-priority root cause CONFIRMED (closes the operand-order lever class)

Read the verbose .sched dump (tmp/cgd_iso.c.sched). The two contended insns:
- insn 42 = `lw vx` (mem reg72+0x44): **priority = 2**, ref_count = 3, class 3 (independent).
- insn 59 = `lh cos_val` (sign_extend mem reg86+Judge): **priority = 2**, ref_count = 6, class 1 (data-dep on the cos-idx sll).
Both feed the SAME immediate successor insn 61 (`mult reg88 vx * reg97 cos`), so GCC 2.7.2 `priority()` assigns them EQUAL height (2). cos_val's extra deep successors (rx*cos / rz*cos in the stores) do NOT raise its computed priority — GCC's priority height is dominated by the shared nearest mult. With priorities tied, rank_for_schedule falls to the class check → lw vx (class 3) beats addu/lh cos (class 1) → vx scheduled first → v1 (cos-idx scratch) still live → vx spills off $v1 → cascade.
**Therefore the INSN_PRIORITY lever is structurally blocked:** to raise cos_val's priority above vx's, cos_val would need a deeper NEAREST successor than vx — impossible while both are the two inputs of the first rotation mult (symmetric). Operand-order sweep (tmp/cgd_v3_variants/: addend-swap, mult-operand cos-first, vx-last, combinations) all = 21 (v25 addend-swap = 24 regression). The [[compare-operand-order-register]] lever class is now EXHAUSTED for this function.
**Dead lever classes (do not re-try): decl-order, narrow-type, ptr-alias, operand/association-order, directed-permuter-statement-mutation.** The matching C exists ([[no-compiler-divergence]]) but is outside every structural dimension explored; next genuinely-new ideas would be a fundamentally different algebraic factoring of the rotation, or a re-seeded long permuter from a cleaner base.

## HEAD's two cheats — evidence

### 1. `do { ... } while (0);` wrapper (lines 2303–2315 in `src/code6cac_b.c` @ HEAD `fcbde9da`)

Wraps from `sin_val = ...` through the `if (v48 < 0)` block. Creates `NOTE_INSN_LOOP_BEG` between `vx = ...` and `sin_val = ...`, which per [[loop-exit-work-inside-loop-sched-fence]] acts as a sched.c fence (sched.c:2067-2095). This splits the post-call work into two basic blocks, constraining sched1's reordering window. Not in scope for [[do-while-zero-exception]] (no LABEL_OUTSIDE_LOOP_P / reorg.c interaction; no NE-invert peephole). Forbidden.

### 2. Dead `rx = ((vx * cos_val) + (vz * sin_val)) >> 12;` reading uninit `vz` (line 2297 inside the do-block)

Followed by `vz = a0[0x4C];` and then `rx = ((vx * cos_val) + (vz * sin_val)) >> 12;` again (the real one). The first `rx = ...` is dead (immediately overwritten) and uses `vz` before its proper assignment. The construct's ONLY effect is to extend `vz`'s pseudo live range upward, biasing greg's conflict graph so vz lands in `$v0` (matching target). Identical intent to forbidden `find_dead_param_assigns` / `find_dead_conditional_stores` patterns — different LHS (a local rx) and uses uninit-RHS rather than dead-store-RHS, but the cheat-by-spelling lens classifies it the same. Forbidden.

## RTL mechanism (sched.c:2399 rank_for_schedule)

In the post-call basic block, after sched1 schedules `sll $3, $3, 1` (cos_idx in $v1), the ready set includes:

| insn | class vs last(sll) | LUID |
|---|---|---|
| `lh cos_val, Judge($3)` | 1 (data-dep on sll's $3) | lower |
| `lw vx, 68($s0)` | 3 (independent) | higher |
| `andi sin_idx, $v0, 0xfff` | 3 (independent) | higher |

sched1's tie-break (sched.c:2435 `tmp_class - tmp2_class`) prefers higher class. lw vx and andi sin_idx are class 3, lh is class 1 → lh loses the class check regardless of LUID. Among the class-3 ties, lower LUID wins (sched.c:2442) — in source-order cos_val FIRST, lw vx has lower LUID than andi sin_idx → lw vx picked.

Cascade: lw vx commits to a register WHILE $v1 still holds the live cos_idx scratch. $v1 unavailable → vx → $a0. After the eventual lh cos_val, $v1 is dead — too late for vx, which is already in $a0. cos_val then takes the next available register ($a2 instead of $a1). Downstream pseudos shift: sin_val → $a1, vz → $v1, etc.

Confirmed via `-da` greg dump (`tmp/cgd_iso.c.greg`): pseudos 88 → $a0 (vx), 97 → $a2 (cos_val) for clean form; HEAD's body gives 88 → $v1, 97 → $a1.

## What HEAD's cheats DO to flip this

HEAD's `do {` between vx-load and sin_val splits the BB, isolating the lw-vs-lh choice inside the pre-loop BB. Combined with the dead `vz`-use forcing `vz` into the conflict graph EARLIER, greg lands the target's 4-out-of-5 critical-pseudo allocation (vx=$v1, cos_val=$a1, sin_val=$a0, vz=$v0) with only the cos_idx scratch misallocated (→$a0 vs target's $v1). The 4 regfix rules close that residual.

## Lever exhaustion — what was tried (negative, 24 forms total)

See `meta.json.rejected_forms` for the full sweep. Categories:
- **Declaration order**: K&R-top decls, vx-first, cos+sin first, early-loads, top-without-init — all 21
- **Type/precompute**: judge-pointer local, intermediate cross-products, pre-negated -vx (variants v8/v23), scratch idx locals, named partial products (v21 = 27 regression) — all 21 or worse
- **Addressing style**: array indexing (a0[17]), base-pointer alias ((u8*)a0 once, reused — v13/v20), explicit cos_addr ptr (v17), judge[] indexed (v19) — all 21
- **Constant math**: `(angle - 0xC00) & 0xFFF` math-equivalent to `(angle + 0x400) & 0xFFF` (v15 = 22 regression)
- **Scope wraps**: inner-block scope around v48 use (v16) — 21 (no LOOP_BEG generated; no sched fence)

The wall is NOT in any of these dimensions. It's the rank_for_schedule class preference, which no decl-order/naming/scope/ptr-alias change perturbs. The LOOP_BEG note from HEAD's `do { } while (0)` IS the surface that flips it — and that's forbidden.

## sched.c read confirms theory (this session)

Direct read of `tools/gcc-2.7.2/sched.c:2399-2443`. `rank_for_schedule` uses:
1. INSN_PRIORITY first (path length to BB end).
2. If priorities tied, class via `find_insn_list (tmp, LOG_LINKS(last_scheduled))`: `link==0 || insn_cost==1 → class 3`; data-dep `cost>1 → class 1`; anti/output-dep `cost>1 → class 2`. Higher class wins.
3. If class tied, LUID wins (stable sort).

At post-`addu $at` choice point, `lw vx`'s LOG_LINK to addu is empty (independent) → class 3. `lh cos_val` has data-dep on addu with `insn_cost > 1` (load latency from arithmetic) → class 1. lw wins regardless of LUID. Target's bytes show lh BEFORE lw → target's RA/sched1 produced a different choice. The only differences in target's RTL stream (vs ours) come from the upstream structural shape; HEAD's do-while creates a NOTE_INSN_LOOP_BEG that splits the BB, removing the competition. No legitimate C surface flips this.

## Next-session hypotheses (highest-ROI first)

1. **Directed permuter (`PERM_*` macros) from candidate.c base.** Manual sweeps covered statement-level mutations across 24 forms. The permuter — especially directed mode — may surface deeper structural mutations (function-level type/signature, expression restructurings, randomization tricks) the manual sweep missed. Setup: build `target.o` from `asm/funcs/cpu_get_dist.s` + `tools/decomp-permuter/prelude.inc` (drop `.set gp=64`), `base.o` from preprocessed candidate.c using the compile.sh shape in `permuter/dbe4/`.
2. **INSN_PRIORITY investigation.** Class-tie-break is downstream of priority. If priorities of lh and lw differ in any C form, the class decision is irrelevant. Try: nested-expression forms that change downstream fanout / chain depth of cos_val vs vx.
3. **Instrumented cc1 dumps** (`tmp/gccdbg/cc1` with `BB2_SCHED_DEBUG`) to confirm sched1's actual choice trace on candidate.c, not just our model of it. Build instructions in [[register-alloc-pure-c]] Step 0.
4. **Maspsx label-nop gate exploration.** Target has `lh $a1, %lo(Judge)($at)` / `lw $v1, 0x44($s0)` adjacent with no nop. Our build may interpose. Per [[maspsx-label-nop-gate]] this is a per-function opt-in surface.

## Related rules

- [[register-alloc-pure-c]] (read first — the lever playbook this exhausted)
- [[no-new-park-categories]] (cheats-by-spelling — why HEAD's do-while + uninit-vz read are forbidden)
- [[do-while-zero-exception]] (do-while(0) sanctioned ONLY for LABEL_OUTSIDE_LOOP_P / reorg.c — not in scope here)
- [[loop-exit-work-inside-loop-sched-fence]] (the LOOP_BEG-as-fence mechanism HEAD exploits)
- [[no-compiler-divergence]] (the toolchain is fixed; the variable is C structure — but sched1's class preference is not a C-tunable surface)


## s1 (grind recon, 2026-07-21)

- Baseline re-confirmed on main: canonical verdict C (distance 15); sandbox --disable all = **15** (build 63 vs target 62, 4 rules dropped). candidate.c 15-form reproduces exactly.
- diagnose: LARGE, 7 raw differing insns (raw-objdump metric; masked=15 — different metrics, do not cross-compare).
- **Duplicate lead is DEAD**: fresh find_duplicates.py re-run still pairs cpu_get_dist ~= func_8003032C (1.000), but asm/funcs/func_8003032C.s is a stale same-address twin of cpu_get_dist itself (0x8003032C, rename artifact; see named_syms.txt:660). No independent COMPLETED-C analog exists. Also cpu_get_dist_2 ~= func_800324D0 is the same artifact class.
- **m2c reference shape** (tmp/grind/cpu_get_dist/s1/m2c_out.c): natural rx-first form — rx = (vx*cos + vz*sin)>>12; rz = -(((-vx)*sin + vz*cos)>>12). I.e. the original source was almost certainly the NATURAL form, which our sched1 compiles to 21 (v1_clean). The 15-form is an algebraic permutation that scores closer under the mask but emits a different mult order.
- **Unmasked diff of the 15-form** (s1/diff_s1.txt): build 63 vs target 68 insns. Target keeps two nop;nop mult-latency pairs our scheduler fills; target mult order = vx*cos, vz*sin, (-vx)*sin, vz*cos with vz loaded LATE; our 15-form emits vz*cos FIRST and hoists lw vz(0x4C) ABOVE the cos Judge lui/addu/lh cluster (same class-3-over-class-1 rank_for_schedule hoist as the 21-form wall, with vz in place of vx).
- **Fresh 15-form RTL dumps banked** (s1/cgd_iso.c.sched/.greg/.lreg etc — prior forensics were all on the 21-form): contended loads all priority-2-tied — insn 42=lw vx(reg88), 56=lw vz(reg95), 59=lh cos(reg97, ref_count 6), 64=lh sin(reg101), 85=lw v48(reg114); downstream mult chains at priority 14/25. Iso file (s1/cgd_iso.c) reproduces sandbox codegen (validated by matching early lw-vz hoist).
- MEASURED KILL: rz-first with (-vx)*sin-lead (target/m2c addend order inside the rz-first statement) = **25** (build 61) — regression. The vz*cos lead is load-bearing for 15. Banked in rejected/rz-first-negvx-sin-lead-25.c.

- [s1] m2c of target (s1/m2c_out.c): original source shape is the NATURAL rx-first form (rx = (vx*cos + vz*sin)>>12; rz = -(((-vx)*sin + vz*cos)>>12)) — which our sched1 compiles to 21; the score-15 candidate is an algebraic permutation of it

- [s1] Unmasked diff (s1/diff_s1.txt): build 63 vs target 68 insns; target keeps nop;nop mult-latency pairs our scheduler fills; target mult order vx*cos, vz*sin, (-vx)*sin, vz*cos with vz loaded LATE; our 15-form emits vz*cos first and hoists lw vz above the cos Judge load cluster (same rank_for_schedule class-3 hoist as the 21-form wall, with vz substituted for vx)

- [s1] Fresh 15-form RTL dumps banked (prior forensics were 21-form only): contended loads all priority-2-tied (insn 42=lw vx, 56=lw vz, 59=lh cos ref6, 64=lh sin ref6, 85=lw v48); mult chains at priority 14/25; iso file validated against sandbox codegen

- [s1] diagnose: LARGE, 7 raw differing insns (raw metric; masked=15)

- [s1] Fresh find_duplicates.py run regenerated tmp/duplicates_leads.txt; both cpu_get_dist leads are same-address rename twins, not real analogs

- [s1] src/code6cac_b.c restored to HEAD at session end (leaving the clean body with the 4 stale regfix rules would break the oracle); candidate.c in memory/grind/cpu_get_dist/ remains the resume point
