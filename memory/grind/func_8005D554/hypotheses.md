# Hypothesis ledger — func_8005D554

Floor history: 65 (retired chassis, 2026-08-19) -> **6** (s1, 2026-09-08, this chassis).
Chassis: main @ fabcda5a, cc1 -mel -msoft-float, 0 regfix/asmfix rules.
Candidate in `candidate.c` (= `tmp/grind/func_8005D554/s1/p1_intorder.c`), 176/176 insns, frame 120 == target.

## s1 (recon) — measured

### H1 — Recovered WIP score-8 form transfers to the soft-float chassis. **CONFIRMED (65 -> 7).**
- mechanism: guard-in-loop-variable births the jump.c `duplicate_loop_exit_test` compare pseudo
  (phantom 8-byte slot = the frame +16); `p_b390 = p_b388 + 2` lifts `p_b388` to 4 refs so
  `local-alloc.c:1079` no longer marks it `reg_equiv_replacement` and global gives it s7;
  `p_b2ec` names the displaced base so `base_offset` loses the priority race and takes the
  sp+0x40 spill; split-init accumulation keeps `(rK - C)` as its own insn before the rnd add.
- probe: `s1/cand8.c` via `sandbox --disable all` -> 7. The old "group 3" residual
  (`%lo(D_8009B390)` vs `addiu a3,a3,8`) is absent on this chassis.

### H2 — Operand order of `s.p0 = base + stride` is fixed by an integer-typed sum. **CONFIRMED (7 -> 6).**
- mechanism: c-typeck `pointer_int_sum` canonicalizes ptr+int to PLUS(ptr,int) regardless of
  source order; an integer PLUS keeps source order, so `stride + (s32)p_b2e0` emits
  `addu v0,s0,fp` (target) instead of `addu v0,fp,s0`.
- probe: `s1/p1_intorder.c` -> 6.

### H3 — OBJECT MODEL: typed 0x3C-stride struct array for D_8009B2E0. **KILLED (instance).**
- statement: declaring `extern EffTmpl D_8009B2E0[]` and indexing `&D_8009B2E0[idx]` /
  `&D_8009B2E0[idx].sub[D_800A3418 & 1]` on the s1 candidate chassis scores 21 (vs 6).
- kill_scope: instance. measured_on: s1 candidate (guard-in-i, p_b2ec, p_b390 = p_b388 + 2,
  split-init x4, int-order p0), no FAKE constructs present. Form: `rejected/typed-struct-array-decl-scores-21.c`.
- result: array indexing re-derives `idx*60 + base` per use; the target keeps `stride` in s0
  and `base+0xC+stride` in sp+0x40 across the loop, which the byte-pointer model reproduces.

### H4 — Statement moves of the a2-site init within the rand()..call span. **KILLED (class).**
- statement: every placement of the `a2_offset = r4 - K` init statement between the
  a2-feeding `D_800A3418 ^= rand()` and the call fails the predicate "the a2-init insn's
  INSN_LUID exceeds the `a1 = 0` arg-load's INSN_LUID at sched1", because expand emits the
  arg-register loads (uids 240, 242) immediately before the call insn with nothing between,
  and sched1's tie-break among equal-priority, equal-class ready insns is INSN_LUID order.
- kill_scope: class. predicate_cite: `tools/gcc-2.7.2/sched.c:2464` (the LUID tie-break in
  `rank_for_schedule`); supporting: `tools/gcc-2.7.2/calls.c:1910` (`emit_queue` is the only
  thing emitted between the arg loads and the call).
- measured_on: exact sched_solver model of the s1 candidate (pass-1 block 6, parity=True),
  `s1/mutate.py` -> `s1/mutate_out.txt`: init luid 31 (as compiled), 36.5, 42.5 all give ours;
  luid 43.5 (> a1's 43) gives the target order in BOTH halves; a1-only-low or a0-only-low
  luid does not; both arg loads before the init does. Sandbox confirmations: `p2_a2init_late.c`
  (init adjacent to its `+=`) 7 = no change; `q2_stores_before_a2init.c` 6 = no change.
- result: the residual is NOT a statement-order lever inside that span. Two exact-model
  routes remain (frontier F1/F2).

### H5 — Struct stores before the a2-feeding rand. **KILLED (instance).**
- statement: moving `s.zero10 = 0; s.one14 = c1; s.ret = ret;` above the a2-site
  `D_800A3418 ^= rand()` on the s1 candidate scores 32.
- kill_scope: instance. measured_on: s1 candidate chassis, no FAKE constructs.
  Form: `rejected/struct-stores-before-a2-rand-scores-32.c`.

### H6 — One `off` variable for both offset sites. **KILLED (instance).**
- statement: merging `a0_offset`/`a2_offset` into a single local on the s1 candidate scores 12.
- kill_scope: instance. measured_on: s1 candidate chassis, no FAKE constructs.
  Form: `rejected/single-offset-var-both-sites-scores-12.c`.
- result: adds output/anti deps on the offset pseudo and changes the a0-site order too.

## Frontier (exact-model verified routes to the last 6 — see evidence.md s1 sched section)

F1. **The a2-site `r4 - K` insn must sit AFTER the call's arg-register loads in pre-sched1 RTL.**
    Mechanism: sched1 clock-64 tie among {a1=0 (242), a0=&s (240), init (211)} at priority 3 /
    class 3 resolves by INSN_LUID (`sched.c:2464`); simulator: init luid > 43 -> target order in
    both halves. GCC only emits insns after the arg loads via `emit_queue` (`calls.c:1910`,
    queued POSTINCREMENT/POSTDECREMENT side effects) or `prepare_call_address` (function-pointer
    calls). Next probe: spell the a2 value so its `r4 - K` insn is born by a queued
    post-decrement or inside the call expression, e.g. a pointer local walked with `p--`
    (element size 12 for the -0xC site does NOT generalize to -0x19/-0x32 — verify the
    constants before spending it), or a second-argument expression that yields 0 after
    computing the offset; measure with `sandbox` and re-run `s1/mutate.py`-style checks on a
    fresh `extract.py text1b`.

F2. **Alternative: the init is a single-set birth (LAUNCH priority, `sched.c:2505`
    `birthing_insn_p`, needs `reg_n_sets == 1`) AND the `s.zero10 = 0` store depends on it.**
    Simulator: `launch_dep228` reproduces the target in both halves; LAUNCH alone puts the
    init right before its add (wrong). A fresh once-written temp is a named-intermediate
    (FAKE-gated) unless it is the natural spelling; the dependence from the zero store is the
    hard part — no C-level read of `s.zero10` exists in the target bytes. Treat as the
    fallback route; do not spend it before F1's post-arg-load spellings are measured.

F3. **Confirm sched2 replays the fixed sched1 order.** Once a form changes the sched1 order,
    run `extract.py text1b` + `simulate.py --func func_8005D554 --pass 2` before trusting the
    sandbox delta; sched2 ties are pure LUID (all four insns priority 3 there).

## [s1] The 2026-08-05 WIP candidate_8 form (guard-in-i, p_b2ec, p_b390 = p_b388 + 2, split-init x4) transfers to the -msoft-float chassis
- mechanism: jump.c duplicate_loop_exit_test phantom slot for the frame +16; refs=4 keeps p_b388 off local-alloc.c:1079 reg_equiv_replacement so global seats it in s7 and base_offset spills to sp+0x40; split-init keeps (rK - C) as its own insn
- probe: sandbox func_8005D554 --disable all on tmp/grind/func_8005D554/s1/cand8.c
- result: 65 -> 7; old group-3 %lo(D_8009B390) residual absent on this chassis
- verdict: CONFIRMED

## [s1] An integer-typed sum stride + (s32)p_b2e0 emits the target operand order addu v0,s0,fp
- mechanism: c-typeck pointer_int_sum canonicalizes ptr+int to PLUS(ptr,int); integer PLUS keeps source operand order
- probe: sandbox on s1/p1_intorder.c
- result: 7 -> 6 (176/176, frame 120 == target)
- verdict: CONFIRMED

## [s1] Declaring D_8009B2E0 as a typed 0x3C-stride struct array and indexing &D_8009B2E0[idx] / .sub[D_800A3418 & 1] on the s1 candidate scores 21
- mechanism: array indexing re-derives idx*60 + base per use; target keeps stride in s0 and base+0xC+stride in the sp+0x40 spill
- probe: sandbox on s1/p4_typed_array.c
- result: 21 vs 6; OBJECT MODEL entry banked (MISMATCH measured 21)
- verdict: KILLED
- kill_scope: instance
- measured_on: s1 candidate chassis (guard-in-i, p_b2ec, p_b390 = p_b388 + 2, split-init x4, int-order p0), no FAKE constructs

## [s1] Every placement of the a2-site init statement between the a2-feeding D_800A3418 ^= rand() and the call fails the predicate luid(init) > luid(a1 = 0) that sched1 rank_for_schedule requires at the clock-64 pick
- mechanism: sched.c rank_for_schedule: equal priority 3, equal class 3 vs last_scheduled 228, then INSN_LUID; expand emits the arg-register loads immediately before the call so no earlier statement can out-LUID them
- probe: exact sched_solver model mutations (s1/mutate.py -> s1/mutate_out.txt): init luid 31/36.5/42.5 -> ours, 43.5 -> target; sandbox s1/p2_a2init_late.c = 7, s1/q2_stores_before_a2init.c = 6
- result: statement moves inside the span are inert; only an init born after the arg loads (F1) or a LAUNCH birth that the s.zero10 store depends on (F2) reproduce the target in the exact model
- verdict: KILLED
- kill_scope: class
- measured_on: sched_solver pass-1 model of the s1 candidate (text1b parity=True) plus sandbox on the s1 candidate chassis, no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/sched.c:2464

## [s1] Moving the s.zero10/one14/ret stores above the a2-site D_800A3418 ^= rand() on the s1 candidate scores 32
- mechanism: stores hoist above the rand call and change the block shape
- probe: sandbox on s1/p2b_stores_before_xor.c
- result: 32
- verdict: KILLED
- kill_scope: instance
- measured_on: s1 candidate chassis, no FAKE constructs

## [s1] Merging a0_offset and a2_offset into one local on the s1 candidate scores 12
- mechanism: adds output/anti deps on the shared pseudo and perturbs the a0-site order too
- probe: sandbox on s1/q3_single_off.c
- result: 12
- verdict: KILLED
- kill_scope: instance
- measured_on: s1 candidate chassis, no FAKE constructs

## s2 (structural) — measured

### H7 — The a2-site `r4 - K` insn can be born AFTER the call's arg-register loads (frontier F1). **KILLED (class).**
- statement: no C spelling can make the a2-site `a2 = r4 - K` insn carry an INSN_LUID greater
  than the `a1 = 0` arg-load's, because `expand_call` emits nothing between
  `load_register_parameters` and the call except `emit_queue`, and the only insns `emit_queue`
  can flush are `expand_increment`'s queued post-inc/dec insns, whose destination is the
  incremented variable and whose result is by construction NOT the value the enclosing
  expression yields — whereas the a2 residual insn's result is consumed by the `addu` that
  follows it in the same iteration.
- mechanism: `tools/gcc-2.7.2/calls.c:1909-1910` (`emit_queue` is the only thing between the
  arg loads and the call); `tools/gcc-2.7.2/expr.c:8641` (`enqueue_insn (op0, GEN_FCN (icode)
  (op0, op0, op1))` — the sole enqueue site, destination == op0); `expr.c:402-418`
  (`protect_from_queue` returns the pre-increment copy as the expression value);
  `tools/gcc-2.7.2/sched.c:2464` (the LUID tie-break that makes the LUID the deciding quantity).
- kill_scope: class. predicate_cite: tools/gcc-2.7.2/expr.c:8641
- measured_on: s1/s2 candidate chassis (guard-in-i, p_b2ec, p_b390 = p_b388 + 2, split-init x4,
  int-order p0), no FAKE constructs; sandbox confirmations v1/v2/v3/v4 (6/15/15/15) plus s1's
  exact sched_solver model runs (s1/mutate_out.txt).
- result: additionally, the arithmetic forbids the queued-post-decrement shape independently —
  the two a2 sites need `r4 - 0xC` and `r4 - 0x19` off a loop-invariant `r4`, a delta of 13
  with a per-iteration reset, which is not a monotone increment chain. F1 is retired from the
  frontier.

### H8 — F2's `reg_n_sets == 1` LAUNCH-birth precondition is reachable without adding instructions. **KILLED (instance).**
- statement: on the s2 chassis the three measured spellings that give the `r4 - K` value its
  own once-written carrier — per-site fresh locals feeding an accumulator (`v5_fresh_singleset`),
  per-site fresh locals whose sum is stored straight into `s.zero1C` (`w1_singleset_direct`),
  or accumulating in the address-taken struct field itself (`w4_field_accum`) — compiles to
  178 instructions instead of 176 and scores 54.
- mechanism: sched.c:2505 `birthing_insn_p` needs `reg_n_sets == 1` on the set register; the
  candidate's split-init writes `a2_offset` four times. Introducing a once-written carrier
  makes GCC keep an extra pseudo/memory copy that neither combine nor the allocator removes.
- kill_scope: instance. measured_on: s2 chassis == s1 candidate chassis, no FAKE constructs.
  Forms: `rejected/fresh-singleset-a2-base-adds-2-insns-scores-54.c`,
  `rejected/accumulate-in-zero1C-field-adds-2-insns-scores-54.c`.
- result: F2's first prong is unaffordable at 176/176 before its second prong (a dependence of
  the `s.zero10 = 0` store on the init) is even reachable. F2 drops off the frontier as a
  C-spelling route.

### H9 — The a2 accumulation must stay split into `= r4 - K` then `+= rnd`. **CONFIRMED (re-confirmed on s2).**
- statement: any single-expression spelling of the a2 sum (`((s32)r4 - K) + rnd`,
  `rnd + ((s32)r4 - K)`) or any spelling that moves the `- K` into the `+=` operand
  (`a2_offset = rnd; a2_offset += (s32)r4 - K;`) scores 15 instead of 6.
- mechanism: fold reassociates `(r4 - K) + rnd` into `r4 + (rnd - K)`, destroying the
  target's standalone `addiu a2,s4,-K`.
- probe: sandbox on s2/v2, v3, v4 — all 15/176. Forms:
  `rejected/unsplit-a2-sum-reassociates-scores-15.c`, `rejected/rnd-first-a2-accum-scores-15.c`.

### H10 — A structural lever (declaration order, block scoping, type narrowing, extra split, store order) moves the last-6 residual. **KILLED (instance).**
- statement: on the s2 chassis eleven structural spellings all score exactly 6 with 176
  instructions — three-way split of the a2 accumulation, `a2_offset` declared before
  `a0_offset`, `a2_offset` declared first of all locals, `r4`/`r5` declared `s32` instead of
  `u32`, `a2_offset` typed `u32`, distinct accumulators per a2 site, the `s.zero1C` store moved
  ahead of the `zero10/one14/ret` group, offset locals moved to do-while block scope, and a
  nested block per loop half with its own offset locals.
- mechanism: none of these changes the relative emission order of the a2 init and the arg
  loads within the block, which is the only quantity the clock-64 LUID tie reads.
- kill_scope: instance. measured_on: s2 chassis == s1 candidate chassis, no FAKE constructs.
  Forms in `tmp/grind/func_8005D554/s2/` (v1, v6, v7, w3, w5, w6, x1, x2, x3); table in
  evidence.md s2.
- result: the structural modality is measured out on this residual. `w2_shared_direct`
  (one shared base var, sum stored straight into the field) is the only non-neutral non-fatal
  data point at 10.

### H11 — Sibling `main` (src/ings.c, COMPLETED-C) carries a transplantable spelling. **KILLED (instance).**
- statement: src/ings.c contains no reference to D_800A326C, D_800A3418, D_8009B2E0 or
  func_80073728, so it shares no block, global or callee with func_8005D554 and has no
  spelling to transplant.
- kill_scope: instance. measured_on: HEAD main @ 8d3c3235 working tree, grep over src/ings.c.

## Frontier after s2 (F1 and F2 are retired; see H7/H8)

G1. **Re-derive the loop-body statement order from the target's scheduling constraints rather
    than from our current chassis.** Everything at 176/176 with only a 3-insn rotation left in
    each half means the whole rest of the function is right; the target must place the a2 init
    later for a reason our RTL does not reproduce, and since it cannot be LUID (H7) or a
    LAUNCH birth (H8) it must be a *dependence* our version lacks or *has* spuriously. Next
    probe: run `tools/sched_solver` extract/simulate on the current candidate and diff the
    pass-1 dependence lists (not just priorities) for uids 211/240/242/205 against what the
    target order requires; look specifically for an anti-dependence our a2 pseudo carries
    because it is written four times (H8 says removing the multi-write costs insns, but the
    solver can say whether the anti-dep is what forces the early pick).

G2. **Attack the register assignment of the a2 pseudo instead of the schedule.** The target
    computes the value in `a2` (caller-saved, dead across nothing) and stores it in the call's
    delay slot; if our pseudo lands in a different hard register the dependence graph around
    the arg loads changes. Next probe: read `.greg`/`.lreg` (`pwsh tools/grinder/dump.ps1
    func_8005D554`) for the a2 pseudo's assignment on the candidate and compare with the
    target's `a2`; if ours is not `a2`, find the C-level lever that changes the allocno
    ordering (the `p_b2ec` / `p_b390 = p_b388 + 2` refs-count technique from s1 is the
    in-family precedent for this function).

G3. **`rederive` modality: re-open the loop chassis itself.** Every s1/s2 probe kept the
    guard-in-i + do-while chassis inherited from the 2026-08-05 WIP. That chassis was chosen
    because it produces the target's 120-byte frame, but the frame is now settled and other
    loop shapes were never re-measured on the soft-float chassis. Next probe: re-measure a
    `for`-loop and a `while`-loop chassis with the same body and check both the frame and the
    12-insn residual — this does NOT evade H7 (the arg loads are emitted
    last whatever the loop shape); its value is that a different loop shape changes the
    block's dependence graph and register pressure, which is what G1/G2 are after.

### H12 — The a2 pseudo is seated in a different hard register than the target (frontier G2). **KILLED (instance).**
- statement: on the s2 chassis the candidate's residual region uses exactly the target's
  registers — `$6`/a2 for the offset, `$20`/s4 for r4, `$4`/a0, `$5`/a1, `$3`/v1, `$22`/s6,
  `$17`/s1 — so no register-allocation difference remains anywhere in the function.
- mechanism: none needed; the cc1 `.s` dump and the target asm agree operand for operand.
- probe: `pwsh tools/grinder/dump.ps1 func_8005D554`, compared
  `tmp/grind/func_8005D554/dumps/text1b.s` (body lines 134-147) against
  `asm/funcs/func_8005D554.s:93-107`.
- kill_scope: instance. measured_on: s2 chassis == s1 candidate chassis, no FAKE constructs.
- result: G2 is retired before it was spent. The last 6 is a pure emission-order residual.

### H13 — The target's `addiu a2,s4,-K` sits after the arg loads because it is a THIRD call argument. **KILLED (class).**
- statement: func_80073728 never reads `$a2` as an incoming argument — the only `$a2`
  references in its body are `mflo $a2` (writes), so its ABI arity is (a0, a1) and no C
  spelling can route the offset through a third parameter to get its insn emitted by
  `load_register_parameters`.
- mechanism: MIPS o32 argument registers are a0-a3 in order; an unread `$a2` at function entry
  means the parameter does not exist.
- probe: `grep -n '\$a2' asm/funcs/func_80073728.s` -> only :194 and :235, both `mflo`.
- kill_scope: class. predicate_cite: asm/funcs/func_80073728.s:194
- measured_on: HEAD main @ 8d3c3235, the shipped callee body; independent of our chassis and
  of any FAKE construct.

## [s2] No C spelling can make the a2-site 'a2 = r4 - K' insn carry an INSN_LUID greater than the 'a1 = 0' arg-load's, because expand_call emits nothing between load_register_parameters and the call except emit_queue, and the only insns emit_queue can flush are expand_increment's queued post-inc/dec insns, whose destination is the incremented variable and whose result is by construction not the value the enclosing expression yields, while the a2 residual insn's result is consumed by the addu two insns later in the same iteration.
- mechanism: tools/gcc-2.7.2/calls.c:1909-1910 (emit_queue is the only thing between the arg loads and the call); tools/gcc-2.7.2/expr.c:8641 (enqueue_insn (op0, GEN_FCN (icode) (op0, op0, op1)) is the sole enqueue site, destination == op0); tools/gcc-2.7.2/expr.c:402-418 (protect_from_queue returns the pre-increment copy as the expression value); tools/gcc-2.7.2/sched.c:2464 (the LUID tie-break at the sched1 clock-64 pick that makes the LUID the deciding quantity).
- probe: Read the GCC 2.7.2 sources at the three sites above; sandbox-measured the four a2 accumulation spellings that move the '- K' toward the call (v1_three_split 6, v2_rnd_first 15, v3_unsplit_kfirst 15, v4_unsplit_rndfirst 15), on top of s1's exact sched_solver mutation runs (s1/mutate_out.txt) which showed init luid 31/36.5/42.5 give our order and 43.5 gives the target's.
- result: Frontier item F1 is retired. Additionally the arithmetic forbids the queued-post-decrement shape independently: the two a2 sites need r4 - 0xC and r4 - 0x19 off a loop-invariant r4, a delta of 13 with a per-iteration reset, which is not a monotone increment chain.
- verdict: KILLED
- kill_scope: class
- measured_on: s1/s2 candidate chassis (guard-in-i, p_b2ec, p_b390 = p_b388 + 2, split-init x4, int-order p0), no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/expr.c:8641

## [s2] On the s2 chassis the three measured spellings that give the r4 - K value its own once-written carrier - per-site fresh locals feeding an accumulator (v5_fresh_singleset), per-site fresh locals whose sum is stored straight into s.zero1C (w1_singleset_direct), and accumulating in the address-taken struct field itself (w4_field_accum) - each compile to 178 instructions instead of 176 and score 54.
- mechanism: sched.c:2505 birthing_insn_p needs reg_n_sets == 1 on the set register for LAUNCH priority; the candidate's split-init writes a2_offset four times. Introducing a once-written carrier makes GCC keep an extra pseudo/memory copy that neither combine nor the allocator removes, and s is address-taken so the struct field cannot live in a register.
- probe: sandbox func_8005D554 --disable all on tmp/grind/func_8005D554/s2/{v5_fresh_singleset.c,w1_singleset_direct.c,w4_field_accum.c}: 54/178, 54/178, 54/178.
- result: Frontier item F2's first prong (reg_n_sets == 1) costs +2 instructions before its second prong (a dependence of the s.zero10 = 0 store on the init) is even reachable, so F2 drops off the frontier as a C-spelling route. Forms banked in rejected/.
- verdict: KILLED
- kill_scope: instance
- measured_on: s2 chassis == s1 candidate chassis (guard-in-i, p_b2ec, p_b390 = p_b388 + 2, split-init x4, int-order p0), no FAKE constructs present

## [s2] On the s2 chassis eleven structural spellings each score exactly 6 with 176 instructions: three-way split of the a2 accumulation, a2_offset declared before a0_offset, a2_offset declared first of all locals, r4/r5 declared s32 instead of u32, a2_offset typed u32, distinct accumulators per a2 site, the s.zero1C store moved ahead of the zero10/one14/ret group, offset locals moved to do-while block scope, and a nested block per loop half with its own offset locals.
- mechanism: None of these changes the relative emission order of the a2 init and the arg-register loads within the scheduling block, which is the only quantity the sched1 clock-64 LUID tie reads.
- probe: sandbox func_8005D554 --disable all on tmp/grind/func_8005D554/s2/{v1_three_split,v6_declorder,v7_r4_signed,w3_distinct_split,w5_store_order,w6_decl_first,x1_blockscope,x2_perhalf_blocks,x3_a2_unsigned}.c - all 6/176; full table in evidence.md s2.
- result: The structural modality is measured out on this residual. w2_shared_direct (one shared base var, sum stored straight into the field) at 10/176 is the only non-neutral non-fatal data point.
- verdict: KILLED
- kill_scope: instance
- measured_on: s2 chassis == s1 candidate chassis, no FAKE constructs present

## [s2] Any single-expression spelling of the a2 sum, or any spelling that moves the '- K' into the '+=' operand, scores 15 instead of 6 on the s2 chassis because fold reassociates (r4 - K) + rnd into r4 + (rnd - K) and destroys the target's standalone addiu a2,s4,-K.
- mechanism: GCC 2.7.2 fold reassociation of PLUS(MINUS(reg, const), reg); the split-init accumulation form is what keeps the constant subtraction as its own insn.
- probe: sandbox on s2/v2_rnd_first.c, s2/v3_unsplit_kfirst.c, s2/v4_unsplit_rndfirst.c - 15/176 each.
- result: Re-confirms on this chassis that the split-init accumulation is load-bearing for the a2 sites; both forms banked in rejected/.
- verdict: CONFIRMED

## [s2] The candidate's hard-register assignment in the residual region is identical to the target's: $6/a2 for the offset, $20/s4 for r4, $4/a0, $5/a1, $3/v1, $22/s6, $17/s1, so no register-allocation difference remains in this function.
- mechanism: None needed - the cc1 .s dump and the shipped target asm agree operand for operand across the whole residual window.
- probe: pwsh tools/grinder/dump.ps1 func_8005D554, then compared tmp/grind/func_8005D554/dumps/text1b.s (func body lines 134-147) against asm/funcs/func_8005D554.s:93-107.
- result: Frontier item G2 (attack the a2 pseudo's seat) is retired before being spent; the last 6 is a pure emission-order residual, not an RA residual.
- verdict: KILLED
- kill_scope: instance
- measured_on: s2 chassis == s1 candidate chassis, no FAKE constructs present

## [s2] func_80073728 never reads $a2 as an incoming argument - the only $a2 references in its shipped body are mflo writes - so its ABI arity is (a0, a1) and the offset cannot be routed through a third parameter to get its insn emitted by load_register_parameters.
- mechanism: MIPS o32 passes arguments in a0-a3 in order; an argument register that is never read before being written at function entry is not a parameter.
- probe: grep -n '\$a2' asm/funcs/func_80073728.s -> only :194 and :235, both mflo $a2. Consistent with extern s32 func_80073728(s32, s32); at src/text1b.c:2642 and every other call site in the file.
- result: Kills the reading that the target's addiu a2,s4,-K sits after the a0/a1 loads because it is a third argument; a2 is a scratch seat for the offset pseudo in both builds.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 8d3c3235, the shipped callee body; independent of our chassis and of any FAKE construct
- predicate_cite: asm/funcs/func_80073728.s:194

## [s2] Sibling main in src/ings.c (COMPLETED-C, floor 0) carries a spelling transplantable onto this chassis.
- mechanism: The sibling sweep named it because its ledger names func_8005D554, not because of shared code.
- probe: grep -n 'D_800A326C|D_800A3418|D_8009B2E0|func_80073728' src/ings.c -> zero hits.
- result: src/ings.c shares no global, callee or block with func_8005D554; there is nothing to transplant. Recorded so no later session re-opens the sibling.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 8d3c3235 working tree, source grep

## s3 (permuter) — measured. FLOOR 6 -> 0.

### H12 — Random permutation of the s1/s2 candidate chassis closes the residual. **KILLED (instance).**
- statement: a decomp-permuter campaign seeded on the s1/s2 candidate chassis ran 21,569
  iterations with 8 jobs and produced zero output directories.
- mechanism: the candidate's block is already 176/176 with target-identical register
  allocation, so almost every random mutation the permuter can express either changes the
  instruction count or is codegen-neutral; the residual needs a specific two-part edit
  (statement hoist + a second write of the carrier) that random single-site mutation on
  this chassis did not reach.
- kill_scope: instance. measured_on: s1/s2 candidate chassis, no FAKE constructs;
  `tmp/perm_5d554` campaign telemetry (metrics/events.jsonl, label s3-candidate-chassis).

### H13 — Re-seeding a structurally different chassis makes the basin yield. **CONFIRMED.**
- statement: the same permuter configuration seeded on the `z3` chassis (the loop guard
  duplicated outside the loop AND used as a `while` test, floor-equivalent at 6/176 but
  with a different setup-block emission order) produced a find at ~413 s / 14,701
  iterations: permuter score 310 -> 160, sandbox 6 -> 3 at 176 instructions.
- probe: `tmp/perm_5d554_z3/output-160-1/source.c`, sandbox-measured as
  `tmp/grind/func_8005D554/s3/f1_perm160.c` = 3.
- result: the find closed loop half 1 only; hand-mirroring it to half 2 closes the
  function (H15). The chassis-rule discipline (permute a structurally different chassis
  rather than re-seeding a banked one) is what produced this.

### H14 — Frontier item 2: a different loop chassis moves the residual. **KILLED (instance).**
- statement: on this chassis a plain `while` loop, a `for` loop, and a `for` loop with the
  increment in the increment slot all compile to 174 instructions and score 25, because
  they lose the jump.c `duplicate_loop_exit_test` phantom frame slot; the only loop shape
  that keeps 176/176 is one whose guard is duplicated (do-while, or `while` with the guard
  also written outside), and that shape scores 6 — the same residual as the do-while.
- kill_scope: instance. measured_on: s1/s2 candidate chassis body, no FAKE constructs.
  Forms: `rejected/while-chassis-without-duplicated-guard-scores-25.c`, plus s3/z2, s3/z4.
- result: the guard-duplicated `while` (z3) is not a floor improvement but IS a
  structurally different emission order, and it is the seed that made the permuter yield.

### H15 — The last-6 residual closes with a fresh per-half staging carrier that is written twice. **CONFIRMED (6 -> 0).**
- statement: giving each loop half a fresh local that receives the a2-site base
  `(s32)r4 - K` at the position between `a0_offset += ...` and `s.zero18 = a0_offset;`,
  and that is written a second time in the same half with another real, immediately-read
  value (`nv = ret; s.ret = nv;` in the saved candidate, or `nv = 0; s.zero10 = nv;`),
  produces honest sandbox distance **0** at 176/176 instructions.
- mechanism: `tools/gcc-2.7.2/sched.c:2505` `birthing_insn_p` gives a single-set pseudo's
  defining insn LAUNCH priority; the second write makes `reg_n_sets > 1`, so the base insn
  keeps ordinary priority and the clock-64 ready-set tie at `sched.c:2464` resolves the
  way the target's does. The statement position supplies the INSN_LUID the tie needs, and
  the second write is what stops GCC from keeping the extra pseudo copy that costs the +2
  instructions H8 measured.
- probe: `sandbox func_8005D554 --disable all` on `s3/g23_stage_ret.c`, `s3/g17_perhalf_base.c`
  and `s3/g5_cross_reuse.c` — all 0/176. Full ablation table in evidence.md s3.

### H16 — The closing carrier can be an EXISTING local (the sanctioned staged-value quadrant). **KILLED (instance).**
- statement: every measured spelling that borrows a local the function already has —
  `v0` and `v3` (dead after the prologue) in both the per-half and the cross-half layout,
  `a0_offset` after its `s.zero18` store, and `a2_offset` itself reused for the `s.zero10`
  store — compiles to 178 instructions and scores 35/61/63/31; and every spelling that
  keeps the fresh carrier single-set (one carrier per value, base and zero in separate
  once-written locals, one shared carrier for all four a0/a2 sites) also compiles to 178.
- mechanism: the borrow sites that are legal (i.e. where the borrowed variable's previous
  value is dead) are all AFTER the `s.zero18 = a0_offset;` store, and the position between
  `a0_offset += ...` and that store — the only position that supplies the needed LUID — is
  a point where every existing local is either live or loop-carried.
- kill_scope: instance. measured_on: s1/s2 candidate chassis, no FAKE constructs. Forms:
  `rejected/borrow-existing-v0v3-carriers-scores-63.c`,
  `rejected/borrow-a0offset-after-zero18-store-scores-35.c`,
  `rejected/a2offset-reused-for-zero10-scores-31.c`,
  `rejected/base-and-zero-in-separate-locals-scores-54.c`,
  `rejected/single-staged-base-for-all-four-sites-scores-35.c`,
  `rejected/fresh-per-half-base-no-second-write-scores-54.c`,
  `rejected/plain-a2-base-reorder-adds-2-insns-scores-31.c`.
- result: this is exactly why the session returns `ruling-request` rather than
  `candidate-ready` — `staged-value-reused-variable.md` bound 2 excludes an INVENTED
  carrier, and the only carriers that close the function are invented.

### H17 — Frontier item 3: a different source expression for the a2 base moves its insn. **KILLED (instance).**
- statement: `(s32)r4 + -K` and `(s32)(r4 - K)` both score 6/176 (neutral); carrying the
  base as a `u8 *` walked by `- K`, or holding `(s32)r4` in a second loop-invariant local,
  both score 8 at 177 instructions.
- kill_scope: instance. measured_on: s1/s2 candidate chassis, no FAKE constructs.
  Forms: `rejected/second-invariant-local-for-r4-scores-8.c`, s3/y2, y3, y4.

### H18 — The closing carrier can be the existing `ret` local (staged-value bound 2 satisfied). **KILLED (instance).**
- statement: on the s3 candidate chassis, the three measured spellings that borrow the
  function's existing `ret` local as the a2-base carrier — hoisting `s.ret = ret;` to sit
  (a) immediately before `ret = (s32)r4 - K` at the required position, (b) above the
  a0-site `D_800A3418 ^= rand()`, or (c) to the top of each loop half — score 33 (176, 175
  and 175 instructions respectively).
- mechanism: `ret` is the ONLY existing local whose previous value is dead at the required
  staging position (between `a0_offset += ...` and `s.zero18 = a0_offset;`) once its
  `s.ret` store is hoisted; every other local there is loop-carried (`i`, `stride`, `c100`,
  `c1`, `r4`, `r5`, `p_b2e0`, `p_b2ec`, `p_b388`, `p_b390`, `base_offset`) or live
  (`a0_offset`). Notably `ret` does NOT pay the +2-instruction penalty that the `v0`/`v3`
  and `a0_offset` borrows pay (h1 compiles at 176 == target), so the multi-set/priority
  mechanism is satisfied by the borrow; the residual 33 is entirely the displaced
  `s.ret` store, whose target position is fixed between the `one14` and `zero1C` stores
  (asm/funcs/func_8005D554.s:91-109, s2 evidence).
- kill_scope: instance. measured_on: s3 candidate chassis (= candidate.c with the fresh
  `nv`/`nw` carriers replaced by `ret`), no FAKE constructs present.
  Form: `rejected/borrow-ret-hoisted-sret-store-scores-33.c`; probes
  `tmp/grind/func_8005D554/s3/h1_ret_carrier.c`, `h2_ret_carrier_early.c`, `h4_ret_carrier_top.c`.
- result: taken with H16, every existing local that is dead or can be made dead at the
  required staging position has now been measured on this chassis, and none reaches 0.
  The only measured spellings at 0 use a FRESH (invented) multi-set carrier, which
  `staged-value-reused-variable.md` bound 2 places outside that family while
  `defeat-licm-hoist-var-reuse.md` (whose shipped `s32 tmp;` shape is exactly ours, see
  src/code6cac_c2.c:1360-1365) is scoped to the loop.c hoist mechanism, not sched.c's
  `birthing_insn_p`. Hence the s3 `ruling-request`.

## s3b (permuter, post-Judge-FAIL re-dispatch) — measured. FLOOR 6 (candidate.c reset to the clean form).

Context: the s3 `ruling-request` body (fresh multi-write carriers `nv`/`nw`, distance 0) was
FAILed by the Judge on 2026-09-08 (docs/grind/decisions.md, "2026-09-08 22:31 — func_8005D554").
Binding constraint carried forward: no fresh (invented) local may be written more than once to
act as a staging carrier for the a2-site base `(s32)r4 - K`, or for any other value here, under
any name. `candidate.c` is therefore reset to the clean 176/176 score-6 body (s2's form); the
FAILed body is banked verbatim at
`rejected/judge-failed-fresh-multiwrite-nv-nw-carrier-scores-0.c` and must never be resubmitted
(the driver keys review verdicts by BODY).

### H19 — Merging the staging carrier INTO the accumulator (per-half a2 accumulators whose base init is hoisted to the pre-`s.zero18` slot) reproduces the winning shape without an invented carrier. **KILLED (instance).**
- statement: on the s3b chassis, replacing the shared `a2_offset` with per-half accumulators
  `a2a`/`a2b` and hoisting each half's `a2X = (s32)r4 - K;` to the slot between
  `a0_offset += ...` and `s.zero18 = a0_offset;` — so the accumulator itself is the multi-set
  variable and no staging local exists — compiles to 178 instructions and scores 47.
- mechanism: the hoisted base is live across the `rand()` call, so it needs a callee-saved
  seat; the function already uses all nine (s0-s7 + fp) and so does the target
  (`asm/funcs/func_8005D554.s:4DD5C-4DD84` saves s0-s7, fp, ra — a 0x78 frame, same as ours).
  The tenth long-lived value evicts `p_b2e0`, which GCC then rematerializes with extra
  `lui/addiu` pairs in the pre-loop block: +2 instructions.
- probe: `sandbox func_8005D554 --disable all` on `tmp/grind/func_8005D554/s3/k1_perhalf_acc_hoist.c`
  -> 47/178; objdump pairdiff of `s3/k0.asm` vs `s3/k1.asm` shows exactly the eviction
  (k0: `lui s8/addiu s8` + `lui s7/addiu s7` once in the pre-loop; k1: three `lui a2/addiu a2`
  pairs, with s0/s1 now holding a2a/a2b).
- kill_scope: instance. measured_on: s3b chassis == s1/s2 candidate chassis (guard-in-i,
  p_b2ec, p_b390 = p_b388 + 2, split-init x4, int-order p0), no FAKE constructs present.
  Form: `rejected/perhalf-accumulator-hoisted-base-scores-47.c`.
- result: **this overturns the s3 mechanism story.** H15 attributed the win to
  `reg_n_sets > 1` defeating `sched.c:2505 birthing_insn_p`; `a2a` here is written twice at the
  same statement positions (reg_n_sets == 2) and still misses by 47. The distinguishing
  property of the FAILed `nv`/`nw` forms is the SEPARATE pseudo copied into the accumulator,
  not the write count. A future session must not re-derive the reg_n_sets reading.

### H20 — The +2 that every hoisted spelling pays is register pressure, and freeing one callee-saved seat buys it back. **CONFIRMED (as a mechanism); the resulting forms KILLED (instance).**
- statement: with the ordinary-C hoist in place (`g7_reorder_only`, 178/31), deleting one
  callee-saved-resident local restores 176 instructions exactly: dropping the
  `p_b390 = p_b388 + 2` local scores 27/176 (`k9`), dropping the `p_b2e0` local scores 28/176
  (`k11`). Symmetrically, deleting the same local WITHOUT the hoist costs two instructions:
  `k10` (no hoist, no p_b390) is 174/45 and `k12` (no hoist, no p_b2e0) is 174/47.
- mechanism: the seat budget is exactly nine callee-saved registers in both our build and the
  target; a hoisted base occupies a tenth, and each freed pointer local returns one.
- probe: `sandbox` on `s3/{k9_g7_nopb390.c, k11_g7_nopb2e0.c, k10_k0_nopb390.c, k12_k0_nopb2e0.c}`.
- kill_scope: instance (for the forms). measured_on: s3b chassis, no FAKE constructs.
  Forms: `rejected/hoist-plus-freed-pb390-seat-scores-27.c`,
  `rejected/hoist-plus-freed-pb2e0-seat-scores-28.c`.
- result: buying the instructions back does NOT buy the target order. `k11`'s objdump
  (`s3/k11.asm`, body insn 75) puts the base at `addiu s0,s4,-12` in the second `rand` delay
  slot — held in a CALLEE-SAVED register across the call — while the target computes it in
  caller-saved `$a2` AFTER the call (`asm/funcs/func_8005D554.s:4DEC0`). Every source-level
  hoist is therefore structurally unlike the target, even the FAILed one that matched bytes:
  in the `nv`/`nw` forms some pass deletes the early insn and re-materializes it after the arg
  loads. Naming that pass is the next session's job (see frontier P1).

### H21 — Removing the `c1` / `c100` constant-holder locals relieves enough pressure for the hoist. **KILLED (instance).**
- statement: on the hoisted (`g7`) chassis, spelling `s.one14 = 1;` instead of the `c1` holder
  scores 32/178, spelling `s.c24 = 0x100; s.c20 = 0x100;` instead of the `c100` holder scores
  33/178, and dropping both scores 34/178 — none recovers an instruction. (Control: dropping
  `c1` on the un-hoisted chassis scores 8/176, i.e. it is mildly harmful on its own.)
- mechanism: both constants are re-loaded per use rather than freeing a seat; the target itself
  keeps the 1 in a callee-saved register (`sw $s6, 0x24($sp)`, `asm/funcs/func_8005D554.s:4DEC8`).
- kill_scope: instance. measured_on: s3b chassis, no FAKE constructs.
  Forms: `rejected/hoist-without-c1-holder-scores-32.c`; probes
  `tmp/grind/func_8005D554/s3/{k4_g7_noc1.c,k5_g7_noc100.c,k7_g7_noc1_noc100.c,k8_k0_noc1.c}`.

### H22 — A permuter campaign seeded on the ordinary-C hoist chassis (`g7`, 178 insns) finds the register-pressure relief that closes it. **KILLED (instance).**
- statement: a campaign on `tmp/perm_5d554_g7` (base.c = `g7_reorder_only`, permuter base score
  940) ran 24,879 iterations with 8 jobs over ~13 minutes and improved only to 655; the z3
  campaign that produced the s3 find started at 310 and reached 160.
- mechanism: at 178 instructions the chassis is two deletions plus a rotation away from the
  target, outside the single-site mutation radius the permuter samples; the score landscape
  between 940 and 655 is all pointer-rematerialization noise.
- kill_scope: instance. measured_on: s3b chassis, no FAKE constructs;
  campaign telemetry in `metrics/events.jsonl`, label `s3b-g7-ordinary-hoist-chassis`;
  finds under `tmp/perm_5d554_g7/output-*`.
- result: three chassis are now spent for permutation — the s1/s2 candidate chassis (21,569
  iters, 0 finds), the `z3` guard-duplicated while (30,891 iters, the banned `new_var` find),
  and `g7` (24,879 iters, best 655). A fourth seed must be a 176/6-class chassis that is
  structurally different from both k0 and z3.

## Frontier after s3b

P1. **Name the pass that re-materializes `addiu a2,s4,-K` after the call in the `nv`/`nw` forms.**
    Mechanism: the FAILed `g17`/`g23` bodies put `nv = (s32)r4 - K;` BEFORE the `rand()` call in
    source, yet their output computes the base in caller-saved `$a2` AFTER the call — so some
    pass (cse.c re-materialization at the copy `a2_offset = nv;`, combine.c folding the copy,
    or flow.c deleting the now-dead early set) moves the arithmetic down, and the insn is
    re-emitted with a LUID above the arg loads. This is the only known mechanism that satisfies
    H4/H7's LUID predicate, and H7's class kill (no C spelling can raise that LUID) is
    contradicted by the measurement — re-open it. Next probe: `pwsh tools/grinder/dump.ps1
    func_8005D554` with `s3/g17_perhalf_base.c` applied and again with
    `s3/k1_perhalf_acc_hoist.c` applied; diff the `.cse`, `.combine` and `.flow` dumps around
    the a2 base insn and identify the exact transformation. Once the pass is named, look for an
    ORDINARY-C construct that triggers the same transformation on the accumulator itself (a
    real copy the program needs, or a sub-expression CSE already has available) rather than an
    invented carrier.

P2. **Attack the sched1 clock-64 tie from the PRIORITY side instead of the LUID side.**
    Mechanism: `sched.c:2464` reaches the INSN_LUID tie-break only when priority and class are
    equal. Every session so far has tried to raise the a2 init's LUID; nobody has tried to
    LOWER its `INSN_PRIORITY` (shorten its path to the block end) or to RAISE the a0/a1 arg
    loads'. Next probe: run `tools/sched_solver` extract/simulate pass 1 on `candidate.c` and
    enumerate, in the exact model, which priority perturbation on uids 211/240/242 flips the
    clock-64 pick; only then look for a C spelling producing it at 176 insns.

P3. **A fourth permuter seed: a stacked-neutral 176/6 chassis.** All of the s2 neutral
    spellings (`x2` per-half blocks, `v6` declaration order, `w5` store order, `z3`
    guard-duplicated while) score 6 at 176; stacking three or four of them gives a chassis with
    a different emission order that is still one edit from the goal, unlike `g7`. Seed that,
    and vet any find hard: the permuter's attractor in this function is the banned
    fresh-multi-write carrier, and a find that spells one is not a candidate.

## [s3] On the s3b chassis, replacing the shared a2_offset with per-half accumulators a2a/a2b and hoisting each half's a2X = (s32)r4 - K to the slot between a0_offset += ... and s.zero18 = a0_offset -- so the accumulator itself is the multi-set variable and no staging local exists -- compiles to 178 instructions and scores 47.
- mechanism: The hoisted base is live across the rand() call so it needs a callee-saved seat; the build already uses all nine (s0-s7 + fp) and so does the target (asm/funcs/func_8005D554.s:4DD5C-4DD84 saves s0-s7, fp, ra into a 0x78 frame). The tenth long-lived value evicts p_b2e0, which GCC rematerializes with extra lui/addiu pairs in the pre-loop block: +2 instructions.
- probe: sandbox func_8005D554 --disable all on tmp/grind/func_8005D554/s3/k1_perhalf_acc_hoist.c -> 47/178; objdump pairdiff s3/k0.asm vs s3/k1.asm shows the eviction (k0 keeps p_b2e0 in fp and p_b388 in s7; k1 has three lui a2/addiu a2 rematerializations with s0/s1 holding a2a/a2b).
- result: 47/178. This also overturns the s3 H15 mechanism story: a2a is written twice at exactly the s3 statement positions (reg_n_sets == 2) and still misses by 47, so the reg_n_sets > 1 / birthing_insn_p reading is not what made the FAILed nv/nw forms work. The distinguishing property is the SEPARATE pseudo copied into the accumulator, not the write count. Form banked at rejected/perhalf-accumulator-hoisted-base-scores-47.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s3b chassis == s1/s2 candidate chassis (guard-in-i, p_b2ec, p_b390 = p_b388 + 2, split-init x4, int-order p0), no FAKE constructs present

## [s3] With the ordinary-C hoist of the a2 base in place (g7_reorder_only, 178/31), deleting one callee-saved-resident pointer local restores 176 instructions but not the target order: dropping p_b390 scores 27/176 and dropping p_b2e0 scores 28/176, while the same deletions without the hoist cost two instructions (174/45 and 174/47).
- mechanism: The callee-saved seat budget is exactly nine in both our build and the target; a hoisted base occupies a tenth and each deleted pointer local returns one, so the +2 and the -2 cancel. But the resulting code holds the base in s0 across the rand call (s3/k11.asm body insn 75: addiu s0,s4,-12 in the rand delay slot) whereas the target computes it in caller-saved $a2 AFTER the call (asm/funcs/func_8005D554.s:4DEC0).
- probe: sandbox func_8005D554 --disable all on tmp/grind/func_8005D554/s3/{k9_g7_nopb390.c,k11_g7_nopb2e0.c,k10_k0_nopb390.c,k12_k0_nopb2e0.c}; objdump of the k11 sandbox object.
- result: 27/176, 28/176, 45/174, 47/174. Buying the instructions back does not buy the target order -- every source-level hoist is structurally unlike the target, so the route to the residual is not a hoist. Forms banked at rejected/hoist-plus-freed-pb390-seat-scores-27.c and rejected/hoist-plus-freed-pb2e0-seat-scores-28.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s3b chassis == s1/s2 candidate chassis, no FAKE constructs present

## [s3] On the hoisted g7 chassis, removing the c1 constant-holder local (s.one14 = 1) scores 32/178, removing the c100 holder (literal 0x100 stores) scores 33/178, and removing both scores 34/178, so neither holder is the seat that the hoist needs; the control without the hoist scores 8/176.
- mechanism: Both constants are re-loaded per use rather than freeing a callee-saved seat, because the target itself keeps the 1 in a callee-saved register (sw $s6, 0x24($sp), asm/funcs/func_8005D554.s:4DEC8).
- probe: sandbox func_8005D554 --disable all on tmp/grind/func_8005D554/s3/{k4_g7_noc1.c,k5_g7_noc100.c,k7_g7_noc1_noc100.c,k8_k0_noc1.c}.
- result: 32/178, 33/178, 34/178, 8/176. Form banked at rejected/hoist-without-c1-holder-scores-32.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s3b chassis == s1/s2 candidate chassis, no FAKE constructs present

## [s3] A permuter campaign seeded on the ordinary-C hoist chassis (g7_reorder_only, 178 instructions, permuter base score 940) ran 24,879 iterations with 8 jobs and improved only to 655, with no find in the 160-class basin the z3 campaign reached.
- mechanism: At 178 instructions the chassis is two deletions plus a rotation away from the target, outside the single-site mutation radius the permuter samples; the score landscape between 940 and 655 is pointer-rematerialization noise, not progress toward the emission-order residual.
- probe: tools/permuter_campaign.py launch/wait/harvest --stop on tmp/perm_5d554_g7, label s3b-g7-ordinary-hoist-chassis; telemetry in metrics/events.jsonl; finds under tmp/perm_5d554_g7/output-*.
- result: Best find 655 of base 940. Three chassis are now spent for permutation on this function: the k0/candidate chassis (21,569 iters, 0 finds), z3 guard-duplicated while (30,891 iters, produced only the banned new_var carrier), and g7 (24,879 iters, best 655). Campaign stopped and harvested in-session; no orphan.
- verdict: KILLED
- kill_scope: instance
- measured_on: s3b chassis == s1/s2 candidate chassis, no FAKE constructs present; campaign workspace tmp/perm_5d554_g7
