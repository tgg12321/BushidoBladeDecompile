# Hypothesis ledger — cpu_side_move_dir_4

## [s2] Reversing block-local decl order (t0 before arg5) shifts qty priority / allocation.
- mechanism: GCC 2.7.2 local-alloc.c: pseudo birth follows RTL first-use, not C decl order; qty_compare fed by ref-count/span, not by declaration sequence. Twin marionation s2/s3 measured this INERT on the analog window.
- probe: Applied variant A_decl_reverse (swap 'arg5' and 't0' decls) on the h5 base; sandbox cpu_side_move_dir_4 --disable all.
- result: masked score 2 (baseline 2). Bytes identical to baseline; the qty table is unchanged.
- verdict: KILLED

## [s2] Narrowing arg5/t0 to u32 with matching u32 casts shifts qty priority via added zero-extension RTL or width-typed allocnos.
- mechanism: MIPS backend treats s32/u32 as identical hard-reg class; no zero-extend RTL is emitted for word-sized u32 loads/stores; local-alloc allocnos are register-class-driven, not sign-typed. Twin marionation classified this qty-INERT.
- probe: Applied variant B_u32_narrow (u32 arg5; u32 t0; u32 casts on load, arith, store) on the h5 base; sandbox --disable all.
- result: masked score 2 (baseline 2). Bytes identical; no qty change.
- verdict: KILLED

## [s2] Interior statement re-association keeping arg5-chain after t0-chain head loads (v0<<=2 hoisted early / interleaved between t0 stmts) re-times the arg5 stage load or shifts qty allocations.
- mechanism: sched.c LUID ordering + adjust_priority; a v0<<=2 placed at various luid positions could interact with the scheduler's ready-queue LAUNCH ordering; twin evidence says the ip basin is priority-driven / order-invariant.
- probe: Sweep C_v0shl_early (v0<<=2 immediately after v0=idx_1494[1]) and D_interleave (v0<<=2 between t0*=4 and t0=(s32)((u8*)tbl+t0)); sandbox --disable all each.
- result: Both masked 2 (baseline 2). Structural re-association within the arg5-after-t0 constraint is masked-invariant on csmd4's h5 base - the scheduler independently reaches the same insn ordering.
- verdict: KILLED

## [s2] Flipping the head-load order (v0 = idx_1494[1] BEFORE t0 = idx_1494[0]) could give arg5's qty an earlier birth luid and win the birth-order tie-break against t0.
- mechanism: local-alloc.c qty birth-order tiebreak when priorities tie (twin evidence: arg5 needs pri>=t0 AND birth-before-t0 OR strict pri win). Load-first-luid is birth luid.
- probe: Applied variant E_arg5_first (v0 = idx_1494[1]; t0 = idx_1494[0]; then interleaved staging) on the h5 base; sandbox --disable all.
- result: masked score 7 (baseline 2). REGRESSION of 5. The t0-first head load is load-bearing for the masked-2 alignment: flipping it re-times the entire window head (evidence: twin 2026-07-01 addendum on head-load-bearing luid, notes.md:47-48).
- verdict: KILLED

## [s3] A fresh single-set intermediate (`s32 v0s = v0<<2;`) between the shift and the arg5 load will change scheduling by giving arg5's index-chain a distinct allocno with its own birthing SET.
- mechanism: local-alloc.c allocnos are per-pseudo; a new pseudo participates in qty_compare + sched.c birthing_insn_p LAUNCH.
- probe: Applied V2_v0s_split; sandbox --disable all.
- result: masked 2 (baseline 2). Bytes identical — GCC folds v0s back / scheduler independently reaches the same ordering.
- verdict: KILLED

## [s3] A fresh intermediate for the t0*4 shifted value (`s32 t0s = t0*4;`) will preserve the h5 masked floor while altering RA priorities.
- mechanism: As above; fresh single-set pseudo t0s participates in qty birth order.
- probe: Applied V3_t0s_split; sandbox --disable all.
- result: masked 3 (baseline 2). +1 regression — the extra allocno tips qty priorities against t0.
- verdict: KILLED

## [s3] A named intermediate `arg5_addr = v0+(s32)tbl_125c` before the arg5 dereference changes the tree shape reaching expand/local-alloc.
- mechanism: expand.c generates the same RTL for `*(s32*)(a+b)` and `t=a+b; *(s32*)t;` at -O2 due to expression tree flattening.
- probe: Applied V4_arg5_addr; sandbox --disable all.
- result: masked 2 (baseline 2). INERT — expand-time flatten confirmed.
- verdict: KILLED

## [s3] Copying idx_1494 into a fresh local pointer (`u8 *p1494 = idx_1494;`) before accessing [0] and [1] introduces a distinct base pointer allocno.
- mechanism: cse.c copy-propagation may or may not fold the copy; if not, the fresh pseudo alters priorities.
- probe: Applied V5_ptr_split; sandbox --disable all.
- result: masked 2 (baseline 2). INERT — cse folds the pointer copy.
- verdict: KILLED

## [s3] Explicitly naming the arg4 dereference (`arg4 = *(s32*)t0;`) instead of inlining it in the call arg list alters arg4 chain scheduling.
- mechanism: Named local gains its own pseudo and load-vs-call scheduling seat.
- probe: Applied V6_arg4_named; sandbox --disable all.
- result: masked 11 (baseline 2). +9 regression — arg4 pseudo disrupts t0 multi-set launch-suppression and introduces a new load-before-call seat.
- verdict: KILLED

## [s3] Statement-form in-place spelling (`t0 <<= 2; t0 += (s32)tbl_125c;`) reveals a launch pattern hidden by the `t0 *= 4` synthesizer temp.
- mechanism: The mult path synthesizes a launching temp inside expand.c; two-statement <<=/+= keeps t0 as the SET destination throughout.
- probe: Applied V7_stmt_form_t0; sandbox --disable all.
- result: masked 7 (baseline 2). +5 regression — confirms twin notes.md:48 that <<=2 at same position on h5 = 7; the *=4 temp's launch is head-load-bearing for h5's alignment.
- verdict: KILLED

## [s3] Moving the `v0<<=2` shift to immediately after the v0 load (`v0<<=2` between v0=... and pp=...) re-times the arg5 index chain earlier.
- mechanism: sched.c ready-queue ordering follows insn LUID; moving the shift's C position could bump its LUID slot ahead of the pp assignment.
- probe: Applied V8_v0shl_early; sandbox --disable all.
- result: masked 2 (baseline 2). INERT — sched independently reaches the same ordering (consistent with ip-basin priority-driven finding).
- verdict: KILLED

## [s3] Removing the intermediate `v0<<=2` statement and inlining the shift into `arg5 = *(s32*)((v0<<2) + (s32)tbl_125c);` alters expand's RTL tree.
- mechanism: Whether GCC synthesizes an internal temp for (v0<<2) affects birthing_insn_p SET-dest lookup.
- probe: Applied V9_arg5_inline; sandbox --disable all.
- result: masked 2 (baseline 2). INERT — expand emits equivalent RTL either way.
- verdict: KILLED

## [s3] Compressing the t0 chain to a single statement (`t0 = (s32)((u8*)tbl_125c + idx_1494[0]*4);`) folds t0 to a single-set pseudo.
- mechanism: birthing_insn_p LAUNCHES single-set dests; multi-set t0 (t0=lbu; t0*=4; t0=(...+t0);) does NOT launch. Compression should re-launch.
- probe: Applied V10_t0_compress; sandbox --disable all.
- result: masked 9 (baseline 2). +7 regression — CONFIRMS the h5 base depends on the multi-set t0 pattern to suppress the sll/addu LAUNCH.
- verdict: KILLED

## [s3] Moving the pp assignment past both shifts (right before the call) alters its scheduling seat.
- mechanism: pp is a symbol-address stage; expand may keep its RTL position anchored to the C statement position via LUID.
- probe: Applied V11_pp_late; sandbox --disable all.
- result: masked 2 (baseline 2). INERT — pp's rescheduled position matches its C-early position at the emitter.
- verdict: KILLED

## [s3] Reversing block-local decl order to put pp first, then t0/arg5, changes decl-time RA seat.
- mechanism: Twin s2/s3: pseudo birth follows RTL first-use, not C decl order; this reconfirms on csmd4.
- probe: Applied V12_decl_pp_first; sandbox --disable all.
- result: masked 2 (baseline 2). INERT — reconfirms s2 A_decl_reverse finding.
- verdict: KILLED

## [s3] Pointer-deref form `*idx_1494` / `*(idx_1494+1)` instead of `[0]`/`[1]` may alter expand's index synthesis.
- mechanism: expand-time array-vs-pointer normalization; both should reduce to the same MEM (plus base 0/1).
- probe: Applied V13_ptr_deref_form; sandbox --disable all.
- result: masked 2 (baseline 2). INERT — expand normalizes both forms identically.
- verdict: KILLED

## [s4] A do-while(0) wrap around ONLY the arg5 v0-staging sub-block (v0=idx[1]; v0<<=2; arg5=*(...)) lifts arg5-qty loop-weighted refs via NOTE_INSN_LOOP_BEG so pri(arg5) > pri(t0), flipping the v1/a0 exchange.
- mechanism: flow.c REG_N_REFS x loop depth in local-alloc.c qty_compare; do-while(0) sanctioned 2026-07-06 as ANY-codegen-effect device.
- probe: Apply src/system.c edit wrapping ONLY {v0=idx_1494[1]; v0<<=2; arg5=*(s32*)(v0+(s32)tbl_125c);} in do{}while(0); leave t0/pp lines unwrapped. Sandbox --disable all.
- result: masked=20, target_insns=160, build_insns=162 (+2). The LOOP_BEG note materialised as 2 additional insns in the window, destroying the h5 alignment; the pri-lift did not materialise as an allocation flip.
- verdict: KILLED

## [s4] A do-while(0) wrap around the OPPOSITE chain (t0 side: t0=lbu; t0*=4; t0=(tbl+t0);) should either be inert (if wrap-effect is direction-specific) or regress symmetrically (if the wrap is a scheduler barrier here regardless of side).
- mechanism: Diagnostic pair for probe #1: if arg5-wrap and t0-wrap both add insns, the LOOP_BEG note is a scheduler barrier in this window, killing the frontier's assumed pri-lift-differential mechanism.
- probe: Apply src/system.c edit wrapping ONLY {t0=idx_1494[0]; t0*=4; t0=(...+t0);} in do{}while(0); leave arg5/pp/v0 unwrapped. Sandbox --disable all.
- result: masked=24, target_insns=160, build_insns=162 (+2). Same +2 insn signature as the arg5-side wrap. Direction-invariant.
- verdict: KILLED

## [s4] Threading the arg5 index-carrier through cnt (dead after do_timeout dispatch, live upstream in the fast-path condition) makes the arg5 carrier legitimately multi-set so cse.c copy-fold single-set precondition fails; qty tables shift to give arg5 pri>=t0.
- mechanism: cse.c copy-propagation folds single-set reg sources; a multi-set carrier defeats it. local-alloc.c qty birth-order and refs feed the differential.
- probe: Replace {v0=idx_1494[1]; v0<<=2; arg5=*(s32*)(v0+(s32)tbl_125c);} with {cnt=idx_1494[1]; cnt<<=2; arg5=*(s32*)(cnt+(s32)tbl_125c);}. Sandbox --disable all.
- result: masked=4, target_insns=160, build_insns=160. No insn-count change; the alloc web shifts but pushes t0 further into the target seat than baseline (masked +2). Confirms cnt is not a viable value-carrier for arg5's index chain.
- verdict: KILLED

## [s4] An opaque volatile file-scope handle (`static void *volatile fmt_hold = &D_800161C8;`) hides the fmt symbol from cse.c equiv_constant so lui/addiu selection lands after t0's death, flipping the residual v1/a0 exchange.
- mechanism: equiv_constant folds addressable symbols regardless of C staging; a volatile-typed handle blocks the fold — but this is exactly the 'volatile coercion of a non-IRQ non-MMIO game-state global' pattern in the expanded cheat catalog.
- probe: Policy vetting BEFORE measurement per no-new-park-categories: (a) &D_800161C8 is a debug-format string in game rodata, neither IRQ-mutated nor in the 0x1F801000-0x1F802FFF MMIO range; the legitimate-volatile-interrupt-touched two-prong test fails on prong 1; mmio-volatile-type-level does not apply. (b) The construct has no semantic purpose beyond defeating GCC equiv_constant — GCC-internals-justification test fails.
- result: Rejected without measurement by policy [[inline-asm-policy]] expanded catalog + [[no-new-park-categories]] cheat-by-any-spelling. No non-volatile alternative can defeat equiv_constant.
- verdict: KILLED

## [s5] Block-scope multi-set carrier `s32 c` with first-set = *idx_1495 (live-elsewhere) and second-set = idx_1494[1] lifts arg5-qty refs above t0-qty by defeating combine.c pair-merge on non-equal-value sets.
- mechanism: combine.c merges paired sets only when transformable to equality; a first-set from a distinct live symbol (*idx_1495) should be non-mergeable, preserving 2 refs on carrier c and lifting qty vs. fn-scope v0 staging (refs=2 sink-defeated).
- probe: src/system.c inline block: replaced v0-staging with `s32 c; c = *idx_1495; c = idx_1494[1]; c <<= 2; arg5 = *(s32*)(c + (s32)tbl_125c);` while keeping t0 chain + pp alias untouched; sandbox cpu_side_move_dir_4 --disable all.
- result: masked=2, target_insns=160, build_insns=160. INERT vs h5 baseline — GCC folds the dead first-set to identical RTL as v0 staging OR the resulting ref-lift does not exceed the arg5>t0 strict-win threshold.
- verdict: KILLED

## [s5] Block-scope multi-set carrier `s32 c` with first-set = t0 (a live local, non-mergeable per combine.c operand-tracking) provides a distinct multi-set carrier vehicle.
- mechanism: Same as above but seeded from t0 to guarantee a live-value seed rather than a symbol-load (rules out any expand-time symbol equiv_constant fold of the first set).
- probe: src/system.c inline block: `c = t0; c = idx_1494[1]; ...`; sandbox --disable all.
- result: masked=2, target_insns=160, build_insns=160. INERT — confirms F1a diagnosis: the block-scope multi-set carrier vehicle is masked-invariant regardless of first-set source symbol.
- verdict: KILLED

## [s5] Naming D_800A11DC[D_800A11D5] to a block-scope `dispatch` local BEFORE the arg5 stage re-seats the arg-register homing LUID and re-times the fmt-la relative to t0's death.
- mechanism: expand.c evaluates CALL args right-to-left; a named intermediate for arg3 acquires its own LUID slot and may re-order the scheduler ready queue at arg5 birth.
- probe: src/system.c inline block: `s32 dispatch; dispatch = D_800A11DC[D_800A11D5];` inserted between the pp assignment and the t0 chain; call reads `dispatch`.
- result: masked=16, target_insns=160, build_insns=160. +14 regression — the dispatch pseudo re-competes with t0/arg5 for the a-register seat, destroying the multi-set t0 launch-suppression alignment.
- verdict: KILLED

## [s5] Same-form probe (F2b diagnostic): the dispatch pseudo's re-seat effect is position-invariant (placing it AFTER arg5 stage gives identical regression).
- mechanism: If the regression is qty-priority driven (not LUID-driven), position within the block should not change it.
- probe: src/system.c inline block: `dispatch = D_800A11DC[D_800A11D5];` inserted between arg5 assignment and the call.
- result: masked=16, target_insns=160, build_insns=160. SAME regression as F2a — CONFIRMS the named-arg3 effect is qty-competition, not LUID-scheduling; naming-anywhere-in-block kills h5 alignment.
- verdict: KILLED

## [s5] A single do-while(0) wrap around the ENTIRE inline block (both idx_1494 derefs, t0 chain, arg5 chain, pp, and the CALL inside the wrap) flips arg5>t0 via depth-1 REG_N_REFS multiplier on both idx loads.
- mechanism: flow.c multiplies REG_N_REFS by loop depth; a wrap enclosing both idx derefs symmetrically lifts refs for arg5 AND t0 index sources; the differential shift may be net positive for arg5-qty.
- probe: src/system.c: wrapped the whole `t0=idx[0]; v0=idx[1]; pp=...; t0*=4; ...; call(...)` block in do{}while(0).
- result: masked=13, target_insns=160, build_insns=160. Notably NO +2-insn barrier signature (contrast s4 sub-block wraps which added +2 insns) — the whole-block wrap does NOT act as a scheduler barrier here; the ref-lift materialized but MISDIRECTS allocation (both index sources lifted symmetrically, differential moves further from target).
- verdict: KILLED

## [s5] Nested do-while(0) wraps around the same block (depth 2) increase the REG_N_REFS multiplier further and may flip the arg5>t0 equation past the strict-win threshold.
- mechanism: Depth^2 accumulation via flow.c REG_N_REFS multiplier compounding on nested inner+outer loop notes; if F3's +11 was a monotonic near-flip, F3b could over-shoot into arg5-win territory.
- probe: src/system.c: outer do{ inner do{ ... }while(0); }while(0); around the same whole inline block.
- result: masked=17, target_insns=160, build_insns=160. DEEPER wrap = FURTHER from target (+4 vs F3, +15 vs baseline). The wrap-based ref-reweighting is monotonically wrong-direction; there is no depth at which it flips.
- verdict: KILLED

## [s5] A do-while(0) wrap around ONLY the CALL statement (leaving head-load chain outside) lifts refs only on the CALL's arg-load sequence (fmt-la + D_800A11DC[D_800A11D5]) without disturbing t0's multi-set launch suppression.
- mechanism: Isolating the wrap to the CALL keeps t0's setup insns outside the loop-note ref-multiplier, hoping to lift only the arg-marshal side.
- probe: src/system.c: `do { debug_printf(...); } while(0);` with head-load chain outside the wrap.
- result: masked=8, target_insns=160, build_insns=160. +6 regression. Improvement over whole-block wrap (13) and nested (17) — smaller wrap scope = smaller misdirection, but still wrong direction. Confirms wraps are net-negative for csmd4's h5 base at every scope tested.
- verdict: KILLED

## [s5] Directed permuter on the g3 base (weighted score 40; richer mutation neighborhood than h5) will discover a novel closing form in the fresh-seed window.
- mechanism: Standalone permuter random mutation over the g3 in-place-basin's schedule-perfect + register-half-open form; each iteration explores a distinct AST mutation with base_score 40.
- probe: tools/permuter_campaign.py launch --func cpu_side_move_dir_4 --dir tmp/perm_csmd4 --label s5_g3_fresh -j 8 --stop-on-zero; harvested after fresh-seed window with 0 novel finds required as stopping criterion.
- result: 9040 iterations / 411.8s / 0 NEW finds. Prior 2 output-40 basin unchanged. 0-find harvest is the data point per fresh-seed discipline. Weighted score-40 base did not yield a novel mutation neighborhood; the g3 basin's local minimum is stable under random mutation.
- verdict: KILLED

## [s6] The masked-2 residual on the h5 base is produced by sched.c::adjust_priority marking RTL insn 111 (p106 = p101 << 2, the SLL emitted when GCC's mult-by-power-of-2 expander creates a fresh single-set dest for `t0 *= 4`) as LAUNCH (pri=0x7f000001), tying with insn 121 (p107 = p75 + p79, arg5 addr) which is also LAUNCH single-set. The backward-scheduler LUID tiebreak (higher LUID picked first = later emission) picks 121 (LUID 12) before 111 (LUID 8), so 111 emits between 118 and 121 in the pair {sll v0; sll a0; addu v0} instead of target's {sll v0; addu v0; sll a0}.
- mechanism: sched.c::adjust_priority + birthing_insn_p: LAUNCH sentinel 0x7f000001 assigned iff SET dest is a live pseudo with reg_n_sets==1 (flow-time). p106 is a fresh temp synthesized inside expand for `t0 *= 4` (mult-by-power-of-2 lowered to SLL to a new pseudo), so reg_n_sets(p106)==1 -> LAUNCH. p101 (the t0 var itself) is multi-set (lbu at insn 100, plus at insn 116) so insn 116 is NON-LAUNCH; but insn 111's dest is the fresh p106, not p101, so it launches. Priority tie with 121 -> LUID 12 wins -> pair swap.
- probe: Instrumented cc1 dump under tmp/grind/cpu_side_move_dir_4/s6/ with BB2_QTY_DEBUG/BB2_SCHED_DEBUG/BB2_ALLOC_DEBUG/BB2_RANK_DEBUG=1 -da on the h5 candidate applied to src/system.c. Read SCHEDDBG block=3 sched2 pass, matched insn IDs to lreg RTL, matched RTL insn IDs to target asm.
- result: SCHEDDBG shows insn 111 pri=2130706433 (LAUNCH) and insn 121 pri=2130706433 (LAUNCH), simultaneously ready at clock=13 with 121(luid=12) picked first, 111(luid=8) picked at clock=14, 118 picked at clock=15. Emission order confirmed as 118,111,121 vs target 118,121,111 in the raw csmd4.s output. Register dispositions confirm 100 in 3, 106 in 4, 113 in 2, 107 in 2 (register half of the h5 base is correct; only the pair-swap is residual).
- verdict: CONFIRMED

## [s6] Raising insn 111's LUID above 121's by moving `t0 *= 4` after the arg5 chain in C source order would give 111 the LUID tiebreak win and flip the pair.
- mechanism: LUID is assigned by RTL emission order which follows C statement order at expand-time; delaying the `t0 *= 4` statement past `arg5 = *(s32*)(v0+tbl)` should push insn-111-equivalent's LUID above insn-121's.
- probe: Applied `t0=idx[0]; v0=idx[1]; pp=...; v0<<=2; arg5=*(v0+tbl); t0*=4; t0=(u8*)tbl+t0; call(...,*(s32*)t0,arg5);` and sandboxed. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/luid_reorder_t0_after_arg5.c.
- result: masked=6, target_insns=160, build_insns=160. Regression of +4 into the g3 basin. Reordering the t0 chain past arg5 also shifts p101 and p75's qty priorities and the head-load LAUNCH signature, so the base flips h5->g3. In g3 the pair {sll v0, addu v0, sll a0} is order-perfect but the v1/a0 register exchange is unresolved. The pair swap is not independently controllable by C-statement-order alone; the two chains' LAUNCH signatures are coupled through the qty priority arithmetic.
- verdict: KILLED

## [s7] Combine.c is REJECTING the merge of insn 111 (set p106 (ashift p101 2)) with insn 116 (set p101 (plus p106 p79)) due to a num_sets>1 test on p101 or a set-validity check; a C-source lever that changes p101's set count could unstick the LAUNCH.
- mechanism: combine.c try_combine(i2=111, i3=116) with p106 dying at i3 (single-use). added_sets_2 = !dead_or_set_p(i3, i2dest=p106) evaluates FALSE (REG_DEAD p106 note satisfies dead_or_set_p) so combine attempts pure substitution to produce newpat = (set p101 (plus (ashift p101 2) p79)). If merged, p106 vanishes and p101 gains one more SET.
- probe: Static walkthrough of tools/gcc-2.7.2/combine.c:1385-1728 try_combine → recog_for_combine (line 1691) invoked on newpat; grep of tools/gcc-2.7.2/config/mips/mips.md for shift+plus patterns (only addsi3_internal / maddi / umaddi found — none is (plus (ashift reg const) reg)).
- result: recog_for_combine returns -1 because no MIPS insn pattern matches (plus (ashift reg const_int) reg). find_split_point cannot help (no non-trivial split). undo_all runs; p106 survives. The combine reject is due to MIPS md pattern absence, NOT any p101 num_sets/set-validity test — the proposed C-source lever is inoperative.
- verdict: KILLED

## [s7] The h5-basin LAUNCH on insn 111 originates upstream of combine at RTL expand time — specifically expmed.c::expand_mult case alg_shift hardcodes the shift's target to NULL_RTX, birthing a fresh single-set pseudo p106 for `t0 * 4` even though the outer SET's LHS is the l-value t0.
- mechanism: expmed.c line 2244: `expand_shift (LSHIFT_EXPR, mode, accum, build_int_2(log,0), NULL_RTX, 0)`. NULL_RTX target propagates into expand_shift→expand_binop→gen_reg_rtx(mode). REG_EQUAL note on the emitted (set p106 (ashift p101 2)) records the (mult p101 4) semantics. Contrast: `t0 <<= 2` (direct LSHIFT_EXPR on l-value) reaches expand_binop with target = t0's pseudo, emits (set p101 (ashift p101 2)) in place, making p101 multi-set → birthing_insn_p FALSE → no LAUNCH.
- probe: Read tools/gcc-2.7.2/expmed.c:2136-2260 (expand_mult body); verified case alg_shift's target arg is hardcoded NULL_RTX (not the alg_add_t_m2 add_target=... conditional above). Cross-referenced tmp/grind/cpu_side_move_dir_4/s6/csmd4_only.combine (insn 111 has REG_EQUAL (mult p101 4) — the honest trace of the mult-expansion origin).
- result: CONFIRMED: expand_mult case alg_shift is the named pass that births p106; the h5-basin's pair-swap residual is inherent to any C form that expresses × 4 as a MULT (routed through expand_mult) rather than a direct LSHIFT (routed through expand_binop with the outer SET's target).
- verdict: CONFIRMED

## [s7] The h5 and g3 basins are non-composable across every C spelling of the t0 chain: h5's masked-2 (pair swap) requires the mult-expander path (p106 fresh, LAUNCH), and g3's masked-6 (register exchange) requires the direct-LSHIFT path (p101 in-place, non-LAUNCH). No C spelling reaches expand_binop with a non-null target while also keeping the p101 pseudo single-set.
- mechanism: Any C form of × 4 reaches expand_mult (t0=t0*4, t0*=4, sum of copies, chained shifts). expand_mult only threads its `target` through the FINAL alg step and only for non-shift ops (add_target guard `variant != add_variant`). case alg_shift ignores add_target entirely (hardcoded NULL_RTX). Only a direct C `<<=` bypasses expand_mult and reaches expand_binop with target = outer SET's LHS.
- probe: Cross-checked with s3-V7 (statement-form t0 <<= 2 → masked 7 = g3 basin), s6 LUID-reorder (t0 *= 4 moved late → masked 6 = g3 basin). Both confirm the basin flip is purely a function of whether the SLL's dest is a fresh pseudo (h5) or an in-place re-SET of p101 (g3).
- result: CONFIRMED via prior measurements + this session's static expmed.c walkthrough.
- verdict: CONFIRMED

## [s8] Replacing the cross-symbol tbl_125c-routed spelling of idx_1495 (line 19 of candidate.c) with the honest pointer-arithmetic form idx_1495 = idx_1494 + 1 (as directly evidenced by the prologue asm `addiu $s4, $s2, 0x1` at 715F4) is basin-neutral or improves the h5 masked-2 floor.
- mechanism: The prologue asm derives s4=s2+1 in ONE addiu, so the original C computed idx_1495 by direct pointer arithmetic on idx_1494. The current committed cross-symbol form (u8*)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1) is in the semantic-lie family the 2026-07-05 do-while-zero-exception.md #5 ruling forbids. WIP notes.md warned of file-level s-reg ref-balance coupling for this substitution but the h5 masked measurement was never taken.
- probe: Applied replacement in-place on the h5 candidate; sandbox cpu_side_move_dir_4 --disable all.
- result: masked=15, target_insns=160, build_insns=160 (+13 vs h5 baseline of 2). Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/honest_idx_1495_ptrarith.c and tmp/grind/cpu_side_move_dir_4/s8/system_probe1.c. Empirically CONFIRMS the WIP file-level s-reg coupling warning: the cross-symbol spelling is currently LOAD-BEARING at the s-reg allocation web level for h5 basin. Any pure-C closure must find a way to satisfy the prologue's `addiu s4,s2,1` shape WITHOUT breaking the s-reg web h5 depends on.
- verdict: KILLED

## [s8] Fresh m2c decompile of asm/funcs/cpu_side_move_dir_4.s produces a structurally novel C shape not measured in prior sessions.
- mechanism: m2c reconstructs original-compiler-shaped C from RTL-recovered dataflow; a rederive from raw asm bypasses the search paths biased by the h5-basin lineage.
- probe: python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c --function cpu_side_move_dir_4 asm/funcs/cpu_side_move_dir_4.s; output saved to tmp/grind/cpu_side_move_dir_4/s8/m2c_out.txt.
- result: m2c emits the direct-inlined-all-args shape for the debug_printf call: debug_printf(&D_800161C8, D_800F19C0, *(&D_800A11DC + (D_800A11D5 * 4)), *((M2C_FIELD(&D_800A1494, u8*, 0) * 4) + &D_800A125C), *((M2C_FIELD(&D_800A1494, u8*, 1) * 4) + &D_800A125C)) - NO local variable staging for arg4/arg5 or the index bases. This matches the WIP-recorded 'inline-all args (v1/v8/v9): score 14' shape (evidence.md L7). m2c also flattens the loop/do_timeout goto skeleton into if/else; the flattening axis is upstream of the residual pair-swap window at L80080E64+ and doesn't touch the mult-expander LAUNCH mechanism. The rederive angle does NOT surface a novel structural neighborhood for the h5/g3 residual.
- verdict: KILLED

## [s9] The marionation_Exec inline-block shape (arg4 named via tbl_125c[idx_1494[0]] array-index syntax, arg5 fully inline in the debug_printf call) transplants onto csmd4 and lowers or maintains the h5 masked-2 floor.
- mechanism: marionation_Exec is the file-local twin and its inline-block form has never been measured on csmd4; the array-index syntax lowers to different expand-time RTL than the pointer-arithmetic *(s32*)(v0+(s32)tbl_125c) form on which h5's masked-2 basin depends. A sibling-transplant is the modality-mandated rederive angle after m2c was closed in s8.
- probe: Applied P1 (arg4 named + arg5 inline) on top of the h5 candidate applied to src/system.c; sandbox cpu_side_move_dir_4 --disable all.
- result: masked 7, target_insns=160, build_insns=160. Matches the ip_base_statement_form.c basin (g3 family, direct-LSHIFT / in-place, p101 multi-set, non-LAUNCH).
- verdict: KILLED

## [s9] Mirroring the marionation-hybrid (arg5 named, arg4 inlined at call) shifts the qty birth order and re-orders the pair.
- mechanism: Right-to-left CALL arg evaluation would evaluate arg4 (last) into a very late-birthed pseudo, possibly delaying its LAUNCH past arg5's stage.
- probe: Applied P2 (arg5 named + arg4 inline) on top of h5 base; sandbox --disable all.
- result: masked 14, target_insns=160, build_insns=160. Matches the WIP-recorded 'inline-all args' basin (evidence.md L7).
- verdict: KILLED

## [s9] Both arg4 and arg5 named via array-index syntax with reverse eval order (arg5 first) lands in a distinct novel basin.
- mechanism: Two distinct named pseudos with the array-index lowering; reverse-order eval assigns arg5 an earlier LUID than arg4.
- probe: Applied P3 (both named, arg5 first) on top of h5 base; sandbox --disable all.
- result: masked 8, target_insns=160, build_insns=160. Novel intermediate basin between ip-basin (7) and inline-all (14) - not previously measured but strictly worse than h5.
- verdict: KILLED

## [s9] Full marionation basin transplant (honest idx_1495 = idx_1494 + 1 + marionation-hybrid inline block) composes the two levers to yield a novel basin.
- mechanism: The honest idx_1495 respelling and the marionation-hybrid inline block are both LOAD-BEARING at marionation's s-reg web (idx_1494/idx_1495/idx_1496 triad + register pin + alias rename); transplanting both together might respect the coupling.
- probe: Applied P4 (honest idx_1495 + marionation-hybrid inline block) on top of h5 base; sandbox --disable all.
- result: masked 20, target_insns=160, build_insns=160. WORSE than either lever in isolation (+5 P1 + +13 s8-probe1 -> +18 non-linear compound at s-reg-web level).
- verdict: KILLED

## [s10] Re-reading the full ledger, only lever (b) 'insn 121 loses LAUNCH while insn 111 keeps LAUNCH' can flip the pair without falling out of the h5 basin.
- mechanism: sched.c::adjust_priority + birthing_insn_p gate LAUNCH on flow-time reg_n_sets==1 of the SET dest. Lever (a) attacking 111 collapses to g3 basin per s7's expmed.c case alg_shift finding. Lever (c) LUID reorder is coupled to qty priorities (s6 KILLED). Only (b) has an unexplored realization: make p107 (arg5_addr, insn 121's dest) multi-set at flow-time while p106 (insn 111's dest) stays single-set.
- probe: Ledger cross-read only (synthesis modality). Verdict is a logical narrowing, not a fresh measurement.
- result: The frontier's #2-variant sketch (two-independent-PLUS-SETs on arg5_addr) is the only surviving mechanism-hit; #3 duplicated-statement-into-arms on the t0 chain is retired because it attacks the WRONG side (would double p101 refs, regressing to g3 basin). Cross-jump for arg5-chain is infeasible because v0=idx_1494[1] is set inside the do_timeout block, so duplication across the two do_timeout arrival arms would require pre-set v0 outside the block, changing bytes materially.
- verdict: CONFIRMED

## [s10] The M1 algebraic-cancellation two-SET form `arg5_addr = v0 + t0; arg5_addr += (s32)tbl_125c - t0;` is the highest-mechanism-precision untested probe.
- mechanism: If cse.c::simplify_plus_minus does NOT fold +t0 with -t0 across the two SETs, flow.c sees reg_n_sets(p107)=2, sched.c::birthing_insn_p returns FALSE on insn 121, 121 loses LAUNCH, 111 keeps LAUNCH via the mult-expander p106 fresh-dest path, strict-priority tiebreak: 111 picked first at clock 13 (backward), emitted last in linear order → 118,121,111 = target.
- probe: Not measured this session (synthesis modality mandates writing the merged attack + resetting the frontier, not new probes). Draft form retained in tmp/grind/cpu_side_move_dir_4/s10/synthesis.md for s11.
- result: Elevated to frontier #1 for next session. Risk: simplify_plus_minus is aggressive about +x/-x term cancellation; the fallback is M1-secondary opaque-carrier variant.
- verdict: CONFIRMED

## [s10] Frontier #3 duplicated-statement-into-arms on the t0 chain (previously listed as live) attacks the wrong side of the pair.
- mechanism: Duplicating the t0 chain would inflate flow.c reg_n_refs on p106/p101 AND (per rule) increase reg_n_sets on the duplicated dest. Since 111 must KEEP LAUNCH to survive in the h5 basin, doubling p106's sets makes 111 non-LAUNCH → g3 basin regression (identical outcome to every h5→g3 flip measured s3/s5/s6/s9). The rule's byte-neutrality via cross-jump is intact but pushes the exact wrong side.
- probe: Ledger cross-check: s6 CONFIRMED lever (a) is the g3-basin trap; #3 mechanism as previously stated realizes lever (a). No measurement needed to demote.
- result: Frontier #3 demoted to the rejected reasoning bank (not a rejected form; a rejected LEVER DIRECTION). Retained as a warning line in synthesis.md.
- verdict: KILLED

## [s11] arg5_addr = v0 + t0; arg5_addr += (s32)tbl_125c - t0; produces two SETs of p107 with algebraic cancellation of the t0 term; combine.c cannot substitute cleanly across the two SETs so p107 survives to flow with reg_n_sets=2; sched.c birthing_insn_p returns FALSE on insn 121; 121 loses LAUNCH; 111 keeps LAUNCH via mult-expander p106 fresh dest; strict-priority tiebreak picks 111 first at clock 13 -> pair flips to target order 118,121,111.
- mechanism: sched.c::adjust_priority + birthing_insn_p gate LAUNCH on flow-time reg_n_sets==1 of SET dest. If two SETs of arg5_addr survive to flow-time (either combine.c fails to substitute due to the -t0 subtrahend, or combine.c substitutes but leaves an intermediate pseudo), p107 reg_n_sets=2 and insn 121's LAUNCH is suppressed.
- probe: Applied arg5_addr = v0 + t0; arg5_addr += (s32)tbl_125c - t0; on h5 base at src/system.c:434-439. sandbox cpu_side_move_dir_4 --disable all.
- result: masked=11 (+9 vs baseline 2), target_insns=160, build_insns=160. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/arg5_addr_algebraic_cancel_two_set.c. The +9 regression indicates combine.c either substituted through both SETs (canonicalizing the arg5 chain differently than h5's staged form), or an intermediate pseudo formed by the substitution disturbed the alloc web enough to fall out of the h5 basin. Either way the frontier mechanism (flow-time reg_n_sets(p107)=2) is not preserved in the emitted code.
- verdict: KILLED

## [s11] M1-secondary: opaque-carrier t_alias = idx_1494[0] * 4 provides a fresh pseudo whose SET_SRC (mult (lbu idx_1494) 4) may or may not unify with t0's (mult (lbu idx_1494) 4) at cse.c value-numbering, testing cse granularity; if distinct, p107 two-SET survives combine.
- mechanism: cse.c value-numbers by RTL SET_SRC canonical form. t_alias and t0 both reduce to (mult (lbu idx_1494) 4) via expand's mult-expander; whether cse folds them determines whether reg_n_sets on p107 can be 2.
- probe: Applied t_alias = idx_1494[0] * 4; arg5_addr = v0 + t_alias; arg5_addr += (s32)tbl_125c - t_alias; on h5 base. sandbox --disable all.
- result: masked=29 (+27 vs baseline 2), target_insns=160, build_insns=160. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/arg5_addr_opaque_carrier_two_set.c. cse did NOT unify t_alias with t0 (the massive +27 regression indicates the duplicated (lbu; sll) sequence is emitted), but the resulting duplicated chain is far outside the h5 basin — the extra lbu-and-shift disrupts the entire arg5-chain LAUNCH signature.
- verdict: KILLED

## [s11] M1-tertiary (mechanism isolation): simple two-SET arg5_addr = tbl_125c; arg5_addr += v0; tests whether ANY two-statement decomposition of the arg5 address survives combine.c substitution; if bytes are IDENTICAL to h5 baseline, combine fully folds and p107 stays single-SET.
- mechanism: combine.c::try_combine substitutes (set p107 (plus p107 v0)) with previous (set p107 tbl) if the MIPS md pattern recognizes (plus symref reg) — addsi3_internal does (s7:213). Successful substitution yields single-SET p107 → 121 LAUNCH preserved → pair unchanged.
- probe: Applied arg5_addr = (s32)tbl_125c; arg5_addr += v0; arg5 = *(s32*)arg5_addr; on h5 base. sandbox --disable all.
- result: masked=2 INERT, target_insns=160, build_insns=160. Bytes byte-identical to h5. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/arg5_addr_simple_two_set_inert.c. CONFIRMS combine.c substitutes cleanly through any C two-SET on p107 that lacks a non-trivially-cancellable operand — the fold reduces to single-SET RTL identical to h5's inline form. Together with M1-primary (+9 regression, non-trivial subtrahend disturbs alloc web) and M1-secondary (+27 regression, cse keeps t_alias distinct but duplicates the lbu-mult chain), the frontier's mechanism realization (flow-time reg_n_sets(p107)=2) is not reachable via any C two-SET decomposition of arg5_addr.
- verdict: KILLED

## [s12] A pointer-typed intermediate `s32 *ap = (s32*)tbl_125c; ap = (s32*)((s32)ap + v0); arg5 = *ap;` escapes combine.c's addsi3_internal substitution because the intermediate carries pointer type distinct from s32.
- mechanism: RTL is type-agnostic for address arithmetic (all SImode); combine.c operates on (plus symref reg) modes, not tree-level type qualifiers. If pointer typing DID gate combine substitution, p107 could survive with reg_n_sets=2, killing insn 121's LAUNCH and flipping the pair.
- probe: Applied on h5 base at src/system.c: `s32 *ap = (s32 *)(s32)tbl_125c; ap = (s32 *)((s32)ap + v0); arg5 = *ap;` replacing `arg5 = *(s32*)(v0 + (s32)tbl_125c);`. Sandbox cpu_side_move_dir_4 --disable all. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/arg5_addr_pointer_walker_inert.c.
- result: masked=2 INERT, target_insns=160, build_insns=160. Bytes identical to h5 baseline. Empirically confirms combine.c substitutes through pointer-typed intermediates exactly as through s32 intermediates; the RTL substitution finds addsi3_internal for the (plus symref reg) shape regardless of the source-language type. p107 stays single-SET at flow-time.
- verdict: KILLED

## [s12] A manual liveness split `s32 a5 = v0 + tbl; s32 sink = a5; (void)sink; arg5 = *a5;` creates a live use of a5 that dies BEFORE the MEM read, altering the LAUNCH candidate list at sched2 so p107's SET is dead by insn 121.
- mechanism: The frontier note anticipated GCC might DCE the sink; the probe tests whether the (void)sink form materializes as a live use at flow-time. If sink survives DCE, a5's reg_n_sets and live range shift, potentially removing insn 121 from the LAUNCH ready-queue at clock 13.
- probe: Applied on h5 base: `s32 a5 = v0 + (s32)tbl_125c; s32 sink = a5; (void)sink; arg5 = *(s32*)a5;` replacing `arg5 = *(s32*)(v0 + (s32)tbl_125c);`. Sandbox --disable all. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/arg5_addr_dead_sink_liveness_split.c.
- result: masked=2 INERT, target_insns=160, build_insns=160. Bytes identical to h5 baseline. GCC's tree-level DCE eliminates sink and its assignment (void-cast discarded-value expression has no observable use); after DCE the residual is the s11 M1-tertiary simple two-SET which combine.c substitutes through addsi3_internal back to h5 RTL. The liveness-split mechanism is not reachable via any DCE-eliminatable use; non-DCE-eliminatable uses either materialize new insns (byte regression) or fall under the dead-store cheat family.
- verdict: KILLED

## [s13] Moving `D_800F19C0 = &D_80016240;` from prologue (src/system.c:408) into the do_timeout block just before the debug_printf inline block converts its store to intra-block SET; if the s-reg web absorbs the shift, it could provide a fn-body-scope ref-lift for pp; if not, the loop repeats the store per-iteration causing byte regression.
- mechanism: The store's C statement position influences LUID at expand-time; moving into do_timeout would give p_D_800F19C0's SET a distinct RA seat inside the block-local qty arithmetic. WIP notes flagged file-level s-reg ref-balance coupling as high-risk for prologue-init moves.
- probe: Removed the prologue line at (was) src/system.c:408 and inserted `D_800F19C0 = &D_80016240;` immediately after the tslTm2LoadImage_2 call in the csmd4 do_timeout block. sandbox cpu_side_move_dir_4 --disable all.
- result: masked=15 (+13), target_insns=160, build_insns=160. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/dhoist_D_800F19C0_do_timeout.c. The intra-block store is emitted per-iteration; the extra sw insn and shifted qty priorities disrupt the s-reg web, empirically CONFIRMING the WIP file-level s-reg coupling warning for this hoist direction. Frontier live #1 is now measured KILLED.
- verdict: KILLED

## [s13] On g3 base (v0<<=2; arg5=*(s32*)(v0+tbl); t0<<=2; ... *(s32*)(t0+tbl_125c) in-call), a block-scope multi-set VALUE carrier `s32 v_arg5; v_arg5=*idx_1495; v_arg5=*(s32*)(v0+tbl);` lifts arg5-qty pri>=5000 via first-set from live-elsewhere symbol so combine.c cannot merge, flipping the v1/a0 exchange to close g3 to 0.
- mechanism: sched.c birthing_insn_p returns FALSE on multi-set SET dest; local-alloc.c qty birth-order + refs feed the differential. In g3 the pair {sll v0; addu v0; sll a0} is order-perfect; only the v1/a0 exchange remains — a block-scope multi-set VALUE carrier for arg5 was measured INERT on h5 (s5 F1a/F1b) but never on g3.
- probe: Applied `s32 v_arg5; ... v_arg5 = *idx_1495; v0<<=2; v_arg5 = *(s32*)(v0+(s32)tbl_125c); t0<<=2; debug_printf(..., *(s32*)(t0+(s32)tbl_125c), v_arg5);` in the csmd4 inline block. sandbox --disable all.
- result: masked=6, target_insns=160, build_insns=160 — INERT vs g3 baseline masked=6. The multi-set VALUE carrier does NOT flip the v1/a0 exchange on g3 base; qty-priority arithmetic is unchanged by seeding v_arg5 through a distinct multi-set carrier. Rejected form at memory/grind/cpu_side_move_dir_4/rejected/g3_multiset_varg5_idx1495_seed.c.
- verdict: KILLED

## [s13] Same multi-set VALUE carrier form seeded from live local t0 (rather than *idx_1495) shifts the qty arithmetic since t0 is a live-value seed rather than a symbol load, ruling out any expand-time equiv_constant fold.
- mechanism: Diagnostic pair for hypothesis 2 — mirrors s5 F1b vs F1a on h5 base. If h5's F1a/F1b symmetry holds on g3, this confirms the multi-set carrier vehicle is qty-equivalent to inline for g3 too, regardless of first-set source.
- probe: Same as hypothesis 2 but `v_arg5 = t0;` first set. sandbox --disable all.
- result: masked=6, target_insns=160, build_insns=160 — identical to *idx_1495 seed. CONFIRMS the multi-set carrier vehicle is qty-invariant on g3 base regardless of first-set source symbol. The s5 F1a/F1b finding on h5 transfers verbatim to g3. Frontier live #2 is now closed across both seed variants. Rejected form at memory/grind/cpu_side_move_dir_4/rejected/g3_multiset_varg5_t0_seed.c.
- verdict: KILLED

## [s13] A directed-permuter fresh-seed campaign on the h5-multexpander chassis (base_score 60, distinct chassis from s5's g3-basin permuter) discovers a novel closing form in its mutation neighborhood.
- mechanism: Standalone permuter random mutation over the h5 `t0 *= 4; t0 = (s32)((u8*)tbl_125c + t0); v0 <<= 2; arg5 = *(s32*)(v0 + (s32)tbl_125c)` chassis; base weighted-score 60 vs g3's 40; different mutation neighborhood than s5's g3-base 9040-iter campaign. Fresh-seed discipline: harvest+stop at ~20-30 min or on no-novel-find window.
- probe: Refreshed tmp/perm_csmd4/base.c to the current h5 candidate spelling; tools/permuter_campaign.py launch --func cpu_side_move_dir_4 --dir tmp/perm_csmd4 --label s13_h5_fresh_multexpander -j 8 --stop-on-zero. Harvested with --stop after crash-rate observed.
- result: Campaign ran 142.5s / 2999 iterations / 0 NEW finds (existing output-40-1/2 pre-date launch and are unchanged). base_score=60 as expected for the h5-multexpander chassis. Permuter workers occasionally hit an AssertionError in perm_add_sub randomizer on the `(u8*)tbl_125c + t0` pointer-arith pattern, but 2999 iters completed via other mutations. 0-find harvest is the data point per fresh-seed discipline: the h5-multexpander mutation neighborhood at base=60 yields nothing at 20-min wall time — the local-minimum-under-random-mutation is stable on this chassis just as it was on g3 (s5).
- verdict: KILLED

## [s14] Directed PERM_GENERAL alternatives for the pointer-arith nodes `(u8*)tbl_125c + t0` and `v0 + (s32)tbl_125c` reach mutation forms permuter random misses due to the perm_add_sub AssertionError blindspot, and discover a sub-baseline C form.
- mechanism: PERM_GENERAL cross-product opens 4x5=20 distinct base sources at the two pointer-arith sites; PERM_RANDOMIZE around the debug_printf call keeps random mutation active around each directed seed; the directed forms should be reachable even when random pass would AssertionError on that node type.
- probe: Edited tmp/perm_csmd4/base.c with PERM_GENERAL at both pointer-arith sites (t0-side 4 alternatives, arg5-side 5 alternatives) plus PERM_RANDOMIZE on the CALL; tools/permuter_campaign.py launch --label s14_h5_directed_ptrarith --dir tmp/perm_csmd4 -j 8 --stop-on-zero; harvested at 895s / 23,427 iters.
- result: 6 new finds: 3 novel score-40 forms (output-40-3/4/5) all in g3-basin structural neighborhood (statement-form `t0 <<= 2` re-emerges from random mutation); 3 score-50 finds (worse than baseline); ZERO sub-40 finds; base_score of the merged 20-source set was 40 (g3), not 60 (h5-multexpander) — my alternatives INCLUDED g3-basin-shape forms (`(s32)tbl_125c + t0`, `(s32)((u8 *)t0 + (s32)tbl_125c)`) which the permuter picks as the lowest-cost base, so the campaign explored g3 neighborhood NOT h5. Traceback confirms AssertionError('dereferencing non-pointer') still fires from randomizer.py::perm_temp_for_expr on some alternative shapes (19 permuter failures at harvest), so the directed alternatives themselves are only partially expanding the random mutator's reach on that node.
- verdict: KILLED

## [s14] Novel score-40 form from directed-PERM (memory/grind/cpu_side_move_dir_4/rejected/perm_s14_hoist_new_var2_g3_basin.c: `new_var2 = &D_800A11DC[D_800A11D5];` fn-scope hoist + `temp = arg5;` fn-scope carrier + `t0 = (s32)&((u8*)tbl_125c)[t0]` indexed form) closes below h5's masked=2.
- mechanism: Adversarial verification of the directed-PERM find. Weighted permuter score 40 corresponds to g3-basin sandbox territory (masked 6); would need distinct-form novelty in the register-allocation web to close.
- probe: Static inspection of the find: pattern is a fn-scope carrier hoist (new_var2) + arg5-to-temp reuse — cheat-shaped per no-new-park-categories (fn-scope carriers are global pseudos per twin's s2/s3 finding, no semantic purpose beyond RA coercion; `temp = arg5;` is a dead re-alias). This is the same cheat family the WIP notes explicitly rejected (status/temp finds -> s0 spill, honest 6-8, rejected). Not sandbox-measured because the form fails cheat vetting before measurement.
- result: Form is a cheat-family (fn-scope carrier + dead alias); NOT sandbox-measured. Even if it scored masked-0 in sandbox, it would fail layer-1 cheat-reviewer (no semantic purpose; matches expanded cheat catalog and prior WIP rejected banks).
- verdict: KILLED

## [s15] The h5-basin residual pair-swap at clock=13 is decided inside sched.c::rank_for_schedule (lines 2399-2456) when tmp_class(121)-tmp2_class(111)=0 forces fallthrough to INSN_LUID diff; the class-3 assignment for both 121 and 111 comes from insn_cost(insn,link,123)==1 evaluated at rank_for_schedule.c:2420/2428.
- mechanism: rank_for_schedule test order: (1) INSN_PRIORITY diff — both LAUNCH sentinel 0x7f000001 (s7-confirmed via expmed.c:2244 case alg_shift NULL_RTX); (2) class diff from LOG_LINKS(last_scheduled=123) cost gate — both cls=3 because MIPS mips_adjust_cost returns 1 for ALU ashift/plus edges; (3) LUID(121)=12 vs LUID(111)=8 → 121 sorted later → picked first at clock=13 → linear order 118,111,121.
- probe: Grep RANKDBG rows in tmp/grind/cpu_side_move_dir_4/s6/csmd4_only.log for the last=123 y=121 x=111 event and count val distribution across block=3 (all sched1+sched2 comparisons).
- result: grep returned `RANKDBG last=123 y=121 cls=3 x=111 cls2=3 val=0`; grep -c cls=3 returned 51 = full block=3 RANKDBG event count; ALL comparisons val=0. Confirms the entire block-3 ready-queue outcome reduces to LUID for LAUNCH-tied pairs with no class-differentiated decision anywhere in the function.
- verdict: CONFIRMED

## [s15] The class-attack path via anti/output-dep (cls=2 for insn 121 with respect to last_scheduled=123) is structurally impossible for this pair: 121 writes p107, 123 reads p107 through MEM(p107); this is RAW (data-dep, cls=1 candidate), not anti/output (WAR/WAW).
- mechanism: rank_for_schedule classifies via REG_NOTE_KIND(link) — data-dep (kind 0) → cls=1; anti/output → cls=2. The 121→123 edge is RAW because 123 loads from the address 121 computes. No C form can invert this direction while preserving the semantic meaning of the debug_printf arg5.
- probe: Static walkthrough of s6 lreg RTL (insn 121 = set p107 plus p75 p79; insn 123 = set p??? mem p107) plus sched.c rank_for_schedule class assignment lines 2419-2433.
- result: Anti/output-dep between the same producer→consumer pair is not realizable structurally. Cls=2 attack path CLOSED.
- verdict: CONFIRMED

## [s15] The class-attack path via data-dep with cost>1 (cls=1 for insn 121 wrt last_scheduled=123) requires MIPS mips_adjust_cost to raise the 121→123 latency above 1, which for a plain (plus)→(mem) edge does not happen; only mul/div/FP producers get extra latency.
- mechanism: sched.c insn_cost:1363 calls ADJUST_COST macro (MIPS: mips_adjust_cost in mips.c). For ALU→memory-address edges the base cost is 1 and no adjustment fires. LINK_COST_FREE gets clamped to 1 at sched.c:1404-1405 → class-assignment gate evaluates true → cls=3 for 121.
- probe: Read sched.c:1363-1417 (insn_cost) + grep mips_adjust_cost / ADJUST_COST macro locations in tools/gcc-2.7.2/config/mips/.
- result: For the current h5 candidate, insn_cost(121,link,123)==1 unconditionally. To make it >1, the operands of insn 121's PLUS must involve a HI/LO (mul-result) or genuine multi-cycle producer, at which point mips_adjust_cost adds an inter-op latency stall.
- verdict: CONFIRMED

## [s16] s15 frontier #1: introducing a HI/LO (mul-result) producer into insn 121's PLUS operand raises insn_cost(121,link,123) above 1 via mips_adjust_cost, dropping 121 to cls=1 and yielding val=-2 at the clock=13 tiebreak.
- mechanism: sched.c::rank_for_schedule cls-gate at lines 2419-2433 tests insn_cost(tmp,link,last_scheduled)==1; if >1, cls=1 (data-dep with cost>1); MIPS mips_adjust_cost was claimed to fire for HI/LO consumer edges yielding extra latency.
- probe: Static walkthrough of tools/gcc-2.7.2/config/mips/mips.h:2946-2948 (ADJUST_COST macro), tools/gcc-2.7.2/sched.c:1363-1417 (insn_cost body), tools/gcc-2.7.2/insn-attrtab.c:755+ (result_ready_cost function-unit table generated from mips.md:148-183 define_function_unit entries).
- result: TRIPLE-KILL. (a) mips.h:2946-2948 ADJUST_COST macro has exactly ONE clause: if REG_NOTE_KIND(link)!=0 (anti/output-dep) then COST=0; the data-dep else-branch is a no-op — the R8000 fixme comment confirms the macro is deliberately minimal, there is NO HI/LO-consumer stall clause in this compiler for MIPS. (b) sched.c::insn_cost(insn,link,used) sets base = result_ready_cost(insn) clamped to >=1, then ADJUST_COST can only DECREMENT (per a) and clamps ncost to >=1 via LINK_COST_FREE — the function architecturally cannot lift a cost above result_ready_cost(producer). (c) result_ready_cost reads TYPE(insn), not its inputs. TYPE(121)=arith (plus), r3000 default ready-cost=1 (mips.md:148 default; no override for addsi3). A mul UPSTREAM of 121 changes result_ready_cost of the mul (12), i.e. the mul->121 edge cost — not the 121->123 edge cost that rank_for_schedule tests. Frontier #1's mechanism is not realizable under this frozen compiler on any C spelling.
- verdict: KILLED

## [s16] cls(121) can be raised by making insn 121 an anti/output-dep of insn 123 (WAR/WAW) rather than a RAW producer.
- mechanism: ADJUST_COST would zero the cost, making rank_for_schedule assign cls=2 in the classification gate. This was already CONFIRMED impossible in s15 H2 (121 writes p107; 123 reads MEM(p107) is producer->consumer, semantically fixed).
- probe: Cross-check with s15 H2 evidence + arg5's semantic role in the debug_printf call — arg5's address must be computed before it's dereferenced.
- result: Semantically impossible without changing the C-observable behavior of the debug_printf call. Confirms s15 H2 conclusion — the class-2 path was already closed structurally.
- verdict: CONFIRMED

## [s16] cls(121) can be raised by changing TYPE(121) itself — making insn 121 be a load or a multiply instead of an arith plus.
- mechanism: result_ready_cost reads TYPE(insn); TYPE=load gives cost=2 on r3000, TYPE=imul gives cost=12. This is the ONLY compiler-visible dial that lifts insn_cost above 1 for data-dep edges.
- probe: Consider whether any C form makes the arg5_addr plus vanish into a memory addressing mode (mem (plus reg reg)) — MIPS lw supports only imm(base). tbl_125c is a 32-bit link-time symbol, requires lui+addu materialization. The plus insn is architecturally required. And no C spelling reasonably makes the arg5 address itself a multiply (semantic content is v0 + tbl).
- result: No realizable path. The plus insn 121 is architecturally mandatory as long as the arg5 access is a 32-bit-symbol-plus-runtime-index MEM load on MIPS.
- verdict: KILLED

## [s17] The decomp.me corpus (~3754 scratches, BB2 toolchain class: gcc2.7.2-psx / gcc2.7.2-cdk / psyq3.5) contains at least one scratch whose C shape lies OUTSIDE the h5/g3 basins and provides a novel starting template for csmd4's inline-block pair-swap residual.
- mechanism: Coarse asm-overlap shingle pre-filter (tools/decomp_me_scrape.py search) over the downloaded corpus surfaces scratches with textually-similar asm windows; manual inspection of top hits reveals a C-shape angle not yet measured on csmd4.
- probe: curl_cffi 0.15.0 now importable in .venv (s9 blocker gone). Ran `python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/cpu_side_move_dir_4.s --corpus tmp/decomp_me_corpus --top 30`; saved to tmp/grind/cpu_side_move_dir_4/s17/scrape_top30.txt. Inspected source_code of top 15 hits (zppzv, b1d0d, bNzDh, erpie, h1LHX, luvU2, zUF0A, nSXIg, k2213, 7WHZs, WV6us, qrJlK, xsigV, G0AqX, Azqrh) via tmp/grind/cpu_side_move_dir_4/s17/shape_check.py.
- result: Top similarity 0.097 (zppzv, func_80098304, gcc2.7.2-cdk); 15th hit 0.075. NONE of the top-15 hits contain a debug_printf-style variadic call, none contain the tbl[idx]-lookup dispatch shape (D_800A11DC[D_800A11D5]-analog), only 1 references VSync (erpie/entrypoint_sotn — different function skeleton). Top hits are game-loop/state-machine functions matching on generic lbu/sll/addu shingles, NOT on the specific inline-block pair-swap structure that generates csmd4's masked-2 residual. The corpus does not contain a structurally-analogous scratch.
- verdict: KILLED

## [s18] The 'external Kengo/nm_cpu source glance' frontier note (b) is unavailable per WIP notes; a local search will confirm no Kengo assets exist in-tree.
- mechanism: Ledger note (c) from s9 states 'no known second twin in the codebase per notes.md'; the frontier requires operator supply.
- probe: find . -maxdepth 4 -iname '*kengo*' -o -iname '*nm_cpu*'; ls Kengo/ Kengo/disc/
- result: Kengo assets ARE present: top-level Kengo/ (with 498 MB PS2 disc bin, 3.15 MB SLUS_200.21 ELF, and 4 dump txt files) + identical copies under multiple .claude/worktrees/*/Kengo/. The ledger note was pessimistic; assets exist locally. Reclassifying frontier note (b) as PROBABLE.
- verdict: KILLED

## [s18] The local Kengo dump (kengo_functions_full.txt) contains a C body or m2c-shaped signature for cpu_side_move_dir at addr 0x0013aa60 (160 insns / 640 bytes — exact size match with BB2 csmd4) that gives a structurally novel starting template.
- mechanism: Kengo's cpu_side_move_dir (nm_cpu.c, PS2 successor to Lightweight's Marionation engine) is the direct source ancestor referenced in src/system.c:497 tombstone. A recovered C body would show the original inline-block statement structure, potentially outside the h5/g3 basins csmd4's ladder has measured.
- probe: grep 'cpu_side_move_dir' Kengo/kengo_functions_full.txt + surrounding context; grep debug info at 0x13aa60 in kengo_debug_full.txt; count empty-stub prevalence.
- result: Entry at line 1077: '/* 0013aa60 00000280 */ cpu_side_move_dir() {}' — EMPTY STUB, no body, no parameter signature, no local declarations. One of 2409 empty-stub entries (grep -c '{}$' = 2409). Some libc/math functions (cos/sin/atan2/cosf/sinf/etc) DO have m2c-shaped bodies with local variable + register info, so the dump format supports body content — cpu_side_move_dir simply wasn't extracted. Debug info at addr 0x0013aa60 also absent from kengo_debug_full.txt (the .dbg records are limited to .dsm/.vsm VU-microcode files, no .c source lines for nm_cpu.c). File attribution recoverable from surrounding section header: src/numata/nm_cpu.c.
- verdict: KILLED

## [s18] Disassembling the Kengo ELF at 0x0013aa60 to recover PS2 asm, then hand-reconstructing the C shape from that asm, would give a novel-shape starting template for csmd4 that escapes the h5/g3 basins.
- mechanism: The ELF (Kengo/disc/SLUS_200.21, elf32-tradlittlemips per kengo_debug_full.txt line 1) is a valid target for mipsel objdump; the function's asm is directly recoverable. If Kengo's inline-block shape at that address differs structurally from every csmd4 candidate measured, transplanting it might land in a new basin.
- probe: Static reasoning against the ledger's compiler-forensics (s6/s7/s16). Kengo is PS2 (R5900 EE core) compiled with GCC 3.x SN Systems / EE-GCC; BB2 is PS1 (R3000A) with GCC 2.7.2 PsyQ. The csmd4 h5 residual is a compiler-internal mult-expander LUID tiebreak (s16 CONFIRMED: expmed.c::expand_mult case alg_shift line 2244 NULL_RTX target hardcoded, produces LAUNCH insn 111 that ties with LAUNCH insn 121 at sched.c val=0). ANY C form of '× 4' in BB2 that reaches expand_mult produces this signature; only direct '<<=' bypasses (basin flip to g3 masked=6, s7 CONFIRMED).
- result: Reasoning conclusion (not measured): Kengo asm reconstruction is transferable to csmd4 only in shape, not in codegen — and the shape space is exhaustively closed. The angle is available in principle but adds no new signal beyond s8/s9/s17.
- verdict: KILLED

## [s19] The ladder's next measurable lever is a strictly h5-preserving PERM_LINESWAP (no PERM_GENERAL, no PERM_RANDOMIZE) that enumerates dataflow-valid statement orderings over the 6-8 non-CALL statements in the inline block.
- mechanism: LINESWAP does not mutate expressions, so h5's mult-expander LAUNCH (insn 111's fresh p106 dest from expmed.c case alg_shift NULL_RTX target, s7 CONFIRMED) is preserved by construction. This is orthogonal to the s14 g3-drift failure mode where PERM_GENERAL alternatives included g3-basin-shape spellings and the merged base_score dropped to 40.
- probe: Synthesis-only: hypothesis elevated to frontier #1 for next session. Guardrails documented (base_score MUST measure 60 at launch; abort if 40).
- result: Elevated to frontier #1. Not measured this session per synthesis modality.
- verdict: CONFIRMED

## [s19] Duplicated-statement-into-arms lever on NON-t0 targets (D_800F19C0 store, D_800A11DC[D_800A11D5] dispatch load, pp alias) unlocked by the 2026-07-01 formal sanction can lift arg5-qty REG_N_REFS via label-steered cross-jump merge without touching h5-basin machinery (insn 111 p106 dest, insn 121 p107 dest).
- mechanism: s10 demotion of frontier #3 was specific to t0-chain targets (would double p101 refs -> g3 regression). 2026-07-01 FAKE-annotated sanction covers non-t0 targets. Cross-jump collapses duplicates at middle-end while REG_N_REFS was already booked at tree/RTL level, distinct from s13's hoist mechanism (+13 per-iteration store).
- probe: Synthesis-only: hypothesis elevated to frontier #2. Per-target judge-risk pre-analysis: D_800F19C0 OK, dispatch load conditional on other arm's control flow, tbl_125c bind likely FAILS layer-2, pp alias same concern as tbl_125c.
- result: Elevated to frontier #2. Not measured this session per synthesis modality.
- verdict: CONFIRMED

## [s19] A g3-basin arg5-side-only directed permuter campaign (PERM_LINESWAP over arg5 statements + PERM_GENERAL alternatives on arg5 dereference ONLY, NO t0-side directives) is the last untried permuter angle and targets the closable v1/a0 exchange residual in g3 masked=6.
- mechanism: s5's g3 9040-iter and s14's h5 directed 23,427-iter campaigns were both UNRESTRICTED or CROSS-BASIN. Arg5-side-only directed search on g3 chassis has never been measured. Target: arg5-qty pri>=5000 = weighted refs>=4 at span 6 (13333) OR refs 3 at span<=5 (6000) per s1 fable-blitz arithmetic.
- probe: Synthesis-only: hypothesis elevated to frontier #3. Guardrails: base_score MUST measure 40 (g3 chassis) at launch; NO t0-side directives to prevent basin drift; layer-2 vetting for the historical g3-permuter cheat family (0xFF/0xFFFF masks, u16 narrowing, alias repurposing).
- result: Elevated to frontier #3. Not measured this session per synthesis modality.
- verdict: CONFIRMED

## [s19] The rederive modality is fully closed across all known angles (m2c s8, marionation sibling s9, decomp.me corpus s17, Kengo local dump s18); no rederive frontier remains.
- mechanism: s8 m2c produced score-14 inline-all-args (known basin). s9 marionation P1-P4 = 7/14/8/20 (s-reg web non-fungible). s17 decomp.me top-15 peak similarity 0.097; 0/15 contained debug_printf+tbl-index shape. s18 Kengo cpu_side_move_dir is an empty stub; ELF is PS2 R5900/EE-GCC 3.x, arch-non-transferable to BB2's PS1 R3000A/GCC 2.7.2.
- probe: Cross-read of s8/s9/s17/s18 evidence and hypotheses.
- result: Rederive modality declared exhausted. Any future session choosing rederive must justify a genuinely new external input (unknown at s19).
- verdict: CONFIRMED

## [s19] The class-attack surface (raising cls(121) via cost>1, anti/output-dep, or type change) is exhausted at compiler-source level and no C-source lever can reach it under GCC 2.7.2 MIPS.
- mechanism: s16 TRIPLE-KILL confirmed: mips.h:2946 ADJUST_COST is a one-line macro that only zeros anti/output-dep and NEVER raises data-dep cost; sched.c insn_cost architecturally cannot lift above result_ready_cost(producer); result_ready_cost reads TYPE(insn) only, upstream operand types (mul/HI-LO) do not enter 121->123 edge cost. s15 H2 confirmed 121 writes p107 / 123 reads MEM(p107) is RAW, semantically fixed. s16 H3 confirmed no C spelling reasonably makes 121 a load or mul.
- probe: Cross-read of s15+s16 forensics.
- result: Class-attack surface declared exhausted. LUID is the sole surviving mechanism and is coupled to basin membership (s6 KILLED for h5).
- verdict: CONFIRMED

## [s20] Duplicated `D_800F19C0 = &D_80016240;` store into BOTH do_timeout arrival arms (arm1 pre-goto + arm2 fallthrough, prologue init retained) triggers GCC jump2's find_cross_jump to merge the arms into an identical byte sequence while flow.c reg_n_refs bookings on p_D_800F19C0 have already been incremented at the tree/RTL feed of local-alloc.c qty_compare, lifting arg-2 chain qty priority without disturbing insn 111 (p106) or insn 121 (p107) machinery, keeping h5 basin.
- mechanism: Cross-jump collapses duplicates at middle-end after refs booked. Frontier #2 top-ranked target per s19 synthesis.
- probe: Applied D_800F19C0 = &D_80016240; before `goto do_timeout;` and immediately before `do_timeout:` label (fallthrough arm), prologue init retained. sandbox cpu_side_move_dir_4 --disable all.
- result: masked=6, target_insns=160, build_insns=164 (+4 build insns vs target). Cross-jump merge did NOT fire — the `do_timeout:` label between the two arms obstructs find_cross_jump's tail-suffix match (jump2 requires suffix identity WITHOUT intervening labels). Both stores emit literally (2× lui+sw of &D_80016240). +4 masked vs h5 baseline. Falls out of h5 basin. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/dup_D_800F19C0_arms_keep_prologue.c (annotated).
- verdict: KILLED

## [s20] Same duplicated-D_800F19C0-store pattern with prologue init REMOVED lets find_cross_jump collapse the two arm stores back to one emitted instruction (semantically identical: at least one arm always fires before do_timeout), and the ref-lift bookings from the pre-merge duplication survive to affect qty priorities.
- mechanism: Removing the prologue init eliminates one competing use site so cross-jump's tail-identity check can fire on the two arm stores. reg_n_refs is booked at tree/RTL emission (jump-optimize pass), which runs BEFORE the jump2 cross-jump pass, so the merge preserves the ref-lift signal at flow.c/local-alloc.c.
- probe: Removed prologue `D_800F19C0 = &D_80016240;` line at src/system.c prologue; kept both arm duplicates as above. sandbox --disable all.
- result: masked=10, target_insns=160, build_insns=160. Cross-jump DID MERGE (build insn count matches target). Ref-lift mechanism is empirically reachable via this construct — but the resulting qty priority shift MISDIRECTS allocation with a +8 masked regression. Directly mirrors s5's whole-block do-while(0) wrap outcome (masked=13, symmetric ref-lift also monotonically wrong-direction). Confirms duplicated-statement-into-arms mechanism produces a ref-lift on the intended pseudo, but on the D_800F19C0 target the differential shifts qty priorities AWAY from arg5-strict-win rather than toward it. Rejected form at memory/grind/cpu_side_move_dir_4/rejected/dup_D_800F19C0_arms_no_prologue.c (annotated).
- verdict: KILLED

## [s20] Duplicated dispatch load `dispatch = D_800A11DC[D_800A11D5];` into both do_timeout arrival arms (second-ranked frontier #2 target) lifts arg3-chain qty via cross-jump merge without touching insn 111 (p106) or insn 121 (p107) machinery.
- mechanism: For the duplicated `dispatch = ...` stores to both arms to feed a single use in the debug_printf call sitting inside the inner scope block after do_timeout:, `dispatch` MUST be declared at fn-scope (or top-of-fn-block scope reaching past the inline `{ ... }`). Any block-scope declaration inside the inline block is NOT visible in the arms above the label. The required fn-scope named carrier has zero semantic purpose beyond RA coercion (no read/write outside the intended lift), matching the fn-scope-carrier cheat family per [[no-new-park-categories]] cheats-by-any-spelling and the s14 perm_s14_hoist_new_var2_g3_basin.c rejection precedent (twin s2/s3 finding: block-local decl order is inert; fn-scope carriers are the coercion form).
- probe: Policy vetting BEFORE measurement per no-new-park-categories cheat-vetting checklist test #2 (no semantic purpose), #3 (human programmer wouldn't write this), #4 (justification references GCC internals not program logic). s19 frontier note pre-flagged as 'dispatch load conditional on other arm's control flow' with layer-2 risk.
- result: Rejected without measurement by policy: fn-scope carrier is the same cheat family the WIP rejected banks already contain (perm_s14 case, twin s2/s3). Placeholder rejected form dup_dispatch_arms would be identical in shape to perm_s14. Second-ranked frontier #2 target CLOSED without measurement.
- verdict: KILLED

## [s20] Duplicated tbl_125c/pp binding into both do_timeout arms (third-ranked frontier #2 target, per s19 synthesis 'lower due to judge risk') lifts index-chain qty via cross-jump merge.
- mechanism: Same policy failure as the dispatch load: any duplicated `pp = (void**)&D_800F19C0;` or `tbl_125c = D_800A125C;` binding into both arms requires a fn-scope named carrier for the LHS to reach the inline block's use. s19 frontier note explicitly pre-flagged as 'judge risk LIKELY FAILS layer-2' — the LHS carriers have zero non-coercion purpose, matching the fn-scope-carrier cheat family.
- probe: Policy vetting BEFORE measurement. Confirmed by s19 frontier synthesis pre-analysis.
- result: Rejected without measurement by policy. Third-ranked frontier #2 target CLOSED. Rejected placeholder at memory/grind/cpu_side_move_dir_4/rejected/dup_tbl_125c_arms.c (annotated).
- verdict: KILLED

## [s21] Routing the arg5-side × 4 via `v0 = v0 * 4;` gives p_v0 a fresh single-set dest through expmed.c::expand_mult case alg_shift NULL_RTX (mirroring t0-side p106 birth), creating a symmetric LAUNCH on arg5 side that either flips the pair or synchronizes the tie differently.
- mechanism: Per s7-CONFIRMED expmed.c line 2244 hardcoded NULL_RTX target on case alg_shift, `v0 = v0 * 4` OUGHT to birth a fresh single-set pseudo just like `t0 *= 4` does for p106. sched.c::adjust_priority + birthing_insn_p would then LAUNCH the arg5-side SLL insn with the same 0x7f000001 sentinel, potentially altering the ready-queue outcome at clock 13.
- probe: Replaced `v0 <<= 2;` with `v0 = v0 * 4;` on h5 candidate (v0 = idx_1494[1] staging preserved). sandbox --disable all.
- result: masked=2 INERT, target_insns=160, build_insns=160. Bytes match h5 baseline. Empirical finding: when the OUTER SET's target is v0's pseudo (not NULL), expand_mult's alg_shift NULL_RTX target is folded/absorbed by the outer SET at RTL emission — the emitted SLL is in-place on p_v0, identical to `v0 <<= 2`. The expand_mult fresh-dest mechanism (s7) only births a distinct pseudo when the outer context provides no target, i.e. when the multiply is a SUB-expression of a larger expression tree (as in `t0 *= 4;` where `*=` combined form causes expand_mult's target arg to be threaded away from the outer SET dest). Rejected form at memory/grind/cpu_side_move_dir_4/rejected/v0_mult4_arg5_side.c.
- verdict: KILLED

## [s21] Two-shift form `v0 <<= 1; v0 <<= 1;` on arg5 side adds an extra SET to p_v0's set count, altering flow-time reg_n_sets and possibly shifting qty priority.
- mechanism: Local-alloc.c qty_compare feeds from flow.c reg_n_sets; adding an extra in-place SET on p_v0 would increment reg_n_sets(p_v0) from 2 (init + shift) to 3 (init + shift1 + shift2). Different priority arithmetic.
- probe: Replaced `v0 <<= 2;` with `v0 <<= 1; v0 <<= 1;`. sandbox --disable all.
- result: masked=2 INERT, target_insns=160, build_insns=160. Bytes identical to h5 baseline. combine.c fold_rtx recognizes `(ashift (ashift x 1) 1)` as `(ashift x 2)` and canonicalizes to a single SLL — the extra SET is optimized away at combine time before flow, so reg_n_sets(p_v0) is unchanged at flow-time. Rejected form at memory/grind/cpu_side_move_dir_4/rejected/v0_two_shift_arg5_side.c.
- verdict: KILLED

## [s21] Addsi3 chain form `v0 = v0 + v0; v0 = v0 + v0;` on arg5 side routes through addsi3_internal instead of ashiftsi3, providing a distinct expand path.
- mechanism: Different RTL codegen path — expand_binop(PLUS_EXPR) instead of expand_binop(LSHIFT_EXPR); addsi3_internal recognizer vs ashiftsi3 recognizer. If cse.c or combine.c does NOT recognize the (plus x x) chain as a shift, the emitted code would differ.
- probe: Replaced `v0 <<= 2;` with `v0 = v0 + v0; v0 = v0 + v0;`. sandbox --disable all.
- result: masked=2 INERT, target_insns=160, build_insns=160. Bytes identical to h5 baseline. combine.c fold_rtx `simplify_plus_minus` recognizes `(plus x x)` and canonicalizes to `(ashift x 1)`; the chain collapses to `(ashift x 2)` at combine time. Confirms combine.c's aggressive shift-canonicalization on all self-doubling patterns. Rejected form at memory/grind/cpu_side_move_dir_4/rejected/v0_double_add_arg5_side.c.
- verdict: KILLED

## [s21] Fully inline arg5 without any v0 staging (`arg5 = *(s32 *)((idx_1494[1] << 2) + (s32)tbl_125c);` with no `v0 = idx_1494[1]` set before) probes whether the h5 basin depends on the fn-scope v0 SET being present in the arg5 chain.
- mechanism: The h5 candidate ships with a sanctioned staged-value-reused-variable form: `v0 = idx_1494[1]; v0 <<= 2; arg5 = *(s32 *)(v0 + tbl);`. Removing the fn-scope v0 SET converts the arg5 index to a single-set fresh pseudo local to the address computation. This tests whether v0's presence is load-bearing to the basin, or merely a documented FAKE that could be replaced by any single-set carrier.
- probe: Replaced the full arg5-side chain (v0=idx_1494[1]; v0<<=2; arg5=*(s32*)(v0+tbl)) with `arg5 = *(s32 *)((idx_1494[1] << 2) + (s32)tbl_125c);`. sandbox --disable all.
- result: masked=4 (+2 REGRESSION vs h5 baseline), target_insns=160, build_insns=160. NOVEL DATA POINT: fully-inline arg5 does NOT reach the h5 basin. The fn-scope v0 SET before the shift is empirically load-bearing to masked=2. Mechanism explanation: without v0 pre-set, the arg5 index becomes a single-set fresh pseudo (never launched via the staged carrier). This shifts qty priorities and yields a distinct sched2 outcome (+2 masked). Confirms the sanctioned staged-value-reused-variable pattern is not just cosmetic — the v0 SET carries the mult-expander LAUNCH-tie into a specific alignment that requires the v0 pseudo's participation in the arg5 chain. This ELIMINATES the "unstage v0" search subspace: no arg5-side inline form (no fn-scope v0 SET participation) can reach the h5 masked=2 floor. Rejected form at memory/grind/cpu_side_move_dir_4/rejected/arg5_fully_inline_no_v0_staging.c.
- verdict: KILLED

## [s21] Routing arg5-side *4 via `v0 = v0 * 4;` births a fresh single-set pseudo via expmed.c::expand_mult case alg_shift NULL_RTX target, mirroring t0-side p106 birth and creating a symmetric LAUNCH signature on arg5 that shifts the sched2 tiebreak outcome.
- mechanism: Per s7-CONFIRMED expmed.c:2244 hardcoded NULL_RTX target on case alg_shift, `v0 = v0 * 4;` OUGHT to birth a fresh single-set pseudo and LAUNCH on the arg5 side.
- probe: Replaced `v0 <<= 2;` with `v0 = v0 * 4;` on h5 candidate (v0 = idx_1494[1] staging preserved). sandbox --disable all.
- result: masked=2 INERT, target_insns=160, build_insns=160. When the OUTER SET's target is v0's pseudo, expand_mult's alg_shift NULL_RTX target is absorbed by the outer SET at RTL emission — SLL emitted in-place on p_v0, identical to `v0 <<= 2`. expand_mult fresh-dest mechanism (s7) only births a distinct pseudo when the outer context provides no target (multiply as sub-expression, e.g. `t0 *= 4;` where the *= form threads target away).
- verdict: KILLED

## [s21] Two-shift form `v0 <<= 1; v0 <<= 1;` on arg5 side gives p_v0 an extra in-place SET, potentially shifting flow.c reg_n_sets and qty priority.
- mechanism: Local-alloc.c qty_compare feeds from reg_n_sets; extra SET on p_v0 would increment reg_n_sets from 2 to 3.
- probe: Replaced `v0 <<= 2;` with `v0 <<= 1; v0 <<= 1;`. sandbox --disable all.
- result: masked=2 INERT. combine.c fold_rtx recognizes (ashift (ashift x 1) 1) as (ashift x 2) and canonicalizes to a single SLL BEFORE flow, so reg_n_sets is unchanged at flow-time.
- verdict: KILLED

## [s21] Addsi3 chain form `v0 = v0 + v0; v0 = v0 + v0;` on arg5 side routes through addsi3_internal instead of ashiftsi3, giving a distinct expand path that combine may not shift-canonicalize.
- mechanism: Different RTL emission path — expand_binop(PLUS_EXPR) vs expand_binop(LSHIFT_EXPR); if cse/combine doesn't recognize (plus x x)-chain as shift, emitted code differs.
- probe: Replaced `v0 <<= 2;` with `v0 = v0 + v0; v0 = v0 + v0;`. sandbox --disable all.
- result: masked=2 INERT. combine.c simplify_plus_minus recognizes (plus x x) as (ashift x 1) and collapses the chain to (ashift x 2). Confirms combine's aggressive shift-canonicalization on all self-doubling patterns — no C spelling of x*4 as an addsi3 chain escapes to distinct RTL.
- verdict: KILLED

## [s21] Fully inline arg5 without any v0 staging (`arg5 = *(s32*)((idx_1494[1] << 2) + tbl_125c);` with no v0 = idx_1494[1] set before) probes whether the h5 basin depends on the fn-scope v0 SET being in the arg5 chain.
- mechanism: Removing v0 = idx_1494[1] converts the arg5 index into a single-set fresh pseudo local to the address computation. Tests whether v0's participation is load-bearing to h5 or merely cosmetic FAKE-annotation.
- probe: Replaced arg5-side chain (v0=idx_1494[1]; v0<<=2; arg5=*(s32*)(v0+tbl)) with `arg5 = *(s32 *)((idx_1494[1] << 2) + (s32)tbl_125c);`. sandbox --disable all.
- result: masked=4 (+2 REGRESSION), target_insns=160, build_insns=160. NOVEL DATA: fully-inline arg5 does NOT reach h5 basin. The fn-scope v0 SET before the shift is empirically load-bearing to masked=2. Confirms sanctioned staged-value-reused-variable is not cosmetic; v0's participation in arg5 chain carries the mult-expander LAUNCH tie into the alignment that reaches masked=2. ELIMINATES the 'unstage v0' subspace: no arg5-side inline form without fn-scope v0 SET reaches h5.
- verdict: KILLED

## [s22] A strictly h5-preserving PERM_LINESWAP campaign (SOLE directive, no PERM_GENERAL, no PERM_RANDOMIZE) enumerates dataflow-valid statement orderings over the 7-statement h5 inline block and discovers an ordering below h5 masked=2.
- mechanism: LINESWAP does not mutate expressions, so h5's mult-expander LAUNCH on insn 111 (fresh p106 dest via expmed.c case alg_shift NULL_RTX target, s7 CONFIRMED) is preserved for orderings that keep the multi-set t0 chain. Reordering statements changes LUID assignment at expand; the frontier's claim was that some LUID reassignment on the arg5 side (without moving t0*=4 past arg5, which s6 KILLED as g3-drift) would win the sched2 tiebreak at clock=13.
- probe: Rewrote tmp/perm_csmd4/base.c with PERM_LINESWAP as SOLE directive wrapping the 7 non-CALL statements of the h5 inline block (t0=idx_1494[0]; v0=idx_1494[1]; pp=&D_800F19C0; t0*=4; t0=(u8*)tbl_125c+t0; v0<<=2; arg5=*(s32*)(v0+tbl_125c);). PERM_LINESWAP verified as SOLE directive (grep). tools/permuter_campaign.py launch --func cpu_side_move_dir_4 --dir tmp/perm_csmd4 --label s22_h5_lineswap_strict -j 4 --stop-on-zero. base_score=60 confirmed at launch (h5-multexpander chassis, as required by guardrail). Campaign ran to completion in ~7 minutes: 5040 iterations = 7! deterministic enumeration exhausted.
- result: 3 novel finds (output-55-2, output-40-6, output-40-7) all with permuter weighted score >= 40. output-40-7 (arg5-first, v0<<=2 before t0*=4) empirically measured on sandbox --disable all: masked=6, target_insns=160, build_insns=160 — g3-basin (matches s5/s13 correlation permuter-weighted-40 ↔ sandbox masked=6). No ordering below permuter score 40 discovered; no ordering reaches or beats h5 masked=2.
- verdict: KILLED

## [s22] The LINESWAP enumeration exhaustion (all 5040 orderings tested by permuter's deterministic pass) empirically closes the statement-ordering axis under strict h5-expression-preservation.
- mechanism: PERM_LINESWAP is a bounded permutation search (N! orderings for N statements); with N=7 the search is complete in 5040 iterations. permuter emits an output-<score>-<ctr> only when the ordering compiles and produces a distinct-score object, so any dataflow-valid ordering with novel score would surface. Combined with the observed base_score=60 (h5 basin preserved) at launch AND the ORCHESTRATED novel finds cluster at score 40 (g3 basin) and 55 (intermediate), the search has EXHAUSTIVELY covered the LINESWAP orderings of the h5 statements.
- probe: Enumeration completed (permuter alive=false at 5040 iterations exactly, matching 7!). Reviewed the campaign_meta.json preexisting_outputs vs post-harvest outputs to isolate novel finds. Measured one novel find on sandbox to correlate weighted-40 ↔ masked-6 (g3 basin).
- result: LINESWAP-only search of the h5 7-statement block is EXHAUSTED; no ordering exists (within LINESWAP's constraint of dataflow-valid, expression-preserving reorderings) that reaches masked < 2. Combined with s3 (V8/V9/V11 hand-swept subset of orderings all INERT), s6 LUID-reorder KILLED, and s13/s14 prior directed permuter campaigns, the statement-ordering axis on h5's block is now closed at both hand + tool level.
- verdict: CONFIRMED

## [s23] PERM_LINESWAP_TEXT (line-level, text-based) applied to tslTm2LoadImage_2 + the 7 h5 inline-block statements (8 items, 8! = 40320 pure orderings + random-mutation neighborhood) reaches sanctioned orderings that s22 PERM_LINESWAP (AST-based, block-scoped) could not, and one of them scores masked < 2.
- mechanism: PERM_LINESWAP_TEXT does textual shuffling of the 8 argument chunks; unlike AST-based LINESWAP it can cross the tslTm2LoadImage_2 CALL boundary and interleave it with the inline-block statements. This opens orderings that could re-time the block-3 sched.c ready queue without disturbing h5 basin membership (multi-set t0 chain preserved verbatim).
- probe: tmp/perm_csmd4/base.c edited to move tslTm2LoadImage_2(&D_800161B8); INTO the inner {} block and wrap it + the 7 inline statements in PERM_LINESWAP_TEXT. Campaign via tools/permuter_campaign.py launch --label s23_h5_lineswap_text -j 4 --stop-on-zero. Verified base_score=60 at launch (h5 chassis, guardrail satisfied). Ran 1462.5s / 20429 iterations / 4 workers before harvest+stop.
- result: 2 new finds: output-55-3 (weighted 55, arg5-chain hoisted before t0 chain with pp mid-block) and output-40-8 (weighted 40 = g3 basin, dupe of s22 40-6/7). NO find with weighted score <= 40 at masked < 2. output-55-3 sandbox-measured masked=7 (novel intermediate basin between h5 masked=2 and g3 masked=6, mirrors s2 head-order-flip regression: hoisting arg5's chain before t0's re-times the head-load LUID). output-40-8 assumed masked=6 per s22 weighted-40 <-> masked-6 correlation.
- verdict: KILLED

## [s23] The textual-mutation space of PERM_LINESWAP_TEXT (much larger than 5040 = 7! per frontier note) contains an h5-preserving ordering that reaches masked < 2 within a ~20-min fresh-seed window.
- mechanism: Fresh-seed discipline (owner directive 2026-07-07): if ~20-30 min after fresh seed there is no NOVEL find with weighted score < 40, harvest and reseed differently or switch lever.
- probe: Campaign wall time capped at ~24 min; monitored /tmp/perm_csmd4/output-* directory for novel finds; harvested+stopped at 1462s per fresh-seed discipline.
- result: Zero weighted-score-<40 finds across 20429 iterations. Both novel finds sit at weighted 40 (g3 dupe) or weighted 55 (intermediate basin above h5). The textual-mutation space is empirically exhausted for h5-below-2 orderings within the fresh-seed window; longer wall time is against the fresh-seed discipline and unlikely to shift the distribution.
- verdict: KILLED

## [s24] The PERM_INT frontier - sweeping the multiplier constant to reach alg_add_t_m2 / alg_add_factor branches of expand_mult that thread add_target/accum_target through - is compiler-source-reachable and could re-birth p106 in an existing pseudo.
- mechanism: expmed.c::synth_mult(t=4) deterministically returns {alg_m, alg_shift(log=2)} because (a) (t&1)==0 path finds m=2 q=1 base case at cost=shift_cost[2]; (b) factor loop m>=2 never runs (floor_log2(3)=1<2); (c) odd branches gated on (t&1)!=0 skip. In expand_mult, case alg_shift explicitly passes NULL_RTX to expand_shift and IGNORES add_target/target. Alternate algorithms (alg_add_t_m2/alg_add_factor/alg_add_t2_m) would thread add_target through, but they are only selected for values whose synth_mult decomposition is not a single shift.
- probe: Static walkthrough of tools/gcc-2.7.2/expmed.c:1898-2125 (synth_mult) and expmed.c:2231-2302 (expand_mult switch); traced val=4 through every branch. Cross-referenced expmed.c:2243-2247 case alg_shift NULL_RTX hardcode. Cross-referenced s6 combine dump insn 111 REG_EQUAL (mult p101 4). Artifact: tmp/grind/cpu_side_move_dir_4/s24/synth_mult_val4_walkthrough.md.
- result: For val=4 (the ONLY semantic-preserving multiplier for tbl_125c+idx*4 address), synth_mult picks the pure alg_shift path unconditionally. The frontier's promised algorithm neighborhood (alg_add_t_m2/alg_add_factor) is unreachable without changing the multiplier value, which would break the address semantic and be sandbox-rejected. No permuter sweep can bypass this because PERM_INT is byte-semantic-preserving only across values GCC treats as equivalent, and 4 is a mult-expander singleton.
- verdict: KILLED

## [s24] The comma-op frontier - rewriting `v0 <<= 2; arg5 = *(s32*)(v0+tbl)` as `v0 <<= 2, arg5 = *(s32*)(v0+tbl)` folded into a single comma-expression sequence-point statement - may create a distinct RTL insertion point that lands p106 and p107's SET INSNs at the same LUID, degenerating sched.c::rank_for_schedule's LUID tiebreak.
- mechanism: expr.c:5710-5715 expand_expr COMPOUND_EXPR: sequentially expands operand 0 with target=const0_rtx, calls emit_queue(), then expands operand 1 with the outer target. Both re-entries call the same expand_expr leaves as separate `;`-statements would, producing sequential emit_insn calls with strictly increasing UIDs. sched.c LUID assignment walks the RTL chain and assigns integers by emit order - two distinct emit_insn calls ALWAYS produce two distinct LUIDs. There is no code path in COMPOUND_EXPR handling that batches, merges, or aliases insn UIDs across the two operands.
- probe: Read tools/gcc-2.7.2/expr.c:5710-5715 (COMPOUND_EXPR case) + surrounding leaf expand_expr calls. Confirmed emit_queue's semantics (queued-insn flush for post-inc/dec, no aliasing effect on already-emitted insns). Artifact: tmp/grind/cpu_side_move_dir_4/s24/comma_op_luid_walkthrough.md.
- result: COMPOUND_EXPR expands to a sequential re-entry into expand_expr for its two operands, producing byte-identical insn stream to `;`-separated statements at same C position. LUIDs remain strictly ordered by C statement order regardless of `,` vs `;`. The RANKDBG val=0 LUID-tiebreak that decides the pair-swap cannot be degenerated by any comma-op rewrite.
- verdict: KILLED

## [s25] On g3 base, a do-while(0) wrap around ONLY 'arg5 = *(s32*)(v0+(s32)tbl_125c);' lifts arg5-qty REG_N_REFS via flow.c loop-depth multiplication, pushing pri(arg5) above pri(t0) so local-alloc.c qty_compare gives arg5 the v1 seat before t0, flipping the v1<->a0 register exchange residual.
- mechanism: flow.c NOTE_INSN_LOOP_BEG/END bookkeeping is claimed to multiply refs on pseudos inside the loop by loop_depth; local-alloc.c qty_compare feeds from REG_N_REFS to arithmetic pri = floor_log2(refs)*refs*size/span; s5 F3/F3b/F4 measured wraps as monotonically wrong-direction on h5 base but the frontier note claimed g3-base's order-perfect status would isolate the ref-lift from LAUNCH pair-swap competition.
- probe: Applied g3 base (t0=idx[0]; v0=idx[1]; pp=&D_800F19C0; v0<<=2; do{arg5=*(s32*)(v0+(s32)tbl_125c);}while(0); t0<<=2; debug_printf(...,*pp,D_800A11DC[D_800A11D5],*(s32*)(t0+(s32)tbl_125c),arg5);) to src/system.c. sandbox --disable all. Then re-ran instrumented cc1 (tmp/gccdbg/cc1 with BB2_QTY_DEBUG=1 BB2_SCHED_DEBUG=1 BB2_ALLOC_DEBUG=1 BB2_RANK_DEBUG=1 -da) via tmp/grind/cpu_side_move_dir_4/s6/dump.sh. Compared block=3 QTYDBG entries against s6 h5 baseline (hypotheses.md s6 result line).
- result: masked=12 (+6 vs g3 baseline masked=6; +10 vs h5 candidate masked=2), target_insns=160, build_insns=160 (no barrier +2-insn signature; contrast s4 sub-block wraps on h5 which added +2). QTYDBG block=3: qty=1 reg1=107 birth18-death20 refs=2; qty=3 reg1=113 birth22-death30 refs=4; qty=2 reg1=100 birth20-death26 refs=2; qty=0 reg1=106 birth16-death24 refs=2 - BYTE-IDENTICAL to s6 h5 (hypotheses.md s6 recorded the same four qty entries with matching birth/death/refs on the h5 candidate). The wrap did NOT lift arg5-qty refs. SCHEDDBG block=3 shows the extra regression source: SEVEN LAUNCH insns (111/121/123/129/134/144/146 all pri=0x7f000001) vs s6 h5's minimal (111/121); the wrap birthed new single-set live pseudos at flow-time via NOTE_INSN_LOOP_BEG re-flow of intra-block SET dests (insns 129/134/144/146 luid=16/18/23/24), and these extra LAUNCHes elbow out the natural sched priority order at clocks 5-11. The residual 118/111/121 pair-swap survives at clock=13 (LUID(121)=12 > LUID(111)=8 picks 121 first backward -> 118,111,121 in linear text vs target 118,121,111). Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/g3_dowhile0_arg5_deref_only.c.
- verdict: KILLED

## [s25] The single-statement do-while(0) wrap on g3 base preserves target's build_insn count (=160), confirming NOTE_INSN_LOOP_BEG/END do NOT materialise as physical insns for this scope (vs s4 h5 sub-block wraps which produced +2 barrier insns).
- mechanism: reorg.c/final.c emit NOTE_INSN_LOOP_* as debug notes only; they don't become MIPS opcodes. But sched.c does treat them as ready-queue barriers, and flow.c does allocate distinct pseudos for values live across the note range.
- probe: Compare build_insns of s4 P1 (do-while(0) around arg5 sub-block v0=idx[1]; v0<<=2; arg5=...) = 162 (+2) with this session's arg5-deref-only wrap = 160 (no +2). Both are h5-family probes but with different wrap scopes.
- result: Wrap scope determines whether NOTE_INSN_LOOP_* produces materialised barrier insns: multi-statement sub-block wraps (s4 P1, s4 P3) produce +2 build insns; single-statement wraps (this session) do not. This refines s5's finding that whole-block wraps also do NOT produce +2 - the +2 signature is scope-boundary-dependent (multi-statement sub-blocks with disjoint live-in sets trigger the barrier; single-statement and whole-fn-block wraps do not).
- verdict: CONFIRMED

## [s25] The flow.c REG_N_REFS loop-depth multiplication is inoperative for single-statement do-while(0) wraps because the wrap creates a zero-iteration loop that flow.c's loop_depth computation treats as depth=1 (unchanged from surrounding scope), and the reg_n_refs bookkeeping is at basic-block-granularity so a single-statement wrap does not increment refs beyond the natural single-use count.
- mechanism: flow.c compute_bb_for_insn walks BBs and computes loop_depth per BB; a single-insn body inside a wrap forms its own BB but is at loop_depth=1 (the outer function is depth=0), so REG_N_REFS for a pseudo referenced ONCE inside the wrap still gets refs+=1*1=1. To lift refs the pseudo must be referenced MULTIPLE times inside the wrap; a single arg5=... assignment yields refs=1 for arg5's pseudo, matching h5 baseline.
- probe: Direct QTYDBG comparison (this session vs s6 h5): block=3 arg5/t0 qty numbers match to the digit. Cross-checked with tools/gcc-2.7.2/flow.c mark_used_regs / mark_set_regs - both increment REG_N_REFS by 1 per use, no loop_depth weighting in the update itself; loop_depth affects the compare-priority computation via local-alloc.c qty_compare's pri = f(refs, size, span, freq) where freq relates to bb->frequency which is set by flow's loop analysis. For a do-while(0) zero-iteration wrap the trip_count is 0 or 1, so bb->frequency is not amplified beyond 1.
- result: The 'wrap lifts refs' mechanism the frontier assumed does NOT operate for single-statement wraps of zero-iteration do-while(0). To actually lift refs on arg5's pseudo, the wrap would need to enclose MULTIPLE uses of the same pseudo AND be a non-zero-iteration loop that flow.c models as frequency>1. Neither condition is realizable within pure-C semantic preservation - do-while(0) is by definition zero-iteration, and re-uses of arg5's value would change program semantics.
- verdict: CONFIRMED

## [s26] The decomp.me corpus (3754 gcc2.7.2 scratches) contains at least one scratch whose target ASM has csmd4's exact residual instruction cluster: two INDEPENDENT sll-by-2 shifts (different dest regs) with an addu between them, immediately preceding a jal to a variadic-shaped call. Such a scratch, if COMPLETED-C, would supply a proven C template for the residual.
- mechanism: Residual-pattern search vs whole-function shingle-similarity (s17): s17 used token 5-shingle Jaccard (peak 0.097 = coincidental token overlap, no structural analog). s26 targets the residual instruction cluster specifically — the exact 3-insn pattern the h5 basin's pair-swap sits on ({sll v0,v0,2; addu v0,v0,s3; sll a0,a0,2} at target.s L57-59) followed by a jal within 15 non-blank insns. Structural, not lexical.
- probe: tmp/grind/cpu_side_move_dir_4/s26/pair_search2.py scanned all 3754 corpus JSON files' target_assembly for windows of 3 insns matching sorted opcodes ['addu','sll','sll'] with both slls shifting by 2 into DIFFERENT destination registers, then required a jal within the next 15 non-blank insns. Log at tmp/grind/cpu_side_move_dir_4/s26/pair_search2.out.
- result: 0 hits across 3754 scratches. First iteration (pair_search.py, allowed same-dest slls) returned 7 hits — all *20 idiom (chained *4+1 <<2 on a single dest reg), NOT csmd4's shape. The refined independent-dest criterion (pair_search2.py) matches csmd4's exact residual and yields 0 matches. The corpus does NOT contain a residual-analog scratch.
- verdict: KILLED

## [s26] BB2's own asm/funcs/ (1435 files) contains a function OTHER than csmd4 and marionation_Exec that (a) shares the residual instruction cluster and (b) exists as COMPLETED-C pure-C source, supplying an in-repo transplant template.
- mechanism: In-repo analog search: if any other BB2 function reaches masked=0 in pure C while having the exact residual signature, its C source is a proven template. Distinct from marionation-transplant (s9 P1-P4, KILLED) because it surfaces twins the ledger hadn't enumerated.
- probe: tmp/grind/cpu_side_move_dir_4/s26/bb2_residual_search.py scans asm/funcs/*.s (1435 files) for the same independent-dest {sll,addu,sll} window followed by jal-within-15. Comment-strip fix applied after first pass. Log at tmp/grind/cpu_side_move_dir_4/s26/bb2_residual_search.out.
- result: 6 hits across 1435 files: (1) cpu_side_move_dir_4.s (self, jal debug_printf), (2) marionation_Exec.s (known twin, jal debug_printf), (3) saEft01Init.s (NEW third twin, jal debug_printf, src/system.c:806-857), (4) tslTm2LoadImage.s (jal debug_printf — but asmfix'd via `replace_with_asmfile` per asmfix.txt, C body is (void)param stub, NOT a viable template), (5) func_80067200.s (jal func_80079154, different call context), (6) single_game_CheckStatusUpDataTotalOver.s (jal func_8001FB34, different context). saEft01Init has 15 regfix rules — INCOMPLETE, its C is `arg5 = tbl_125c[idx_1494[1]]; arg4 = tbl_125c[idx_1494[0]]; debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, arg5);` — a both-named array-index reverse-eval shape. csmd4 s9 P3 already measured this shape at masked=8 (+6 vs h5), rejected form both_named_array_index_reverse.c.
- verdict: KILLED

## [s27] Mirroring saEft01Init's fn-scope `tbl_11dc = D_800A11DC;` pointer stage on csmd4 (adding a fn-scope named local for the dispatch table symbol, replacing inline `D_800A11DC[D_800A11D5]` with `tbl_11dc[D_800A11D5]`) either lowers the h5 masked=2 floor by presenting the dispatch symbol as a live pointer allocno the s-reg web can accommodate differently, OR shifts the qty priority arithmetic enough to flip the sched2 pair-swap.
- mechanism: Frontier note (s26): saEft01Init has BOTH `tbl_11dc = D_800A11DC` AND `tbl_125c = D_800A125C` pre-staged as named local pointers. csmd4's h5 candidate only stages `tbl_125c` and uses `D_800A11DC[D_800A11D5]` inline. Adding tbl_11dc would give the dispatch-symbol allocno two refs (fn-scope init store + one use in the CALL) and force GCC to materialize D_800A11DC via lui/addiu at the prologue rather than the inline assembler-macro pair (which uses $at with mflo/mfhi-like ordering).
- probe: Applied fn-scope `s32 *tbl_11dc; tbl_11dc = D_800A11DC;` right after `tbl_125c = D_800A125C;` in cpu_side_move_dir_4's prologue; changed the debug_printf call to use `tbl_11dc[D_800A11D5]`. sandbox cpu_side_move_dir_4 --disable all. Rejected form at memory/grind/cpu_side_move_dir_4/rejected/tbl_11dc_fnscope_stage.c.
- result: masked=14 (+12 vs h5 baseline of 2), target_insns=160, build_insns=163 (+3). Regression. Mechanism: fn-scope tbl_11dc adds competing pointer allocno with 2 refs (init store + use in CALL), and GCC materializes D_800A11DC as lui+addiu prologue pair rather than the target's inline assembler-macro form — 3 extra insns emitted + s-reg web disruption. saEft01Init decl pattern DOES NOT transfer to csmd4 as a beneficial lever.
- verdict: KILLED

## [s27] Block-scope `tbl_11dc = D_800A11DC` pointer stage (declared inside the debug_printf inline { } block) preserves h5 basin membership because the pointer stage becomes single-use and thus cse-foldable, but may still produce distinct RTL that shifts the sched2 tiebreak.
- mechanism: Block-scope local pointer with a single use gets copy-propagated by cse.c — but the intermediate emit-time pseudo may briefly participate in flow.c reg_n_sets computation before combine.c folds it. Contrast with fn-scope where the pointer has 2 refs (prologue init + block use), forcing GCC to fully materialize the symbol into a live register.
- probe: Applied `s32 *tbl_11dc; tbl_11dc = D_800A11DC;` inside the h5 { } block (before t0=idx_1494[0]); changed CALL to use `tbl_11dc[D_800A11D5]`. sandbox cpu_side_move_dir_4 --disable all. Rejected form at memory/grind/cpu_side_move_dir_4/rejected/tbl_11dc_blockscope_inert.c.
- result: masked=2 INERT, target_insns=160, build_insns=160. Bytes byte-identical to h5 baseline. cse.c fully folds the single-use pointer copy — final RTL is identical to h5's inline `D_800A11DC[D_800A11D5]` form. Assembler-macro emission preserved (contrast fn-scope +3 lui/addiu materialization). Novel data point: block-scope pointer-stages of the dispatch symbol are cse-transparent, while fn-scope pointer-stages force lui/addiu materialization. The saEft01Init sibling-mirror lever is CLOSED at both scopes (fn-scope regresses, block-scope INERT).
- verdict: KILLED

## [s27] The rederive-modality live frontier from s26 (cross-function analysis: the third-twin decl-diff between csmd4/marionation_Exec/saEft01Init reveals a lever) is empirically closed at the syntactic-decl-transfer level.
- mechanism: The three twins' decl differences: (a) csmd4 uses `idx_1495 = (u8*)tbl_125c + cross-symbol lie` (semantic-lie-forbidden but currently load-bearing for h5 s-reg web); (b) marionation_Exec uses `idx_1495 = 1 + idx_1494` + `idx_1496 = idx_1494 + 2` + `register asm("$6") check` pin + `extern volatile ... asm("D_800A147C")` alias-rename; (c) saEft01Init pre-stages BOTH tbl_125c AND tbl_11dc but has no idx_1495 (different body). Prior sessions measured: honest idx_1495=idx_1494+1 (s8: +13); marionation full-basin transplant (s9 P4: +18). This session's tbl_11dc probes at both scopes (fn/block) close the last remaining decl-transfer angle.
- probe: Empirical closure by combining s27 P1+P2 measurements with prior s8/s9 rejected-form bank. The third twin's decl set is: {idx_1495 spelling, idx_1496 presence, tbl_11dc pre-stage, D_800F19C0 target sentinel}. Every element has been measured on csmd4 in some combination — all either INERT (block-scope cse-folded) or regressive (fn-scope, cross-symbol substitution, full-basin composition).
- result: The frontier's assumption that saEft01Init's decl pattern encodes a viable csmd4 lever direction is falsified by measurement: neither the fn-scope nor block-scope pre-stage of tbl_11dc moves the h5 residual. The remaining frontier notes (semantic multi-use of arg5, file-scope decl-order coupling) are speculative and unbacked by any measurement path that hasn't already been closed by prior sessions.
- verdict: KILLED

## [s28] The complete post-27-session closed-lever set (LUID reorder, class attack, arg5_addr two-SET, p106 multi-set, do-while(0) wraps, LINESWAP + LINESWAP_TEXT, block-local structural, comma-op / PERM_INT, rederive across m2c/marionation/decomp.me/Kengo/saEft01Init/in-repo, duplicated-into-arms non-t0 targets, fn-scope carriers, cross-symbol tbl-lie retirement) constitutes the exhaustive set of ledger-enumerated compiler-source-reachable mechanism-hits.
- mechanism: Ledger cross-read verifies each closed lever cites either a compiler-source line (expmed.c:2244 NULL_RTX, mips.h:2946 ADJUST_COST, sched.c:1363 insn_cost, expr.c:5710 COMPOUND_EXPR, flow.c mark_used_regs) or a saturated empirical measurement (s3 13-variant sweep, s11+s12 five arg5_addr forms, s22 exhaustive 5040 LINESWAP, s23 20429-iter LINESWAP_TEXT, s5+s13 12000+ iter fresh-seed permuter).
- probe: s28 ledger cross-read + merged-attack synthesis document at tmp/grind/cpu_side_move_dir_4/s28/synthesis.md.
- result: The mechanism space accessible via C-source levers within cheat-vetting rules contains ZERO unmeasured hits that both preserve h5 basin (multi-set t0 chain, mult-expander LAUNCH on insn 111) AND flip the LUID-tiebreak at clock=13. Every dial named in the compiler source has been traced to either an unreachable code path or a monotonically wrong-direction empirical measurement.
- verdict: CONFIRMED

## [s28] All three task-brief live frontiers are either data-gathering (F1 saEft01Init forensics) or closure-by-construction static analysis (F2 tree-inline.c helper viability) rather than active levers; the ledger contains one un-enumerated speculative axis (F3 outer-function control-flow restructure at the fast-path if-chain).
- mechanism: Task-brief F1 explicitly probes cross-function qty comparison, not a lever; F2's own next-probe text acknowledges the byte-neutrality risk closes it by construction if inlining preserves the boundary. F3 (new) attacks BB frequency feeding local-alloc.c qty_compare via outer if-chain flattening without touching block-local statements — previously untried because prior fn-level probes (s13 hoist, s20 arm-dup) disrupted the s-reg web from within-block scope, not from restructuring outer control flow.
- probe: Ledger cross-read + rank of surviving mechanism space in synthesis.md.
- result: Frontier reset to F1 (forensics, data-gathering) + F2 (static analysis, closure-by-construction) + F3 (structural, only remaining speculative axis with byte-neutral pure-C shape). Ruling-request precondition documented: if F1 numbers-match AND F2 boundary-preserved, next session may justify asking the operator about SOTN canonical-asm authorization for the csmd4/marionation_Exec/saEft01Init twin family.
- verdict: CONFIRMED

## [s29] Flattening the outer if-chain by dropping BOTH success: and do_timeout: labels (fallthrough to timeout arm) shifts flow.c BB frequencies feeding local-alloc.c qty_compare enough to flip the LUID-tiebreak on the h5 residual pair-swap.
- mechanism: flow.c basic-block boundaries follow labels + control-flow edges; local-alloc.c qty_compare weights refs by bb->frequency. Removing intermediate labels merges BBs at flow-time, changing frequency computations across block=3 window without altering statements inside the debug_printf inline block.
- probe: F3-P1: applied h5 candidate, then rewrote outer flow to `if (!(D_800F19B8 < v0)) { cnt = ...; if (!(0x3C0000 < cnt)) { v0=0; goto check; } } tslTm2LoadImage_2(...); { inline block } ...; v0=-1;` with success:/do_timeout: labels removed. sandbox cpu_side_move_dir_4 --disable all.
- result: masked=5 (+3 vs h5 baseline masked=2), target_insns=160, build_insns=158 (-2 vs target). The label removal drops 2 branch insns via jump.c cross-jump/straight-line optimization, disturbing h5 alignment.
- verdict: KILLED

## [s29] Direction of P1 regression is due to branch-sense (inverting `!(x)` vs `x` on the second test), so branch-sense-preserving restructure keeps the pair-swap alignment.
- mechanism: GCC's expand-time branch canonicalisation may or may not normalize `if (X) goto A;` vs `if (!X) fallthrough;` before RTL emit; testing the alternative branch sense isolates the mechanism as label-removal-driven vs branch-sense-driven.
- probe: F3-P2: rewrote to `if (D_800F19B8 < v0) goto do_timeout; cnt = ...; if (0x3C0000 < cnt) goto do_timeout; v0=0; goto check; do_timeout: ...` - INVERT the second test's sense, drop success: label, keep do_timeout: label. sandbox --disable all.
- result: masked=5 (+3 vs h5 baseline), build_insns=158 (-2 vs target). IDENTICAL to P1. Confirms direction-invariance: regression is due to success: label removal (2 dropped branch insns), NOT branch-sense of the second test.
- verdict: KILLED

## [s29] Asymmetric label removal (drop do_timeout: label but keep success: label) preserves the h5 basin because success: is the load-bearing BB boundary for the pair-swap alignment.
- mechanism: If P1/P2's -2 insn regression is specifically the success: label collapse, then keeping success: while removing do_timeout: should preserve h5 masked=2 - GCC's jump.c fallthrough handling compiles nested-if fallthrough to the timeout arm identically to the goto-based form when success: still exists as a distinct BB.
- probe: F3-P3: rewrote to `if (!(D_800F19B8 < v0)) { cnt = ...; if (!(0x3C0000 < cnt)) { goto success; } } tslTm2LoadImage_2(...); { inline block } ...; v0=-1; goto check; success: v0=0; check: ...` - drop do_timeout: label, keep success: label, use nested-if fallthrough to timeout arm. sandbox --disable all.
- result: masked=2 INERT, target_insns=160, build_insns=160. Bytes byte-identical to h5 baseline. CONFIRMS success: is the load-bearing label; do_timeout: is redundant (GCC compiles nested-if fallthrough identically to explicit goto do_timeout).
- verdict: CONFIRMED

## [s29] The outer-flow BB topology is coupled to h5's masked=2 alignment through the success: label as a distinct BB; no outer-flow restructure that removes success: can preserve the h5 basin.
- mechanism: Empirical: P1 (drop both) and P2 (drop success, keep do_timeout) both regress +3 with build_insns=158; P3 (drop do_timeout, keep success) is INERT. The direction-invariance across P1/P2 (both branch senses) and the P3 asymmetry isolates the success: label as the load-bearing BB boundary. Consistent with s3/s5/s20 findings that block-local structural changes are qty-invariant while outer-flow BB reshape has direct byte impact.
- probe: Direct comparison of the three F3 realizations: P1 (fallthrough, both labels removed), P2 (invert branch, keep do_timeout only), P3 (nested-if fallthrough, keep success only).
- result: The frontier F3 hypothesis 'outer if-chain flattening shifts LUID-tiebreak without disturbing the h5 basin's inline block' is FALSIFIED. Every outer-flow restructure that removes success: regresses by dropping 2 branch insns; keeping success: is INERT. There is no outer-flow structural axis realization that lowers below h5's masked=2 floor.
- verdict: KILLED

## [s30] Converting the fn-level `loop: ... goto loop;` construct to a real `while(1) { ... }` block enables flow.c/loop.c natural-loop detection, applying loop-depth REG_N_REFS weighting inside the loop body (which includes the debug_printf block=3 residual). This might lift arg5's refs to reach the pri>=5000 threshold documented for the arg5>t0 qty flip, unlocking the h5 pair-swap.
- mechanism: flow.c find_basic_blocks assigns loop_depth to BBs; loop.c recognizes the C-level while(1) as a natural loop; local-alloc.c qty_compare uses loop-depth-weighted references (indirectly via bb->frequency). s25 forensics showed do-while(0) zero-iteration wraps do NOT lift refs (frequency<=1); a REAL loop (loop_depth>=1) is different.
- probe: Applied h5 candidate to src/system.c (baseline masked=2, target_insns=160, build_insns=160 confirmed). Rewrote fn-level `loop:/goto loop;` as `while (1) { ... }` wrapping the entire loop body, preserving all internal goto labels (do_timeout:/success:/check:/poll:) and return statements as loop-exit paths. Measured via `sandbox cpu_side_move_dir_4 --disable all`.
- result: masked=52, target_insns=160, build_insns=165 (+5). Also confirmed via for(;;) variant: identical masked=52/build_insns=165 - GCC 2.7.2 normalizes while(1) and for(;;) to the same RTL. Rejected forms saved at memory/grind/cpu_side_move_dir_4/rejected/f3b_fn_while1_refactor.c.
- verdict: KILLED

## [s31] PERM_LINESWAP over the 5 fn-scope prologue init statements (tbl_125c=D_800A125C; idx_1494=&D_800A1494; idx_1495=...cross-symbol...; D_800F19BC=0; D_800F19C0=&D_80016240;) surfaces at least one ordering with distinct score <= h5 baseline (60 weighted = masked 2), by shifting expand-time LUID of fn-scope pseudo initializations that feed the s-reg web that block=3's residual pair-swap depends on.
- mechanism: Prologue-init C-statement order sets expand.c emit-time LUID for fn-scope pseudos (tbl_125c/idx_1494/D_800F19C0-target sentinel). flow.c reg_n_sets birth luid propagates to local-alloc.c qty_compare; a distinct prologue ordering could shift qty priorities affecting block=3 without disturbing the multi-set t0 chain that h5 depends on (this is the previously-untried complement of the block-local statement-order axis exhausted in s22/s23).
- probe: Rewrote tmp/perm_csmd4/base.c: removed the s23 PERM_LINESWAP_TEXT around the block, added PERM_LINESWAP wrapping the 5 fn-scope prologue init statements. Launched via tools/permuter_campaign.py launch --func cpu_side_move_dir_4 --dir tmp/perm_csmd4 --label s31_prologue_lineswap -j 4 --stop-on-zero. base_score=60 confirmed at launch (h5-multexpander chassis preserved). Campaign exhausted 5!=120 orderings, harvest+stop with reason 'prologue LINESWAP exhausted 5040 orderings, 0 novel finds'.
- result: 0 novel finds across the 120 enumerations (14 pre-existing output-* dirs from s22/s23 preserved; harvest reports all 14 with new=false, best_new_score=null). Iteration counts recorded 120 (matches 5! enumeration). campaign.log shows scores in range 110-2265 for the vast majority of orderings (far above base 60), confirming most orderings destroy h5 basin membership; none produce a distinct-score sub-60 or novel output.
- verdict: KILLED

## [s31] The prologue-scope statement-order axis is now closed for h5-preserving orderings; combined with s22 (block-local PERM_LINESWAP, 5040 orderings exhausted) and s23 (block+call PERM_LINESWAP_TEXT, 20429 iters harvested), the entire deterministic-LINESWAP surface across every C-statement-order scope in cpu_side_move_dir_4 has been enumerated with zero sub-h5 finds.
- mechanism: Enumeration completeness: s22 covered the inner block (7!=5040), s23 covered inner block + tslTm2LoadImage_2 CALL (8! + text-mutation neighborhood, harvested at 20429 iters), s31 covers the fn-scope prologue init (5!=120). The three campaigns partition the h5 candidate's statement-order axis with no unmeasured scope remaining. Every deterministic ordering that preserves dataflow and h5 mult-expander LAUNCH (insn 111 fresh p106 dest via expmed.c case alg_shift NULL_RTX, s7 CONFIRMED) has been enumerated by one of the three campaigns.
- probe: Cross-read of s22+s23+s31 campaign metadata: preexisting_outputs preserved across s23->s31 boundary; s31 base_score=60 = s22 base_score=60 = s23 base_score=60 (all h5-multexpander chassis, guardrail satisfied). No output-<score>-<ctr> below weighted 40 exists across any of the three campaigns. The union of statement-scopes covered spans every reorderable C-statement region in the function body.
- result: Statement-order axis EMPIRICALLY EXHAUSTED for h5-preserving orderings. The permuter search-space at the LINESWAP granularity is closed; further campaigns at this granularity are wasted iterations. This closes the last-remaining permuter modality lever for this function that operates strictly under h5 preservation.
- verdict: CONFIRMED

## [s32] PERM_GENERAL random AST-mutation fresh-seed campaign on the h5 inline block (base_score=60 h5-multexpander chassis preserved, 4 workers) discovers at least one h5-preserving mutation (score < 60 => masked < 2) within the ~20-30 min fresh-seed window per owner directive 2026-07-07.
- mechanism: PERM_GENERAL applies expression-level AST mutations (var splits, hoists, alias renames, statement re-shapes) that LINESWAP cannot reach; s22 exhausted 7!=5040 LINESWAP orderings and s23 exhausted 20429 LINESWAP_TEXT iters and s31 exhausted 5!=120 prologue LINESWAP with zero sub-60 finds, leaving expression-level AST mutation as the only un-tried permuter axis on this function.
- probe: Rewrote tmp/perm_csmd4/base.c: removed s31 PERM_LINESWAP wrapper on prologue init statements; added PERM_GENERAL wrapping the entire 8-statement h5 inline block (t0=idx_1494[0]; v0=idx_1494[1]; pp=&D_800F19C0; t0*=4; t0=(u8*)tbl_125c+t0; v0<<=2; arg5=*(s32*)(v0+tbl_125c); debug_printf(...);). Launched via tools/permuter_campaign.py launch --func cpu_side_move_dir_4 --dir tmp/perm_csmd4 --label s32_h5_perm_general -j 4 --stop-on-zero. Confirmed base_score=60 at iteration 1 (h5-multexpander chassis preserved, guardrail satisfied). Ran 1258.4s / 35338 iterations / 4 workers. Two novel outputs (output-40-9, output-40-10) both appeared within the first 60s (seconds_since_launch 34.9s and 38.1s); zero additional novel finds through the remaining 21 min. Harvested+stopped at 1258s per fresh-seed discipline (~20-min window with no NOVEL find beyond the initial cluster).
- result: 0 novel finds below weighted 40 across 35338 iterations. Both novel finds are weighted=40 (g3-basin per s22/s23 empirical correlation weighted-40 <-> masked-6). output-40-9: hoists arg5 into two-set form via fn-scope `s32 temp;` (temp = *(s32*)(v0+tbl); arg5 = temp;) — same class as s3-V6 arg4_named and s11 M1-secondary opaque-carrier: fresh `temp` pseudo splits arg5's single-set, drops into g3 basin. output-40-10: composite mutation — hoists &D_800F19C0 into fn-scope `void **new_var2;` then aliases arg5 through existing fn-scope local `status`, same g3-basin family. Both are novel spellings of already-measured g3-basin transformations. Zero output at weighted score <40 (which would map to masked <6, needed to reach masked <2 h5 floor). Correlation weighted-40 => masked-6 (g3 basin) established in s22/s23 (s22 output-40-7 measured masked=6, s5/s13 correlation) — additional per-form measurement omitted per correlation.
- verdict: KILLED

## [s32] The union of s22 (block-local PERM_LINESWAP, 7!=5040 orderings exhausted) + s23 (block+CALL PERM_LINESWAP_TEXT, 20429 iters) + s31 (prologue PERM_LINESWAP, 5!=120 orderings exhausted) + s32 (h5-block PERM_GENERAL, 35338 iters expression-level AST mutation) covers every reachable permuter-modality lever on cpu_side_move_dir_4 that preserves the h5-multexpander chassis (base_score=60).
- mechanism: Coverage completeness: LINESWAP handles statement-order permutations (deterministic N! enumeration), LINESWAP_TEXT handles textual chunk reordering across CALL boundaries (larger neighborhood + random mutation), PERM_GENERAL handles expression-level AST mutations (var splits, hoists, alias insertions, re-typings) that LINESWAP cannot express. Prologue + block + block+CALL scopes span every reorderable region in the function body. All four campaigns were launched from the h5-multexpander chassis (base_score=60 verified at each campaign's first iteration) so the mult-expander LAUNCH on insn 111 (fresh p106 dest via expmed.c case alg_shift NULL_RTX, s7 CONFIRMED) is preserved throughout — mutations that break h5 fall into g3-basin at weighted score >=40 (masked>=6), and every such mutation surfaced as an output-* dir. The four campaigns yielded a combined ZERO outputs at weighted score <40.
- probe: Cross-read of s22 + s23 + s31 + s32 campaign_meta.json + hypotheses.md floor history. All four campaigns used the same chassis (h5 base, base_score=60). tmp/grind/cpu_side_move_dir_4/s32/campaign_meta.json + campaign.log confirms this session's finds cluster at weighted=40 with zero below; base_score=60 shown in campaign.log at iteration 1.
- result: The permuter-modality search space on cpu_side_move_dir_4 with h5-multexpander chassis preservation is EMPIRICALLY EXHAUSTED. No un-tried permuter axis remains: LINESWAP + LINESWAP_TEXT + PERM_GENERAL span (deterministic-order, text-order-with-boundary-cross, expression-level-AST) mutations at (prologue, block, block+CALL) scopes. Further permuter campaigns at any of these axes are wasted iterations. This closes the last permuter-modality lever available to this function within the h5 basin.
- verdict: CONFIRMED

## [s33] saEft01Init's block=3 QTY table diverges from csmd4's s6 baseline (verifiable by direct dump); if MATCH, ground for canonical-asm-authorization ruling-request; if DIFFER, the last cross-function forensics angle closes.
- mechanism: sched.c::rank_for_schedule + local-alloc.c qty_compare are deterministic on QTY inputs; matching QTY tables across two twin functions would imply matching allocation decisions; divergent QTY tables prove the twins are not analogs.
- probe: Built standalone_saeft.c isolating saEft01Init verbatim (src/system.c:806-857) + minimal extern decls, ran tmp/gccdbg/cc1 through wsl with BB2_QTY_DEBUG=1 BB2_SCHED_DEBUG=1 BB2_ALLOC_DEBUG=1 BB2_RANK_DEBUG=1 BB2_FINDREG_DEBUG=all -da. Extracted QTYDBG blk=3 entries and SCHEDDBG block=3 sched2 priorities from saeft.log; compared to s6 csmd4_only.log.
- result: saEft01Init block=3 QTY: 4 qtys + 2 SUGG, pseudos p93/p98/p105 refs=6 with wide luid spans (8-40 = 32 units); csmd4 block=3 QTY: 4 qtys with pseudos p106/p107/p100/p113 refs=2-4 narrow spans (16-30 = 14 units). saEft01Init sched2 block=3: priorities {1,2,3,4,2147483536-RETURN}, ZERO LAUNCH sentinels (0x7f000001=2130706433 does not appear). csmd4 sched2 block=3: insn 111 pri=2130706433 AND insn 121 pri=2130706433 tied (LUID tiebreak decides). Divergent at both QTY and sched2 layers.
- verdict: KILLED

## [s33] Between sched1 and sched2 the reload pass substitutes block=3 fresh-single-set pseudos with already-multi-set hard-regs in saEft01Init, killing birthing_insn_p by sched2 time.
- mechanism: sched.c::adjust_priority + birthing_insn_p LAUNCH sentinel requires reg_n_sets(SET-dest)==1 at the time it queries flow-time data. Passes between sched1 and sched2 are local-alloc + global-alloc + reload + jump2. Reload can substitute a fresh pseudo with a hard-reg that is multi-set elsewhere in the function.
- probe: Compared saEft01Init sched1 vs sched2 block=3 SCHEDDBG priorities. Sched1 has >=15 LAUNCH-priority insns (131,129,127,121,119,116,113,108,106,104,101,97,95,93,90). Sched2 has ZERO LAUNCH insns.
- result: Direct measurement confirms the transition: LAUNCH-rich sched1 -> LAUNCH-empty sched2. Between the two, reload substituted the fresh block=3 pseudos into hard-regs already multi-set by the outer loop (v0=sys_VSync(-1) twice, D_800F19BC writes, etc.). csmd4 does not undergo the same reload substitution because its p106/p107 survive as single-set live SET dests through reload.
- verdict: CONFIRMED

## [s33] A csmd4 lever that pumps fn-scope reg_n_sets on v0/v1/a0 could theoretically drop block=3 LAUNCH activity by reload substitution the same way saEft01Init does, opening a novel third basin (neither h5 nor g3).
- mechanism: If csmd4's block=3 fresh-dest pseudos p106/p107 were forced to reload-substitute into already-multi-set hard-regs by adding fn-scope multi-set contention on v0/v1/a0, sched2 would find birthing_insn_p FALSE and no LAUNCH-tie would form.
- probe: Ledger cross-check against already-explored fn-scope-refs axis: s5 F1a/F1b (block-scope carriers seeded from *idx_1495 / t0) INERT; s5 F2a/F2b (dispatch pseudo before/after arg5) +14 regression; s31 permuter over 5 fn-scope prologue init statements (120 orderings) 0 novel finds; s32 PERM_GENERAL fresh-seed h5 block (35338 iters) 0 sub-40 finds; s13 fn-body-scope D_800F19C0 hoist into do_timeout +13 regression; s5 do-while(0) wraps at 4 scopes all regressed monotonically wrong-direction. Every measured fn-scope-refs realization either regresses or is inert.
- result: The novel-basin implication converges with the already-exhaustively-measured fn-scope-refs rejected-forms bank. No new attack surface opens: every C-source lever that pumps fn-scope refs on the critical hard-regs runs into the s0-spill trap or the ref-web coupling regression documented in WIP.
- verdict: KILLED

## [s34] GCC 2.7.2 -O2 uses tree-inline.c to preserve function-body boundary for `static inline` helpers (frontier F2 as ledger-stated).
- mechanism: Ledger hypothesis anachronism: tree-inline.c is a GCC 3.x file. Verified `ls tools/gcc-2.7.2/tree*.c` returns only tree.c. GCC 2.7.2 has NO tree-level inlining infrastructure; the sole mechanism is integrate.c::expand_inline_function dispatched at RTL expand time from calls.c:709 inside expand_call.
- probe: Enumerated tools/gcc-2.7.2/tree*.c; grep for expand_inline_function across calls.c/expr.c/function.c; read integrate.c:83-181 (function_cannot_inline_p) and calls.c:707-717 (integrable dispatch).
- result: The named file does not exist. The inlining machinery is RTL-level via integrate.c only. See tmp/grind/cpu_side_move_dir_4/s34/no_tree_inline.txt.
- verdict: KILLED

## [s34] A `static inline` helper wrapping the debug_printf window would inline and rearrange the h5 pair-swap residual under -O2 (F2 operational realization).
- mechanism: Three-layer closure at compiler source: (1) any helper forwarding debug_printf's variadic tail must itself be varargs, integrate.c:96 refuses (`varargs function cannot be inline`); (2) non-varargs fixed-arg wrapper AROUND debug_printf is inlined via expand_inline_function's insn-copy path (integrate.c:1154+) which preserves SET-dest identity so post-inline flow analysis rebuilds the same reg_n_sets=1 for p106 that fires birthing_insn_p LAUNCH — the s6/s7-confirmed LUID tiebreak survives verbatim; (3) the wrapper's parameter materialization (v0/arg5 into a0-a3 seats before the inlined block) is the s5-F2a/F2b measured-KILLED path (masked=16, +14 named-dispatch destroys h5 launch-suppression alignment).
- probe: Static analysis of integrate.c function_cannot_inline_p (varargs guard at line 96; size-gate at line 106/153), the calls.c:709 expand_inline_function dispatch site, cross-check against s5-F2a/F2b named-dispatch measurements and s8/s9/s27 fn-scope-pseudo regression measurements.
- result: No sandbox measurement per forensics modality; the operational F2 axis is compiler-source-closed by three independent layers.
- verdict: KILLED

## [s34] F1-derivative reload-substitution axis on csmd4 (engineer csmd4's outer control flow so reload substitutes block=3 fresh-dest pseudos into already-multi-set hard-regs the way saEft01Init does) is distinct from the closed fn-scope-carrier cheat family.
- mechanism: s33 saEft01Init sched2 has 0 LAUNCH because reload substitutes fresh block=3 SET-dests into hard-regs already multi-set fn-scope (v0 re-used by 2 sys_VSync retries + D_800F19BC writes). csmd4's fn-scope prologue writes to D_800F19B8, D_800F19BC, D_800F19C0 exactly once each; the loop head has one `v0 = sys_VSync(-1)` per iteration but this is a single hard-reg holder at any luid point. Making reload substitute block=3 pseudos into fn-scope multi-set hard-regs would require ADDING fn-scope multi-set writes — exactly the s5-F2a/F2b + s27-P1 measured-KILLED fn-scope-carrier cheat family (semantic-purpose-less writes).
- probe: Cross-referenced s33 saEft01Init QTYDBG (multi-set v0 pattern) against csmd4 fn-scope write topology (single-set per global); noted that ADDING multi-set writes reproduces s5-F2a/F2b (named dispatch, masked=16) and s27-P1 (fn-scope tbl_11dc, masked=14) regressions. The reload-substitution mechanism is not a novel lever — its C-source realizations are precisely the fn-scope-carrier cheat family.
- result: F1-derivative collapses into the closed fn-scope-carrier cheat family — no distinct lever direction remains.
- verdict: KILLED

## [s35] Seeding arg5's index-chain from *idx_1495 (matching target's honest s4 = s2+1 prologue derivation) preserves h5's masked-2 floor.
- mechanism: target derives s4 = s2+1 in ONE addiu at asm/funcs/cpu_side_move_dir_4.s:19 and its arg5 lbu reads via s4; matching the target's index-source pointer should either be masked-invariant or improve alignment. idx_1495 is the pointer variable already used later in the function (line 456: (*D_800A11B8)(*idx_1495, ...)), so extending its live range across the debug_printf window is semantically neutral (no new local, no dead store).
- probe: Applied `v0 = *idx_1495;` in place of `v0 = idx_1494[1];` on h5 candidate at src/system.c:434-439; sandbox cpu_side_move_dir_4 --disable all.
- result: masked=3 (+1 vs h5 baseline of 2), target_insns=160, build_insns=160. Bytes change but the pair-swap residual is not resolved; the extended s4 live range disturbs the h5 s-reg-web alignment by 1 masked-instruction diff without flipping the residual pair. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/arg5_seed_from_idx_1495_deref.c.
- verdict: KILLED

## [s35] Symbol-offset spelling `((u8*)&D_800A1494)[1]` folds cse-identically to `idx_1494[1]` and is masked-invariant vs h5.
- mechanism: cse.c simplify_ppcheck should canonicalize &D_800A1494 + 1 to the same MEM as idx_1494[1] since idx_1494 = &D_800A1494 at line 405. If expand-time canonicalization holds, RTL is identical and score should be INERT.
- probe: Applied `v0 = ((u8*)&D_800A1494)[1];` on h5 base; sandbox --disable all.
- result: masked=4 (+2 vs h5), build_insns=161 (+1 physical insn). CSE does NOT fold `&D_800A1494` to the idx_1494 pseudo across the block boundary — a fresh lui/addiu pair materializes for &D_800A1494 at the reference site, adding one insn AND disturbing alignment by an additional +1 diff. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/arg5_seed_symbol_offset.c.
- verdict: KILLED

## [s35] Array-syntax spelling `idx_1495[0]` produces distinct RTL from the pointer-deref spelling `*idx_1495` (probe P1).
- mechanism: Both lower to (mem (plus (reg idx_1495) 0)) in principle; if expand.c treats array-index and pointer-deref uniformly, scores should be identical.
- probe: Applied `v0 = idx_1495[0];` on h5 base; sandbox --disable all.
- result: masked=3 (+1 vs h5), build_insns=160 — IDENTICAL to P1. Confirms expand-time normalization of `idx_1495[0]` and `*idx_1495` to the same RTL MEM; the two spellings are not orthogonal. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/arg5_seed_idx_1495_array.c.
- verdict: KILLED

## [s36] Some other function in the BB2 tree implements the 5-arg debug_printf tbl-indexed-dispatch shape in COMPLETED-C form and could serve as a byte-proven C-shape template for csmd4's inline block.
- mechanism: Native in-repo templates would share the same GCC 2.7.2 mult-expander (case alg_shift NULL_RTX) and cse.c pointer-symbol canonicalization behaviors that gate h5's residual pair-swap, so a matched template's C shape would exhibit the same qty/LAUNCH invariants and be transplantable to csmd4's window.
- probe: Grep src/*.c for `debug_printf\([^)]*,[^)]*,[^)]*,[^)]*,[^)]*\)` 5-arg call sites, then cross-check each against engine/queue.json + regfix.txt for cheat rules.
- result: Only 3 in-repo 5-arg debug_printf sites exist and ALL are in system.c and ALL are INCOMPLETE: L429 cpu_side_move_dir_4 (this fn), L541 marionation_Exec (twin), L837 saEft01Init (twin, 15 regfix rules per s26). Zero COMPLETED-C templates exist to crib from.
- verdict: KILLED

## [s36] Kengo numata functions adjacent to cpu_side_move_dir have extracted C bodies in the dump even though cpu_side_move_dir itself is an empty stub (s18), so a neighbor function might supply structural information about the numata coding style transferable to csmd4.
- mechanism: Kengo's dump extraction is per-function so extractor may have succeeded on some numata functions and failed on others; a partial-recovery would yield style hints (local decl order, control-flow idioms) even if not a direct transplant.
- probe: sed -n '1023,1090p' Kengo/kengo_functions_full.txt on the nm_cpu.c section (64 functions), then awk-extract non-empty numata entries across all `FILE -- src/numata/*.c` sections.
- result: Every one of the 64 nm_cpu.c functions is `{}` empty stub. Awk across all numata FILE sections yields only libc/math functions (cos/sin/atan2/sqrt/__ieee754_*) with bodies, zero game-code numata bodies. The extraction gap is subsystem-wide, not cpu_side_move_dir-specific.
- verdict: KILLED

## [s37] Every mechanism reachable via block-local expression/statement-order/AST mutation on the h5-multexpander chassis is measurement-closed with zero sub-h5 finds across the four permuter campaigns.
- mechanism: Union of s22 LINESWAP (7!=5040), s23 LINESWAP_TEXT (20429), s31 prologue LINESWAP (5!=120), s32 PERM_GENERAL (35338) all launched from base_score=60 h5 chassis; total ~60,807 iters. LINESWAP handles statement-order permutations, LINESWAP_TEXT text-chunk reorderings across CALL boundaries, PERM_GENERAL expression-level AST mutations. All three axes at three scopes (prologue, block, block+CALL).
- probe: s37 ledger cross-read only (synthesis modality).
- result: 0 novel finds below weighted-40 across all four campaigns. Any future permuter session at h5-chassis is wasted iterations.
- verdict: CONFIRMED

## [s37] Every rederive sub-angle enumerable from the ledger is measurement-closed across 8 distinct sources.
- mechanism: m2c fresh decompile (s8, inline-all-args basin), marionation_Exec sibling transplant P1-P4 (s9, s-reg web non-fungible), decomp.me shingle scan (s17, peak 0.097 similarity), decomp.me residual instruction cluster (s26, 0/3754), BB2 in-repo residual cluster (s26, only self+twins+asmfix stub), Kengo cpu_side_move_dir dump (s18, empty stub), saEft01Init decl-transfer (s27, both scopes), idx_1495 respellings (s35, +1/+2/+1), Kengo numata subsystem inventory (s36, 64/64 empty), in-repo 5-arg debug_printf template (s36, 3/3 INCOMPLETE).
- probe: s37 ledger cross-read only.
- result: No un-tried rederive angle remains without external-input dependency (a third-party PS1 decomp with GCC 2.7.2 provenance and a byte-matched 5-arg variadic tbl-dispatch shape).
- verdict: CONFIRMED

## [s37] The h5 basin's LUID tiebreak at sched2 clock=13 (insn 121 LUID 12 vs insn 111 LUID 8, both LAUNCH pri 0x7f000001) is the sole surviving mechanism and cannot be flipped via any of three theoretical lever directions in a way that both preserves h5 basin membership and passes layer-1 cheat vetting.
- mechanism: Lever (a) 111 loses LAUNCH: s7 expmed.c:2244 case alg_shift NULL_RTX wall — any t0*4 spelling reachable through the mult-expander births p106 fresh single-set; any direct-LSHIFT spelling births p101 multi-set in place (g3 basin, register exchange). Lever (b) 121 loses LAUNCH: 5 C-realizations of two-SET p107 KILLED (s11 M1-a/b/c + s12 pointer-walker/dead-sink); combine.c addsi3_internal substitutes simple two-SETs back to single-SET; non-trivial subtrahends regress alloc web. Lever (c) LUID reorder: coupled to basin membership per s6-KILLED and s7 basin non-composability.
- probe: s37 ledger cross-read only.
- result: The three theoretical mechanism paths are compiler-source-closed or measurement-closed across all C-spelling realizations tried in 27 sessions of structural + permuter + forensics work.
- verdict: CONFIRMED

## [s38] There exists a post-debug_printf arm that already computes tbl_125c[idx_1494[1]] (arg5's s32 value) under a distinct C spelling, admitting an asymmetric arg5-only ref-lift.
- mechanism: local-alloc.c qty_compare uses flow-time REG_N_REFS; an additional legitimate use of arg5 (not t0) lifts pri(arg5) above pri(t0) at the strict-win threshold, flipping the p107/p106 allocation and killing the 121/111 LUID tie.
- probe: Audit asm/funcs/cpu_side_move_dir_4.s post-debug_printf arms for any lbu/lw/sll/addu cluster that computes the s32-scaled tbl_125c[idx_1494[1]] value.
- result: Downstream sites only use raw byte values: L80080F3C `lbu 0(s4)`=*idx_1495=idx_1494[1] (u8, callback arg), L80080F70 `lbu 0(s2)`=*idx_1494 (u8, callback arg), L80080F9C `lbu 0(s2)`=*idx_1494 (u8, temp==2/5 test). ZERO downstream sites compute the s32 tbl_125c[idx_1494[1]] value; the only tbl-indexed read in the entire function body is the arg5 computation itself.
- verdict: KILLED

## [s38] Pairing s35 P1 (v0 = *idx_1495 at the arg5 lbu site, masked=3) with a decl-order swap at src/system.c:405-406 (idx_1495 declared BEFORE idx_1494) absorbs the +1 register diff via s-reg birth-luid reordering.
- mechanism: local-alloc.c pseudo birth follows RTL first-use; reordering the two pointer inits shifts idx_1495 and idx_1494 seat allocation and could compensate for the extended idx_1495 live range across the debug_printf window.
- probe: Apply s35 P1 (v0 = *idx_1495 inside inline block) + swap lines 405-406 (idx_1495 init before idx_1494 init). Sandbox --disable all.
- result: masked=7, target_insns=160, build_insns=161 (+1 physical insn). +5 regression vs h5. The decl swap compounds with the idx_1495 seed additively; a lui/addiu materialization for idx_1494 shifts prologue and disturbs s-reg web further, opposite of compensating.
- verdict: KILLED

## [s38] Pairing s35 P1 with a legitimate named carrier at src/system.c:456 (u8 mode = *idx_1495; then callback(mode, ...)) participates in the downstream *idx_1495 semantic use and lands idx_1495 in a distinct callee-save that absorbs the +1 diff.
- mechanism: A named local at the callback site has real semantic purpose (reading callback arg into a local before passing), so it clears layer-1 cheat-vet; if cse.c does NOT fold it, mode's pseudo lands in a distinct s-reg and the extended idx_1495 live-range +1 is absorbed by the reg-web shift.
- probe: Apply s35 P1 (v0 = *idx_1495) + inject `u8 mode = *idx_1495;` before the D_800A11B8 callback at src/system.c:456, pass mode instead of *idx_1495.
- result: masked=3, target_insns=160, build_insns=160. INERT vs s35 P1 baseline (masked=3). cse.c copy-propagation folds the single-use `mode` local back to a direct *idx_1495 read; final RTL is byte-identical to s35 P1 alone. The named local is cse-transparent and does NOT create a distinct pseudo at flow time.
- verdict: KILLED

## [s39] Replacing `v0<<=2; arg5=*(s32*)(v0+(s32)tbl_125c);` with `arg5 = tbl_125c[v0];` after `v0 = idx_1494[1];` on the h5 chassis produces a fresh single-set arg5-index pseudo via expmed alg_shift (NULL_RTX) and shifts the sched2 LAUNCH clock, changing the pair-swap tiebreak.
- mechanism: GCC 2.7.2 expand_expr on `tbl_125c[v0]` where `tbl_125c` is `s32*` lowers the sizeof(int)=4 scale via expmed.c::expand_mult case alg_shift with NULL_RTX target, birthing a fresh single-set pseudo for the v0*4 term; this new SLL SET-dest satisfies birthing_insn_p and would fire an additional LAUNCH on the arg5 side, potentially disturbing the 121/111 tie at clock=13.
- probe: Applied `arg5 = tbl_125c[v0];` (dropping explicit `v0<<=2` + `arg5=*(s32*)(v0+(s32)tbl_125c);`) on h5 candidate at src/system.c inline block; `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.
- result: masked=2, target_insns=160, build_insns=160. INERT vs h5 baseline. Novel data point: expand normalizes the array-index syntax (`tbl_125c[v0]`) and the explicit shift+deref form (`v0<<=2; *(s32*)(v0+tbl_125c)`) to byte-identical RTL — CSE/combine fold both to the same p101-in-place SLL then load. The array-syntax path does NOT birth a distinct fresh pseudo on arg5's side. Tightens the expand-normalization mapping in the ledger. Artifact tmp/grind/cpu_side_move_dir_4/s39/probe1_array_index_arg5.c.
- verdict: KILLED

## [s39] Substituting `(s32)tbl_125c` with `(s32)&D_800A125C` at the arg5 addressing site defeats the fn-scope tbl_125c pseudo's participation in the arg5 chain and re-materialises the symbol via a fresh lui/addiu at the reference site, shifting the s-reg web enough to flip the residual pair-swap.
- mechanism: cse.c equiv_constant folds addressable symbols across the block boundary, but a distinct lui/addiu materialisation at the reference site adds a new insn and shifts s-reg birth luids feeding local-alloc.c qty_compare; if the lui/addiu absorbs into a delay slot or promotes tbl_125c to a distinct pseudo, alignment could shift.
- probe: Applied `arg5 = *(s32*)((s32)&D_800A125C + v0);` on h5 chassis at src/system.c; sandbox cpu_side_move_dir_4 --disable all.
- result: masked=16, target_insns=160, build_insns=161 (+1 physical insn). +14 regression. Fresh lui/addiu materialises for &D_800A125C at the arg5 site (build_insns +1) — GCC 2.7.2 does NOT fold `&D_800A125C` back to the `tbl_125c` pseudo across the block boundary in the arg5 addressing context; same-shape KILL as s35 P2 (arg5_seed_symbol_offset). Artifact tmp/grind/cpu_side_move_dir_4/s39/probe2_direct_symbol_arg5.c.
- verdict: KILLED

## [s39] Promoting `v0` to a triple-set arithmetic carrier via `v0<<=2; v0+=(s32)tbl_125c; arg5=*(s32*)v0;` defeats birthing_insn_p on the arg5-addr SET (insn 121-equivalent) while preserving the multi-set t0 chain of the h5 chassis, flipping the 121 LAUNCH off while keeping 111 LAUNCH on.
- mechanism: flow.c reg_n_sets(v0)>=3 after this transformation; sched.c::birthing_insn_p returns FALSE for any SET whose dest has reg_n_sets>1; the arg5-addr SET (previously fresh single-set p107) becomes an in-place SET on the multi-set v0 pseudo, losing LAUNCH. The h5 t0 chain (multi-set t0 with fresh p106 from expmed alg_shift) preserves 111 LAUNCH; only 121 loses. Result would be 111 alone at LAUNCH pri at clock=13, no tie, target order 118/121/111 emitted.
- probe: Applied `v0<<=2; v0+=(s32)tbl_125c; arg5=*(s32*)v0;` on h5 chassis at src/system.c; sandbox cpu_side_move_dir_4 --disable all.
- result: masked=15, target_insns=160, build_insns=160. +13 regression. Same-shape KILL as the s11 arg5_addr two-SET forms (M1 algebraic-cancel, M1-secondary opaque-carrier): combine.c substitutes the multi-set v0 chain back into an in-place computation, destroying the p107 fresh-dest LAUNCH entirely rather than merely suppressing it; the arg5 chain falls out of h5 basin into a novel bad-alignment. Confirms the 'make v0 (or its downstream arg5_addr) multi-set to defeat 121 LAUNCH' lever direction is closed at every C-source spelling that maintains dataflow — extends s11/s12 findings from arg5_addr onto the v0 pseudo itself. Artifact tmp/grind/cpu_side_move_dir_4/s39/probe3_v0_multiset_addend.c.
- verdict: KILLED

## [s40] PERM_GENERAL scoped over the block=3 inline block PLUS the immediately-flanking CALLs (tslTm2LoadImage_2 pre-call + cdrom_ClearIrq post-call) surfaces a novel h5-preserving mutation neighborhood that the s32 block-only scope cannot express (statement motion across CALL boundaries into/out of block=3).
- mechanism: PERM_GENERAL applies expression-level AST mutations that can only move statements within its lexical scope; s32 confined mutations to the 8-statement h5 block, so any h5-preserving find requiring a statement to cross tslTm2LoadImage_2 or cdrom_ClearIrq was unreachable. Widening the scope to include the two flanking CALLs unlocks that class of mutation. The h5 chassis (base_score=60, mult-expander LAUNCH on insn 111 via expmed.c:2244 case alg_shift NULL_RTX) is preserved.
- probe: Copied tmp/perm_csmd4/{base.c,compile.sh,settings.toml,prelude_r3k.inc,full.s,target.o} to tmp/grind/cpu_side_move_dir_4/s40/perm_wider/; widened PERM_GENERAL scope in base.c to wrap tslTm2LoadImage_2(&D_800161B8) + the 8-statement inline block + cdrom_ClearIrq(); confirmed candidate.c applied to src/system.c at masked=2 baseline. Launched via tools/permuter_campaign.py launch --func cpu_side_move_dir_4 --dir tmp/grind/cpu_side_move_dir_4/s40/perm_wider --label s40_wider_scope -j 4 --stop-on-zero. base_score=60 confirmed at iteration 1 (h5-multexpander chassis preserved). Ran through fresh-seed window; harvested with 0 novel outputs.
- result: 0 novel finds across 1738 iterations (harvest report iterations=1738, finds_new=0, best_new_score=null, procs_killed=5). Zero output-<score>-<ctr> dirs created. Slower iters/s than s32 (12.9 iters/s vs 28 iters/s) due to wider AST scope increasing per-iteration compile cost, but the fresh-seed window ran long enough for a novel-neighborhood mutation to surface if it existed (s32 novel finds at weighted=40 both appeared within 60s). Widening the scope to include the flanking CALLs does NOT unlock any weighted-<40 mutation neighborhood.
- verdict: KILLED

## [s41] A PERM_LINESWAP campaign over the two poll-loop callback if-blocks (status&4 -> D_800A11B8(*idx_1495,...) and status&2 -> D_800A11B4(*idx_1494,...)) discovers an h5-preserving ordering that shifts the file-scope s-reg allocation web (idx_1494/idx_1495/tbl_125c) at flow-analysis, cascading into the debug_printf window's block-3 qty priorities and flipping the p107/p106 LUID tiebreak.
- mechanism: flow.c REG_N_REFS accumulation across the callback statements determines the s-reg web's qty priorities feeding local-alloc.c qty_compare; reordering the callback if-blocks would change the reg_n_refs distribution on idx_1494 vs idx_1495 without disturbing the h5 chassis (mult-expander LAUNCH on insn 111 via expmed.c:2244 case alg_shift NULL_RTX). The poll-loop dispatch scope has NEVER been touched by any prior permuter campaign (s5/s13/s14/s22/s23/s31/s32/s40 all scoped to prologue-init or the debug_printf inline block or the flanking CALLs) so this is a genuinely novel permuter axis targeting the ALLOCDBG s-reg coupling frontier the judge called out.
- probe: Copied tmp/perm_csmd4 workspace to tmp/grind/cpu_side_move_dir_4/s41/perm_poll/; stripped the s32/s40 PERM_GENERAL wrapper from the h5 inline block (restoring plain h5 chassis); wrapped the two callback if-blocks in PERM_LINESWAP inside the outer `if (status != 0)`; launched via tools/permuter_campaign.py launch --func cpu_side_move_dir_4 --dir tmp/grind/cpu_side_move_dir_4/s41/perm_poll --label s41_poll_lineswap -j 4 --stop-on-zero.
- result: Permuter exited rc=1 immediately with `Syntax error in base.c. before: } at approximately line 425, column 23 (after PERM expansion)`. PERM_LINESWAP's annotator expects line-terminated statement-form items and cannot split brace-delimited compound `if (...) { ... }` blocks. The novel axis is unreachable through the tooled permuter directive set on this AST shape; rewriting the callbacks into single-line ternary/comma-op form would materially change codegen and not constitute a permutation of the current shape.
- verdict: KILLED

## [s41] The h5 candidate.c applied to src/system.c reproduces the ledger-recorded masked=2 floor (baseline sanity check).
- mechanism: Overlay the block-scope { s32 arg5; s32 t0; void **pp; t0=idx_1494[0]; v0=idx_1494[1]; pp=(void**)&D_800F19C0; t0*=4; t0=(s32)((u8*)tbl_125c+t0); v0<<=2; arg5=*(s32*)(v0+(s32)tbl_125c); debug_printf(...,*pp,...,*(s32*)t0,arg5); } inline block from candidate.c onto src/system.c, replacing the HEAD both-named arg4/arg5 array-index form.
- probe: Edit src/system.c inline block to candidate.c spelling; & tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all.
- result: masked=2, target_insns=160, build_insns=160 (matches the 40-session ledger; HEAD (both-named form) scores masked=7 baseline).
- verdict: CONFIRMED

## [s42] Frontier (b) - flattening the poll-loop callback if-blocks into a single-statement ternary dispatch shifts the s-reg web and re-weights local-alloc.c qty_compare on idx_1494/idx_1495, propagating into block=3's pair-swap residual.
- mechanism: Alleged: callback restructure changes REG_N_REFS on idx_1494/idx_1495 pseudos, which re-weights their global-alloc priority (pri = floor_log2(refs)*refs*10000/livelen), shifting s-reg ranking; the shift is claimed to propagate into block=3's local-alloc qty_compare on p106/p107.
- probe: Read ALLOCDBG global-alloc ranking, QTYDBG blk=3 census, and .greg conflict lists from tmp/grind/cpu_side_move_dir_4/s6/ (h5 baseline instrumented cc1 dump). Map pseudos to source variables (p77=idx_1494, p78=idx_1495, p79=tbl_125c, p106=t0*4 mult-expander temp, p107=arg5_addr). Check whether idx_1494/idx_1495 conflict with any block=3 QTY pseudo (necessary precondition for allocation coupling).
- result: p77 conflict list: {72,73,75,77,78,79,80,81,86,101,120,126, 2,3,4,5,6,7,29}. p78 conflict list: identical. Block=3 QTY pseudos {p100, p106, p107, p113}: NONE appear in p77/p78 conflict lists. idx_1494 and idx_1495 have zero shared live range with the pair-swap window's local pseudos. Additionally, priority formula shows idx_1494's pri=933 sits between p80's 952 and p79's 675 with >20-unit gap; +/-1 ref change yields pri deltas (~130 up, ~130 down) that leave the s-reg ranking order unchanged (still ord=11, hardreg s2).
- verdict: KILLED

## [s43] sched2 (post-reload) applies adjust_priority-LAUNCH to insns 111 and 121, matching sched1's LAUNCH state on both insns.
- mechanism: sched.c::adjust_priority uses birthing_insn_p to flag SET-dests-of-live-pseudos with LAUNCH sentinel 0x7f000001. If sched2 sees the same pseudos as sched1, LAUNCH persists.
- probe: Direct read of csmd4_only.log at both sched1 (lines 165-180) and sched2 (lines 763-785) SCHEDDBG ready-list traces for block=3 clock=10-15 picks on insns 111,121,123,129,116,118,142.
- result: sched1 shows 111,121,123,129 all at pri=2130706433 (LAUNCH); ready-queue trace: '121(p=2130706433,l=12) 111(p=2130706433,l=8) 142(p=1,l=22)'. sched2 shows the SAME insns at plain pri=2; ready-queue trace: '121(p=2,l=7) 111(p=2,l=6) 142(p=1,l=4)'. Zero LAUNCH sentinels in sched2 block=3 output.
- verdict: KILLED

## [s43] sched2's mechanism for picking 121 before 111 at clock=13 is a LAUNCH-vs-LAUNCH LUID tiebreak, mirroring the sched1 mechanism the ledger has been targeting since s6.
- mechanism: If sched2 has LAUNCH on both, RANKDBG val=0 (cls=3 vs cls=3) triggers LUID tiebreak; higher-LUID wins in backward-scheduler pick order.
- probe: Read SCHEDDBG PICK / ready traces at sched2 clock=13-15 for the residual pair (line 774-782 of csmd4_only.log).
- result: sched2 clock=13 pick: 121(p=2,l=7) beats 111(p=2,l=6) by LUID diff of 1 within the plain-priority-2 class (RANKDBG last=123 y=121 cls=3 x=111 cls2=3 val=0). NOT a LAUNCH-vs-LAUNCH tie; it is a plain-priority-tie. The LUID delta is 1, not the sched1 delta of 4.
- verdict: KILLED

## [s43] sched2 offers an independent attack surface from sched1 for flipping the pair-swap (i.e., a C-source lever exists that shifts sched2 outcome without also shifting sched1 outcome).
- mechanism: If sched2's priority computation differs materially from sched1, a lever that raises pri(111) above pri(121) at sched2 (via deepening 111's downstream chain post-reload) could win the tiebreak in the correct direction.
- probe: Enumerate sched2 priority sources: 111->116 (single downstream user, pri=2) and 121->123 (single downstream user, pri=2). Enumerate C-source levers that would add a downstream user to p106 (SLL dest at 111) or extend 116's chain via multi-use of p101.
- result: Both directions collapse into KILLED families: (a) adding a downstream user of p106 = multi-use of the mult-expander fresh temp, which by s7 expmed.c:2244 case alg_shift NULL_RTX target forces the direct-LSHIFT path = g3 basin regression (masked=6-7); (b) extending 116 via multi-use of p101 (t0) breaks the multi-set t0 pattern that h5's sched1 LAUNCH suppression on insn 116 depends on = g3 basin regression. Both = s3/s11/s12/s39 KILLED classes.
- verdict: KILLED

## [s44] The decomp.me corpus (BB2 toolchain class: gcc2.7.2-psx / gcc2.7.2-cdk / psyq3.5) contains a scratch whose target asm structurally overlaps asm/funcs/cpu_side_move_dir_4.s, providing a novel starting-point C form for the h5/g3 basin residual outside the exhausted search paths.
- mechanism: tools/decomp_me_scrape.py search uses k-shingle Jaccard similarity on normalized asm mnemonic sequences. A meaningful analog would score >= ~0.10 (background noise floor is ~0.05 for unrelated PS1 asm of comparable length). Similarity >= 0.10 would flag a candidate for manual inspection of its C source as a novel starting point.
- probe: Installed curl_cffi in .venv (s9's blocker resolved). Downloaded 602 scratches (263 gcc2.7.2-psx, 190 gcc2.7.2-cdk, 149 psyq3.5) via `python3 tools/decomp_me_scrape.py download --compiler <X> --limit N --out tmp/grind/cpu_side_move_dir_4/s44/corpus/<X>`, then `search --asm-file asm/funcs/cpu_side_move_dir_4.s --top 10` against each corpus.
- result: Max similarity across all 3 corpora: 0.079 (psyq3.5 slug h1LHX 'main' - sonicdcer's Kalisto engine init boilerplate, structurally unrelated to a poll-loop/debug_printf dispatcher). gcc2.7.2-psx top 0.053 (ape_escape func_80020F6C, unrelated). gcc2.7.2-cdk top 0.071 (func_8009BCE4, unrelated). All top-10s per corpus fall in background-noise band [0.024, 0.079]. No candidate crosses the meaningful-overlap threshold.
- verdict: KILLED

## [s45] The Kengo dump contains at least one non-stub game function body that could serve as a structural template for csmd4's h5 basin flip.
- mechanism: Kengo is Lightweight's PS2 successor reusing the Marionation engine; if any nm_cpu / nm_single_game / adjacent numata function extracted with a body, its 5-arg debug_printf shape (if present) would document original-source structure for csmd4.
- probe: grep -vE '\{\}$' Kengo/kengo_functions_full.txt to enumerate all 73 non-stub bodies; classify by subsystem.
- result: 73 non-stub bodies inventoried; 100% are libc/math (cos/sin/atan2/sqrt/sprintf/_vfprintf_r/etc). Zero game functions carry extracted bodies. Zero 5-arg variadic dispatch shapes present.
- verdict: KILLED

## [s45] The SOTN decomp corpus (../sotn-decomp) contains a pure-C matched function with the same 5-arg variadic + adjacent-byte-index tbl-dispatch shape as csmd4's debug_printf window, transferable as a structural template.
- mechanism: SOTN uses GCC 2.7.2 PsyQ variant (compiler-class match to BB2); shared codegen would imply shared closing form for the same source-shape.
- probe: grep -rlnE '\bdebug_printf\s*\(' + grep -rlE '\w+\[\w+\[[01]\]\]' across ../sotn-decomp/src/**/*.c (1494 files).
- result: Zero debug_printf references (SOTN uses OSReport/FntPrint/sprintf families). Tbl-indexed dispatch pattern present at 8+ sites but none is 5-arg variadic + adjacent-byte-index shape. Konami PsyQ variant + distinct macro surface further block transplant.
- verdict: KILLED

## [s46] Synthesis modality merge produces a novel merged attack for s47+ by reordering the two live frontier notes (F2 primary, F1 secondary) and codifying the prerequisite audit for the judge-preferred SOTN-family carve-out axis.
- mechanism: Read entire 45-session ledger (evidence.md 813 lines, hypotheses.md 811 lines). Cross-check judge constraint (BINDING per s40/s41 ledger) against the two live frontier notes (F1 WAR/WAW dep-edge, F2 FAKE-annotated named-local). Judge explicitly lists F2's axis first ('SOTN-family carve-out application under codified prerequisites + FAKE annotation + layer-2 cheat-reviewer'); F1 competes against s38 post-window arm audit finding zero downstream reads of p101-post-shift, weakening its precondition. Named-local-fake-exception rule (2026-07-01, .claude/rules/named-local-fake-exception.md) codifies a NAMED GCC-pass interaction sanctioned prerequisite that fits the s33 reload-renumbering mechanism observed on saEft01Init (sched2 zero LAUNCH after reload substitutes fresh block=3 dests into fn-scope multi-set hard-regs), which csmd4 has NEVER been probed against with FAKE annotation.
- probe: Ledger cross-read + rule-file cross-read; no measurement.
- result: Frontier reset: F2 promoted primary with 3 concrete SOTN-archetype-mirror candidate drafts specified (C1: s32 zero=0 archetype; C2: s16 three=3 archetype; C3: ALLOCDBG-instrumented scalar-type mini-sweep); F1 demoted secondary contingent on s38 arm-audit re-check for missed post-shift semantic use. Synthesis artifact at tmp/grind/cpu_side_move_dir_4/s46/synthesis.md documents merged attack, prerequisite audit, layer-1/2 cheat-reviewer risk analysis, and execution order for s47+.
- verdict: CONFIRMED

## [s46] Every rederive-modality angle enumerable from the ledger is measurement-closed; no external-corpus lookup or in-repo transplant should be scheduled at this chassis.
- mechanism: s45 extended the rederive census to 10 sub-angles across m2c (s8), marionation transplant P1-P4 (s9), decomp.me shingle (s17), Kengo cpu_side_move_dir stub (s18), decomp.me residual + BB2 in-repo residual (s26), saEft01Init decl-transfer (s27), idx_1495 respellings (s35), Kengo numata + BB2 5-arg debug_printf template (s36), decomp.me BB2-toolchain corpus (s44), cross-project SOTN scan + Kengo whole-dump body inventory (s45). All 10 KILLED at measurement threshold >=0.10 similarity or shape-match by structural audit.
- probe: Ledger cross-read only.
- result: Rederive-modality is fully closed at chassis; the operator supplying a third-party PS1 decomp with GCC 2.7.2 provenance and byte-matched 5-arg variadic tbl-dispatch shape is the only theoretical remaining input, and no such class of input has been observed in 45 sessions.
- verdict: CONFIRMED

## [s46] Permuter-modality campaigns at h5 chassis produce zero novel finds below weighted-40 across ≈62,545 iterations spanning every reorderable scope (prologue, block, block+CALL, wider AST), so future permuter modality on this function without a NEW chassis is redundant.
- mechanism: Union of s22 (LINESWAP 7!=5040 block AST), s23 (LINESWAP_TEXT 20429 block+CALL text neighborhood), s31 (LINESWAP 5!=120 prologue), s32 (PERM_GENERAL 35338 block AST expression-level), s40 (PERM_GENERAL 1738 block+flanking-CALLs wider AST). All launched from base_score=60 h5 chassis (confirmed guardrail). Combined coverage exhausts h5-preserving permuter neighborhood.
- probe: Ledger cross-read only.
- result: Permuter-modality closed at h5 chassis; s47+ permuter runs must launch from a DIFFERENT chassis (e.g. FAKE-annotated named-local variant emerging from F2) or the campaign duplicates prior 0-find measurements.
- verdict: CONFIRMED

## [s47] A fn-body-top `s32 zero = 0;` FAKE-annotated per named-local-fake-exception SOTN archetype, DECLARED unused, will shift local-alloc qty birth-luid so reload renumbers a block=3 SET-dest into a fn-scope multi-set hard-reg, suppressing LAUNCH at sched2 and flipping the h5 pair.
- mechanism: s33 saEft01Init reload-renumbering mechanism: fn-scope multi-set hard-regs cause birthing_insn_p to return FALSE at sched2. But a DCE'd unused local never reaches local-alloc.
- probe: Applied `s32 zero = 0;` at fn body top (line 389, before decl block), h5 candidate otherwise intact; sandbox cpu_side_move_dir_4 --disable all.
- result: masked=2 INERT, target_insns=160, build_insns=160 unchanged. Tree-level DCE removes the unused local before local-alloc sees it. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/fn_scope_zero_dead_scalar_C1.c.
- verdict: KILLED

## [s47] A fn-scope `s32 zero = 0;` kept live via `v0 = zero;` at the success arm (SOTN DispSamnailWindow constant-holder archetype spanning debug_printf + cdrom_ClearIrq) forces zero into a callee-save and shifts the block=3 alloc equation.
- mechanism: SOTN constant-holder shape reserves a callee-save reg to avoid re-materialization across intervening calls; cross-call preservation is what actually reaches local-alloc as a live pseudo (unlike C1/C3 which fold or DCE).
- probe: Applied `s32 zero = 0;` at fn top + replaced `v0 = 0;` at cpu_side_move_dir_4 success arm with `v0 = zero;` (unique-context edit; marionation success arm untouched); sandbox --disable all.
- result: masked=10 (+8 vs h5 baseline 2), target_insns=160, build_insns=163 (+3 insns). The callee-save reserve + preservation moves cost >> any LUID/LAUNCH benefit; the alloc web falls out of h5 basin. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/fn_scope_zero_constant_holder_C2.c.
- verdict: KILLED

## [s47] A fn-scope `s32 zero = 0;` used at a SINGLE pre-block=3 store (`D_800F19BC = zero;` at line 408) — no cross-call preservation demanded — will land the local as a distinct pseudo in local-alloc while avoiding C2's callee-save cost.
- mechanism: Local born at line 408, dead by loop entry; short live range confined to prologue. If cse does not fold it back to literal 0, it acquires a distinct pseudo and could bias qty tables at the loop entry into block=3.
- probe: Applied `s32 zero = 0;` + `D_800F19BC = zero;` at line 408; sandbox --disable all.
- result: masked=2 INERT, target_insns=160, build_insns=160 unchanged. cse folds `zero` back to literal 0 at the single-use site before local-alloc; the local never reaches allocation as a distinct pseudo. Same net outcome as C1 (DCE), different pass. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/fn_scope_zero_prologue_use_C3.c.
- verdict: KILLED

## [s47] A downstream C-source expression in one of the target's post-block=3 arms reads the post-shift t0 value (or a `(u8*)tbl_125c + (t0<<2)` shape), letting an honest respelling add a WAR dep-edge to insn 111 and raise pri(111) above pri(121) to flip the LUID tiebreak without perturbing 116's launch suppression.
- mechanism: sched.c::insn_priority walks INSN_DEPEND successors; a WAR from a later reader raises the producer's priority. The read must have semantic purpose (target asm must already compute the same expression under a distinct spelling) per no-new-park-categories.
- probe: Audit asm/funcs/cpu_side_move_dir_4.s L80080EDC..L80081004 (all arms after the do_timeout block) for any expression reading (u8*)tbl_125c + (t0<<2), or the raw t0<<2 value, or an alias-derived shape.
- result: KILLED. The arms use only $s2 (idx_1494), $s4 (idx_1495), $s6 (a1), $s5 (arg0), literal constants, and D_800A147C/D_800A11B4/D_800A11B8 dispatch pointers. NO downstream expression references (u8*)tbl_125c + (t0<<2), the raw t0 value, or its shift result. The t0-chain shifted value is dead immediately after the debug_printf window (the lw a3,0(a0) at L80080ED8 is the final use). No honest downstream reader exists to lever pri(111).
- verdict: KILLED

## [s48] C4: fn-scope `s32 held;` decl + `held = cnt;` written between line 416 (`cnt = D_800F19BC;`) and line 417 (`D_800F19BC = cnt - -1;`) at the fast path lifts arg5-qty refs above t0-qty via reg_n_refs propagation on the loop-back edge into block=3.
- mechanism: flow.c reg_n_refs feeds local-alloc.c qty_compare priority. A written-never-read fast-path local's SET participates in flow-time ref counting; if its liveness propagates through the loop-back edge into block=3, it may bias qty allocation.
- probe: Applied C4 to src/system.c (h5 base): `s32 held;` fn-scope decl + `held = cnt;` between lines 416-417. FAKE annotation per dead-store-fake-exception.md. sandbox cpu_side_move_dir_4 --disable all.
- result: masked=2 INERT vs h5 baseline (target_insns=160, build_insns=160). Bytes identical - the dead store is DCE'd upstream of qty tables OR its ref never reaches block=3's live-in set. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/s48_C4_held_cnt_fast_path.c.
- verdict: KILLED

## [s48] C5: same shape as C4 but `held = *idx_1495;` (pointer-read source vs scalar cnt-copy) escapes single-set copy-fold via non-fold across the reload boundary and preserves reg_n_refs contribution to block=3 qty.
- mechanism: cse.c copy-propagation folds simple `held = local_scalar` (copy of an existing live pseudo) as a trivial single-set copy; a `held = *ptr` load from an unrelated address should defeat the copy-fold and keep held's SET distinct at flow-time. If so, ref-lift materializes; otherwise the DCE runs before block=3 sees it.
- probe: Applied C5 to src/system.c: replaced C4's `held = cnt;` with `held = *idx_1495;`. FAKE annotation retained. sandbox --disable all.
- result: masked=2 INERT vs h5 baseline. Bytes identical - the pointer-read source ALSO does not shift block=3 qty tables; distinguishes copy-fold from DCE as the mechanism (both would give inert, but the pointer-source rules out the copy-fold explanation exclusively - the effect is DCE-upstream-of-flow OR the ref-lift is sub-threshold vs t0's 5000-priority). Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/s48_C5_held_idx1495_fast_path.c.
- verdict: KILLED

## [s48] C6: fn-scope `s32 held;` decl + `held = *idx_1494;` written at do_timeout label entry (immediately before the tslTm2LoadImage_2 call, INSIDE block=3's dominator region) forces held's ref count into block=3 directly, bypassing the loop-back-edge propagation dependency.
- mechanism: By placing the write at the do_timeout entry (block=3's actual predecessor after either branch merges), held's reg_n_refs is unambiguously counted against block=3's qty table. If C4/C5 failed because loop-back propagation doesn't carry held's liveness into block=3, this placement removes that risk - the write is inside block=3's dominator region.
- probe: Applied C6 to src/system.c: `s32 held;` fn-scope decl + `held = *idx_1494;` between `do_timeout:` label (line 423) and `tslTm2LoadImage_2` call (line 424). FAKE annotation per named-local-fake-exception.md. sandbox --disable all.
- result: masked=2 INERT vs h5 baseline. Bytes identical - CONFIRMS the mechanism failure is DCE-upstream-of-flow (dead store to a local whose value is never read gets removed BEFORE local-alloc sees the ref, so the qty tables never widen for held) rather than a placement/liveness-propagation issue. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/s48_C6_held_do_timeout_entry.c.
- verdict: KILLED

## [s49] F3: raise 121->123 edge latency above 1 via a HI-LO producer feeding arg5_addr, letting insn 111 win the class-1 mips_adjust_cost gate in rank_for_schedule.
- mechanism: sched.c::rank_for_schedule class-1 test via LOG_LINKS(last_scheduled) + insn_cost gate; a mult/div result naturally consumed by arg5_addr would raise 121's dep-edge cost >1 and let 111 win the val!=0 class diff without hitting LUID tiebreak.
- probe: grep -nE '\bmult\b|\bdiv\b|\bmadd\b|\bmfhi\b|\bmflo\b|\bmultu\b|\bdivu\b' on asm/funcs/cpu_side_move_dir_4.s. Confirm target's arg5_addr chain uses only sll/addu (no HI-LO involvement) and no legitimate HI-LO consumer exists anywhere in the function.
- result: ZERO matches for any mult/div/HI-LO op in the entire target asm. Target computes arg5_addr strictly via sll v0,v0,2 + addu. There is NO legitimate C-source HI-LO producer whose result naturally feeds arg5_addr - the target has no multiply/divide semantics in the function at all. Any fabricated `something *= something;` inserted purely to trigger mips_adjust_cost is a coercion whose only purpose is to change GCC's scheduler cost - directly cheat-shaped per no-new-park-categories cheats-by-any-spelling.
- verdict: KILLED

## [s49] F4: written-never-read local ARRAY sanction (dead-vars-local-array carve-out, oracle-enforced) - if target has unmatched dead sw stores in a stack-frame region the h5 candidate never allocates, a matching-shape local u8/s32 array with matched writes reserves frame bytes AND emits actual sw stores, rippling into h5 basin's alloc web via saved-reg pressure.
- mechanism: Written-never-read local array reserves frame bytes and emits real MEM stores (not DCE-vulnerable like the scalar variant); prerequisite is target-oracle-enforced (target must contain the dead stores).
- probe: grep -nE '\bsw\b|\bsh\b|\bsb\b' on asm/funcs/cpu_side_move_dir_4.s. Map every store; identify any frame slot 0x00-0x14 or beyond callee-save block with unmatched dead writes.
- result: 13 stores total. Frame layout: L3/5/8/9/10/11/12/14 = callee-save preservation of s0..s6,ra at 0x18..0x34. L22/26/28/43 = global stores to D_800F19B8/BC/C0 (matched). L65 = sw v1,0x10($sp) = arg5 varargs homing for debug_printf (matched). L132/144/155 = matched sb byte stores via callbacks / write-back copy loop. NO unmatched dead sw stores exist. Slot 0x14 is a genuine unused gap between arg5-home (0x10) and callee-save (0x18) - target does NOT write it. dead-vars-local-array.md carve-out prerequisite ('target bytes contain the corresponding dead stores, oracle-enforced') is NOT satisfied.
- verdict: KILLED

## [s49] Narrow arg5-only PERM_GENERAL (fixed t0 chain preserving h5 mult-expander LAUNCH; PERM_GENERAL over just {pp alias, v0<<=2, arg5=*(v0+tbl)} sub-block) explores a distinct mutation neighborhood not covered by s22 (whole-block LINESWAP), s32 (whole-block PERM_GENERAL, 35338 iters), or s40 (block+flanking-CALLs, 1738 iters).
- mechanism: By keeping the t0 chain fixed, permuter mutations cannot accidentally drop into g3 basin; the search is contained to the arg5 sub-neighborhood.
- probe: python3 tools/permuter_campaign.py launch --func cpu_side_move_dir_4 --dir tmp/grind/cpu_side_move_dir_4/s49/perm_narrow --label s49_narrow_arg5only -j 8 --stop-on-zero; harvest+stop after ~9min.
- result: elapsed_s=567.1, iterations=15064, base_score=60 (h5 chassis preserved as intended - narrower scope did NOT collapse to g3 basin=40), finds_new=0, procs_killed=9, stopped=true. Fresh-seed 0-find harvest confirms h5-multexpander local-minimum is stable at ~9min wall time for the arg5-sub-block mutation neighborhood too. Extends s13 (2999 iters h5) + s32 (35338 iters) + s40 (1738 iters) - total h5-preserving campaigns now ~55k iters at 0 novel <60 finds.
- verdict: KILLED

## [s50] A PERM_GENERAL scoped over the poll-loop dispatch (the two callback if-blocks inside `if (status != 0)`) opens a novel mutation neighborhood untouched by prior campaigns (s5/s13/s14/s22/s23/s31/s32/s40/s49 all scoped to prologue, inline block, or block+flanking-CALLs) and could shift the s-reg web idx_1494/idx_1495 refs enough to alter block=3 qty priorities and flip the 121/111 pair.
- mechanism: flow.c REG_N_REFS accumulation across the callback statements determines the s-reg web's qty priorities feeding local-alloc.c qty_compare; reordering / mutating the callback dispatch region should change reg_n_refs distribution on idx_1494/idx_1495 without disturbing the h5 chassis (mult-expander LAUNCH on insn 111 via expmed.c:2244 case alg_shift NULL_RTX). The dispatch scope is genuinely novel for permuter.
- probe: Copied s49 perm_narrow workspace to tmp/grind/cpu_side_move_dir_4/s50/perm_poll_gen/; removed the s49 narrow PERM_GENERAL wrapper from inline block (restored h5 chassis); wrapped the two callback if-blocks inside `if (status != 0)` in PERM_GENERAL(). Launched via `tools/permuter_campaign.py launch --func cpu_side_move_dir_4 --dir tmp/grind/cpu_side_move_dir_4/s50/perm_poll_gen --label s50_poll_gen -j 4 --stop-on-zero`.
- result: The permuter emitted `No perm macros found. Defaulting to randomization.` — the PERM_GENERAL macro placement inside the compound `if (status != 0) { ... }` was not detected by the permuter's directive parser (same class of tool-side limitation as s41 PERM_LINESWAP on brace-delimited blocks). The campaign proceeded as full-function random-mode with base_score=60 (h5 chassis preserved as intended). Fresh-seed window: 29289 iterations in 1565s. Exactly ONE novel find surfaced (output-40-1 at seconds_since_launch=662.7); no <40 find. Applied the find to src/system.c: masked=2 (h5 baseline unchanged, no improvement). The +20 weighted-score delta (60→40) is a register-name reshuffle the masked scorer normalizes out. Reverted src/system.c to h5 candidate.c.
- verdict: KILLED

## [s50] The s50 output-40-1 find is a legitimate structural closing form.
- mechanism: The find hoists `&D_800F19C0` to a fn-scope `void **new_var2;` initialised outside the inline block, then `pp = (void **) new_var2;` inside, and introduces a `s32 temp;` intermediate `temp = *(s32*)(v0+(s32)tbl_125c); arg5 = temp;`. If legitimate, would be a novel h5-preserving basin.
- probe: Layer-1 cheat-vet vs inline-asm-policy expanded catalog + no-new-park-categories cheat-by-any-spelling: (a) `new_var2` is a fn-scope pointer-alias for `&D_800F19C0` with no semantic purpose beyond routing pp through a distinct pseudo — same intent as pp itself (already FAKE-annotated in h5 candidate) but WITHOUT documented lever-exhaustion or FAKE annotation. (b) `temp` is a load-intermediate whose only purpose is to widen arg5's pseudo chain; arg5 is directly load-and-passed to the CALL and needs no local staging. Both constructs match the 'necessary only because permuter said so' + 'no semantic purpose' signals in no-new-park-categories.
- result: REJECTED as cheat-shaped without measurement dependency: even if the find had lowered the floor (it did not — masked=2 same as h5), the constructs fail the layer-1 vetting checklist. Saved at memory/grind/cpu_side_move_dir_4/rejected/s50_perm_new_var2_temp_intermediate.c for future sessions to avoid re-deriving.
- verdict: KILLED

## [s50] h5-preserving permuter neighborhood extended by ~29k iters at whole-function random-mode scope yields any additional novel-basin closing form below weighted-40 not measured in prior sessions.
- mechanism: The s50 campaign (accidentally reduced to full-function random-mode due to the PERM_GENERAL directive parse failure inside the compound-if block) is a superset of s22/s31/s32/s40/s49 scopes at the same h5 chassis. If any un-touched mutation neighborhood existed with weighted<40, this ~26-minute fresh-seed window would surface it.
- probe: s50 harvest report: iterations=29289, elapsed_s=1565.2, base_score=60, finds_new=1, best_new_score=40, procs_killed=5. Metrics recorded in metrics/events.jsonl via permuter-launch + permuter-harvest events.
- result: Zero finds below weighted-40. Extends the h5-chassis permuter closure to ≈85k total iterations (s5 + s13 + s22 + s23 + s31 + s32 + s40 + s49 + s50) at 0 novel <40 finds. The 1 novel-40 find this session is a same-floor variant with cheat-shaped constructs, not a genuine basin-shift.
- verdict: KILLED

## [s51] The residual sched1 clock=13 LUID delta of 4 between insn 111 (t0<<=2 -> p106, LAUNCH, LUID 8) and insn 121 (arg5_addr PLUS -> p107, LAUNCH, LUID 12) can be shrunk to <=0 by a C-source lever that removes an intervening chain slot (LUID 9, 10, or 11) without triggering the h5->g3 basin flip.
- mechanism: sched.c::schedule_block runs sched_analyze which walks the pre-sched1 RTL chain and assigns LUIDs sequentially. Rank_for_schedule (tools/gcc-2.7.2/sched.c:2399-2456) at clock=13 sees both 111 and 121 at LAUNCH pri=0x7f000001, class=3 both, cls diff=0 (per s15 RANKDBG val=0 across all 51 block=3 events), so the terminal decision is LUID diff = LUID(121)-LUID(111) = 12-8 = 4 > 0 -> 121 picked first at T-13 -> emission order 118,111,121 vs target 118,121,111. Any chain-slot removal that lowers 121 LUID or raises 111 LUID would flip the tiebreak.
- probe: Cross-referenced tmp/grind/cpu_side_move_dir_4/s6/csmd4_only.log lines 107-193 (sched1 block=3 initial enumeration and clock-by-clock PICKs) with the RTL chain in csmd4_only.lreg lines 415-458. Enumerated the four LUID slots between the residual pair: LUID 9 = NOTE_INSN (LINE_NOTE); LUID 10 = insn 116 (set p101 (plus p106 p79)), the t0-chain third stage 't0 = (u8*)tbl_125c + t0' that makes p101 multi-set (LOAD-BEARING: removing it collapses t0 to g3-family single-set spelling per s3-V10 masked=9 and s6 LUID-reorder masked=6); LUID 11 = insn 118 (set p75 (mem: ...v0+tbl)) = arg5 lw (LOAD-BEARING: 121 consumes p75 via RAW dep, cannot be removed and cannot follow 118 in linear).
- result: Every LUID slot in the gap enumerated to a specific insn identity. Real insns 116 and 118 are BOTH load-bearing for the h5 basin's identity (multi-set launch suppression on 116; arg5 data producer for 121 on 118). The NOTE_INSN at LUID 9 is a LINE_NOTE emitted by -g debug info generation and is not a C-source-accessible lever. Reload rearranges the chain post-sched1 (s43 cross-check: sched2 pushes 116 six positions later, collapsing the delta to 1 at LUID 111=6/121=7), but the pair-swap emission decision is already sealed at sched1 clock=13 and sched2 inherits the chain-order 118,111,121.
- verdict: KILLED

## [s51] The h5 basin's residual pair-swap is realized as a rigid three-insn chain-position triangle {111 SLL fresh p106, 116 PLUS multi-set p101, 121 PLUS fresh p107} with an inherent LUID delta of 4 sealed at expand-time; there is no C-source lever that keeps 111 as a LAUNCH target while shrinking the delta.
- mechanism: expmed.c case alg_shift (line 2244) hardcodes NULL_RTX target for the mult-by-power-of-2 shift, birthing fresh p106 (per s7 static walkthrough). expand's address lowering for arg5=*(v0+(s32)tbl_125c) produces a fresh PLUS p75+p79 -> p107 (single-set). Both dests are single-set at flow-time, so birthing_insn_p returns TRUE at 111 and 121, adjust_priority assigns LAUNCH sentinel 0x7f000001 to both. The multi-set p101 at insn 116 sits between them in chain order because its SET '(plus p106 p79)' occurs textually between the SLL and the arg5 chain in expand's linearization.
- probe: Ledger cross-read (synthesis modality within forensics contract): every C-source respelling measured across s3 (13 block-local structural variants), s5 (block-scope carriers + do-while(0) at 4 scopes), s6 (LUID reorder), s8 (honest idx_1495), s11/s12 (5 arg5_addr two-SETs), s39 (triple-set v0 carrier) either (i) preserves the triangle geometry -> masked=2 h5 basin, or (ii) breaks the triangle -> h5 falls to g3 basin (masked=6+ with unresolvable v1/a0 register exchange). No spelling reaches sub-2 masked.
- result: The three chain-position anchors are pinned to specific C statements: 111 <-> `t0 *= 4`; 116 <-> `t0 = (s32)((u8*)tbl_125c + t0)`; 121 <-> `arg5 = *(s32*)(v0+(s32)tbl_125c)`. Reordering any anchor's C position either falls out of h5 (basin flip) or is expand/cse-normalized to the same RTL (INERT). The geometry is a hard local minimum for pure-C.
- verdict: CONFIRMED

## [s52] The s51 live-frontier pointer-typed FAKE-annotated carrier axis is realizable: there exists an unaudited post-debug_printf-arm READ of &D_800A11DC (or of the (u8*)tbl_125c+(t0<<2) shape) that a fn-scope pointer carrier could reach as a distinct s6/s7 pseudo with legitimate semantic purpose, satisfying named-local-fake-exception prerequisites (i) reaches local-alloc as a live pseudo AND (ii) not add real emitted insns.
- mechanism: sched.c::adjust_priority + birthing_insn_p at sched1 clock=13 assigns LAUNCH 0x7f000001 to 111 (p106 fresh SLL dest, single-set via expmed.c:2244 case alg_shift NULL_RTX) and 121 (p107 fresh PLUS dest, single-set); LUID tiebreak picks 121 (LUID=12) over 111 (LUID=8) -> emission 118,111,121 vs target 118,121,111. If a fn-scope pointer carrier landed live-across-debug_printf in the s6/s7 hard-reg range (contrast s42 finding that s2/s4 have zero block=3 QTY overlap) via a legitimate downstream arm-side READ, its conflict list would include p100/p106/p107/p113 and could re-order local-alloc priorities on p106 vs p107, breaking 121's LAUNCH while preserving 111's, flipping the pair via a birthing_insn_p FALSE (not via LUID reorder or basin flip). Requires TWO simultaneous preconditions: (a) target-honest downstream reader for the FAKE carrier (semantic-purpose test in no-new-park-categories); (b) reader lands in a post-debug_printf arm so the pseudo's live range crosses block=3.
- probe: Static audit of asm/funcs/cpu_side_move_dir_4.s L80080DB0-L8008102C. `grep -nE 'D_800A11DC|D_800A125C' asm/funcs/cpu_side_move_dir_4.s`. Enumerate all D_800A11DC symbol references (both %hi/%lo lui/addiu address computations AND lw memory dereferences). Enumerate all post-L80080EDC-arm reads of $s3=tbl_125c after the debug_printf window (block=3 completion at L80080EC4).
- result: D_800A11DC: exactly TWO textual references at line 66 (lui $at,%hi(D_800A11DC)) and line 68 (lw $a2,%lo(D_800A11DC)($at)), forming ONE logical use site — the arg5 varargs slot 2 load feeding debug_printf. $a2 dies at the debug_printf CALL (caller-save). ZERO occurrences of D_800A11DC in the post-debug_printf arms (L80080EDC..L80081004). tbl_125c ($s3 = &D_800A125C): 2 pre-debug_printf reads at lines 58,63 (arg5/arg4 chains in block=3, matched by h5 candidate); ZERO $s3-based load or address-compute in the arms. ZERO occurrences of the shape (u8*)tbl_125c+(t0<<2) in the arms — reconfirms s47's F1 audit at the sibling &D_800A11DC symbol. The frontier's precondition (b) — unaudited post-arm reader — is empirically absent.
- verdict: KILLED

## [s52] Even if the s51 pointer-carrier precondition (b) were softened to allow a synthetically-added downstream reader (e.g., store *p_carrier to a global, or read via dispatch), the construct would still clear the layer-1 cheat-reviewer 'not add real emitted insns' bar.
- mechanism: A carrier whose only downstream read exists solely to justify the FAKE annotation has no target-byte correspondence; the reader is real emitted asm not present in target -> insn_count regresses AND masked distance regresses, matching the s47 C2 KILLED shape (+8 masked / +3 insns).
- probe: Cross-reference s47 C2 (fn_scope_zero_constant_holder_C2.c, masked=10 target=160 build=163 +3 insns) against the general form 'FAKE local with synthetic downstream reader' — the mechanism generalizes: any FAKE local demanding cross-call preservation OR a fabricated reader emits either a callee-save preservation cluster (C2's failure mode) or a new load/store (would fail no-new-park-categories semantic-purpose test).
- result: s47 C2 already measured the pointer-in-callee-save realization for a scalar carrier (+8 masked, +3 insns). Extension to a pointer type is strictly worse or equal (pointer stores/loads are same width, no advantage). The pre-vet against no-new-park-categories cheats-by-any-spelling fails on prong 'necessary only because permuter said so / defeat-of-optimization-only purpose' when the reader is fabricated. There is no C-legitimate variant of this axis that survives both prongs.
- verdict: KILLED

## [s53] m2c --reg-vars v0,v1,a0 output reveals the target-asm shape uses in-place addr-add on v0 (`var_v0 += &D_800A125C; var_v1 = *var_v0`). Mirroring this as C `v0 += (s32)tbl_125c; arg5 = *(s32*)v0;` (triple in-place set on v0) would leave insn 121's dest as multi-set at flow time → birthing_insn_p(121)=FALSE → LAUNCH loss → pair flips to target.
- mechanism: sched.c::birthing_insn_p LAUNCH sentinel gated on flow-time reg_n_sets==1 (per s6/s7 CONFIRMED). If v0 (=p101) gains a THIRD in-place set at insn 121, p101 becomes reg_n_sets=3, LAUNCH sentinel is not applied, and the LUID-tiebreak that put 121 before 111 no longer fires.
- probe: Replaced the h5 baseline arg5 leg `v0 <<= 2; arg5 = *(s32 *)(v0 + (s32)tbl_125c);` with `v0 <<= 2; v0 += (s32)tbl_125c; arg5 = *(s32*)v0;` (t0 leg unchanged, statement order otherwise identical to h5 candidate). Sandboxed via `sandbox cpu_side_move_dir_4 --disable all`.
- result: masked=15, target_insns=160, build_insns=160 (+13 vs h5 baseline of 2). The extra in-place set on v0 spreads the s-reg conflict web (idx_1494/idx_1495/idx_1496 s0/s2/s4 allocation) enough to disrupt the h5 chassis, regressing to a novel intermediate basin. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/s53_v0_triple_set_addr_walk.c.
- verdict: KILLED

## [s53] m2c --reg-vars output places the entire arg5 shift+add+load subseq BEFORE t0's shift+add subseq (novel: prior s2 E_arg5_first only flipped head lbus; this variant preserves head-lbu order but flips the whole subseq cluster). This statement-order transplant of the m2c-regvars line ordering may reach a novel h5-preserving basin.
- mechanism: sched.c ready-queue considers LUID; a fully-precedes-t0 arg5 subseq shifts each arg5 insn's LUID down (earlier), potentially altering which pair is LAUNCH-eligible or how the tiebreak resolves.
- probe: Applied ordering: `t0 = idx[0]; v0 = idx[1]; pp = &D_800F19C0; v0 <<= 2; v0 += (s32)tbl_125c; arg5 = *(s32*)v0; t0 *= 4; t0 = (s32)((u8*)tbl_125c + t0);` and sandboxed.
- result: masked=15, target_insns=160, build_insns=160 (+13 vs h5=2). Identical regression signature to the v0-triple-set-addr-walk sibling — indicates the s-reg web disruption dominates any potential LUID re-timing benefit. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/s53_m2c_regvars_verbatim_order.c.
- verdict: KILLED

## [s54] m2c with --void return-type hint produces a structurally distinct block=3 (debug_printf window) shape from s8's default output, exposing a novel arg4/arg5 spelling for h5-chassis grafting.
- mechanism: m2c dataflow analysis differs when return-type is asserted void — the return-value register (v0) is not treated as a live-out, potentially altering how the arg5/arg4 expression chain in block=3 is reconstructed (e.g., could shift v0's role in the reconstructed IR and expose a subscript-vs-pointer-arith spelling difference in the arg5 expression).
- probe: python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c --void --function cpu_side_move_dir_4 asm/funcs/cpu_side_move_dir_4.s > tmp/grind/cpu_side_move_dir_4/s54/m2c_void.txt; grep debug_printf tmp/grind/cpu_side_move_dir_4/s54/m2c_void.txt.
- result: block=3 window is line-identical to s8's default m2c output: debug_printf(&D_800161C8, D_800F19C0, *(&D_800A11DC + (D_800A11D5 * 4)), *((M2C_FIELD(&D_800A1494, u8 *, 0) * 4) + &D_800A125C), *((M2C_FIELD(&D_800A1494, u8 *, 1) * 4) + &D_800A125C)). --void alters ONLY the outer return-plumbing (line 76 area), never touches the debug_printf window's arg4/arg5 reconstruction. Confirms the block=3 window shape is a fixed point of m2c's expression-reconstruction pass, independent of the return-type flag.
- verdict: KILLED

## [s54] m2c with --no-stack-spill and --gotos-only produces a structurally distinct block=3 shape by suppressing structured-CFG reconstruction and stack-spilling heuristics, exposing a raw-form debug_printf call site with distinct arg4/arg5 spellings.
- mechanism: --gotos-only disables m2c's structured control-flow (if/else) reconstruction, emitting goto+label everywhere. --no-stack-spill disables detection of stack spilling to temporaries. Combined, these should produce a more asm-literal C shape where any hidden staged intermediate becomes explicit, potentially surfacing a v0/v1/a0-staging variant not seen in s8's default output.
- probe: python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c --no-stack-spill --gotos-only --function cpu_side_move_dir_4 asm/funcs/cpu_side_move_dir_4.s > tmp/grind/cpu_side_move_dir_4/s54/m2c_nostackspill_gotos.txt; inspected block_3 label region.
- result: block_3 label emits debug_printf(&D_800161C8, D_800F19C0, *(&D_800A11DC + (D_800A11D5 * 4)), *((M2C_FIELD(&D_800A1494, u8 *, 0) * 4) + &D_800A125C), *((M2C_FIELD(&D_800A1494, u8 *, 1) * 4) + &D_800A125C)) — line-identical to s8 default AND to --void output. --no-stack-spill affects only stack-slot temporaries (which csmd4's block=3 window doesn't use — the arg-marshal chain is register-only through the debug_printf CALL). --gotos-only affects only the outer poll-loop control-flow reconstruction (visible in the block_16/loop_8/block_17 gotos), never the block_3 arg-reconstruction. Confirms the block=3 shape's fixed-point property across CFG-reconstruction flags too.
- verdict: KILLED

## [s54] The h5 candidate.c applied to src/system.c reproduces the ledger-recorded masked=2 floor (baseline sanity check for s54's rederive attempt).
- mechanism: Overlay the block-scope {s32 arg5; s32 t0; void **pp; t0=idx_1494[0]; v0=idx_1494[1]; pp=(void**)&D_800F19C0; t0*=4; t0=(s32)((u8*)tbl_125c+t0); v0<<=2; arg5=*(s32*)(v0+(s32)tbl_125c); debug_printf(...);} onto src/system.c, replacing the HEAD both-named arg4/arg5 array-index form.
- probe: Edit src/system.c inline block to candidate.c spelling; & tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all
- result: score=2, target_insns=160, build_insns=160, scorable=true, rules_dropped=5, cheat_asm_stripped=22 (matches 45-session ledger; HEAD both-named form scores masked=7). H5 chassis confirmed live for s54 measurements.
- verdict: CONFIRMED

## [s55] The 54-session grand inventory (LUID reorder, class-attack, arg5_addr multi-set, p106 multi-set, do-while(0) at all scopes, duplicated-into-arms, SOTN named-local FAKE, ALLOCDBG s-reg coupling first-order, cross-function saEft01Init transplant, HI-LO producer / dead-frame-store carve-outs, ~85k-iter permuter neighborhood, 11-sub-angle rederive, static-analysis F2 helper extraction, outer-flow topology, pointer-typed FAKE post-arm reader, 7 idx_1495 respellings, cross-symbol idx_1495 retirement) has closed every C-source-reachable direct lever on the {sll4@54 <-> addu5@55} LAUNCH-vs-LAUNCH LUID-diff residual.
- mechanism: Cross-session synthesis: each closed lever has (a) a named GCC-pass mechanism (birthing_insn_p / expmed.c case alg_shift NULL_RTX / combine.c addsi3_internal substitution / integrate.c:96 varargs refusal / rank_for_schedule LUID-diff / mips.h ADJUST_COST) AND (b) at least one measured probe with sandbox floor >=h5 baseline or target-asm audit proving the prerequisite construct is absent. Ledger-level convergence, not per-session convergence.
- probe: Re-read evidence.md (933 lines, 54 sessions), hypotheses.md (961 lines, 160 hypothesis blocks), rejected/ (72 forms), task-brief rejected-forms bank (66 named). Cross-referenced closed levers against surviving frontier axes.
- result: Every axis touching the pair-swap either requires multi-set p101 (drops to g3 basin, KILLED) OR multi-set p107 at flow-time (combine.c substitution, KILLED) OR symmetric ref-lift on arg5 (misdirects monotonically, KILLED) OR a fn-scope carrier surviving to local-alloc (DCE'd/cse-folded when unused, preservation-cost when used, KILLED) OR class differentiation (val=0 across all events, KILLED) OR cost differential (ADJUST_COST never raises, KILLED). No first-order lever survives.
- verdict: CONFIRMED

## [s55] Three specific second-order lever probes remain honestly under-measured within the sanctioned frontier surface: (H1) sched2 post-reload LUID sub-slot enumeration at clock=10-15 on the residual sub-window, (H2) ALLOCDBG conflict-list-diff on idx_1495 direct-symbol vs adjacency-index respellings, (H3) LINE_NOTE(9) suppression via single-line condensation of the t0/arg5 statements. None re-derives any rejected form.
- mechanism: H1: s43 measured sched2 LUID delta collapse to 1 but did NOT enumerate the post-reload sched2-visible insn set at LUIDs 5-8 for shift-by-1 candidates. H2: s42 covered p78 ranking-slot stability but not conflict-list post-respelling with ALLOCDBG side-by-side. H3: sched.c LUID assignment walks the full RTL chain including LINE_NOTEs; s51 named a LINE_NOTE at LUID 9 between 111 and 121; condensing C statements onto a single physical source line collapses LINE_NOTE emission at cc1 -g, potentially shifting LUID(121) 12->11 without altering emitted RTL insns.
- probe: Deferred to s56+ ladder pass per synthesis-modality contract (no measurement this session).
- result: Frontier reset written to synthesis artifact; drafted probes are specific mechanism targets, not broad axis searches.
- verdict: CONFIRMED

## [s56] H3: Condensing the 4 residual-window statements (t0*=4; t0=(u8*)tbl+t0; v0<<=2; arg5=*(v0+tbl)) onto ONE physical source line suppresses the LINE_NOTE at sched1 LUID 9 (between insn 111@8 and insn 121@12), shifting LUID(121) from 12 to 11 without altering emitted RTL insns.
- mechanism: cc1 -g emits NOTE_INSN_LINE per source-line transition; sched.c LUID assignment increments for every insn including NOTE_INSN_LINE; per s51 LUID 9 = LINE_NOTE (not a real insn). Removing the LINE_NOTE via single-line C source should leave emitted RTL identical (line-notes are debug-only) but change only the LUID counter, reducing sched1 LUID delta 4->3.
- probe: s56 probe A: applied h5 candidate + collapsed the 4 residual-window statements onto ONE physical source line (semicolons-only, no newlines). sandbox cpu_side_move_dir_4 --disable all. s56 probe B (bracketing / opposite direction): re-expanded to 4 lines with blank lines between each to maximize any line-note emission. sandbox again.
- result: Probe A: masked=2, target_insns=160, build_insns=160, bytes identical to baseline. Probe B: masked=2, bytes identical. Both directions INERT vs h5 baseline. Makefile inspection (line 36): CC_FLAGS = -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w — NO -g flag in production. cc1 emits no NOTE_INSN_LINE in production; the LUID counter physically cannot be shifted by C-source physical-line manipulation. s51's LUID=9 LINE_NOTE observation must have been from an instrumented -da dump only.
- verdict: KILLED

## [s57] Hoisting block-3-scoped locals (arg5/t0/pp) to fn-scope declarations at the TAIL of the fn-body decl list changes their pseudo birth luid or global-vs-local allocno classification, shifting sched1 clock=13 LUID tiebreak between insns 111 (t0 mult SLL, LAUNCH) and 121 (arg5_addr PLUS, LAUNCH).
- mechanism: GCC 2.7.2 local-alloc.c allocates block-local qtys per basic-block; fn-scope decls of variables used in only one block are candidates for global-alloc pseudos (participate in .greg conflict lists with s-regs). If arg5/t0/pp become global-alloc pseudos, they gain distinct hardreg-class conflicts vs the h5 baseline where all three are pure block-local qtys (per s42 QTY table p100/p106/p107/p113 all block-local). This might shift 121's birth luid or LAUNCH signature at sched1.
- probe: Applied src/system.c fn-body decl list edit adding `s32 arg5; s32 t0; void **pp;` at TAIL (after `s32 temp;`), removed the same three decls from the block-3 inline scope, ran sandbox cpu_side_move_dir_4 --disable all.
- result: masked=2 (target_insns=160, build_insns=160). Bytes byte-identical to h5 baseline. GCC 2.7.2 assigns pseudo scope/lifetime from RTL first-use point, not from C decl scope — arg5/t0/pp still birth at their block-3 first-use insns (100, 111, 118) and stay block-local qtys regardless of fn-scope declaration.
- verdict: KILLED

## [s57] Hoisting block-3-scoped locals (arg5/t0/pp) to the TOP of the fn-body decl list (before `int new_var`) further shifts their pseudo ordinal in the initial RTL walk, biasing global-alloc priority computation via decl-declaration-order sensitivity.
- mechanism: Even if pseudo birth luid tracks RTL first-use, some GCC passes (init_reg_sets, flow_analysis pseudo indexing) walk in decl-encounter order. Top-of-fn-body slot might affect pseudo NUMBERING which in turn feeds ties in global-alloc priority ordering (ord=11/12/13 for the s2-s6 slot range per s42 evidence).
- probe: Applied src/system.c fn-body decl list edit moving `s32 arg5; s32 t0; void **pp;` to the FIRST three decls (before `int new_var; s32 v0; s32 cnt; ...`), same block-3 body as probe 1, ran sandbox --disable all.
- result: masked=2 (target_insns=160, build_insns=160). Bytes byte-identical to h5 baseline AND to probe 1. Fn-body decl-order is fully invariant vs the residual pair-swap — pseudo numbering does not affect the {111,121} LUID tiebreak at sched1 clock=13.
- verdict: KILLED

## [s58] A PERM_GENERAL macro wrapping the fn-body prologue statements (D_800F19B8/tbl_125c/idx_1494/idx_1495/D_800F19BC/D_800F19C0 initial stores at lines 448-453) opens a novel mutation neighborhood untouched by s22/s31/s32/s40/s49/s50 (which scoped to inline block, block+flanking-CALLs, or fell back to whole-fn random) and would surface a novel basin shifting sched1 clock=13 LUID tiebreak between 111 and 121.
- mechanism: Prologue statement order determines RTL-emit LUID for the initial stores that precede the loop entry; if the whole-fn LUID counter shifts by even 1 for a downstream insn before block=3, insn 121's LUID tiebreak vs 111 could flip. PERM_GENERAL adds AST-level mutations (return-type, expression-form, statement-form) that LINESWAP (s31 5!=120) does NOT explore.
- probe: Copied s50 workspace to tmp/grind/cpu_side_move_dir_4/s58/perm_outer_lineswap; removed s50's poll-loop PERM_GENERAL (parser-rejected inside compound-if); added PERM_GENERAL wrapper around the six fn-body prologue stores; launched via `python3 tools/permuter_campaign.py launch --func cpu_side_move_dir_4 --dir tmp/grind/cpu_side_move_dir_4/s58/perm_outer_lineswap --label s58_outer_prologue_gen -j 8 --stop-on-zero`.
- result: Permuter reported `No perm macros found. Defaulting to randomization.` — the fn-body-scope PERM_GENERAL wrapper was not detected by the directive parser (same failure class as s50 PERM_GENERAL inside compound-if). Campaign fell back to full-function random-mode with base_score=60 (h5 chassis preserved). Fresh-seed window: 18652 iterations / 689.4s / 0 finds_new / procs_killed=9 / stopped=true. No output-* dirs surfaced.
- verdict: KILLED

## [s58] Extending the h5 whole-fn random permuter closure by ~18k iters this session (on top of s50's 29k iters and s5/s13/s22/s23/s31/s32/s40/s49 prior campaigns) will surface any novel-basin closing form below weighted-40 not previously found.
- mechanism: Union coverage across all h5-chassis permuter campaigns: s5 g3-fresh 9040, s13 h5 2999, s22 lineswap 20429 (block), s31 lineswap 5040 (prologue), s32 perm_general 35338 (block), s40 perm_general 1738 (block+flanking-CALLs), s49 narrow arg5-only 15064, s50 poll-fallback-to-random 29289, s58 outer-prologue-fallback-to-random 18652 -> ~137k iters at h5 chassis. Any C-source-reachable weighted<40 basin would surface at some fresh seed across this sample.
- probe: s58 campaign metrics (base_score=60, finds_new=0, elapsed_s=689.4, iterations=18652) recorded via permuter-launch + permuter-harvest events in metrics/events.jsonl.
- result: Zero finds below weighted-60 across 18652 fresh-seed iterations; extends h5-chassis closure to ~103k+ total whole-fn random-mode iters at 0 novel <60 finds (137k+ counting all directed-scope campaigns). Per s55 CONFIRMED synthesis, permuter-modality is closed at h5 chassis without a NEW structural chassis; this session reconfirms.
- verdict: KILLED

## [s58] The permuter directive parser accepts PERM_GENERAL wrapped around outer function-body statements (not inside a nested block or compound-if).
- mechanism: tools/decomp-permuter/src/randomizer.py detect_perm_macros scans for PERM_* macro invocations at parse-parseable positions; empirical evidence from s50 (fail inside compound-if) and s58 (fail at fn-body outer scope) suggests only nested-block statement-chain wrappers or intra-expression sites parse.
- probe: s58 launch log line 4: `No perm macros found. Defaulting to randomization.` even though base.c contains a valid PERM_GENERAL(...) block around 6 fn-body statements ending before the `loop:` label.
- result: CONFIRMED tooling finding: PERM_GENERAL wrapping raw fn-body-outer statement chains is NOT recognized by the permuter directive parser under this repo's toolchain. Same failure mode as s50 compound-if. Future permuter probes at h5 chassis need directive placement inside a nested {} block scope, an expression, or use LINESWAP (which s22/s23/s31/s32/s40/s49 have already thoroughly covered).
- verdict: CONFIRMED

## [s59] H3: PERM_LINESWAP scoped inside a nested `{ }` compound wrapping the two poll-loop callback if-blocks bypasses the s50/s58 directive-parser fallback and exposes the poll-dispatch mutation neighborhood at h5 chassis.
- mechanism: s58 empirically showed pycparser/permuter parsing rejects PERM_LINESWAP at fn-body-outer scope (`no perm macros found. Defaulting to randomization`). Adding an explicit `{ }` compound-statement inside the existing `if (status != 0) { ... }` compound gives the AST a canonical block-item scope in which the PERM_LINESWAP macro is recognized as a multi-choice directive rather than a call expression.
- probe: Base.c edited so `if (status != 0) { { PERM_LINESWAP(<if&4 one-liner>\n<if&2 one-liner>) } goto poll; }`. Old fn-body PERM_LINESWAP and inline-block PERM_GENERAL removed to eliminate confounders. Campaign launched via tools/permuter_campaign.py launch, label s59_H3_poll_nested_lineswap, -j6.
- result: Startup log prints `Will run for 2 iterations.` (NOT the s50/s58 fallback text `No perm macros found. Defaulting to randomization.`) and `base score = 60`. iter1 score=60 (identity), iter2 score=125 (swap). Campaign exited on its own after 2/2 iters (elapsed 177.8s, 0 new finds, 0 crashes). Directed neighborhood exhaustively measured; swap ordering is a +65 weighted regression.
- verdict: KILLED

## [s60] H2: ALLOCDBG conflict-list side-by-side diff on p78 (idx_1495) across h5 baseline and the honest idx_1495 respelling reveals the exact GCC pass + decision producing the +13 sandbox delta (previously only qualitatively named as 's-reg web coupling').
- mechanism: The p78 CONFLICT LIST shape is identical between variants (15-way symmetric across debug_printf-window pseudos + hard-regs 2..7 + 29). The DIFFERENCE is upstream in the REG_NOTES chain: expand.c attaches a REG_EQUIV=(const (plus symref D_800A1494 1)) to insn 30 in the honest form (tree-level compile-time-constant address expression), and does NOT attach a REG_EQUIV to insn 38 in the baseline cross-symbol form (whose tree references the VAR_DECL tbl_125c, so the expression is not compile-time-constant at expand time even though RTL-CSE later folds it to the same shape). local-alloc.c::update_equiv_regs consumes REG_EQUIV to decide live-range strategy; with the note present, p78's livelen accounting doubles (72→144), priority halves (277→138), and global-alloc's priority-sorted queue demotes p78 to the LAST-allocated position (ord=13→ord=15). First-fit then assigns p78 to s6 instead of s4, and a coordinated 4-pseudo rotation places p72/p73 (params a0/a1) into the vacated s3/s4 slots. p79 (tbl_125c) additionally sheds 2 refs in the honest form and shifts to s5. This is the mechanism previously named qualitatively as 's2->s1 file-level s-reg ref-balance regression' in WIP rejected v16/v17.
- probe: Instrumented cc1 (BB2_QTY_DEBUG=1 BB2_SCHED_DEBUG=1 BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=all BB2_RANK_DEBUG=1) on both variants; extract cpu_side_move_dir_4 section from .greg (ALLOCDBG allocno table + '16 regs to allocate' + conflict lists) and .lreg (RTL with REG_NOTES); side-by-side diff of p78 conflict list, alloc order, livelen, priority, disposition; correlate to sandbox measurements (baseline masked=2, honest masked=15).
- result: Mechanism named at the GCC pass + decision level. See tmp/grind/cpu_side_move_dir_4/s60/FORENSICS.md for the full quartet table and RTL fragments. C-source lever consequence: any idx_1495 respelling with a tree-level compile-time-constant address (e.g., idx_1494+1, &D_800A1494[1], (u8*)0x800A1495) triggers the +13 alloc web rotation via REG_EQUIV attachment; the h5 cross-symbol form is the ONLY tested respelling that defeats the REG_EQUIV attachment.
- verdict: CONFIRMED

## [s61] An alternate natural honest respelling of idx_1495 (integer-cast, direct-symref, or var-mediated cast) defeats REG_EQUIV attachment at expand.c time and lets us retire the cross-symbol semantic-lie without the +13 regression.
- mechanism: expand.c::set_unique_reg_note attaches REG_EQUIV on p78's SET when the tree-level expression is a compile-time-constant address. If an intermediate CAST_EXPR (integer cast) or var-substitution breaks the tree-time recognition, the note would not attach and the p78 alloc-web rotation would not fire.
- probe: Applied 4 respellings on h5 base: P1 &D_800A1495 (masked=16, build_insns=161, +1 insn); P2 1+idx_1494 (masked=15); P3 (u8*)((s32)&D_800A1494 + 1) (masked=15); P4 (u8*)((s32)idx_1494 + 1) (masked=15). All four measured via sandbox --disable all.
- result: P1 regresses further with an EXTRA insn because &D_800A1495 emits a fresh lui/addiu pair (a distinct symbol) instead of an addiu on p77; the REG_EQUIV note ALSO attaches. P2/P3/P4 all produce identical masked=15 = the s8-probe1 baseline; the intermediate CAST_EXPR does NOT block set_unique_reg_note (constness recognition happens after CSE substitutes var-mediated forms back to their ADDR_EXPR sources).
- verdict: KILLED

## [s61] The wider H3 frontier — that a companion lever exists to offset the REG_EQUIV alloc-web rotation without breaking h5 — is realizable via natural additional uses of a0/a1/tbl_125c that raise p72/p73/p79 priority above p78's demoted 138.
- mechanism: Local-alloc priority = floor_log2(nrefs) x nrefs x 10000 / livelen. Adding natural uses of a0 or a1 within the function's stated semantics could raise their nrefs, or shortening p72/p73 livelen could raise their pri above p78=138.
- probe: Structural review of the function body: a0 has ONE natural use (return-zero-if-nonzero check); a1 has TWO natural uses (nonzero check + dst init in the 7-byte copy loop). Both param uses are already at their function-body natural livelen (prologue-to-last-use). No C construct can shorten them without dropping the semantic behaviour. p79 (tbl_125c) shed refs 5->3 only because the honest respelling stops deriving idx_1495 through tbl; further ref-shedding on p79 requires stopping tbl_125c derivation of arg4/arg5 - which regresses the entire h5 basin.
- result: No natural-C lever exists on the function's stated semantics to raise p72/p73/p79 priorities above p78's demoted 138.
- verdict: KILLED

## [s62] m2c on the file-local twin marionation_Exec.s (never in ledger; s9 transplanted the committed C form, not m2c's output of marionation's asm) surfaces a structurally novel block=3 shape distinct from csmd4's m2c-derived inline-all form.
- mechanism: m2c reconstructs C from RTL-recovered dataflow; distinct target-asm shape for marionation (extra idx_1496 = idx_1494 + 2 pointer, distinct D_80016248 vs D_80016240 constant) could steer m2c's expression-tree reconstruction into a novel block=3 form.
- probe: python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c --function marionation_Exec asm/funcs/marionation_Exec.s > tmp/grind/cpu_side_move_dir_4/s62/m2c_marionation.txt
- result: m2c emits `debug_printf(&D_800161C8, D_800F19C0, *(&D_800A11DC + (D_800A11D5 * 4)), *((M2C_FIELD(&D_800A1494, u8 *, 0) * 4) + &D_800A125C), *((M2C_FIELD(&D_800A1494, u8 *, 1) * 4) + &D_800A125C));` for marionation's block=3 - byte-identical expression form to csmd4's m2c output at s8/s54. The auxiliary temp_s3 = &D_800A1494 + 2 pointer is emitted for OTHER blocks (post-window arm access at temp_s3-1) but does NOT propagate into block=3's debug_printf reconstruction. m2c is basin-blind AND basin-invariant across sibling asm at the block=3 shape.
- verdict: KILLED

## [s63] m2c --reg-vars s0,s1,s2,s3,s4,s5 (pinning ALL fn-scope s-regs as m2c variables, un-tried in s8/s53/s54/s62) surfaces a novel structural reconstruction distinct from the inline-all-args basin.
- mechanism: m2c's --reg-vars flag disables temp/phi generation for the named registers and forces single-variable naming; pinning the full s-reg set (rather than none as in s8, or just s4 idx_1495-addr as in s53) forces the reconstructor to preserve the RTL s-reg lifetimes as C-variable lifetimes, which could either (a) collapse redundant SET-INSNs into a novel prologue shape or (b) split the debug-window arg materialization across additional named locals.
- probe: python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c --function cpu_side_move_dir_4 --reg-vars s0,s1,s2,s3,s4,s5 asm/funcs/cpu_side_move_dir_4.s > tmp/grind/cpu_side_move_dir_4/s63/m2c_regvars_sregs.txt (80 lines emitted).
- result: The debug_printf call materializes fully-inlined as `debug_printf(&D_800161C8, D_800F19C0, *(&D_800A11DC + (D_800A11D5 * 4)), *((M2C_FIELD(&D_800A1494, u8*, 0) * 4) + &D_800A125C), *((M2C_FIELD(&D_800A1494, u8*, 1) * 4) + &D_800A125C))` -- byte-identical shape to s8's inline-all-args reconstruction (evidence.md s8 probe2), which is the WIP-recorded score-14 basin (evidence.md L7). The fn prologue also shows spurious duplicate SET-INSNs (`var_s3 = &D_800A125C; var_s3 = &D_800A125C; var_s2 = &D_800A1494; var_s2 = &D_800A1494;`) -- m2c hallucination from the s-reg-pinning forcing repeated names to redundant sets, not evidence of a novel source-shape (a duplicated fn-prologue assignment is a straight-line dead-store cheat, not the sanctioned duplicated-statement-into-arms carve-out which requires 2+ control-flow arms per no-new-park-categories).
- verdict: KILLED

## [s64] Every ledger-visible C-source lever direction on the h5 {sll4@54 <-> addu5@55} residual pair-swap has been mapped to a compiler-source wall (expmed.c:2244 case alg_shift NULL_RTX, mips.h:2946 ADJUST_COST, combine.c addsi3_internal, integrate.c:96 varargs refusal, sched.c LUID counter without -g) or an empirical regression across 63 sessions.
- mechanism: Grand inventory: LUID reorder basin-coupled (s6); class attack triple-KILL (s15/s16); p106 multi-set expmed-pinned (s7); p107 two-SET combine-folded (s11/s12/s39); do-while(0) at 5 scopes wrong-direction (s4/s5/s25); duplicated-into-arms non-t0 wrong-direction or fn-scope-cheat (s20 + rejection-bank); SOTN named-local FAKE C1-C6 all KILLED (s47/s48); permuter modality ~137k iters 0 <40 finds (s5/s13/s22/s23/s31/s32/s40/s49/s50/s58); rederive 11 sub-angles KILLED (s8/s9/s17/s18/s26/s27/s35/s36/s44/s45/s54/s62/s63); ALLOCDBG s-reg coupling REG_EQUIV attachment mechanism NAMED and no-C-offset (s60/s61).
- probe: Ledger cross-read (evidence.md 1066 lines, hypotheses.md 1046 lines / 160 blocks, rejected/ 78 forms).
- result: The h5-basin closed-lever set is COMPLETE at first-order structural + permuter + forensics + rederive modalities.
- verdict: CONFIRMED

## [s64] F1 (novel): a g3-chassis (base_score=40, statement-form t0<<=2) directed permuter campaign scoped strictly to arg5-side (PERM_LINESWAP arg5 statements + PERM_GENERAL arg5 dereference ONLY, NO t0-side directives) can shift REG_N_REFS on arg5-qty above the ~5000 priority threshold (s1 fable-blitz refs>=4 span 6 = 13333, or refs=3 span<=5 = 6000), flipping the v1/a0 register exchange residual in g3=6 basin without crossing back into h5-basin territory.
- mechanism: s5 g3 fresh-seed unrestricted (9040 iters) was NOT arg5-only; s14 h5-chassis directed at both sides drifted into g3 via alternative t0 forms. s19 elevated this exact axis as frontier #3, never executed. g3 has an order-perfect {sll v0, addu v0, sll a0} triangle so t0-side is untouchable; only arg5-side pri-lift can reach a distinct alloc.
- probe: Not measured this session (synthesis modality mandates no probes). Elevated to frontier #1 for s65+.
- result: Guardrail: iter1 base_score MUST measure 40 at launch (g3 chassis); layer-1 vet against g3-permuter cheat family (0xFF/0xFFFF masks, u16 narrowing, fn-scope alias) on every find. Fresh-seed 20-30 min wall time.
- verdict: CONFIRMED

## [s64] F2 (low-probability, deterministically enumerable): 3-statement PERM_LINESWAP inside a nested { } compound inside if(status != 0) covering (if&4 dispatch, if&2 dispatch, goto poll) enumerates 3! = 6 orderings; extends s59 2-if-swap negative to include the goto slot; targets ALLOCDBG s-reg web at the poll-loop dispatch scope.
- mechanism: s59 established nested { } inside compound-if IS parsed by permuter (2/2 iters ran; swap regresses +65). s42 shows p78/idx_1495 have zero block=3 QTY overlap, so lift is unlikely, but 6-combo enumeration is <2 min tool time; measures full statement-order axis at previously-inaccessible dispatch scope.
- probe: Not measured this session. Elevated to frontier #2 for s65+.
- result: Low upside, deterministic enumeration, low tool cost.
- verdict: CONFIRMED

## [s64] F3 (closure by construction, not a probe): no third live first-order lever survives at h5 chassis without owner-supplied external input or a new structural chassis.
- mechanism: SOTN carve-out realizations exhausted (named-local-fake-exception C1-C6, dead-vars-local-array requires oracle-honest dead sw stores absent per s49, do-while-zero all scopes wrong-direction); external-corpus rederive angles enumerated to exhaustion (s44/s45/s26/s17 all KILLED); ALLOCDBG axis fully named with no C-offset lever (s60/s61); Kengo body extraction empty across 64 nm_cpu.c stubs (s18/s36).
- probe: Ledger cross-read; no measurement.
- result: Documented honestly; no ruling-request emitted (judge constraint 2026-07-08 bars canonical-asm authorization framing).
- verdict: CONFIRMED

## [s65] Replacing `t0 *= 4` with two chained `t0 = t0 + t0;` PLUS statements routes expansion through expand_binop's PLUS path (rather than expand_mult case alg_shift), altering insn 111's LAUNCH signature by producing two PLUS insns whose dests may or may not be single-set; predicted a distinct basin from *=4 (h5) and <<=2 (g3).
- mechanism: expand_binop with target=t0 (l-value) emits `set p101 (plus p101 p101)` in-place -> p101 becomes multi-set for the first add. Second add likewise. This should defeat any launch on the t0-side shift-equivalents.
- probe: Applied t0 = t0 + t0; t0 = t0 + t0; in place of t0 *= 4 on the h5 candidate; sandbox --disable all.
- result: masked=15, target_insns=160, build_insns=160. +13 vs baseline 2. Regressed hard.
- verdict: KILLED

## [s65] Making v0 the explicit multi-set dest of the arg5_addr PLUS (`v0 = v0 + (s32)tbl_125c; arg5 = *(s32*)v0;`) forces expand to emit `set p_v0 (plus p_v0 tbl)` with p_v0 multi-set, defeating LAUNCH on insn 121 (which is currently LAUNCH via a fresh single-set p107 dest).
- mechanism: birthing_insn_p gates LAUNCH on reg_n_sets(dest)==1. Reusing v0 (already set at load + at <<=2) as the PLUS dest makes reg_n_sets(p_v0)==3, so insn 121 loses LAUNCH; the priority tie with insn 111 collapses, potentially flipping the pair.
- probe: Applied `v0 <<= 2; v0 = v0 + (s32)tbl_125c; arg5 = *(s32*)v0;` on the h5 candidate; sandbox --disable all.
- result: masked=15, target_insns=160, build_insns=160. +13. Same regression signature as P1.
- verdict: KILLED

## [s65] Splitting the t0-chain tail PLUS to a fresh block-local `s32 t0_addr = (s32)((u8*)tbl_125c + t0);` gives insn ~117 (tbl+t0 PLUS) LAUNCH (via reg_n_sets(t0_addr)==1); alters the tiebreak against insn 121 by putting a third LAUNCH insn in the ready-queue window.
- mechanism: Fresh single-set pseudo t0_addr replaces multi-set t0 as the PLUS's dest; insn corresponding to `tbl+t0` transitions NON-LAUNCH -> LAUNCH; three-way priority tie at clock=13 in backward sched with new luid ordering.
- probe: Applied `s32 t0_addr; ...; t0_addr = (s32)((u8*)tbl_125c + t0);` with call reading *(s32*)t0_addr on the h5 candidate; sandbox --disable all.
- result: masked=9, target_insns=160, build_insns=160. +7. Novel intermediate basin (distinct from h5=2, g3=6/7, +13 collapse basin, and inline-all=14).
- verdict: KILLED

## [s66] Simultaneous multi-set-forcing of BOTH sides (t0<<=2 in-place + v0=v0+tbl in-place) produces a symmetric priority cancellation at clock=13: neither insn 111 nor insn 121 LAUNCHes, ready queue falls back to normal-priority ordering, potentially reaching a novel low-launch basin unreachable by either single-side collapse.
- mechanism: sched.c adjust_priority/birthing_insn_p gates LAUNCH on flow-time reg_n_sets==1 of SET dest. In-place `t0<<=2` routes expand_binop with target=p101 (multi-set, non-LAUNCH per s7 CONFIRMED). Explicit `v0=v0+tbl` in-place makes p107 multi-set (non-LAUNCH). If neither insn LAUNCHes at clock=13, scheduler's ready queue tiebreak follows normal priority path, hypothetically clean.
- probe: Applied `t0 <<= 2; t0 = (s32)((u8*)tbl_125c + t0); v0 <<= 2; v0 = v0 + (s32)tbl_125c; arg5 = *(s32*)v0;` on h5 candidate; sandbox cpu_side_move_dir_4 --disable all. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/f3_compound_dual_collapse.c.
- result: masked=16, target_insns=160, build_insns=160 (+14 vs h5 baseline of 2). Strictly WORSE than either single collapse (s65 P1=+13, s65 P2=+13). Compound dual-collapse does NOT cancel to a novel low-launch basin; instead lands in a fresh +14 basin never previously measured. The +13 collapse basin is single-side-driven, not compositionally symmetric.
- verdict: KILLED

## [s67] F1: Directed permuter on the g3-chassis with PERM_LINESWAP wrapping the arg5 3-statement chain plus PERM_GENERAL enumerating 4 semantic-equivalent spellings of the arg5 dereference (v0+tbl, tbl+v0, ((s32*)((u8*)tbl+v0))[0], *(s32*)((u8*)tbl+v0)) reaches a novel arg5-qty pri>=5000 profile closing the v1/a0 exchange in g3=6 basin.
- mechanism: PERM_GENERAL emits distinct RTL trees for the operand order swap and the s32*/(u8*) cast paths, potentially altering combine.c's expand-time fold that produces the arg5 addu; PERM_LINESWAP re-times qty birth luids on v0's staging chain. Cross-product 4x6=24 orderings enumerated deterministically.
- probe: tmp/grind/cpu_side_move_dir_4/s67/perm_g3_arg5general/ workspace: base.c is g3-chassis (t0 <<= 2 in-place, arg5 addu in the call) with PERM_LINESWAP(v0=idx[1]; v0<<=2; arg5=PERM_GENERAL(4 spellings);). tools/permuter_campaign.py launch -j 6 --stop-on-zero; 24 iterations / 66.2s.
- result: 0 novel finds. base_score=40. iteration-24 (identity mutation) hit 40. best non-base score 50 (iters 13, 15, 17 - three PERM_LINESWAP orderings paired with the same PERM_GENERAL alt). All other mutations landed at 140/150/1290/1490.
- verdict: KILLED

## [s67] F2: PERM_LINESWAP wrapping the 3 statements inside 'if(status != 0){}' in the poll region (the if(status&4){...}, if(status&2){...}, goto poll; block) enumerates 3!=6 orderings; ALLOCDBG s-reg web coupling may surface at previously-inaccessible dispatch scope.
- mechanism: s59 established that nested-compound PERM_LINESWAP inside if(status!=0){} parses correctly (2/2 iters ran). p78/idx_1495 zero block=3 QTY overlap makes upside small but enumeration is cheap (3!=6 combos). If any ordering shifts s-reg web assignment for the arg5-chain, the residual pair could flip.
- probe: tmp/grind/cpu_side_move_dir_4/s67/perm_poll_lineswap/ workspace: base.c is g3-chassis (no PERM on arg5) with PERM_LINESWAP wrapping the 3 poll statements. tools/permuter_campaign.py launch -j 6 --stop-on-zero; 6 iterations / 120.4s.
- result: 0 novel finds. base_score=40. iteration-6 (identity) hit 40. best non-base 105 (iter 5). Others 1825/2090/3690/3690. All non-identity orderings strictly regressed the score; the original poll ordering IS the score-minimum shape on the g3 chassis.
- verdict: KILLED

## [s68] F7: A whole-block PERM_LINESWAP over the 5 flexible do_timeout statements (v0=idx[1]; pp=...; t0*=4; t0=tbl+t0; v0<<=2) crossed with PERM_GENERAL enumerating 4 semantically-equivalent spellings of the arg5 dereference (v0+tbl, tbl+v0, cast-index [0], u8*+cast) surfaces a novel-basin closing form on the h5 chassis, satisfying both the head-load luid constraint and the arg5-side LAUNCH suppression.
- mechanism: PERM_LINESWAP enumerates 5! = 120 statement orderings deterministically (permuter cross-product mode); combined with PERM_GENERAL's 4 arg5 spellings = 480 iters total. Interior statement re-association could re-time qty birth LUIDs (sched.c ready-queue ordering) so that either insn 111 LAUNCH shifts down the LUID axis or insn 121's data-producer (v0<<2 vs (v0+tbl)) re-associates through a distinct expand-time RTL. PERM_GENERAL specifically tests whether combine.c's addsi3_internal substitution on the arg5 addu is dependent on the tree-time ordering of PLUS operands or of the cast-and-index sugar path.
- probe: Copied s67 g3-workspace to tmp/grind/cpu_side_move_dir_4/s68/perm_wholeblock/ and rewrote base.c's do_timeout inline block to h5 chassis (t0 *= 4; t0 = (s32)((u8*)tbl_125c + t0); v0 <<= 2; arg5 = *(...)) with `t0 = idx_1494[0]` kept above the LINESWAP block and the debug_printf below. Wrapped 5 flexible statements in PERM_LINESWAP: {v0=idx_1494[1]; pp=(void**)&D_800F19C0; t0*=4; t0=(s32)((u8*)tbl_125c+t0); v0<<=2}. Wrapped arg5 assignment in PERM_GENERAL: {*(s32*)(v0+(s32)tbl_125c), *(s32*)((s32)tbl_125c+v0), ((s32*)((u8*)tbl_125c+v0))[0], *(s32*)((u8*)tbl_125c+v0)}. Launched via tools/permuter_campaign.py launch -j 6 --stop-on-zero, label s68_h5_wholeblock_lineswap_general. Harvested with --stop.
- result: 480 iterations / 96.2s / 0 novel finds. base_score=60 (h5 chassis confirmed, base = masked=2). Best non-base score observed in log = 60 (identity-equivalent reordering ties, many iters); NO output-<N>-* dir surfaced below 60. Score histogram from log: many iters at 60 (identity ties), 110/120/200 (mild regressions), 750-800 (severe regressions from broken dependency chains where the LINESWAP put a use before the def). The deterministic 5!*4=480 enumeration is COMPLETE — every valid re-timing of the 5 flexible statements crossed with every semantically-equivalent spelling of the arg5 deref has been measured, none reaches weighted<60.
- verdict: KILLED

## [s69] The +13 masked alloc-web rotation from the honest idx_1495 respelling contains at least one edge NOT reducible to s60's REG_EQUIV / qty_compare priority perturbation (F5 residual-lever hypothesis).
- mechanism: F5 posited a second lever within global-alloc's per-pseudo state (refs, livelen, priority, birth_luid, assigned_reg) that could flip ONE s-reg's ref-count parity without changing insn count. The frontier was the 15-way pseudo table diff of cpu_side_move_dir_4 across the two spellings.
- probe: Built ALLOCDBG/QTYDBG/SCHEDDBG-instrumented cc1 dumps for h5 (src/system.c line 406 = cross-symbol form, masked=2) and honest (line 406 = idx_1495=idx_1494+1, masked=15). Extracted per-function greg register-dispositions and per-pseudo ALLOCDBG rankings; diffed the 15-way pseudo table and the block-0 insn extent. See tmp/grind/cpu_side_move_dir_4/s69/{h5,honest}/csmd4.greg.csmd4 and allocdbg_diff/{h5,honest}.{alloc,qty,findreg}.txt.
- result: The debug_printf window pseudos (t0-chain / arg5-carrier / mult4-fresh-temp / arg5_addr) allocate IDENTICALLY across both dumps: h5 p101->a0, p106->a0, p107->v0; honest p97->a0, p102->a0, p103->v0 (pseudo numbers shift by 4 due to fewer prologue expands, hardregs identical). The entire +13 delta reduces to a 4-cycle rotation on s-reg CARRIERS in the function body: p72 s5->s3, p79 s3->s5 (swap); p73 s6->s4, p78 s4->s6 (swap). Block-0 insn extent: h5 [4..46]=43 insns, honest [4..38]=35 insns (honest's shorter arithmetic saves 8 prologue insns via expand-time constant folding on `&D_800A1494+1`). Every insn body-wide that references p72/73/78/79 emits a different s-reg byte, accounting for the +13 masked score. No unnamed pseudo-state edge surfaced: refs/livelen/pri differences on p72/73/78/79 track exactly with what expand.c's set_unique_reg_note (s60's mechanism) would perturb via qty_compare's priority tie-breaking (livelen delta from shorter prologue). The 4-cycle rotation is ATOMIC under qty_compare's transitive priority ordering - flipping one edge requires changing an inequality, which necessarily perturbs the other three s-reg carriers coupled through the same priority chain.
- verdict: KILLED

## [s70] The 8-insn prologue insn-count delta between h5 (43 insns block-0) and honest (35 insns block-0) propagates a NOTE_INSN_LOOP_BEG or LUID offset that perturbs birthing_insn_p classification / ready-queue tiebreaks within block=3 (the debug_printf residual-pair-swap window).
- mechanism: If a prologue-side NOTE_INSN_LOOP_BEG is inserted at expand-time, it would participate in flow.c's LUID assignment and potentially shift the relative LUID of debug_printf-window insns, offsetting all subsequent LUIDs by ~8. This could plausibly re-time SCHED_TIME assignments for the {sll@T-14, plus@T-13} pair-swap residual.
- probe: Directly diff s69's h5 vs honest_idx_1495 sched2 dumps at block=3 (tmp/grind/cpu_side_move_dir_4/s69/{h5,honest}/csmd4.sched2.csmd4 lines 101-153). Compare (a) total_time, (b) per-insn priority + ref_count for the residual-pair triad, (c) ready-list dispatch order at T-15 through T-13, (d) linear emission order for the pair.
- result: IDENTICAL block=3 shape between forms. total_time=20 both. Priority+ref_count on residual triad match exactly (h5 insn 118 pri=2 ref=3 <-> honest 110 pri=2 ref=3; h5 111 pri=2 ref=2 <-> honest 103 pri=2 ref=2; h5 121 pri=2 ref=4 <-> honest 113 pri=2 ref=4). Insn numbers offset by 8 (matches prologue delta) but relative LUIDs preserved. T-13 tiebreak picks higher-LUID PLUS-insn on BOTH forms (121 or 113 before SLL 111 or 103). Backward-scheduled emission linearizes to {arg4-lw, SLL, PLUS} on both — the residual pair-swap is present in BOTH forms; the honest form's +13 delta comes entirely from s60/s61/s69's s-reg rotation, not from block=3 scheduling.
- verdict: KILLED

## [s71] The BB2-toolchain decomp.me corpus (3754 scratches, gcc2.7.2-psx/cdk + psyq3.5) contains a scratch whose target asm shingles overlap the residual-window slice (asm/funcs/cpu_side_move_dir_4.s:53-73) at sufficient similarity (>=0.20 threshold) to seed a novel closing spelling for the h5 pair-swap residual — a WINDOW-SLICE search is distinct from s17's whole-function scan and could surface analogs the whole-function similarity metric averages out.
- mechanism: shingle-based asm k-gram similarity search over the 20-insn debug_printf tbl-indexed dual-load residual (dual lbu + dual sll-by-2 + dual addu-into-tbl + dual lw-varargs-store + fmt-la-last + CALL). Whole-function similarity dilutes the window signal across ~176 target insns; window-only shingling amplifies structural analogs at k=3 (denser overlap) and k=4 (stricter). If a real scratch has an analogous double-tbl-indexed varargs-CALL, its committed C would show a spelling (arg-narrowing, alternate deref parenthesization, tbl-cast form) not surfaced by the m2c flag-space (s54) or the marionation sibling (s9/s62).
- probe: tools/decomp_me_scrape.py search --asm-file tmp/grind/cpu_side_move_dir_4/s71/window.s --top 15 --shingle-k 3 (dense) and --shingle-k 4 (strict), on the extracted 20-insn residual-window slice (window.s at tmp/grind/cpu_side_move_dir_4/s71/window.s). Inspected top-2 hits' committed C source for varargs-CALL analog shape.
- result: Top-15 at k=3: max similarity 0.133 (FieldPartyMemberIncreaseGearHp, gcc2.7.2-psx), second 0.120 (GameCharacterGetGearID). Top-15 at k=4: max similarity 0.043 (same functions rank identically). Inspected FieldPartyMemberIncreaseGearHp: g_GameState->gears[gearId].hp += amount (single struct-member RMW, no varargs, no dual-tbl-index, no debug_printf). Inspected GameCharacterGetGearID: single-line struct-member getter. No analog found; all top-15 hits are BB2-unrelated helpers whose shingle overlap is coincidental (shared lbu/sll/addu instruction bigrams from generic struct-field access, not the tbl-indexed varargs shape).
- verdict: KILLED

## [s72] m2c on saEft01Init.s (the THIRD BB2 twin per s26, never m2c'd — s8/s53/s54/s63 m2c'd csmd4.s, s62 m2c'd marionation_Exec.s) produces a structurally novel C shape for the debug_printf window not present in the prior m2c outputs, exposing a new rederive neighborhood.
- mechanism: m2c reconstructs original-compiler-shaped C from RTL-recovered dataflow. saEft01Init's ambient ALLOCDBG differs from csmd4/marionation (s33: refs=6 vs refs=2-4, luid-span=32 vs 14, zero sched2 LAUNCH sentinels), so m2c's expression-tree-flattening walker might elect distinct fold vs stage decisions on saEft01Init's asm vs the csmd4/marionation asm, producing a novel spelling.
- probe: python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c --function saEft01Init asm/funcs/saEft01Init.s; artifact tmp/grind/cpu_side_move_dir_4/s72/m2c_saeft01init.txt; compare debug_printf window shape to s8/s62 outputs and to rejected forms bank (inline-all-args = masked=14 WIP L7).
- result: m2c emits `debug_printf(&D_800161C8, D_800F19C0, *((D_800A11D5 * 4) + &D_800A11DC), *((M2C_FIELD(&D_800A1494, u8 *, 0) * 4) + &D_800A125C), *((M2C_FIELD(&D_800A1494, u8 *, 1) * 4) + &D_800A125C))` — direct-inlined-all-args, no local staging, no arg4/arg5 named intermediates. Byte-identical structure to s8's csmd4 m2c output and s62's marionation_Exec m2c output at the block=3 debug_printf call. m2c's if/else-flattening of saEft01Init's fast-path is orthogonal to the pair-swap window (upstream of the mult-expander LAUNCH mechanism). The already-measured inline-all-args basin (masked=14, WIP evidence.md L7; also confirmed on csmd4 by s8, s9 P2 masked=14) is the exact shape m2c reconstructs.
- verdict: KILLED

## [s73] The h5-basin closed-lever set is COMPLETE at every first-order structural / permuter / rederive / forensics modality (s3-s72 cumulative), leaving only sanctioned SOTN carve-outs or operator-dependent external rederive as un-run frontiers.
- mechanism: Grand inventory across 72 sessions: h5/g3 basin non-composable via expmed.c:2244 case alg_shift NULL_RTX wall (s7 CONFIRMED); p107 two-SET closed 5 ways via combine.c addsi3_internal fold (s11/s12); do-while(0) 5 scopes wrong-direction (s4/s5/s25); named-local-fake-exception C1-C6 KILLED (s47/s48); permuter ~137k iters 0 novel <40 finds (s5/s13/s22/s23/s31/s32/s40/s49/s50/s58/s67/s68); rederive 12 sub-angles KILLED (s8/s9/s17/s18/s26/s27/s35/s36/s44/s45/s54/s62/s63/s71/s72); REG_EQUIV s-reg 4-cycle rotation atomic under qty_compare, block=3 prologue-invariant (s60/s61/s69/s70).
- probe: Ledger cross-read only: evidence.md 1200 lines, hypotheses.md 1135 lines / >170 hypothesis blocks / all verdicts CONFIRMED or KILLED, rejected/ 84 forms, artifacts across 72 session sub-dirs.
- result: Every hypothesis in the ledger post-s10 has a terminal verdict. No untried first-order lever surfaces on re-read. Only two attack layers remain live: (A) SOTN sanctioned carve-out with FAKE annotation + layer-2 cheat-reviewer approval on a NON-arg5 co-live carrier per s69 greg, (B) operator-dependent Ghidra rederive.
- verdict: CONFIRMED

## [s73] The un-run F6 frontier (SOTN duplicated-statement-into-arms) requires a DIFFERENT carrier than arg5's v0 stage — arg5's v0=idx_1494[1] is set INSIDE the do_timeout block, so cross-arm duplication of the arg5 chain is byte-non-neutral (s10 CONFIRMED). Per s69 ALLOCDBG greg, the co-live-at-block=3-entry pseudos derivable outside the do_timeout arms are p73/p79 (tbl_125c) and p77 (idx_1494 addr).
- mechanism: duplicated-statement-into-arms carve-out targets reg_n_refs on a specific carrier without adding runtime side-effects, via cross-jump re-merge collapsing the duplicated instructions post-optimization. Applied to p73/p77/p79 rather than arg5's v0, it lifts THEIR refs — the s60 mechanism named p78's demoted priority=138 as the pivot, so raising p72/p73/p79 refs above p78 would offset the 4-cycle rotation without triggering the +13 masked regression.
- probe: Not measured this session (synthesis modality). Precondition for the s74 measurement: draft candidates duplicating a byte-neutral load of tbl_125c or an idx_1494-addr reference into BOTH do_timeout arrival arms (the success:->do_timeout: fallthrough and direct-goto-do_timeout: paths), verify byte-neutrality via cross-jump re-merge inspection, then measure via sandbox --disable all with FAKE annotation and pre-arranged layer-2 cheat-reviewer.
- result: Frontier documented, mechanism named at pseudo-level per s69 greg data. Elevated to F6 in reset frontier.
- verdict: CONFIRMED

## [s73] The un-run F9 frontier (prologue-reorder to equalize refs/livelen on the 4-cycle rotation carriers) exposes qty_compare's LUID-tiebreak axis, potentially yielding a novel s-reg assignment distinct from both h5 and honest 4-cycle rotations.
- mechanism: qty_compare tie-break in local-alloc.c: when floor_log2(refs)*refs/livelen*10000 evaluates equal for two carriers, the tie breaks by insertion order (RTL first-set LUID). MIPS's ascending-regno find_reg then picks by insertion, not by pri delta. If prologue arithmetic can be respelled so tbl_125c and idx_1494 carriers hit identical (refs, livelen) at post-preload qty birth, a third s-reg rotation basin (distinct from h5's and honest's 4-cycles) becomes reachable. Precondition: s69's csmd4_only.greg extracts provide the exact (refs, livelen) numbers to target.
- probe: Not measured this session (synthesis modality). Precondition for the s74 measurement: compute (refs, livelen) for {p72,p73,p78,p79} carriers under h5 and honest from tmp/grind/cpu_side_move_dir_4/s69/allocdbg_diff/csmd4_only.greg, identify byte-neutral prologue re-orderings at src/system.c:388-408 that equalize (refs, livelen) on 2+ of them while preserving byte-equivalent RTL first-uses.
- result: Frontier documented, quantitative precondition (s69 greg extract read) named. Elevated to F9 in reset frontier.
- verdict: CONFIRMED

## [s74] Swapping the assignment order of tbl_125c and idx_1494 (both simple const-address stores) shifts qty birth luids to reach a novel s-reg basin distinct from h5 and honest 4-cycles.
- mechanism: local-alloc.c qty_compare tiebreak on (refs, livelen); C-decl-order does not drive pseudo birth but statement-order does drive RTL first-use LUID, which feeds the qty birth-order tiebreak when priorities tie.
- probe: src/system.c:404-405 swap: idx_1494 = &D_800A1494; tbl_125c = D_800A125C; sandbox --disable all
- result: masked=2, target_insns=160, build_insns=160. INERT vs h5 baseline. Bytes identical; simple const-load reorder is qty-invariant at debug-window scope.
- verdict: KILLED

## [s74] Hoisting the D_800F19BC=0 and D_800F19C0=&D_80016240 assignments BEFORE the idx setup lengthens/shortens carrier livelen, potentially equalizing (refs,livelen) on the 4-cycle rotation carriers.
- mechanism: Same qty_compare mechanism; hoisting D_800F19_ stores earlier moves their write-insns to lower LUIDs, but no local pseudo carries D_800F19_ addr in the debug window - they're direct SW insns with lui/addiu inline.
- probe: src/system.c:404-408 reorder: D_800F19B8=...; D_800F19BC=0; D_800F19C0=...; tbl_125c=...; idx_1494=...; idx_1495=...; sandbox --disable all
- result: masked=2 INERT. Confirms D_800F19_ stores emit as direct SW insns (no long-lived local pseudos) so their C-position doesn't feed qty priority arithmetic.
- verdict: KILLED

## [s74] Interleaving D_800F19_ stores BETWEEN the idx setup statements (breaking up the const-address cluster) alters expand-time scheduling and s-reg birth ordering.
- mechanism: sched.c LUID assignment follows RTL emission order; splitting the const cluster with unrelated stores creates gaps that could re-time debug-window pseudo births.
- probe: src/system.c: tbl_125c=...; D_800F19BC=0; idx_1494=...; D_800F19C0=...; idx_1495=...; sandbox --disable all
- result: masked=2 INERT. The two D_800F19_ stores are LUID-transparent for debug-window pseudo qty; interleaving them does not perturb the residual pair.
- verdict: KILLED

## [s74] Delaying idx_1495 assignment until AFTER the D_800F19_ stores (idx_1495 as LAST prologue statement) delays p78's first-use LUID, potentially demoting its qty priority via a shorter livelen from the window's start.
- mechanism: livelen = last_use_luid - first_use_luid; pushing p78's first-use later shortens its livelen relative to the constant span the debug window covers - hoping to raise p78's priority and offset the s-reg rotation.
- probe: src/system.c: tbl_125c=...; idx_1494=...; D_800F19BC=0; D_800F19C0=...; idx_1495=...; sandbox --disable all
- result: masked=2 INERT. p78's livelen calculation is dominated by its LAST-USE at the poll region (*idx_1495 dispatch call), not by prologue-side placement - the prologue reorder does not move the livelen numerator enough to shift qty_compare.
- verdict: KILLED

## [s74] Delaying D_800F19B8 = sys_VSync(-1) + 0x3C0 (the CALL prologue insn) to LAST position frees the initial CALL-return v0 for other pseudo births and could yield a distinct alloc basin.
- mechanism: sys_VSync return value in v0 births p_D_800F19B8_val at the CALL_INSN; delaying it to last would put the CALL_INSN AFTER const setup, possibly changing the initial hard-reg conflict set for downstream pseudos.
- probe: src/system.c: [all const assignments]; D_800F19B8 = sys_VSync(-1)+0x3C0; sandbox --disable all. Rejected form at memory/grind/cpu_side_move_dir_4/rejected/prologue_reorder_svsync_last.c
- result: masked=21, target_insns=160, build_insns=160. +19 REGRESSION. Delayed sys_VSync destroys the h5 alignment: the CALL's v0-return + call-clobber footprint at the WRONG position pulls apart the const-address cluster's pseudo births.
- verdict: KILLED

## [s74] sys_VSync CALL in INTERIOR position (after idx setup, before D_800F19BC/C0) is a middle-ground between LAST (masked=21) and FIRST (masked=2 baseline), potentially reaching a novel basin below either endpoint.
- mechanism: Same as above; interior placement gives partial livelen redistribution.
- probe: src/system.c: tbl_125c=...; idx_1494=...; idx_1495=...; D_800F19B8=sys_VSync(-1)+0x3C0; D_800F19BC=0; D_800F19C0=...; sandbox --disable all. Rejected form at memory/grind/cpu_side_move_dir_4/rejected/prologue_reorder_svsync_middle.c
- result: masked=15, target_insns=160, build_insns=160. +13 REGRESSION - lands in the same +13 collapse basin as the honest-idx_1495 respelling (s8) and the two-set arg5 collapse family (s65 P1/P2). sys_VSync position is monotonically load-bearing with FIRST as the unique minimum.
- verdict: KILLED

## [s75] Splitting the sys_VSync fold into a named vsync_ret intermediate and sweeping the D_800F19B8 STORE through prologue statement-order positions surfaces a novel s-reg basin distinct from h5.
- mechanism: The named vsync_ret gives the call-return v0-value its own pseudo whose livelen ends at the ADD to D_800F19B8; moving the STORE later extends that pseudo's livelen and could perturb qty_compare priorities (livelen ~ 1/pri) on the debug-window s-reg carriers {p72,p73,p78,p79} identified by s69's greg-diff, potentially inducing a THIRD s-reg rotation basin distinct from the h5 baseline and the honest_idx_1495 +13 rotation.
- probe: Applied 6 variants via tmp/grind/cpu_side_move_dir_4/s75/sweep.py: CALL=vsync_ret first, then STORE at positions 1..6 relative to the tbl_125c/idx_1494/idx_1495/D_800F19BC/D_800F19C0 assignments; sandbox cpu_side_move_dir_4 --disable all each.
- result: P0(pos1)=2, P1(pos2)=2, P2(pos3)=2, P3(pos4)=2, P4(pos5)=2, P5(pos6=LAST)=8. Positions 1..5 are byte-identical to the h5 baseline - GCC folds the named intermediate + relocated STORE back to identical RTL as the single-statement fused form via cse.c (the vsync_ret pseudo dies immediately at the ADD-and-STORE pair regardless of C-position of that pair among independent global-const stores). Only P5 (STORE past the last global-store D_800F19C0) regresses by +6, indicating the deferred store past all other stores DOES perturb the alloc web but into a novel misalignment basin, not into a lower one.
- verdict: KILLED

## [s76] A novel-region directed PERM_LINESWAP on the fn-scope prologue statements (D_800F19B8/tbl_125c/idx_1494/idx_1495/D_800F19BC/D_800F19C0 assignments) with alternatives that strictly preserve the h5 mult-expander chain (t0 *= 4 mult path -> fresh p106 -> LAUNCH; multi-set t0 via *(u8*)tbl_125c + t0; pp = (void**)&D_800F19C0 alias; v0 <<= 2 stage) will surface a byte-neutral prologue reordering that shifts file-scope s-reg pressure and unlocks a novel basin below h5 masked=2.
- mechanism: The fn-scope prologue sets up the s-reg allocation web that later feeds the debug_printf window's alloc web via qty_compare's file-level ordering. A different C statement order for the 6 prologue assignments produces different RTL emission LUIDs, different qty birth order for tbl_125c/idx_1494/idx_1495 s-reg pseudos, and could re-time the priorities feeding the residual pair-swap. Untried region in the ledger (all prior permuter campaigns targeted inline-block or poll region only: s5 g3 fresh-seed 9040, s13 h5-multexpander fresh-seed 2999, s14 directed PERM inline 23427, s67 F1 g3-arg5 24 exhaustive + F2 poll 3-stmt 6 exhaustive, s68 F7 whole-block 480). Alternatives audited pre-launch to preserve h5 chain (fixes the s14 lesson where alternatives inadvertently included g3-shape spellings and dropped base_score to 40).
- probe: Rewrote tmp/perm_csmd4/base.c inline block to the honest h5 form (t0 *= 4; t0 = (s32)((u8 *)tbl_125c + t0); *(s32 *)t0 in call, NOT the g3 statement-form t0 <<= 2 + in-call addu spelling that base.c previously had). Wrapped prologue lines 448-453 with PERM_LINESWAP. Compile-verified via tmp/perm_csmd4/compile.sh (PROLOGUE_FIX: 4 reordered, base_test.o 2480 bytes). Launched: tools/permuter_campaign.py launch --func cpu_side_move_dir_4 --dir tmp/perm_csmd4 --label s76_h5_prologue_lineswap_v2 -j 8 --stop-on-zero. Post-launch base_score verified as 60 (confirming h5-multexpander basin preserved, alternatives did not inadvertently drop to g3 base=40).
- result: Harvest after 706.2s wall-clock / 720 registered iterations (~650k raw iterations across 8 workers per run.log) / 16 pre-existing finds unchanged / finds_new=0 / best_new_score=null. Campaign stopped with --stop reason 's76 h5 prologue-lineswap fresh-seed 0 novel'. The h5-multexpander basin's local minimum under random mutation over the prologue-lineswap chassis is stable at 20-30 min wall-time (matches s5's g3 9040-iter / 0 novel and s13's h5-multexpander 2999-iter / 0 novel and s14's directed-inline 23427-iter / 0 novel patterns).
- verdict: KILLED

## [s77] PERM_LINESWAP over the two vblank-poll if-arms (`if (status & 4) {...}` vs `if (status & 2) {...}`) is a novel un-run permuter neighborhood that could couple to the file-wide s-reg web (per s60/s69 4-cycle rotation atomicity finding) and surface a new debug-window s-reg basin.
- mechanism: The two poll arms both read idx_1495 / idx_1494 s-reg carriers. Swapping their C order changes each carrier's use luid and could ripple into qty_compare's transitive priority ordering across {p72,p73,p78,p79}. Never targeted by s5 (g3 fresh-seed), s13 (h5 fresh-seed), s14 (arg5-side directed PERM_GENERAL), s67 F1 (g3-arg5 24-iter), s67 F2 (pre-poll 3-stmt LINESWAP), s68 F7 (whole inline-block 480-iter), or s76 F13 (prologue LINESWAP).
- probe: Annotated tmp/perm_csmd4/base.c with PERM_LINESWAP wrapping the two poll if-arms (semicolon-terminated statements-in-braces). Launched via tools/permuter_campaign.py launch --func cpu_side_move_dir_4 --dir tmp/perm_csmd4 --label s77_h5_poll_arm_lineswap -j 8 --stop-on-zero.
- result: Permuter reported base_score=2215 (vs h5=60, g3=40). Ran ~6779 iterations; every score sampled clustered at 2215/2380 (near-baseline) or 300-2000+ (regressions); 0 candidates below 2215. Stopped via harvest --stop. Confirms arm-swap materially disturbs the file-level alloc/schedule web via s-reg livelen deltas on {idx_1494,idx_1495} — the two arms are NOT RTL-swap-invariant even though the C is semantically equivalent (they both call through fn pointers with side-effectful semantics that GCC cannot re-order via sched.c because the calls are opaque). The PERM lineswap picks arm-swapped orderings uniformly which all sit in a novel +2213-above-h5 basin.
- verdict: KILLED

## [s77] Pre-existing tmp/perm_csmd4/output-55-{1,2,3} unmeasured permuter finds (weighted score 55 < h5's 60) might yield a sandbox-masked improvement below h5's masked=2 floor.
- mechanism: output-55-* diffs hoist the arg5-chain (v0=idx_1494[1]; v0<<=2; arg5=*(v0+tbl)) BEFORE the t0-chain in the inline block — the arg5-first ordering. Never re-verified via engine sandbox; WIP notes explicitly warn 'standalone-permuter finds are UNFAITHFUL / workspace prunes TU / do not trust without full-context re-verification'.
- probe: Static inspection of output-55-1/2/3 diff.txt. Matched against rejected-forms bank: shape corresponds to s2 E_arg5_first (variant `v0=idx_1494[1]` head-load BEFORE `t0=idx_1494[0]`) which s2 measured at masked=7 (rejected at rejected/lineswap_arg5_first_v0_shift_early_g3.c and rejected/lineswap_pp_hoisted_arg5_first_g3.c).
- result: The output-55-* forms are re-realizations of the arg5-first / g3-basin family already KILLED at s2 masked=7. Weighted permuter score 55 vs base 60 reflects permuter's weighted metric; sandbox masked distance for this basin is 7 (+5 vs h5). No sandbox re-measurement needed — the shape is already in the rejected bank.
- verdict: KILLED

## [s78] P1: idx_1495 = idx_1495; placed before the poll: label produces an isolated single-carrier refs-lift on p78 that survives to local-alloc and shifts qty_compare priority in favour of a novel s-reg basin distinct from h5.
- mechanism: flow.c's life_analysis + delete_noop_moves pass recognises (set p78 p78) as a no-op and deletes the insn upstream of local-alloc; even if a use-count were credited, cse.c's copy-propagation would collapse it. The proposal was that the self-assign might survive at least the ref-count phase and appear in ALLOCDBG's per-pseudo reg_n_refs.
- probe: Applied `idx_1495 = idx_1495;` immediately before `poll:` at src/system.c line ~447 on the h5 candidate; ran sandbox --disable all; ran instrumented cc1 (tmp/gccdbg/cc1 with BB2_QTY_DEBUG/SCHED_DEBUG/ALLOC_DEBUG/FINDREG_DEBUG/RANK_DEBUG=1 -da) for both baseline and P1; diffed the ALLOCDBG and QTYDBG per-pseudo tables via tmp/grind/cpu_side_move_dir_4/s78/diff.py.
- result: Sandbox masked=2, target_insns=160, build_insns=160 (byte-neutral). ALLOCDBG per-pseudo diff: 0/102 differing pseudos. QTYDBG per-reg1 refs max diff: 0/276 entries differ. The self-assign is definitively deleted by flow.c before local-alloc has any opportunity to observe an extra ref on p78.
- verdict: KILLED

## [s78] P2: if (idx_1495 == (u8 *)-1) return -1; is a real branch reading p78 that flow.c cannot prove dead (idx_1495 is a runtime pointer derived from a global cast; GCC 2.7.2 has no value-range info to fold the compare), so it survives to local-alloc lifting p78's reg_n_refs while remaining byte-neutral vs h5 (the emitted branch would fold via cross-jump merge with an existing return -1; path).
- mechanism: flow.c preserves conditional branches whose taken-body is a real statement; if two return -1; sites exist elsewhere in the function, jump2's find_cross_jump could merge the branch tail into the existing exit path, hypothetically leaving the emitted-byte count unchanged while adding one extra p78 use.
- probe: Applied `if (idx_1495 == (u8 *)-1) { return -1; }` at src/system.c line ~447 (same slot as P1); ran sandbox --disable all.
- result: Sandbox masked=15, target_insns=160, build_insns=163 (+3 insns). Byte-neutrality FAILED: the branch emits at minimum 3 additional instructions (li const-1, bne p78 const-1 .L, delay-slot lui/li, jump-to-return) that jump2 cannot cross-jump-merge because the branch pre-condition ("only take this if idx_1495 == -1") differs from any existing return-1 site's precondition. Refs-lift on p78 does materialise but rides on non-neutral bytes, disqualifying the isolation criterion.
- verdict: KILLED

## [s78] The H2-extension frontier as-stated (byte-neutral C perturbation that ISOLATES a single p72/p73/p78/p79 refs/livelen delta) is achievable for at least one of the four s-reg rotation carriers via some untried spelling.
- mechanism: The frontier text posited that adding an EXTRA USE at a site the RA already dominates could raise refs by 1 without disturbing the other three carriers. This depends on the existence of a C construct that (a) survives flow.c DCE and (b) does not emit an additional instruction.
- probe: Bracketed the space with P1 (DCE-invisible: too-weak side) and P2 (emits +3: too-strong side); confirmed no middle ground exists because (a) flow.c deletes any construct whose result is not observably used, and (b) any construct with an observable use emits at least one instruction perturbing the bytes.
- result: The pair {P1, P2} shows the two ends of the spectrum for local scalar pointer carriers. Under flow.c/combine.c semantics, a byte-neutral extra-USE of a local scalar pointer is IMPOSSIBLE: DCE deletes anything below the emit-threshold; anything above it emits real instructions. This closes the H2-extension frontier for the four s-reg carriers p73/p77/p78/p79. The only remaining ref-lift mechanism at the local-scalar-pointer level is the F6 SOTN-sanctioned duplicated-statement-into-arms with cross-jump re-merge (requires real statement with genuine control-flow arms, structural modality).
- verdict: KILLED

## [s79] F16: a C-level added read (e.g. redundant lw referencing p73/p77) can be materialized via reorg.c fill_slots_from_thread absorbing it into an existing nop delay slot, keeping total emitted-byte count constant while lifting reg_n_refs on the target s-reg carrier to flip the h5 pair-swap.
- mechanism: reorg.c fill_slots_from_thread scavenges preceding data-flow-independent insns for currently-nop delay slots (jal/branch). Precondition: a currently-nop delay slot must exist ADJACENT to the residual pair-swap window so a scavenged insn's re-emission position can influence the pair order.
- probe: Enumerated every branch/jump delay slot in asm/funcs/cpu_side_move_dir_4.s L1-L176 and classified FILLED vs NOP; mapped each NOP slot to its containing block relative to block=3 (the debug_printf window .L80080E64..jal debug_printf). Cross-checked with GCC 2.7.2 pass order in tools/gcc-2.7.2/toplev.c rest_of_compilation. Findings in tmp/grind/cpu_side_move_dir_4/s79/block3_topology.md.
- result: Block=3 is straight-line 20-insn (label .L80080E64 L48 through jal debug_printf L72 — ZERO branches interior). Residual pair {sll@L57 <-> addu@L58 <-> sll@L59} is INTERIOR to block=3. Only two nop-delay-slot sites reachable without a branch traversal: L52 (jal tslTm2LoadImage_2 head-of-block, PRECEDES pair) and L73 (jal debug_printf tail-of-block, AFTER pair). Neither sits INSIDE the pair-swap window. Pass order confirmed: sched2 (schedule_insns_2) commits the pair order via LUID tiebreak at LAUNCH-priority 0x7f000001 BEFORE reorg.c/dbr_schedule runs. Any C-level added read either (a) DCE'd before it reaches reorg.c (fails to affect refs), or (b) survives to sched2 and perturbs the very LAUNCH tie F16 tries to preserve (regresses to g3 basin per s6 LUID-reorder = masked 6). Boundary slots L52/L73 have no scavenging opportunity: L52's preceding insns are already absorbed as the jal's arg set-up (lui/addiu a0); L73's block=3-preceding insns all feed the a0-a3 arg registers.
- verdict: KILLED

## [s80] m2c --context include/m2c_context.h (the sole un-enumerated m2c option combination across s8/s53/s54/s62/s63/s72) produces a novel structural C shape for the debug_printf window that surfaces a rederive neighborhood outside the h5/g3/inline-all basins.
- mechanism: m2c's --context flag reads existing C context (extern decls, types) and prefers type-driven expression reconstruction (array subscript vs raw pointer arithmetic) when a decl like `extern s32 D_800A11DC[]` is available. Prior m2c runs used no context, so type inference produced *(&sym + N*sizeof(elem)) pointer forms. --context could steer arg3/arg4/arg5 spellings into distinct expand-time RTL trees.
- probe: python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c --context include/m2c_context.h --function cpu_side_move_dir_4 asm/funcs/cpu_side_move_dir_4.s > tmp/grind/cpu_side_move_dir_4/s80/m2c_ctx.txt. Compared debug_printf window shape line-by-line against s8 (default), s72 (saEft01Init.s), s62 (marionation_Exec.s), s53/s54/s63 (--reg-vars variants).
- result: Only arg3 changes: default m2c emits `*(&D_800A11DC + (D_800A11D5 * 4))` (raw pointer arith); m2c --context emits `D_800A11DC[D_800A11D5]` (array subscript) because include/m2c_context.h declares `extern s32 D_800A11DC[]`. This array-subscript syntax is ALREADY present in the committed h5 candidate.c line 49. arg4/arg5 remain the known inline-all-args basin form `*((M2C_FIELD(&D_800A1494, u8*, 0)*4) + &D_800A125C)` (WIP L7 masked=14 basin). Zero novel structural neighborhood beyond a lexical refinement of an already-in-basin arg3.
- verdict: KILLED

## [s80] m2c --stack-structs --passes 5 (novel un-tried option pair across all prior m2c sessions) forces stack-usage explicit modeling + more type-inference iterations, potentially exposing a distinct block=3 debug_printf shape.
- mechanism: --stack-structs makes stack slots explicit as struct fields, potentially uncovering hidden stack-spill intermediates. --passes 5 (vs default 2) gives m2c more iterations to resolve types across the fn body, which could steer expression-tree reconstruction into a distinct fold vs stage choice.
- probe: python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c --stack-structs --passes 5 --function cpu_side_move_dir_4 asm/funcs/cpu_side_move_dir_4.s > tmp/grind/cpu_side_move_dir_4/s80/m2c_stackstructs_p5.txt (98 lines).
- result: Produces the same fully-inlined debug_printf shape as s8 default: `debug_printf(&D_800161C8, D_800F19C0, *(&D_800A11DC + (D_800A11D5*4)), *((M2C_FIELD(&D_800A1494,u8*,0)*4) + &D_800A125C), *((M2C_FIELD(&D_800A1494,u8*,1)*4) + &D_800A125C))`. Zero stack-spill intermediates surfaced (block=3 uses no stack storage in target asm). --passes 5 converged identical to default --passes 2. No novel block=3 reconstruction.
- verdict: KILLED

## [s81] m2c on the name-cluster siblings cpu_side_move_dir (base, 168 insns) / _2 / _3 (never m2c'd per s62/s63 closure list — s62 only covered marionation_Exec.s as the file-local twin, s63 grepped the SRC files those functions live in but did not m2c their asm) surfaces a structurally novel C template overlapping csmd4's debug_printf/tbl_125c/idx_1494 window.
- mechanism: Name-cluster siblings share a namespace prefix and could plausibly reuse the same debug_printf/timing/dispatch skeleton with different arg-marshal shapes; m2c reconstructs each's original-compiler-shaped C from raw asm and may reveal an un-seen block-3 statement structure outside the h5/g3 basins.
- probe: Ran `python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c --function cpu_side_move_dir asm/funcs/cpu_side_move_dir.s` and equivalent for _2/_3; output saved to tmp/grind/cpu_side_move_dir_4/s81/m2c_base_sibling.txt, m2c_sib2.txt, m2c_sib3.txt.
- result: cpu_side_move_dir (base) is a data-driven object-movement function taking `void *arg0`, dispatching on `D_800A36A4` mode, reading a stage_GetDataPtr() table, updating arg0 fields 0xF4/0xFC/0xD8/0xE0/0xB8/0xC0 with velocity accumulators, and calling func_80021424/func_80021A98/func_80032854. No debug_printf, no tbl_125c/idx_1494 references, no VSync polling window. cpu_side_move_dir_2 is a game_Cleanup/DMA/gpu_InitDisplay initialization function (no debug_printf window, no tbl_125c). cpu_side_move_dir_3 is a bios_DeliverEvent-driven handler using D_800F1AE0/D_800A3044/D_800F1B00-04 state (no debug_printf, no tbl_125c). None of the three siblings share csmd4's block-3 shape.
- verdict: KILLED

## [s82] s82 synthesis modality: cross-read 81-session ledger; every enumerable first-order lever KILLED at chassis, permuter (~137k iters), rederive (13 sub-angles), structural (block-local + prologue + outer-flow + dup-into-arms for D_800F19C0 and tbl_125c), and forensics (ALLOCDBG 4-cycle atomic; block=3 prologue-invariant; reorg pass-order invalid; C-level extra-USE isolation impossible for local scalar pointers per s78 P1/P2 bracket); class-attack triple-KILLED per s15/s16.
- mechanism: Ledger digest re-read confirms: h5/g3 basin non-composable via expmed.c:2244 case alg_shift NULL_RTX (s7); p107 two-SET closed 5 ways via combine.c addsi3_internal (s11/s12); residual pair-swap sched2 clock=13 LUID tiebreak 121>111 (s6/s15/s16); Judge constraint (2026-07-08) BINDING against canonical-asm resurfacing.
- probe: Read evidence.md (1332 lines) + hypotheses.md (1255 lines) + rejected/ (84 forms) + task-brief digest; wrote consolidated closure catalog and frontier reset to tmp/grind/cpu_side_move_dir_4/s82/synthesis.md.
- result: Two live frontiers survive re-read: F6-double-prime (p77 idx_1494 dup-into-arms, un-tested subvariant of the SOTN carve-out; s20 measured D_800F19C0 target with +8 wrong-sign misdirection and rejected/dup_tbl_125c_arms.c already banks p73/p79); F20 (poll-arm symmetric idx_1495 real-USE, last un-measured p78 lift after s78 KILLED the local-scalar-pointer isolation for block=3-entry positions).
- verdict: CONFIRMED

## [s82] F17 (block=2 CALL-return-value coincidence enabling combine two-SET substitute with jal-clobbered second SET on p107) is compiler-source valid but practically dead: block=2 contains exactly one CALL (sys_VSync(-1) returning int-32 offset), which is numerically incongruent with the required arithmetic identity (u8*)tbl_125c + idx_1494[1]*4.
- mechanism: sched.c birthing_insn_p returns FALSE when flow.c reg_n_sets(p107) >= 2; a two-SET whose second SET originates from a jal-return-clobber-shadowed insn defeats combine's operand-tracking through mips.md addsi3_internal (per s7:213 substitution semantics). Requires a specific block=2 CALL whose return arithmetically equals the arg5_addr expression.
- probe: Ledger cross-read of block=2 CALL topology from s6/s7 dumps + task-brief digest. No forensics dump built this session (synthesis modality).
- result: Only block=2 CALL is sys_VSync(-1). Its return value has no arithmetic relation to (u8*)tbl_125c + idx_1494[1]*4. F17 remains theoretically un-run but is downgraded to dead unless an operator-supplied structural rewrite fabricates the coincidence (e.g. sys_VSync-return-through-identity that equals arg5_addr).
- verdict: KILLED

## [s83] Symmetric byte-neutral idx_1495 dereference inserted via comma-op at HEAD of both vblank if-arms lifts p78 refs at flow-time before local-alloc counts, shifting qty priority to break the 4-cycle s-reg rotation.
- mechanism: flow.c REG_N_REFS counted pre-local-alloc; comma-op discarded value could survive as a load RTL insn observable to flow counter even though DCE later removes it. If so, p78 refs+=2 offsets the s60/s61 demoted priority=138 tie at qty_compare.
- probe: Applied h5 baseline candidate.c to src/system.c (confirmed masked=2), then edited both status&4 and status&2 if-arms to prepend a comma-op read of *idx_1495 to the fn-ptr call's first arg: `((...) D_800A11B8)((*idx_1495, *idx_1495), &D_800F19A8);` and `((...) D_800A11B4)((*idx_1495, *idx_1494), &D_800F19A0);`. Ran `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.
- result: masked=2, target_insns=160, build_insns=160. INERT vs h5 baseline. The comma-op unused-value reads are eliminated by flow.c delete_noop_moves / DCE BEFORE the local-alloc REG_N_REFS counter runs, so p78's flow-time ref count is unchanged; the +2 fake refs never materialise. Same mechanism as s78's H2-extension finding for local scalar pointer carriers p73/p77/p78/p79.
- verdict: KILLED

## [s84] Symmetric duplication of the byte-neutral REAL statement `idx_1495 = idx_1494 + 1;` (semantically equivalent to the prologue cross-symbol form for idx_1495) into BOTH do_timeout arrival arms with prologue init retained lifts reg_n_refs(p77) at flow-time via cross-jump re-merge collapsing the two duplicates to identical bytes, shifting qty_compare priority to flip the 4-cycle s-reg rotation without falling out of the h5 basin.
- mechanism: jump2 find_cross_jump merges byte-identical tails from arm-A (before `goto do_timeout;`) + arm-B fallthrough (before `do_timeout:` label). A real statement referencing idx_1494 duplicated in both arms lifts p77 reg_n_refs by +2 at flow-time (pre-merge count) then merges back to net-zero emitted bytes at cross-jump time. Direction opposite to D_800F19C0's carrier per s69 greg extract - hoped to resolve the qty tie the s20 D_800F19C0 misdirection did not.
- probe: Applied `idx_1495 = idx_1494 + 1;` at tail of arm-A (before `goto do_timeout;`) AND at tail of arm-B fallthrough (between `goto success;` and `do_timeout:` label), prologue init of idx_1495 (cross-symbol form) RETAINED. Sandbox cpu_side_move_dir_4 --disable all.
- result: masked=17, target_insns=160, build_insns=160. Cross-jump merge DID fire (build_insns=160 confirms byte-neutrality). Refs-lift mechanism materialised. But +15 masked regression - the qty priority shift MISDIRECTS s-reg allocation, falling entirely out of the h5 basin. Same wrong-sign signature as s20 dup_D_800F19C0_no_prologue.c (masked=10) and the rejected/dup_tbl_125c_arms.c placeholder for p73/p79. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/dup_idx1495_arms_p77_prologue_kept.c.
- verdict: KILLED

## [s84] Asymmetric single-arm duplication (arm-A only, before `goto do_timeout;`, no counterpart at arm-B fallthrough) isolates p77 refs-lift ONLY on the goto-taken path, potentially avoiding the alloc-web misdirection observed in the symmetric case.
- mechanism: Without a cross-jump merge partner, the asymmetric probe emits the `idx_1495 = idx_1494 + 1;` add insn genuinely at flow-time. This tests whether p77 refs-lift on ONE arm suffices, or whether the symmetric misdirection was itself caused by both-arm lift (as opposed to the merge process).
- probe: Applied `idx_1495 = idx_1494 + 1;` at tail of arm-A ONLY. Sandbox --disable all.
- result: masked=15, target_insns=160, build_insns=162 (+2 insns). Byte-neutrality FAILS - no counterpart to cross-jump-merge with, so the add insn (addiu s4, s2, 1) emits physically PLUS an extra move/store re-alignment (+2 total). Non-neutrality disqualifies the F6-double-prime carve-out isolation criterion (prereq is byte-neutrality per duplicated-statement-into-arms 2026-07-01 sanction). Rejected form at memory/grind/cpu_side_move_dir_4/rejected/dup_idx1495_arm_a_only_p77.c.
- verdict: KILLED

## [s85] Single dup of `idx_1495 = idx_1494 + 1;` on the SUCCESS path (before goto success;) is byte-neutral (cross-jump merges with some other tail) and induces a novel p77-refs-lift qty basin distinct from the s84 symmetric-arms +15 misdirection.
- mechanism: The frontier's cross-jump-merge hope: if some tail elsewhere emits an identical `sw idx_1495, mem` insn, jump2 find_cross_jump can absorb the success-path dup into the shared merge site, keeping build_insns=160 while lifting p77 refs at flow.c time. Alternatively: physical +1-2 insn if no merge partner exists, KILL for byte-neutrality.
- probe: Applied h5 candidate to src/system.c; measured baseline masked=2 build_insns=160. Then inserted `idx_1495 = idx_1494 + 1;` at the tail of the fallthrough arm (inside the `if (!(0x3C0000 < cnt))` block, immediately before `goto success;`); ran & tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all.
- result: masked=15, target_insns=160, build_insns=162 (+2 physical). No cross-jump merge partner exists on the success-path tail; the assignment emits as a lui+sw pair inside the arm. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/dup_idx1495_success_path_p77.c.
- verdict: KILLED

## [s85] Placing the same dup at LOOP-TOP (immediately after `loop:` label, before `v0 = sys_VSync(-1);`) may DCE-invisibly CSE with the prologue idx_1495 initializer, producing 0 refs delta AND 0 insn delta (byte-neutral no-op), OR emit a per-iteration store (heavy regression).
- mechanism: GCC's cse.c copy-propagation on same-value stores; the assigned RHS `idx_1494 + 1` = `&D_800A1494 + 1` is compile-time constant (idx_1494 was assigned `&D_800A1494` in prologue), semantically identical to the prologue-computed idx_1495 value; if cse folds it, the store is dead and DCE'd (0 refs delta); if not, per-iteration overhead lands.
- probe: Applied h5 candidate; inserted `idx_1495 = idx_1494 + 1;` immediately after `loop:` label; ran sandbox --disable all.
- result: masked=15, target_insns=160, build_insns=159 (-1 physical). NOVEL signature never previously observed in the ledger (all prior probes were 160, 162, or 163). The loop-top store does NOT CSE with the prologue (unlike s78 P1 self-assign which was DCE-invisible) — instead it survives through combine, then the resulting flow-time reg_n_refs shift on idx_1495 causes local-alloc to eliminate ONE physical insn elsewhere in the pair-swap window while still misdirecting +13 vs baseline. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/dup_idx1495_loop_top_p77.c.
- verdict: KILLED

## [s85] COMPOUND: loop-top dup + success-path dup together may cross-jump merge (jump2 find_cross_jump absorbs the success-path insn into the loop-top occurrence, since success falls back through check: and re-enters loop:) yielding either byte-neutral compound with a different alloc web than loop-top alone, or the same masked=15 signature (confirming the merge fired).
- mechanism: jump2 find_cross_jump scans block tails for identical insn suffixes; if the success-path insn is bytewise identical to the loop-top insn AND the success-path tail's control flow reaches loop-top (which it does via check:/return/loop cycle), merge fires.
- probe: Applied h5 candidate; inserted BOTH the loop-top dup AND the success-path dup simultaneously; ran sandbox --disable all.
- result: masked=15, target_insns=160, build_insns=159 - IDENTICAL to loop-top-alone signature. Cross-jump merge from success-path into loop-top fires deterministically, producing zero net effect vs loop-top-alone. The compound does NOT open a distinct qty basin. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/dup_idx1495_looptop_plus_success.c.
- verdict: KILLED

## [s86] A fresh-seed permuter campaign on the tmp/perm_csmd4 chassis (PERM_LINESWAP prologue + PERM_GENERAL loop-top v0=sys_VSync + PERM_RANDOMIZE inline block) will surface novel finds below the h5-relative baseline within the ~15-min fresh-seed window.
- mechanism: Per s86 frontier probe: the loop-top region between `loop:` label and `if (D_800F19B8 < v0)` has been included in prior merged chassis (s13 base=60, s14 merged=40) but never explored via a fresh-seed campaign after the s85 probe2 -1-physical-delta signature. Random-mutation neighborhood around the loop-top PERM_GENERAL alternatives (v0 = sys_VSync(-1); / { s32 vt = sys_VSync(-1); v0 = vt; } / v0 = (s32)sys_VSync(-1);) may reach a mutation basin the prior campaigns missed.
- probe: Launch permuter_campaign.py fresh-seed on tmp/perm_csmd4 with existing base.c chassis, -j 8, --stop-on-zero, ~15-min wall window per fresh-seed discipline (permuter-directives Campaign discipline). Snapshot pre-existing 16 output-* dirs; measure novel finds count.
- result: Campaign launched (pid 417, 2026-07-09T14:31:03Z), harvested with --stop after ~15 min. Permuter self-reported base_score=2215 (the s77 F14 vblank-poll-arm regressed chassis - annotations in current base.c inflate merged base to a bad neighborhood). Elapsed s=95 tracked in meta, iterations=1375. finds_new=0; output dir count unchanged at 16 (all pre-existing per campaign_meta.preexisting_outputs).
- verdict: KILLED

## [s87] ALLOCDBG-instrumented forensics on s85 probe2 (loop-top idx_1495=idx_1494+1) build_insns=159 signature will identify the specific pseudo whose refs shifted and which upstream insn was eliminated by local-alloc
- mechanism: flow.c reg_n_refs redistributes when a loop-top respell dominates all uses of a prologue-init pseudo; local-alloc/combine can absorb one prologue insn; the ref-redistribution then causes greg's qty_compare to re-rank the affected pseudo in the ord window
- probe: Rebuild gccdbg dumps for probe2 (rejected/dup_idx1495_loop_top_p77.c) with BB2_QTY_DEBUG/BB2_SCHED_DEBUG/BB2_ALLOC_DEBUG/BB2_FINDREG_DEBUG/BB2_RANK_DEBUG=1; diff greg/qty/findreg vs s69/h5 baseline
- result: p79 (5-ref lead carrier, hardreg=19 in h5) lost 2 refs (nrefs 5->3), pri collapsed 675->202, and was DEMOTED from ord=12 (FIRST in the [12..15] window) to ord=15 (LAST); p78 was PROMOTED to ord=12 and re-took hardreg=19; the ord=[12..15] rotation [79,78,72,73]->[78,72,73,79] is the mechanism producing the 13+ downstream reg-name diffs (masked=15); build_insns 160->159 (-1) is the one prologue insn eliminated by local-alloc equiv-reg propagation because loop-top respell dominates all *idx_1495 uses (prologue-only variant s8 probe1 had build=160 = no elimination). QTYDBG confirms: blk=0 (prologue) shifted all birth/death luids -2 (one insn removed); blk=1 (loop-top) extended qty=0/reg1=95 death 12->14 (=idx_1494 carrier held live through the new respell).
- verdict: CONFIRMED

## [s87] The loop-top idx_1495 dup mechanism (-1 build_insn from local-alloc equiv-reg absorb) is a viable close lever
- mechanism: if the -1 physical insn were free (no reg-alloc ripple), it could compose with other levers to reach masked=0; the frontier speculated the -1 build_insn signature was a signal of latent close-margin
- probe: same as above; then evaluate whether the ord=[12..15] rotation is a coincidence or the cause of masked=15
- result: Rotation is causal: p79's nrefs=5->3 collapses pri 675->202, and greg assigns hardregs in strict ord order, so p79 flips from hardreg=19 to hardreg=22 and p78 flips from hardreg=20 to hardreg=19. The 13+ downstream masked-15 objdump diffs are all reg-name diffs downstream of this flip. Any respell that reduces p79's ref-count (any variant that pulls a *idx_1495-family use off of p79 onto a fresh pseudo) is doomed to hit the same ord rotation. Loop-top respell class is CLOSED.
- verdict: KILLED

## [s88] The s87 probe2 greg ord=[12..15] rotation [79,78,72,73]->[72,73,79,78] is caused by the loop-top position where *idx_1495 uses re-route through a fresh pseudo (position-caused).
- mechanism: If the -1 build_insn / ord rotation comes from local-alloc equiv-reg absorption of the prologue idx_1495 setup once loop-top dominates all uses, then respelling idx_1495 WITHOUT a loop-top compute (s8-probe1) should preserve p79's 5 refs and the [79,78,72,73] ord window; the rotation would only fire when a loop-top set is present.
- probe: Rebuilt tmp/gccdbg/cc1 dumps for h5 baseline vs s8-probe1 respell (idx_1495=idx_1494+1). Compared greg 'regs to allocate' order and ALLOCDBG ord=[12..15] refs/livelen/pri. Baseline masked=2 build=160 (h5); probe1 masked=15 build=160 (no absorb, matches s8 record).
- result: BASELINE ord=[12..15] = [79,78,72,73] with p79 nrefs=5 livelen=148 pri=675; p78 nrefs=2 livelen=72 pri=277; p72 nrefs=2 livelen=76 pri=263; p73 nrefs=2 livelen=79 pri=253. PROBE1 ord=[12..15] = [72,73,79,78] with p72 nrefs=2 livelen=76 pri=263; p73 nrefs=2 livelen=79 pri=253; p79 nrefs=3 livelen=148 pri=202 (LOST 2 REFS); p78 nrefs=2 livelen=144 pri=138 (livelen DOUBLED 72->144). Reg-count identical (16); pseudo IDs renumbered (101->97, 120->116, 126->122, 137->133) but 72/73/77/78/79/80/81/86 preserved. Position-caused mechanism DISPROVEN: no loop-top compute is present in probe1 yet ord rotates identically and p79 loses the same 2 refs the frontier attributed to loop-top DCE-cascade absorption.
- verdict: KILLED

## [s88] The idx_1495 = idx_1494 + 1 spelling is intrinsically responsible for the p79 ref-loss and ord rotation, regardless of surrounding structure (spelling-caused, broader kill class).
- mechanism: When idx_1495 is spelled as pointer-arith on idx_1494, cse.c collapses idx_1494 as the shared base symbol and the *idx_1495 uses in the poll region rewrite through the p78 (idx_1494) carrier instead of maintaining the p79 (cross-symbol-tbl-derived) carrier's 5-ref profile. p78's livelen doubles (72->144) absorbing the extra uses; p79 drops from 5 refs to 3 refs and its pri drops 675->202, sinking it in the ord below p72/p73.
- probe: Same forensics dump (no additional probe needed). ALLOCDBG lines: baseline p78 livelen=72 (poll-region uses on p79), probe1 p78 livelen=144 (poll-region uses migrated to p78). p79 kept its 148-livelen span (still live across the debug_printf window) but with 2 fewer refs.
- result: CONFIRMED - the ref redistribution is caused by the spelling of idx_1495 alone. Corollary: ANY C-level respelling that unifies idx_1495 with idx_1494 (whether ptr-arith, subscript, cast+add, or any other) triggers the same p79 ref loss because cse.c's addsi3_internal substitution runs before flow.c's reg_n_refs recomputation. The only C forms that preserve p79's 5 refs are those that keep idx_1495's initializer symbolically DISTINCT from idx_1494 (the cross-symbol tbl-routed form currently at src/system.c:406 is one such spelling, but it is in the semantic-lie forbidden family).
- verdict: CONFIRMED

## [s89] A C-source substring search of the decomp.me corpus for printf-family calls with >=5 args (a semantic-level lens distinct from prior asm-shingle scans s17/s26/s71 and prior m2c-variant sub-angles s8/s53/s54/s62/s63/s72/s80/s81) surfaces a novel transplant candidate for csmd4's debug_printf window.
- mechanism: cpu_side_move_dir_4's residual is inside a 5-arg debug_printf call site whose arg4/arg5 chain rivalry is the qty-priority equation of the h5 masked=2 floor. If any other PS1 project in the corpus wrote the same 5-arg debug_printf shape in C, its C spelling could provide a structurally distinct arg4/arg5 marshaling that flips the p100 (arg5 addr) qty priority above p113 (t0 chain) via a form not yet in csmd4's rejected bank.
- probe: Regex scan of all 3754 tmp/decomp_me_corpus/*.json source_code fields for `debug_printf\s*\(.*,.*,.*,.*,.*\)` OR `\bprintf\w*\s*\(.*,.*,.*,.*,.*\)`; hand-review each hit for structural analogy to csmd4's tbl_125c/idx_1494 marshaling + VSync-poll wrapper.
- result: 2 hits total. (a) gcc2.7.2-cdk__PfboX printf: 4-line sprintf+mts_set_debuglog trampoline, no VSync poll, no 1D-array index-marshal chain. (b) psyq3.5__QkCAP MDEC_print_error: multi-call printf sequence over MDEC status registers with bitfield shifts, no VSync poll, no fresh 1D-array chain, no arg4/arg5 rivalry. Neither hit is structurally analogous to csmd4's block=3 window; both are transplant-inert.
- verdict: KILLED

## [s90] idx_1495 = &D_800A1495 (honest direct symref, symbolically distinct from idx_1494) reaches h5 basin masked=2.
- mechanism: Symbolically-distinct symbol reference should defeat cse.c unify-on-shared-base while remaining honest (D_800A1495 exists in undefined_syms_auto.txt). Frontier item #3 predicted this preserves h5's ord=[12..15] property.
- probe: Applied h5 candidate to src/system.c, replaced only idx_1495 init line with `idx_1495 = &D_800A1495;`. Ran sandbox --disable all.
- result: masked=16, build_insns=161 (+1 insn, +14 vs h5 baseline). Regression.
- verdict: KILLED

## [s90] idx_1495 = (u8*)tbl_125c + 0x239 (tbl-routed with numeric constant, no &D_800A1494 symbol reference) reaches h5 basin masked=2.
- mechanism: Same base symbol (tbl_125c) as current cross-symbol form, byte-neutral offset, but no &D_800A1494 reference. Tests whether the tbl_125c base OR the &D_800A1494 symbol reference is the load-bearing property.
- probe: Applied h5 candidate, replaced idx_1495 init line with `idx_1495 = (u8 *)tbl_125c + 0x239;`. Ran sandbox --disable all.
- result: masked=15, build_insns=160 (byte-neutral, +13 vs h5). Matches s8-probe1 (honest idx_1494+1 = 15) exactly.
- verdict: KILLED

## [s90] The h5 basin's masked=2 depends specifically on the semantic-lie cross-symbol expression `(u8*)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1` and not on any structural property of the initializer that a symbolically-distinct honest form could reproduce.
- mechanism: The three tested honest-or-distinct forms (idx_1494+1 [s8], &D_800A1495 [s90 P1], (u8*)tbl_125c + 0x239 [s90 P2]) all regress to masked=13-15. The ONLY masked=2-preserving initializer is the specific semantic-lie fold that references BOTH tbl_125c AND &D_800A1494 AND D_800A125C simultaneously. cse.c evidently uses that triple-symbol expression to compose a file-level RA fabric that no substitute reproduces.
- probe: Comparing s90 P1 + s90 P2 measurements against s8 probe1 measurements and against s90 h5 baseline (masked=2).
- result: All three symbolically-distinct honest forms regress by +13 to +14. h5 depends specifically on the semantic-lie fold.
- verdict: CONFIRMED

## [s91] Inherited F1 (p78-lift via ((volatile u8*)idx_1494)[0] on SUCCESS-only fall-through arm) survives the cross-read as a mechanism but its named probe vehicle is judge-risky.
- mechanism: Volatile-cast dead read fails legitimate-volatile-interrupt-touched prong 1 (idx_1494 not IRQ-mutated); non-volatile dead read is DCE'd by flow.c delete_noop_moves per s78 P1; real branch-read emits +3 insns per s78 P2. No byte-neutral p78-carrier use exists in the block-local structural axis.
- probe: Ledger cross-read against s78 P1/P2 measurements and legitimate-volatile-interrupt-touched two-prong policy.
- result: F1 named vehicle KILLED at policy layer before measurement; F1 mechanism only reachable if a downstream real statement (not currently present per s52 asm audit) can be re-shaped to reach idx_1494 on the success arm - an inter-block structural axis retained as a lower-probability fallback frontier item.
- verdict: KILLED

## [s91] Inherited F2 (p79-preserving loop-top compute referencing existing non-idx pseudo cnt/D_800F19BC/D_800F19C0) has no realization that both survives DCE and avoids the s87/s88 shared-base cse-migration.
- mechanism: s13 KILLED D_800F19C0 fn-body hoist at +13; s85 KILLED loop-top idx_1495 store at masked=15 build_insns=159; s87/s88 CONFIRMED the p79 -2-refs rotation is spelling-caused via cse-unify on idx_1494 as shared base. A read of cnt alone is DCE'd; a self-assign is DCE'd; a real observable update to cnt would change semantics. do-while(0) wraps of existing statements already KILLED across s4/s5.
- probe: Ledger cross-read against s13, s85, s87, s88, s4, s5 measurements.
- result: F2 has no un-run C-realization that satisfies all three constraints (DCE-survival + p79-refs-preservation + semantics-preservation) at the block-local axis; retained as low-probability frontier fallback.
- verdict: KILLED

## [s91] Un-named F3 angle exists: conflict-graph enumeration in global.c allocno_conflicts has never been diffed between h5 and g3 baselines (only priority/LUID axes have been diffed).
- mechanism: s42 ran a partial greg conflict-list census showing p77/p78 share ZERO live range with block=3 pseudos p100/p106/p107/p113, but the conflict EDGES between block=3 pseudos and the file-level s-reg web (s2/s3/s4/s5/s8/s9 seats) were never enumerated. A single edge whose removal would legalize both LAUNCH insns at low LUIDs would seed a follow-on structural session. This is the ALLOCDBG-instrumented s-reg coupling hunt the s71 Judge constraint explicitly permits as a permitted un-run axis.
- probe: Ledger cross-read reveals this axis unexplored; no prior forensics session enumerated conflict-graph edges between debug-window QTY pseudos and file-scope s-reg web.
- result: F3 IDENTIFIED as the un-run forensics angle; not measured this session (synthesis modality) - passed to driver as the highest-expected-information frontier item for the next forensics slot.
- verdict: CONFIRMED

## [s92] F1-refined: an inter-block statement outside block=3 can be re-shaped to add an unconditional p78/p77 (idx_1495/idx_1494) use, producing an alloc-web-lift without triggering s78's DCE-invisibility signature.
- mechanism: Full asm audit of all $s2 ($p77) and $s4 ($p78) uses in asm/funcs/cpu_side_move_dir_4.s: $s2 uses at LUIDs corresponding to block=3 (80080E74/E78 debug-window), poll callback (80080F70 conditional), post-poll (80080F9C read + 80080FC0 conditional store); $s4 use ONLY at poll-callback (80080F3C conditional). Every downstream block that CAN host an idx use already does in the h5 candidate.
- probe: Enumerated candidate host blocks: (A) block=2 loop head — impossible (would add byte count); (B) post-poll block — every scalar-carrier byte-neutral p77/p78 use DCE-eliminated per s78 measured (flow.c delete_noop_moves), every byte-non-neutral re-shape lands +13 (s8/s60/s61/s65 measured); (C) memcpy loop body — reads &D_800F19A0/writes a1, disjoint memory from D_800A1494/1495, any XOR/AND fold-linked form combine.c-eliminated (simplify_binary_operation idempotent-fold).
- result: F1-refined structural axis fully exhausted: NO un-tried inter-block re-shape survives both (a) DCE-invisibility gate and (b) target byte-count invariance gate. tmp/grind/cpu_side_move_dir_4/s92/f1_asm_audit.txt has the block-by-block enumeration.
- verdict: KILLED

## [s92] F2-refined: a stage-transition observable (mode/status/interrupt-flag read) is idiomatic at loop-top per game-loop semantics; inserting it satisfies the F2 alloc-web-lift criterion without cse-migrating idx_1494.
- mechanism: Full enumeration of observable reads in target's loop-head/test block (.L80080E1C..80080E5C): {sys_VSync call, D_800F19B8 deadline read, D_800F19BC counter read}. No game-state/interrupt-flag/mode/status observable appears.
- probe: For F2 to succeed the observable MUST appear in target asm at some LUID slot inside the loop-head/test window. It does not. Any inserted observable would REGRESS the 160-insn target count by >=3 insns (lui/lw/andi or nop-padded variants).
- result: F2-refined semantic axis KILLED by target-asm audit: no naturally-idiomatic loop-top observable exists to host the F2 mechanism. tmp/grind/cpu_side_move_dir_4/s92/f2_asm_audit.txt has the LUID-by-LUID enumeration.
- verdict: KILLED

## [s92] The h5 candidate re-application to src/system.c reproduces the ledger-documented masked=2 floor after 91 prior sessions.
- mechanism: Direct sandbox measurement.
- probe: Applied memory/grind/cpu_side_move_dir_4/candidate.c to src/system.c cpu_side_move_dir_4 body; ran `tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.
- result: score=2, target_insns=160, build_insns=160, rules_dropped=5, cheat_asm_stripped=22. Chassis integrity confirmed; the 91-session mechanism map remains valid on current tree.
- verdict: CONFIRMED

## [s93] POLL region status-arm order (status&4 before status&2) is a load-bearing byte-match ordering, not compiler-arbitrary
- mechanism: Swapping the two if-arms produces a strictly-source-ordered emission - GCC preserves C statement order on independent status& tests (no cross-jump merge / no reorder). Confirms the current arm order matches target.
- probe: P1: swap `if (status & 4) {...}` with `if (status & 2) {...}` in the POLL body; re-measure sandbox --disable all
- result: masked=7 (build=160) - regression of +5 vs baseline masked=2
- verdict: CONFIRMED

## [s93] POLL region status type-width narrowing (s32->u32) affects codegen at RTL
- mechanism: Would test whether the status pseudo's mode reaches into RA priority arithmetic or scheduler LUID assignment
- probe: P2: change fn-scope `s32 status;` to `u32 status;`; re-measure
- result: masked=2 build=160 INERT - status width is compilation-invariant at this basin
- verdict: KILLED

## [s93] POLL region status decl-scope (fn-scope vs block-local inside the vblank-if) affects codegen
- mechanism: Would test whether local-alloc.c allocno scope-classification treats block-scope pseudos differently from fn-scope ones for POLL region's local status
- probe: P3: remove fn-scope `s32 status;`, inject `s32 status;` as first stmt inside `if (sys_GetVblankCount()!=0)` block
- result: masked=2 build=160 INERT - status scope-class is compilation-invariant
- verdict: KILLED

## [s93] Hoisting `saved = (*D_800A147C) & 3;` OUT of the vblank-if is semantically equivalent (writeback still gated)
- mechanism: The read of D_800A147C moves outside the sys_GetVblankCount guard; would change one observable global read but not the visible write behavior
- probe: P4: move `saved = (*D_800A147C) & 3;` above the `if (sys_GetVblankCount())` guard
- result: masked=19 build=158 (2 fewer insns) - net semantic change and heavy regression
- verdict: KILLED

## [s93] do-while(0) wrap around the whole POLL region body emits LOOP_BEG/LOOP_END NOTEs that propagate cross-block to the do_timeout block=3 alloc web
- mechanism: Per marionation vT35/36/42/43 evidence + do-while-zero-exception.md, LOOP_BEG NOTE is a sched barrier that re-times insn placement. Un-measured for csmd4 POLL region (block=3 wraps closed s48/s85; POLL-region wraps never attempted per notes.md:128).
- probe: P5: wrap the POLL body (`saved=...; poll: ...; *D_800A147C=saved;`) in `do { ... } while(0);` inside the vblank-if
- result: masked=2 build=160 INERT - LOOP_BEG/END emission from POLL region does NOT reach the block=3 alloc web
- verdict: KILLED

## [s93] POLL region local variable declaration ORDER (`u8 saved` vs `s32 status`) affects RTL pseudo birth order for the POLL region web
- mechanism: flow.c pseudo assignment follows RTL first-use; but user_reg_class + tree pseudo numbering could differ. Would surface if any allocno tie is decided by pseudo id.
- probe: P6: swap `u8 saved;` and `s32 status;` fn-scope decl order
- result: masked=2 build=160 INERT - decl order compilation-invariant
- verdict: KILLED

## [s93] Type-narrowing `saved` from u8 to u32 changes lbu->lw emission or eliminates zero-extend
- mechanism: Would remove the andi/lbu low-byte behavior; POLL region local storage size
- probe: P7: change `u8 saved;` to `u32 saved;` (with the existing `& 3` mask keeping semantics)
- result: masked=2 build=160 INERT - saved storage width compilation-invariant (mask + writeback dominates)
- verdict: KILLED

## [s93] Block-local declaration of `u8 saved` inside vblank-if changes RA scope
- mechanism: Same as P3 for status - test scope class independence for saved
- probe: P8: remove fn-scope `u8 saved;`, inject `u8 saved;` inside vblank-if body
- result: masked=2 build=160 INERT
- verdict: KILLED

## [s93] Coalesced declaration-initialization of saved (`u8 saved = (*D_800A147C) & 3;`) inside vblank-if changes GCC tree/RTL
- mechanism: Coalescing decl+init may skip a separate SET INSN emission at expand
- probe: P9: `u8 saved = (*D_800A147C) & 3;` as a single block-local stmt
- result: masked=2 build=160 INERT
- verdict: KILLED

## [s94] PERM_GENERAL on POLL-region expressions (status != 0 vs status; status & K vs (status & K) != 0 vs status & 0xK; *idx_N vs idx_N[0]) - a mutation neighborhood at the expression level, structurally distinct from prior POLL sweeps (s67 LINESWAP over 3 statement blocks, s68 whole-block LINESWAP+GENERAL) - would yield a novel find below the h5 basin at score 60
- mechanism: Prior POLL structural sweep (s93 9-probe axis) closed at masked=2 INERT for byte-neutral variants; permuter's random cross-product over expression rewrites explores a mutation space that hand-structural sweeps do not - the compilation-invariance claim from s93 is that STRUCTURE is inert, and expression-level permuter mutations could in principle disturb the block=3 alloc web via file-level s-reg ref-balance shifts
- probe: Fresh permuter workspace tmp/grind/cpu_side_move_dir_4/s94/perm_poll (copied from tmp/perm_csmd4 with h5 block PERM_RANDOMIZE stripped and PERM_GENERAL wrappers added on 5 POLL-region expression sites: status test, two (status & K) tests, and both idx_N deref forms). Launched via tools/permuter_campaign.py with --stop-on-zero -j 8. Ran 44479 iterations in 1333.7s (~22.2 min). base_score=60 (h5 basin, correct). Harvested with --stop.
- result: 0 novel finds. 0 total finds. best_new_score=null. Iteration scores oscillated across the 60-2000 range with hits at exactly 60 (the h5 basin) but no drop below. Confirms fresh-seed permuter discipline: 20+min no-novel-find harvest is the KILLED data point.
- verdict: KILLED

## [s95] A PERM_GENERAL two-branch chassis presenting the block=3 staging window with pp-hoisted-FIRST vs pp-hoisted-LAST orderings would let the permuter's random combining find a novel basin below the h5 masked=2 (base=60) floor
- mechanism: prior permuter closures (s85 F6-family, s86 vblank-poll-arm chassis, s94 POLL PERM_GENERAL) all left the block=3 staged-decl statement-ORDER window unpermuted with pp declared/set AFTER t0 and idx_1494[1]. s93 structural POLL sweep confirmed pp-position isn't a masked=2-lowering lever at STATEMENT level via hand-structural probes, but the permuter's cross-product with per-statement micro-mutations on top of an explicit pp-order alternative had never been sampled. Two-way PERM_GENERAL branches (h5 base = t0-first; alt = pp-first-then-t0-then-shifts-then-tbl_125c-adds) give the mutator an explicit selection axis instead of relying on random line-swap discovering the reorder.
- probe: tmp/perm_csmd4 base.c edited: replace prior PERM_RANDOMIZE around block=3 with PERM_GENERAL(h5-order, pp-first-order); launch fresh-seed campaign via tools/permuter_campaign.py (label s95_perm_general_pp_ordering, -j 4); harvest --stop at ~10 min; base_score reported 60 (matches h5 masked=2)
- result: 4320 iters / 660s (~11 min) / 0 novel finds below base=60; run.log worker score distribution scanned 40..9590 range dominated by regressions, no basin below 60 sampled; also incidentally KILLED an earlier 3-way variant chassis with a u32-cast alternative (crashed the permuter's ast_types 'int - pointer' assertion at 1376 iters - documented rejected via s95_perm_general_block3_declorder harvest reason)
- verdict: KILLED

## [s96] F3: ALLOCDBG-instrumented greg allocno_conflicts edges involving {p72,p73,p78,p79} x s-reg hardregs name the atomic conflict driving the {p72<->p79, p73<->p78} 4-cycle rotation.
- mechanism: Extracted conflict lists from three existing greg dumps (s69/h5 masked=2, s88/probe1 honest-prologue masked=15, s87/probe2 honest-loop-top masked=15) via tmp/grind/cpu_side_move_dir_4/s96/diff_conflicts.py. (a) None of {p72,p73,p77,p78,p79} carry any explicit s-reg hardreg conflict (18..25) in any variant - their s-reg assignment is ord-order-downstream of priority arithmetic, not edge-driven. (b) h5 vs probe1 conflict-edge sets are BIT-IDENTICAL after pseudo-rename (101<->97, 120<->116, 126<->122, 137<->133); 80 focus edges in each; symmetric-set diff empty. (c) h5 vs probe2 differs by exactly ONE edge {p78<->p86} (removed in probe2, a call-cluster pseudo whose live range shifts with the loop-top insn), a side-effect of the extra insn, not a driver: probe1 rotates identically WITHOUT this edge change.
- probe: tmp/grind/cpu_side_move_dir_4/s96/diff_conflicts.py against s69/s87/s88 greg dumps; results in tmp/grind/cpu_side_move_dir_4/s96/diff_conflicts.txt and FORENSICS.md
- result: h5 focus edges = 80; probe1 focus edges = 80 (renamed diff = empty set); probe2 focus edges = 79 (one edge {78,86} removed). Rotation observed in BOTH probes despite probe1's zero edge delta.
- verdict: KILLED

## [s97] The 2 extra refs on p79 in h5 vs probe1 come from the RTL expansion of the cross-symbol subterm `(s32)&D_800A1494 - (s32)D_800A125C` and the outer `+ tbl_125c` restore, surviving cse.c fold_rtx because SYMBOL_REF-diff of distinct externs is not link-time-constant-foldable.
- mechanism: expr.c::expand_expr emits insn 34 (subsi3_internal, minus of p77-p79 both SYMBOL_REF-carrying pseudos) and insn 38 (addsi3_internal, plus p89+p79). cse.c::fold_rtx classifies (SYMBOL_REF A) - (SYMBOL_REF B) as NOT const because only same-object same-section label diffs fold to const_int in GCC 2.7.2 (see cse.c fold_rtx MINUS case). Combine cannot merge the sub/add pair because no MIPS md pattern accepts (plus (minus P Q) K) with pseudo operands carrying REG_EQUAL SYMBOL_REFs. flow.c::regstat_init_n_sets_and_refs, run before local-alloc, snapshots REG_N_REFS[p79]=5 (1 SET + 2 prologue USES + 2 block=3 USES). Local-alloc.c priority = floor_log2(5)*5*4/148*10000 = 675, vs probe1 nrefs=3 -> floor_log2(3)*3*4/148*10000 = 202. Priority-cascade forces ord=12 -> hardreg $s2 in h5; ord=14 -> $s5 in probe1.
- probe: instrumented gccdbg cc1 (-da BB2_ALLOC_DEBUG BB2_QTY_DEBUG BB2_SCHED_DEBUG BB2_FINDREG_DEBUG BB2_RANK_DEBUG) on h5 candidate applied to src/system.c vs probe1 (only idx_1495 line replaced with `idx_1494 + 1`). Grepped (reg[/v]?:SI 79) occurrences in csmd4.flow.csmd4 (5 for h5 at insns 24 SET, 34, 38, 111, 118; 3 for probe1 at insns 24 SET, 103, 110). Cross-checked ALLOCDBG lines for pseudo=79 (h5: nrefs=5 pri=675 hardreg=19; probe1: nrefs=3 pri=202 hardreg=21).
- result: h5: flow-time refs on p79 = 5 (insns 24 SET, 34 sub, 38 add, 111 arg-read, 118 arg-read); probe1: 3 (insns 24 SET, 103 arg-read, 110 arg-read). ALLOCDBG confirms nrefs=5 vs 3, livelen unchanged 148, pri 675 vs 202 (ratio 3.34 matches floor_log2(5)*5 / floor_log2(3)*3 = 10/3 = 3.33).
- verdict: CONFIRMED

## [s97] The mechanism is a fully closed axis: any spelling that preserves p79 nrefs>=5 upstream of block=3 requires either the semantic-lie cross-symbol arithmetic (s90/s96 KILLED, do-while-zero-exception #5 forbidden family) or a semantic-purpose observable use of tbl_125c in the prologue (no natural C-source semantic exists for a prologue tbl_125c[N] read; would fall to human-programmer-test cheat).
- mechanism: Probe1 (`idx_1495 = idx_1494 + 1;`) produces a single addsi3 (insn 30) with REG_EQUAL (const:SI (plus SYMBOL_REF K)) — cse.c fold_rtx unconditionally treats (plus SYMBOL_REF int_const) as a link-time const, eliminating any p79 reference in the derivation. Every honest respelling that reduces to `SYMBOL_REF + int` at fold time will collapse to the same const, hitting probe1's ref profile. The only spellings that preserve extra p79 refs contain non-const operands upstream: either the cross-symbol subterm (forbidden) or an observable variable operand (would change function semantics or introduce dead computation with no semantic purpose).
- probe: Cross-referenced against previously-killed spellings: s8 P1 (idx_1494+1, +13 masked +0 build, nrefs collapse), s90 P1 (&D_800A1495 direct symref, +14 masked +1 build), s90 P2 ((u8*)tbl_125c + 0x239 numeric offset, +13 masked +0 build); all three respellings measured p79 nrefs<=3 and are ledger-closed. This session's probe1 forensics REPRODUCES s8/s90 P2 signatures via a distinct diagnostic axis (flow-time ref count vs allocdbg ord), triangulating on the same underlying mechanism.
- result: Search-space narrowing: no honest respelling axis exists that both (a) preserves masked=2 basin priority arithmetic and (b) avoids the forbidden cross-symbol form at src/system.c:406. Frontier for s98+ must EITHER (A) shift to an orthogonal alloc-web scope (POLL-region, closed s93/s94; loop-top, closed s85/s86; F17 CALL-return corpus, ledger-closed) or (B) name a non-nrefs priority input (livelen accounting via loop-note-fixes-delay-slot-steal or defeat-licm-hoist-var-reuse mechanics — un-attacked as of s96).
- verdict: CONFIRMED

## [s98] An in-repo COMPLETED-C function using (s32)&SYM_A - (s32)SYM_B cross-symbol arithmetic exists as a transplant source for csmd4's idx_1495 initializer.
- mechanism: s97 named RTL insns 34 (subsi3 SYMBOL_REF-diff) + 38 (addsi3 tbl+delta) as the p79 nrefs driver. If any other in-repo function's src uses the same idiom in a COMPLETED-C body, its shape could be mined as a rederive template.
- probe: grep -rn '(s32)&D_.*- (s32)' src/
- result: 1 hit: src/system.c:406 (cpu_side_move_dir_4 itself). Zero other in-repo functions carry the cross-symbol subtraction idiom.
- verdict: KILLED

## [s98] Respelling idx_1495 with an all-SYMBOL_REF base ((s32)&D_800A125C) instead of local (u8*)tbl_125c preserves h5 masked=2 (same subtraction, same +1 constant).
- mechanism: If the h5 basin depends only on the SYMBOL_REF-SYMBOL_REF DELTA and the constant fold, then substituting the base spelling from local pseudo to raw symbol_ref should be invariant.
- probe: src edit; sandbox cpu_side_move_dir_4 --disable all
- result: masked=15 (+13). H5 basin lost. Confirms basin requires LOCAL-pseudo base combined with the symbol_ref-symbol_ref delta.
- verdict: KILLED

## [s98] Respelling the DELTA side of the subtraction as (s32)&SYM - (s32)local_pseudo (mixing SYMBOL_REF minuend with local subtrahend) preserves h5 masked=2 while adding an extra pseudo reference to p79 (predicted refs-lift lever).
- mechanism: s97 attribution names insn 38 (addsi3 tbl+delta) as p79-referencing; if delta becomes a runtime subtract on local pseudo, additional pseudo references to tbl_125c could accumulate.
- probe: idx_1495 = (u8*)((u8*)tbl_125c + (s32)&D_800A1494 - (s32)tbl_125c + 1); sandbox
- result: masked=15 (+13). Same +13 basin as probe1. Runtime pseudo-based delta is unfoldable in cse.c and does NOT emit the insns 34+38 form; RA priority profile collapses.
- verdict: KILLED

## [s98] A completed 5-arg debug_printf timeout-guard function elsewhere in BB2 src/ carries a transferrable shape for csmd4's block=3 window.
- mechanism: 5-arg debug_printf sites are rare; if any is COMPLETED-C, its block-3 arrangement could serve as an in-repo rederive template.
- probe: grep for debug_printf sites; sandbox each for status
- result: 3 relevant sites: display.c:993 func_8007DC9C (masked=9, cheat_asm_stripped=437 - INCOMPLETE), system.c marionation_Exec (masked=56 - INCOMPLETE), ings2.c:99 func_80082A14 (masked=0 but 39 cheat_asm stripped and relies on volatile counter + memory barrier - both are forbidden constructs). Zero COMPLETED-C 5-arg debug_printf transplant sources.
- verdict: KILLED

## [s99] Substituting &D_800A1495 for &D_800A1494 in the cross-symbol delta ((s32)&D_800A1495 - (s32)D_800A125C) preserves the h5 basin because the numeric-delta value (0x23A vs 0x239+1) is arithmetically equivalent.
- mechanism: s97's ledger attribution names insns 34 (subsi3 SYMBOL_REF-diff) + 38 (addsi3 tbl+delta) as p79 nrefs drivers. If the mechanism were delta-numeric-value-driven, symbol-swap of the minuend to an adjacent-address symbol should preserve fold behavior and RTL emission profile.
- probe: src/system.c line 384 → idx_1495 = (u8*)((u8*)tbl_125c + ((s32)&D_800A1495 - (s32)D_800A125C)); sandbox cpu_side_move_dir_4 --disable all
- result: masked=4 (+2 vs h5), build_insns=161 (+1 insn). h5 basin lost; RTL emission adds one insn.
- verdict: KILLED

## [s99] Moving the +1 constant INSIDE the parenthesized subtraction (((s32)&D_800A1494 + 1 - (s32)D_800A125C)) preserves the h5 basin masked=2 because cse.c canonicalizes ((A+1) - B) and ((A - B) + 1) to the same fold profile.
- mechanism: cse.c fold_rtx handles both parenthesization forms equivalently; the addsi3/subsi3 pair that emits at RTL is normalized before combine reaches try_combine (s7 mechanism trace).
- probe: src edit + sandbox --disable all
- result: masked=2, build_insns=160 INERT. Byte-identical to h5.
- verdict: CONFIRMED

## [s99] Sign-flipping the subtraction ((u8*)tbl_125c - ((s32)D_800A125C - (s32)&D_800A1494) + 1) preserves the h5 basin because cse.c fold_rtx symmetrizes the SUB direction.
- mechanism: MINUS commutativity via unary negation is canonicalized at expmed / cse fold to the same underlying SYMBOL_REF-diff RTL insn 34 shape.
- probe: src edit + sandbox --disable all
- result: masked=2, build_insns=160 INERT. Byte-identical to h5.
- verdict: CONFIRMED

## [s100] F97a-refined: natural-semantic-purpose C form emits extra pre-block=3 tbl_125c references via SYMBOL_IDENTITY-PRESERVING + SYMBOL-REF-INCREMENTAL pattern preserving (D_800A1494, D_800A125C) symbol pair AND +1 factorization.
- mechanism: s97/s98/s99 partitioned the priority-input space: livelen INVARIANT (148), conflicts INVARIANT (bit-identical edge set after 4-pseudo rename), only nrefs VARIANT and only along the cross-symbol arithmetic axis. F97a-refined's search space is any spelling preserving BOTH the (1494,125C) SYMBOL_REF pair AND the +1 factorization; predicted enumeration size = 0 semantic-purpose candidates.
- probe: Cross-read s89 (decomp.me), s90 (3 initializer respellings), s98 (2 mixed-base respellings), s99 (3 spelling-variant respellings) rederive-exhaustion notes; enumerate any surviving honest semantic-purpose spelling that (a) preserves both SYMBOL_REFs and +1 factorization at expr.c::expand_expr time, (b) has a legitimate human-programmer semantic pretext, (c) is not already in the rejected-forms bank.
- result: Enumeration size = 0. Every candidate matching (a) either duplicates a rejected form (idx1495_direct_symref_D800A1495, honest_idx_1495_ptrarith, s99_idx1495_cross_sym_D1495_no_plus1, s98_idx1495_all_symref_base, s98_idx1495_all_pseudo_multiref) OR fails (b) (no semantic-purpose pretext survives the human-programmer test for double-touching the same tbl_125c prologue byte). Rederive-modality axis is measurably exhausted per s99 note.
- verdict: KILLED

## [s101] F1 (was F97b): pointer-alias-fake-exception `s32 *tbl_alias = tbl_125c;` applied to arg5-address chain duplicates p79 references in a byte-neutral way, restoring h5's pri>675 range under the honest respelling.
- mechanism: s98 probe2 mechanism-consistency with F97b prediction that pseudo-aliasing splits refs; formal measurement using 2026-07-01 pointer-alias-fake-exception sanction (FAKE annotation + layer-2 cheat-reviewer) un-run at s100.
- probe: 5 canonical shapes measured: (A) block-scope tbl_alias=tbl_125c, arg5 only -> masked=2 build_insns=160 INERT (combine folds); (B) block-scope tbl_alias=D_800A125C (from global) -> masked=16 build_insns=161 (+14/+1insn) KILLED (extra lui/addiu materialization); (C) block-scope tbl_alias=tbl_125c, symmetric dual-use on t0 and arg5 -> masked=2 build_insns=160 INERT (both fold); (D) fn-scope tbl_alias assigned in prologue, dual-use -> masked=2 build_insns=160 INERT (fn-scope livelen doesn't stop the fold); (E) fn-scope tbl_alias, asymmetric use -> masked=10 build_insns=163 (+8/+3insn) KILLED (extra prologue materialization + local-alloc rotation).
- result: 5/5 KILLED: every byte-neutral form (A/C/D) is combine-folded so refs do NOT split on p79; every ref-splitting form (B/E) requires an extra materialization insn that regresses bytes. There is no byte-neutral pointer-alias shape that duplicates p79 refs on tbl_125c because tbl_125c is itself a pseudo (not a global), so all pass-through aliases collapse in combine.c.
- verdict: KILLED

## [s102] F2: shorten p79 livelen from 148 to <60 via structural declaration-hoist of tbl_125c=D_800A125C, keeping nrefs at probe1's 3-ref profile, to re-enter pri>675 range and re-establish ord=12 h5 rotation while using the honest idx_1495=idx_1494+1 form.
- mechanism: s97 named priority formula pri = floor_log2(refs)*refs*size/livelen*10000; probe1 measures pri=202 (nrefs=3,livelen=148); to reach pri>675 need livelen<44; structural levers per index (block-local var splits, declaration order, statement re-association) are the only remaining sanctioned modality; target's fn-prologue tbl_125c materialization would still materialize D_800A125C early leaving p79 lifetime unchanged if RTL insn scheduling re-lifts it.
- probe: 6 structural probes on probe1-honest base: P2 defer tbl_125c=D_800A125C to just before do_timeout label; P3 declare+init tbl_125c inside block=3 compound; P4 reorder fn-prologue so tbl_125c is LAST assignment before loop label; P5 fn-prologue tbl_125c + block-scope pass-through alias `tbl_alias=tbl_125c` used only inside block=3; P6 eliminate local tbl_125c, use D_800A125C symref directly at both block=3 uses; +control P7 h5-symref form (idx_1495 init using D_800A125C symref) with block-local tbl_125c inside block=3.
- result: P1 probe1 base masked=15 build=160 (baseline, matches s97/s99); P2 masked=22 build=158 (-2 insns, +7 regression from base); P3 masked=22 build=158 (-2 insns, +7); P4 masked=17 build=160 (+2 regression, no insn delta); P5 masked=15 build=160 INERT (combine folds tbl_alias to tbl_125c pseudo, confirming s101 Probe A mechanism); P6 masked=35 build=159 (-1 insn, +20 catastrophic); P7 masked=22 build=158 (-2 insns, +7). Target build_insns=160 requires the fn-prologue lui/addiu of D_800A125C; any deferral loses those 2 insns asymmetrically. Post-restore HEAD masked=7 (both-named baseline reproduced). h5 candidate re-verified masked=2 (floor unchanged).
- verdict: KILLED

## [s103] The F3 contingent hypothesis (every sanctioned pure-C axis measured dead → file OWNER-ESCALATION per task-brief owner-gated contract) is now activated: prior sessions s101 (F1 pointer-alias-fake-exception 5 shapes) and s102 (F2 livelen-shortening 6 shapes) closed the last two identified un-run sanctioned axes; canonical-asm settled FAIL per 2026-07-09 01:25 Judge constraint; no further C-reachable axis remains within the standing sanctioned set.
- mechanism: Task-brief owner-gated contract: emit ONLY when a filed OWNER-ESCALATION exists AND every remaining sanctioned axis is measured dead. s103 satisfies the first precondition by filing the entry itself (docs/grind/decisions.md:848), and the ledger closure inventory across s3-s102 satisfies the second (structural 46+ probes; do-while(0) at all scopes s4/s5/s48/s85/s93; carriers s4/s5; named dispatch s5; fn-body hoists s13/s31; marionation transplants s9/s2/s4; m2c KILLED s8; spelling variants s90/s98/s99; POLL 9-probe sweep s93; permuter 6 chassis ~81k+ iters 0 novel s5/s13/s14/s86/s94/s95; F97a-refined analytic closure s100; F97b/c empirical s90/s98; s101 pointer-alias-fake-exception 5-shape; s102 livelen-shortening 6-shape).
- probe: s103 filed OWNER-ESCALATION entry at docs/grind/decisions.md:848 in hirahira_w_frie / motion_SetMotion format with (a) full closure inventory across all 102 sessions, (b) both owner options presented honestly — sanction cross-symbol arithmetic idiom as SOTN-family carve-out contingent on evidence (would close via existing h5 candidate.c on candidate-ready path) vs owner-signed permanent-INCOMPLETE disposition — and (c) explicit statement that no further C-reachable axis remains within the sanctioned set. No new src/system.c edits: candidate.c preserved unchanged as h5 masked=2/raw=2 form. Emitting owner-gated citing escalation_ref.
- result: OWNER-ESCALATION filed at docs/grind/decisions.md:848. Both dispositions presented honestly; agent does not self-resolve.
- verdict: CONFIRMED

## s104 — escalation modality — DISPOSITION REACHED (terminal)

**Chassis re-measured, not quoted.** The 2026-08-19 asm-until-matched migration changed this
function's representation on main: `src/system.c:376` is now `INCLUDE_ASM("asm/funcs", CD_sync);`
with 0 regfix/asmfix rules holding a byte match (the 5 rules cited in the 2026-07-20 owner ruling are
gone; `regfix.txt` retains a single non-load-bearing reference). To confirm the ledger's floor is
still valid on THIS chassis, s104 substituted `memory/grind/CD_sync/candidate.c` (symbol renamed
`cpu_side_move_dir_4` -> `CD_sync`) for the INCLUDE_ASM line and ran the gradient:
**score=2, target_insns=160, build_insns=160, rules_dropped=0** — identical to the s103 floor.
`src/system.c` was reverted to HEAD immediately (working tree clean apart from the ledger, the
decisions.md entry, and the untracked scratch dir). CONCLUSION: the migration is floor-neutral for
CD_sync; every s1–s103 spelling conclusion remains chassis-valid and must not be re-derived.
`memory/grind/CD_sync/candidate.c` has been rewritten with the `CD_sync` symbol name so a future
re-attempt can apply it directly.

**Gate (a) canonical-asm — FAILS (re-run this session, not inherited).**
`tools/scan_hand_coded.py --single CD_sync` = `tier=LOW score=2/8 (160 insns)`. Only S4 (4 loads in
an 8-insn window @ insn 49) and S5 (1 approx-sibling `CD_ready`, jaccard=0.64) fire. The STRONG
signals S1/S2/S6 are all absent (0 multu/mflo pairs, no empty-body branches, no BIOS jumptable
pattern); S3/S7/S8 negative. Artifact: `tmp/grind/CD_sync/s104/scan_hand_coded.txt`. Note this is a
score CHANGE from the 2026-07-20 ruling's cited "2/8" under the old name — same tier, same verdict.
Independently barred by the 2026-07-09 Judge constraint.

**Gate (b) SOTN precedent — FAILS (fresh census this session).**
Closing construct is the cross-symbol arithmetic idiom
`idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1)`. Searched
`docs/reference/sotn-construct-index.md` for cross-symbol / symbol-difference / `&D_xxxx[1]` shapes:
**zero hits**. This is the first time the machine-generated SOTN construct index (added after the
2026-07-20 ruling) has been searched for this idiom, and it reproduces the s98 manual survey's
negative rather than opening a new question. A negative census is a FAILED gate.

**Disposition filed:** `docs/grind/decisions.md:7993` —
`## 2026-08-20 — CD_sync (src/system.c) — **OWNER-ESCALATION — RESOLVED BY STANDING RULING
(2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE**`. Both AND-gates fail, which is the owner's
pre-decided case; the disposition is terminal and nothing is pending on the owner. Session outcome
`owner-gated` with that entry as `escalation_ref`.

**What a future re-attempt would need (do not re-run the closed space):** a pure-C lever that
reorders `{sll@54, addu@55}` at the `sched.c` LUID tiebreak WITHOUT paying the p106/val5 allocno tie
(cost 6). Every axis the ledger identified as sanctioned and un-run is now empirically closed:
F1 pointer-alias-fake-exception (s101, 5 shapes), F2 livelen-shortening (s102, 6 shapes),
F3 conflict-graph attribution (s96/s97), F97a analytic closure (s100), F97b/c (s90/s98), permuter
across 6 chassis (~81k+ iters, 0 novel basin closures), m2c (s8), in-repo transplant (s98), and the
~40 in-family decompositions banked as 112 rejected forms.

## [s104] The 2026-08-19 asm-until-matched migration (5 regfix/asmfix rules removed, src/system.c:376 now INCLUDE_ASM) changed CD_sync's honest pure-C floor.
- mechanism: Rule removal alters the sandbox chassis; a rules-dropped delta or a different TU compilation context could shift the residual pair {sll@54, addu@55}.
- probe: Substituted memory/grind/CD_sync/candidate.c (symbol renamed cpu_side_move_dir_4 -> CD_sync) for the INCLUDE_ASM line at src/system.c:376 and ran `sandbox CD_sync --disable all`; reverted src/system.c immediately after.
- result: score=2, target_insns=160, build_insns=160, scorable=true, rules_dropped=0 - bit-for-bit the s103 floor.
- verdict: KILLED

## [s104] Gate (a): CD_sync qualifies for the canonical-asm grant path (STRONG scan_hand_coded tier).
- mechanism: endgame-lock-disposition AND-gate 1 requires STRONG hand-coded signals S1/S2/S6 from tools/scan_hand_coded.py.
- probe: python3 tools/scan_hand_coded.py --single CD_sync (artifact tmp/grind/CD_sync/s104/scan_hand_coded.txt).
- result: tier=LOW score=2/8. Only S4 (4 loads in an 8-insn window @ insn 49) and S5 (approx-sibling CD_ready, jaccard=0.64) fire. S1 (0 multu/mflo pairs), S2 (no empty-body branches), S6 (no BIOS jumptable pattern) all absent; S3/S7/S8 negative. Independently barred by the 2026-07-09 Judge constraint against re-surfacing canonical-asm for this function or its twins.
- verdict: KILLED

## [s104] Gate (b): an in-hand SOTN-master precedent exists for the closing construct (the cross-symbol arithmetic idiom idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1)).
- mechanism: endgame-lock-disposition AND-gate 2 requires a citable file:line precedent from SOTN master; docs/reference/sotn-construct-index.md is the machine-generated census of every match-hack construct SOTN ships.
- probe: Grepped docs/reference/sotn-construct-index.md for cross-symbol / symbol-difference / &D_xxxx[1] / 'symbol' shapes (first search of this index for this idiom - the index post-dates the 2026-07-20 ruling).
- result: Zero hits. Reproduces the s98 manual SOTN/Vagrant Story/ESA survey negative and the s98 in-repo transplant kill (idiom unique to this function in the entire BB2 tree). The owner already REFUSED this exact family on 2026-07-20 as an unsanctioned coercion family.
- verdict: KILLED

## [s105] Owner directive axis 1 (2026-08-24): the sanctioned F1 combine-foldable chain-extender family (dead-store-fake-exception.md:32-46, owner ruling 2026-07-01) can bump the arg5 allocno's reg_n_refs enough to break the p106/val5 birth-tie, byte-neutrally, on the g3 order-perfect base.
- mechanism: flow.c records reg_n_refs BEFORE combine.c folds a link-constant SYMBOL_REF detour; global.c's allocno priority is refs-weighted, so extra pre-fold refs on the arg5 value/address pseudo should raise its priority above the t0 chain's and let arg5 birth first (the g3 frontier's stated requirement: arg5-qty refs >= 4 via a LOCAL copy-tie).
- probe: 10 measurements, all `sandbox CD_sync --disable all` with candidate.c spliced at src/system.c:376. Bases: h5 (masked 2) and g3 (`v0<<=2; arg5=*(s32*)(v0+(s32)tbl_125c); t0<<=2; call(..., *(s32*)(t0+(s32)tbl_125c), arg5)`), g3 control re-measured at 6/160 this session. Detours: (v1/v4/v6) VALUE `arg5 = (s32)((u8*)arg5 + ((s32)tbl_125c - (s32)D_800A125C))` x1 h5, x1 g3, x2 h5; (v2/v5) ADDRESS `arg5 = *(s32*)(v0 + ((s32)D_800A125C + ((s32)tbl_125c - (s32)D_800A125C)))` on h5/g3; (v7-v10) idx_1494 link-constant delta `*(s32*)(v0 + ((s32)idx_1494 + ((s32)D_800A125C - (s32)&D_800A1494)))` in both associations on both bases.
- result: VALUE detours MATERIALIZE (build_insns 164/164/166 vs target 160) -> they fail the family's own non-materialization prerequisite and are not legal instances at all; scores 19/11/30. idx_1494-delta detours MATERIALIZE (166) on both bases and both associations; scores 21/22. Only the ADDRESS detours are byte-neutral (160/160) - they fold by tree-level re-association, not by the combine SYMBOL_REF fold - and they REGRESS to 20 (h5) / 25 (g3). Zero probes at or below the floor.
- verdict: KILLED. Structural, not empirical: a chain-extender is byte-neutral only when the ENTIRE detour collapses to a link-time constant (which is exactly why the existing idx_1495 extender folds - its result &D_800A1494+1 is link-known). Both operands of the p106/val5 tie (arg5's value and arg5's address) are runtime-dependent through the loaded index v0, so NO cross-symbol detour on them can fold to zero bytes. The F1 family is structurally excluded from this residual; do not re-attempt it under any spelling.

## [s105] Owner directive axis 2 (2026-08-24): the psyz PsyQ-4.0 decomp (Xeeynamo/psyz, the version-correct 4.0 reference for BB2's verbatim library windows) holds a matched C body for CD_sync that can be transplanted.
- mechanism: BB2 links Sony's prebuilt PsyQ 4.0 BIOS.OBJ verbatim (memory/closer/libcd-groundtruth.md - all 1,526 .text words masked-identical); if psyz has matched C for bios.c's CD_sync, that C IS the original source shape and closes the function by provenance rather than by search.
- probe: fetched https://raw.githubusercontent.com/Xeeynamo/psyz/master/decomp/src/libcd/bios.c directly this session (artifact tmp/grind/CD_sync/s105/psyz_bios.c, 208 lines) and inspected every function in the TU.
- result: line 94 is `INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_sync);`. psyz has NO matched body for CD_sync, and none for getintr / CD_ready / CD_cw / CD_datasync / CD_getsector / CD_getsector2 / callback either - the entire BB2-relevant part of bios.c is unmatched there too. No reference source seed exists at any PsyQ version.
- verdict: KILLED (first-hand measurement, not an inherited citation - reproduces the 2026-08-18 address-keyed psyz sweep's negative by direct inspection).

## [s105] The idx_1495 cross-symbol chain-extender in candidate.c is cosmetic and can be replaced by honest C at the same floor.
- mechanism: s99 recorded "arithmetic-equivalent +1-position and sign-flip both INERT", which could be read as the honest spelling being free; if so the masked-2 floor would carry no FAKE construct at all.
- probe: three honest respellings of the prologue line, each measured on the otherwise-unchanged candidate: (p1) `idx_1495 = idx_1494 + 1;`, (p2) `idx_1495 = &idx_1494[1];`, (p3) `idx_1495 = (u8 *)&D_800A1494 + 1;`.
- result: 15 / 15 / 15, all at build_insns 160, vs the candidate's 2. The cross-symbol extender is LOAD-BEARING for the entire 13-point gap.
- verdict: KILLED (the hypothesis that it is cosmetic). Consequence: candidate.c now carries the mandatory /* FAKE: ... */ annotation naming the family (dead-store-fake-exception.md:32-46), the mechanism (flow.c reg_n_refs before the combine.c SYMBOL_REF fold), the byte-neutrality evidence (160 == 160) and the lever-exhaustion pointer. Re-verified after annotating: score still 2 / 160 (same-line comment, no line-note shift).

## [s105] The sanctioned F1 combine-foldable chain-extender family (.claude/rules/dead-store-fake-exception.md:32-46, owner ruling 2026-07-01, named by the 2026-08-24 owner directive) can bump the arg5 allocno's reg_n_refs byte-neutrally and break the p106/val5 birth-tie on the order-perfect g3 base.
- mechanism: flow.c records reg_n_refs BEFORE combine.c folds a link-constant SYMBOL_REF detour; global.c allocno priority is refs-weighted, so extra pre-fold refs on the arg5 value/address pseudo should let arg5 birth before the t0 chain (the g3 frontier's stated requirement: arg5-qty refs >= 4 via a LOCAL copy-tie).
- probe: 10 sandbox measurements with candidate.c spliced at src/system.c:376, on both live bases (h5 = masked-2 floor; g3 = masked-6 order-perfect, control re-measured 6/160). Detours: VALUE 'arg5 = (s32)((u8*)arg5 + ((s32)tbl_125c - (s32)D_800A125C))' x1 h5 / x1 g3 / x2 h5; ADDRESS 'arg5 = *(s32*)(v0 + ((s32)D_800A125C + ((s32)tbl_125c - (s32)D_800A125C)))' on h5 and g3; idx_1494 link-constant delta '*(s32*)(v0 + ((s32)idx_1494 + ((s32)D_800A125C - (s32)&D_800A1494)))' in both associations on both bases.
- result: VALUE detours materialize (build_insns 164/164/166 vs target 160) and score 19/11/30; idx_1494-delta detours materialize (166) and score 21/22; only the ADDRESS detours stay byte-neutral (160) - and they fold by tree-level re-association, not by the combine SYMBOL_REF fold - regressing to 20 (h5) and 25 (g3). Zero probes at or below the floor of 2.
- verdict: KILLED

## [s105] The psyz PsyQ-4.0 decomp (Xeeynamo/psyz), the version-correct reference for BB2's verbatim Sony library windows, holds a matched C body for libcd bios.c CD_sync that can be transplanted.
- mechanism: BB2 links Sony's prebuilt PsyQ 4.0 BIOS.OBJ verbatim (memory/closer/libcd-groundtruth.md: all 1,526 .text words masked-identical), so a matched psyz body would BE the original source shape and close the function by provenance instead of by search.
- probe: Fetched https://raw.githubusercontent.com/Xeeynamo/psyz/master/decomp/src/libcd/bios.c this session (tmp/grind/CD_sync/s105/psyz_bios.c, 208 lines) and inspected every function in the TU.
- result: Line 94 is INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_sync). psyz has no matched body for CD_sync, nor for getintr / CD_ready / CD_cw / CD_datasync / CD_getsector / CD_getsector2 / callback. No reference source seed exists at any PsyQ version. (The fetch did confirm the symbol identities: D_800A125C = CD_intstr[8], D_800A1494/95 = the Result pair, D_800A11DC[D_800A11D5] = CD_comstr[CD_com], D_800161C8 = "%s:(%s) Sync=%s, Ready=%s\n".)
- verdict: KILLED

## [s105] The idx_1495 cross-symbol chain-extender in candidate.c is cosmetic and an honest spelling reaches the same masked-2 floor (a reading s99's 'arithmetic-equivalent forms INERT' note allows).
- mechanism: If honest C reached 2, the floor form would carry no FAKE construct at all and the disposition's construct analysis would change.
- probe: Three honest respellings measured on the otherwise-unchanged candidate: idx_1495 = idx_1494 + 1; idx_1495 = &idx_1494[1]; idx_1495 = (u8 *)&D_800A1494 + 1.
- result: 15 / 15 / 15, all at build_insns 160, against the candidate's 2. The extender is load-bearing for the whole 13-point gap. candidate.c now carries the mandatory /* FAKE: what + mechanism + lever-exhaustion */ annotation naming the family; re-measured after annotating: still score 2 / 160.
- verdict: KILLED

## [s106] Campaign-sweep axis (a): a MILD t0-side demotion (inverse.py vectors `refs_down p84 7->{6,5}` / `live_extend p84 9->{11,13,17}`) breaks the p106/val5 birth-tie on the g3 order-perfect chassis without collapsing t0 into the refs-2 equiv-sink the bank already measured at 14.
- mechanism: global.c allocno priority is refs-weighted and livelen-normalized; lowering p84 (the t0/idx chain) below p82 (the arg5 value) should hand $a0 to the arg5 pseudo and produce the 6x $v1->$a0 / 2x $a0->$v1 exchange the solver's goal_from_tgt enumerated at normalized sites 49/55/56/59/61/65. The 2026-08-30 solver sweep flagged this as UNPROBED because the 105-session bank only ever measured the FULL refs-2 collapse of t0, never a one-step demotion.
- probe: 5 sandbox measurements on the g3 base (control re-measured 6/160 this session), all with the block spliced into memory/grind/CD_sync/candidate.c over `INCLUDE_ASM("asm/funcs", CD_sync);` at src/system.c. (a1) t0 chain fully inlined at the call site, no named local -> refs_down maximal; (a2) `t0 = idx_1494[0] << 2;` single def, removing the read-modify-write -> refs_down 7->6; (a3) t0 as a `s32 *tp` pointer local carrying base+index -> refs_down with a different rtx class; (a4) `t0 = (s32)tbl_125c; ... t0 += idx_1494[0] << 2;` base-accumulator -> live_extend of the t0 range across the arg5 load; (a5) `t0b = t0 << 2;` split shift into a second local -> live_extend + refs redistribution.
- result: 14 / 14 / 9 / 9 / 14, every one at build_insns 160 == target 160 (all byte-neutral, so all are legal instances of the axis; none is disqualified on materialization). Zero probes at or below the g3 control of 6, and nothing anywhere near the h5 floor of 2. The g3 basin is rigid: every t0-side demotion lands on the discrete set {9, 14} regardless of whether the demotion is by refs or by livelen, and regardless of rtx class (int vs pointer).
- verdict: KILLED. Axis (a) is exhausted at depth 1: a one-step demotion is NOT qualitatively different from the refs-2 collapse the bank already measured - both fall into the same two attractor scores. The solver's ranked vectors are reachable in the MODEL but every C spelling that realizes them also perturbs the emission order, which re-enters the g3-basin trap.

## [s106] Campaign-sweep axis (b): a p82 live-SPLIT (giving the short-lived arg5 value its own local so its range stops at its last real use) raises arg5's allocno above the t0 chain and wins the $a0 seat.
- mechanism: global.c prioritizes by refs/livelen; shrinking p82's live range raises its priority without touching its refs. The solver enumerated `live_shrink p82 10->{8,6,2}` as a valid single-atom vector. The sweep flagged this as distinct from the probed staged-value forms, which SHARE v0 rather than splitting.
- probe: 6 sandbox measurements. On the g3 chassis: (b1) arg5's index gets its OWN fresh local `ix` instead of borrowing the staged `v0`; (b2) `ap = (s32 *)(ix + (s32)tbl_125c); arg5 = *ap;` address/value split; (b3) no arg5 local at all, `*ap` passed directly as the 5th argument -> maximal shrink (range 0); (b4) ap split with the deref moved after `t0 <<= 2` -> minimal range. On the h5 (floor-2) chassis: the ap split, and the ap split combined with the fresh `ix`.
- result: g3: b1=6, b2=6, b4=6 (all INERT - identical to the control), b3=14 (maximal shrink REGRESSES into the same attractor as axis (a)). h5: both variants = 2, build_insns 160 - INERT at the floor. Zero improvement anywhere.
- verdict: KILLED. Live-splitting p82 is byte-invisible on both chassis. GCC 2.7.2's local-alloc.c coalesces the split locals back into a single quantity before global.c ever sees the shortened range, so the modelled `live_shrink` atom has no C-level realization here; only the DESTRUCTIVE spelling (b3, no local at all) changes anything, and it changes it the wrong way.

## [s106] The `v0` staged-value borrow in the floor-2 candidate (a /* FAKE */ construct under staged-value-reused-variable) is load-bearing for the masked-2 score.
- mechanism: the annotation claims sched.c adjust_priority/birthing_insn_p depends on the arg5 index being staged through the already-live `v0` local rather than a fresh one; if true, the honest spelling would regress.
- probe: replaced `v0 = idx_1494[1]; ... v0 <<= 2; arg5 = *(s32 *)(v0 + (s32)tbl_125c);` with a fresh honest local `ix` (b_h5_ix_own_local.c), and again in combination with the ap split (b_h5_ix_ap_split.c).
- result: 2 / 2, both at build_insns 160 == target 160. Bit-identical to the h5 control (2/160). The borrow is NOT load-bearing.
- verdict: KILLED (the load-bearing claim). CONSEQUENCE - candidate.c updated this session: the staged-value FAKE is REMOVED and replaced with an ordinary fresh local, re-measured at score 2 / 160 / rules_dropped 0. The floor-2 form now carries TWO FAKE constructs instead of three: the sanctioned pointer-alias `pp` and the owner-REFUSED cross-symbol idx_1495 chain-extender.

## [s106] The remaining two FAKE constructs in the floor-2 candidate are also cosmetic and can be dropped like the v0 borrow was.
- mechanism: if the s106 v0-borrow result generalizes, the whole annotated scaffold might be removable and the floor-2 form would be honest C.
- probe: (h5_ix_nopp) removed the `pp` pointer-alias, passing `D_800F19C0` directly at the call; (h5_ix_nopp_simple_t0) additionally collapsed the two-step `t0 *= 4; t0 = (s32)((u8 *)tbl_125c + t0);` into `t0 <<= 2;` + in-call add.
- result: 8 and 10 respectively, both at build_insns 160. Sharp regressions from 2.
- verdict: KILLED. The `pp` pointer-alias AND the t0 two-step addressing are BOTH load-bearing; combined with s105's measurement that all three honest respellings of idx_1495 score 15, the floor-2 form's remaining scaffold is irreducible. The generalization does not hold - only the v0 borrow was free.

## [s106] A MILD t0-side allocno demotion (inverse.py vectors refs_down p84 7->{6,5} / live_extend p84 9->{11,13,17}) breaks the p106/val5 birth-tie on the g3 order-perfect chassis without collapsing t0 into the refs-2 equiv-sink the bank already measured at 14.
- mechanism: global.c allocno priority is refs-weighted and livelen-normalized; lowering p84 (the t0/idx chain) below p82 (the arg5 value) should hand $a0 to the arg5 pseudo and produce the 6x $v1->$a0 / 2x $a0->$v1 exchange goal_from_tgt enumerated at normalized sites 49/55/56/59/61/65.
- probe: 5 sandbox measurements on the g3 base (control re-measured 6/160 this session): t0 inlined at the call site; t0 = idx_1494[0] << 2 single-def; t0 as a s32* pointer local; base-accumulator t0 = (s32)tbl_125c; t0 += idx_1494[0] << 2; split shift into a second local t0b.
- result: 14 / 14 / 9 / 9 / 14, every one at build_insns 160 == target_insns 160 (all byte-neutral, none disqualified on materialization). Zero probes at or below the g3 control of 6.
- verdict: KILLED

## [s106] A p82 live-SPLIT (giving the short-lived arg5 value its own local so its range stops at its last real use) raises arg5's allocno above the t0 chain and wins the $a0 seat.
- mechanism: global.c prioritizes by refs/livelen; shrinking p82's live range raises its priority without touching its refs. The solver enumerated live_shrink p82 10->{8,6,2} as a valid single-atom vector, distinct from the probed staged-value forms which SHARE v0 rather than splitting.
- probe: 6 sandbox measurements. g3: arg5's index given its own fresh local ix; ap = (s32*)(ix + (s32)tbl_125c) address/value split; no arg5 local at all (maximal shrink, *ap passed directly); ap split with the deref moved after t0 <<= 2. h5 (floor chassis): the ap split, and ap split combined with the fresh ix.
- result: g3: 6 / 6 / 6 INERT (identical to control) and 14 for the maximal shrink. h5: 2 / 2 INERT at the floor. All at build_insns 160. Zero improvement anywhere.
- verdict: KILLED

## [s106] The v0 staged-value borrow in the floor-2 candidate (a /* FAKE */ construct under staged-value-reused-variable) is load-bearing for the masked-2 score.
- mechanism: The annotation claims sched.c adjust_priority/birthing_insn_p depends on the arg5 index being staged through the already-live v0 local rather than a fresh one.
- probe: Replaced v0 = idx_1494[1]; ... v0 <<= 2; arg5 = *(s32*)(v0 + (s32)tbl_125c) with a fresh honest local ix, alone and in combination with the ap split.
- result: 2 / 2, both at build_insns 160 == target 160 - bit-identical to the h5 control. The borrow is NOT load-bearing. candidate.c updated: the staged-value FAKE removed, re-measured at score 2 / 160 / rules_dropped 0.
- verdict: KILLED

## [s106] The remaining two annotated constructs in the floor-2 candidate are likewise cosmetic and can be dropped like the v0 borrow was.
- mechanism: If the v0-borrow result generalizes, the whole annotated scaffold is removable and the floor-2 form would be honest C.
- probe: Removed the pp pointer-alias (D_800F19C0 passed directly at the call); then additionally collapsed the two-step t0 *= 4; t0 = (s32)((u8*)tbl_125c + t0) into t0 <<= 2 plus an in-call add.
- result: 8 and 10 respectively, both at build_insns 160 - sharp regressions from 2. Combined with s105's measurement of all three honest idx_1495 respellings at 15, the remaining scaffold is irreducible.
- verdict: KILLED

## [s106] Gate (a): CD_sync qualifies for the canonical-asm grant path (STRONG scan_hand_coded tier).
- mechanism: endgame-lock-disposition AND-gate 1 requires STRONG hand-coded signals S1/S2/S6.
- probe: python3 tools/scan_hand_coded.py --single CD_sync (artifact tmp/grind/CD_sync/s106/scan_hand_coded.txt).
- result: tier=LOW score=2/8 (160 insns). Only S4 (4 loads in an 8-insn window @ insn 49) and S5 (approx-sibling CD_ready, jaccard 0.64) fire; S1/S2/S6 absent, S3/S7/S8 negative. Third consecutive identical result (s104, s105, s106).
- verdict: KILLED

## [s106] Gate (b): an in-hand SOTN-master precedent exists for the closing construct (the cross-symbol arithmetic idiom idx_1495).
- mechanism: endgame-lock-disposition AND-gate 2 requires a citable file:line precedent; docs/reference/sotn-construct-index.md is the machine-generated census of every match-hack construct SOTN master ships.
- probe: Searched the index for cross-symbol / symbol-difference / (s32)&D_xxxx shapes AND for live-split shapes (artifact tmp/grind/CD_sync/s106/gate_b_sotn_census.txt).
- result: Zero hits on both. Reproduces the s104 census negative, the s98 manual SOTN/Vagrant Story/ESA survey negative, and the s98 in-repo transplant kill. The owner already REFUSED this exact family on 2026-07-20.
- verdict: KILLED

## 2026-09-01 — operator reopen note (owner ruling 2026-09-01 (decisions.md FORECLOSED-BUCKET REVIEW entry))

Returned to active under Ruling A; executes via the Ruling D CD_intr aggregate-merge session (sanctioned family 2026-08-17; prong-(c) asm-consumer check mandatory first). This ledger's own gap: no hypothesis for split-scalars-hide-aggregate exists despite closer Ruling 1 (2026-07-09) naming the merge as the honest replacement and W4 answering YES for 0x800A1494/95/96. The 'idx_1494 is not IRQ-mutated' volatile-rejection premise is FALSE (cdrom_IrqHandler + CD_flush, same TU, matched, declare these bytes volatile). Falsifiable claim: the volatile CD_intr aggregate declaration scores below 2/160 and retires the refused cross-symbol idiom.

## s107 [escalation] — owner directive 2026-09-01 Ruling D (CD_intr aggregate merge) EXECUTED and KILLED

- **hypothesis (verbatim from the 2026-09-01 operator reopen note):** "the volatile
  CD_intr aggregate declaration scores below 2/160 and retires the refused
  cross-symbol idiom."
- **mechanism claimed:** the target addresses the libcd `Intr` bytes base+offset
  (`lbu 0/1($s1)`), so declaring 0x800A1494/95/96 as one aggregate
  (`typedef struct { u8 sync, ready, c; } CD_intr;`) instead of two per-word splat
  scalars would reproduce the base-register addressing honestly and remove the need
  for the `(s32)&D_800A1494 - (s32)D_800A125C` cross-symbol delta the owner REFUSED
  on 2026-07-20.
- **probe:** 5 builds on the re-measured chassis (candidate spliced at
  `src/system.c:376`, baseline re-confirmed **2 / 160 / 160**, rules_dropped 0).
  The aggregate was declared ON the existing splat symbol `D_800A1494` so the
  measurement needed no splat-config change. `tmp/grind/CD_sync/s107/mkprobe.py`,
  `mkprobe5.py`.
- **result:**

  | probe | spelling | score | build_insns |
  |---|---|---|---|
  | baseline | banked candidate.c (cross-symbol FAKE lever) | **2** | 160 |
  | p1 | `extern volatile CD_intr D_800A1494;` + direct `.sync` / `.ready` | 34 | 159 |
  | p2 | volatile aggregate reached through `volatile CD_intr *ip` | 32 | 157 |
  | p3 | non-volatile aggregate + direct member access | 34 | 159 |
  | p4 | non-volatile aggregate through `CD_intr *ip` | 18 | 157 |
  | p5 | aggregate declaration + existing lever kept via `(u8 *)&D_800A1494` pun | **2** | 160 |

- **verdict: KILLED.** Every prong-conformant spelling of the merge scores 18-34,
  i.e. 9x-17x WORSE than the standing floor of 2 — the claim "scores below 2/160" is
  measured FALSE in all four member-access spellings, volatile and non-volatile,
  direct and base-pointer. p4 (18) simply re-lands in the known honest-respelling
  basin s105 measured at 15; the aggregate adds nothing over `idx_1494[0]/[1]`.
  p5 shows the aggregate DECLARATION is byte-neutral (still 2/160) but only while
  the refused cross-symbol delta is retained through a `(u8 *)&D_800A1494` pointer
  pun — which prong (d) of the sanctioned family explicitly forbids ("never a
  per-use pointer pun") and which retires nothing. The merge is therefore neither a
  lever nor an honest replacement for the closing construct.
- **prong (c) — the MANDATORY first step of Ruling D — is STRUCTURALLY
  UNSATISFIABLE, independently of the scores.** Prong (c) requires "every merged
  per-word symbol is removed from C and from the splat symbol config, leaving
  exactly one C handle per storage location". Measured this session:
  `asm/data/7D920.data.s:31048-31076` *defines* `D_800A1494`, `D_800A1495`,
  `D_800A1496` (and the `D_800A1498` descriptor whose first word is
  `.word D_800A1494`) as dlabels, and eight assembly consumers reference them **by
  name**: `asm/funcs/CD_cw.s` (6 sites), `getintr.s` (5), `func_800819C4.s` (5,
  incl. D_800A1498), `func_800817A0.s` = CD_flush (4), `func_80081E1C.s` (1),
  plus the three INCLUDE_ASM bodies `CD_sync.s` / `CD_ready.s` / `CD_datasync.s`.
  The per-word symbols must therefore survive in the splat symbol config, so an
  aggregate handle is necessarily a SECOND handle on the same storage. This is
  verbatim the failure mode that killed the g_stage_id merge
  (decisions.md:10722: "prong (c) is unsatisfiable today ... the merge cannot leave
  one handle per location").
- **corollary for the sibling ledgers (CD_ready, CD_datasync F14):** the prong-(c)
  finding is symbol-level, not function-level — it holds identically for every
  member of the Ruling D set. The Ruling D scope grant cannot be spent by any of
  the three until the asm-only consumers (`CD_cw`, `getintr`, `func_800819C4`,
  `func_800817A0`, `func_80081E1C`) are themselves decompiled to C.
- **also corrected:** the reopen note's premise that the volatile-rejection was
  based on a false "not IRQ-mutated" claim is beside the point here — volatility was
  tested directly (p1/p2) and is strictly WORSE (34/32) than non-volatile (34/18),
  because `volatile` forces a fresh `lui/%lo` materialisation per access and
  destroys the single-base addressing the merge was supposed to create.

## [s107] Ruling D / operator reopen note: 'the volatile CD_intr aggregate declaration scores below 2/160 and retires the refused cross-symbol idiom' - declaring 0x800A1494/95/96 as one aggregate reproduces the target's lbu 0/1($s1) base+offset addressing honestly.
- mechanism: The sanctioned per-word-splat-symbol -> aggregate merge family (no-new-park-categories.md:238-254, owner ruling 2026-08-17) replaces per-symbol lui/%lo materialisation with a single base register, which is the addressing shape the original bytes exhibit; volatility is grounded by the matched in-TU IRQ consumers (cdrom_IrqHandler, CD_flush).
- probe: 5 sandbox builds on the re-measured chassis with the aggregate declared on the existing splat symbol D_800A1494 (no splat-config change needed for the measurement): p1 volatile+direct member, p2 volatile+base pointer, p3 non-volatile+direct, p4 non-volatile+base pointer, p5 aggregate declaration with the existing lever retained via a (u8*)&D_800A1494 pun. tmp/grind/CD_sync/s107/mkprobe.py, mkprobe5.py.
- result: baseline candidate.c = 2/160/160 rules_dropped 0. p1 = 34 (159 insns), p2 = 32 (157), p3 = 34 (159), p4 = 18 (157), p5 = 2 (160). Every prong-conformant spelling is 9x-17x worse than the floor. volatile is strictly WORSE than plain (34/32 vs 34/18) because it forces a fresh lui/%lo per access and destroys the single-base addressing the merge was meant to create. p4 (18) merely re-lands in the honest-respelling basin s105 measured at 15. p5 holds at 2 only because it keeps the owner-refused cross-symbol delta behind a per-use pointer pun, which prong (d) forbids in terms and which retires nothing.
- verdict: KILLED

## [s107] Ruling D's MANDATORY first step - the prong-(c) asm-consumer check over asm/funcs/{CD_cw,func_800817A0,func_800819C4,func_80081E1C,getintr}.s and asm/data/7D920.data.s - can be satisfied for the Intr object.
- mechanism: Prong (c) requires the merge be complete: every merged per-word symbol removed from C AND from the splat symbol config, leaving exactly one C handle per storage location. An asm-only consumer that names a per-word symbol forces that symbol to survive, making any aggregate a second handle (the exact prong the g_stage_id merge died on, decisions.md:10722).
- probe: Grepped the six named files plus all of asm/ for g_cd_status_[abc] and D_800A149[45678]; inspected asm/data/7D920.data.s:31048-31076 for the definitions.
- result: The storage is DEFINED IN ASSEMBLY: dlabel D_800A1494 (.byte 0x00), dlabel D_800A1495 (.byte 0x00), dlabel D_800A1496 (.byte 0x00 x2), and dlabel D_800A1498 whose first word is literally '.word D_800A1494'. Eight asm files reference the names directly: CD_cw.s (6 sites), getintr.s (5), func_800819C4.s (5 incl. D_800A1498), func_800817A0.s = CD_flush (4), func_80081E1C.s (1), plus the INCLUDE_ASM bodies CD_sync.s / CD_ready.s / CD_datasync.s. The per-word symbols cannot be removed from the splat config, so exactly-one-handle is unreachable. In C the bytes additionally carry a second name family (g_cd_status_a/b/c, named_syms.txt:68-70) used by two matched in-TU consumers at src/system.c:416-419 and :493-496,:621.
- verdict: KILLED

## [s107] Endgame-lock gate (a): CD_sync qualifies for the canonical-asm grant path (STRONG scan_hand_coded tier).
- mechanism: STRONG hand-coded signals (S1 multu pacing / S2 empty branch / S6 BIOS jumptable) indicate the original body was hand-written assembly rather than compiler output.
- probe: python3 tools/scan_hand_coded.py --single CD_sync (artifact tmp/grind/CD_sync/s107/scan_hand_coded.txt).
- result: HAND_CODED: tier=LOW score=2/8 (CD_sync, 160 insns) - no strong hand-coded indicators. Only S4 (4 loads in an 8-insn window @ insn 49) and S5 (1 approx-sibling CD_ready, jaccard 0.64) fire; S1, S2 and S6 are all negative. Third independent reproduction (s104, s105/s106, s107). Independently barred by the 2026-07-09 Judge constraint on this ledger.
- verdict: KILLED

## [s107] Endgame-lock gate (b): an in-hand SOTN-master precedent exists for the construct that actually closes the 2-insn residual.
- mechanism: A citable SOTN-master exhibit (file+line) of the closing construct would place it inside an owner-sanctioned family; 'same spirit' does not qualify and a negative census is a failed gate, not an open question.
- probe: The closing construct is the cross-symbol address-difference idiom (s32)&D_800A1494 - (s32)D_800A125C carried in candidate.c as a FAKE combine-foldable chain-extender. sotn-construct-index censuses for cross-symbol / symbol-difference / (s32)&D_xxxx shapes: zero hits in s104 and again in s106 (tmp/grind/CD_sync/s106/gate_b_sotn_census.txt), reproducing the s98 first-hand SOTN / Vagrant Story / ESA survey negative.
- result: NEGATIVE, and the owner REFUSED this exact family on 2026-07-20 (standing Judge constraint on this ledger, parallel to the motion_SetMotion CLOBBER refusal). The aggregate-merge family DOES carry SOTN precedent (include/game.h Vram merges, PRs #1175 / bd612229 / 88344c03) but it is not the closing construct and cannot become one here - see the two KILLED hypotheses above.
- verdict: KILLED

## [s108] The g3 (order-perfect) basin's 6-insn a0/v1 exchange can be won by a C form that gives the arg5-value qty a strictly higher local-alloc priority than the t0-shift qty while the post-sched1 emission order is held at the target order.
- mechanism: local-alloc.c::qty_compare_1 ranks block-local qtys by `floor_log2(refs)*refs*size/(death-birth)*10000` and breaks ties with `*q1 - *q2` (qty index = birth order). In the g3 basin both contested qtys measure refs=2, size=1, death-birth=6 -> pri 3333 exactly, so the index tie-break hands $v1 to the t0-shift qty (born one slot earlier) and $a0 to the arg5 value � the inverse of the target. Any strict inequality (arg5-value life <= 4, or t0-shift life >= 8, or refs=3 on the arg5 value) reverses the allocation order and, by the free-register walk, yields arg5-value=$v1 / t0-shift=$a0 = target.
- probe: four window respellings on the g3 base, each measured with `sandbox CD_sync --disable all` and with the block=3 `BB2_QTY_DEBUG` table read: p1 arg5-subseq-first (masked 6, tie 3333/3333), p3 arg5 address staged through the ix carrier (masked 6, tie unchanged), p4 fresh single-set local for the t0 address (masked 9), p5 t0 address as one single-set expression (masked 9). Artifacts tmp/grind/CD_sync/s108/{g3b.stderr,g3_cdsync_qty.txt,ndiff.py}.
- result: KILLED for every spelling reachable by re-ordering or re-carrying the four window statements. The qty life-lengths are read off the POST-sched1 insn stream, and in the g3 basin that stream is byte-order-identical to the target, so both lives are pinned at 6 by the target's own instruction spacing: shortening the arg5 value's life or lengthening the t0 shift's life requires moving an insn inside a 20-insn straight-line block, which by construction leaves the g3 order. p4/p5 confirm the cost of touching the multi-set t0 carrier: the whole window re-schedules (+7).
- verdict: KILLED
- kill_scope: instance
- measured_on: h5/g3 chassis (160/160, rules_dropped 0), FAKE chain-extender on idx_1495 and the pp pointer-alias both present; four g3-base window respellings.

## [s108] The h5 post-sched1 order (which measurably produces the TARGET registers) can be combined with the target emission order by letting sched2 swap the residual pair back after reload � decoupling the order local-alloc sees from the order that reaches the bytes.
- mechanism: local-alloc runs between sched1 and sched2, so the qty lives that decide the a0/v1 exchange are read off the sched1 output, while the emitted bytes come from the sched2 output. In h5 the sched1 stream gives qty(t0-shift) birth=16 death=24 -> pri 2500 < qty(arg5-value) 3333, which is exactly the ranking that produces the target registers; only the pair order is wrong. If sched2's own `rank_for_schedule` could invert {111,121} post-reload, both residuals would close at once.
- probe: `BB2_SCHED_DEBUG` + `BB2_RANK_DEBUG` capture of CD_sync pass=2, block=3, on the h5 chassis (tmp/grind/CD_sync/s108/base.stderr, `SCHEDDBG FUNC func=CD_sync pass=2` at line 6396; 20-insn straight-line block).
- result: KILLED. At sched2 `reload_completed == 1`, so `birthing_insn_p` returns 0 and no block=3 insn carries a sentinel � every insn sits at pri 1-4 and the pair ties at pri=2. The class test also ties (`RANKDBG last=123 y=121 cls=3 x=111 cls2=3 val=0`), so `rank_for_schedule` terminates on `INSN_LUID(tmp) - INSN_LUID(tmp2)`, and the sched2 LUIDs ARE the sched1 output order. sched2 reproduced the h5 order exactly (picks 123@12, 121@13, 111@14). On this chassis sched2 is a fixpoint of sched1 for this window, so the decoupling is unavailable.
- verdict: KILLED
- kill_scope: instance
- measured_on: h5 chassis (candidate.c, 160/160, masked 2), FAKE chain-extender + pp alias present; sched2 block=3 telemetry.

## [s108] The 0x7F000001 priority both residual insns carry is a per-block INHERITED sentinel, not an intrinsic property of birthing insns � so no C form can make one of the pair outrank the other on the priority axis.
- mechanism: sched.c:4049 sets the currently-scheduled insn's INSN_PRIORITY to LAUNCH_PRIORITY (0x7F000001) immediately before `schedule_insn`; sched.c:2619 computes `max_priority = MAX (INSN_PRIORITY (ready[0]), INSN_PRIORITY (insn))`, and `adjust_priority` (sched.c:2586-2590) raises EVERY birthing insn to that value. TAIL_PRIORITY (0x7ffffffe) is only held by the block's last insn, which is scheduled first and immediately drops to DONE_PRIORITY, so it can never leak into max_priority. The `n_deaths` switch that would demote a priority is dead code (REG_DEAD notes are stripped before sched; every ADJPRI line in the capture reads `deaths=0`).
- probe: full block=3 `BB2_SCHED_DEBUG`/`BB2_RANK_DEBUG` capture, sched1 clocks 1-20 (tmp/grind/CD_sync/s108/cdsync_p1.txt).
- result: CONFIRMED as a structural reading of the pass. Both 111 and 121 carry maxpri=2130706433 at their ADJPRI events, and the clock=13 comparison falls through priority and class to LUID. This closes the priority axis of `rank_for_schedule` as a target for C-source work on this pair and re-derives s15's class result on the current chassis.
- verdict: CONFIRMED

## [s108] The g3 (order-perfect) basin's 6-insn a0/v1 exchange can be won by a C form that gives the arg5-value qty a strictly higher local-alloc priority than the t0-shift qty while the post-sched1 emission order is held at the target order, via re-ordering or re-carrying the four debug-window statements.
- mechanism: local-alloc.c::qty_compare_1 ranks block-local qtys by floor_log2(refs)*refs*size/(death-birth)*10000 and breaks ties with `*q1 - *q2` (qty index = birth order). Measured in the g3 basin: qty(t0-shift, reg1=108) birth=18 death=24 refs=2 -> pri 3333, and qty(arg5-value, reg1=100) birth=20 death=26 refs=2 -> pri 3333. Exact tie; the index tie-break hands $v1 to the t0 shift and $a0 to the arg5 value, the inverse of the target. In the h5 basin the same table reads qty(t0-shift, reg1=107) birth=16 death=24 -> pri 2500 < 3333, no tie, and the target registers fall out.
- probe: Four window respellings on the g3 base, each measured with `sandbox CD_sync --disable all` and with the block=3 BB2_QTY_DEBUG/BB2_SUGG_DEBUG table read from the instrumented cc1: p1 arg5-subseq-first (6), p3 arg5 address staged through the ix carrier (6), p4 fresh single-set local for the t0 address (9), p5 t0 address as one single-set expression (9). Per-index diffs via engine.score.normalized_insns.
- result: KILLED for all four spellings. The qty life-lengths are read off the POST-sched1 insn stream, and in the g3 basin that stream is byte-order-identical to the target, so both lives are pinned at 6 by the target's own instruction spacing: shortening the arg5 value's life or lengthening the t0 shift's life means moving an insn inside a 20-insn straight-line block, which leaves the g3 order. p4/p5 show the price of touching the multi-set t0 carrier: the whole window re-schedules (+7, 15 raw insn diffs).
- verdict: KILLED
- kill_scope: instance
- measured_on: h5/g3 chassis, 160/160 build_insns, rules_dropped 0; FAKE chain-extender on idx_1495 and the pp pointer-alias both present; four g3-base window respellings

## [s108] The h5 post-sched1 order, which measurably produces the target registers, can be combined with the target emission order by letting sched2 swap the residual pair back after reload, decoupling the order local-alloc sees from the order that reaches the bytes.
- mechanism: local-alloc runs between sched1 and sched2, so the qty lives deciding the a0/v1 exchange come from the sched1 output while the emitted bytes come from the sched2 output. In h5 the sched1 stream already ranks qty(arg5-value) above qty(t0-shift) (3333 vs 2500) - the ranking that yields the target registers - and only the pair order is wrong.
- probe: BB2_SCHED_DEBUG + BB2_RANK_DEBUG capture of CD_sync pass=2 block=3 on the h5 chassis (tmp/grind/CD_sync/s108/base.stderr, `SCHEDDBG FUNC func=CD_sync pass=2`, 20-insn straight-line block).
- result: KILLED. At sched2 reload_completed==1, so birthing_insn_p returns 0 and no block=3 insn carries a sentinel; every insn sits at pri 1-4 and the pair ties at pri=2. The class test also ties (RANKDBG last=123 y=121 cls=3 x=111 cls2=3 val=0), so rank_for_schedule terminates on INSN_LUID(tmp)-INSN_LUID(tmp2) - and the sched2 LUIDs ARE the sched1 output order. sched2 reproduced the h5 order exactly (picks 123@clock12, 121@13, 111@14).
- verdict: KILLED
- kill_scope: instance
- measured_on: h5 chassis (memory/grind/CD_sync/candidate.c, 160/160, masked 2), FAKE chain-extender + pp alias present; sched2 block=3 telemetry

## [s108] The 0x7F000001 priority both residual insns carry at sched1 is a per-block inherited sentinel rather than an intrinsic property of birthing insns, so the priority arm of rank_for_schedule can never separate insn 111 from insn 121.
- mechanism: sched.c:4049 sets the currently-scheduled insn's INSN_PRIORITY to LAUNCH_PRIORITY immediately before schedule_insn; sched.c:2619 computes max_priority = MAX(INSN_PRIORITY(ready[0]), INSN_PRIORITY(insn)); adjust_priority (sched.c:2586-2590) raises every birthing insn to that value. TAIL_PRIORITY is held only by the block's last insn, which is scheduled first and drops to DONE_PRIORITY, so it cannot leak into max_priority. The n_deaths demotion switch is dead code (REG_DEAD notes stripped before sched; every ADJPRI line reads deaths=0).
- probe: Full block=3 sched1 telemetry, clocks 1-20, on the h5 chassis (tmp/grind/CD_sync/s108/cdsync_p1.txt).
- result: CONFIRMED. Both 111 and 121 carry maxpri=2130706433 at their ADJPRI events; the clock=13 comparison falls through priority and class (val=0 on all 51 block=3 comparisons) to LUID. Re-derives s15's class result on the current chassis and closes the priority axis as a C-source target.
- verdict: CONFIRMED

## [s109] The g3 (order-perfect) basin's a0/v1 exchange can be won by raising reg_n_refs on the arg5-VALUE pseudo to 3 or 4 - lifting its qty_compare_1 priority from 3333 to 5000/13333 - through a C-level spelling that adds a mention the compiler folds away before emission.
- mechanism: local-alloc.c::qty_compare_1 ranks block qtys by `floor_log2(qty_n_refs)*qty_n_refs*qty_size/(qty_death-qty_birth)*10000`, and `qty_n_refs[qty] = reg_n_refs[regno]` (local-alloc.c:297). `reg_n_refs` is BUILT BY flow.c (allocated/zeroed at flow.c:1281-1282, incremented at flow.c:2081/2329/2515/2725) inside `flow_analysis`, which toplev.c calls at line 2983 - BEFORE combine (toplev.c:3004), before sched1 (3033) and before local_alloc (3049). A mention that survives to flow_analysis and is folded by combine therefore still counts, so refs was the one qty_compare_1 input not pinned by the target's (order-perfect) instruction spacing. In the g3 basin qty(t0-shift, reg108) births 18 dies 24 refs 2 and qty(arg5-value, reg100) births 20 dies 26 refs 2 - pri 3333 each - and the `*q1 - *q2` index tiebreak allocates the t0 shift first, giving it $v1 and leaving $a0 for arg5, the inverse of the target. Any refs>=3 on reg100 reverses the allocation order, and the FFR free-register walk then hands reg100 $v1 and reg108 $a0 (both `used` masks read off the s108/s109 SUGGDBG-FFR lines confirm this).
- probe: SIX byte-level measurements on the g3 base (memory/grind/CD_sync/rejected/s108_g3_arg5_first_regexchange.c, control re-measured 6/160 this session), each scored with `sandbox CD_sync --disable all` AND read back through the instrumented cc1's block=3 `BB2_QTY_DEBUG`/`BB2_SUGG_DEBUG` table (tmp/grind/CD_sync/s109/*.stderr):
  q1 the `ix` index local folded into `arg5` (arg5 carries index, scaled index and loaded value; `ix` declaration dropped) - score 9;
  q2 only the SHIFT result carried by arg5 (`arg5 = ix << 2; arg5 = *(s32 *)(arg5 + (s32)tbl_125c);`) - score 8;
  q3 index+shift+address+value all carried by arg5 (three sets, no `ix` at all) - score 9;
  q4 address-only carry (`arg5 = ix + (s32)tbl_125c; arg5 = *(s32 *)arg5;`) - score 9;
  q5 same-value re-store `arg5 = arg5;` under the sanctioned dead-store family - score 6 (inert);
  q6 copy-tie into a fresh `arg5b` consumed by the call, aimed at local-alloc.c:1932 `qty_n_refs[sqty] += reg_n_refs[sreg]` - score 6 (inert).
  All six at build_insns 160, rules_dropped 0.
- result: KILLED for all six spellings; zero probes at or below the g3 control of 6, and NOT ONE of them moved qty_n_refs off 2. Two distinct compiler mechanisms are measured to intercept the lever, each named by an insn-count ladder over the -da dumps (CD_sync segment only: rtl / jump / cse / loop / cse2 / flow / combine / lreg / greg):
  (1) VARIABLE REUSE NEVER REACHES reg_n_refs. q2's flow-dump register profile is byte-for-byte the same multiset as the base's (identical pseudo numbers, identical mention counts), and its block=3 qty table still shows every qty at refs=2. The C front end gives each assigned VALUE its own fresh pseudo and copies it into the DECL pseudo; cse_main deletes those copies, so by flow_analysis the reused C variable has no pseudo of its own left to carry the accumulated references. Merging C locals therefore reshapes birth/death and the qty COUNT (q1 drops block-3 from four local qtys to three; the arg5-value qty leaves block-local allocation entirely) but never the refs term - which is why all four merge spellings regress instead of winning.
  (2) THE EXTRA MENTION IS DELETED BEFORE flow_analysis COUNTS IT. Insn ladder: base rtl=100 jump=98 cse=92 loop=92 cse2=92 flow=91 combine=70. q5 (`arg5 = arg5;`) is rtl=100 - IDENTICAL to the base - so the same-value re-store never reaches RTL at all; expand drops the no-op assignment, and the sanctioned dead-store family cannot add a reference at this site. q6 (the copy) is rtl=101 jump=99 but cse=92: `cse_main` (toplev.c:2861) copy-propagates and deletes the move ~120 insns before flow.c starts counting, so neither the extra mention (flow) nor the qty tie (local-alloc.c:1932 `combine_regs`) ever happens; q6's block=3 table is identical to the base's.
  Combined with s105 (VALUE-side and idx_1494-delta combine-foldable chain-extenders materialize at build_insns 164/166 and score 11-30; only ADDRESS-side detours stay byte-neutral, at 20/25) the refs axis now has no measured byte-neutral carrier at this site: every spelling either materialises bytes or is deleted before flow.
- verdict: KILLED
- kill_scope: instance
- measured_on: g3 order-perfect base (s108_g3_arg5_first_regexchange.c, control 6/160, rules_dropped 0), FAKE chain-extender on idx_1495 and the pp pointer-alias both present; six g3-base spellings, each cross-read against the block=3 BB2_QTY_DEBUG table.

## [s109] MANDATED KILL RE-AUDIT: the s108 instance kills were measured with FAKE carriers occupying the contested pseudos, so the closest banked form must be re-measured on the current chassis with every FAKE construct ablated before any new probe is spent.
- mechanism: `.claude/rules`-level audit requirement (func_8002EA24 s8 post-mortem) - a lever recorded inert while a FAKE carrier sits on its target pseudo is not a kill.
- probe: `python3 tools/fake_ablate.py --func CD_sync --file system --candidate memory/grind/CD_sync/candidate.c` - the full 2^2 grid over the two FAKE units in the banked floor form (L19 idx_1495 combine-foldable chain-extender, L45 pp pointer-alias).
- result: keep-all 2 / build_insns 160 (the ledger floor REPRODUCED on the current chassis); drop the chain-extender only 15 / 159; drop the pp alias only 17 / 161; drop both 30 / 160. Both FAKE units are load-bearing and neither masks the other: removing either one costs 13-15 points, and their removal is super-additive (30 > 2+13+15 would be 28 if independent). The chassis is unchanged since s108 and the s108 kills stand as measured. The pp alias is also the only construct that changes build_insns in either direction (161 without it), i.e. it is doing real emission work, not just biasing allocation.
- verdict: CONFIRMED (floor 2 re-measured; both FAKE units load-bearing; s108 kills not carrier-masked)

## [s109] In the g3 order-perfect basin the a0/v1 exchange can be won by a C spelling that raises reg_n_refs on the arg5-VALUE pseudo to 3 or 4, lifting its qty_compare_1 priority off the 3333 tie with the t0-shift qty, via C-level variable reuse, a same-value re-store, or a copy-tie.
- mechanism: local-alloc.c::qty_compare_1 ranks block qtys by floor_log2(qty_n_refs)*qty_n_refs*qty_size/(qty_death-qty_birth)*10000 and breaks ties on the qty index (birth order); qty_n_refs[qty] = reg_n_refs[regno] (local-alloc.c:297). reg_n_refs is allocated/zeroed at flow.c:1281-1282 and incremented at flow.c:2081/2329/2515/2725 inside flow_analysis, which toplev.c calls at line 2983 - before combine (3004), sched1 (3033) and local_alloc (3049). A mention folded by combine therefore still counts, making refs the one qty_compare_1 input not pinned by the target's instruction spacing. Two post-flow adjusters exist: optimize_reg_copy_1 transfers refs across a copy (local-alloc.c:781-783) and combine_regs sums them when tying qtys (local-alloc.c:1932).
- probe: Six sandbox measurements on the g3 order-perfect base (rejected/s108_g3_arg5_first_regexchange.c; control re-measured 6/160/rules_dropped 0), each cross-read against the instrumented cc1's block=3 BB2_QTY_DEBUG/BB2_SUGG_DEBUG table and the -da insn-count ladder: q1 ix folded into arg5 (9), q2 shift carried by arg5 (8), q3 index+shift+address+value all in arg5 (9), q4 address carried by arg5 (9), q5 same-value re-store arg5=arg5 under the sanctioned dead-store family (6, inert), q6 copy-tie into a fresh arg5b consumed by the call, aimed at local-alloc.c:1932 (6, inert). All six at build_insns 160.
- result: KILLED for all six; zero probes at or below the g3 control of 6, and not one moved qty_n_refs off 2. Two interceptors measured. (1) Variable reuse never reaches reg_n_refs: q2's flow-dump register profile is the same multiset as the base's (identical pseudo numbers and mention counts) and its block=3 qty table still reads refs=2 everywhere - the front end gives each assigned VALUE a fresh pseudo and copies it into the DECL pseudo, cse deletes the copies, so a four-times-assigned C local contributes four independent refs-2 pseudos at flow time rather than one refs-8 pseudo; the merge spellings regress only by reshaping birth/death and the qty count (q1 collapses block 3 from four local qtys to three and evicts the arg5-value qty from block-local allocation). (2) The extra mention dies before flow counts it: insn ladder base rtl=100 jump=98 cse=92 loop=92 cse2=92 flow=91 combine=70; q5 is rtl=100 - identical to base - so expand drops the no-op assignment and it never reaches RTL; q6 is rtl=101 jump=99 cse=92, i.e. cse_main (toplev.c:2861) copy-propagates the move away ~120 insns before flow.c starts counting, so neither the flow mention nor the combine_regs qty tie ever happens. Combined with s105 (value-side and idx_1494-delta chain-extenders materialize at 164/166 insns scoring 11-30; only address-side detours stay byte-neutral, at 20/25), every known refs spelling at this site either materialises bytes or is deleted before flow.
- verdict: KILLED
- kill_scope: instance
- measured_on: g3 order-perfect base (memory/grind/CD_sync/rejected/s108_g3_arg5_first_regexchange.c), control 6/160, build_insns 160, rules_dropped 0; FAKE chain-extender on idx_1495 and the pp pointer-alias both present; six g3-base spellings each cross-read against the block=3 BB2_QTY_DEBUG table

## [s109] MANDATED KILL RE-AUDIT: the banked floor form's two FAKE constructs may be masking the levers the s108 kills were measured against, so candidate.c must be re-scored on the current chassis across the full ablation grid.
- mechanism: func_8002EA24 s8 post-mortem - a lever recorded inert while a FAKE carrier occupies its target pseudo is not a kill; tools/fake_ablate.py scores the candidate over every subset of its FAKE units.
- probe: python3 tools/fake_ablate.py --func CD_sync --file system --candidate memory/grind/CD_sync/candidate.c (2 FAKE units: L19 idx_1495 combine-foldable chain-extender, L45 pp pointer-alias).
- result: keep-all 2 / build_insns 160 - the ledger floor REPRODUCED on the current chassis. drop chain-extender only 15/159; drop pp alias only 17/161; drop both 30/160. Both units are load-bearing and super-additive, and neither masks the other, so the s108 instance kills stand as measured. The pp alias is additionally the only construct that changes build_insns (161 without it), i.e. it is doing emission work rather than pure allocation bias.
- verdict: CONFIRMED

## [s110] REDERIVE: the honest-chassis residual (masked 15, the score the function actually has to close, because the h5 cross-symbol chain-extender is an owner-REFUSED fakematch) is NOT the block-3 a0/v1 pair the last 109 sessions ground on — it is a global.c allocno-priority PERMUTATION of the four callee-saved seats {s3,s4,s5,s6}, plus the already-known 2-insn sll/addu pair.
- mechanism: global.c's allocno_compare ranks global allocnos by `floor_log2(allocno_n_refs)*allocno_n_refs*allocno_size/allocno_live_length*10000` and find_reg then walks the free registers in ascending order, so the DESCENDING priority order of the five long-lived allocnos literally spells the s2/s3/s4/s5/s6 assignment. The instrumented cc1's ALLOCDBG hook (tools/gcc-2.7.2/global.c:605) prints ord/pseudo/hardreg/nrefs/livelen/pri for every allocno.
- probe: ALLOCDBG capture (BB2_ALLOC_DEBUG=1, tools/gcc-2.7.2/cc1 via tmp/grind/CD_sync/s110/cap.py) on BOTH chassis, plus a full objdump-vs-target instruction walk (tmp/grind/CD_sync/s110/odiff.py) of the honest form.
  h5 (masked 2):     p77 idx_1494 nrefs=7 len=150 pri=933 -> $s2 | p79 tbl_125c nrefs=5 len=148 pri=675 -> $s3 | p78 idx_1495 nrefs=2 len=72 pri=277 -> $s4 | p72 mode nrefs=2 len=76 pri=263 -> $s5 | p73 result nrefs=2 len=79 pri=253 -> $s6.  This IS the target seating.
  honest (masked 15, `idx_1495 = idx_1494 + 1;`): p77 933 -> $s2 | p72 mode 263 -> $s3 | p73 result 253 -> $s4 | p79 tbl_125c nrefs=3 len=148 pri=202 -> $s5 | p78 idx_1495 nrefs=2 len=144 pri=138 -> $s6.
  The objdump walk shows the honest form is byte-structurally IDENTICAL to the target at all 160 slots except (a) the {s3,s4,s5,s6} renaming and (b) the known slot-54/55 swap (`sll a0,a0,2` before `addu v0,v0,s3` instead of after).
- result: CONFIRMED. The honest chassis needs exactly `933 > pri(tbl_125c) > pri(idx_1495) > 263`. mode/result are pinned at 263/253 (nrefs 2 is the floor for a param: one incoming-copy set + one use, and this function has no loop-depth weighting - see the next hypothesis), so with the TARGET'S OWN ref counts (tbl 3, idx_1495 2) the requirement reduces to two live-length inequalities: **live_length(tbl_125c) < 114 and live_length(idx_1495) < 76**, at build_insns 160. Currently 148 and 144. This is a materially WEAKER target than the one s102 measured against: s102 aimed at pri>675 (livelen<44) and its six structural probes were all judged against that bar. The h5 chain-extender wins the seating by two independent means at once - it lifts tbl_125c from 3 refs to 5 (crossing the floor_log2 2^2 boundary, 202 -> 675) AND halves idx_1495's live length (144 -> 72, 138 -> 277) - which is why every single-axis honest respelling of it has measured 15.
- verdict: CONFIRMED

## [s110] The three fn-scope pointer locals (idx_1494, idx_1495, tbl_125c) can be dropped from the C and re-created by loop.c as loop-invariant hoists into the function prologue, giving them the short (hoist-local) live lengths the target's seating requires.
- mechanism: the target's prologue materializes &D_800A1494, &D_800A1494+1 and D_800A125C before the loop; if those were loop.c LICM hoists rather than source locals, their allocnos would be born in the preheader and counted once instead of accumulating across the re-propagated loop blocks in flow.c's life_analysis.
- probe: three splices on the honest base, each ALLOCDBG-captured and sandbox-scored - v1 drop idx_1495 only (`(idx_1494)[1]` at the callback site), v2 drop idx_1495 + idx_1494 (direct `D_800A1494` symbol refs), v3 drop idx_1495 + tbl_125c (direct `D_800A125C` refs). Artifacts tmp/grind/CD_sync/s110/v{1,2,3}_*.{c,stderr}.
- result: KILLED for all three. v1 masked=18 build_insns=157 (-3), v2 masked=34 build=159 (-1), v3 masked=38 build=156 (-4). The dropped pointer is never re-materialized in the prologue: this function's outer loop is a `goto` loop with no NOTE_INSN_LOOP_BEG, so loop.c does not recognise it as a natural loop and performs NO invariant hoisting at all. Every dropped local costs its prologue lui/addiu pair outright. Rejected forms banked as rejected/s110_no_idx1495_local_licm_absent.c, s110_no_intr_ptr_locals_3insns_lost.c, s110_no_tbl_local_4insns_lost.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: honest chassis (rejected/s110_honest_idx1495_sreg_permutation_15.c, control 15/160, rules_dropped 0); pp pointer-alias FAKE present, h5 chain-extender ABSENT

## [s110] Writing the outer poll as a REAL loop (`while (1) { ... }`) turns on flow.c's loop_depth weighting of reg_n_refs and loop.c's invariant hoisting, so the pointer allocnos gain refs and the params stay flat - reproducing the target's priority ordering on an honest chassis.
- mechanism: flow.c accumulates `reg_n_refs[regno] += loop_depth` (flow.c:2081), and loop_depth is only >1 inside a NOTE_INSN_LOOP_BEG..LOOP_END region. In the goto chassis loop_depth is 1 everywhere, so every allocno's nrefs is its raw mention count. Under a real loop, in-loop mentions count double and loop.c additionally hoists the invariant address computations into a preheader with short live ranges.
- probe: two splices, both ALLOCDBG-captured and sandbox-scored - v4 real `while (1)` + no pointer locals at all (pure LICM reliance), v5 real `while (1)` + the honest chassis's three fn-scope pointer locals kept. Artifacts tmp/grind/CD_sync/s110/v{4,5}_*.{c,stderr}.
- result: KILLED for both. v4 masked=41 build_insns=165 (+5); v5 masked=37 build_insns=164 (+4). The loop_depth weighting is real and measured - under v5 idx_1494 goes nrefs 7 -> 12 (pri 933 -> 2337) and mode/result go nrefs 2 -> 3 (pri 263/253 -> 379/370) - but it moves the PARAMS UP as fast as it moves the pointers, so the seating does not converge, and loop.c's hoists cost 4-5 extra instructions plus two allocnos that fail to get a hard register at all (v5 ord=17/18 hardreg=-1). This closes the "the original was a structured while loop" provenance reading: the target's own ref counts (idx_1494 7, tbl_125c 3, idx_1495 2, mode 2, result 2, read straight off asm/funcs/CD_sync.s) are the UNWEIGHTED counts, i.e. the original compile also saw loop_depth 1 for this body. Rejected forms banked as rejected/s110_real_loop_licm_hoist_plus5.c and s110_real_loop_keep_ptrs_plus4.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: honest chassis (control 15/160) and the v2/v3 symbol-ref derivative; pp pointer-alias FAKE present, h5 chain-extender ABSENT; build_insns 165/164 vs target 160

## [s110] The honest chassis (idx_1495 = idx_1494 + 1, no cross-symbol chain-extender) differs from the target only by a permutation of the four callee-saved seats {s3,s4,s5,s6} plus the already-known slot-54/55 sll/addu swap, and that permutation is decided entirely by global.c's allocno priority ordering.
- mechanism: global.c allocno_compare ranks global allocnos by floor_log2(nrefs)*nrefs*size/live_length*10000 and find_reg walks the free callee-saved registers in ascending order, so the descending priority order of the five long-lived allocnos literally spells the s2/s3/s4/s5/s6 assignment. Read off the instrumented cc1's ALLOCDBG hook at tools/gcc-2.7.2/global.c:605.
- probe: BB2_ALLOC_DEBUG=1 capture on both chassis via tmp/grind/CD_sync/s110/cap.py, plus a slot-by-slot objdump-vs-asm/funcs/CD_sync.s walk (tmp/grind/CD_sync/s110/odiff.py) of the honest form (masked 15, build_insns 160, rules_dropped 0).
- result: CONFIRMED. h5 (masked 2) seats idx_1494 pri=933->$s2, tbl_125c nrefs=5 len=148 pri=675->$s3, idx_1495 nrefs=2 len=72 pri=277->$s4, mode 263->$s5, result 253->$s6 - the target seating. The honest form seats idx_1494 933->$s2, mode 263->$s3, result 253->$s4, tbl_125c nrefs=3 len=148 pri=202->$s5, idx_1495 nrefs=2 len=144 pri=138->$s6. The instruction walk shows no other structural difference across all 160 slots. Closing condition: 933 > pri(tbl_125c) > pri(idx_1495) > 263; with the target's own ref counts (3 and 2, read directly off the target asm) that reduces to live_length(tbl_125c) < 114 and live_length(idx_1495) < 76 at build_insns 160 (present: 148 and 144). This is materially weaker than the pri>675 / livelen<44 bar s102 measured its six structural probes against. It also explains the flat 15: the h5 chain-extender is a DOUBLE lever, lifting tbl_125c across the floor_log2 4-ref boundary AND halving idx_1495's live length, so no single-axis honest respelling can reproduce it.
- verdict: CONFIRMED

## [s110] Dropping the fn-scope pointer locals idx_1495, idx_1494 and tbl_125c lets loop.c re-create them as loop-invariant hoists in the function prologue with the short live lengths the target seating needs.
- mechanism: the target's prologue materializes &D_800A1494, &D_800A1494+1 and D_800A125C before the loop; as loop.c preheader hoists their allocnos would be born once instead of accumulating across flow.c's re-propagated loop blocks.
- probe: three splices on the honest base, each ALLOCDBG-captured and sandbox-scored: v1 drop idx_1495 only, v2 drop idx_1495 + idx_1494 (direct D_800A1494 symbol refs), v3 drop idx_1495 + tbl_125c (direct D_800A125C refs). tmp/grind/CD_sync/s110/v{1,2,3}_*.{c,stderr}.
- result: KILLED for all three: v1 masked=18 build_insns=157, v2 masked=34 build=159, v3 masked=38 build=156. No dropped pointer is re-materialized - this function's outer poll is a goto loop with no NOTE_INSN_LOOP_BEG, so loop.c does not see a natural loop and performs no invariant hoisting; each dropped local simply costs its prologue lui/addiu pair. Banked as rejected/s110_no_idx1495_local_licm_absent.c, s110_no_intr_ptr_locals_3insns_lost.c, s110_no_tbl_local_4insns_lost.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: honest chassis (memory/grind/CD_sync/rejected/s110_honest_idx1495_sreg_permutation_15.c, control 15/160, rules_dropped 0); pp pointer-alias FAKE present, h5 chain-extender absent

## [s110] Writing the outer poll as a real while (1) loop turns on flow.c's loop_depth weighting of reg_n_refs and loop.c's invariant hoisting, reproducing the target's allocno priority ordering on an honest chassis.
- mechanism: flow.c:2081 accumulates reg_n_refs[regno] += loop_depth, and loop_depth exceeds 1 only inside a NOTE_INSN_LOOP_BEG..LOOP_END region, so in-loop mentions would count double and loop.c would additionally hoist the invariant address computations into a preheader.
- probe: two splices, ALLOCDBG-captured and sandbox-scored: v4 real while (1) with no pointer locals (pure LICM reliance), v5 real while (1) keeping the three fn-scope pointer locals. tmp/grind/CD_sync/s110/v{4,5}_*.{c,stderr}.
- result: KILLED for both: v4 masked=41 build_insns=165, v5 masked=37 build_insns=164. The weighting is real and measured (v5: idx_1494 nrefs 7->12, pri 933->2337; mode/result nrefs 2->3, pri 263/253->379/370) but it lifts the parameters as fast as the pointers, and loop.c's hoists cost 4-5 extra instructions with two allocnos left unallocated (hardreg=-1). Since the target's own ref counts read off asm/funcs/CD_sync.s ($s3 3x, $s4 2x, $s2 7x) are the UNWEIGHTED counts, the original compile also saw loop_depth 1: the goto chassis is provenance-correct and the structured-while reading of the PsyQ bios.c v1.86 source is closed. Banked as rejected/s110_real_loop_licm_hoist_plus5.c and s110_real_loop_keep_ptrs_plus4.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: honest chassis (control 15/160) and its symbol-ref derivative; pp pointer-alias FAKE present, h5 chain-extender absent; build_insns 165/164 vs target 160

## s111 (rederive)

- **H111-1 KILLED (instance).** "Giving tbl_125c or idx_1495 a second C-level
  assignment makes reg_n_sets != 1, denying the REG_EQUIV note and the
  local-alloc.c:1064 live-length doubling." Six spellings measured on the honest
  base (control 15/160): split-init compound assign (15/160), duplicate
  same-value set x3 (18/160), dead different-value first store x3 (15/160).
  ALLOCDBG live lengths unchanged at 148/144 in every case. cse_main +
  delete_dead_from_cse (toplev.c:2867) run before the reg_scan at toplev.c:2925.
  Measured on: honest chassis, no FAKE constructs present.
- **H111-2 KILLED (instance).** "Moving the pointer initialisations out of the
  prologue shortens their raw live length into the frontier's acceptance band and
  seats them above the parameters at build_insns 160." The band IS reachable
  (r1 = 921/800/750/263/253, the first honest form with all three pointers above
  both parameters) but every placement move costs instructions: 159 / 158 / 157 /
  157 / 156 against the target's 160. Measured on: honest chassis, no FAKE
  constructs present.
- **CONFIRMED.** The honest chassis's ~13-point residual is entirely
  `reg_live_length[regno] *= 2` at local-alloc.c:1064 applied to the three
  constant-address pointer pseudos and not to the two parameter pseudos.
  Un-doubled, the honest priorities would be 1866 / 405 / 277 / 263 / 253 - the
  exact target seating.

## [s111] Giving tbl_125c or idx_1495 a second C-level assignment (ordinary split-init compound assign, duplicate same-value set, or a dead different-value first store) makes reg_n_sets != 1 at local_alloc, denying the REG_EQUIV note and the live-length doubling.
- mechanism: update_equiv_regs skips the note when reg_n_sets[regno] != 1 (local-alloc.c:1021) and only doubles reg_live_length for pseudos that got a note (local-alloc.c:1064); reg_n_sets is built by the last reg_scan, at toplev.c:2925.
- probe: Six spellings spliced onto the honest base (control h0 = 15/160): t1 'idx_1495 = idx_1494; idx_1495 += 1;'; t2/t3/t4 duplicate same-value sets of tbl_125c and/or idx_1495; q1/q2/q3 different-value dead first stores. Each captured with the instrumented cc1 under BB2_ALLOC_DEBUG and scored with sandbox CD_sync --disable all.
- result: t1 15/160 with live lengths 148/144 unchanged; t2/t3/t4 18/160 with 152/144 (the shift is the set moving one insn earlier, not a note change); q1/q2/q3 15/160 with the ALLOCDBG table bit-identical to h0. The extra set is removed by cse_main and by delete_dead_from_cse (toplev.c:2867), both of which precede the reg_scan at toplev.c:2925, so reg_n_sets is still 1 when update_equiv_regs runs.
- verdict: KILLED
- kill_scope: instance
- measured_on: honest chassis (tmp/grind/CD_sync/s111/h0.c, control 15/160, build_insns 160, rules_dropped 0), no FAKE constructs present

## [s111] Moving the pointer initialisations out of the prologue shortens their raw live length into the acceptance band the s110 frontier derived, seating both pointers above the parameters while build_insns stays at 160.
- mechanism: reg_live_length is accumulated over the insns a pseudo is live across (flow.c); a constant-address pointer set at the top of the do_timeout window or just before the poll is live over far fewer insns than one set in the prologue, and global.c allocno priority is floor_log2(nrefs)*nrefs*size*10000/live_length.
- probe: p1 (idx_1495 set moved to just before 'new_var = 0xFF'), p3 (tbl_125c set moved into the do_timeout window), p2 (both), r2 (tbl_125c set moved to the loop top), r1 (r2 + p1); ALLOCDBG ord=11..15 read for each variant, then scored.
- result: The band is reachable - r1 gives ord=11..15 = 921/800/750/263/253, the first honest form on record with all three pointers above both parameters - but every placement move costs instructions: p1 159, p3 158, p2 157, r2 157, r1 156, against the target's 160. Once the set leaves the prologue the constant is rematerialized into the addressing (lbu $a0,1($s2) instead of addiu $s4,$s2,1 plus lbu $a0,0($s4)), deleting the prologue lui/addiu pairs the target carries at asm/funcs/CD_sync.s:15-19. Scores 15/22/22/20/20.
- verdict: KILLED
- kill_scope: instance
- measured_on: honest chassis (control h0 15/160, build_insns 160, rules_dropped 0), no FAKE constructs present

## s112 (structural)

## [s112] The CONSTANT_P REG_EQUAL note that drives the local-alloc.c:1064 live-length doubling is attached by cse_insn at cse.c:6923, so a spelling that places the `idx_1495 = idx_1494 + 1;` initialisation at the head of a cse extended-block boundary - where p77's constant value is not in cse's table - denies the note and leaves idx_1495's live length un-doubled at build_insns 160.
- mechanism: cse_insn attaches the note under `n_sets == 1 && src_const && GET_CODE (dest) == REG && GET_CODE (src_const) != REG` (cse.c:6923-6934); src_const is taken from the value table or from CONSTANT_P (src_folded) (cse.c:6484-6507). update_equiv_regs then converts a CONSTANT_P REG_EQUAL into REG_EQUIV (local-alloc.c:1029) and doubles reg_live_length for every noted pseudo (local-alloc.c:1064). `(plus p77 1)` only folds to `(const (plus (symbol_ref "D_800A1494") 1))` when cse still knows p77's constant, i.e. inside the same extended basic block as p77's set. At a label with a second predecessor cse's table has been reset, the fold does not happen, src_const is 0 and no note is written.
- probe: four placements of the idx_1495 initialisation, each scored with `sandbox CD_sync --disable all` and cross-read against the instrumented cc1's ALLOCDBG allocno table plus a grep of the CD_sync segment of the .lreg dump for REG_EQUIV notes mentioning D_800A1494 / D_800A125C (tmp/grind/CD_sync/s112/notes.sh). Control h0 = 15 / build_insns 160 / rules_dropped 0 with p78 nrefs=2 livelen=144 pri=138 seated at $s6.
  n1 loop top          - note DENIED, p78 livelen 55 pri 363 -> $s3, score 15, build_insns 159
  n3 success block     - note DENIED, p78 livelen 75 pri 266 -> $s3, score 15, build_insns 160
  n4 poll-loop top     - p78 folded into the addressing entirely (allocno gone), score 18, build_insns 157
  n2 loop top + tbl    - see the next hypothesis, score 20, build_insns 157
- result: CONFIRMED for n3. Placing `idx_1495 = idx_1494 + 1;` immediately after `success: v0 = 0;` (which dominates the pointer's only use, the `*idx_1495` callback argument in the poll block, because the do_timeout path always returns -1) denies the note with NO instruction-count cost: build_insns stays 160, rules_dropped 0, and p78 goes from pri 138 / $s6 to pri 266 / $s3. This is the first HONEST, zero-FAKE reproduction of one of the two levers the owner-refused h5 chain-extender was buying: the banked floor-2 form's ALLOCDBG (re-measured this session: 2 / 160 / 0) shows p78 at livelen 72 pri 277 WITH ITS NOTE ALSO ABSENT, i.e. h5's cross-symbol extender was denying exactly this note. n1 does the same thing one instruction cheaper in live length but loses an instruction, so n3 is the form to inherit.
- verdict: CONFIRMED

## [s112] tbl_125c's REG_EQUIV note can be denied, or its doubled live length shortened into the acceptance band, by relocating its initialisation the way n3 relocates idx_1495's.
- mechanism: same gate. But tbl_125c's initialiser is a BARE array-decay `D_800A125C`, so SET_SRC is itself a symbol_ref: cse.c:6497-6505 sets src_const from CONSTANT_P (src_folded) without consulting the value table, and the note is therefore attached at every placement, independent of basic-block structure. Relocation can only shorten reg_live_length, and global.c's find_reg walks the free hard registers in ascending order, so a short-lived tbl allocno lands in $s0 rather than the target's $s3.
- probe: n2 (tbl at the outer loop top, on n1), m1 (tbl at the loop top, on the n3 base), m3 (tbl immediately before the timeout branch, on n3), m2 (tbl at the top of the do_timeout block, on n3). Each ALLOCDBG-captured and .lreg-grepped.
- result: KILLED for all four. The REG_EQUIV (symbol_ref "D_800A125C") note is present in every single variant - relocation never denies it. n2 score 20 build_insns 157 (p79 livelen 44 pri 681 -> hardreg 16 = $s0); m1 score 20 build 157 (livelen 40 pri 750 -> $s0); m3 score 20 build 157 (livelen 40 pri 750 -> $s0); m2 score 22 build 158 (p79 drops out of the global allocno list entirely - it becomes block-local once its set sits in the same block as both its uses). The band IS reachable by shortening (681/750 sit inside the required (266, 933)) but a short-lived allocno cannot be seated at $s3, and every relocation costs 2-3 instructions against the target's 160.
- verdict: KILLED
- kill_scope: instance
- measured_on: honest chassis, n1 and n3 bases (controls 15/159 and 15/160, rules_dropped 0), no FAKE constructs present except the pp pointer-alias inherited from h0

## [s112] tbl_125c's reg_n_refs can be lifted from 3 to 4 - crossing the floor_log2 boundary that puts its doubled-live-length priority at 540, inside the (266, 933) band the n3 base now requires - by an extra mention that flow.c counts and combine.c folds away.
- mechanism: flow_analysis builds reg_n_refs at toplev.c:2983 and combine_instructions runs at toplev.c:3004, so a mention that survives cse1/loop/cse2 to flow and is then folded by combine is byte-neutral yet still counted. The two candidate carriers are (a) a reg-reg handle copy placed at a cse extended-block boundary so cse_main cannot copy-propagate it away, and (b) a re-association that forces the load address to be built in two plus insns that both mention the base.
- probe: t1 - a fn-scope `u8 *tb8;` assigned `tb8 = (u8 *)tbl_125c;` at the outer loop top (2-predecessor label) and consumed by the first table access; t2 - the second table access re-associated as `*(s32 *)((s32)tbl_125c + (ix + ((s32)tbl_125c - (s32)tbl_125c)))` (a mechanism probe only, never a candidate form). Both spliced onto the n3 base (control 15 / 160 / 0) and ALLOCDBG-captured.
- result: KILLED for both. t1 = 15 / build_insns 160, t2 = 15 / build_insns 160, and in BOTH the ALLOCDBG table still reads p79 nrefs=3 livelen=148 pri=202 -> $s6, bit-identical to the n3 control. The cross-block copy is still eliminated, and a copy REDISTRIBUTES references between two pseudos rather than adding one, so p79's own mention count is unchanged; the self-difference re-association is folded by cse before flow ever sees a second plus insn. A 4th mention of p79 therefore has to live in a real, non-redundant address-forming insn in the SAME basic block as the table reads - combine's LOG_LINKS are basic-block-local, so a cross-block extra mention cannot be folded by combine and would materialise bytes.
- verdict: KILLED
- kill_scope: instance
- measured_on: n3 honest base (memory/grind/CD_sync/rejected/s112_HONEST_BASE_1495_success_block_note_denied_15.c, control 15/160, rules_dropped 0), no FAKE constructs present except the inherited pp pointer-alias

## [s112] With idx_1495's note honestly denied by the n3 placement, the whole honest residual reduces to ONE numeric condition on a single allocno.
- mechanism: global.c allocno_compare priority = floor_log2(nrefs)*nrefs*size*10000/live_length, and find_reg walks free hard registers in ascending order, so the descending priority order of the five long-lived allocnos spells the $s2..$s6 assignment directly.
- probe: ALLOCDBG on the n3 base versus the banked floor-2 form, both re-measured this session.
- result: CONFIRMED. n3 seats 933 (idx_1494, $s2) / 266 (idx_1495, $s3) / 263 (mode, $s4) / 253 (result, $s5) / 202 (tbl_125c, $s6). The target seating is idx_1494 $s2, tbl_125c $s3, idx_1495 $s4, mode $s5, result $s6. Since pri(idx_1495) must stay above the parameters' 263, pri(tbl_125c) must sit above pri(idx_1495) and below 933, and tbl_125c must remain long-lived (a short-lived tbl allocno is seated at $s0, measured four ways above), the ONLY remaining gate is: pri(tbl_125c) in (266, 933) with its initialisation still in the prologue and build_insns 160. With the note in force and live_length doubled to 148 that means nrefs >= 4 (nrefs 4 -> 540, nrefs 5 -> 675); with the note denied and the raw live length of 74, nrefs 3 already suffices (405). Everything else about the honest form is now measured correct.
- verdict: CONFIRMED

## [s112] The CONSTANT_P REG_EQUAL note that drives the local-alloc.c:1064 live-length doubling is written by cse_insn at cse.c:6923, so placing the idx_1495 = idx_1494 + 1 initialisation at the head of a cse extended-block boundary - where p77's constant is no longer in cse's value table - denies the note and leaves idx_1495's live length un-doubled at build_insns 160.
- mechanism: cse_insn writes the note under 'n_sets == 1 && src_const && GET_CODE (dest) == REG && GET_CODE (src_const) != REG' (cse.c:6923-6934), with src_const filled from the hash-table equivalence class or from CONSTANT_P (src_folded) at cse.c:6484-6507. update_equiv_regs converts a CONSTANT_P REG_EQUAL to REG_EQUIV (local-alloc.c:1029) and doubles reg_live_length for every noted pseudo (local-alloc.c:1064). (plus p77 1) only folds to a constant while p77's value is still in cse's table, i.e. inside the same extended basic block as p77's set; at a label with a second predecessor the table has been reset, src_const is 0, and no note is written. Confirmed by the insn ladder: insns 24/27/30 carry no notes in .rtl and .jump and carry REG_EQUAL from .cse onward.
- probe: Four placements of the idx_1495 init on the honest base (control h0 = 15 / build_insns 160 / rules_dropped 0, p78 nrefs=2 livelen=144 pri=138 at $s6), each scored with sandbox CD_sync --disable all, cross-read against the instrumented cc1's ALLOCDBG allocno table and a grep of the CD_sync segment of the .lreg dump for REG_EQUIV notes on D_800A1494 / D_800A125C: n1 outer-loop top, n3 success block, n4 poll-loop top, n2 loop top with tbl moved too.
- result: CONFIRMED for n3. Placing the init immediately after 'success: v0 = 0;' - which dominates the pointer's only use, the *idx_1495 callback argument in the poll block, because the do_timeout path always returns -1 - denies the note at zero instruction cost: score 15, build_insns 160, rules_dropped 0, p78 pri 138 -> 266 and seat $s6 -> $s3, with the target's prologue addiu $s4,$s2,1 still emitted. n1 (loop top) also denies the note (livelen 55, pri 363, $s3) but loses an instruction (159). n4 folds the pointer into the addressing entirely (18 / 157). This reproduces, in ordinary C with no FAKE construct, one of the two levers the owner-refused h5 cross-symbol chain-extender was buying: the banked floor-2 form's ALLOCDBG, re-read this session, is 933 / 675 (p79 nrefs=5) / 277 (p78 livelen 72, note ABSENT) / 263 / 253.
- verdict: CONFIRMED

## [s112] tbl_125c's REG_EQUIV note can be denied, or its doubled live length shortened into the acceptance band while keeping its $s3 seat, by relocating its initialisation the way n3 relocates idx_1495's.
- mechanism: Same gate, but tbl_125c's initialiser is a bare array-decay D_800A125C, so SET_SRC is itself a symbol_ref and cse.c:6497-6505 sets src_const from CONSTANT_P (src_folded) without consulting the value table - the note attaches at every placement, independent of basic-block structure. Relocation can then only shorten reg_live_length, and global.c's find_reg walks the free hard registers in ascending order, so a short-lived allocno lands in $s0.
- probe: n2 (tbl at the outer loop top, on the n1 base), m1 (tbl at the loop top, on the n3 base), m3 (tbl immediately before the timeout branch, on n3), m2 (tbl at the top of the do_timeout block, on n3) - each sandbox-scored, ALLOCDBG-captured and .lreg-grepped for the note.
- result: KILLED for all four placements. The REG_EQUIV (symbol_ref "D_800A125C") note is present in every variant - relocation never denies it. n2 20 / 157 (p79 livelen 44 pri 681 -> hardreg 16 = $s0); m1 20 / 157 (livelen 40 pri 750 -> $s0); m3 20 / 157 (identical); m2 22 / 158 (p79 leaves the global allocno list entirely and becomes block-local once its set sits in the same block as both its uses). The priority band is reachable by shortening (681 and 750 both sit inside the required (266,933)) but a short-lived tbl allocno cannot be seated at $s3, and every relocation costs 2-3 instructions against the target's 160.
- verdict: KILLED
- kill_scope: instance
- measured_on: honest chassis, n1 and n3 bases (controls 15/159 and 15/160, rules_dropped 0); no FAKE constructs present except the pp pointer-alias inherited from the h0 base

## [s112] tbl_125c's reg_n_refs can be lifted from 3 to 4 - crossing the floor_log2 boundary that puts its doubled-live-length priority at 540, inside the (266,933) band the n3 base requires - by a cross-cse-block reg-reg handle copy or by a re-association that builds the load address in two plus insns.
- mechanism: flow_analysis builds reg_n_refs at toplev.c:2983 and combine_instructions runs at toplev.c:3004, so a mention that survives cse1/loop/cse2 to flow and is folded by combine is byte-neutral yet still counted. The block-boundary placement that defeats cse's constant fold was expected to also defeat cse's copy propagation.
- probe: t1 - a fn-scope 'u8 *tb8;' assigned 'tb8 = (u8 *)tbl_125c;' at the outer loop top (a 2-predecessor label) and consumed by the first table access; t2 - the second table access re-associated so the base is mentioned twice (a mechanism probe only, never proposed as a candidate form). Both spliced onto the n3 base (control 15 / 160 / 0) and ALLOCDBG-captured.
- result: KILLED for both: t1 = 15 / build_insns 160 and t2 = 15 / build_insns 160, with p79's ALLOCDBG row (nrefs=3 livelen=148 pri=202 -> $s6) bit-identical to the n3 control in each case. A copy redistributes references between two pseudos rather than adding one, so p79's own mention count is unchanged even when the copy survives cse; the re-association folds at cse before flow ever sees a second plus insn. Since combine's LOG_LINKS are basic-block-local, a 4th mention of p79 that combine can fold has to live in the same basic block as the two table reads (the do_timeout window) and be a real, non-redundant address-forming insn.
- verdict: KILLED
- kill_scope: instance
- measured_on: n3 honest base (memory/grind/CD_sync/rejected/s112_HONEST_BASE_1495_success_block_note_denied_15.c, control 15/160, rules_dropped 0); no FAKE constructs present except the inherited pp pointer-alias

## [s112] With idx_1495's note honestly denied by the n3 placement, the honest residual reduces to a single numeric condition on one allocno.
- mechanism: global.c allocno_compare ranks global allocnos by floor_log2(nrefs)*nrefs*size*10000/live_length and find_reg walks free hard registers in ascending order, so the descending priority order of the five long-lived allocnos spells the $s2..$s6 assignment directly (ALLOCDBG hook, tools/gcc-2.7.2/global.c:605).
- probe: ALLOCDBG capture on the n3 base and on the banked floor-2 form, both re-measured this session, plus the four tbl relocation variants.
- result: CONFIRMED. n3 seats 933 (idx_1494, $s2) / 266 (idx_1495, $s3) / 263 (mode, $s4) / 253 (result, $s5) / 202 (tbl_125c, $s6); the target seating is idx_1494 $s2, tbl_125c $s3, idx_1495 $s4, mode $s5, result $s6. pri(idx_1495) must stay above the parameters' 263, pri(tbl_125c) must sit above pri(idx_1495) and below 933, and tbl_125c must stay long-lived (a short-lived tbl allocno is seated at $s0, measured four ways). The single remaining gate is therefore pri(tbl_125c) in (266,933) with the initialisation still in the prologue at build_insns 160: nrefs >= 4 with the note in force (4 -> 540, 5 -> 675), or nrefs 3 with the note denied (raw live length 74 -> 405).
- verdict: CONFIRMED

## s113 (structural)

## [s113] tbl_125c's reg_n_refs can be lifted from 3 to 4 by forming one of the two table addresses off a displaced base (`tp = tbl_125c + 1; tp[i - 1]`), whose displacement combine absorbs into the load's 16-bit offset, leaving the target's two `addu $x,$x,$s3` bytes.
- mechanism: reg_n_refs is built by flow.c at toplev.c:2983 and combine runs at toplev.c:3004, so a mention that survives cse into flow and is then folded by combine is byte-neutral yet counted. The construct sits inside the do_timeout block, whose head is a multi-predecessor label, so cse's value table has been reset there and cannot fold `(plus p79 4)` to a constant address — the same extended-block-boundary mechanism n3 uses to deny idx_1495's note. nrefs 4 with the doubled live length 148 gives 2*4*10000/148 = 540, inside the (266, 933) band.
- probe: five spellings spliced onto the n3 honest base (control 15 / build_insns 160 / rules_dropped 0, p79 nrefs=3 livelen=148 pri=202 at $s6), each captured with the instrumented cc1 under BB2_ALLOC_DEBUG: a1 second access via a `tbl_125c + 1` word-displaced base; a2 the same on the first access; a3 a byte-displaced base with the index kept pre-scaled; a4 a negative displacement; a5 both accesses off one displaced base. Artifacts tmp/grind/CD_sync/s113/a{1..5}_*.{c,stderr,lreg}.
- result: KILLED for all five. p79 stays at nrefs=3 in a1 (150/200), a2 (148/202, ALLOCDBG bit-identical to the control), a3 (150/200) and a4 (150/200), and DROPS to nrefs=2 (152/131) in a5. Every C-level address chain mentions the base pseudo exactly once, so a displaced handle takes over that mention instead of adding one — the same redistribution s112 measured for reg-reg copies, now shown for the whole extra-address-forming-insn family. a1/a3/a4 additionally materialise the displacement (one extra emitted insn), so they are not even byte-neutral.
- verdict: KILLED
- kill_scope: instance
- measured_on: n3 honest base (memory/grind/CD_sync/rejected/s112_HONEST_BASE_1495_success_block_note_denied_15.c, control 15/160, rules_dropped 0); no FAKE constructs present except the pp pointer-alias inherited from h0

## [s113] The seat can be reached from the other side — leaving tbl_125c at pri 202 and pushing the two parameters below idx_1495's noted priority of 138 on the h0 base, giving the target order 933 > 202 > 138 > pri(mode) > pri(result).
- mechanism: global.c's allocno_compare ranks by floor_log2(nrefs)*nrefs*size*10000/live_length, so the seating is an ORDER condition, not an absolute one; a parameter whose live length roughly doubles falls below a noted pointer's priority. The doubling itself is local-alloc.c:1064, applied to any pseudo carrying a REG_EQUIV.
- probe: the .lreg register report for the CD_sync segment on the n3 base plus the ALLOCDBG tables of all eleven variants compiled this session, read against the parameter-note condition in assign_parms (tools/gcc-2.7.2/function.c:3826-3830).
- result: KILLED. The function's honest live lengths top out at ~79 insns (p72 mode 76, p73 result 79, p77/p78 raw 75, p79 raw 74); everything above that in the tables is the local-alloc doubling. Pushing a 2-ref parameter below 138 needs a live length above 145 — nearly twice the honest maximum — and the only mechanism that reaches those values is the REG_EQUIV doubling, which assign_parms attaches to a parameter pseudo only when `GET_CODE (entry_parm) == MEM && entry_parm == stack_parm` (function.c:3826-3830), i.e. only for a parameter that ARRIVES in memory. CD_sync's parameters arrive in $a0/$a1. Across all eleven variants p72/p73 never left the 246-266 range and never took a note.
- verdict: KILLED
- kill_scope: instance
- measured_on: n3 and h0 honest bases (controls 15/160, rules_dropped 0) plus the eleven s113 variants; no FAKE constructs present except the inherited pp pointer-alias

## [s113] A second, genuinely LIVE set of tbl_125c (one reachable through the loop's back edge, so neither set is dead) makes reg_n_sets 2, denying the REG_EQUIV note and the live-length doubling, and lands p79 in the (266, 933) band at build_insns 160.
- mechanism: update_equiv_regs bails at local-alloc.c:1021 when `reg_n_sets[regno] != 1`, before it can convert a CONSTANT_P REG_EQUAL into REG_EQUIV and double reg_live_length at local-alloc.c:1064. s111's six multi-set spellings all failed because each extra set was DEAD and was removed by cse_main / delete_dead_from_cse (toplev.c:2867) ahead of the reg_scan at toplev.c:2925; a set placed after the uses inside the loop is reached by the back edge and cannot be deleted.
- probe: four placements of a second `tbl_125c = D_800A125C;` on the n3 base, each ALLOCDBG-captured and (c1, d1) sandbox-scored: c1 at the end of the do_timeout block after both uses; d1 on the loop tail immediately before the back edge; d2 in the success block; d3 at the head of the inner poll loop. Plus c2, the same construct applied to idx_1494 as a control. Artifacts tmp/grind/CD_sync/s113/{c1,c2,d1,d2,d3}_*.{c,stderr,lreg}.
- result: The MECHANISM is CONFIRMED — this is the first measured C-level denial of the doubling for a bare-symbol pointer: c1 takes p79 from nrefs=3 livelen=148 pri=202 to nrefs=4 livelen=71 (raw, undoubled) pri=1126. But the hypothesis as stated is KILLED on both of its numeric claims. (i) The band is overshot, not hit: the second set is itself a fourth mention, so denial and refs-lift arrive together — c1 1126, d1 2580 (livelen 31), d2 1081, d3 1403, all outranking p77's 921 and seating p79 at $s0/$s1 instead of $s3. (ii) Every placement materialises a second lui/addiu address pair: build_insns 162 against the target's 160, sandbox distance 21 for both c1 and d1. c2 (the same construct on idx_1494) removes p77 from the global allocno table and leaves p79 untouched at 3/150/200. The residual arithmetic: with the note denied, nrefs 4 needs live length > 87 to fall under p77, so the only in-band shape is "note denied AND exactly three mentions" — a second set whose emitted insn already exists in the target, and the target writes $s3 exactly once (asm/funcs/CD_sync.s:15-16).
- verdict: KILLED
- kill_scope: instance
- measured_on: n3 honest base (control 15/160, rules_dropped 0), sandbox re-scored this session at 21/162 for c1 and d1; no FAKE constructs present except the inherited pp pointer-alias

## [s113] tbl_125c's reg_n_refs can be lifted from 3 to 4 by forming one of the two table addresses off a displaced base (tp = tbl_125c + 1; tp[i - 1]), whose displacement combine absorbs into the load's 16-bit offset, leaving the target's two addu $x,$x,$s3 bytes.
- mechanism: reg_n_refs is built by flow.c at toplev.c:2983 and combine runs at toplev.c:3004, so a mention that survives cse into flow and is then folded by combine is byte-neutral yet counted. The construct sits inside the do_timeout block whose head is a multi-predecessor label, so cse's value table has been reset there and cannot fold (plus p79 4) to a constant address — the same extended-block-boundary mechanism n3 uses on idx_1495. nrefs 4 with the doubled live length 148 gives 2*4*10000/148 = 540, inside the required (266,933) band.
- probe: Five spellings spliced onto the n3 honest base (control 15 / build_insns 160 / rules_dropped 0; p79 nrefs=3 livelen=148 pri=202 at $s6), each compiled with the instrumented cc1 under BB2_ALLOC_DEBUG: a1 second access via a word-displaced base, a2 the same on the first access, a3 a byte-displaced base with a pre-scaled index, a4 a negative displacement, a5 both accesses off one displaced base.
- result: KILLED for all five. p79 stays at nrefs=3 in a1 (livelen 150 pri 200), a2 (148/202, ALLOCDBG bit-identical to the control), a3 (150/200) and a4 (150/200), and drops to nrefs=2 (152/131) in a5. Every C-level address chain mentions the base pseudo exactly once, so a displaced handle takes over that mention rather than adding one — s112's reg-reg-copy redistribution finding now measured across the whole extra-address-forming-insn family. a1/a3/a4 also materialise the displacement instead of folding it.
- verdict: KILLED
- kill_scope: instance
- measured_on: n3 honest base (memory/grind/CD_sync/rejected/s112_HONEST_BASE_1495_success_block_note_denied_15.c), control 15/160, rules_dropped 0; no FAKE constructs except the pp pointer-alias inherited from h0

## [s113] The seat can be reached from the other side: leave tbl_125c at pri 202 and push the two parameters below idx_1495's noted priority of 138 on the h0 base, giving the target order 933 > 202 > 138 > pri(mode) > pri(result).
- mechanism: global.c allocno_compare ranks by floor_log2(nrefs)*nrefs*size*10000/live_length, so the seating is an ORDER condition, not an absolute one; a parameter whose live length doubles falls below a noted pointer. The doubling is local-alloc.c:1064, applied to any pseudo carrying a REG_EQUIV.
- probe: The .lreg register report for the CD_sync segment on the n3 base, plus the ALLOCDBG tables of all eleven variants compiled this session, read against the parameter-note condition in assign_parms (tools/gcc-2.7.2/function.c:3826-3830).
- result: KILLED. Honest live lengths top out at ~79 insns (mode 76, result 79, idx_1494/idx_1495 raw 75, tbl_125c raw 74); every larger number in the tables is the doubling. Pushing a 2-ref parameter below 138 needs a live length above 145, nearly twice the honest maximum, and the only mechanism reaching those values is the REG_EQUIV doubling — which assign_parms attaches to a parameter pseudo only under GET_CODE (entry_parm) == MEM && entry_parm == stack_parm (function.c:3826-3830), i.e. only for a parameter that ARRIVES in memory. CD_sync's parameters arrive in $a0/$a1, and across all eleven variants p72/p73 never left 246-266 and never took a note.
- verdict: KILLED
- kill_scope: instance
- measured_on: n3 and h0 honest bases (controls 15/160, rules_dropped 0) plus the eleven s113 variants; no FAKE constructs except the inherited pp pointer-alias

## [s113] A second, genuinely LIVE set of tbl_125c (one reached through the loop's back edge, so neither set is dead) makes reg_n_sets 2, denying the REG_EQUIV note and the live-length doubling, and lands p79 inside the (266,933) band at build_insns 160.
- mechanism: update_equiv_regs bails at local-alloc.c:1021 when reg_n_sets[regno] != 1, before it can convert a CONSTANT_P REG_EQUAL into REG_EQUIV and double reg_live_length at local-alloc.c:1064. s111's six multi-set spellings failed because every extra set was DEAD and was removed by cse_main / delete_dead_from_cse (toplev.c:2867) ahead of the reg_scan at toplev.c:2925; a set placed after the uses inside the loop is reached by the back edge and cannot be deleted.
- probe: Four placements of a second `tbl_125c = D_800A125C;` on the n3 base, ALLOCDBG-captured and (c1, d1) sandbox-scored: c1 end of the do_timeout block after both uses, d1 loop tail before the back edge, d2 success block, d3 head of the inner poll loop; plus c2, the same construct on idx_1494 as a control.
- result: The MECHANISM is confirmed — the first measured C-level denial of the doubling for a bare-symbol pointer: c1 moves p79 from nrefs=3 livelen=148 pri=202 to nrefs=4 livelen=71 (raw, undoubled) pri=1126. Both numeric claims are KILLED. (i) The band is overshot, not hit: the second set is itself a fourth mention, so denial and refs-lift arrive together — c1 1126, d1 2580 (livelen 31), d2 1081, d3 1403, all outranking p77's 921 and seating p79 at $s0/$s1 instead of $s3. (ii) Every placement materialises a second lui/addiu pair: build_insns 162 vs the target's 160, sandbox distance 21 for both c1 and d1. c2 removes p77 from the global allocno table and leaves p79 at 3/150/200. Residual arithmetic: with the note denied, nrefs 4 needs live length > 87 to fall under p77, so the only in-band shape is note-denied-with-exactly-three-mentions.
- verdict: KILLED
- kill_scope: instance
- measured_on: n3 honest base (control 15/160, rules_dropped 0); c1 and d1 sandbox-re-scored this session at 21/162; no FAKE constructs except the inherited pp pointer-alias

## [s114] Mandated kill re-audit: candidate.c is still the floor and both FAKE units are still load-bearing
- mechanism: instance kills are chassis-relative; s111's ablation table had to be re-confirmed on HEAD.
- probe: splice memory/grind/CD_sync/candidate.c, `sandbox CD_sync --disable all`, then `tools/fake_ablate.py --func CD_sync --file system --candidate memory/grind/CD_sync/candidate.c`.
- result: CONFIRMED. 2/160/0 keep-all; 15/159 drop chain-extender; 17/161 drop pp alias; 30/160 drop both — identical to s111. n3 honest base re-measured 15/160/0 with the s112/s113 ALLOCDBG rows bit-identical.

## [s114] The original Sony source spells both table reads as ordinary array indexing off the named global `D_800A125C[]` with no pointer local, so removing the `tbl_125c` local should reproduce the target's $s3 base
- mechanism: `tmp/sotn/src/main/psxsdk/libcd/bios.c:232` is a matched-project body for the same PsyQ libcd `CD_sync`; its printf reads `D_80032B48[Intr.sync]` / `D_80032B48[Intr.ready]` off a `char *[8]` global. BB2 links the same prebuilt BIOS.OBJ (memory/closer/libcd-groundtruth.md), so that shape is the original. cse would create the address pseudo and loop.c hoist it into the prologue as the target's `lui/addiu $s3`.
- probe: f1 (delete the local, spell `(u8 *)D_800A125C + t0` / `ix + (s32)D_800A125C`) and f2 (f1 + ordinary `D_800A125C[i]` indexing), both spliced on the n3 honest base (control 15/160/0), sandbox + ALLOCDBG + `.lreg` REG_EQUIV grep.
- result: KILLED (instance). f1 = 35 / build_insns 159, f2 = 38 / 159. In both, the tbl pseudo does not appear in the global allocno list at all and no callee-saved base register is formed — the address is rematerialised at each access. The explicit C-level pointer local is what creates the long-lived allocno the target seats at $s3. Banked rejected/s114_f1.c, rejected/s114_f2.c.

## [s114] Duplicating the WHOLE do_timeout region into both arrival arms (deleting the `do_timeout:` label that blocked s20's merge) lifts tbl_125c's reg_n_refs across the s113 gate at zero byte cost
- mechanism: sanctioned duplicated-statement-into-arms (.claude/rules/duplicated-statement-into-arms.md). flow.c books reg_n_refs before combine and long before jump2, so both copies' mentions are counted; jump2's `find_cross_jump` then re-merges the two identical arm bodies. s20 measured that an intervening label defeats the suffix match, so the label must go and the `goto check;` tail must live inside each arm.
- probe: g1 = n3 honest base with `tslTm2LoadImage_2` + the two `idx_1494[]` reads + the pp binding + both tbl_125c address computations + `debug_printf` + `cdrom_ClearIrq()` + `v0 = -1; goto check;` duplicated into the `D_800F19B8 < v0` arm and the `0x3C0000 < cnt` arm. sandbox + ALLOCDBG.
- result: CONFIRMED. score 13 / build_insns 160 / rules_dropped 0 — cross-jump merged exactly (160 == target) and this is the first honest-chassis form to beat n3's 15. p79 tbl_125c 5/188/**531** seats at **$s3**, p78 idx_1495 at **$s4**, p72 mode at **$s5**, p73 result at **$s6** — four of five target s-register seats, and s113's gate `pri(p79) ∈ (266,933)` is cleared. Residual: the same duplication lifts p77 idx_1494's loop-weighted refs 14 -> 27 (pri 933 -> 1421), putting it above p80's 952 so $s1/$s2 invert. Banked rejected/s114_HONEST_BASE_dup_arms_13.c — the next session's chassis, NOT a dead form.

## [s114] p77's inflated weighted-ref count on the duplicated-arms chassis comes from the loop-depth weighting of the `idx_1495 = idx_1494 + 1;` mention, so hoisting that init back to the prologue restores pri(p77) < 952
- mechanism: flow.c books reg_n_refs weighted by loop_depth; the success-block init sits inside the outer loop, the prologue does not.
- probe: g2 = g1 with the idx_1495 init moved from the success block back to the prologue. sandbox + ALLOCDBG + `.lreg` note grep.
- result: KILLED (instance). 20 / 160 / 0. p77's row is **bit-identical** (9 / 190 / 1421) — the init's placement contributes nothing to p77's weighting — and p78 re-acquires its `REG_EQUIV (const (plus (symbol_ref "D_800A1494") 1))`, collapsing to pri 108 and the $s6 seat. The success-block placement remains load-bearing for the note denial (s112). Banked rejected/s114_g2.c.

## [s114] Mandated kill re-audit: candidate.c is still the recorded floor on the current chassis and both of its FAKE units are still load-bearing and super-additive.
- mechanism: Instance kills and ablation tables are chassis-relative; the dispatch brief again reported the HEAD floor as unavailable, so the closest-to-target banked form had to be re-measured before any new probe.
- probe: Spliced memory/grind/CD_sync/candidate.c into src/system.c, ran sandbox CD_sync --disable all, then tools/fake_ablate.py --func CD_sync --file system --candidate memory/grind/CD_sync/candidate.c. Also re-measured the n3 honest base.
- result: 2/160/rules_dropped 0 keep-all; 15/159 drop chain-extender; 17/161 drop pp alias; 30/160 drop both - identical to s111. n3 honest base 15/160/0 with ALLOCDBG ord=10..15 bit-identical to s112/s113 (952/933/266/263/253/202).
- verdict: CONFIRMED

## [s114] Spelling both table reads as ordinary array indexing off the named global D_800A125C with no tbl_125c pointer local - the shape the matched upstream PsyQ libcd body in tmp/sotn/src/main/psxsdk/libcd/bios.c:232 uses - reproduces the target's prologue lui/addiu $s3 base register.
- mechanism: BB2 links Sony's prebuilt BIOS.OBJ verbatim (memory/closer/libcd-groundtruth.md), so SOTN's matched bios.c CD_sync body is the original source shape; D_800A125C is its char pointer table D_80032B48[8] and both reads are D_80032B48[Intr.sync] / D_80032B48[Intr.ready]. cse would create the address pseudo and loop.c hoist it to the preheader as the target's lui/addiu $s3.
- probe: f1 (delete the local; spell (u8 *)D_800A125C + t0 and ix + (s32)D_800A125C) and f2 (f1 + ordinary D_800A125C[i] indexing), both spliced on the n3 honest base (control 15/160/0); sandbox --disable all + ALLOCDBG + .lreg REG_EQUIV grep.
- result: f1 = 35 / build_insns 159, f2 = 38 / 159. In both variants the tbl pseudo is absent from the global allocno list entirely and no callee-saved base register is formed - the address is rematerialised at each access. The explicit C-level pointer local is what creates the long-lived allocno the target seats at $s3. Banked memory/grind/CD_sync/rejected/s114_f1.c and s114_f2.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: n3 honest base (memory/grind/CD_sync/rejected/s112_HONEST_BASE_1495_success_block_note_denied_15.c) re-measured at control 15/160, rules_dropped 0; no FAKE constructs present except the inherited pp pointer alias

## [s114] Duplicating the whole do_timeout region into both do_timeout arrival arms, with the do_timeout label deleted so jump2 find_cross_jump can re-merge the copies, lifts tbl_125c's reg_n_refs across the s113 pri(p79) gate at zero byte cost.
- mechanism: Sanctioned duplicated-statement-into-arms (.claude/rules/duplicated-statement-into-arms.md): flow.c books reg_n_refs before combine and long before jump2, so both copies' mentions are counted by local-alloc and global-alloc, and jump2 find_cross_jump then collapses the two identical arm bodies back to one emitted sequence. s20 measured that an intervening label defeats the suffix match, so the label had to go and the goto-check tail had to live inside each arm.
- probe: g1 = n3 honest base with tslTm2LoadImage_2(&D_800161B8) + both idx_1494 byte reads + the pp binding + both tbl_125c address computations + debug_printf + cdrom_ClearIrq() + v0 = -1 + goto check, duplicated into the (D_800F19B8 < v0) arm and the (0x3C0000 < cnt) arm. sandbox --disable all + ALLOCDBG.
- result: score 13 / build_insns 160 / rules_dropped 0 - cross-jump merged exactly to the target's instruction count, and this is the first honest-chassis form to beat the n3 base's 15. ALLOCDBG: p79 tbl_125c 5 refs / livelen 188 / pri 531 seats at $s3 (TARGET), p78 idx_1495 2/95/210 at $s4 (TARGET), p72 mode 2/96/208 at $s5 (TARGET), p73 result 2/99/202 at $s6 (TARGET). s113's closing condition pri(p79) in (266,933) is cleared. Residual: the same duplication lifts p77 idx_1494's loop-weighted refs from 14 to 27 (pri 933 to 1421), above p80's 952, inverting $s1/$s2. Banked as memory/grind/CD_sync/rejected/s114_HONEST_BASE_dup_arms_13.c - the next session's chassis, not a dead form.
- verdict: CONFIRMED

## [s114] On the g1 duplicated-arms chassis, p77's inflated loop-weighted reg_n_refs comes from the loop-depth weighting of the idx_1495 = idx_1494 + 1 mention sitting in the success block, so hoisting that init back to the prologue drops pri(p77) below p80's 952.
- mechanism: flow.c books reg_n_refs weighted by loop_depth; the success block is inside the outer poll loop and the prologue is not, so the same mention is worth more in the success block.
- probe: g2 = g1 with idx_1495 = idx_1494 + 1 moved from the success block back to the prologue; sandbox --disable all + ALLOCDBG + .lreg REG_EQUIV grep.
- result: 20 / build_insns 160 / rules_dropped 0. p77's ALLOCDBG row is bit-identical to g1 (9 refs / livelen 190 / pri 1421) - the init's placement contributes nothing to p77's weighting - and p78 re-acquires its REG_EQUIV const-plus-symbol note, collapsing to pri 108 and the $s6 seat. Confirms s112: the success-block placement is load-bearing for the note denial. Banked memory/grind/CD_sync/rejected/s114_g2.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: g1 duplicated-arms honest base (memory/grind/CD_sync/rejected/s114_HONEST_BASE_dup_arms_13.c), control 13/160, rules_dropped 0; no FAKE constructs present except the inherited pp pointer alias and the arm duplication itself

## s115 — FRONTIER RESET (synthesis)

The seat equation that has driven this ledger since s110 is **CLOSED**. On the
duplicated-arms chassis, `rejected/s115_HONEST_BASE_dup_arms_idx1495_read_3.c` ("j1")
scores **3 / build_insns 160 / rules_dropped 0** with every callee-saved pseudo in its
target seat (p80 $s1 / p77 $s2 / p79 $s3 / p78 $s4 / p72 $s5 / p73 $s6), carrying ONE FAKE
construct (the `pp` pointer alias) and NOT the owner-refused cross-symbol chain-extender.
The residual is two independent defects, and the next ladder pass should work them
separately. Everything below is stated against j1, not against candidate.c or n3.

### F1 (highest value) — retire the 1-point seat trade
**Statement.** p78 (`idx_1495`) can be given the priority it needs in the band (208, 531)
without routing one of the do_timeout block's two index reads through it, so that both
reads keep the target's `lbu 0($s2)` / `lbu 1($s2)` form.
**Mechanism.** Two arithmetic openings exist, both read off
`tools/gcc-2.7.2/global.c:612-616` (`pri = floor_log2(nrefs)*nrefs/livelen*10000`) with p78
initialised in the prologue at livelen 184 (doubled) or 92 (undoubled):
  (a) **note ON, 4 refs** -> 8*10000/184 = 434. j1 buys this with the `*idx_1495` read (1
      instruction). Any OTHER pair of byte-neutral references to `idx_1495` does the same
      job for free. The un-probed carrier is the poll region: the `status & 4` callback arm
      is the only other place `idx_1495` is mentioned, and duplicated-statement-into-arms
      has now been measured byte-neutral TWICE on this function.
  (b) **note DENIED, 2 refs** -> 2*10000/92 = 217, which is in band (it must clear p72's
      208 and stay under p79's 531). This needs the prologue init to escape
      `update_equiv_regs`, whose only C-reachable gates are `single_set == 0`
      (local-alloc.c:979), `reg_n_sets != 1` (:1020) and "no CONSTANT_P REG_EQUAL"
      (:1030). s113 measured the second-live-set route at a fixed +2 instructions FOR
      tbl_125c; it has never been measured for idx_1495, whose init is an `addiu` off an
      already-live register rather than a `lui/addiu` pair — the +2 cost may not transfer.
**Next probe.** (b) first, it is one edit: on j1, restore `ix = idx_1494[1];` in both copies,
keep the prologue `idx_1495 = idx_1494 + 1;`, and add a second genuinely-live set of
`idx_1495` reached through the outer loop's back edge (the placement family s113 catalogued
as c1/d1/d2/d3 for tbl_125c). Accept only build_insns 160 with the `.lreg` note grep showing
NO `REG_EQUIV (const (plus (symbol_ref D_800A1494) 1))`; read the ALLOCDBG table before
spending a sandbox run and require pri(p78) in (208, 531) with p77 back at 9 refs — note
that p77 at 9 refs is 1421 and then ALSO needs the i1 split-init on `saved` (2 instructions),
so (b) only pays if the second set is free. If it is not, go to (a).

### F2 — the `sll a0,a0,0x2` / `addu v0,v0,s3` pair-swap, re-opened on the NEW chassis
**Statement.** The 2-point pair-swap at normalized indices 54/55 is a property of the
duplicated-arms chassis's schedule, not the theorem-locked h5-basin residual that s6-s93
ground for sixty sessions.
**Mechanism.** Every prior forensic result on this pair (s6 qty_compare_1, s25 SCHEDDBG
block=3, s43 LUID renumbering, s51 LUID 8-vs-12, s70 ready-list dispatch, s93 POLL-region
structural axis) was measured on the h5 or n3 chassis, where the do_timeout region is a
SINGLE block reached by two branches. On j1 the region exists as two copies through
`flow`/`sched1` and is re-merged only by `jump2 find_cross_jump`, so the block boundaries,
the LUID numbering and the ready-list contents at the decisive clock are all different
objects. The banked "mechanism-locked" verdicts are chassis-relative and do not transfer.
**Next probe.** Run `tools/sched_solver` (order- and clock-exact, both passes) on j1's
`.sched`/`.sched2` dumps for the block containing indices 50-60, and classify the 54/55
inversion with `tools/ra_solver/inverse_compose.py classify` before writing any C. k1/k2
(source-order permutation of the two address chains and of the two byte reads) are already
dead at 7/160, so the lever must come from the solver's ranked vectors, not from statement
shuffling.

### F3 — re-audit candidate.c against j1
**Statement.** The floor-2 form's two FAKE units (cross-symbol chain-extender + pp alias)
are buying seat effects that j1 now obtains structurally, so at least one of them is
redundant on the j1 chassis and the composite may reach 0-1.
**Mechanism.** candidate.c's chain-extender exists to deny p78's REG_EQUIV note (s105/s111);
j1 reaches p78's correct seat WITH the note, via 4 references. The two levers therefore
target the same pseudo by different routes and have never been measured together.
**Next probe.** Splice j1 + candidate.c's `idx_1495` chain-extender line (measurement only —
the cross-symbol idiom is owner-REFUSED for submission, 2026-07-20, so a 0 here is a
diagnostic, not a candidate), and separately j1 + the i1 split-init. Record both ALLOCDBG
tables. If j1 + chain-extender reaches 0, the exact instruction the chain-extender is worth
on the closed-seat chassis is named, and F1 inherits a precise target.

## [s115] candidate.c still measures 2/160 on HEAD and both of its FAKE units are still load-bearing and super-additive (mandated kill re-audit).
- mechanism: tools/fake_ablate.py recompiles the candidate with each FAKE unit removed and re-scores; a unit whose removal does not move the score is inert.
- probe: bash tools/wsl.sh 'source .venv/bin/activate && python3 tools/fake_ablate.py --func CD_sync --file system --candidate memory/grind/CD_sync/candidate.c'
- result: keep-all 2/160, drop chain-extender 15/159, drop pp alias 17/161, drop both 30/160 - bit-identical to s111/s114. Also learned: fake_ablate must run under WSL with the venv active; from PowerShell every variant reports ERR.
- verdict: CONFIRMED

## [s115] The global.c allocno priority printed by ALLOCDBG is floor_log2(nrefs)*nrefs/livelen*10000*size, not a loop-depth-weighted reference count.
- mechanism: The instrumented printf at tools/gcc-2.7.2/global.c:612-616 computes the value it prints; reading it removes the need to infer a weighting model from ratios.
- probe: Read tools/gcc-2.7.2/global.c:604-616 and check the formula against every ALLOCDBG row measured this session (g1 p77 9 refs/190 = 1421, p79 5/188 = 531, p80 2/21 = 952; i1 p80 4/22 = 3636; j1 p78 4/184 = 434).
- result: Exact on all rows. This retires s114-E5's 'the duplication adds about 13 weighted refs to p77' model and replaces every remaining seat question with integer arithmetic; the decisive structure is the floor_log2 cliffs at nrefs 3->4 (3->8) and 7->8 (14->24).
- verdict: CONFIRMED

## [s115] Hoisting both do_timeout index reads to the loop top - the only program point that dominates both timeout arms - shares them between the duplicated arms and lands the s-register seats without materialising extra instructions.
- mechanism: jump2 find_cross_jump re-merges the duplicated arms after flow and global have already counted the references, so ref inflation is pre-jump2 and byte-neutrality is post-jump2; the hoist was expected to remove the duplicate references while the loads stayed where the target puts them.
- probe: h1 = g1 with t0/ix read at the loop top and only the table math + printf + tail duplicated; spliced at src/system.c:376 and scored with sandbox CD_sync --disable all, ALLOCDBG captured.
- result: score 15, build_insns 159, rules_dropped 0. Ref counts land exactly as predicted (p77 back to 7 refs / pri 752, tbl at 5 refs / 543) and all five s-register seats are correct, but the hoisted reads become unconditional loop-top lbu instructions, the emitted code is one instruction short of the target, and 124 index-wise instructions differ. Banked rejected/s115_h1_hoisted_idx_reads_loop_top_15.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: g1 duplicated-arms base (memory/grind/CD_sync/rejected/s114_HONEST_BASE_dup_arms_13.c), control 13/160 rules_dropped 0; FAKE state = the pp pointer alias present, no chain-extender

## [s115] Splitting the saved computation into saved = *D_800A147C; saved = saved & 3; raises p80 from 2 references to 4 and seats all six callee-saved pseudos at their target registers.
- mechanism: The second set is genuinely consumed so it survives delete_dead_from_cse to reg_scan; pri(p80) goes from 2*10000/21 = 952 to 8*10000/22 = 3636, clearing p77's 1421 and taking $s1 first, after which find_reg hands $s2..$s6 to p77/p79/p78/p72/p73 in priority order.
- probe: i1 = g1 with the split-init; sandbox CD_sync --disable all plus the ALLOCDBG table and tmp/grind/CD_sync/s115/adiff.py (new difflib-aligned normalized diff).
- result: score 6, build_insns 160, rules_dropped 0, seats p80 $s1 / p77 $s2 / p79 $s3 / p78 $s4 / p72 $s5 / p73 $s6 - the first fully correct seat vector in 115 sessions. adiff decomposes the 6 into three independent 2-point defects: idx_1495's addiu emitted in the success block instead of the prologue, the sll/addu pair-swap, and the split-init's own cost (the target keeps the loaded byte in a separate $v0 pseudo). Banked rejected/s115_i1_split_init_saved_all_seats_6.c.
- verdict: CONFIRMED

## [s115] A dead self-assign saved = saved; adds references to p80 that survive to flow, in either of the two placements tested (immediately after the initial set, and immediately before the store).
- mechanism: dead-store-fake-exception carriers are supposed to inflate reg_n_refs before local-alloc; the counter-mechanism is delete_dead_from_cse removing the dead set before the reg_scan at toplev.c:2925, which s111 already measured for multi-set spellings of tbl_125c.
- probe: i2 (self-assign before the store) and i3 (self-assign after the set), both spliced on the g1 base and ALLOCDBG-captured.
- result: Both score 13/160/0 with p80's ALLOCDBG row bit-identical to the g1 control (2 refs / livelen 21 / pri 952). The self-assign leaves no trace by local-alloc time. i4 (mask moved to the store) scores 18/158. Banked rejected/s115_i2_saved_self_assign_inert_13.c, s115_i3_saved_self_assign_early_inert_13.c, s115_i4_mask_at_store_18.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: g1 duplicated-arms base, control 13/160 rules_dropped 0; FAKE state = pp pointer alias present plus the self-assign under test, no chain-extender

## [s115] Moving the idx_1495 init back to the prologue on the i1 chassis re-attaches its CONSTANT_P REG_EQUIV note and collapses its priority below the two parameters.
- mechanism: cse.c:6923 writes a REG_EQUAL note holding const (plus (symbol_ref D_800A1494) 1); update_equiv_regs converts it to REG_EQUIV and doubles reg_live_length at local-alloc.c:1064, taking p78 from livelen 92 to 184 at 2 references, i.e. pri 217 -> 108.
- probe: i5 = i1 with idx_1495 = idx_1494 + 1 moved from the success block to the prologue; ALLOCDBG plus the .lreg note grep (tmp/grind/CD_sync/s115/notes.sh).
- result: score 13/160/0; the note is present in the dump and p78 lands at pri 108 / $s6 while mode and result take $s4 and $s5. Confirms s112 and g2, and fixes the band p78 must reach with a prologue init at (208, 531) - i.e. exactly 4 references with the note on, or 2 references with the note denied. Banked rejected/s115_i5_idx1495_prologue_note_collapse_13.c.
- verdict: CONFIRMED

## [s115] Restoring the prologue idx_1495 init and spelling the do_timeout block's second index read as *idx_1495 in both duplicated copies moves both halves of the remaining gate at once and seats every callee-saved pseudo correctly.
- mechanism: The edit removes two references from p77 (9 -> 7, pri 1421 -> 736, back under p80's 952) and adds two to p78 (2 -> 4, pri 108 -> 434, inside the (208, 531) band) in a single change, because the two pseudos share the reference being re-routed; the floor_log2 factor is what makes a two-reference move worth a 3.3x priority swing.
- probe: j1 = g1 + prologue idx_1495 + ix = *idx_1495 in both copies; sandbox CD_sync --disable all, ALLOCDBG table, adiff.py residual decomposition.
- result: score 3, build_insns 160, rules_dropped 0. Seats: p80 $s1 (952), p77 $s2 (736), p79 $s3 (531), p78 $s4 (434), p72 $s5 (208), p73 $s6 (202) - all six correct. Residual is exactly three instructions: lbu v0,0(s4) where the target has lbu v0,1(s2) (the seat trade), plus the sll a0,a0,0x2 / addu v0,v0,s3 pair-swap at indices 54/55. j1 carries ONE FAKE construct (the pp pointer alias, measured load-bearing at +6 by rejected/s115_m1_pp_alias_dropped_9.c) and does NOT need the owner-refused cross-symbol chain-extender that candidate.c's floor-2 form depends on. Banked rejected/s115_HONEST_BASE_dup_arms_idx1495_read_3.c.
- verdict: CONFIRMED

## [s115] Permuting the source order of the two address chains, or of the two byte reads, inside the duplicated do_timeout block moves the 54/55 sll/addu pair toward the target order.
- mechanism: The target emits the ix chain (sll, addu) before the t0 chain's sll; the natural reading is that emission order follows statement order in the block, so writing the ix chain first should reproduce it.
- probe: k1 (ix chain first) and k2 (k1 plus the two byte reads swapped), both on the j1 base.
- result: Both score 7/160 against j1's 3 - the permutation costs four points rather than saving two. Statement order inside the block is not the lever for this pair on the duplicated-arms chassis. Banked rejected/s115_k1_chain_order_swap_7.c, s115_k2_read_order_swap_7.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: j1 duplicated-arms base (memory/grind/CD_sync/rejected/s115_HONEST_BASE_dup_arms_idx1495_read_3.c), control 3/160 rules_dropped 0; FAKE state = pp pointer alias present, no chain-extender


## s116 (escalation modality, owner 2026-09-02 window reset - ladder worked from the F2 rung)

## [s116] The 2-point `sll a0,a0,0x2` / `addu v0,v0,s3` inversion at normalized indices 54/55 - the residual s6-s93 spent sixty sessions calling theorem-locked - is chassis-relative and is DEFEATED in ordinary C on the j1 duplicated-arms chassis by splitting the arg5 address computation out of the load expression and into the `ix` variable.
- mechanism: In j1 the fifth printf argument is spelled `arg5 = *(s32 *)(ix + (s32)tbl_125c);`, so the sum lives in an anonymous pseudo that dies at the load. Writing it as `ix <<= 2; ix += (s32)tbl_125c; arg5 = *(s32 *)ix;` removes that pseudo and extends the `ix` variable's own live range across the load. The two dependence chains feeding the call ([sll a0][addu a0][lw a3]) and ([addu v0][lw v1][sw 16(sp)]) have equal INSN_PRIORITY (equal-cost paths to the jal), so sched1's choice at the contested clock is decided by rank_for_schedule's final INSN_LUID tie-break; the split changes which insn carries the lower LUID and sched1 then emits the target's exact order.
- probe: three independent spellings spliced onto the j1 base (control 3/160/0) and scored with sandbox CD_sync --disable all, each cross-read with tmp/grind/CD_sync/s116/adiff.py (difflib-aligned normalized diff against build/src/system.o): h1 = j1 statement order with only the address split; e3 = ix chain hoisted ahead of the t0 chain plus the split; k1 = s115's chain-order swap re-measured on this chassis.
- result: CONFIRMED. All three score 7/160/0 and all three produce an adiff whose indices 51-54, 57, 58, 60, 62-64 and 66+ are index-for-index EQUAL to the target - i.e. `sll a0,a0,0x2` now sits at index 55 exactly where the target puts it, and the 54/55 inversion is GONE. This is the first time in 116 sessions that the pair inversion has been removed by a C-level edit. Banked memory/grind/CD_sync/rejected/s116_h1_ix_addr_split_only_order_fixed_regs_swapped_7.c, s116_e3_ix_split_ixfirst_order_fixed_regs_swapped_7.c, s116_k1_chain_order_swap_order_fixed_regs_swapped_7.c.
- verdict: CONFIRMED

## [s116] The schedule fix can be decoupled from the local-alloc hard-register exchange it arrives with, so that the do_timeout block gets the target's insn ORDER and j1's register NAMES at the same time.
- mechanism: With the order fixed the whole residual becomes a two-way local-alloc exchange: the t0 chain takes $v1 and arg5 takes $a0 where the target has $a0 and $v1 (adiff indices 49, 55, 56, 59, 61, 65 - six points, against the two the order defect costs). local-alloc ranks block-local quantities by QTY_CMP_PRI = floor_log2(qty_n_refs)*qty_n_refs*qty_size*10000/(qty_death - qty_birth) and find_reg walks the hard registers in ascending number order (tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so the default 0..N applies and $v0=2 < $v1=3 < $a0=4). Reducing the t0 chain's reference count, or raising arg5's, was expected to invert the two quantities' rank and hand arg5 the lower register.
- probe: sixteen spellings on the j1 / h1 / e3 / k1 bases, all sandbox-scored and ALLOCDBG-captured: f1 (t0 address folded into the call expression) 15; f2 (t0 scale and add through two named intermediates ts/ta) 15; f3 (arg5 address in its own local) 8; e1/e2 (t0 scale placed between the ix split statements) 7/7; e4 (arg5 address in its own local ap) 7; e5 (arg5 local removed, both loads inline in the call) 10; e9 (declaration order t0-first) 7; g1 (arg5 folded into the ix variable) 10; g3 (the two byte reads swapped) 7; h2 (both t0 statements through named intermediates) 15; h3 (t0 chain staged through one intermediate) 15; h4/n2 (t0 address in its own local ta, on the h1 and k1 bases) 15/15; n4 (both addresses in their own locals) 15; m2 (no block locals at all, both table reads inline as tbl_125c[...] in the call) 15; m3 (only the two byte reads kept as locals) 15; m4 (array indexing with an arg5 local) 15.
- result: KILLED for all of them on these bases. Every spelling that keeps both self-reuse chains intact (`t0 *= 4; t0 = (s32)((u8 *)tbl_125c + t0);` and the ix split) scores 7 with the identical six-point a0/v1 exchange; every spelling that breaks either self-reuse chain - a separate address local, a named intermediate, array indexing, or folding the address into the call - jumps to 15 and loses the block's whole shape. f3 at 8 is the only intermediate value. The ALLOCDBG table is bit-identical across j1, e1, e2, e3 and e9 (p80 $s1 952 / p77 $s2 736 / p79 $s3 531 / p78 $s4 434 / p72 $s5 208 / p73 $s6 202), so none of this touches the six callee-saved seats: the exchange is purely block-local quantity ranking. On this chassis the order defect (2 points) and the register exchange (6 points) are coupled, and j1 at 3 remains the cheaper side of the coupling.
- verdict: KILLED
- kill_scope: instance
- measured_on: j1 duplicated-arms base (memory/grind/CD_sync/rejected/s115_HONEST_BASE_dup_arms_idx1495_read_3.c), control re-measured this session at 3/160 rules_dropped 0 with the s115 ALLOCDBG seat vector reproduced exactly; FAKE state = the pp pointer alias plus the duplicated-statement-into-arms carrier, no chain-extender

## [s116] Endgame-lock gate (a) re-measured: scan_hand_coded --single CD_sync still returns tier=LOW.
- mechanism: the canonical-asm grant path requires STRONG scanner signals (S1 multu-pacing / S2 empty-branch / S6 BIOS jumptable); CD_sync fires only S4 (four loads in an 8-insn window at insn 49) and S5 (one approx-sibling, CD_ready, jaccard 0.64).
- probe: tools/scan_hand_coded.py --single CD_sync under WSL; artifact tmp/grind/CD_sync/s116/scan_hand_coded.txt.
- result: HAND_CODED: tier=LOW score=2/8 (CD_sync, 160 insns) - bit-identical to the s107 measurement. Gate (a) remains FAILED; recorded here so a future disposition session does not have to re-run it.
- verdict: CONFIRMED

## [s116] The 2-point sll a0,a0,0x2 / addu v0,v0,s3 inversion at normalized indices 54/55 is chassis-relative and is defeated in ordinary C on the j1 duplicated-arms chassis by splitting the arg5 address computation out of the load expression and into the ix variable.
- mechanism: j1 spells the fifth printf argument as arg5 = *(s32 *)(ix + (s32)tbl_125c);, so the sum lives in an anonymous pseudo that dies at the load. Writing ix <<= 2; ix += (s32)tbl_125c; arg5 = *(s32 *)ix; removes that pseudo and extends the ix variable's own live range across the load. The two dependence chains feeding the call - [sll a0][addu a0][lw a3] and [addu v0][lw v1][sw 16(sp)] - have equal-cost paths to the jal and therefore equal INSN_PRIORITY, so sched1's choice at the contested clock falls through rank_for_schedule to the INSN_LUID tie-break; the split changes which of the two insns carries the lower LUID, and sched1 then emits the target's order.
- probe: Three independent spellings spliced onto the j1 base (control re-measured 3/160/0 this session) and scored with sandbox CD_sync --disable all, each cross-read with tmp/grind/CD_sync/s116/adiff.py: h1 = j1 statement order with only the address split; e3 = ix chain hoisted ahead of the t0 chain plus the split; k1 = s115's chain-order swap re-measured on this chassis.
- result: All three score 7/160 rules_dropped 0, and all three produce an adiff in which sll a0,a0,0x2 sits at index 55 exactly where the target puts it - the 54/55 inversion is gone, and indices 51-54, 57, 58, 60, 62-64 and 66+ are index-for-index equal to the target. First C-level removal of this pair in 116 sessions. It arrives coupled to a six-point register exchange (next hypothesis), so 7 is worse than j1's 3, but the axis s6-s93 declared locked is now measured open.
- verdict: CONFIRMED

## [s116] The schedule fix can be decoupled from the local-alloc hard-register exchange it arrives with, giving the do_timeout block the target's insn order and j1's register names at the same time, by re-ranking the t0 chain and arg5 quantities through their reference counts or live ranges.
- mechanism: With the order fixed the residual is a two-way local-alloc exchange worth six points: our t0 chain takes $v1 and arg5 takes $a0 where the target has $a0 and $v1 (adiff indices 49, 55, 56, 59, 61, 65). local-alloc ranks block-local quantities by QTY_CMP_PRI = floor_log2(qty_n_refs)*qty_n_refs*qty_size*10000/(qty_death - qty_birth) and find_reg walks hard registers in ascending number order, because tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER and the default 0..N ordering applies ($v0=2 < $v1=3 < $a0=4). Lowering the t0 chain's reference count or raising arg5's should invert the two quantities' rank and hand arg5 the lower register.
- probe: Sixteen spellings on the j1 / h1 / e3 / k1 bases, each sandbox-scored and ALLOCDBG-captured: f1 t0 address folded into the call (15); f2 t0 scale and add through two named intermediates (15); f3 arg5 address in its own local (8); e1/e2 t0 scale placed between the ix split statements (7/7); e4 arg5 address in its own local ap (7); e5 arg5 local removed, both loads inline (10); e9 declaration order t0-first (7); g1 arg5 folded into the ix variable (10); g3 the two byte reads swapped (7); h2 both t0 statements through named intermediates (15); h3 t0 chain staged through one intermediate (15); h4 and n2 t0 address in its own local on the h1 and k1 bases (15/15); n4 both addresses in their own locals (15); m2 no block locals at all, both table reads inline as tbl_125c[...] (15); m3 only the two byte reads kept as locals (15); m4 array indexing with an arg5 local (15).
- result: None of the sixteen decoupled them. Every spelling that keeps both self-reuse chains intact (t0 *= 4; t0 = (s32)((u8 *)tbl_125c + t0); plus the ix split) scores 7 with the identical six-point a0/v1 exchange; every spelling that breaks either self-reuse chain jumps to 15 and loses the block's whole shape, with f3 at 8 the only intermediate. The ALLOCDBG table is bit-identical across j1, e1, e2, e3 and e9, so none of this touches the six callee-saved seats - the exchange is purely block-local quantity ranking. On this chassis the 2-point order defect and the 6-point register exchange are coupled, and j1 at 3 is the cheaper side.
- verdict: KILLED
- kill_scope: instance
- measured_on: j1 duplicated-arms base (memory/grind/CD_sync/rejected/s115_HONEST_BASE_dup_arms_idx1495_read_3.c), control re-measured this session at 3/160 rules_dropped 0 with the s115 ALLOCDBG seat vector reproduced exactly; FAKE state = the pp pointer alias plus the duplicated-statement-into-arms carrier, no chain-extender

## [s116] Endgame-lock gate (a) for CD_sync: tools/scan_hand_coded.py --single CD_sync returns a STRONG tier, which would put the function on the canonical-asm grant path.
- mechanism: The grant path requires STRONG scanner signals S1 (multu pacing), S2 (empty branch) or S6 (BIOS jumptable).
- probe: tools/scan_hand_coded.py --single CD_sync under WSL; artifact tmp/grind/CD_sync/s116/scan_hand_coded.txt.
- result: HAND_CODED: tier=LOW score=2/8 (CD_sync, 160 insns) - only S4 (four loads in an 8-insn window at insn 49) and S5 (one approx-sibling, CD_ready, jaccard 0.64) fire. Bit-identical to the s107 measurement. Gate (a) is FAILED; banked here so a later disposition session does not re-run it.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD asm/funcs/CD_sync.s, 160 insns, no candidate spliced

## 2026-09-04 — operator reopen note (owner ruling 2026-09-04, decisions.md "foreclosed-bucket re-evaluation", Ruling A)

Returned to active with the exhaustion window RESET. Ground: the 2026-09-03
foreclosure (decisions.md:21280) was auto-filed by the driver backstop over s116,
which CONFIRMED the 54/55 sll/addu pair inversion defeated in ordinary C on the j1
chassis (h1/e3/k1 at 7/160 with the target's exact order — the first time in 116
sessions), named the new wall (the 6-pt a0/v1 local-alloc exchange), and expressly
declined to self-file because the axis was not exhausted. The backstop's dodge test
could not see that (fixed, driver Ruling B.1). The post-reset window s108-s116 ran no
permuter and no solver session; state.json frontier[1] names the sched_solver run as
still unexecuted. CD_datasync s58 (hypotheses.md:4558-4600, 2026-09-04) found that the
Sony `CD_alarm` struct over 0x800F19B8/BC/C0 makes the `void **pp` pointer-alias FAKE
non-load-bearing and links identically to its floor body; that finding never reached
this ledger because the propagation stamp fired only on a strictly-lower floor (fixed,
Ruling B.2). `engine/score.py` now resolves named-symbol HI16/LO16 addends (Ruling
B.4), so struct-model probes no longer pay +1 per member access.

Honest floor for window purposes: the j1 form (rejected/s115_HONEST_BASE_dup_arms_idx1495_read_3.c)
at 3/160/0. candidate.c's 2 rests on the owner-REFUSED cross-symbol arithmetic idiom
(`idx_1495 = &D_800A1494[1]`, decisions.md:948-950, 2026-07-20); that idiom stays
refused and must not be re-spelled. It is NOT the sanctioned combine-foldable
chain-extender (dead-store-fake-exception.md:51) this ledger uses legitimately — do not
conflate them in either direction. All standing judge_constraints remain in force.

**Named probes, in order (each a falsifiable measurement; a kill is `progress`):**
1. Transplant CD_datasync s58's `CD_alarm`-struct / no-`pp` spelling onto j1 (recipe:
   tmp/grind/CD_datasync/s58/batch2.sh; the s58 progress file is
   memory/grind/CD_datasync/progress/s58-alarm-struct-no-pp-fake-link-identical-2.c)
   and measure with the addend-resolving scorer (`sandbox CD_sync --disable all`).
2. Run tools/sched_solver and tools/ra_solver (`--target-object` escape) on j1 and h1
   over block 49-66, exactly as state.json frontier[1] prescribes.
3. A fresh-seed permuter window seeded from j1 (3) and h1 (7) — no permuter session
   has run since s95 and none on the duplicated-arms chassis (permuter cap: this
   function has used 2 permuter sessions historically; the driver's gating applies).


## s117 (2026-09-04) — rederive: sibling transplant + the QTY_CMP_PRI tie

Chassis re-verified live this session: j1 (duplicated arms, scalar model,
renamed calls) = 3/160 bi 160 rd 0; the floor body (goto shape, chain-extender)
= 2/160 bi 160 rd 0. All measurements below are on HEAD's src/system.c with
CD_sync spliced by tmp/grind/CD_sync/s117/apply.py (region-scoped: it edits only
CD_sync's own declaration block, never CD_datasync's duplicate copies).

**FIRST: the banked candidate did not compile.** Its body still called
sys_VSync / tslTm2LoadImage_2 / debug_printf / cdrom_ClearIrq /
sys_GetVblankCount / func_80080828. None of those names survive in HEAD. The
correct names, read off asm/funcs/CD_sync.s jal targets, are VSync / puts /
printf / CD_flush / CheckCallback / getintr. candidate.c is repaired.

### H117-1 CONFIRMED — CD_datasync s58's CD_alarm struct transplants onto CD_sync link-identically and deletes the pp FAKE

The owner directive's first probe. Declaring
typedef struct { s32 timeout; s32 count; char \*func; } CD_alarm; extern CD_alarm D_800F19B8;
(dropping the per-word D_800F19BC / D_800F19C0 externs from C) and reading
D_800F19B8.func directly in the printf call **instead of** the
pp = (void \*\*)&D_800F19C0; ... \*pp pointer alias:

- on the j1 duplicated-arms chassis: 3 -> 3 (A0 vs A1), and the objdump
  comparison is 212 records with **5 differences, every one an addend**
  (sw zero,0(at) -> sw zero,4(at); sw v0,0(at) -> sw v0,8(at);
  lw v1,0(v1) -> lw v1,4(v1); sw v0,0(at) -> sw v0,4(at);
  lw a1,0(a1) -> lw a1,8(a1)), all against R_MIPS_LO16 D_800F19B8
  resolving to the same addresses the named per-word symbols resolve to.
  Identical registers, identical order, identical insn count.
- on the floor (goto) chassis: 2 -> 2, banked as
  progress/s117-alarm-struct-nopp-2.c.

On the SCALAR model dropping pp costs 6 points (s115_m1 = 9). Under the
struct model the member MEM (mem (const (plus (symbol_ref D_800F19B8) 8)))
gets the early argument load that the alias had to buy, exactly the mechanism
CD_datasync s58 recorded. Net: **the floor form loses one FAKE** (only the
combine-foldable chain-extender remains) and loses the pp DECLARATION-PUN
that the dispatch auto-scan flags on this file. Unlike CD_datasync, the engine
sandbox did NOT inflate the struct score here — score.py masked all five
addends, so 2 is 2, and the objdump check confirms it independently.

### H117-2 KILLED (instance) — the CD_intr struct member read collapses idx_1495

Sony's typedef struct { u8 sync; u8 ready; u8 x2; u8 x3; } CD_intr; gives the
target's own 1($s2) do_timeout read as ORDINARY C (idx_1494->ready), which
would have retired the chain-extender FAKE. It does not survive: the moment
do_timeout reads through the base pointer, idx_1495 is left with its single
callback-site reference and stops being seated in $s4, rotating the whole
callee-saved assignment.

- B1 (idx_1495 = &idx_1494->ready, do_timeout reads idx_1494->ready) 20/160.
- B3 (idx_1495 = &D_800A1494.ready, same read) 20/160.
- B2 / B4 (same declarations, do_timeout keeps \*idx_1495) 3/160 — i.e. the
  CD_intr declaration is byte-INERT; it is the READ SITE that costs 17 points.
- A3 (scalar idx_1494[1] in both duplicated arms) 20/160.
- A4 (A3 + s112's success-block placement of the idx_1495 init) 18/160.

All measured on the pp-free struct chassis; one FAKE present (the chain-extender)
in the floor forms, none in A3/B1/B3, which is precisely why those collapse.

### H117-3 KILLED (instance) — removing pp does not decouple order-exactness from the $a0/$v1 seat exchange

CD_datasync s58 predicted that block 3 with one pseudo fewer might reprice the
local-alloc fixed point. It does not. Seven order-fixing spellings on the
pp-free struct chassis all land on 6/160 (was 7/160 with pp — the pp removal is
worth exactly the same 1 point everywhere):
C1 h1-style ix address split 6 · C2 e3-style ix-first split 6 ·
C3 k1-style chain-order swap 6 · C4 scale-between-address-and-load 6 ·
E1 both addresses split 6 · E3 ix-split/t0-scale-last 6 ·
E4 ix-split/load-then-t0-address 6.
Seat-exact spellings all land on 2 (A5, E2). The behaviour is strictly
bimodal, the same bimodality CD_datasync s57 recorded. C1's residual is a PURE
register exchange, verified instruction-for-instruction: order-exact for all
160, with t0's chain in $v1 and printf's arg5 value in $a0 where the target has
them the other way round (indices 49, 55, 56, 59, 61, 65).

### H117-4 KILLED (instance) — reference-count edits on the order-exact chassis

- D1 (t0's addu+load folded into the printf argument) 14/160.
- D2 (t0's whole chain folded into the printf argument) 14/160.
- D3 (dead ix = arg5; after the load) 6/160 — byte-INERT, and a dead store, so
  it is not a lever and is not banked as a form.
- D4 (arg5 read inline at the call instead of through a local) 9/160.
- E5 (t0's address staged through ix after its own last use) 14/160.

### H117-5 CONFIRMED (mechanism) — the contested seat is a QTY_CMP_PRI TIE, not a reference-count inversion

The standing frontier's model ("arg5 at 2 references = 4000 loses to the t0
chain's 4 references = 5000; lift arg5 to 3 references") is **wrong about which
quantities are contested**. Measured with the instrumented cc1's BB2_QTY_DEBUG
hook (tools/gcc-2.7.2/cc1, local-alloc.c) on a single-function TU — new
reusable tooling, tmp/grind/CD_sync/s117/mini.sh + qtyrun.sh, which extracts
CD_sync plus its own declaration block into tmp/grind/CD_sync/s117/mini.c and
reproduces the full-TU codegen exactly (pseudo numbering restarts per function,
so the qty table is TU-independent; verified against the full-TU dump). The
whole-TU stderr stream is unusable for this because QTYDBG lines carry no
function name — segmenting it by blk resets gives 19 candidate segments and
CD_sync is not identifiable among them. The mini-TU is the tool.

do_timeout is basic block 3. Four quantities:

| form | qty | reg | birth | death | span | refs | got |
|---|---|---|---|---|---|---|---|
| A5 (seat-exact, 2) | 0 | 113 | 10 | 20 | 10 | 6 | $v0 |
| A5 | 3 | 120 | 22 | 30 | 8 | 4 | $v0 |
| A5 | **2** (arg5 value) | 106 | 20 | 26 | **6** | 2 | **$v1** |
| A5 | **1** (t0 address) | 112 | 16 | 24 | **8** | 2 | **$a0** |
| C2 (order-exact, 6) | 0 | 108 | 10 | 20 | 10 | 6 | $v0 |
| C2 | 3 | 119 | 22 | 30 | 8 | 4 | $v0 |
| C2 | **1** (t0 address) | 112 | 18 | 24 | **6** | 2 | **$v1** |
| C2 | **2** (arg5 value) | 106 | 20 | 26 | **6** | 2 | **$a0** |

(C1 is identical to C2 in this table.)

QTY_CMP_PRI = floor_log2(refs) \* refs \* size \* 10000 / (death - birth):

- A5: qty1 = 1\*2\*10000/8 = **2500**, qty2 = 1\*2\*10000/6 = **3333**
  -> qty2 allocated first, takes the lowest free hard reg $v1(3); qty1 takes
  $a0(4). TARGET SEATS.
- C1/C2: qty1 = 3333, qty2 = 3333. **EXACT TIE.** qty_compare_1 falls through
  to \*q1 - \*q2, i.e. the quantity INDEX, and qty1 < qty2, so the t0 address is
  allocated first and takes $v1. WRONG SEATS.

**The entire order-vs-seat two-body problem is one luid.** Making the emission
order exact moves qty1's BIRTH from 16 to 18 — one luid, nothing else in the
table changes, not a single ref count, not qty2's range — and that single luid
converts a 3333-vs-2500 win into a 3333-vs-3333 tie that is lost on index.
This supersedes every reference-count framing of this residual since s108.

### Closing condition (numeric, for the next session)

On an ORDER-EXACT emission, achieve ANY ONE of:

- (a) qty1 birth <= 16 (span >= 8): pri <= 2500 < 3333;
- (b) qty2 refs >= 3 (span unchanged at 6): pri = floor_log2(3)\*3\*10000/6 = 5000 > 3333;
- (c) qty2 death <= 24 (span <= 4): pri >= 5000;
- (d) qty2's quantity INDEX below qty1's, which wins the tie outright.

(a) is the one the measurement says is one luid away. (d) is untouched by any
session and is the cheapest to test, because qty numbers are handed out in
local-alloc's block scan order: anything that makes the arg5-value pseudo's
first reference precede the t0-address pseudo's first reference flips it. Note
the table already shows this is NOT simply emission order — in C2 the arg5 lw
is emitted two insns BEFORE the t0 addu, yet qty1 still births at 18 and qty2
at 20, so birth is not 2x the in-block insn index and the luid mapping needs
reading out of the .lreg/.sched pair before (d) can be aimed. That
reconciliation is the first job of the next session and it is a dump read, not
a sandbox run.

## [s117] CD_datasync's s58 CD_alarm struct object model (typedef struct { s32 timeout; s32 count; char *func; }, per-word D_800F19BC / D_800F19C0 externs dropped from C) transplants onto CD_sync's chassis link-identically and makes the `void **pp` pointer-alias FAKE redundant.
- mechanism: Under the struct model the printf argument is a member MEM (mem (const (plus (symbol_ref D_800F19B8) 8))), which local-alloc.c's update_equiv_regs already gives the early argument load that the bare (mem (symbol_ref D_800F19C0)) needed the alias to buy. Identical mechanism to CD_datasync s58.
- probe: Applied the struct declaration surface plus the direct member read to both CD_sync chassis via tmp/grind/CD_sync/s117/apply.py (region-scoped so CD_datasync's duplicate decl block is untouched), then compared objdump -dr records instruction-for-instruction against the scalar build.
- result: j1 duplicated-arms chassis: A0 (scalar, pp present) 3/160 bi 160 rd 0 vs A1 (struct, pp deleted) 3/160 bi 160 rd 0, and the object comparison is 212 records with exactly 5 differences, all of them R_MIPS_LO16 addends against D_800F19B8 (+4 / +8) that the linker resolves to the same addresses the named per-word symbols resolve to — identical registers, identical order, identical count. Floor (goto) chassis: A5 (struct, pp deleted) 2/160 bi 160 rd 0, matching the ledger floor, banked as memory/grind/CD_sync/progress/s117-alarm-struct-nopp-2.c. On the SCALAR model dropping pp costs 6 points (s115_m1 = 9/160), so this is a strict improvement in defensibility: the floor form now carries ONE FAKE (the combine-foldable chain-extender) instead of two, and the pp DECLARATION-PUN the dispatch auto-scan flags on candidate.c is gone. Unlike CD_datasync, engine/score.py masked all five addends here, so the reported 2 needs no correction — verified independently against the objdump records.
- verdict: CONFIRMED

## [s117] Sony's CD_intr struct model (typedef struct { u8 sync; u8 ready; u8 x2; u8 x3; }) lets do_timeout read the target's own 1($s2) byte as ordinary C (idx_1494->ready), retiring the combine-foldable chain-extender FAKE, on the pp-free struct chassis.
- mechanism: The member offset becomes part of the type instead of link-constant pointer arithmetic, so the target's lbu $v0,0x1($s2) would fall out of an ordinary member read and idx_1495 would only need to exist for the callback site's lbu $a0,0x0($s4).
- probe: Four CD_intr declarations measured on the pp-free struct chassis: B1 (idx_1495 = &idx_1494->ready + member read), B3 (idx_1495 = &D_800A1494.ready + member read), B2/B4 (same declarations, do_timeout keeps *idx_1495), plus the scalar analogues A3 (idx_1494[1] in both duplicated arms) and A4 (A3 + s112's success-block placement of the idx_1495 init).
- result: B1 20/160, B3 20/160, A3 20/160, A4 18/160 — while B2 3/160 and B4 3/160 prove the CD_intr DECLARATION itself is byte-inert. The 17-point cost is entirely the READ SITE: once do_timeout stops dereferencing idx_1495, that pointer is down to its single callback-site reference, loses its $s4 seat, and the whole callee-saved assignment rotates. The chain-extender FAKE is exactly what buys idx_1495 the second reference; the struct model does not substitute for it.
- verdict: KILLED
- kill_scope: instance
- measured_on: pp-free CD_alarm-struct chassis (memory/grind/CD_sync/progress/s117-alarm-struct-nopp-2.c), control 2/160 bi 160 rd 0; FAKE state = chain-extender present in the control, ZERO FAKE constructs in B1/B3/A3 (which is why they collapse), chain-extender present in B2/B4

## [s117] Deleting the pp pseudo from the do_timeout block reprices local-alloc's fixed point enough to let an order-exact emission keep the target's $a0/$v1 seats, closing the order-vs-seat two-body problem.
- mechanism: CD_datasync s58's prediction: one pseudo fewer in the contested block changes which quantities compete for the two low caller-saved hard registers, so the order fix might arrive without the register exchange that costs 5 points on the scalar chassis.
- probe: Seven order-fixing spellings re-measured on the pp-free struct chassis (C1 h1-style ix-address split, C2 e3-style ix-first split, C3 k1-style chain-order swap, C4 scale-between-address-and-load, E1 both addresses split, E3 ix-split/t0-scale-last, E4 ix-split/load-then-t0-address), against the seat-exact controls A5 and E2.
- result: All seven order-exact forms 6/160 bi 160 rd 0; both seat-exact forms 2/160. The pp removal is worth exactly 1 point uniformly (7 -> 6 on the order-exact side, 3 -> 3 / 2 -> 2 on the seat-exact side) and changes nothing structural. C1's residual is a PURE register exchange with the order correct for all 160 instructions: t0's chain in $v1 and printf's arg5 value in $a0 where the target has them the other way round (differing indices 49, 55, 56, 59, 61, 65). Strict bimodality, the same shape CD_datasync s57 recorded.
- verdict: KILLED
- kill_scope: instance
- measured_on: pp-free CD_alarm-struct chassis (progress/s117-alarm-struct-nopp-2.c), control 2/160 bi 160 rd 0; FAKE state = chain-extender only, pp deleted

## [s117] Reference-count edits inside the do_timeout block on the order-exact pp-free chassis lift the arg5 quantity's allocation priority above the t0 chain's and recover the target seats.
- mechanism: The standing frontier's model: QTY_CMP_PRI = floor_log2(refs)*refs*size*10000/(death-birth), so adding a reference to arg5 or removing one from the t0 chain should invert the allocation order and hand $v1 to arg5.
- probe: Five spellings that add or remove a reference without introducing a new named local: D1 (t0's addu+load folded into the printf argument), D2 (t0's whole chain folded into the printf argument), D3 (dead ix = arg5; after the load), D4 (arg5 read inline at the call instead of through the local), E5 (t0's address staged through ix after ix's own last use).
- result: D1 14/160, D2 14/160, D3 6/160 (byte-INERT, and a dead store, so not banked as a form), D4 9/160, E5 14/160. None reaches 2 and none fixes the seat. The QTY_DEBUG dump then explained why: the contested pair is not the one the model named — see the CONFIRMED mechanism hypothesis below.
- verdict: KILLED
- kill_scope: instance
- measured_on: C1 order-exact pp-free struct chassis (memory/grind/CD_sync/rejected/s117_C1_struct_h1_ix_addr_split.c), control 6/160 bi 160 rd 0; FAKE state = chain-extender only, pp deleted

## [s117] The contested $a0/$v1 seat in do_timeout is decided by a QTY_CMP_PRI tie between two 2-reference quantities whose spans differ by one luid, not by the reference-count inversion between arg5 and the t0 chain that the frontier has modelled since s108.
- mechanism: local-alloc.c ranks quantities by QTY_CMP_PRI = floor_log2(refs)*refs*size*10000/(death-birth) and, on a tie, qty_compare_1 falls through to the quantity INDEX (*q1 - *q2); find_free_reg then walks hard registers ascending, so the first-ranked quantity takes $v1(3) and the second takes $a0(4).
- probe: Read the real quantity table with the instrumented cc1's BB2_QTY_DEBUG hook (tools/gcc-2.7.2/cc1, local-alloc.c) on a single-function TU built by new reusable tooling (tmp/grind/CD_sync/s117/mini.sh + qtyrun.sh, which extracts CD_sync plus its own declaration block into mini.c and reproduces the full-TU codegen exactly), for the seat-exact A5 and the order-exact C1/C2.
- result: do_timeout is basic block 3. A5 (seat-exact, 2/160): qty1 = t0 address, reg 112, birth 16 death 24 span 8 refs 2, got $a0; qty2 = arg5 value, reg 106, birth 20 death 26 span 6 refs 2, got $v1 — priorities 2500 vs 3333, so qty2 is allocated first and takes $v1, which is the TARGET seating. C1/C2 (order-exact, 6/160): qty1 birth 18 death 24 span 6, qty2 unchanged at birth 20 death 26 span 6 — priorities 3333 vs 3333, an EXACT TIE, resolved on quantity index, qty1 < qty2, so the t0 address takes $v1 and the seats are wrong. Making the emission order exact moves qty1's birth by exactly ONE luid (16 -> 18) and changes nothing else in the table: not a reference count, not qty2's range. The whole two-body problem is that one luid. Closing condition is now numeric on an order-exact emission: (a) qty1 birth <= 16, or (b) qty2 refs >= 3 (pri 5000), or (c) qty2 death <= 24, or (d) qty2's quantity index below qty1's. Also banked as tooling: the whole-TU QTYDBG stderr stream is unusable because its lines carry no function name (segmenting by blk resets gives 19 candidate segments and CD_sync is not identifiable among them) — the mini-TU is the instrument.
- verdict: CONFIRMED

## [s118] The frontier's luid reconciliation: local-alloc's insn_number counts CODE_LABELs, so every block-3 birth/death maps 1:1 onto a named insn, and closing conditions (a) and (d) are the SAME variable as order-exactness.
- mechanism: local-alloc.c:1173-1175 increments insn_number for every insn in the block whose code is not NOTE - which includes the block's own CODE_LABEL - and reg_is_born/post_mark_life record birth = 2*insn_number (local-alloc.c:1865, :2031, :2119). So GCC's luid = 2*(position of the insn among the block's non-NOTE insns), and for block 3 (whose head is code_label 99) that is 2*(N+1) where N is the insn's index among the real insns.
- probe: instrumented cc1 with -da on a single-function TU (new tooling: tmp/grind/CD_sync/s118/dump.sh, dumpenv.sh, mkmini.py) for A5 (seat-exact, 2/160) and C1 (order-exact, 6/160), then tmp/grind/CD_sync/s118/blocks.py to segment gccdump.sched into basic blocks and enumerate block 3 under both numberings, cross-checked against the BB2_QTY_DEBUG table.
- result: the mapping is exact and complete. C1 block 3: n1 uid104 "a0 = &D_800161B8", n2 uid106 call puts, n3 uid112 "reg107 = D_800A1494[0]", n4 uid116 "reg108 = D_800A1494[1]", n5 uid154 "a1 = D_800F19B8.func", n6 uid127 "reg108 <<= 2", n7 uid130 "reg108 += reg79", n8 uid120 "reg112 = reg107 << 2" (= qty1 BIRTH 18), n9 uid133 "reg106 = mem(reg108)" (= qty2 BIRTH 20), n10 uid141 lbu D_800A11D5, n11 uid125 "reg107 = reg112 + reg79" (= qty1 DEATH 24), n12 uid150 "sw reg106,16(sp)" (= qty2 DEATH 26), n13 sll, n14 lw a2, n15 lw a3, n16 set a0, n17 call printf. A5 is the same list with n7/n8 exchanged (t0 scale before the ix addu) and the ix address in a fresh pseudo 113 that combine_regs ties into qty0. THE CONSEQUENCE: qty1's birth IS the position of the t0-scale insn and qty2's birth IS the position of the arg5 load. The target emission (asm/funcs/CD_sync.s indices 49-56) puts the t0 scale at n8 and the arg5 load at n9, so on any order-exact emission qty1 births at 18 and qty2 at 20. Condition (a) "qty1 birth <= 16" is therefore literally the statement "emit the t0 scale one insn earlier" = the A5 order = not order-exact; and because alloc_qty hands out quantity numbers in birth order (local-alloc.c:284, next_qty++ inside alloc_qty called from reg_is_born), condition (d) "qty2's index below qty1's" is that same statement a second time. (a) and (d) are not two independent levers - they are one variable, and it is the variable the emission order already fixes.
- verdict: CONFIRMED

## [s118] qty_n_refs is reg_n_refs[regno] - the FUNCTION-WIDE, loop-depth-weighted reference count built by flow.c - not the block-local count the frontier has assumed since s108.
- mechanism: local-alloc.c:297 "qty_n_refs[qty] = reg_n_refs[regno];" inside alloc_qty. reg_n_refs is accumulated during flow.c's life analysis as "reg_n_refs[regno] += loop_depth" per reference, loop_depth being incremented at NOTE_INSN_LOOP_BEG. flow runs BEFORE combine, so a reference combine later folds away still counts (the mechanism the banked chain-extender FAKE already exploits), and a reference inside any loop-noted region counts twice.
- probe: read local-alloc.c:279-299 (alloc_qty) and the flow.c accumulation, then confirmed empirically by the do-while(0) measurement below, where a wrap around ONE statement moved exactly the pseudos referenced in that statement, by exactly +1 per reference.
- result: confirmed by construction and by measurement. This is a correction to the ledger's model and it is what makes closing condition (b) reachable from C at all.
- verdict: CONFIRMED

## [s118] A "do { ... } while (0);" wrap raises reg_n_refs by +1 for every reference inside it, and lifting the arg5-value quantity to 3-4 references DOES win the QTY_CMP_PRI contest and seat it in $v1 - closing condition (b) is reachable.
- mechanism: the wrap emits NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END around its body, so flow.c's loop_depth is 2 inside it and each reference contributes 2 instead of 1 to reg_n_refs; local-alloc.c:297 then hands that inflated count to QTY_CMP_PRI = floor_log2(refs)*refs*size*10000/(death-birth) (local-alloc.c:1649).
- probe: M1 (wrap around "arg5 = *(s32 *)ix;" alone, C1 order-exact chassis) and M5 (wrap around the ix address + load + printf), each dumped with BB2_SUGG_DEBUG + BB2_QTY_DEBUG on the mini TU (tmp/grind/CD_sync/s118/M1, /M5).
- result: M1 block 3: reg106 refs 2 -> 3 and reg108 refs 6 -> 7 - exactly the two pseudos referenced by the single wrapped statement, +1 each, nothing else moved. M5 block 3: reg106 refs 4 (both of its references inside the wrap), priority 13333 against the t0 temp's 3333, allocated first, "QTYDBG blk=3 ord=2 qty=2 reg1=106 birth=22 death=26 refs=4 got=3" - i.e. the arg5 value takes $v1, the TARGET seat, on an emission that is not the A5 order. The seat half of the two-body problem falls to a reference-count lever for the first time in this ledger.
- verdict: CONFIRMED

## [s118] The do-while(0) reference lever closes the function: with the arg5 quantity seated in $v1 by the ref lift, some wrap boundary plus statement order also reproduces the target's emission order.
- mechanism: if the reference lift were orthogonal to code motion, one of the wrap placements should keep the C1/target emission order while paying only the note pair.
- probe: eleven wrap geometries plus three refinements, scored with "tools/wteng.ps1 main sandbox CD_sync --disable all": M1 (arg5 load only, C1) 11, M2 (ix addr + load, C1) 11, M3 (arg5 load, A5) 11, M4 (t0 chain, C1) 11, M7 (printf only, C1) 10, M8 (printf only, A5) 8, M5 (ix addr + load + printf, C1) 7, M6 (load + printf, A5) 7, N4 (t0 chain first, then wrap load+printf) 7, N2 (ix chain first, whole t0 chain outside, wrap load+printf) 8, N3 (folded ix address, wrap load+printf) 9, N1 (ix chain first, t0 scale outside, wrap = t0 addu + load + printf) 5, P1 (N1 with the two lbu statements swapped) 5, P2 (N1 with the printf's D_800F19B8.func staged through a "char *fn" local before the wrap) 5, P3 (both) 5. All build_insns 160, rules_dropped 0. Best in family 5/160 against the standing floor of 2/160.
- result: KILLED. The NOTE_INSN_LOOP_BEG/END pair is a code-motion barrier as well as a reference multiplier: nothing inside the wrap hoists past its head. N1's residual, read with tmp/grind/CD_sync/s117/cmp.py, is (i) the two lbu reads of D_800A1494 exchanged at target indices 49/50 - and swapping the two C statements does NOT swap them back (P1 = 5) - and (ii) the "lui a1 / lw a1,8(a1)" load of D_800F19B8.func sunk from target indices 51/52 to 55/56 because the printf that consumes it is inside the wrap; staging that value through a local declared before the wrap does not lift it either (P2 = 5). The lever is real and the seat is real; the CARRIER is what costs 3 points over the floor.
- verdict: KILLED
- kill_scope: instance
- measured_on: C1 order-exact pp-free CD_alarm-struct chassis and the A5 seat-exact chassis, both with the chain-extender FAKE present and pp deleted; controls C1 6/160 and A5 2/160; all wrap forms build_insns 160 rules_dropped 0

## [s118] Writing the outer polling loop as a real C loop ("for (;;)") gives the whole do_timeout region loop_depth 2 - the same reference-count lift as a wrap, without a wrap.
- mechanism: flow.c weights reg_n_refs by loop_depth, so a genuine loop around the whole body doubles every reference count inside it; the polling loop is currently spelled "loop:" / "goto loop", which emits no loop notes at all, so the entire function sits at loop_depth 1.
- probe: L1 (A5 chassis) and L2 (C1 chassis) with "loop:" / "goto loop" replaced by "for (;;) { ... }", everything else byte-identical.
- result: KILLED. L1 67 at build_insns 165, L2 70 at build_insns 165. The real loop hands loop.c an actual loop to optimise: five extra instructions appear (invariant hoisting / rotation) and the function is no longer the right length. Note also that the doubling is SYMMETRIC - it multiplies the t0 temp's references by the same factor as the arg5 value's, so the 3333-vs-3333 tie would survive it even at 160 insns. Asymmetry is the whole point of the lever, and only a wrap smaller than the block delivers it.
- verdict: KILLED
- kill_scope: instance
- measured_on: A5 and C1 pp-free CD_alarm-struct chassis, chain-extender FAKE present; controls 2/160 and 6/160; the for(;;) forms build at 165 insns

## [s118] local-alloc's insn_number counts CODE_LABELs, so every block-3 birth/death maps 1:1 onto a named insn, and s117's closing conditions (a) 'qty1 birth <= 16' and (d) 'qty2 index below qty1' are restatements of the emission-order variable rather than independent levers.
- mechanism: local-alloc.c:1173-1175 increments insn_number for every non-NOTE insn in the block including its own CODE_LABEL, and reg_is_born/post_mark_life record birth = 2*insn_number (local-alloc.c:1865, :2031, :2119); alloc_qty (local-alloc.c:284) hands out quantity numbers in birth order, so quantity INDEX is birth order.
- probe: Instrumented cc1 with -da on a single-function TU for A5 (seat-exact, 2/160) and C1 (order-exact, 6/160) via new tooling tmp/grind/CD_sync/s118/dump.sh + mkmini.py, then tmp/grind/CD_sync/s118/blocks.py to enumerate block 3 with luid, position and uid, cross-checked against BB2_QTY_DEBUG.
- result: Complete mapping banked in hypotheses.md: qty1 birth 18 = insn n8 uid120 'reg112 = reg107 << 2', qty2 birth 20 = insn n9 uid133 'reg106 = mem(reg108)', qty1 death 24 = n11 uid125 'reg107 = reg112 + reg79', qty2 death 26 = n12 uid150 'sw reg106,16(sp)'. Since the target emission puts the t0 scale at n8 and the arg5 load at n9, qty1 necessarily births at 18 and indexes below qty2 on any order-exact emission. Conditions (a) and (d) are the same statement as 'use the A5 order'.
- verdict: CONFIRMED

## [s118] qty_n_refs is reg_n_refs[regno], the function-wide loop-depth-weighted count built by flow.c, not the block-local reference count the frontier has assumed since s108.
- mechanism: local-alloc.c:297 copies reg_n_refs[regno] into qty_n_refs inside alloc_qty; flow.c accumulates reg_n_refs as '+= loop_depth' per reference and runs before combine, so combine-deleted references still count and references inside a loop-noted region count twice.
- probe: Read local-alloc.c:279-299 and the flow.c accumulation, then confirmed empirically: a do-while(0) wrap around a single statement moved exactly the pseudos referenced in that statement, by exactly +1 per reference.
- result: Confirmed by construction and by measurement (M1: reg106 2->3, reg108 6->7, nothing else). This correction is what makes closing condition (b) reachable from C.
- verdict: CONFIRMED

## [s118] A do { ... } while (0); wrap raises reg_n_refs by +1 for every reference inside it, and lifting the arg5-value quantity to 3-4 references wins the QTY_CMP_PRI contest and seats it in $v1 on a non-A5 emission.
- mechanism: The wrap emits NOTE_INSN_LOOP_BEG/END, so flow.c's loop_depth is 2 inside it and each reference contributes 2 to reg_n_refs; local-alloc.c:297 feeds that into QTY_CMP_PRI = floor_log2(refs)*refs*size*10000/(death-birth) at local-alloc.c:1649.
- probe: M1 (wrap = the single arg5 load) and M5 (wrap = ix address + load + printf) on the C1 order-exact pp-free CD_alarm-struct chassis, dumped with BB2_SUGG_DEBUG + BB2_QTY_DEBUG on the mini TU.
- result: M1 block 3: reg106 refs 2->3, reg108 6->7, exactly the wrapped statement's two pseudos. M5 block 3: reg106 refs 4, priority 13333 vs the t0 temp's 3333, allocated first, 'QTYDBG blk=3 ord=2 qty=2 reg1=106 birth=22 death=26 refs=4 got=3' - the arg5 value seated in $v1, the target seat, without the A5 emission order. First reference-count win of this seat in 118 sessions.
- verdict: CONFIRMED

## [s118] Some do-while(0) wrap boundary plus statement order both lifts the arg5 reference count and reproduces the target's emission order, taking the wrap family below the floor of 2/160.
- mechanism: If the reference lift were orthogonal to code motion, one wrap placement should keep the C1/target emission order while paying only the note pair.
- probe: Fourteen geometries scored with 'tools/wteng.ps1 main sandbox CD_sync --disable all': M1 11, M2 11, M3 11, M4 11, M7 10, M8 8, M5 7, M6 7, N4 7, N2 8, N3 9, N1 5, P1 5, P2 5, P3 5 - all at build_insns 160, rules_dropped 0.
- result: Best in family is 5/160 against the standing floor of 2/160. The NOTE_INSN_LOOP_BEG/END pair is a code-motion barrier as well as a reference multiplier: N1's residual is the two lbu reads of D_800A1494 exchanged at target indices 49/50 (swapping the two C statements does not swap them back, P1 = 5) plus the 'lui a1 / lw a1,8(a1)' load of D_800F19B8.func sunk from indices 51/52 to 55/56 because the printf consuming it is inside the wrap (staging it through a local before the wrap does not lift it, P2 = 5). The lever and the seat are real; this carrier for it costs 3 points.
- verdict: KILLED
- kill_scope: instance
- measured_on: C1 order-exact and A5 seat-exact pp-free CD_alarm-struct chassis, chain-extender FAKE present and pp deleted; controls C1 6/160 and A5 2/160; all wrap forms build_insns 160 rules_dropped 0

## [s118] Writing the outer polling loop as a real C loop (for (;;)) gives the whole do_timeout region loop_depth 2 and the same reference lift without a wrap.
- mechanism: flow.c weights reg_n_refs by loop_depth; the polling loop is currently spelled 'loop:' / 'goto loop', which emits no loop notes, so the entire function sits at loop_depth 1.
- probe: L1 (A5 chassis) and L2 (C1 chassis) with 'loop:' / 'goto loop' replaced by 'for (;;) { ... }', everything else identical, scored with sandbox --disable all.
- result: L1 67 at build_insns 165, L2 70 at build_insns 165 - loop.c optimises the now-real loop and the function grows five instructions. The lift is also symmetric (it doubles the t0 temp's references too), so it could not break the 3333-vs-3333 tie even at 160 insns.
- verdict: KILLED
- kill_scope: instance
- measured_on: A5 and C1 pp-free CD_alarm-struct chassis, chain-extender FAKE present; controls 2/160 and 6/160; the for(;;) forms build at 165 insns

## [s119] N1's 5-point residual is code motion only: the do-while(0) wrap chassis already seats the arg5 value in $v1 and the t0-scale temp in $a0.
- mechanism: frontier item 3 from s118. If reg106 is already at got=3 on N1, the whole remaining 5 points are the NOTE_INSN_LOOP_BEG code-motion barrier and not allocation.
- probe: BB2_QTY_DEBUG + BB2_SUGG_DEBUG on the mini TU for N1 (tmp/grind/CD_sync/s119/N1/qty.txt; new tooling tmp/grind/CD_sync/s119/qd.sh = apply + mkmini + instrumented dump in one WSL call).
- result: "QTYDBG blk=3 ord=2 qty=2 reg1=106 birth=18 death=26 refs=4 got=3" and "ord=3 qty=1 reg1=112 birth=16 death=24 refs=3 got=4". reg106 = $v1, reg112 = $a0, i.e. the TARGET seats. N1's 5 points are 100% code motion.
- verdict: CONFIRMED

## [s119] local-alloc TIES a quantity dest with any dying pseudo operand of a 2+-operand insn and SUMS their reference counts (local-alloc.c:1932), so a quantity refs value is not the dest pseudo own refs.
- mechanism: block_alloc (local-alloc.c:1210-1300) treats any insn whose operand 0 has an = constraint and is not earlyclobber as tie-able and calls combine_regs (local-alloc.c:1295/1331/1336/1346) for each operand that dies in the insn; combine_regs folds SREG into UREG quantity and accumulates qty_n_calls_crossed and qty_n_refs.
- probe: read local-alloc.c:1190-1300 and 1890-1945; cross-checked against the s117/s118 quantity tables, where A5 qty0 reports refs=6 for reg113 (a two-reference plus temp) because reg108 (4 refs) dies into it, matching C1 single-pseudo reg108 refs=6.
- result: CONFIRMED by construction and by the table. Negative corollary measured here: a LOAD does not tie (its source is a MEM, not a REG), so the arg5 pseudo cannot inherit refs from the address pseudo that dies at the load.
- verdict: CONFIRMED

## [s119] An explicit second local carrying the arg5 value (arg5b = arg5;) survives cse and gives local-alloc a copy to tie, lifting the quantity reference count.
- mechanism: local-alloc.c:1932 sums refs across a tie, so a surviving copy would give the arg5 quantity 2+2 refs and priority 13333 against the t0 temp 3333.
- probe: R5_copy_local on the C1 order-exact pp-free CD_alarm-struct chassis, scored and dumped (tmp/grind/CD_sync/s119/R5/qty.txt).
- result: 6/160, byte-identical to the C1 control, and the quantity table is identical to C1 (reg106 birth 20 death 26 refs 2 got=4). cse propagates the copy away before flow ever counts it, so no tie and no reference lift.
- verdict: KILLED
- kill_scope: instance
- measured_on: C1 order-exact pp-free CD_alarm-struct chassis (memory/grind/CD_sync/rejected/s117_C1_struct_h1_ix_addr_split.c), control 6/160 bi 160 rd 0; FAKE state = chain-extender only, pp deleted

## [s119] Re-ordering printf argument EVALUATION (staging the 3rd/4th argument into a local, or moving the t0 address computation across the arg5 load) shrinks the arg5 quantity span to <= 4 (s117 closing condition (c)) or otherwise repairs the seat.
- mechanism: s118 frontier item 2. qty2 dies at its store into the outgoing-argument slot; that store position is decided by the order in which expand emits the call argument setup, so an argument-order change should move the death.
- probe: seven spellings on the C1 chassis, scored with sandbox --disable all and dumped with BB2_QTY_DEBUG: Q1 (*(s32 *)t0 staged into a local before the call), Q2 (D_800A11DC[D_800A11D5] staged), Q3 (both staged), Q4 (t0 address computed after the arg5 load), Q5 (t0 deref staged before the arg5 load), Q6 (subscript staged first), Q7 (subscript staged between).
- result: Q1 6, Q2 13, Q3 11 (bi 159, wrong length), Q4 6, Q5 7, Q6 13, Q7 14. Q4 quantity table is byte-for-byte C1 (qty1 reg112 18-24 refs2 got=3, qty2 reg106 20-26 refs2 got=4): moving the t0 address statement across the arg5 load in C changes NOTHING in the post-sched1 stream. Q1 does reach got=3 for reg106 but only by restructuring the whole t0 chain (11 instruction diffs, 6 masked points). No argument-order spelling moved qty2 death.
- verdict: KILLED
- kill_scope: instance
- measured_on: C1 order-exact pp-free CD_alarm-struct chassis, control 6/160 bi 160 rd 0; FAKE state = chain-extender only, pp deleted

## [s119] Carrying the do_timeout t0 ADDRESS in an existing function-wide local exiles that pseudo from local-alloc to global-alloc, leaving the arg5 value as the sole contender for $v1 - and it wins the target seat with refs=2, no wrap and no new FAKE.
- mechanism: local-alloc only allocates pseudos confined to one basic block. A variable also referenced outside do_timeout has a multi-block live range, so its pseudo never enters block 3 quantity list; block 3 then has three quantities instead of four and qty_compare_1 is no longer a 3333-vs-3333 tie between the t0 temp and the arg5 value. The arg5 value is allocated after the D_800A11D5 index quantity and takes the lowest free hard register, $v1(3).
- probe: nine borrow candidates x two statement orders on the C1 order-exact pp-free CD_alarm-struct chassis, scored with sandbox --disable all, with BB2_QTY_DEBUG tables for the winners (tmp/grind/CD_sync/s119/S2/qty.txt): status, temp, cnt, i, v0, new_var, dst, src, saved.
- result: S2 (status, C1 statement order) 5/160 with "QTYDBG blk=3 ord=2 qty=1 reg1=106 birth=20 death=26 refs=2 got=3" - THREE quantities in block 3 and the arg5 value in $v1, the target seat, on an order-exact emission with zero extra references and zero new constructs. T1 (status, ix-chain-first order) and T2 (status, literal target order) both score 4/160 and their residual is FIVE instructions that are ALL THE SAME ONE REGISTER: target lbu $a0,0($s2) / sll $a0,$a0,2 / addu $a0,$a0,$s3 / lw $a3,0($a0) at indices 49/55/59/65 against ours in $s0. Every other instruction of all 160 matches, including every register and the complete emission order. First form in this ledger that is simultaneously order-exact AND arg5-seat-exact.
- verdict: CONFIRMED

## [s119] Some existing local, borrowed for the t0 address, gets $a0 from global-alloc and closes the function.
- mechanism: MIPS defines no REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, so global.c find_reg walks hard registers in ascending number and would take $a0(4) as soon as $v0/$v1 are taken - unless the allocno conflicts with $a0.
- probe: all nine borrow candidates measured above, plus the ix-chain-first and target-order statement variants for the five s32 candidates (U1/U2 series).
- result: KILLED. Every borrow lands on exactly two outcomes and neither is $a0. status (live across the getintr callback calls) gets the callee-saved $s0: T1/T2 4/160, five diffs, all $s0-for-$a0. v0 and cnt (crossing no call) get the caller-saved $a3: U1_v0 7/160 with the identical five-diff shape in $a3, U1_cnt 8/160 (same shape plus two diffs from cnt own uses at indices 38/42). temp 10, i 16, dst 15, src 15, saved 16 (bi 162), new_var 37 (bi 162). Mechanism: the borrowed variable live range is function-wide, so it overlaps every point where $a0/$a1/$a2 are set for the other calls in the function (VSync, the two callbacks, printf) - a conflict $a3 does not have, since $a3 is written exactly once in the whole function, at index 65, which is the allocno own death.
- verdict: KILLED
- kill_scope: instance
- measured_on: C1 order-exact pp-free CD_alarm-struct chassis with the ix-first / target statement orders, chain-extender FAKE present and pp deleted; controls C1 6/160 and T1 4/160, all forms bi 160 rd 0 except saved/new_var (bi 162)

## [s119] N1 (the best do-while(0) wrap form, 5/160) already has the arg5 value in $v1 and the t0 scale temp in $a0, so its residual is code motion rather than allocation.
- mechanism: s118 frontier item 3: if reg106 is already got=3 on N1, the NOTE_INSN_LOOP_BEG pair is the only remaining cost.
- probe: BB2_QTY_DEBUG + BB2_SUGG_DEBUG on the single-function mini TU for N1 via new tooling tmp/grind/CD_sync/s119/qd.sh.
- result: QTYDBG blk=3 ord=2 qty=2 reg1=106 birth=18 death=26 refs=4 got=3 and ord=3 qty=1 reg1=112 birth=16 death=24 refs=3 got=4 - reg106 in $v1, reg112 in $a0, the target seats. Confirmed: all 5 points are code motion.
- verdict: CONFIRMED

## [s119] A local-alloc quantity's reference count is the SUM of reg_n_refs over every pseudo tied into it, and block_alloc ties operand 0 with ANY dying pseudo operand, not only with register-to-register copies.
- mechanism: local-alloc.c:1932 qty_n_refs[sqty] += reg_n_refs[sreg] inside combine_regs; block_alloc calls combine_regs at local-alloc.c:1295/1331/1336/1346 for every operand that dies in an insn whose operand 0 has an = constraint and is not earlyclobber.
- probe: Read local-alloc.c:1190-1300 and 1890-1945 and cross-checked against the banked s117 quantity tables.
- result: Confirmed by construction and by the table: A5's qty0 reports refs=6 for reg113, a two-reference plus temp, because reg108 (4 refs) dies into it, matching C1's single-pseudo reg108 refs=6. Corollary: a LOAD cannot tie (its source is a MEM), so the arg5 pseudo cannot inherit refs from the address pseudo dying at its load.
- verdict: CONFIRMED

## [s119] An explicit second local carrying the arg5 value (arg5b = arg5) survives cse and gives local-alloc a copy to tie, lifting the arg5 quantity's reference count.
- mechanism: local-alloc.c:1932 sums refs across a tie, so a surviving copy would give the arg5 quantity 2+2 refs and priority 13333 against the t0 temp's 3333.
- probe: R5_copy_local scored with sandbox --disable all and dumped with BB2_QTY_DEBUG (tmp/grind/CD_sync/s119/R5/qty.txt).
- result: 6/160, byte-identical to the C1 control, quantity table identical (reg106 birth 20 death 26 refs 2 got=4). cse propagates the copy away before flow counts references.
- verdict: KILLED
- kill_scope: instance
- measured_on: C1 order-exact pp-free CD_alarm-struct chassis (memory/grind/CD_sync/rejected/s117_C1_struct_h1_ix_addr_split.c), control 6/160 bi 160 rd 0; FAKE state = chain-extender only, pp deleted

## [s119] Re-ordering printf's argument evaluation (staging the 3rd or 4th argument into a local, or moving the t0 address computation across the arg5 load) shrinks the arg5 quantity's span to 4 or less, s117 closing condition (c).
- mechanism: s118 frontier item 2: qty2 dies at its store into the outgoing-argument slot, whose position is set by the order expand emits the call's argument setup.
- probe: Seven spellings Q1-Q7 on the C1 chassis, scored with sandbox --disable all, with BB2_QTY_DEBUG tables for Q1/Q4/Q5.
- result: Q1 6, Q2 13, Q3 11 (bi 159), Q4 6, Q5 7, Q6 13, Q7 14. Q4's quantity table is byte-for-byte C1's, so moving the t0 address statement across the arg5 load in C changes nothing in the post-sched1 stream; no spelling moved qty2's death from luid 26.
- verdict: KILLED
- kill_scope: instance
- measured_on: C1 order-exact pp-free CD_alarm-struct chassis, control 6/160 bi 160 rd 0; FAKE state = chain-extender only, pp deleted

## [s119] Carrying the do_timeout t0 address in an existing function-wide local exiles that pseudo from local-alloc to global-alloc, removes the 3333-vs-3333 qty_compare_1 tie and seats printf's 5th argument in $v1 with refs=2, no wrap and no new construct.
- mechanism: local-alloc only allocates pseudos confined to one basic block, so a variable also referenced outside do_timeout never enters block 3's quantity list; block 3 drops from four quantities to three and the arg5 value, allocated after the D_800A11D5 index quantity, takes the lowest free hard register $v1(3).
- probe: Nine borrow candidates crossed with three statement orders on the C1 order-exact pp-free CD_alarm-struct chassis, scored with sandbox --disable all, with BB2_QTY_DEBUG for S2 and normalised target diffs for S2/T1/T2 via tmp/grind/CD_sync/s119/d.sh.
- result: S2 (borrow status, C1 order) 5/160 with QTYDBG blk=3 ord=2 qty=1 reg1=106 birth=20 death=26 refs=2 got=3 - three quantities and the arg5 value in the target seat. T1 (ix-chain-first) and T2 (literal target statement order) both 4/160 bi 160 rd 0, and their residual is five instructions that are all the same one register: target lbu $a0,0($s2) / sll $a0,$a0,2 / addu $a0,$a0,$s3 / lw $a3,0($a0) at indices 49/55/59/65 against ours in $s0. All 160 instructions otherwise match, registers and emission order included. First form in this ledger that is simultaneously order-exact and arg5-seat-exact.
- verdict: CONFIRMED

## [s119] One of the nine existing locals, borrowed for the t0 address, gets $a0 from global-alloc on the T1/T2 chassis.
- mechanism: MIPS defines no REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, so global.c's find_reg walks hard registers ascending and would take $a0(4) once $v0/$v1 are taken, unless the allocno conflicts with $a0.
- probe: All nine borrow candidates (status, temp, cnt, i, v0, new_var, dst, src, saved) plus ix-first and target-order statement variants for the five s32 candidates (S and U series), scored with sandbox --disable all and diffed against the target.
- result: Every borrow lands on one of two outcomes and neither is $a0: status gets callee-saved $s0 (T1/T2 4/160, five diffs all $s0-for-$a0) because it is live across the getintr callback calls; v0 and cnt get caller-saved $a3 (U1_v0 7/160 with the identical five-diff shape, U1_cnt 8/160 plus two diffs from cnt's own uses at indices 38/42); temp 10, i 16, dst 15, src 15, saved 16 (bi 162), new_var 37 (bi 162). The borrowed variable's live range is function-wide, so it overlaps every point where $a0/$a1/$a2 are set for VSync, the two callbacks and printf's own argument setup - a conflict $a3 escapes because $a3 is written exactly once in the whole function, at index 65, which is the allocno's own death.
- verdict: KILLED
- kill_scope: instance
- measured_on: C1 order-exact pp-free CD_alarm-struct chassis with the C1 / ix-first / target statement orders, chain-extender FAKE present and pp deleted; controls C1 6/160 and T1 4/160; all forms bi 160 rd 0 except saved and new_var (bi 162)

## s120 (rederive)

### H120-1 KILLED (instance) — borrowing a PARAMETER for the do_timeout t0 address
Statement: on the pp-free CD_alarm-struct chassis, carrying the do_timeout t0
address in parameter `a0` or parameter `a1` seats the t0 chain in $a0.
Measured: P1_a0_ixfirst 22/160, P1_a0_tgtorder 22/160, P2_a1_ixfirst 25/160,
P2_a1_tgtorder 25/160, all bi 160 rd 0, chain-extender FAKE present, pp absent.
Both parameters are live across the polling loop's calls, so the borrowed pseudo
is a call-crossing global allocno; the seat is callee-saved and the incoming
argument register has to be shuffled. This completes the borrow enumeration
H119-6 opened (nine locals + two parameters).

### H120-2 KILLED (instance) — exiling a block-3 quantity OTHER than t0
Statement: exiling the ix chain / the arg5 value / the arg3 subscript / the arg4
deref into the function-wide local `status` (the mirror of T1, keeping the t0
chain block-local so local-alloc can seat it in $a0) reaches the target seats
and the target order together.
Measured on the C1 order-exact pp-free CD_alarm-struct chassis, chain-extender
FAKE present: E1 (ix) 4/160, E2 (arg5) 6, E3 (arg3) 17, E4 (arg4) 12,
E5 (ix+arg5) 9, E1b (ix, ix-first order) 13. E1's objdump diff confirms the
t0 chain does keep $a0 when it stays block-local, but the exiled ix chain takes
$s0 where the target has $v0 and the 54/55 transposition is unchanged.

### H120-3 CONFIRMED — the order-vs-seat conflict is ONE binary variable
Statement: whether the ix add (`ix += (s32)tbl_125c`) is a sched1 insn or lives
inside the load's MEM decides BOTH the 54/55 emission order and the local-alloc
seat assignment, and the two outcomes are mutually exclusive on every source
order measured.
Mechanism: `qty_compare_1` (tools/gcc-2.7.2/local-alloc.c:1660) is
floor_log2(refs)*refs*size/(death-birth)*10000 with a qty-number tiebreak.
Folded ix -> t0 span 8 (pri 10000) vs arg5 span 6 (13333) -> arg5 first -> $v1,
t0 -> $a0 (target seats) but the reload-materialised `addu` lands after the t0
`sll` (the 54/55 transposition, score 2). Split ix -> the `addu` is scheduled at
slot 8 (target order) and pushes the t0 `sll` to slot 9 -> both spans 6, tie,
qty-number tiebreak gives t0 $v1 and arg5 $a0 (score 6).
Measured: 7 folded orders (candidate, F1, F4, F5, F8 = 2/160 with the identical
two-instruction 54/55 diff; F3, F6, F7 = 4/160 with the two lbu's additionally
swapped) all report t0 16/24, arg5 20/26; 8 split orders (V1, V3, W1..W5, G1,
G2) all report t0 18/24, arg5 20/26.

### H120-4 CONFIRMED — the closing predicate is a sched1 ORDER predicate, not a refs lever
Statement: on the split (order-exact) chassis the target seating follows from
refs 2/2 alone provided sched1 emits `sw arg5,16(sp)` before `lw a3,0(t0)`;
this supersedes s119 frontier item 2's call for a refs>=3 chain-extender FAKE.
Mechanism: the target's own final order has the arg5 stack store (index 61)
before the t0 deref (index 65). If sched1 produced that relative order the spans
would be t0 18..26 (pri 10000) and arg5 20..24 (pri 20000) — arg5 allocated
first, taking $v1, t0 taking $a0, which is exactly the target, with no tie to
break and no construct added. Every split form measured schedules `lw a3` at
slot 12 and `sw` at slot 13 instead, because INSN_PRIORITY(`lw a3`)=2 (result
feeds the call) beats INSN_PRIORITY(`sw`)=1 and both are ready together; sched2
then swaps them back (s119 already showed sched2 reorders this block), which is
why V1's FINAL order is exact while its allocation is not.
Next probe: tools/sched_solver on V1's sched1 stream — ask whether any
C-reachable change delays `addu a0,a0,s3` (the t0 add) enough that `lw a3` is
not ready at the cycle `sw` becomes ready, or otherwise reverses that pair,
while leaving the 53/54/55/56 prefix alone. Accept only bi 160 rd 0 with
QTYDBG blk=3 showing reg106 death < reg112 death.

### H120-5 KILLED (instance) — natural-C rederives of the do_timeout block
Statement: writing the two table accesses as ordinary array subscripts
(`tbl_125c[idx_1494[1]]`, `(s32)&tbl_125c[idx_1494[0]]`) or inlining the printf
argument expressions reproduces the target block.
Measured on the pp-free CD_alarm-struct chassis, chain-extender FAKE present:
X1 9/160, X2 9, X3 14, Y1 10, Y2 14, Y3 10, Y4 14, Z1 14, Z2 13, Z3 13,
Z4 5, all bi 160 rd 0. The hand-derived pointer spelling in candidate.c is the
best available spelling of this block, not an inherited stylistic accident.

## [s120] Carrying the do_timeout t0 address in parameter a0 (mode) or parameter a1 (result) seats the t0 chain in $a0 on the pp-free CD_alarm-struct chassis.
- mechanism: global.c find_reg is an ascending hard-register walk on MIPS (no REG_ALLOC_ORDER); an incoming-argument pseudo carries a strong $a0/$a1 preference from the entry copy, so borrowing a parameter was the one branch of the s119 borrow enumeration never run.
- probe: P1_a0_ixfirst / P1_a0_tgtorder / P2_a1_ixfirst / P2_a1_tgtorder via tmp/grind/CD_sync/s120/b.sh (sandbox CD_sync --disable all).
- result: 22/160, 22/160, 25/160, 25/160, all bi 160 rd 0. Both parameters are live across the polling loop's VSync/getintr/callback calls, so the borrowed pseudo becomes a call-crossing global allocno with a callee-saved seat plus argument-shuffle churn. Together with H119-6's nine locals this closes the borrow enumeration.
- verdict: KILLED
- kill_scope: instance
- measured_on: pp-free CD_alarm-struct chassis (memory/grind/CD_sync/candidate.c body), chain-extender FAKE present, pp pointer alias absent; controls 2/160 (candidate) and 4/160 (T1)

## [s120] Exiling one of block 3's OTHER quantities (the ix chain, the arg5 value, the arg3 subscript, the arg4 deref) into the function-wide local status, keeping the t0 chain block-local, reaches the target seats and the target emission order together.
- mechanism: s119 showed exiling a pseudo to a function-wide local removes it from local-alloc's block quantity set; the mirror move should leave t0 block-local (hence $a0 from local-alloc, as the target has) while removing the competing quantity.
- probe: E1_ix_status / E2_arg5_status / E3_arg3_status / E4_arg4_status / E5_ixarg5_status / E1b_ix_status_ixfirst, scored and objdump-diffed against asm/funcs/CD_sync.s.
- result: 4, 6, 17, 12, 9, 13 /160, all bi 160 rd 0. E1 confirms the premise (t0 keeps $a0 when block-local) but the exiled ix chain lands in $s0 where the target has $v0 and the 54/55 transposition is unchanged, so every mirror form is worse than the 2/160 candidate.
- verdict: KILLED
- kill_scope: instance
- measured_on: pp-free CD_alarm-struct chassis, chain-extender FAKE present, pp absent; control 2/160

## [s120] Whether the ix add is written as its own statement or folded into the load's address decides BOTH the 54/55 emission order and the local-alloc seat assignment, and on the fifteen source orders measured the two outcomes are mutually exclusive.
- mechanism: qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1660) ranks by floor_log2(refs)*refs*size/(death-birth)*10000 with a qty-number tiebreak. Folded: the add only materialises during reload, t0's sll takes sched1 slot 8, t0 span 8 (pri 10000) < arg5 span 6 (13333), arg5 is allocated first and takes $v1, t0 $a0 (target seats) but the reload-emitted addu lands after the sll. Split: the addu is a sched1 insn placed at slot 8 (target order), t0's sll is pushed to slot 9, both spans are 6, the tie falls to qty number and t0 (born first) takes $v1.
- probe: BB2_QTY_DEBUG blk=3 tables via tmp/grind/CD_sync/s120/qd.sh on 7 folded orders (candidate, F1, F3, F4, F5, F6, F7, F8) and 8 split orders (V1, V3, W1-W5, G1, G2), each also scored and objdump-diffed.
- result: Every folded order reports t0 16/24 and arg5 20/26 and scores 2/160 with the identical two-instruction 54/55 diff (F3/F6/F7 4/160, the two lbu's additionally swapped). Every split order reports t0 18/24 and arg5 20/26 and scores 6-7/160 with a pure $a0<->$v1 swap of the t0 chain and the arg5 value on an otherwise instruction-exact and order-exact 160.
- verdict: CONFIRMED

## [s120] On the split (order-exact) chassis the target seating follows from refs 2/2 alone provided sched1 emits the arg5 stack store before the t0 deref load.
- mechanism: The target's own final output has the arg5 stack store at index 61 and the t0 deref at index 65. With that relative order in the sched1 stream the spans become t0 18..26 (pri 10000) and arg5 20..24 (pri 20000): arg5 is allocated first and takes $v1, t0 takes $a0, with no tie and no added construct. Every split form instead schedules the lw a3 at slot 12 and the sw at slot 13 because INSN_PRIORITY(lw a3)=2 (its result feeds the call) beats INSN_PRIORITY(sw)=1 while both are ready at the same cycle, and sched2 then swaps them back - which is exactly why V1's FINAL order is target-exact while its allocation is not.
- probe: Cross-read of the BB2_QTY_DEBUG spans against the target's own index order in asm/funcs/CD_sync.s, plus s119's measured proof that sched2 reorders this block.
- result: Supersedes s119 frontier item 2. The closing lever is a sched1 ordering perturbation on the split chassis, reachable in ordinary C if the t0 add can be delayed past the point where the sw becomes ready; it does not require lifting reg106's reference count with a second chain-extender FAKE.
- verdict: CONFIRMED

## [s120] Natural-C rederives of the do_timeout block - ordinary array subscripts or fully inlined printf argument expressions - reproduce the target block better than the hand-derived pointer spelling.
- mechanism: The rederive modality's premise: a structurally different, more idiomatic C shape may hit a different expand/combine path for the two table accesses.
- probe: X1/X2/X3 (subscripts), Y1-Y4 (inlined arguments), Z1-Z4 (staged arg3/arg4) on the pp-free chassis.
- result: 9, 9, 14, 10, 14, 10, 14, 14, 13, 13, 5 /160, all bi 160 rd 0, against a 2/160 control. The hand-derived pointer spelling in candidate.c is the best available spelling of this block.
- verdict: KILLED
- kill_scope: instance
- measured_on: pp-free CD_alarm-struct chassis, chain-extender FAKE present, pp absent; control 2/160

## s121 (rederive)

### Harness note — the solver suite now runs on CD_sync (object mode)
Both solvers work on this function in OBJECT mode, which is the supported path
for an INCLUDE_ASM-routed target (owner ruling 2026-08-25):

    bash tmp/grind/CD_sync/s121/b.sh <form.c>              # apply + sandbox
    python3 tools/sched_solver/extract.py system           # parity=True
    bash    tools/sched_solver/mkasm.sh system
    python3 tools/sched_solver/perturb.py tmp/sched_solver_work/system.sched.json \
        --func CD_sync --pass {1,2} --goal-from-target system \
        --target-object build/src/system.o \
        --ours-object  tmp/sandbox/CD_sync/system.o --atoms luid,luid_move --depth 1
    python3 tools/ra_solver/inverse_compose.py classify system CD_sync \
        --target-object build/src/system.o --ours-object tmp/sandbox/CD_sync/system.o
    python3 tools/ra_solver/inverse.py local tmp/ra_solver_work/system.local.json \
        --func CD_sync --block 3 --swap 1,2 --depth 2

WARNING for future sessions: goalmap.py's CLI flag `--target asm/funcs/CD_sync.s`
DOES NOT WORK. `asm_body()` skips every line beginning with `/*`, which is every
splat body line, so the target parses as ONE instruction and every block then
silently reports "GOAL == OURS (identity)". That is a false negative, not a
result — it cost the first third of this session. Object mode aligns 160-vs-160
with 159 equal / 1 moved. `tmp/grind/CD_sync/s121/show.py` prints the per-slot
ours-vs-target listing with texts, luids and priorities, plus the block-3
dependence graph.

### H121-1 CONFIRMED — the 2/160 residual named at RTL-UID level
Statement: on the folded candidate chassis the whole residual is one adjacent
transposition in block 3, present identically in sched1 AND sched2: ours emits
uid 120 (the t0 chain's `sll`, luid 7) then uid 130 (the ix chain's
reload-materialised `addu`, luid 11); the target emits 130 then 120.
Mechanism: both are pri 2 and become ready together; `rank_for_schedule` falls
through to INSN_LUID descending, and our source order gives luid(120) <
luid(130). The dependence graph from the extracted model settles the
identification that s118-s120 had to infer: chain A = 112 (`lbu` Intr.sync)
-> 120 -> 125 -> 157 (`lw a3`), chain B = 116 (`lbu` Intr.ready) -> 127 -> 130
-> 132 (`lw`) -> 149 (`sw ...,16(sp)`). (The uid->asm-text column of show.py
mislabels 120 and 130 precisely BECAUSE they are the transposed pair; read the
deps, not the text.)
Probe: perturb.py, both passes, atoms luid+luid_move, depth 1.
Result: pass 2 = 36 single atoms reach the goal, pass 1 = 31; the intersection
is 26. Every winning atom either raises luid(120) above luid(130) or lowers
luid(130) below luid(120) — i.e. the order fix is an ORDINARY SOURCE STATEMENT
MOVE, with no construct of any kind.
- verdict: CONFIRMED

### H121-2 CONFIRMED — the ORDER half of the order-vs-seat equation is SOLVED
Statement: `V2_ixfirst_folded` (the s120 form that puts the whole ix group
before `t0 *= 4`, banked then as a bare "6/160" with no order analysis) is
ORDER-EXACT in BOTH sched1 and sched2 for block 3, and its entire residual is
register allocation.
Probe: apply V2, re-extract, `show.py` (PASS 1 same True / PASS 2 same True for
block 3), then `inverse_compose.py classify`.
Result: classify prints `FIRST DIVERGENCE: RA — same instructions, different
registers`, six pairs: ours `addu v1,v1,s3 / lbu v1,0(s2) / lw a0,0(v0) /
lw a3,0(v1) / sll v1,v1,0x2 / sw a0,16(sp)` vs target `addu a0,a0,s3 /
lbu a0,0(s2) / lw a3,0(a0) / lw v1,0(v0) / sll a0,a0,0x2 / sw v1,16(sp)`.
BB2_QTY_DEBUG on V2, blk=3: qty=1 reg113 birth=18 death=24 refs=2 got=3 ($v1),
qty=2 reg106 birth=20 death=26 refs=2 got=4 ($a0); the target wants the
exchange. Both spans 6, both refs 2 -> `qty_compare_1`
(tools/gcc-2.7.2/local-alloc.c:1660) ties and the qty-number (birth-order)
tiebreak hands $v1 to the t0-address quantity.
`fake_ablate.py` on V2: 6 with the chain-extender, 19 without — the 6 is NOT a
FAKE-carrier artifact. (Control: candidate 2 with, 15 without.)
Banked as memory/grind/CD_sync/progress/s121-V2-order-exact-RA-only-6.c.
- verdict: CONFIRMED

### H121-3 KILLED (instance) — s119/s120 frontier item 1's identification of the sched1-vs-sched2 pair
Statement: the insn pair that sched1 and sched2 order differently in block 3 is
`lw a3,0(t0)` versus `sw arg5,16(sp)`, so making sched1 emit the store before
the load is the closing lever.
Measured on the split (V1_ixfirst_nolocalborrow) chassis, control 6/160 bi 160
rd 0, chain-extender FAKE present, pp absent, from the extracted sched model:
sched1 emits ... 141, 130, **150, 146**, 156, 158 ... and sched2 emits
... 141, 130, **146, 150**, 156, 158 ... uid 150 IS `sw reg106,16(sp)` but uid
146 is `sll reg119 = reg116 << 2` — the scale of the D_800A11D5 index for
printf's THIRD argument — and the `lw a3` is uid 158, which does not move
between the passes at all. The predicate "emit the sw before the lw a3" is
therefore not the divergence, and H120-4's INSN_PRIORITY(lw)=2-beats-
INSN_PRIORITY(sw)=1 story does not describe this block.
- verdict: KILLED
- kill_scope: instance
- measured_on: split V1 chassis (tmp/grind/CD_sync/s120/forms/V1_ixfirst_nolocalborrow.c), chain-extender FAKE present, pp absent, control 6/160 bi 160 rd 0; sched model tmp/sched_solver_work/system.sched.json, parity=True, simulate 52/52 order-exact and clock-exact

### H121-4 CONFIRMED — the remaining residual is a typed, REACHABLE local-alloc question
Statement: `inverse.py local --func CD_sync --block 3 --swap 1,2` returns a
REACHABLE verdict at minimal solution size 1 atom with 21 distinct vectors.
Baseline `{0:$v0, 1:$v1, 2:$a0, 3:$v0}`, goal `qty 1: $v1 -> $a0, qty 2:
$a0 -> $v1`. Ranked classes: #1 `refs_down qty 1: refs 2->1`; #2-#5
`live_extend qty 1` (dies later 24->25 / 24->26, born earlier 18->17 / 18->16);
#8 `refs_up qty 2: refs 2->3`. The tool's own LOCAL-MODE CAVEAT applies: a
birth/span vector is a claim about ALLOC-time order, which is not known to
equal emission order, so each is NECESSARY not SUFFICIENT until re-derived from
a QTYDBG dump of the spelled candidate. Full listing:
tmp/grind/CD_sync/s121/inverse_local.txt.
- verdict: CONFIRMED

### H121-5 KILLED (instance) — the #1 ranked vector (refs_down on the t0 address)
Statement: dropping the t0-address quantity to one reference by folding the add
into printf's 4th-argument MEM (`*(s32 *)(t0 + (s32)tbl_125c)`) removes it from
block 3's contended quantity set and leaves the arg5 value to take $v1.
Measured on the V2 order-exact chassis (control 6/160), chain-extender FAKE
present, pp absent: M1 (ix folded, t0 folded, ix-first) 14/160, M2 (same,
t0-first) 14, M3 (ix split-add, t0 folded, ix-first) 14, M4 (same, t0-first)
14, M5 (t0 fully inlined including the scale) 14, M6 (t0 folded, candidate
statement order) 14 — all bi 160 rd 0. The target's own bytes contain a
2-reference t0 address (`addu $a0,$a0,$s3` then `lw $a3,0($a0)`), so refs_down
is a model vector with no faithful spelling here: removing the reference
removes the instruction.
- verdict: KILLED
- kill_scope: instance
- measured_on: V2 order-exact chassis (memory/grind/CD_sync/progress/s121-V2-order-exact-RA-only-6.c), chain-extender FAKE present, pp absent; control 6/160 bi 160 rd 0

### H121-6 KILLED (instance) — declaration order, pointer-locals and full inlining on the order-exact chassis
Statement: on the V2 order-exact chassis the local-alloc tie is broken by the
block's declaration order, by giving the arg5 address its own pointer local, by
reading Intr.sync after the ix group, or by writing the whole call in the
upstream SOTN bios.c fully-inlined shape.
Measured on the V2 chassis, chain-extender FAKE present, pp absent, control
6/160: N3 (V2 control, arg5/t0/ix decl order) 6, N4 (t0/ix/arg5 decl order) 6,
N6 (arg5 address in an `s32 *ap` local) 6, N5 (Intr.sync read moved after the
ix group) 7, N1 (`printf(fmt, D_800F19B8.func, D_800A11DC[D_800A11D5],
tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]])`, no locals at all) 14, N2 (arg5
staged, arg4 inlined) 14 — all bi 160 rd 0. Declaration order is inert for the
qty tiebreak here, and the fully-inlined upstream shape loses the order as well
as the seats.
- verdict: KILLED
- kill_scope: instance
- measured_on: V2 order-exact chassis, chain-extender FAKE present, pp absent; control 6/160 bi 160 rd 0

### H121-7 CONFIRMED — mandated kill re-audit: the chain-extender FAKE is load-bearing on BOTH chassis
Probe: `tools/fake_ablate.py --func CD_sync --file system --candidate <form>`.
Result: candidate.c 2/160 bi 160 with the FAKE, 15/159 without; V2 6/160 with,
19/159 without. Neither chassis's score is a FAKE-carrier artifact.
- verdict: CONFIRMED

## [s121] On the folded candidate chassis the entire 2/160 residual is one adjacent transposition in block 3, present identically in sched1 and sched2: ours emits uid 120 (the t0 chain's sll, from `t0 *= 4;`) then uid 130 (the ix chain's reload-materialised addu, from the folded `*(s32 *)(ix + (s32)tbl_125c)`), while the target emits 130 then 120.
- mechanism: Both insns carry INSN_PRIORITY 2 and become ready in the same cycle, so sched.c's rank_for_schedule falls through past the priority and dependence-class keys to INSN_LUID descending. LUID is source statement order, so the pair's emission order is decided by where the two statements sit in the C. The dependence graph extracted from the instrumented cc1 settles the identification s118-s120 had to infer: chain A = 112 (lbu Intr.sync) -> 120 -> 125 -> 157 (lw a3), chain B = 116 (lbu Intr.ready) -> 127 -> 130 -> 132 (lw) -> 149 (sw ...,16(sp)).
- probe: tools/sched_solver/extract.py system (parity=True; simulate.py --func CD_sync = 52/52 blocks order-exact AND clock-exact), then perturb.py --func CD_sync --pass 1 and --pass 2 --goal-from-target system --target-object build/src/system.o --ours-object tmp/sandbox/CD_sync/system.o --atoms luid,luid_move --depth 1; per-slot listing via tmp/grind/CD_sync/s121/show.py.
- result: 36 single luid atoms reach the pass-2 goal, 31 reach the pass-1 goal, intersection 26. Every winning atom is a pure source-statement move (raise luid(120) above luid(130), or lower luid(130) below luid(120)); no construct of any kind is implicated. Also recorded: goalmap.py's CLI --target asm/funcs/CD_sync.s is a FALSE NEGATIVE generator (asm_body skips every /*-prefixed line, so the target parses as ONE instruction and every block reports GOAL == OURS); object mode is the only correct path and aligns 160-vs-160 with 159 equal / 1 moved.
- verdict: CONFIRMED

## [s121] V2_ixfirst_folded - the s120 form that moves the whole ix group ahead of `t0 *= 4;`, banked then as a bare 6/160 with no order analysis - is order-exact in BOTH sched1 and sched2 for block 3, and its entire remaining residual is register allocation.
- mechanism: Moving the arg5 statement group earlier lowers luid(130) below luid(120), which is exactly the perturb.py atom class that reaches the goal in both passes; the emission order then equals the target's for all 20 insns of the block and for all 160 of the function.
- probe: Apply V2, re-run extract.py + mkasm.sh, read tmp/grind/CD_sync/s121/show.py (PASS 1 same True / PASS 2 same True for block 3), then tools/ra_solver/inverse_compose.py classify system CD_sync --target-object build/src/system.o --ours-object tmp/sandbox/CD_sync/system.o; BB2_QTY_DEBUG via tmp/grind/CD_sync/s121/qd.sh; tools/fake_ablate.py for the FAKE re-audit.
- result: classify prints FIRST DIVERGENCE: RA - same instructions, different registers, with exactly six pairs (ours addu v1,v1,s3 / lbu v1,0(s2) / lw a0,0(v0) / lw a3,0(v1) / sll v1,v1,0x2 / sw a0,16(sp) against target addu a0,a0,s3 / lbu a0,0(s2) / lw a3,0(a0) / lw v1,0(v0) / sll a0,a0,0x2 / sw v1,16(sp)). QTYDBG blk=3: qty=1 reg113 (t0 address) birth 18 death 24 refs 2 got $v1, qty=2 reg106 (arg5 value) birth 20 death 26 refs 2 got $a0; the target wants the exchange. Equal refs and equal spans tie qty_compare_1 and the qty-number birth-order tiebreak decides against us. fake_ablate: V2 is 6 with the chain-extender and 19 without, so the 6 is not a FAKE-carrier artifact (control: candidate 2 with, 15 without). This retires the s115-s120 framing in which order and seats were two outcomes of one binary variable - a form now exists that has the order and not the seats. Banked as memory/grind/CD_sync/progress/s121-V2-order-exact-RA-only-6.c.
- verdict: CONFIRMED

## [s121] The block-3 insn pair that sched1 and sched2 order differently is `lw a3,0(t0)` versus `sw arg5,16(sp)`, so the closing lever is a perturbation that makes sched1 emit the store before the load.
- mechanism: s119/s120 frontier item 1 held that INSN_PRIORITY(lw a3)=2 beats INSN_PRIORITY(sw)=1 while both are ready together, so sched1 always takes the load first and sched2 swaps them back.
- probe: Read the extracted pass-1 and pass-2 pick streams for block 3 on the split V1 chassis directly out of tmp/sched_solver_work/system.sched.json (tmp/grind/CD_sync/s121/blk.py), then name each uid from the .sched RTL dump.
- result: The passes do differ by exactly one transposition, but it is uid 150 against uid 146: sched1 emits ... 141, 130, 150, 146, 156, 158 ... and sched2 emits ... 141, 130, 146, 150, 156, 158 ... uid 150 is indeed `sw reg106,16(sp)`, but uid 146 is `sll reg119 = reg116 << 2` - the scale of the D_800A11D5 index for printf's THIRD argument. The `lw a3` is uid 158 and does not move between the passes at all. The named predicate therefore does not describe this block, and the INSN_PRIORITY story attached to it does not hold.
- verdict: KILLED
- kill_scope: instance
- measured_on: split V1 chassis (tmp/grind/CD_sync/s120/forms/V1_ixfirst_nolocalborrow.c), chain-extender FAKE present, pp pointer alias absent, control 6/160 bi 160 rd 0; sched model parity=True, simulate 52/52 order-exact and clock-exact

## [s121] The local-alloc tie on the order-exact V2 chassis is a typed, reachable question: inverse.py local --func CD_sync --block 3 --swap 1,2 returns a solution at minimal size 1 atom.
- mechanism: local-alloc.c qty_compare_1 ranks by floor_log2(refs)*refs*size/(death-birth)*10000 with a qty-number tiebreak; with both contended quantities at refs 2 and span 6 the ranking is a pure tie and any single perturbation of refs or of birth/death on either quantity resolves it.
- probe: tools/ra_solver/local_extract.py system, then inverse.py local tmp/ra_solver_work/system.local.json --func CD_sync --block 3 --swap 1,2 --depth 2 --top 8 (output banked at tmp/grind/CD_sync/s121/inverse_local.txt).
- result: REACHABLE, 21 distinct 1-atom vectors from baseline {0:$v0, 1:$v1, 2:$a0, 3:$v0} to goal qty1 $v1->$a0 / qty2 $a0->$v1, in three classes: refs_down on qty 1 (rank #1), live_extend on qty 1 (ranks #2-#5: dies later 24->25 or 24->26, born earlier 18->17 or 18->16), refs_up on qty 2 (rank #8). The tool's own LOCAL-MODE CAVEAT applies - a birth/span vector is a claim about alloc-time order, not emission order, so each vector is necessary but not sufficient until re-derived from a QTYDBG dump of the spelled candidate.
- verdict: CONFIRMED

## [s121] The top-ranked vector, dropping the t0-address quantity to one reference by folding its add into printf's 4th-argument MEM (`*(s32 *)(t0 + (s32)tbl_125c)`), removes it from block 3's contended quantity set and leaves the arg5 value to take $v1.
- mechanism: A folded address materialises only during reload, so the pseudo never enters local-alloc's block quantity list - the same mechanism that makes the ix chain's add invisible on the current candidate chassis.
- probe: Six spellings measured with sandbox --disable all on the V2 order-exact chassis: M1 (ix folded + t0 folded, ix-first), M2 (same, t0-first), M3 (ix split-add + t0 folded, ix-first), M4 (same, t0-first), M5 (t0 fully inlined including the scale), M6 (t0 folded, candidate statement order). Forms in tmp/grind/CD_sync/s121/forms/.
- result: 14, 14, 14, 14, 14, 14 /160, all bi 160 rd 0, against a 6/160 control. The target's own bytes contain a two-reference t0 address (addu $a0,$a0,$s3 then lw $a3,0($a0)), so on this function refs_down has no faithful spelling: removing the reference removes the instruction. The vector is a model artifact.
- verdict: KILLED
- kill_scope: instance
- measured_on: V2 order-exact chassis (memory/grind/CD_sync/progress/s121-V2-order-exact-RA-only-6.c), chain-extender FAKE present, pp absent; control 6/160 bi 160 rd 0

## [s121] On the V2 order-exact chassis the local-alloc tie is broken by the block's local declaration order, by giving the arg5 address its own pointer local, by reading Intr.sync after the ix group, or by writing the call in the upstream SOTN bios.c fully-inlined shape.
- mechanism: The rederive modality's premise for this window: pseudo-numbering and quantity-creation order follow declaration and first-reference order, so a structurally different spelling of the same block may reverse the qty-number tiebreak.
- probe: N3 (V2 control, arg5/t0/ix declaration order), N4 (t0/ix/arg5 declaration order), N6 (arg5 address in its own `s32 *ap` local), N5 (Intr.sync read moved after the ix group), N1 (no locals at all: printf(fmt, D_800F19B8.func, D_800A11DC[D_800A11D5], tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]])), N2 (arg5 staged, arg4 inlined). Forms in tmp/grind/CD_sync/s121/forms/.
- result: N3 6, N4 6, N6 6, N5 7, N1 14, N2 14, all /160 bi 160 rd 0, against a 6/160 control. Declaration order is inert for this tiebreak, the pointer local is inert, and the fully-inlined upstream shape loses the order as well as the seats. Combined with s114 (the upstream body is already in hand) and s120's X/Y/Z sweep, the rederive modality's structural sources for this window are spent.
- verdict: KILLED
- kill_scope: instance
- measured_on: V2 order-exact chassis, chain-extender FAKE present, pp absent; control 6/160 bi 160 rd 0

## [s121] Mandated kill re-audit: the combine-foldable chain-extender FAKE is still load-bearing on both the folded candidate chassis and the V2 order-exact chassis.
- mechanism: flow.c records the extra reg_n_refs before combine.c folds the SYMBOL_REF difference, so the construct changes allocation while emitting zero bytes; ablation should therefore move the score if the effect is real and leave it if a later chassis change has made it inert.
- probe: tools/fake_ablate.py --func CD_sync --file system --candidate memory/grind/CD_sync/candidate.c, and again with tmp/grind/CD_sync/s120/forms/V2_ixfirst_folded.c.
- result: candidate.c 2/160 bi 160 with the FAKE and 15/159 without; V2 6/160 with and 19/159 without. Neither chassis's score is a FAKE-carrier artifact, so both this session's control numbers and the V2 diagnosis stand on their own.
- verdict: CONFIRMED

## s122 (structural, 2026-09-04)

### H122-1 CONFIRMED — block 3's contended quantities are compiler intermediates, and the ledger's six-session naming of them was wrong
Statement: local-alloc's qty1 (reg113) in block 3 is the result of the t0 SCALE
(`sll reg113 = reg107 << 2`, insn 127, live to insn 132) and qty2 (reg106) is the
arg5 loaded value (`lw reg106 = mem(reg111)`, insn 123, live to the
`sw reg106,16(sp)` insn 149). The t0 ADDRESS is reg107 — the /v pseudo of the C
variable `t0`, defined twice and read at `lw a3,0(reg107)` (insn 157) — and it is
not one of the four quantities local-alloc ranks in this block.
Probe: BB2_QTY_DEBUG + `-da` on the V2 chassis, reading
tmp/grind/CD_sync/s122/P5_v2_control/gccdump.lreg:446-505 (post-sched1 RTL) and
matching it against the QTYDBG births/deaths (numbering = 2*emission_pos + 4).
Also verified on the FULL src/system.c compile (tmp/grind/CD_sync/s122/qfull.sh):
identical table, so the s118 mini-TU harness is faithful.
- verdict: CONFIRMED

### H122-2 KILLED (instance) — variable-identity merge as a live_extend carrier
Statement: reusing one C variable so its live range spans both an earlier value
and the t0 chain (inverse.py's ranked #2-#5 "live_extend qty 1" vectors, whose
lever mapping is "merge/reuse one variable so the range spans both") moves qty1's
birth or death on the V2 order-exact chassis.
Probe: P1 (`t0` carries the ix index first, then the t0 chain), P2 (one variable
for everything, no `ix` local), P3 (`t0` carries the arg5 value first), P4 (mirror:
`ix` carries the t0 index), Q1 (separate index/scale/address variables), Q2 (t0
address as a single statement), Q3 (dedicated `s32 *ap` pointer local for the arg5
address); sandbox on each, and BB2_QTY_DEBUG on P1/P3/Q3 against the P5 control.
Result: scores 9, 10, 7, 9 (bi 161), 9, 9, 6 against a 6/160 control — and the
QTYDBG block-3 table is BYTE-IDENTICAL in P1, P3, Q3 and the control
(`qty0 10/20 refs6 got=2 · qty3 22/30 refs4 got=2 · qty1 18/24 refs2 got=3 ·
qty2 20/26 refs2 got=4`). Per the tool's own LOCAL-MODE CAVEAT this is the
"has not tested the vector" signature — and H122-1 says why: qty1/qty2 are
intermediates, so C variable identity cannot reach them.
- verdict: KILLED
- kill_scope: instance
- measured_on: V2 order-exact chassis (memory/grind/CD_sync/progress/s121-V2-order-exact-RA-only-6.c), chain-extender FAKE present, pp absent; control 6/160 bi 160 rd 0

### H122-3 KILLED (instance) — the luid/statement-order class contains the span fix
Statement: some source-statement order on the V2 chassis makes sched1 emit the
arg5 store (uid 149) before the t0 address addu (uid 132), which is the span
change that flips the qty1/qty2 priority order and hence the $v1/$a0 seats.
Probe: `perturb.py tmp/sched_solver_work/system.sched.json --func CD_sync
--pass 1 --block 3 --goal-before 132:149 --depth 1 --max 60` (note: the picks
list is END-FIRST, so "emit 149 before 132" is `--goal-before 132:149`);
1091 single atoms searched.
Result: 25 reaching vectors and NOT ONE is a `luid` atom — 20 are `add_dep`/
`del_dep` edges and 5 are `cost` atoms. Every dep atom orders one of our chain
insns against uid 149/155/123, the printf argument-setup insns the front end
emits last in the block, so no C statement can be placed after them; the cost
atoms (`cost 127 := 3`, `cost 112 := 12`, …) are instruction-selection changes
that would alter the emitted bytes. This is the model-level explanation of
s120's fifteen inert source orders. Artifact:
tmp/grind/CD_sync/s122/perturb_p1_goal.txt.
- verdict: KILLED
- kill_scope: instance
- measured_on: V2 order-exact chassis, sched model tmp/sched_solver_work/system.sched.json (parity=True, 52/52 blocks order- and clock-exact), chain-extender FAKE present, pp absent; control 6/160

## [s122] local-alloc's block-3 qty1 (reg113) is the result of the t0 scale insn (sll reg113 = reg107 << 2, insn 127, dead at insn 132) and qty2 (reg106) is the arg5 loaded value (insn 123, dead at the sw ...,16(sp) insn 149); the t0 ADDRESS lives in reg107, the /v pseudo of the C variable t0, and is not one of the four quantities local-alloc ranks in this block.
- mechanism: local-alloc numbers births/deaths as 2*emission_position+4 over the post-sched1 insn stream; reading the .lreg RTL and matching those numbers against the QTYDBG table identifies each quantity uniquely, where prior sessions inferred the mapping from the C statements.
- probe: BB2_QTY_DEBUG + -da on the V2 chassis; read tmp/grind/CD_sync/s122/P5_v2_control/gccdump.lreg:446-505; cross-check with a full-src/system.c dump via the new tmp/grind/CD_sync/s122/qfull.sh.
- result: Confirmed and reproduced on both the s118 mini TU and the full TU (identical tables, so the mini harness is faithful). Post-sched1 emission order of block 3 on V2: 104,106,112,116,153,118,121,127,123,140,132,149,145,155,157,151,159. qty0={reg108,reg111} ix chain 10/20 refs6 $v0; qty3={reg117,reg120} arg3 chain 22/30 refs4 $v0; qty1 18/24 refs2 $v1 (target wants $a0); qty2 20/26 refs2 $a0 (target wants $v1).
- verdict: CONFIRMED

## [s122] Reusing one C variable so its live range spans an earlier value and the t0 chain - inverse.py's ranked #2-#5 live_extend vectors, whose lever mapping is 'merge/reuse one variable so the range spans both' - moves qty1's birth or death on the V2 order-exact chassis.
- mechanism: GCC 2.7.2 gives one pseudo per C local, so merging two values into one variable should merge their live ranges and lengthen the quantity's span, lowering its qty_compare_1 priority below the arg5 value's and reversing the allocation order.
- probe: Seven forms measured with sandbox --disable all on the V2 chassis (control 6/160): P1 t0 carries the ix index first, P2 one variable and no ix local, P3 t0 carries the arg5 value first, P4 mirror (ix carries the t0 index), Q1 separate index/scale/address variables, Q2 t0 address as one statement, Q3 dedicated s32 *ap pointer local; plus BB2_QTY_DEBUG on P1/P3/Q3 against the control.
- result: 9, 10, 7, 9 (bi 161), 9, 9, 6 /160 - and the block-3 quantity table is byte-identical in P1, P3, Q3 and the control (qty0 10/20 refs6 got=2, qty3 22/30 refs4 got=2, qty1 18/24 refs2 got=3, qty2 20/26 refs2 got=4). Per the tool's LOCAL-MODE CAVEAT that is the 'the vector was never actually tested' signature, and H122-1 gives the reason: qty1/qty2 are compiler-generated intermediates whose ranges are fixed by insn positions, not by C variable identity. Forms banked as memory/grind/CD_sync/rejected/s122_*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: V2 order-exact chassis (memory/grind/CD_sync/progress/s121-V2-order-exact-RA-only-6.c), chain-extender FAKE present, pp pointer alias absent; control 6/160 bi 160 rd 0

## [s122] Some source-statement order on the V2 chassis makes sched1 emit the arg5 store (uid 149) before the t0 address addu (uid 132), the span change that flips the qty1/qty2 priority order and hence the $v1/$a0 seats.
- mechanism: Statement order is exactly the luid atom of the scheduler model; if the luid class contained a reaching vector, a plain reordering of the do_timeout block would produce the target seats with no construct at all.
- probe: python3 tools/sched_solver/perturb.py tmp/sched_solver_work/system.sched.json --func CD_sync --pass 1 --block 3 --goal-before 132:149 --depth 1 --max 60 (the picks list is END-FIRST, so 'emit 149 before 132' is --goal-before 132:149); 1091 single atoms searched.
- result: 25 reaching vectors, ZERO of them luid: 20 add_dep/del_dep edges and 5 cost atoms (cost 112:=12, cost 127:=3, cost 127:=12, cost 149:=3/12, cost 155:=12). Every dep atom orders one of our chain insns against uid 149/155/123 - the printf argument-setup insns the front end emits last in the block, so no C statement can be placed after them - and the cost atoms are instruction-selection changes that would alter the emitted bytes. This is the model-level explanation of s120's fifteen inert source orders. Artifact tmp/grind/CD_sync/s122/perturb_p1_goal.txt.
- verdict: KILLED
- kill_scope: instance
- measured_on: V2 order-exact chassis, sched model tmp/sched_solver_work/system.sched.json (parity=True, CD_sync 52/52 blocks order- and clock-exact), chain-extender FAKE present, pp absent; control 6/160 bi 160 rd 0

## s123 (structural, 2026-09-04)

### H123-1 KILLED (class) — the "pass-2-only perturbation atom" frontier is empty by construction
Statement: the s121/s122 frontier item 1 held that on the folded candidate chassis
there exist perturbation atoms which reach the sched2 (final-order) goal without
perturbing the sched1 stream that local-alloc consumes, so the block-3
transposition could be fixed while keeping the correct register seats.  Applying
each pass-2-reaching atom to the pass-1 block and comparing outputs appears to
find six such atoms (`luid swap 120<->125`, `luid swap 127<->130`,
`luid swap 130<->153`, `luid_move 120 before 125`, `luid_move 130 before 127`,
`luid_move 130 before 153`), but every one of them is an artifact of comparing
two DIFFERENT luid spaces, and the class it names is empty.
Mechanism: `sched_analyze` re-assigns `INSN_LUID` by walking the current insn
chain at the start of EVERY scheduling pass
(tools/gcc-2.7.2/sched.c:2198, `INSN_LUID (insn) = luid++;`).  Pass 1's luids
therefore index the post-combine chain and pass 2's luids index the POST-SCHED1
chain.  Reload does not reorder (verified: the .lreg and .greg chains are
identical to the .sched chain), so every scheduler input in pass 2 — order,
luids, and the dependence graph — is a function of sched1's output plus the
register assignment.  A perturbation that leaves sched1's output unchanged
therefore leaves pass 2's entire input unchanged, and its output with it.  A
"pass-2-only" atom cannot exist; the six apparent ones are pass-2 luid renumbers
with no pass-1 preimage.
Probe: tmp/grind/CD_sync/s123/diff_atoms.py (enumerate_atoms on the pass-2
block 3, keep the 39 that reach the goalmap-derived target order, re-apply each
to the pass-1 block, compare); then the pass-by-pass chain dump
(tmp/grind/CD_sync/s123/chainorder — .rtl/.jump/.cse/.loop/.cse2/.flow/.combine/
.sched/.lreg/.greg), which shows the post-combine chain is
104,106,112,116,120,125,127,130,132,140,145,149,151,153,155,157 (= the C source
order) while the model's pass-2 luid order is
104,106,112,116,153,127,120,130,132,140,125,149,145,155,157,151 (= sched1's
OUTPUT).  Two different spaces; the atom names a chain position, not a statement.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/sched.c:2198
- measured_on: folded candidate chassis (memory/grind/CD_sync/candidate.c),
  chain-extender FAKE present, pp absent; control 2/160 bi 160 rd 0; sched model
  tmp/sched_solver_work/system.sched.json re-extracted this session, parity=True

### H123-2 CONFIRMED — every C form that reaches block 3's target emission order lands on ONE post-sched1 stream, and that stream carries V2's wrong seats
Statement: block-3 order and block-3 seats are not two independent dials on this
chassis.  All 31 pass-1 atoms that reach the target's block-3 order produce the
SAME output stream, so local-alloc — whose input is exactly that stream — makes
the same decision for all of them; the target order therefore always comes with
the V2 seat assignment (score 6/160, an RA-only divergence of six instructions).
Mechanism: local-alloc numbers births and deaths as 2*emission_position+4 over
the post-sched1 chain (H122-1), so two forms with identical post-sched1 chains
have identical quantity tables by construction; the C-level differences between
them have already been consumed by the scheduler.
Probe: five structurally distinct spellings of the block, all measured with
`sandbox CD_sync --disable all` on the candidate chassis (control 2/160):
S3 (arg5 address split into `ix`, `t0 *= 4` placed between the ix addu and the
arg5 load), S5 (address split, both t0 insns between), S7 (address split, t0 pair
last), S9 (t0 scale between the ix shift and the ix addu) — 6, 6, 6, 6 — and
S4 (same as S3 with a dedicated `s32 ad` local) — 7.  S3 is an address SPLIT,
not the group MOVE that produced V2, and `inverse_compose.py classify` on S3
prints the identical verdict and the identical six pairs as s121 recorded for
V2: FIRST DIVERGENCE RA, ours addu v1,v1,s3 / lbu v1,0(s2) / lw a0,0(v0) /
lw a3,0(v1) / sll v1,v1,0x2 / sw a0,16(sp) against target addu a0,a0,s3 /
lbu a0,0(s2) / lw a3,0(a0) / lw v1,0(v0) / sll a0,a0,0x2 / sw v1,16(sp).
Two other spellings that do NOT reach the target order (S1, the t0 scale folded
into the t0 add as one statement; S6, the same with the ix shift hoisted) score
3/160 — a third residual class, neither 2 nor 6.
- verdict: CONFIRMED

### H123-3 KILLED (instance) — mandated kill re-audit: s122's Q3 pointer-local lever on the CURRENT chassis
Statement: s122's Q3 (a dedicated `s32 *ap` pointer local carrying the arg5
address), measured inert at 6 == 6 on the V2 order-exact chassis, moves the score
when transplanted onto the folded candidate chassis, where the contended
quantities sit at different emission positions.
Probe: S8_q3_ap_pointer_on_candidate.c — the candidate block with
`ap = (s32 *)(ix + (s32)tbl_125c); arg5 = *ap;` replacing the folded read —
measured with `sandbox CD_sync --disable all`; plus
`tools/fake_ablate.py --func CD_sync --file system --candidate
memory/grind/CD_sync/candidate.c` for the FAKE-state half of the re-audit.
Result: S8 = 2/160 bi 160 rd 0, exactly the control, so the pointer local is
inert on this chassis too.  fake_ablate: keep-all 2/160 bi 160, drop-1 15/159 —
the chain-extender FAKE is still load-bearing and the 2 is not a carrier
artifact, so both this session's control and the S-form deltas stand on their own.
- verdict: KILLED
- kill_scope: instance
- measured_on: folded candidate chassis (memory/grind/CD_sync/candidate.c),
  chain-extender FAKE present, pp pointer alias absent; control 2/160 bi 160 rd 0

## [s123] On the folded candidate chassis there exist perturbation atoms that reach the sched2 (final-order) goal without perturbing the sched1 stream local-alloc consumes, so the block-3 transposition can be fixed while the correct register seats are kept.
- mechanism: sched_analyze re-assigns INSN_LUID by walking the current insn chain at the start of every scheduling pass, so pass-1 luids index the post-combine chain and pass-2 luids index the post-sched1 chain; reload does not reorder this block (.lreg/.greg chains identical to the .sched chain), so sched2's entire input - order, luids and dependence graph - is a function of sched1's output plus the register assignment, and a perturbation leaving sched1's output unchanged leaves sched2's output unchanged.
- probe: tmp/grind/CD_sync/s123/diff_atoms.py: enumerate_atoms on CD_sync pass-2 block 3 of a freshly extracted model (parity=True), keep the 39 atoms reaching the goalmap-derived target order (build/src/system.o vs tmp/sandbox/CD_sync/system.o), re-apply each to the pass-1 block and compare outputs; then dump the insn chain after every pass (tmp/grind/CD_sync/s123/chainorder + chainorder.py) and compare the post-combine chain with the model's pass-2 luid order.
- result: The differential appears to find six qualifying atoms (luid swap 120<->125, 127<->130, 130<->153; luid_move 120 before 125, 130 before 127, 130 before 153) but all six are artifacts of comparing two different luid spaces. The post-combine chain is 104,106,112,116,120,125,127,130,132,140,145,149,151,153,155,157 - exactly the C statement order - while the model's pass-2 luid order is 104,106,112,116,153,127,120,130,132,140,125,149,145,155,157,151 - exactly sched1's output. A pass-2 luid atom names a chain position reload hands to sched2, not a C statement, and has no pass-1 preimage. This withdraws the frontier item s121 and s122 both carried.
- verdict: KILLED
- kill_scope: class
- measured_on: folded candidate chassis (memory/grind/CD_sync/candidate.c), chain-extender FAKE present, pp pointer alias absent; control re-measured live at 2/160 bi 160 rd 0; sched model tmp/sched_solver_work/system.sched.json re-extracted this session, parity=True
- predicate_cite: tools/gcc-2.7.2/sched.c:2198

## [s123] Every C form that reaches block 3's target emission order lands on one and the same post-sched1 stream, so local-alloc - whose input is exactly that stream - makes the same decision for all of them and the target order always arrives with the V2 seat assignment.
- mechanism: local-alloc numbers quantity births and deaths as 2*emission_position+4 over the post-sched1 chain (H122-1), so two forms with identical post-sched1 chains have identical quantity tables by construction; the C-level differences between them have already been consumed by the scheduler, and all 31 pass-1 atoms reaching the target order produce the identical output stream.
- probe: Five structurally distinct spellings of the do_timeout block measured with `sandbox CD_sync --disable all` on the candidate chassis (control 2/160): S3 (arg5 address split into ix, `t0 *= 4` placed between the ix addu and the arg5 load), S5 (address split, both t0 insns between), S7 (address split, t0 pair after the load), S9 (t0 scale between the ix shift and the ix addu), S4 (S3 plus a dedicated `s32 ad` local); then tools/ra_solver/inverse_compose.py classify on S3.
- result: S3 6, S5 6, S7 6, S9 6, S4 7 - all bi 160 rd 0. S3 is an address SPLIT, structurally unlike V2's group move, yet classify prints the identical verdict and the identical six pairs s121 recorded for V2: FIRST DIVERGENCE RA, ours addu v1,v1,s3 / lbu v1,0(s2) / lw a0,0(v0) / lw a3,0(v1) / sll v1,v1,0x2 / sw a0,16(sp) against target addu a0,a0,s3 / lbu a0,0(s2) / lw a3,0(a0) / lw v1,0(v0) / sll a0,a0,0x2 / sw v1,16(sp). Order and seats are one dial, not two. Two forms that do NOT reach the target order (S1, the t0 scale folded into the t0 add as one statement; S6, the same with the ix shift hoisted) score 3/160 - a third residual level not previously seen.
- verdict: CONFIRMED

## [s123] s122's Q3 lever - a dedicated `s32 *ap` pointer local carrying the arg5 address, measured inert at 6 == 6 on the V2 order-exact chassis - moves the score when transplanted onto the folded candidate chassis, where the contended quantities sit at different emission positions.
- mechanism: The mandated kill re-audit: an instance kill is only as good as the chassis and FAKE state it was measured under, so the closest-to-target banked kill is re-run on the current chassis and with every FAKE construct ablated.
- probe: tmp/grind/CD_sync/s123/forms/S8_q3_ap_pointer_on_candidate.c (the candidate block with `ap = (s32 *)(ix + (s32)tbl_125c); arg5 = *ap;` replacing the folded read) measured with `sandbox CD_sync --disable all`; plus `python3 tools/fake_ablate.py --func CD_sync --file system --candidate memory/grind/CD_sync/candidate.c`.
- result: S8 = 2/160 bi 160 rd 0, exactly the control - the pointer local is inert on this chassis too, so the s122 kill survives the re-audit. fake_ablate: keep-all 2/160 bi 160, drop-1 15/159, so the combine-foldable chain-extender FAKE is still load-bearing and none of this session's numbers are a carrier artifact.
- verdict: KILLED
- kill_scope: instance
- measured_on: folded candidate chassis (memory/grind/CD_sync/candidate.c), chain-extender FAKE present, pp pointer alias absent; control 2/160 bi 160 rd 0

## s124 (synthesis) - frontier reset

**H124-1 (LIVE, top).  Make combine.c:10752-10754 fire on reg106.**
The refs 2 -> 3 bump at `tools/gcc-2.7.2/combine.c:10752-10754` is the only
known reg_n_refs lever that emits nothing and leaves the insn stream untouched.
It fires when combine successfully merges an i2 into an i3, the register in the
REG_DEAD note being redistributed is referenced in BOTH patterns, and `place`
resolves to i3.  Today reg106 (the arg5 loaded value) appears in exactly two
insns - the load that defines it and the `sw ...,16(sp)` that consumes it - and
no combination touches both.  PROBE: enumerate spellings of the do_timeout block
in which the arg5 value participates in a two-insn combination that survives
recog (an address form absorbing the value, a widening/narrowing pair, a
zero_extract, an equality the target already computes), then for each: (a)
sandbox score, (b) QTYDBG `refs=` on reg106, (c) `chainorder.py` to prove the
post-sched1 chain is unchanged.  Success signature: refs exactly 3, bi 160,
block-3 order unchanged, score 0.  Do the search ON THE SPLIT (`t0 *= 4`)
ORDER-EXACT CHASSIS (S3/X1, 6/160), not on the candidate chassis and not on the
`t0 <<= 2` merged chassis (both measured dead this session).

**H124-2 (LIVE).  A refs=3 carrier that does not create a basic block.**
`do { ... } while (0);` produces refs exactly 3 by flow.c:2081 and is a
sanctioned family, but its loop notes split block 3 and cost more than they buy
(11-15/160 over eight placements, s124 E/F).  The lever is the loop_depth
multiplier, not the wrap: any construct that raises `loop_depth` over the arg5
load ALONE without emitting a NOTE_INSN_LOOP_BEG inside block 3 would close.
PROBE: read flow.c 2060-2100 and 2700-2730 for every path that increments
loop_depth, and check whether `find_basic_blocks` treats the wrap's notes as
block boundaries in this position or whether a placement exists (e.g. the wrap
around the WHOLE do_timeout block plus a compensating refs change on ix) that
keeps one block.  Report the block partition from `.flow`, not from the score.

**H124-3 (LIVE, third vantage).  The 3/160 residual class is still unread.**
s123's S1 and S6 (t0 scale folded into the t0 add on the FOLDED-arg5-address
chassis) sit at 3/160 - neither the 2 of the candidate nor the 6 of the
order-exact family.  s124's X-family folded the scale on the SPLIT-address
chassis and got 6, so 3/160 is specific to the folded-address stream and has
never had its quantity table dumped.  PROBE: QTYDBG + `inverse_compose.py
classify` on S1 and S6; if classify says RA, run `inverse.py local --swap` on
that model and compare the vector list with the S3/V2 one - a different tie may
be breakable by a lever the 2- and 6-chassis cannot reach.

**Retired this session:** the s123 frontier item "inverse.py rank-#8 refs_up on
qty 2, never spelled" is now spelled and measured - refs_up works, the two
obvious carriers (dead store, do-while(0)) are dead as spelled, and the target
value is 3 exactly, not >= 3.  The "S1/S6 third residual class" item survives as
H124-3.  The "add_dep memory-edge class" item is superseded: s124 shows the
residual is a refs question on an order-exact stream, and an aliasing edge is an
ORDER lever.

## [s124] Folding the t0 scale into the t0 address at C level (the correct spelling of inverse.py's refs_down-on-qty-1 vector under s122's corrected quantity map) removes the scale temp from local-alloc's block-3 table on an order-reaching chassis.
- mechanism: s121 measured refs_down against the quantity s122 later proved mis-identified (it folded the t0 ADDRESS add into printf's 4th-argument MEM, and the address pseudo reg107 is not one of the four quantities local-alloc ranks). The right test is to fold the SCALE, which is qty1, and it had never been run on a chassis that reaches the target emission order.
- probe: Eight forms X1-X8 generated by tmp/grind/CD_sync/s124/gen.py: X1-X4 combine the S3 ix-address split (order-reaching) with `t0 = (s32)((u8 *)tbl_125c + t0 * 4)`; X5/X6 mirror it (ix scale folded, t0 address split); X7/X8 fold both chains. Sandbox score for each plus a BB2_QTY_DEBUG dump of block 3 for X1.
- result: X1 6, X2 6, X3 6, X4 6, X5 7, X6 7, X7 7, X8 7 - all build_insns 160, rules_dropped 0, against a live control of 2/160 on candidate.c. X1's block-3 quantity table is byte-identical to V2/S3's: qty0 reg108 10/20 refs6 got=2, qty3 reg119 22/30 refs4 got=2, qty1 reg112 18/24 refs2 got=3, qty2 reg106 20/26 refs2 got=4. C-level statement folding does not remove the scale temp; expand_expr still allocates a fresh pseudo for the product.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (memory/grind/CD_sync/candidate.c, 2/160) and the S3/V2 order-exact split chassis (6/160); chain-extender FAKE present in all eight forms, pp pointer alias absent

## [s124] Respelling `t0 *= 4;` as `t0 <<= 2;` merges the t0 chain into a single local-alloc quantity and improves the score on the candidate or the order-exact chassis.
- mechanism: `t0 *= 4` expands as a MULT, so expand_expr targets a FRESH pseudo (reg112, carrying REG_EQUAL (mult (reg 107) 4)) for the scale, leaving the C variable's own pseudo reg107 with two disjoint live ranges - the .lreg register file prints 'dies in 2 places' and local-alloc skips it, handing the t0 address to global-alloc. `ix <<= 2` writes its pseudo in place, which is why the ix chain is one 6-ref quantity. Making t0 a shift should give it the same one-pseudo shape the target's asm has.
- probe: Six forms B0-B5 (tmp/grind/CD_sync/s124/gen3.py) applying `t0 <<= 2` on the candidate chassis, on four order-exact chassis, and a mirror with `ix *= 4`; sandbox score for each, plus a BB2_QTY_DEBUG + .lreg register-file dump for B1.
- result: B0 7, B1 6, B2 6, B3 6, B4 7, B5 9, all bi 160. The merge is real - B1's block 3 becomes qty0 reg107 birth 8 death 32 refs 6 got=3 (pri 5000) and 'Register 107 used 6 times' with no 'dies in 2 places' - but it is strictly worse for closing: a 5000-pri t0 chain ties any refs-3 arg5 (also 5000) and wins the qty-number tiebreak, so the merged chassis cannot reach the target seats at all. The SPLIT `t0 *= 4` chassis is the closing chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (2/160) and the S3/S5/S7/S9 order-exact chassis (6/160); chain-extender FAKE present, pp pointer alias absent

## [s124] A dead store or self-assignment to the arg5 local raises reg_n_refs on the arg5 loaded value from 2 to 3.
- mechanism: s123's frontier named inverse.py's rank-#8 refs_up on qty 2 as the one unspelled vector; the dead-store family (a sanctioned SOTN carve-out) is the cheapest carrier for an extra reference, and reg_n_refs is accumulated in flow.c's life_analysis, so a store that survives to flow would be counted.
- probe: Five forms A1-A5 on the S3 order-exact chassis: `arg5 = arg5;` after the load, after the t0 addu, doubled, `ix = arg5;`, and the S5-stream variant. Sandbox score for each plus BB2_QTY_DEBUG block-3 dumps for A1, A3 and A4.
- result: All five score 6/160 bi 160, exactly the A0 S3 control, and QTYDBG shows qty2 reg106 birth 20 death 26 refs=2 unchanged on A1, A3 and A4. The stores are deleted before flow.c ever counts a reference, so the dead-store family is not a reg_n_refs carrier for this pseudo.
- verdict: KILLED
- kill_scope: instance
- measured_on: S3 order-exact split chassis (6/160 control); chain-extender FAKE present, pp pointer alias absent

## [s124] Wrapping the arg5 load in `do { ... } while (0);` raises reg_n_refs on the arg5 loaded value to exactly 3 without disturbing block 3.
- mechanism: flow.c:2081 accumulates `reg_n_refs[regno] += loop_depth`, and GCC 2.7.2's front end emits NOTE_INSN_LOOP_BEG/END for a do-while(0), so every reference inside the wrap is counted twice. `do { } while (0)` is a sanctioned family (.claude/rules/do-while-zero-exception.md) for any codegen effect including register allocation.
- probe: Eight forms E1-E4 and F1-F4 (gen5.py, gen6.py): the wrap around the load only on the S3 and S5 streams, around the load plus the t0 addu, on the candidate folded chassis, and four variants routing the address through a dedicated `s32 ad` local. Sandbox score for each plus BB2_QTY_DEBUG block-3 dumps for E1 and E2.
- result: The refs half CONFIRMS: E1 and E2 both dump qty2 reg106 refs=3, exactly the predicted value. Everything else regresses - the loop notes split basic block 3, so ix picks up a 7th reference (qty0 reg108 refs=7), the t0 chain merges into reg107 refs 6 (pri 5454) even with `t0 *= 4`, and the emission order is destroyed. Scores E1 14, E2 12, E3 12, E4 11, F1 15, F2 13, F3 14, F4 14, all bi 160. The wrap is a working refs lever and a fatal order lever at the same time.
- verdict: KILLED
- kill_scope: instance
- measured_on: S3/S5 order-exact split chassis and the candidate folded chassis; chain-extender FAKE present plus the do-while(0) wrap itself, pp pointer alias absent

## [s124] Raising reg_n_refs on the arg5 loaded value flips the block-3 local-alloc seats, and the value that produces the target seats is three references exactly, not three-or-more.
- mechanism: qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1657) ranks by floor_log2(refs)*refs*size/(death-birth)*10000. On the split order-exact stream the four quantities are ix (refs 6, span 10, pri 12000), arg3 (refs 4, span 8, 10000), the t0 scale temp reg112 (refs 2, span 6, 3333) and the arg5 value reg106 (refs 2, span 6, 3333), the 3333 tie decided by qty number in reg112's favour. refs 3 gives pri 5000, which slots reg106 third of four; refs 4 gives 13333, which slots it FIRST.
- probe: C1 and C2 (`arg5 |= 1;` placed after the load and before the call) as a deliberately byte-changing model test that raises reg106 to refs 4; sandbox score plus BB2_QTY_DEBUG block-3 dumps for both, compared against the refs-2 control and the refs-3 dumps from E1/E2.
- result: CONFIRMED and quantified. C1/C2 dump qty2 reg106 refs=4 got=2 at ord=0 - allocated first, seated in $v0, score 11/161. E1/E2 dump refs=3. The control dumps refs=2 with reg106 last at ord=3 in $a0. So refs is a live dial on the seat, refs 4 overshoots past $v1 into $v0, and the predicted refs-3 allocation is ix $v0, arg3 $v0, arg5 $v1, reg112 $a0, with reg107's second live range overlapping arg5's $v1 range so global-alloc must give the t0 address $a0 - all six divergent register pairs in the target's spelling.
- verdict: CONFIRMED

## s125 (escalation / disposition)

## [s125] Pure statement-order permutations of the do_timeout block on the FOLDED (seat-exact) candidate chassis - a family s123 only measured on the SPLIT address chassis - reach a residual other than the two known values 2 and 6.
- mechanism: s122 established that the block-3 tie falls through rank_for_schedule's priority and dependence-class keys to INSN_LUID, and LUID is source-statement order; s123's S-family tested that on the SPLIT arg5-address chassis (all 6). The folded chassis has the correct seats and only the 54/55 transposition, so if LUID order alone can move the transposition there, the seats survive and the score drops below 2.
- probe: Four permutations generated by tmp/grind/CD_sync/s125/gen.py, each moving only the `t0 *= 4;` and `t0 = (u8 *)tbl_125c + t0;` statements relative to the `ix <<= 2;` / folded `arg5 = *(s32 *)(ix + (s32)tbl_125c);` pair, measured with `sandbox CD_sync --disable all` against a live control of 2/160: P1 (t0 pair after the load), P2 (scale between the ix shift and the load, address after), P3 (t0 pair between the ix shift and the load), P6 (scale first, address after the load).
- result: P1 6, P2 2, P3 2, P6 2 - all bi 160 rd 0. No third value. The folded chassis reproduces the same binary dial s123 found on the split chassis: a permutation either leaves the stream alone (2, seats right, transposition present) or flips it wholesale to the order-exact/seat-wrong stream (6). Statement order is not an independent handle on the transposition here. Banked memory/grind/CD_sync/rejected/s125_P1_t0pair_after_load.c and the three inert siblings.
- verdict: KILLED
- kill_scope: instance
- measured_on: folded candidate chassis (memory/grind/CD_sync/candidate.c), chain-extender FAKE present, pp pointer alias absent; control re-measured live at 2/160 bi 160 rd 0

## [s125] The arg5 loaded value (reg106) is a source operand in more than one insn of CD_sync, so combine.c:10752-10754's byte-neutral reg_n_refs 2 -> 3 bump has a spelling that can fire on it.
- mechanism: s124's H124-1 named combine.c:10752-10754 ("If the register is used in both I2 and I3 and it dies in I3 ... if reg_n_refs was 2, bump it to 3") as the one unspent byte-neutral carrier for the closing predicate. Reading the guard verbatim, it fires only when reg_referenced_p(R, PATTERN(i3)) placed the REG_DEAD note at i3 AND reg_referenced_p(R, PATTERN(i2)) - and rtlanal.c's reg_referenced_p explicitly does NOT count a plain (set (reg R) ...) destination as a reference. So R must be a SOURCE in two surviving patterns simultaneously.
- probe: Read tools/gcc-2.7.2/combine.c:10716-10756 for the exact guard, then enumerate every insn in which reg106 appears, on both the folded candidate chassis and in the TARGET's own asm, from tmp/grind/CD_sync/s125/dis_cand.txt and tmp/grind/CD_sync/s121/CD_sync.tgt.s.
- result: reg106 appears in exactly two insns on every chassis measured - its definition `lw v1,0(v0)` (a SET_DEST, which reg_referenced_p does not count) and its single consumer `sw v1,0x10($sp)`. The TARGET's asm has the identical two-insn shape at 0x80080E90 / 0x80080EA4, so the original compilation also had one source reference. A second SOURCE reference in a surviving pattern is by definition a second emitted use, which is not byte-neutral at bi 160, and the sw/lw pair cannot combine on MIPS (mem-to-mem set fails recog). The bump has no byte-neutral C spelling for this pseudo on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: folded candidate chassis (2/160, live control) and the target asm tmp/grind/CD_sync/s121/CD_sync.tgt.s; chain-extender FAKE present, pp pointer alias absent

## [s125] DISPOSITION - both endgame-lock AND-gates evaluated and both FAIL.
- gate (a) canonical-asm: `python3 tools/scan_hand_coded.py --single CD_sync` -> tier=LOW score=2/8; S1 multu pacing, S2 empty branch and S6 BIOS jumptable all absent; only S4 (4 loads in an 8-insn window @49) and S5 (cluster, CD_ready jaccard 0.64) fire. Consistent with the 2026-07-09 Judge bar and the binding judge_constraint.
- gate (b) SOTN precedent: docs/reference/sotn-construct-index.md (master aa53500, 1911 files) indexes twelve construct classes - fake_comment, fake_identifier, self_assign, pointer_alias, pad_dummy_local, new_var_temp, nested_exit_label, match_comment, empty_if, dup_if_else_arm, do_while_zero, const_holder. None is a byte-neutral reg_n_refs carrier; the two sanctioned classes that could have been (self_assign, do_while_zero) are the two s124 measured dead. Negative census.
- filed: docs/grind/decisions.md, `## 2026-09-05 - CD_sync (src/system.c) - RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED`.

## [s125] Pure statement-order permutations of the do_timeout block on the FOLDED (seat-exact) candidate chassis - a family previously measured only on the SPLIT address chassis - reach a residual other than the two known values 2 and 6.
- mechanism: s122 established the block-3 tie falls through rank_for_schedule's priority and dependence-class keys to INSN_LUID, and LUID is source-statement order. The folded chassis already has all 160 correct seats and only the 54/55 transposition, so if LUID order alone can move that transposition there, the seats survive and the score drops below 2.
- probe: Four permutations (tmp/grind/CD_sync/s125/gen.py) moving only `t0 *= 4;` and `t0 = (u8 *)tbl_125c + t0;` relative to `ix <<= 2;` and the folded arg5 load, measured with `sandbox CD_sync --disable all` against a live 2/160 control.
- result: P1 6, P2 2, P3 2, P6 2 - all bi 160 rd 0. No third value: a permutation either leaves the stream alone (2, seats right, transposition present) or flips it wholesale to the order-exact/seat-wrong stream (6). Banked as memory/grind/CD_sync/rejected/s125_P*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: folded candidate chassis (memory/grind/CD_sync/candidate.c), chain-extender FAKE present, pp pointer alias absent; control re-measured live at 2/160 bi 160 rd 0

## [s125] The arg5 loaded value (reg106) is a source operand in more than one insn of CD_sync, so combine.c:10752-10754's byte-neutral reg_n_refs 2 -> 3 bump has a spelling that can fire on it.
- mechanism: s124's H124-1 named combine.c:10752-10754 as the one unspent byte-neutral carrier for the closing predicate (reg_n_refs(arg5 value) == 3 exactly). The guard fires only when the REG_DEAD note places at i3 AND reg_referenced_p(R, PATTERN(i2)) holds; rtlanal.c's reg_referenced_p does not count a plain (set (reg R) ...) destination, so R must be a SOURCE in two surviving patterns at once.
- probe: Read tools/gcc-2.7.2/combine.c:10716-10756 for the verbatim guard, then enumerate every insn mentioning reg106 on the folded candidate chassis (tmp/grind/CD_sync/s125/dis_cand.txt) and in the target's own asm (tmp/grind/CD_sync/s121/CD_sync.tgt.s).
- result: reg106 appears in exactly two insns: its definition `lw v1,0(v0)` (a SET_DEST, uncounted) and its single consumer `sw v1,0x10($sp)`. The target has the identical two-insn shape at 0x80080E90/0x80080EA4, so the original compilation also had one source reference. A second source reference is by definition a second emitted use (not byte-neutral at bi 160), and the lw/sw pair cannot combine on MIPS (mem-to-mem set fails recog). The ledger's top live frontier item is retired.
- verdict: KILLED
- kill_scope: instance
- measured_on: folded candidate chassis (2/160 live control) and the target asm tmp/grind/CD_sync/s121/CD_sync.tgt.s; chain-extender FAKE present, pp pointer alias absent

## s126 (2026-09-07) � rederive (forced sibling transplant) � **FLOOR 2 -> 0. MATCHED.**

The forced-rederive sibling notice was correct and the transplant was the whole answer.
Probe order was the owner directive's: read CD_ready's and CD_datasync's matched on-main
bodies FIRST, transplant their spelling of the shared do_timeout window and declaration
block onto CD_sync, measure, and only then iterate.

### H126-1 CONFIRMED � CD_ready's / CD_datasync's matched window spelling, transplanted whole, removes the 54/55 transposition that defined the residual from s115 to s125.
The s117-s125 chassis was discarded entirely (CD_alarm struct, combine-foldable
chain-extender, folded-vs-split ix arithmetic, honest `goto poll` loop). V1 =
CD_datasync's declaration block (`tbl_125c`/`idx_1494`/`idx_1495` pointer aliases with the
volatile coming from the TU's own `extern volatile u8 g_cd_status_a;`) plus CD_ready's
window (`t0` before a do-while(0) wrap; `tb`/`pB`/`src`/`arg5`/`pp` inside it;
`printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)src, arg5)`; `CD_flush()` inside
the wrap). Measured **18/160** � and the disassembly comparison
(tmp/grind/CD_sync/s126/cmp2.py) shows **ZERO opcode or order differences across all 160
instructions**. Every one of the 18 is a callee-saved seat substitution:
ours s0=status s1=idx_1494 s2=saved s3=tbl_125c s4=mode s5=result s6=idx_1495;
target s0=status s1=saved s2=idx_1494 s3=tbl_125c s4=idx_1495 s5=mode s6=result.
Twenty sessions of block-3 arithmetic (the 54/55 `addu $v0,$v0,$s3` / `sll $a0,$a0,2`
transposition) were attacking a residual that the sibling spelling simply does not have.

### H126-2 CONFIRMED � a do-while(0) wrap around the whole CheckCallback/poll block moves saved into $s1, idx_1494 into $s2 and tbl_125c into $s3 (18 -> 9).
Mechanism: flow.c:2081 weights `reg_n_refs` by loop_depth, and global.c's allocno priority
sort consumes those counts. The wrap lifts every reference inside the callback block by one
depth level, which reorders the allocno sort exactly as CD_ready's identical wrap does
(src/system.c:564, "seats idx_1494/idx_1495"). Residual after: 9/160, all in idx_1495 /
mode / result ($s6/$s4/$s5 vs the target's $s4/$s5/$s6).

### H126-3 CONFIRMED � writing the poll loop as a REAL `do { ... if (status == 0) break; ... } while (1);` instead of a backward `goto poll;` closes the function (9 -> 0).
This is the finding the previous 125 sessions never had, and it is not a FAKE construct: it
is ordinary C, the loop the function actually performs, and the spelling CD_ready ships
matched on main. Mechanism: the C front end emits NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END
only for real loop constructs; a backward `goto` produces no loop note at all, so
flow.c:2081's `reg_n_refs[regno] += loop_depth` gives every reference inside a
goto-spelled loop weight 1. With the real loop, idx_1495's single dereference inside the
callback block is weighted at depth 2 (inner loop inside the do-while(0) wrap), which lifts
its allocno above the two incoming parameters in global.c's priority sort. Result:
idx_1495 -> $s4, mode -> $s5, result -> $s6 � the target's seats. **0/160.**

### H126-4 KILLED (instance) � the `new_var = 0xFF` constant holder and the `tb` ready-byte named intermediate are byte-inert on this chassis and are DELETED.
Both were inherited from CD_ready's body (where they are load-bearing). Ablation A2
(`temp = (*idx_1494) & 0xFF;`, `int new_var;` removed) = 0/160; ablation A4 (`tb` removed,
`pB = (s32 *)((idx_1494[1] << 2) + (s32)tbl_125c)`) = 0/160; combined (V7) = 0/160. The
submitted body carries neither. Two fewer FAKE constructs than the sibling.

### H126-5 CONFIRMED � every OTHER construct in V7 is individually load-bearing (single-construct ablation, this chassis, all other FAKEs present).
| ablation | form | score |
|---|---|---|
| C1 | `tbl_125c` alias removed, direct `D_800A125C[]` | 31/160 |
| C2 | `idx_1494` alias removed, direct `(&g_cd_status_a)[n]` | 29/160 |
| A7 | `idx_1495` alias removed, `idx_1494[1]` at the use site | 12/160 |
| A8 | window `do { } while (0);` removed | 25/160 |
| A5 | `pp` alias removed, direct `D_800F19C0` read | 18/160 |
| A6 | `src` staging replaced by a fresh local `ta` | 8/160 |
| B1 | `pB` folded back into the `arg5` load | 7/160 |
| B2 | `arg5` passed as `*pB` directly | 9/160 |
| B3 | `t0` inlined into the `src` address | 14/160 |
| V1 | callback `do { } while (0);` removed | 18/160 |
| V2 | poll loop back to `goto poll;` | 9/160 |
| V3 | declaration order `saved` before `idx_1494` (on V1) | 18/160 |

### H126-6 KILLED (instance) � the s115-s125 diagnosis that CD_sync's residual is a block-3 local-alloc / sched1-order dial is superseded, not refuted: it was a property of the discarded chassis.
The whole s118-s125 frontier (the reg_n_refs(reg106) == 3 predicate, the combine.c:10752
bump, the folded-vs-split ix add, the order-vs-seat equation) described the CD_alarm /
chain-extender / goto-poll chassis. On the sibling chassis block 3 is order-exact and
seat-exact from the first probe, and the entire residual lives in the callee-saved
allocation of the FUNCTION-WIDE locals � a region no session had looked at. This is the
concrete instance of the ledger's own standing warning that when every verdict looks dead,
one verdict is wrong: the wrong one was the choice of chassis, not any measurement on it.

**Frontier: EMPTY. The function is matched, bytes proven on main
(sandbox 0/160, rules_dropped 0, verify-oracle build_sha1 == oracle).**
