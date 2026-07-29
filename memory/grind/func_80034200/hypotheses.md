# Hypothesis ledger — func_80034200

Recon-session (s1) frontier. Honest floor 21 decomposes into 3 orthogonal-looking
components; the first two can be attacked independently, the third is the
residual RA-plateau this function's grind will live and die on.

## H1 — phantom frame -8 arises from a specific live local pattern
- **Mechanism:** GCC 2.7.2's frame-size accounting reserves stack bytes for
  ordinary LIVE locals whose values happen to never be stored (see
  [[phantom-frame-slots-gcc272]]). Target has `addiu $sp,-8` / `addiu $sp,8`
  and 0 stores between them; clean C emits no frame. Something in the target's
  C — likely a small handful of scalars that all live entirely in registers
  yet count against `get_frame_size()` — was allocated 8 bytes of stack.
- **Next probe:** compile the clean form with `-da` and read the `.stack` /
  `.greg` dumps to see what allocnos GCC assigned stack slots for; then vary
  the local count/types (e.g. add a `s32 v0_outer;` outside the inner block,
  merge the two `s32 v0` locals, promote the pointer aliases to explicit
  `s16*` walk) to trip GCC into the 8-byte reservation. Compare frame_size
  across variants.
- **Verdict:** OPEN.

## H2 — outer-loop `D_800A389B` reload is CSE-defeat surface
- **Mechanism:** target re-reads `D_800A389B` at outer-loop tail (lui+lbu),
  clean C hoists to a register because CSE sees no intervening writes. The
  `[[split-read-defeats-hoist]]` / `[[store-const-reload-cse]]` family covers
  exactly this shape: inserting a barrier that defeats the hoist (a
  duplicate-read into an arm, or a global-write that could alias the read).
- **Next probe:** try three variants: (a) drop the local `count` and use
  `D_800A389B` in both the entry `<= 0` check and the loop tail directly, no
  intermediate binding; (b) hoist the initial `g_disp_enable = DISP_LOADING`
  store BETWEEN the count-read and the loop, so GCC's alias analysis may
  refuse to CSE across an unrelated global store; (c) if (a) and (b) both
  hoist, re-read D_800A389B via a duplicated statement in an arm that reaches
  the tail unconditionally. Measure sandbox floor after each.
- **Verdict:** OPEN.

## H3 — the residual is a register-rename plateau (target uses arg regs;
  clean C uses t-regs for the accumulator and inner counter)
- **Mechanism:** function is void-void with no calls, so `$a0-$a3` are all
  free from arg-1. Target's `global.c` allocation picked `$a0` for the
  accumulator, `$a3` for outer i, `$a2` for shift; clean-C build picked
  `$t0`/`$a2`/`$a0` respectively — a mirror-image priority-tiebreaker
  outcome. See [[register-alloc-pure-c]] Levers A–D and
  [[call-return-if-result-reuse-v0]].
- **Next probe:** (a) instrumented cc1 `-da` `.greg` dump on the clean form
  to identify each pseudo's `reg_n_refs` / priority and see which reg it
  would be forced to under natural allocation; (b) once identified, try the
  playbook levers — narrow int type (u8 for the byte load path), block-local
  scope for shift/i, and, if H1 opens up phantom-frame territory, use the
  same construct to bias RA at the same time.
- **Verdict:** OPEN — likely the last front to close after H1 and H2.

## Rejected — do NOT re-propose

- **register-asm pins on all locals + `volatile char _pad[8]`** (the
  as-inherited src). Scored 16 by coercing GCC's allocator via
  forbidden-family cheats (`register T x asm("$N")` × 9,
  `dead-vars-local-array` on an unwritten array). Detector-flagged; policy
  is unambiguous under `inline-asm-policy` and `no-new-park-categories`.
  Preserved verbatim in `rejected/register-asm-pins-plus-volatile-pad.c`.

## [s2] H1 CONFIRMED — the phantom -8 frame is a free artifact of the loop shape
- mechanism: cc1 reserves the 8 locals bytes for any form where the outer loop is
  a compiler-recognised loop shape (goto-form or real-loop); the s1 hand-written
  do-while-with-explicit-guard was the anomaly at `vars= 0`. No dead local, no
  array, no coercion.
- probe: `.frame ... # vars=` read directly out of the cc1 `.s` for ~40 variants
- result: vars=8 on every goto-form and real-loop variant; vars=0 only on the s1 form
- verdict: CONFIRMED (closed — do not spend further sessions on the frame)

## [s2] H2 CONFIRMED — the D_800A389B tail reload requires a goto-form outer loop
- mechanism: loop.c only runs scan_loop on loops delimited by NOTE_INSN_LOOP_BEG.
  A `goto` loop has none, so the tail read is never a movable. Every real-loop
  spelling hoists it: the load is single-set + invariant_p (no stores/calls in the
  loop), reg_in_basic_block_p is true, and the may_trap_p/maybe_never guard cannot
  fire because rtx_addr_can_trap_p(SYMBOL_REF) == 0.
- probe: while / for / do-while / goto forms, bound inline vs via a local, tail
  test reordered; cc1 `.s` inspected for a second `lbu D_800A389B`
- result: reload present ONLY in goto-form. Floor 21 -> 16 once adopted.
- verdict: CONFIRMED

## [s2] H3 PARTIALLY CONFIRMED — the rotation is RA-priority driven and reachable
- mechanism: `global.c:allocno_compare` = floor_log2(n_refs)*n_refs/live_length.
  Dropping ONE reference of a value can cross a floor_log2 step and reorder the
  whole allocation.
- probe: 35+ structural variants through a standalone-TU harness, reading the
  `.greg` "regs to allocate" order and the emitted registers
- result: target's full triple (acc=$a0, base=$a1, shift=$a2) IS reachable — the
  single-`sllv` form w18 produces it exactly. Removing the `v0` temp reached
  acc=$a0 while keeping all 40 instructions (floor 14 -> 11). base<->shift is the
  last swap and is insensitive to every ref-count-preserving mutation tried.
- verdict: CONFIRMED reachable; the specific base>shift ordering is still OPEN.

## [s2] KILLED — variable reuse does not defeat the LICM hoist
- mechanism: the intent was n_times_set[dest] != 1 so consec_sets_invariant_p
  fails. It does not apply: the load always lands in its own fresh temp pseudo,
  and the copy into the reused user variable is a separate insn.
- probe: v15/v20, `n` used both for the useReal flag and for the loop bound
- result: reload absent in both; registers got worse ($8/$5 for acc/shift)
- verdict: KILLED (form banked in rejected/variable-reuse-does-not-defeat-licm.c)

## [s2] KILLED — declaration order / statement order as an RA lever here
- mechanism: allocno_compare only falls back to allocno number on an exact
  priority tie; these allocnos are not tied.
- probe: 14 declaration/init/tail-order permutations + 15 statement-level
  mutations (endp-from-p, `p != end_p`, shift-before-or, s16 narrowing of
  useReal and innerBound, inlined bound, negated compare, advance-early, ...)
- result: byte-identical allocation in every single one
- verdict: KILLED — do not re-run this family; only ref-COUNT changes move it

## [s2] KILLED — preheader constant-holder (`emptyVal = 3;`) does not lower the floor
- mechanism: it does put `li 3` in the preheader at target's slot and does fix
  base to $a1, but it swaps innerBound/const3 and swaps shift/i.
- probe: z02/z03 built and scored with `sandbox --disable all`
- result: **11 — identical to the clean pure-C form.** No gain.
- verdict: KILLED. This also disposes of the classification question: the
  FAKE-family construct is not worth a ruling because it does not help.
  (Form banked in rejected/constant-holder-preheader-no-floor-gain.c)

## [s2] KILLED — inner loop as a goto loop (to lower shift's loop_depth weight)
- mechanism: it does flip base above shift, but with `3` unhoisted both arms end
  in an identical `sll`, jump.c's find_cross_jump merges them.
- probe: x02 / z04
- result: build_insns 31 vs target 40
- verdict: KILLED unless `3` is already in a register before the inner loop

## [s1] as-inherited src used forbidden register-asm pins + volatile-pad array to reach floor 16
- mechanism: 9x `register T x asm("$N")` allocator pins + `volatile char _pad[8]; (void)_pad;` frame reservation — matches inline-asm-policy expanded cheat catalog and no-new-park-categories cheats-by-any-spelling
- probe: compared as-inherited src vs a clean pure-C rewrite; both sandboxed --disable all
- result: cheatful=16, clean=21; the 5-insn delta was entirely the illicit coercion of GCC's allocator and phantom-frame reservation
- verdict: KILLED

## [s2] The target's phantom -8 frame is a free artifact of the loop shape, not evidence of a dead local/array.
- mechanism: cc1's get_frame_size reserves 8 locals bytes for any compiler-recognised outer-loop shape (goto-form or real loop) in this function; the s1 hand-written do-while-with-explicit-guard was the anomaly at vars=0. No store is ever emitted into those bytes, matching target.
- probe: Read '.frame $sp,N # vars= N' straight out of the cc1 .s for ~40 source variants via tmp/grind/func_80034200/s2/probe.sh and the standalone sweep harness.
- result: vars=8 on every goto-form and real-loop variant; vars=0 only on the s1 form. Target's addiu $sp,-8 / addiu $sp,8 pair now reproduces with zero frame-coercion constructs.
- verdict: CONFIRMED

## [s2] The target's outer-loop-tail re-read of D_800A389B is reachable ONLY with a goto-form outer loop.
- mechanism: loop.c runs scan_loop only on loops delimited by NOTE_INSN_LOOP_BEG. A goto loop has no such note, so the tail lbu is never a movable. In a real loop the load is a single-set movable whose src is invariant_p (loop_store_mems empty, no calls), reg_in_basic_block_p is true, and the may_trap_p/maybe_never guard cannot fire because rtx_addr_can_trap_p(SYMBOL_REF) returns 0; move_movables' cost test passes trivially since threshold = 2*(1+n_non_fixed_regs).
- probe: while / for / do-while / goto spellings, bound inline vs via a local, tail test reordered; each cc1 .s inspected for a second 'lbu D_800A389B' and scored with sandbox --disable all.
- result: Reload present only in the goto form. Floor dropped 21 -> 20 -> 16 as the shape came in. Corollary banked: outer-LICM (which would place the preheader li 3) and the tail reload are mutually exclusive under this compiler.
- verdict: CONFIRMED

## [s2] Writing 'base += 2;' before 'i++;' in the loop tail lets cc1 fill the tail lbu's load-delay slot with the counter increment.
- mechanism: cc1 schedules addiu i,i,1 between the lbu and its consuming slt, so maspsx emits no nop. Instance of loop-counter-fills-load-delay.
- probe: 14-variant tail-order sweep reading the '#nop' count out of the cc1 .s, then sandbox --disable all.
- result: #nop 1 -> 0, build_insns 41 -> 40 == target_insns 40, floor 16 -> 14. The source-level reorder 's32 n = D_800A389B; i++;' does NOT achieve it; the lever is specifically which increment is written first.
- verdict: CONFIRMED

## [s2] The register rotation is an allocno-priority effect driven by reference COUNTS, and the target triple is pure-C reachable.
- mechanism: global.c:allocno_compare ranks by floor_log2(n_refs)*n_refs/live_length*10000*size. floor_log2 is a step function, so a single reference crossing a power of two reorders the whole allocation. Dropping the v0 temp (OR-ing straight into acc in both arms) lifts acc to the top of the priority list.
- probe: 35+ structural variants through a standalone-TU harness that reproduces the in-file codegen exactly; read the .greg ';; N regs to allocate:' order plus the emitted registers for shift/acc/base.
- result: Target's full triple (acc=$a0, base=$a1, shift=$a2) is produced exactly by the single-sllv variant, proving reachability. The 40-instruction-preserving version reaches acc=$a0 and drops the floor 14 -> 11. base<->shift is the one remaining swap.
- verdict: CONFIRMED

## [s2] Reusing one C variable for the useReal flag and the loop bound defeats the LICM hoist (n_times_set != 1).
- mechanism: Intended to make consec_sets_invariant_p fail so the tail load is not a movable.
- probe: Two variants where 'n' carries both the boolean and the count; cc1 .s inspected for a second lbu.
- result: Reload absent in both, and the allocation got worse (acc=$8, shift=$5). The load always lands in its own fresh temp pseudo, so the user variable's set count is irrelevant.
- verdict: KILLED

## [s2] Declaration order / init order / statement order can steer this function's register allocation.
- mechanism: allocno_compare falls back to allocno number (which follows pseudo creation, i.e. first-use order) only on an exact priority tie.
- probe: 14 declaration-order and init-order permutations plus 15 statement-level mutations (endp-from-p, 'p != end_p', shift-before-or, base = base + 2, s16 narrowing of useReal and innerBound, inlined bound, negated compare, advance-early, tail-order swaps).
- result: Byte-identical allocation in every single variant. The allocnos are not tied, so only reference-COUNT changes move the ordering.
- verdict: KILLED

## [s2] A preheader-bound constant holder ('emptyVal = 3;') closes the remaining li-3 placement and the base<->shift swap.
- mechanism: Binding the constant before the loop is the only way to place it in the preheader once outer-LICM is unavailable; it also lifts base above shift.
- probe: Two constant-holder variants (inner do-while and inner goto) built and scored with sandbox --disable all.
- result: It does place li 3 in the preheader at target's slot and does fix base to $a1, but it swaps innerBound/const3 ($10<->$11) and swaps shift/i ($a2<->$a3). Sandbox = 11, IDENTICAL to the clean pure-C form. The FAKE-family construct buys nothing, so no owner ruling is needed on it.
- verdict: KILLED

## [s2] Writing the inner loop as a goto loop too (lowering shift's loop_depth ref weight) fixes the base<->shift order.
- mechanism: flow.c counts reg_n_refs weighted by loop_depth; removing the inner loop's notes drops shift's inner refs from depth 2 to depth 1.
- probe: Two both-goto variants, cc1 .s inspected.
- result: It DOES flip base above shift, but with 3 unhoisted both arms end in an identical 'sll $2,$2,$sh' and jump.c's find_cross_jump merges them: build_insns 31 vs target 40. Only viable if 3 is already in a register before the inner loop.
- verdict: KILLED

## [s3] CONFIRMED (and s2's corollary KILLED) -- a `u8`-typed loop-bound variable keeps the tail reload inside a REAL loop
- mechanism: loop.c:scan_loop's movable gate needs one of (1) !maybe_never &&
  !loop_reg_used_before_p, (2) !REG_USERVAR_P && !REG_LOOP_TEST_P, (3)
  reg_in_basic_block_p. An `s32` bound forces a widening temp, which passes (2);
  a `u8` bound puts the load directly in the user variable, and with an earlier
  pre-loop reference of that variable (3) fails, while maybe_never (set past any
  label or jump, loop.c:921-930) kills (1).
- probe: variants b01/b02/b03/b05 (`s32 n`, four placements of the tail re-read)
  vs b04 (`u8 n`), standalone-TU harness, counting `lbu D_800A389B` occurrences.
- result: `s32` forms rel=n (hoisted) in all four; `u8` form rel=Y. Adopted, and
  the restored outer LICM then placed the preheader `li 3` for free: floor 11 -> 10.
- verdict: CONFIRMED. s2's banked "outer-LICM and the tail reload are mutually
  exclusive" is hereby KILLED -- it was true only for the s32 spelling it swept.

## [s3] CONFIRMED -- expressing `base` as a strength-reduced induction variable fixes both s2 residuals at once
- mechanism: loop_optimize calls move_movables before strength_reduce, so a giv's
  preheader initialisation is emitted after the hoisted invariants (giving
  target's [lbu, li 3, la] preheader, unreachable from any source-statement
  ordering); and the giv is a fresh high-priority allocno that takes $a1, pushing
  shift to $a2 and completing the target triple.
- probe: `base = &D_800F65F8 + i * 2;` inside the loop, replacing the pre-loop
  init plus the tail `base += 2;`; 4 giv spellings measured, then a 12-form
  ordering sweep.
- result: floor 10 -> 2, then -> 0 with `p = base;` written before
  `end_p = base + 2;`. sandbox --disable all = 0; verify-oracle ok, build SHA1 ==
  oracle.
- verdict: CONFIRMED -- this is the match.

## [s3] KILLED -- the s2 frontier's "change a reference COUNT to flip base>shift" programme
- mechanism: it was sound arithmetic (base needed >=8 refs, or shift <=6, at the
  measured live lengths) but it was solving the wrong problem: the swap is not a
  ref-count wall, it is an artifact of `base` being an ordinary pseudo instead of
  a giv.
- probe: read allocno_n_refs / allocno_live_length straight out of the `.lreg`
  dump (no instrumented cc1 needed) for the score-11 form, computed the required
  deltas, then reached the target triple by the giv route without touching any
  ref count.
- result: target triple reached with ref counts untouched.
- verdict: KILLED as a necessary axis. This also kills the frontier item that
  called for building an instrumented cc1 under tmp/gccdbg -- `-da` already emits
  the numbers.

## [s3] KILLED -- the decomp-permuter frontier item
- mechanism: it proposed seeding the permuter from the score-11 base to search
  the ref-count space.
- probe: not needed -- the function matched by hand-derived structural levers
  first. No permuter campaign was launched this session.
- verdict: KILLED (moot).
