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
