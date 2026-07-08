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
