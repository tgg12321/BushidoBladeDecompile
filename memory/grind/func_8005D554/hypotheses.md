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

## [s4] The complete local-spelling space of the a2-site region -- 12 arithmetic forms x 2 cast spellings of the r4 read x 4 insertion points among the tail stores x 2 positions of the s.zero1C store, 168 spellings applied identically to both loop halves -- is quantized to exactly two scores, 6 and 15, both at 176 instructions, and nothing in it scores below the standing floor of 6.
- mechanism: rank_for_schedule (tools/gcc-2.7.2/sched.c:2408) orders the ready list by INSN_PRIORITY first (sched.c:2418) and only reaches the INSN_LUID tie-break (sched.c:2464) when priority and dependence class are equal. priority() (sched.c:1434, max at sched.c:1499) is the longest dependence path to the block end, so the a2 base insn (addiu a2 -> addu a2,a2,v0 -> sw a2,0x2C -> jal) always outranks the call's argument setup (addiu a0 -> jal) on term 1, for every spelling in which the target's `addiu a2,s4,-K` insn exists at all. Spellings that reassociate the sum so the addiu is not applied to r4 first lose the insn entirely and score 15.
- probe: generator tmp/grind/func_8005D554/s4/gen1.py -> tmp/grind/func_8005D554/enum1 (168 complete bodies), swept with tools/sweep_variants.py --func func_8005D554 --file text1b --json; histogram in tmp/grind/func_8005D554/s4/sweep1.json.
- result: 71 spellings at 6/176 (every form whose first written term is the r4-K base), 98 at 15/176 (every random-first or single-expression reassociation). The cast axis and both placement axes are entirely inert: all 8 combinations of a given arithmetic form score identically. The representative dead form was already banked at rejected/rnd-first-a2-accum-scores-15.c in s1.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ f95f6b8a chassis, candidate.c body (176/176, floor 6 re-measured this session), no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/sched.c:2464

## [s4] The basic block of every movable struct store is byte-fixed: the full power set of relocating {byte28, c24, c20, p1, zero10, one14, ret} into the tail block after the last rand() call, x 2 positions for `i += 1` (256 spellings), yields only 9 forms at the floor, and all nine are the ones that keep zero10, one14 and ret in that order inside the existing tail block.
- mechanism: Each rand() call is a basic-block boundary, so a store that crosses one leaves the block whose schedule the target fixes; and inside the tail block the three stores sw zero,0x20 / sw s6,0x24 / sw s1,0x1C are emitted in source order because they are independent of the last scheduled insn (class 3 in rank_for_schedule, sched.c:2424-2458) and are therefore separated only by the LUID tie-break at sched.c:2464.
- probe: generator tmp/grind/func_8005D554/s4/gen2.py -> tmp/grind/func_8005D554/enum2 (256 complete bodies; the all-zero flag set was diffed against candidate.c and is byte-identical, so the generator is faithful), swept with tools/sweep_variants.py; histogram in tmp/grind/func_8005D554/s4/sweep2.json.
- result: move zero10 alone 6/176 (neutral); move one14 alone 10/176; move ret alone 10/176; move byte28, c24 or c20 alone 10/176 each; move p1 alone 13/176; `i += 1` in either position 6/176; all seven moved 28-30/177. Forms banked at rejected/tail-moved-setup-stores-cross-rand-blocks-scores-30.c and rejected/tail-store-order-transposed-one14-first-scores-10.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ f95f6b8a chassis, candidate.c body, no FAKE constructs present

## [s4] Frontier item P3's premise is false as stated: stacking the neutral spellings does NOT produce a different emission order. The full cross product of 5 neutral a0-site arithmetic forms x 5 neutral a2-site arithmetic forms x 4 declaration scopes for a0_offset/a2_offset (function top in either order, top of the do-body, per-half nested braces) x 2 positions of s.zero1C x 4 neutral tail-store move sets -- 800 complete bodies -- scores 6 at 176 instructions in every single case.
- mechanism: All five surviving arithmetic forms lower to the identical RTL once combine.c folds the constant term into the addiu, and the declaration scope of a non-loop-carried local changes only the pseudo's DECL scope, not its RTL emission order, so INSN_LUID (sched.c:2464) is unchanged. These axes are not weak levers, they are exact no-ops at the byte level.
- probe: generator tmp/grind/func_8005D554/s4/gen3.py -> tmp/grind/func_8005D554/enum3 (800 complete bodies; identity variant a1_a1_top_zl_m0 diffed byte-identical to candidate.c), swept with tools/sweep_variants.py; histogram in tmp/grind/func_8005D554/s4/sweep3.json (801 rows in a single bucket).
- result: 800/800 at exactly 6/176. A neutral stacked form is banked at rejected/neutral-stack-a11-arith-perhalf-scope-storemoves-6.c as documentation that the stack is inert. A fourth permuter seed built from this space would start in the SAME basin as the already-spent k0/candidate chassis, so P3 as written is retired; a new permuter seed must differ in BLOCK STRUCTURE (loop form, guard duplication, object model), not in these axes.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ f95f6b8a chassis, candidate.c body, no FAKE constructs present

## [s4] FRONTIER (replaces P1-P3, written after 1,224 measured spellings)

F1. **The residual is a LUID problem and only a later-pass re-emission can solve it.** The
    scheduler reads INSN_PRIORITY before INSN_LUID (sched.c:2418 vs sched.c:2464), and the a2
    base insn's dependence path to the block end is strictly longer than the argument setup's for
    every spelling that keeps `addiu a2,s4,-K`. The one bump that could equalise them,
    adjust_priority's birthing bump (sched.c:2584), is gated on `reg_n_sets[dest] == 1`
    (birthing_insn_p, sched.c:2526) and both hard argument registers are set at two call sites in
    this function, so it is unavailable. Therefore the target's order requires
    LUID(a2 base) > LUID(a0 arg setup), i.e. the arithmetic insn must be RE-EMITTED below the
    argument setup by a pass that runs after expand. Next probe (forensics modality):
    `pwsh tools/grinder/dump.ps1 func_8005D554` with
    rejected/judge-failed-fresh-multiwrite-nv-nw-carrier-scores-0.c applied and again with
    candidate.c applied; diff the `.cse`, `.combine` and `.flow` dumps around the a2 base insn and
    name the pass that moved it. Only once the pass is named can a legal C trigger be searched
    for; blind spelling search of the region is now exhausted.

F2. **Attack the a0/a1 side: make the argument-setup insns birthing.** birthing_insn_p
    (sched.c:2526) returns nonzero iff the destination register is live and
    `reg_n_sets[REGNO] == 1`, in which case adjust_priority (sched.c:2584) raises the insn to
    max_priority -- which would put the arg setup AHEAD of the a2 base without touching the a2
    base at all. The hard argument registers are set at two `func_80073728` call sites here, so
    reg_n_sets is 2 and the bump never fires. NOT YET MEASURED: the block's shape when the source
    has only ONE `func_80073728` call site (both halves funnelled through one call -- e.g. a
    two-iteration inner loop over the per-half constants, or an if/else that selects the constants
    and falls into a shared call). That is ordinary C, is structurally plausible for the original
    given how symmetric the two halves are, and is the only route found so far that raises the arg
    setup's priority instead of lowering the a2 base's. Next probe: write the single-call-site
    chassis, measure it, and if the arg setup does move first, check whether that form can still
    reach 176 instructions.

F3. **Block structure, not local spelling.** Sweeps 1-3 prove the residual is invariant under
    every local-spelling axis inside the block. The untried structural axes are the loop form
    itself (the s2 `z3` while + duplicated guard, measured neutral at 6 but NOT inside the enum3
    space), the `if (i < ...)` guard shape, and the object model of `s` / `D_8009B2E0` (a real
    struct or array declaration instead of the `p_b2e0 = (u8 *)&D_8009B2E0` pun that the
    DATA MODEL block flags). Next probe: rebuild the s2 `z3` guard-duplicated-while body on the
    current chassis and re-run sweep 3's arithmetic x scope cross product on top of it (200
    spellings) to see whether the score is still invariant once the block structure changes.

## [s4] The complete local-spelling space of the a2-site region -- 12 arithmetic forms x 2 cast spellings of the r4 read x 4 insertion points among the tail stores x 2 positions of the s.zero1C store, 168 complete bodies applied identically to both loop halves -- is quantized to exactly two scores, 6 and 15, both at 176 instructions, and nothing in it scores below the standing floor of 6.
- mechanism: rank_for_schedule (tools/gcc-2.7.2/sched.c:2408) orders the ready list by INSN_PRIORITY first (sched.c:2418), then by dependence class against last_scheduled_insn, and only then by INSN_LUID ascending (sched.c:2464). priority() (sched.c:1434, max taken at sched.c:1499) is the longest dependence path from the insn to the end of the block, so the a2 base insn (addiu a2,s4,-K -> addu a2,a2,v0 -> sw a2,0x2C(sp) -> jal) strictly outranks the call's argument setup (addiu a0,sp,0x10 -> jal) on term 1, for any spelling in which the target's addiu insn exists at all; the LUID tie-break is therefore never consulted between them. Spellings that reassociate the sum so the constant is not applied to r4 first lose that insn entirely and score 15.
- probe: Generator tmp/grind/func_8005D554/s4/gen1.py wrote 168 complete function bodies to tmp/grind/func_8005D554/enum1; swept in one call with tools/sweep_variants.py --func func_8005D554 --file text1b --variants tmp/grind/func_8005D554/enum1 --json; histogram banked at tmp/grind/func_8005D554/s4/sweep1.json.
- result: 71 spellings at 6/176 (every form whose first written term is the r4-K base: base-first split-init, three-way split, `= -K; += r4; += rnd`, `= r4-K; = rnd + a2`), 98 at 15/176 (random-first and single-expression reassociations). The cast axis and both placement axes are entirely inert -- all 8 combinations of a given arithmetic form score identically. The representative dead form was already banked in s1 at rejected/rnd-first-a2-accum-scores-15.c.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ f95f6b8a chassis with memory/grind/func_8005D554/candidate.c applied to src/text1b.c (floor re-measured 6/176 this session); no FAKE constructs present in any variant
- predicate_cite: tools/gcc-2.7.2/sched.c:2464

## [s4] Of the 256 spellings that relocate subsets of the seven movable struct stores (byte28, c24, c20, p1, zero10, one14, ret) into the tail block after the last rand() call, crossed with 2 positions for `i += 1`, only 9 sit at the floor of 6/176, and those 9 are precisely the ones that keep zero10, one14 and ret in that relative order inside the existing tail block.
- mechanism: Each rand() call is a basic-block boundary, so a store that crosses one leaves the block whose schedule the target fixes. Inside the tail block the three stores sw zero,0x20 / sw s6,0x24 / sw s1,0x1C are independent of the last scheduled insn (class 3 in rank_for_schedule, sched.c:2424 onward) and so are separated only by the INSN_LUID tie-break at sched.c:2464, which is source order.
- probe: Generator tmp/grind/func_8005D554/s4/gen2.py wrote 256 complete bodies to tmp/grind/func_8005D554/enum2 (the all-flags-off variant was diffed byte-identical to candidate.c, proving the generator faithful); swept with tools/sweep_variants.py; histogram at tmp/grind/func_8005D554/s4/sweep2.json.
- result: Per-axis costs: move zero10 alone 6/176 (neutral); one14 alone 10/176; ret alone 10/176; byte28, c24 or c20 alone 10/176 each; p1 alone 13/176; `i += 1` in either position 6/176; all seven moved 28-30/177. Two facts fall out: the tail store order is byte-fixed and any transposition costs +4, and each setup store's basic block is byte-fixed. Forms banked at rejected/tail-moved-setup-stores-cross-rand-blocks-scores-30.c and rejected/tail-store-order-transposed-one14-first-scores-10.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ f95f6b8a chassis with candidate.c applied to src/text1b.c; no FAKE constructs present in any variant

## [s4] Frontier item P3's premise is false as stated: stacking the neutral spellings does not perturb the emission order. The cross product of 5 neutral a0-site arithmetic forms x 5 neutral a2-site arithmetic forms x 4 declaration scopes for a0_offset/a2_offset (function top in either order, top of the do-body, per-half nested braces) x 2 positions of s.zero1C x 4 neutral tail-store move sets -- 800 complete bodies -- scores 6 at 176 instructions in 800 out of 800 cases.
- mechanism: The five surviving arithmetic forms lower to identical RTL once combine.c folds the constant term into the addiu, and the declaration scope of a non-loop-carried local changes only the pseudo's DECL scope, not its RTL emission position, so INSN_LUID (sched.c:2464) is unchanged. These axes are exact byte-level no-ops rather than weak levers.
- probe: Generator tmp/grind/func_8005D554/s4/gen3.py wrote 800 complete bodies to tmp/grind/func_8005D554/enum3 (identity variant a1_a1_top_zl_m0 diffed byte-identical to candidate.c); swept with tools/sweep_variants.py; histogram at tmp/grind/func_8005D554/s4/sweep3.json shows 801 rows collapsing into the single bucket (score 6, 176 insns).
- result: 800/800 at exactly 6/176. A representative stacked form is banked at rejected/neutral-stack-a11-arith-perhalf-scope-storemoves-6.c as documentation that the stack is inert. A fourth permuter seed built from this space would start in the same basin as the already-spent k0/candidate chassis, so P3 is retired; a new seed must differ in block structure (loop form, guard duplication, object model), not in these axes.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ f95f6b8a chassis with candidate.c applied to src/text1b.c; no FAKE constructs present in any variant

## [s4] The residual is confirmed to be a three-slot rotation of one 4-insn window per loop half and nothing else: an instruction-by-instruction alignment of the candidate's sandbox object against asm/funcs/func_8005D554.s matches all 176 slots except target T88-T91 / ours O88-O91 and target T134-T137 / ours O129-O132.
- mechanism: The window is the ready-list pick order of {a2 base, a0 argument, a1 argument} inside the block that follows the third rand() call. The target picks addiu a0,sp,0x10 then addu a1,zero,zero then lw v1 then addiu a2,s4,-K; ours picks addiu a2 then addiu a0 then lw v1 then move a1. The lw v1 slot and all three following struct stores (sw zero,0x20 / sw s6,0x24 / sw s1,0x1C) are already byte-aligned in both halves.
- probe: mipsel-linux-gnu-objdump -d on tmp/sandbox/func_8005D554/text1b.o after a clean `sandbox func_8005D554 --disable all` on the candidate body (score 6, build_insns 176), aligned against the target listing by tmp/grind/func_8005D554/s4/pairdiff.py; disassembly banked at tmp/grind/func_8005D554/s4/ours.dis.
- result: Confirmed: 176 slots, 6 differing, two identical 3-diff windows. Note for future sessions -- tmp/sandbox/<func>/ holds the LAST object built, so a disassembly taken after a sweep_variants run shows the last VARIANT, not the candidate; re-run sandbox on the candidate before dumping.
- verdict: CONFIRMED

## s5 (synthesis) — measured. FLOOR 6/176 (re-measured this session on HEAD main @ 4f43e5cf).

This session ran the PASS ATTRIBUTION that s4's frontier F1 asked for, and the answer
**overturns the mechanism story of s3 (H15/H19) and of s4 (F1/F2)**. All four dump sets are
banked under `tmp/grind/func_8005D554/s5/dumps_{cand,nvnw,v2,freshsingle}/` (per-function
extracts `f.rtl`, `f.combine`, `f.lreg`, `f.greg`, produced by
`pwsh tools/grinder/dump.ps1`-equivalent `tmp/grind/func_8005D554/run_dump.sh` +
`s5/ext.sh`).

### H23 — PASS ATTRIBUTION: the Judge-FAILed `nv`/`nw` body wins through a two-phase
`loop.c`-then-`combine.c` interaction, not through `reg_n_sets`/`birthing_insn_p` and not
through a two-pseudo RTL shape. **CONFIRMED.**
- statement: in the FAILed body the a2 base carrier `nv` is written twice IN SOURCE, so at
  `loop.c` time it is not a movable and its loop-invariant value `(s32)r4 - K` is NOT hoisted
  out of the loop. `combine.c` then erases the second write (`nv = ret; s.ret = nv;` collapses
  to `s.ret = ret`), leaving a pseudo that is SINGLE-SET, single-use and carries a `REG_DEAD`
  note at the consuming `addu`. `sched1` places that insn immediately after the
  `D_800A3418` load, and `local-alloc` therefore seats it in caller-saved `$a2` — the target's
  exact shape.
- mechanism + evidence, insn by insn (all insn numbers from the banked dumps):
  * `dumps_nvnw/f.rtl:484` — `(insn 198 195 201 (set (reg/v:SI 83) (plus (reg/v:SI 78) (const_int -12))))`,
    i.e. the base insn is born EARLY, before the third `rand` call (`call_insn 204`).
  * `dumps_nvnw/f.combine:493` — insn 198 survives combine unchanged AND
    `grep -c 'set (reg/v:SI 83)' = 1`: the second source write of `nv` is gone, and
    `f.combine:572-574` shows the consuming `addu` with `REG_DEAD (reg/v:SI 83)`.
  * `dumps_nvnw/f.lreg` — insn 198 is now chained `208 -> 198 -> 209`, i.e. sched1 SANK it
    across the call to sit right after the `D_800A3418` load.
  * `dumps_cand/f.rtl:515` — on `candidate.c` the same value is born LATE
    (`(insn 211 208 214 ...)`, already after the `D_800A3418` load) into the accumulator
    pseudo 82, and sched1 HOISTS it above the argument setup: that is the whole 6-point residual.
  * `dumps_freshsingle/f.combine:339` — with a FRESH SINGLE-SET per-half carrier the base insn
    is renumbered to `(insn 393 133 394 ...)` and sits in the PRE-LOOP block: `loop.c` hoisted
    the loop-invariant out of the loop entirely; `dumps_freshsingle/f.greg:391` then shows it
    allocated to `$a2` in the pre-loop but the loop body pays two extra insns.
- probe: `run_dump.sh` on each of the four bodies; extracts via `s5/ext.sh`; chain reader
  `s5/chain.py` printed the post-sched1 window before each `func_80073728` call.
- result: the correct causal chain is LICM-avoidance (source multi-set) -> combine erasing the
  extra set -> short single-use live range -> sched1 sink -> caller-saved `$a2`. Future sessions
  must NOT re-derive the s3 `reg_n_sets > 1 defeats birthing_insn_p` reading (already overturned
  by s3's own H19) nor the s4 reading that the residual needs a LUID above the argument loads:
  the winning insn's LUID is BELOW the argument loads and it still schedules last.
- verdict: CONFIRMED
- measured_on: HEAD main @ 4f43e5cf, floor re-measured 6/176 on candidate.c this session;
  no FAKE constructs in candidate.c; the nv/nw body is the banked Judge-FAILed form.

### H24 — The `+2` instructions that every "born early" spelling pays is `loop.c` LICM of a
single-set loop-invariant, and the two-pseudo (dest != accumulator) RTL shape is a byte-level
no-op. **KILLED (instance).**
- statement: sixteen new spellings measured this session — 7 arithmetic/position shapes at the
  a2 site (base after the `s.zero18` store / at the `nv` slot / at the natural late slot, each
  crossed with direct-store `s.zero1C = base + rnd` vs read-modify-write accumulate, plus a
  variant summing into `a0_offset`), crossed with three seat states (both pointer locals kept,
  `p_b390` freed, `p_b2e0` freed, both freed) — reproduce exactly three score classes and
  nothing below the floor: every early-birth form is 178 instructions and scores 30-35; every
  early-birth form with ONE pointer local freed is 176 instructions and scores 27-33; freeing
  both is 174/53; the late-birth direct-store form is 176/10.
- mechanism: the direct-store shape and the accumulate shape lower to the same RTL because
  `combine.c` cannot merge the base insn into the add in either case (the base pseudo is
  multi-set at combine time), so the only quantity that changes is WHERE the base pseudo lives:
  a per-function local borrowed for the base keeps a live range that spans the half boundary,
  gets a callee-saved seat from `global-alloc`, and evicts a pointer local (+2); a fresh
  single-set per-half local is a `loop.c` movable and gets hoisted clean out of the loop (+2 in
  the body). Freeing one pointer local pays the +2 back but leaves the base in a callee-saved
  register across the `rand` call (`dumps_v2/f.greg:455` — `(set (reg/v:SI 16 s0) (plus (reg:SI 20 s4) (const_int -12)))`),
  which is structurally unlike the target's caller-saved `$a2`.
- probe: `tmp/grind/func_8005D554/s5/gen.py|gen2.py|gen3.py|gen4.py|gen5.py` ->
  `s5/enum{,2,3,4,5}`; swept with `tools/sweep_variants.py --func func_8005D554 --file text1b
  --json`; histograms `s5/sweep{,2,3,4,5}.json`.
- result: v1 31/178, v2 31/178, v3 10/176, v4 31/178, v5 31/178, v6 30/178, v7 30/178;
  v{1,2,4}_nob390 27/176, v{1,2,4}_nob2e0 28/176, v{1,2,4}_both 53/174; fresh single-set
  per-half base 54/178 (plain), 32/176 (nob390), 35/176 (nob2e0); `a0_offset` borrow 35/178,
  33/176, 32/176; existing-local borrow with the `X = ret; s.ret = X;` restage that erases the
  second set in the FAILed body 30/178, 30/176 (`a2_offset` carrier), 35/178, 33/176
  (`a0_offset` carrier). Forms banked at
  `rejected/early-birth-existing-local-two-pseudo-scores-31.c`,
  `rejected/fresh-single-set-perhalf-base-licm-hoisted-scores-54.c`,
  `rejected/existing-local-borrow-ret-restage-scores-30.c`.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4f43e5cf with candidate.c as the chassis (floor re-measured 6/176);
  no FAKE constructs present in any variant

### H25 — s4 frontier F2: a single `func_80073728` call site in the source raises the argument
setup's priority via `birthing_insn_p`. **KILLED (class).**
- statement: the shipped target emits TWO distinct `jal func_80073728` instructions
  (`asm/funcs/func_8005D554.s:108` and `:155`), so a source with a single call site — an
  if/else selecting the per-half constants and falling into one call, or a two-iteration inner
  loop — emits one `jal` and cannot produce the target's 176-instruction body under GCC 2.7.2,
  which does not unroll loops at `-O2` without `-funroll-loops`.
- mechanism: one `CALL_EXPR` in the source expands to one `call_insn`; no pass in the 2.7.2
  pipeline duplicates a call insn (`reorg.c` delay-slot filling excepted, which cannot create a
  second `jal` to the same target in a different block).
- probe: `grep -n 'jal' asm/funcs/func_8005D554.s` -> 9 `jal`, of which exactly two are
  `func_80073728` (lines 108, 155) and seven are `rand`.
- verdict: KILLED
- kill_scope: class
- predicate_cite: asm/funcs/func_8005D554.s:155
- measured_on: the shipped target listing; independent of our chassis and of any FAKE construct

### H26 — KILL RE-AUDIT (mandated): the two closest-to-target instance kills reproduce on the
current chassis. **CONFIRMED.**
- statement: `rejected/shared-a2-base-direct-store-scores-10.c` (the closest non-floor form,
  score 10) re-measures at 10/176 as this session's `s5/enum/v3.c`, and
  `rejected/hoist-plus-freed-pb390-seat-scores-27.c` (score 27) re-measures at 27/176 as
  `s5/enum2/v4_nob390.c`. `tools/fake_ablate.py --func func_8005D554 --file text1b --candidate
  rejected/judge-failed-fresh-multiwrite-nv-nw-carrier-scores-0.c` reports "no FAKE-annotated
  constructs found", confirming there is no FAKE carrier masking a lever anywhere in the banked
  forms (candidate.c is likewise FAKE-free).
- verdict: CONFIRMED
- measured_on: HEAD main @ 4f43e5cf, candidate.c chassis, floor 6/176

## Frontier after s5 (replaces s4's F1-F3)

Q1. **Find the ordinary-C construct that keeps the a2 base out of `loop.c`'s movable set AND
    out of a callee-saved seat.** H23 names the exact requirement: the base pseudo must be
    multi-set when `loop.c` runs (so the invariant is not hoisted) and must end up with a live
    range confined to the single half (so `local-alloc` seats it in caller-saved `$a2`). Every
    EXISTING local measured so far (`a0_offset`, `a2_offset`, `v0`, `v3`, `ret`) satisfies the
    first condition and fails the second, because each is declared at function scope and is
    read again in the other loop half. The untried lever is therefore NOT another borrow but a
    change that shortens the borrowed local's range — e.g. giving the two halves genuinely
    separate a0/a2 offset variables (four function-scope locals instead of two) so each is
    written and read within one half only, then borrowing the half-local for the base. Next
    probe: build the 4-local chassis (`a0a/a2a` for half 1, `a0b/a2b` for half 2), confirm it
    still measures 6/176, then re-run the s5 early-birth cross product on top of it.

Q2. **Attack `loop.c` directly: make the invariant not invariant.** `(s32)r4 - K` is
    loop-invariant only because `r4` is. If the source computes the base from a value that
    `loop.c` cannot prove invariant — e.g. reading `r4` back out of a location the loop writes,
    or deriving it from `i` in a way that cancels — the movable disappears and a single-set
    fresh-free spelling becomes affordable. Any such spelling must not add instructions, so the
    cancellation has to be one `combine.c` folds away. Next probe: dump `.loop` for
    `s5/enum3/acc_plain.c` and read the `movable` list to see exactly which invariants `loop.c`
    records for this loop, then look for a spelling that removes `(s32)r4 - K` from it without
    changing the emitted arithmetic.

Q3. **A fourth permuter seed on a BLOCK-STRUCTURE-different 176/6 chassis.** s4 retired the
    stacked-neutral seed (P3) because those axes are byte-level no-ops; s5 adds sixteen more
    no-op/penalty points. The remaining structurally different 176/6 chassis is the s2 `z3`
    guard-duplicated `while` — already permuted in s3 and it produced only the banned
    fresh-multi-write carrier. A genuinely new seed therefore needs a new BLOCK structure: the
    Q1 4-local chassis, or an object-model change at the `p_b2e0 = (u8 *)&D_8009B2E0`
    declaration pun (the DATA MODEL block flags it; the typed-array spelling was killed at 21,
    but a `struct`-typed base pointer with a real member for the +0xC sub-object was not).

## s5b (synthesis, re-run after the s5 outcome was discarded on a wording defect) — measured. FLOOR 6/176.

The s5 session's measurements (H23-H26) survive on disk and were re-read, not re-derived; that
discard was purely an outcome-JSON kill-scope wording defect, not a measurement defect. This
section adds 55 NEW measured spellings on top of them (`tmp/grind/func_8005D554/s5b/`,
generators `gen{,2,3,4,5}.py`, histograms `sweep{1,2,3,4,5}.json`), and retires all three of
the s5 frontier items Q1/Q2/Q3 with measurements.

### H27 — s5 frontier Q1: giving the two loop halves genuinely separate offset locals shortens
the borrowed carrier's live range enough to win a caller-saved seat for the a2 base. **KILLED (instance).**
- statement: on the current candidate chassis, the 4-local chassis (`a0a`/`a2a` for half 1,
  `a0b`/`a2b` for half 2) measures 6/176 — byte-identical to the 2-local candidate — and the
  three early-birth placements measured on top of it (base at the `nv` slot before the a2-site
  `rand`, base right after the `a0 +=` statement, each with and without the `p_b390` pointer
  local) measure 47/178, worse than the same placements on the 2-local chassis (31/178).
- mechanism: per-half offset locals are still written twice inside the loop (`= r5 - K` then
  `+= rnd`), so they are not `loop.c` movables either way; separating them does not shorten the
  live range that `global-alloc` sees, because both halves sit in the same basic block and the
  half-1 locals stay live across the half-2 code for the loop back-edge. Early-birthing the base
  into a per-half local adds a fourth simultaneously-live offset pseudo, which costs the two
  instructions the 2-local chassis already paid plus a second callee-saved seat.
- probe: `tmp/grind/func_8005D554/s5b/gen.py` -> `s5b/enum` (12 bodies), swept with
  `tools/sweep_variants.py --func func_8005D554 --file text1b --json` -> `s5b/sweep1.json`.
- result: q2_late_none 6/176 (control = candidate.c), q4_late_none 6/176, q2_late_b390 18/175,
  q4_late_b390 18/175, q2_posta0/prerand_b390 29/177, q2_posta0/prerand_none 31/178,
  q4_posta0/prerand_{none,b390} 47/178 (all four). Form banked at
  `rejected/four-perhalf-offset-locals-early-birth-scores-47.c`.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4f43e5cf with candidate.c as the chassis (floor re-measured 6/176);
  no FAKE constructs present in any variant

### H28 — The identity and freshness of the a2 base carrier is a byte-level no-op in the
accumulate shape, and decides between 176 and 178 instructions only in the direct-store shape.
**CONFIRMED (as a mechanism); the 18 measured forms KILLED (instance).**
- statement: eighteen bodies crossing three carriers (the shared `a2_offset`, one fresh local
  shared by both halves, two fresh per-half locals) x two consumption shapes (accumulate
  `X += rnd; s.zero1C = X;` vs direct store `s.zero1C = X + rnd;`) x three positions of the
  `zero10/one14/ret` store group relative to the base statement, all born LATE (after the
  a2-site `D_800A3418 ^= rand()`), collapse to exactly three scores: the nine accumulate bodies
  are 6/176, the six direct-store bodies with a carrier written twice in the loop are 10/176,
  and the three direct-store bodies with fresh PER-HALF carriers are 54/178.
- mechanism: a carrier written twice anywhere in the loop is not a `loop.c` movable
  (`loop.c:791` records a movable only for a register whose set count qualifies), so the
  invariant `(s32)r4 - K` stays in the loop; a fresh per-half carrier is set exactly once in the
  loop, becomes a movable, and passes the desirability test at `loop.c:1631`
  (`threshold * savings * lifetime >= insn_count`), so the invariant is hoisted to the pre-loop
  block and the loop body pays two extra instructions. In the accumulate shape `combine.c`
  merges the base set into the accumulator so the carrier's identity never reaches the
  scheduler at all — which is why naming it changes nothing.
- probe: `s5b/gen2.py` -> `s5b/enum2` (18 bodies) -> `s5b/sweep2.json`.
- verdict: KILLED (no form below the floor)
- kill_scope: instance
- measured_on: HEAD main @ 4f43e5cf, candidate.c chassis, floor 6/176; no FAKE constructs
  present in any variant. Form banked at
  `rejected/fresh-perhalf-carrier-direct-store-hoisted-scores-54.c`.

### H29 — The permuter's sandbox-3 `new_var` shape (base staged through a carrier in one half,
the same carrier re-written to 0 to feed `s.zero10` in the other half) works with a PARAMETER
as the carrier, which is inside the sanctioned staged-value/dead-store quadrant rather than the
Judge-banned invented-carrier quadrant. **KILLED (instance).**
- statement: nine bodies that reproduce `tmp/perm_5d554_z3/output-160-1/source.c`'s two-write
  staging shape with the dead parameters `arg0`/`arg1` and with the dead locals `v0`/`v3` as
  the carriers — both halves staged, half-1-only staged, and the half-1 stage without the
  second write — measure 25/178 (`arg0` half-1 only), 37/177, 42/178, 43/177 (three bodies),
  43/179, 50/179 and 61/178 (two bodies). None of the nine reaches 176 instructions.
- mechanism: `arg0` and `arg1` are dead inside the loop but their pseudos are born in the
  function prologue block, so `global-alloc` gives each a callee-saved seat that lives across
  the whole loop; re-writing them inside the loop therefore adds a copy rather than reusing a
  free seat, exactly as the `v0`/`v3` borrows measured in s3 (H16) did.
- probe: `s5b/gen3.py` -> `s5b/enum3` (9 bodies) -> `s5b/sweep3.json`.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4f43e5cf, candidate.c chassis, floor 6/176; no FAKE constructs
  present in any variant. Forms banked at
  `rejected/param-borrow-arg0-arg1-zero10-restage-scores-43.c` and
  `rejected/param-borrow-arg0-h1only-scores-25.c`.

### H30 — s5 frontier Q3: a different LOOP chassis changes the emission order in the residual
window. **KILLED (instance).**
- statement: twenty bodies crossing four loop chassis (the candidate's `do { } while`, the s2
  `z3` guard-duplicated `while`, a `for (;;)` with a trailing `if (!cond) break;`, and an
  explicit label + `goto` loop) x three a2-site arithmetic spellings (split `= r4 - K` then
  `+= rnd`, `+ -K`, direct store) x two positions of the `s.zero1C` store measure 6/176 for the
  do-while, while and for-break bodies in the split and `+ -K` columns, 10/176 for the three
  direct-store bodies, and 37/175 or 41/175 for the five label+goto bodies.
- mechanism: `jump.c`'s `duplicate_loop_exit_test` normalises the guard-duplicated `while` and
  the `for (;;) { ... if (!c) break; }` into the same `do { } while` RTL as the candidate, so
  the three forms share one insn stream; the label+goto form loses the phantom compare pseudo
  that gives the candidate its 176th instruction and drops to 175.
- probe: `s5b/gen4.py` -> `s5b/enum4` (20 bodies) -> `s5b/sweep4.json`.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4f43e5cf, candidate.c chassis, floor 6/176; no FAKE constructs
  present in any variant. Form banked at `rejected/goto-loop-chassis-scores-37.c`.

### H31 — The untried side of the sched1 tie is the ARGUMENT SETUP: routing the call's literal
second argument through a zero constant-holder local changes the `a1 = 0` insn's shape and
therefore its position in the tie. **KILLED (instance).**
- statement: five bodies — a `c0 = 0;` constant holder initialised inside the guard block and
  used as the second call argument (19/179), used for the `s.zero10` stores (21/179), used for
  both (30/179), initialised at function top and used as the second argument (16/178), and a
  re-parenthesised `(s32)(&s)` first argument (6/176) — measure no body below the floor, and
  the four constant-holder bodies each add two or three instructions.
- mechanism: a pseudo holding 0 that is live across the loop is not propagated back to `$zero`
  by `local-alloc`, so `load_register_parameters` emits a register-to-register copy plus the
  holder's own set, and `global-alloc` spends a seat keeping it live; the target's
  `addu $a1, $zero, $zero` is what GCC emits for a literal `0` argument, which is what the
  candidate already emits.
- probe: `s5b/gen5.py` -> `s5b/enum5` (5 bodies) -> `s5b/sweep5.json`.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4f43e5cf, candidate.c chassis, floor 6/176; no FAKE constructs
  present in any variant. Form banked at
  `rejected/zero-constant-holder-second-call-arg-scores-19.c`.

## Frontier after s5b (replaces Q1/Q2/Q3)

R1. **s5 frontier Q2 is now the only untried mechanism-level lever: remove `(s32)r4 - K` from
    `loop.c`'s movable set by making it not loop-invariant, so a single-set fresh carrier stays
    in the loop.** H28 pins the exact gate: `loop.c:1631` moves a movable when
    `threshold * savings * lifetime >= insn_count`, with
    `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (`loop.c:532`) and
    `savings = n_times_used[regno]`, `lifetime = luid(last use) - luid(first def)`
    (`loop.c:790-793`). Everything measured so far attacks the set count; nothing has attacked
    `insn_count`, `lifetime`, or the invariance test itself. Next probe: run `run_dump.sh` on
    `s5b/enum2/fresh_direct_before.c` and read the `.loop` dump's movable list for this loop to
    get the recorded `lifetime`, `savings` and `insn_count` numbers, then look for a source
    spelling of the base whose value `invariant_p` does not accept — e.g. deriving the `- K`
    displacement from a loop-written location that `cse.c` folds back to a constant, so the
    emitted arithmetic is unchanged but the movable disappears.

R2. **The object model at the `p_b2e0 = (u8 *)&D_8009B2E0` declaration pun is the last untried
    STRUCTURAL axis** (the loop chassis is now measured out by H30, and the typed-array spelling
    was killed at 21 by H3). The remaining spelling is a base pointer to a real struct type with
    a member for the +0xC sub-object, which changes which insns exist in the setup block and
    therefore the INSN_LUID sequence the sched1 tie-break reads. NOTE: a TU-local struct
    declaration is excluded by the aggregate-merge family's header-canonical prong, so this axis
    needs a header edit and therefore an integration handoff, or a ruling, before it can be
    submitted — but it can be MEASURED first with a TU-local declaration to see whether the
    residual moves at all. Next probe: declare the type locally, measure, and only pursue the
    header route if the score drops below 6.

R3. **Re-open the `s.zero10` dependence route with a construct that is not an invented carrier.**
    The permuter's only sub-6 find (`tmp/perm_5d554_z3/output-160-1`, sandbox 3) works by giving
    the a2-base set and the `s.zero10` store the same destination pseudo, which creates the
    output dependence s1's exact scheduler model (`s1/mutate_out.txt`, `launch_dep228`) predicted
    would reproduce the target order. H29 shows parameters and dead locals cannot carry it
    without adding instructions. The untried carrier class is a value the program ALREADY stages
    for both purposes — i.e. a spelling in which the number stored into `s.zero10` is genuinely
    derived from the a2 site. Next probe: read the callee `func_80073728` for whether the +0x10
    field of the argument struct is read at all; if it is a genuine input field the original
    source may have zeroed it from a variable, and that variable is the carrier.

## [s5] On the current candidate chassis the 4-local chassis (separate a0a/a2a and a0b/a2b offset locals per loop half) measures 6/176, and the three early-birth placements measured on top of it (base at the pre-rand slot, base right after the a0 += statement, each with and without the p_b390 pointer local) measure 47/178, worse than the same placements on the 2-local chassis at 31/178.
- mechanism: Per-half offset locals are still written twice inside the loop (= r5 - K then += rnd) so they are not loop.c movables either way; both halves sit in one basic block, so separating them does not shorten the live range global-alloc sees. Early-birthing the base into a per-half local adds a fourth simultaneously-live offset pseudo, costing the two instructions the 2-local chassis already paid plus a second callee-saved seat.
- probe: tmp/grind/func_8005D554/s5b/gen.py -> s5b/enum (12 bodies), swept with tools/sweep_variants.py --func func_8005D554 --file text1b --json -> s5b/sweep1.json
- result: q2_late_none 6/176 (control), q4_late_none 6/176, q2_late_b390 18/175, q4_late_b390 18/175, q2_posta0/prerand_b390 29/177, q2_posta0/prerand_none 31/178, the four q4_posta0/prerand cells 47/178. s5 frontier Q1 retired. Form banked at memory/grind/func_8005D554/rejected/four-perhalf-offset-locals-early-birth-scores-47.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4f43e5cf, candidate.c chassis (floor re-measured 6/176 this session); no FAKE constructs present in any variant

## [s5] Eighteen bodies crossing three a2-base carriers (the shared a2_offset, one fresh local shared by both halves, two fresh per-half locals) x accumulate vs direct-store consumption x three positions of the zero10/one14/ret store group, all born after the a2-site rand, collapse to three scores: the nine accumulate bodies 6/176, the six direct-store bodies whose carrier is written twice in the loop 10/176, and the three direct-store bodies with fresh per-half carriers 54/178.
- mechanism: A carrier written twice anywhere in the loop is not a loop.c movable (loop.c:791 records a movable only for a register whose set count qualifies), so the invariant (s32)r4 - K stays in the loop; a fresh per-half carrier is set once, becomes a movable, and passes the desirability test at loop.c:1631 (threshold * savings * lifetime >= insn_count), so the invariant is hoisted to the pre-loop block and the loop body pays two extra instructions. In the accumulate shape combine.c merges the base set into the accumulator, so the carrier's identity never reaches the scheduler.
- probe: tmp/grind/func_8005D554/s5b/gen2.py -> s5b/enum2 (18 bodies) -> s5b/sweep2.json
- result: Nothing in the measured set sits below the floor. Naming or freshening the accumulate-shape carrier is a byte-level no-op; the direct-store shape is 4 points worse at best. Form banked at memory/grind/func_8005D554/rejected/fresh-perhalf-carrier-direct-store-hoisted-scores-54.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4f43e5cf, candidate.c chassis, floor 6/176; no FAKE constructs present in any variant

## [s5] Nine bodies reproducing the permuter's sandbox-3 shape from tmp/perm_5d554_z3/output-160-1/source.c (a carrier holding one half's a2 base and re-written to 0 in the other half to feed s.zero10) with the dead parameters arg0/arg1 and the dead locals v0/v3 as the carrier measure 25/178, 37/177, 42/178, 43/177 (three bodies), 43/179, 50/179 and 61/178 (two bodies); none of the nine reaches 176 instructions.
- mechanism: arg0 and arg1 are dead inside the loop but their pseudos are born in the prologue block, so global-alloc gives each a callee-saved seat live across the whole loop; re-writing them inside the loop adds a copy rather than reusing a free seat, matching the v0/v3 borrow measurements banked in s3 (H16).
- probe: tmp/grind/func_8005D554/s5b/gen3.py -> s5b/enum3 (9 bodies) -> s5b/sweep3.json
- result: The sanctioned staged-value / dead-param quadrant does not reach 176 instructions on this chassis, so the permuter's only sub-6 find has no measured re-spelling inside it. Forms banked at rejected/param-borrow-arg0-arg1-zero10-restage-scores-43.c and rejected/param-borrow-arg0-h1only-scores-25.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4f43e5cf, candidate.c chassis, floor 6/176; no FAKE constructs present in any variant

## [s5] Twenty bodies crossing four loop chassis (the candidate's do-while, the s2 z3 guard-duplicated while, a for(;;) with a trailing if(!cond) break, and an explicit label + goto loop) x three a2-site arithmetic spellings x two positions of the s.zero1C store measure 6/176 for the do-while, while and for-break bodies in the split and '+ -K' columns, 10/176 for the three direct-store bodies, and 37/175 or 41/175 for the five label+goto bodies.
- mechanism: jump.c's duplicate_loop_exit_test normalises the guard-duplicated while and the for-break form into the same do-while RTL as the candidate, so the three share one insn stream; the label+goto form loses the phantom compare pseudo that supplies the candidate's 176th instruction and drops to 175.
- probe: tmp/grind/func_8005D554/s5b/gen4.py -> s5b/enum4 (20 bodies) -> s5b/sweep4.json
- result: s5 frontier Q3's loop-structure half is retired: the three normalising chassis are indistinguishable at the floor. Form banked at memory/grind/func_8005D554/rejected/goto-loop-chassis-scores-37.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4f43e5cf, candidate.c chassis, floor 6/176; no FAKE constructs present in any variant

## [s5] Five bodies attacking the argument-setup side of the sched1 tie -- a c0 = 0 constant holder initialised in the guard block and used as the second call argument (19/179), used for the s.zero10 stores (21/179), used for both (30/179), initialised at function top and used as the second argument (16/178), and a re-parenthesised (s32)(&s) first argument (6/176) -- measure no body below the floor, and the four constant-holder bodies each add two or three instructions.
- mechanism: A pseudo holding 0 that is live across the loop is not folded back to $zero by local-alloc, so load_register_parameters emits a register-to-register copy plus the holder's own set and global-alloc spends a seat keeping it live. The target's addu $a1, $zero, $zero is what GCC emits for a literal 0 argument, which the candidate already emits.
- probe: tmp/grind/func_8005D554/s5b/gen5.py -> s5b/enum5 (5 bodies) -> s5b/sweep5.json
- result: The a1 = 0 argument insn is already target-shaped; re-spelling the literal only adds instructions. Form banked at memory/grind/func_8005D554/rejected/zero-constant-holder-second-call-arg-scores-19.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4f43e5cf, candidate.c chassis, floor 6/176; no FAKE constructs present in any variant

## [s5] The callee func_80073728 genuinely reads the +0x10 and +0x14 fields of its argument struct, so the candidate's s.zero10 = 0 and s.one14 = 1 are real input stores of literal constants rather than staging sites for another value.
- mechanism: asm/funcs/func_80073728.s:314 is `lw $a1, 0x10($s2)` and :317 `lw $v0, 0x14($s2)` with :323 `sw $t0, 0x14($s2)`, where $s2 is the incoming $a0 (set at :4, `addu $s2, $a0, $zero`). Both offsets are read before being written, so both are inputs.
- probe: grep of asm/funcs/func_80073728.s for `s2)` references and for the a0 copy at line 4
- result: This removes the semantic justification for the permuter's shared-pseudo trick (giving the a2 base and the s.zero10 store the same destination pseudo): the original source has no program reason to route a 0 through a variable here. It also confirms the field layout the candidate's S46C struct assumes.
- verdict: CONFIRMED

## [s6 — SOLVER] The residual is typed SCHED, and the target order is UNREACHABLE by statement order on this insn multiset: all 420 dependence-legal statement permutations of the half-1 window, replayed through the exact sched1+sched2 funnel, miss it. **KILLED (class, statement-order axis).**
- statement: `inverse_compose.py classify` types the whole 6-point residual as SCHED (176 vs 176
  register-blanked-identical instructions, 6 slots in a different order — no PRE-RA and no RA
  component at all). Within that layer, the goal for half 1 is the pick order
  `211 -> 205 -> 242 -> 240` (emission `addiu a0,sp,16; move a1,zero; lw v1,gp; addiu a2,s4,-K`
  against our `addiu a2; addiu a0; lw v1; move a1`). A depth-1 `perturb.py` search over ALL
  12,558 single atoms of pass-1 block 6 (add_dep / del_dep / luid / luid_move / cost) returns
  exactly 102 hits, every one of them a LUID move of insn 211 (`a2 = s4 - K`) to a position at or
  after uid 244 (the `func_80073728` call); the cheapest is
  `luid_move 211 -> immediately before 244`. No dependence edge, no edge deletion and no
  instruction-cost change reaches the goal alone. Insn 211's LUID must therefore exceed the
  LUIDs of the call's own argument-setup insns 240 (`a0 = sp+16`, luid 42) and 242
  (`a1 = 0`, luid 43), which GCC emits inside `expand_call` at the call itself — while 211's
  consumer 225 (`a2 += v0`) sits at luid 37, so no C statement ordering can put 211 there.
- mechanism: `sched.c:2464` (`rank_for_schedule`) makes INSN_LUID the FINAL tie-break, and in
  this window the four insns are mutually independent with equal INSN_PRIORITY (3) and equal
  dependence class against the last-scheduled insn, so the LUID comparison alone decides the
  order. LUIDs in sched1 are source RTL-emission order; LUIDs in sched2 are sched1's OUTPUT
  order. Priorities cannot differ without a different dependence graph, and the dependence graph
  is fixed by the (identical) insn multiset.
- probe: (a) `inverse_compose.py classify text1b func_8005D554 --target-object build/src/text1b.o
  --ours-object tmp/sandbox/func_8005D554/text1b.o`; (b)
  `sched_solver/perturb.py ... --pass 1 --block 6 --goal-before 211:205 --goal-before 205:242
  --goal-before 242:240 --depth 1` with and without `--atoms luid,luid_move`
  (`tmp/grind/func_8005D554/s6/perturb_p1.txt`, `perturb_p1_all.txt`); (c)
  `tmp/grind/func_8005D554/s6/stmtperm.py` — all 420 dependence-legal permutations of the eight
  half-1 source statements simulated through sched1; (d)
  `tmp/grind/func_8005D554/s6/compose.py` — the same 420 replayed through sched1 AND sched2, with
  sched2's LUIDs rebuilt from each permutation's sched1 emission order. compose.py carries a
  self-check that the control replay reproduces the real sched2 dump exactly (`replay == real:
  True`), so the funnel model is not an approximation here.
- result: 0 of 420 permutations reach the target order at either pass. The four spellings swept
  against the real compiler agree with the model: `v3_a2base_last_legal_slot` (the a2 base moved
  to the last slot the dependence graph allows, immediately after the zero10/one14/ret stores)
  measures 6/176, i.e. exactly the control; `v4_base_folded_into_store`
  (`s.zero1C = ((s32)r4 - K) + a2_offset`, the subtract born at the store) measures 15/176; and
  `v2_struct_via_pointer` (all struct stores through an `S46C *ps = &s;` local, to change the
  memory-dependence graph rather than the order) measures 60/179.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/sched.c:2464
- measured_on: HEAD main @ 07aadcc3, candidate.c applied to src/text1b.c, floor re-measured 6/176
  this session; no FAKE constructs present in the control or in any of the four swept variants.
  The class is the STATEMENT-ORDER axis for the half-1 window on the current insn multiset and
  dependence graph — a different multiset or a different dependence graph is NOT covered.

## [s6] Writing the argument struct through a pointer local (`S46C *ps = &s;`) — the one ordinary-C way to change sched.c's MEMORY dependence graph without changing the insn multiset — costs three instructions and 54 points. **KILLED (instance).**
- statement: `v2_struct_via_pointer`, identical to the candidate except that `s` is written as
  `ps->field` throughout and the call is `func_80073728((s32)ps, 0)`, measures 60/179 against the
  control's 6/176.
- mechanism: the pointer local becomes a pseudo that global-alloc must keep live across both
  calls, so `addiu a0,sp,16` is replaced by a copy from that pseudo plus its own initialisation,
  and the frame-address insn no longer sits in the window at all.
- probe: `tools/sweep_variants.py --func func_8005D554 --file text1b --variants
  tmp/grind/func_8005D554/s6/vars/` (`tmp/grind/func_8005D554/s6/sweep.json`).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 07aadcc3, candidate.c chassis, floor 6/176; no FAKE constructs present.
  Form banked at `rejected/struct-via-pointer-local-scores-60.c`.

## [s6] The depth-2 dependence-only search over the half-1 window returns 30 solutions, and every one of them either forces the call's argument-setup insns 240/242 ahead of the load 205 / the a2 base 211, or deletes the anti/output dependences those two insns carry on the `rand` call 201. **KILLED (instance — the dependence-edge axis at depth 2, dep atoms only).**
- statement: restricting `perturb.py`'s atom enumeration to the 421 `add_dep`/`del_dep` atoms whose
  both endpoints lie in the half-1 window and searching all pairs gives 30 hits: 24
  `add_dep`+`add_dep`, 4 `add_dep`+`del_dep`, 1 `del_dep`+`add_dep` and 1 `del_dep`+`del_dep`.
  The `del_dep`+`del_dep` solution is exactly `del_dep 240<-201` plus `del_dep 242<-201`; every
  `add_dep` solution is drawn from the set {205<-240, 205<-242, 211<-240, 211<-242, 242<-240,
  211<-205} in kind 0 or kind 14.
- mechanism: 240 (`a0 = sp+16`) and 242 (`a1 = 0`) set the argument hard registers `$a0`/`$a1`,
  which the preceding `jal rand` (uid 201) clobbers, so `sched.c`'s call handling gives each of
  them an unconditional output/anti dependence on 201. Those two edges are what pins the arg
  setup behind the rand call; without them the scheduler floats the setup forward and the
  rotation happens by itself. They are a property of the MIPS call-clobber model, not of the C.
  The `add_dep` half of the solution space is the mirror image: it forces 240/242 ahead by making
  the load or the a2 base depend on them, which would require `D_800A3418` to alias the frame or
  the a2 base to be computed from the literal second argument — neither is true of this program.
- probe: `tmp/grind/func_8005D554/s6/depth2_deps.py` -> `s6/depth2_deps.txt` (421 atoms, all
  pairs). The unrestricted depth-2 window search over all 934 window atoms
  (`s6/depth2_window.py`, background job, 1,792 hits) shows the same shape: 1,133 of the 1,792
  hits pair a dep atom or a cost atom with the same unspellable `luid`/`luid_move` of uid 211 to
  slot 244 that the depth-1 search already found.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 07aadcc3, candidate.c chassis, floor 6/176; model self-check baseline
  exact for both passes; no FAKE constructs present. This is the dependence-edge axis restricted
  to the half-1 window at depth 2 with dep atoms only — wider windows, depth 3, and pairs that
  mix a dep atom with a SPELLABLE luid move are not covered.

## [s6] The 6-point residual is entirely a sched.c ordering residual: inverse_compose.py classify reports SCHED with 176 honest vs 176 target instructions, register-blanked multisets equal AND register-bearing texts equal as a multiset, 6 slots in a different order.
- mechanism: classify compares the two objdump streams in three stages -- register-blanked multiset (PRE-RA), register-bearing multiset (RA), then position (SCHED). Equality at the first two stages means no instruction and no register assignment differs, so nothing upstream of sched.c can be the cause.
- probe: python3 tools/ra_solver/inverse_compose.py classify text1b func_8005D554 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_8005D554/text1b.o, with candidate.c applied to src/text1b.c and the sandbox re-measured at 6/176 first.
- result: FIRST DIVERGENCE: SCHED -- 'same instructions and registers, 6 slot(s) in a different order'. This settles the layer question that s1-s5 inferred from asm reading: there is no PRE-RA and no RA component left in this function.
- verdict: CONFIRMED

## [s6] In sched1 block 6 the target's half-1 order (emission addiu a0,sp,16 / move a1,zero / lw v1,gp / addiu a2,s4,-K) is reached by exactly 102 of the 12,558 single atoms, and every one of them is a LUID move of the a2-base insn 211 to a slot at or after the func_80073728 call insn 244; no add_dep, del_dep or cost atom reaches it.
- mechanism: sched.c:2464 makes INSN_LUID the final tie-break in rank_for_schedule. The four insns in the window are mutually independent, all at INSN_PRIORITY 3, and all in the same dependence class against the last-scheduled insn, so the LUID comparison alone decides their order. Insn 211 (LUID 31) must out-rank 240 (LUID 42, a0 = sp+16) and 242 (LUID 43, a1 = 0), which expand_call emits at the call itself, while 211's consumer 225 (a2 += v0) sits at LUID 37.
- probe: python3 tools/sched_solver/perturb.py tmp/sched_solver_work/text1b.sched.json --func func_8005D554 --pass 1 --block 6 --goal-before 211:205 --goal-before 205:242 --goal-before 242:240 --depth 1 --max 500, run twice: once with --atoms luid,luid_move (tmp/grind/func_8005D554/s6/perturb_p1.txt) and once over all atom classes (perturb_p1_all.txt). Model self-check: baseline exact for both passes.
- result: 102 hits, all luid/luid_move on uid 211; cheapest is 'luid_move 211 -> immediately before 244'. Zero add_dep, zero del_dep, zero cost hits. The pass-2 search (perturb_half1.txt) additionally shows that if sched1 merely emitted 211 after the three struct stores, sched2 would finish the rotation on its own.
- verdict: CONFIRMED

## [s6] No ordering of the eight source statements in the half-1 window produces the target instruction order: all 420 dependence-legal permutations, replayed through sched1 and then through sched2 with its LUIDs rebuilt from that permutation's sched1 emission order, miss the goal, and the compiler agrees with the model on the best-placed spelling.
- mechanism: sched.c:2464 -- INSN_LUID is the final tie-break in rank_for_schedule, sched1's LUIDs are source RTL-emission order and sched2's LUIDs are sched1's output order. The dependence graph and the INSN_PRIORITY values are fixed by the insn multiset, which is already target-identical, so statement order is the only input a C statement move can change.
- probe: tmp/grind/func_8005D554/s6/stmtperm.py (420 permutations through sched1) and tmp/grind/func_8005D554/s6/compose.py (the same 420 through sched1+sched2, with a self-check that the control replay reproduces the real sched2 dump exactly). Cross-checked against the real compiler with tools/sweep_variants.py over four bodies (tmp/grind/func_8005D554/s6/sweep.json).
- result: 0 of 420 at sched1 and 0 of 420 after sched2; compose.py prints 'real sched2 exact: True' and 'replay == real: True', so the funnel replay is exact rather than approximate. The compiler agrees: v3_a2base_last_legal_slot, which moves the a2 base to the last slot the dependence graph allows, measures 6/176 -- identical to the control -- and v4_base_folded_into_store (the subtract born at the s.zero1C store) measures 15/176.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 07aadcc3, memory/grind/func_8005D554/candidate.c applied to src/text1b.c, floor re-measured 6/176 this session; no FAKE constructs present in the control or in any swept variant. The class is the statement-order axis for the half-1 window on the current insn multiset and dependence graph.
- predicate_cite: tools/gcc-2.7.2/sched.c:2464

## [s6] Writing the argument struct through a pointer local (S46C *ps = &s; every field written as ps->field; the call taking (s32)ps), the one ordinary-C way to change sched.c's memory-dependence graph without changing the insn multiset, measures 60/179 against the control's 6/176.
- mechanism: The pointer local becomes a pseudo global-alloc must keep live across both calls, so addiu a0,sp,16 is replaced by a copy from that pseudo plus its own initialisation and the frame-address insn leaves the window entirely; the three extra instructions also change every downstream LUID.
- probe: tools/sweep_variants.py --func func_8005D554 --file text1b --variants tmp/grind/func_8005D554/s6/vars/ (tmp/grind/func_8005D554/s6/sweep.json); form banked at memory/grind/func_8005D554/rejected/struct-via-pointer-local-scores-60.c
- result: 60/179 -- 54 points and three instructions worse. The cheapest memory-aliasing lever available in ordinary C does not pay for itself on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 07aadcc3, candidate.c chassis, floor 6/176; no FAKE constructs present.

## [s6] The depth-2 dependence-only search over the half-1 window returns 30 solutions, and every one of them either forces the call's argument-setup insns 240/242 ahead of the load 205 / the a2 base 211, or deletes the anti/output dependences those two insns carry on the rand call 201.
- mechanism: 240 (a0 = sp+16) and 242 (a1 = 0) set the argument hard registers $a0/$a1, which the preceding jal rand (uid 201) clobbers, so sched.c's call handling gives each an unconditional output/anti dependence on 201. Those two edges pin the argument setup behind the rand call; without them the scheduler floats the setup forward and the rotation happens by itself. They are a property of the MIPS call-clobber model, not of the C. The add_dep half of the solution space is the mirror image and would require D_800A3418 to alias the frame, or the a2 base to be computed from the literal second argument.
- probe: tmp/grind/func_8005D554/s6/depth2_deps.py over the 421 add_dep/del_dep atoms whose endpoints both lie in the window, all pairs (tmp/grind/func_8005D554/s6/depth2_deps.txt); plus the unrestricted 934-atom depth-2 window search tmp/grind/func_8005D554/s6/depth2_window.py, which returned 1,792 hits.
- result: 30 dep-only hits: 24 add_dep+add_dep, 4 add_dep+del_dep, 1 del_dep+add_dep, 1 del_dep+del_dep (the last being exactly del_dep 240<-201 plus del_dep 242<-201). Every add_dep solution is drawn from {205<-240, 205<-242, 211<-240, 211<-242, 242<-240, 211<-205}. In the unrestricted search, 1,133 of the 1,792 hits pair a dep or cost atom with the same unspellable luid move of uid 211 to slot 244 that depth 1 already found.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 07aadcc3, candidate.c chassis, floor 6/176; model self-check baseline exact for both passes; no FAKE constructs present. This is the dependence-edge axis restricted to the half-1 window at depth 2 with dep atoms only.

## [s7] The instrumented cc1 (tools/gcc-2.7.2/cc1, self-checked byte-identical to the build cc1) reports pass-1 INSN_PRIORITY 3 for all four contested insns in half 1 (205 the lw, 211 the a2 base, 240 `a0 = fp+16`, 242 `a1 = 0`) and 6 for all four of their half-2 counterparts (310, 316, 346, 348). **CONFIRMED.**
- mechanism: `priority()` (tools/gcc-2.7.2/sched.c:1434) walks LOG_LINKS (an insn's PREDECESSORS) and takes `max(priority(pred) + insn_cost(pred, link, insn) - 1)` (sched.c:1497). A call sets `reg_pending_sets_all` (sched.c:1991/2095/2236), so every register-setting insn after a call in the same block hangs an anti-dependence off that call. `insn_cost` is 1 for every MIPS ALU insn and 2 only for a load's data-dependent consumer, so every post-call non-load-consumer inherits exactly the call's priority.
- probe: `BB2_PRIO_DEBUG=1 bash tmp/grind/func_8005D554/s7/run_dump.sh` -> `tmp/grind/func_8005D554/s7/prio.log` (45,426 lines); the half-1 region is anchored by the unique line `PRIODBG insn=240 pred=201 kind=14 pred_pri=3 cost=1 contrib=3`, which occurs exactly twice in the whole TU (once per scheduler pass).
- result: 201=3, 205=3, 206=4, 211=3, 217/220/222=4, 225=4, 228/231/234=3, 237=4, 240=3, 242=3, 244=4; half 2: 306=6, 310=6, 316=6, 331=7, 343=7, 346=6, 348=6, 350=7. The only priority-4 insns in the window are the load's consumers and their downstream.
- verdict: CONFIRMED

## [s7] The PRIORITY criterion of `rank_for_schedule` cannot be made to separate the a2 base insn from the call's argument-setup insns by any C-level variable reuse: an anti-dependence manufactured by sharing a local always points backward to an insn whose accumulated priority is BELOW the nearest preceding call's, so it is dominated. **KILLED (class).**
- statement: half-2's a2 base insn 316 already carries exactly the anti-dependences that a C-level borrow manufactures — `pred=225` (priority 4) and `pred=237` (priority 4), because `a2_offset` is one shared pseudo (reg/v 82) across both halves — and both contribute 4 against the `pred=306` call contribution of 6, so 316's priority is 6, identical to 346 (`a0 = fp+16`) and 348 (`a1 = 0`). Sharing the local buys nothing.
- mechanism: sched.c:1497 takes the MAX over predecessors of `priority(pred) + insn_cost - 1`. Because a call forces an anti-dep from every later register set (sched.c:1991/2095/2236), the nearest preceding call is always a predecessor, and priority accumulates monotonically along the block, so any predecessor EARLIER than that call has a strictly smaller (or equal) priority and contributes no more. The only way to exceed the call's level is `insn_cost == 2`, which sched.c grants only to a load's data-dependent consumer; the a2 base is an `addiu` off a callee-saved loop-invariant and consumes no load.
- probe: `BB2_PRIO_DEBUG=1` dump, `tmp/grind/func_8005D554/s7/prio.log`, lines for `insn=316` (three predecessor contributions printed: 306 -> 6, 225 -> 4, 237 -> 4) versus `SET insn=346 final_pri=6` and `SET insn=348 final_pri=6`.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 8a6f96b1, memory/grind/func_8005D554/candidate.c applied to src/text1b.c, floor re-measured 6/176 this session; no FAKE constructs present; instrumented cc1 self-checked byte-identical to the build cc1.
- predicate_cite: tools/gcc-2.7.2/sched.c:1497

## [s7] The CLASS criterion of `rank_for_schedule` never separates the four contested insns: across 203 rank comparisons whose both endpoints carry window uids, the class delta is 0 in every one. **KILLED (class).**
- statement: every `RANKDBG` line in the window prints `cls=3 ... cls2=3 val=0`, in both scheduler passes and in both loop halves. There is no last-scheduled insn for which any of {the lw, the a2 base, `a0 = fp+16`, `a1 = 0`, the three struct stores} falls out of class 3.
- mechanism: sched.c:2429 assigns class 3 whenever the candidate is not a LOG_LINK predecessor of `last_scheduled_insn` OR `insn_cost` of that link is 1. Every insn in the window is an ALU insn or a store, all of cost 1, so the `insn_cost == 1` disjunct fires unconditionally. Class 1 or 2 would require the candidate to be a load (cost 2) feeding the last-scheduled insn.
- probe: `BB2_RANK_DEBUG=1 bash tmp/grind/func_8005D554/s7/run_dump.sh` -> `tmp/grind/func_8005D554/s7/rank.log` (9,689 lines), filtered to comparisons whose `x` and `y` uids both lie in {201,205,206,211,225,228,231,234,237,240,242,244,306,310,316,331,334,337,340,343,346,348,350}.
- result: 203 comparisons, 0 with a nonzero class delta.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 8a6f96b1, candidate.c chassis, floor 6/176; no FAKE constructs present.
- predicate_cite: tools/gcc-2.7.2/sched.c:2429

## [s7] The `p_b2e0` declaration-pun object model (s6 frontier item 3 — the last untried STRUCTURAL axis) does not reach below the floor: a TU-local struct-pointer model scores 10/176 in the multiset-preserving form and 43/172 in the full form. **KILLED (instance).**
- statement: `v3_structptr_mixed` declares `struct EffEnt { u8 a[0xC]; u8 b[0xC]; u8 rest[0x24]; }` at block scope, replaces `p_b2ec`/`base_offset` with `struct EffEnt *e = (struct EffEnt *)&D_8009B2E0 + arg1`, and writes half 2's `s.p0` as `e->b + (D_800A3418 & 1) * 0xC`; it keeps the instruction count at 176 and scores 10 against the control's 6. `v2_structptr` additionally replaces `p_b2e0` (half 1's `s.p0` becomes `e->a`) and scores 43/172 — the typed model folds four instructions out of the setup block, so the multiset no longer matches.
- mechanism: the typed pointer materialises `base + arg1*0x3C` once as a single pseudo, which removes the separate `stride` add that the byte-pointer model keeps live for half 1's `s.p0`; the setup block's insn count and its LUID sequence both change, and the register allocation follows, but neither change reaches the half-1/half-2 rotation, which is decided entirely by the LUID order of insns born at the call.
- probe: `tools/sweep_variants.py --func func_8005D554 --file text1b --variants tmp/grind/func_8005D554/s7/vars/` -> `tmp/grind/func_8005D554/s7/sweep.json`; forms banked at `rejected/tu-local-struct-pointer-mixed-scores-10.c` and `rejected/tu-local-struct-pointer-object-model-scores-43.c`.
- result: control 6/176, v3_structptr_mixed 10/176, v2_structptr 43/172.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 8a6f96b1, candidate.c chassis, floor re-measured 6/176; no FAKE constructs present in the control or in either variant.

## [s7] Reassociating the a2 sum so the base subtraction is born after the shift chain (`a2_offset = shifted; a2_offset += (s32)r4 - K;`) — the maximum-LUID legal birth point for insn 211 approached from the operand-order side rather than the statement-order side — scores 15/176. **KILLED (instance).**
- statement: `v4_a2base_born_after_shift` applies the reassociation in both halves and measures 15/176 against the control's 6/176.
- mechanism: the reassociation makes the accumulate read the shift result first, so the `addiu` off the loop-invariant is emitted after the shift chain (raising its LUID from 31 towards 37) but is also re-canonicalised as `addu a2, v0, s4` + `addiu a2, a2, -K`, which changes the register-bearing text of the two insns even though the opcode multiset is preserved. It confirms s6's `v3_a2base_last_legal_slot` result (6/176, no gain) from the operand-order direction: LUID 31 -> 37 is irrelevant when the bar is LUID 42.
- probe: `tools/sweep_variants.py` sweep above; form banked at `rejected/a2base-born-after-shift-chain-scores-15.c`.
- result: 15/176.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 8a6f96b1, candidate.c chassis, floor 6/176; no FAKE constructs present.

## [s7] The instrumented cc1 (tools/gcc-2.7.2/cc1, self-checked byte-identical assembly to the build cc1) reports pass-1 INSN_PRIORITY 3 for all four contested insns of half 1 (205 the D_800A3418 load, 211 the a2 base addiu, 240 a0 = fp+16, 242 a1 = 0) and 6 for all four of their half-2 counterparts (310, 316, 346, 348); the only priority-4 insns in the window are the load's data consumers and their downstream.
- mechanism: priority() (tools/gcc-2.7.2/sched.c:1434) walks LOG_LINKS — an insn's PREDECESSORS — and takes max(priority(pred) + insn_cost(pred, link, insn) - 1) at sched.c:1497. A call sets reg_pending_sets_all (sched.c:1991/2095/2236), so every register-setting insn after a call in the same block hangs an anti-dependence off that call; insn_cost is 1 for every MIPS ALU insn and 2 only for a load's data-dependent consumer. Every post-call non-load-consumer therefore inherits exactly the call's priority.
- probe: BB2_PRIO_DEBUG=1 bash tmp/grind/func_8005D554/s7/run_dump.sh -> tmp/grind/func_8005D554/s7/prio.log (45,426 lines). The half-1 region is anchored by the line 'PRIODBG insn=240 pred=201 kind=14 pred_pri=3 cost=1 contrib=3', which occurs exactly twice in the whole TU (once per scheduler pass). RTL identities cross-checked against tmp/grind/func_8005D554/s7/dumps2/text1b.flow.
- result: 201=3, 205=3, 206=4, 211=3, 217/220/222=4, 225=4, 228/231/234=3, 237=4, 240=3, 242=3, 244=4; half 2: 306=6, 310=6, 316=6, 331=7, 343=7, 346=6, 348=6, 350=7. This also CORRECTS the s6 ledger's half-2 uid map, which was shifted by one entry: 343 is the s.zero1C store, 346 is a0 = fp+16, 348 is a1 = 0.
- verdict: CONFIRMED

## [s7] No C-level variable reuse can lift the a2 base insn above the argument-setup insns on the PRIORITY criterion, because an anti-dependence manufactured by sharing a local always points backward to an insn whose accumulated priority is at or below the nearest preceding call's and is therefore dominated by the call's own anti-dependence.
- mechanism: sched.c:1497 takes the MAX over predecessors of priority(pred) + insn_cost - 1. A call forces an anti-dep from every later register set (sched.c:1991/2095/2236), so the nearest preceding call is always a predecessor, and priority accumulates monotonically along the block. Any predecessor earlier than that call contributes no more than the call does. The only way to exceed the call's level is insn_cost == 2, which sched.c grants only to a load's data-dependent consumer; the a2 base is an addiu off a callee-saved loop invariant and consumes no load.
- probe: BB2_PRIO_DEBUG dump, tmp/grind/func_8005D554/s7/prio.log. Half-2's a2 base insn 316 ALREADY carries exactly the anti-deps a C-level borrow would manufacture, because a2_offset is one shared pseudo (reg/v 82) across both halves: the log prints 'insn=316 pred=306 ... contrib=6', 'insn=316 pred=225 ... pred_pri=4 contrib=4' and 'insn=316 pred=237 ... pred_pri=4 contrib=4', giving SET insn=316 final_pri=6 — identical to SET insn=346 final_pri=6 and SET insn=348 final_pri=6.
- result: The borrow's anti-deps contribute 4 against the call's 6 and are discarded by the max. Sharing the local buys zero priority. This kills the one lever the s7 priority measurement initially suggested was open.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 8a6f96b1, memory/grind/func_8005D554/candidate.c applied to src/text1b.c, floor re-measured 6/176 this session; no FAKE constructs present; instrumented cc1 self-checked byte-identical to the build cc1.
- predicate_cite: tools/gcc-2.7.2/sched.c:1497

## [s7] The last-scheduled CLASS criterion of rank_for_schedule never separates any two insns of the residual window: across all 203 rank comparisons whose two endpoints both carry window uids, in both scheduler passes and both loop halves, the class delta is 0 and both endpoints are class 3.
- mechanism: sched.c:2429 assigns class 3 whenever the candidate is not a LOG_LINK predecessor of last_scheduled_insn OR insn_cost of that link is 1. Every insn in the window is an ALU insn or a store, all of cost 1, so the insn_cost == 1 disjunct fires unconditionally. Falling to class 1 or 2 would require the candidate to be a load (cost 2) feeding the last-scheduled insn, and the only load in the window (205/310) is not a predecessor of any of the contested insns.
- probe: BB2_RANK_DEBUG=1 bash tmp/grind/func_8005D554/s7/run_dump.sh -> tmp/grind/func_8005D554/s7/rank.log (9,689 lines), filtered to comparisons whose x and y uids both lie in {201,205,206,211,225,228,231,234,237,240,242,244,306,310,316,331,334,337,340,343,346,348,350}.
- result: 203 comparisons, 0 with a nonzero class delta — every line reads 'cls=3 ... cls2=3 val=0'. Combined with the priority tie, this leaves INSN_LUID (sched.c:2464) as the sole surviving criterion, exactly as s6's model predicted, but now measured inside the compiler rather than inferred from it.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 8a6f96b1, candidate.c chassis, floor 6/176; no FAKE constructs present; instrumented cc1 self-checked byte-identical to the build cc1.
- predicate_cite: tools/gcc-2.7.2/sched.c:2429

## [s7] The p_b2e0 declaration-pun object model — s6 frontier item 3, the last untried structural axis — does not score below the control on this chassis: a TU-local struct-pointer model scores 10/176 in the multiset-preserving form and 43/172 in the full form against the control's 6/176.
- mechanism: The typed pointer materialises base + arg1*0x3C once as a single pseudo, which removes the separate stride add that the byte-pointer model keeps live for half 1's s.p0; the setup block's insn count and LUID sequence both change, and the register allocation follows, but neither change reaches the half-1/half-2 rotation, which is decided entirely by the LUID order of insns born at the call.
- probe: tools/sweep_variants.py --func func_8005D554 --file text1b --variants tmp/grind/func_8005D554/s7/vars/ -> tmp/grind/func_8005D554/s7/sweep.json. Variants v2_structptr (struct EffEnt { u8 a[0xC]; u8 b[0xC]; u8 rest[0x24]; } declared at block scope, both s.p0 sites written through it) and v3_structptr_mixed (only base_offset replaced, keeping the byte pointer for half 1).
- result: control 6/176, v1_control 6/176, v3_structptr_mixed 10/176, v2_structptr 43/172. Forms banked at rejected/tu-local-struct-pointer-mixed-scores-10.c and rejected/tu-local-struct-pointer-object-model-scores-43.c. The header route plus integration handoff that this axis would have required is therefore not worth opening.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 8a6f96b1, candidate.c chassis, floor re-measured 6/176; no FAKE constructs present in the control or in either variant.

## [s7] Reassociating the a2 sum so the base subtraction is born after the shift chain (a2_offset = shifted; a2_offset += (s32)r4 - K, applied in both halves) scores 15/176 against the control's 6/176.
- mechanism: The reassociation makes the accumulate read the shift result first, so the addiu off the loop invariant is emitted after the shift chain, raising its LUID from 31 towards 37, but the pair is re-canonicalised as addu a2,v0,s4 + addiu a2,a2,-K, which changes the register-bearing text of both insns even though the opcode multiset is preserved at 176. It confirms s6's v3_a2base_last_legal_slot result (6/176, no gain) from the operand-order direction rather than the statement-order direction: raising the base's LUID from 31 to 37 is irrelevant when the bar set by the argument setup is 42.
- probe: tools/sweep_variants.py sweep above (tmp/grind/func_8005D554/s7/sweep.json), variant v4_a2base_born_after_shift; form banked at rejected/a2base-born-after-shift-chain-scores-15.c.
- result: 15/176.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 8a6f96b1, candidate.c chassis, floor 6/176; no FAKE constructs present.

## [s8] The candidate's floor and the five closest banked forms all reproduce their recorded scores exactly on the current chassis, and `fake_ablate` finds no FAKE construct in the candidate to ablate.
- mechanism: The mandated kill re-audit exists because an instance kill measured with a FAKE carrier occupying the contested pseudo is not a kill. The candidate carries no `/* FAKE */` construct at all, so no lever in this ledger was ever measured behind one; and the chassis (HEAD main @ 4297dfd2) is byte-identical in behaviour to s7's, so the s2-s7 instance kills stand as recorded.
- probe: `tools/fake_ablate.py --func func_8005D554 --file text1b --candidate memory/grind/func_8005D554/candidate.c`, then `tools/sweep_variants.py --func func_8005D554 --file text1b --variants tmp/grind/func_8005D554/s8/reaudit/` over the five lowest-scoring banked forms.
- result: ablate reports "no FAKE-annotated constructs found ... nothing to ablate". Sweep: stores-before-a2-init 6/176, a2-init-adjacent-to-plus-eq 7/176, second-invariant-local-for-r4 8/177, shared-a2-base-direct-store 10/176, tail-store-order-transposed 10/176 — every score identical to its banked value. Control floor re-measured 6/176.
- verdict: CONFIRMED

## [s8] m2c's independent re-derivation types the callee as three-argument (`func_80073728(&sp10, 0, temp_a2)`), but adding that third argument is byte-inert on the candidate chassis: 6/176 with it and 6/176 without.
- mechanism: m2c infers the third argument from `$a2` being written in the insn before the `jal` with no proof of death — the same inference it makes for the definitely-spurious `rand(temp_a0)` two insns earlier. In GCC 2.7.2 `expand_call` evaluates every argument expression into a pseudo BEFORE emitting the hard-register moves, so an argument whose value the source already computed for a struct store expands to no new insns and to no new birth position: the value is already in a pseudo when the argument list is walked, and the only insn the third argument would add (a move to `$a2`) is coalesced away because local-alloc already seats that pseudo in `$a2`. The reading may still be the true signature of func_80073728; it is simply not a lever on the schedule.
- probe: `python3 tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax asm/funcs/func_8005D554.s`; then `tools/sweep_variants.py` over tmp/grind/func_8005D554/s8/v/ (m2c body, 2-arg vs 3-arg) and tmp/grind/func_8005D554/s8/v2/ (candidate body, 2-arg vs 3-arg). The 3-arg forms carry a block-scope `extern s32 func_80073728();` so the 2-argument sibling call in func_8005D46C still compiles.
- result: m2c body 75/176 (2-arg) and 75/176 (3-arg); candidate body 6/176 (control) and 6/176 (3-arg). Forms banked at rejected/m2c-fresh-decompile-shape-scores-75.c and rejected/three-argument-call-reading-inert-scores-6.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4297dfd2, candidate.c chassis and the fresh m2c chassis, floor re-measured 6/176; no FAKE constructs present in any variant.

## [s8] The full m2c re-derivation (single-assignment temps, no pointer locals, no constant holders, hoisted `sp40`) scores 75/176 against the control's 6/176, and the same body with func_8005D46C's field-store order scores 86/176.
- mechanism: Dropping the `p_b2e0`/`p_b388`/`p_b390` pointer locals forces the two `%hi/%lo` symbol bases to be re-materialised rather than held in `$fp`/`$s7` across the loop, and dropping the `c100`/`c1` holders changes which pseudos survive to local-alloc; the callee-saved seating that the candidate matches byte-for-byte is lost. The opcode multiset is still 176, so this is entirely a register-and-order divergence, not a count divergence — the m2c shape is a strictly worse chassis, not a different-length program.
- probe: `tools/sweep_variants.py --func func_8005D554 --file text1b --variants tmp/grind/func_8005D554/s8/v/`.
- result: v3_m2c_shape 75/176, v2_sibling_field_order 86/176, v1_m2c_3arg 75/176. Banked at rejected/m2c-fresh-decompile-shape-scores-75.c and rejected/m2c-shape-sibling-field-order-scores-86.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4297dfd2, fresh-m2c chassis, control floor 6/176; no FAKE constructs present.

## [s8] The matched sibling func_8005D46C's field-store order (`zero1C` written before `zero18`) transplanted onto the candidate chassis scores 28/178.
- mechanism: func_8005D46C writes both offset fields as literal zeros, so its field order is free; ours must consume the second and third `rand` results in a fixed sequence, and writing `zero1C` first forces BOTH offsets to be staged in locals across the third `rand` call. Two extra live values across a call cost a callee-saved seat and two instructions (178 vs 176), and the seating change propagates through the whole loop body.
- probe: `tools/sweep_variants.py --func func_8005D554 --file text1b --variants tmp/grind/func_8005D554/s8/v2/`, variant c2_cand_sibling_field_order.
- result: 28/178 against the control's 6/176. Banked at rejected/sibling-8005D46C-field-order-scores-28.c. func_8005D46C is now a SPENT sibling: it shares the struct, the callee and the file, and its only transferable spelling is measured and worse.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4297dfd2, candidate.c chassis, floor 6/176; no FAKE constructs present.

## [s8] Making the a2 base insn a data consumer of a LOAD moves it from before the call's argument setup to six slots after it — the first form in eight sessions that emits `addiu a2,...,-K` later than `addiu a0,sp,0x10` / `addu a1,zero,zero`.
- mechanism: This is the dependence graph, not INSN_LUID. sched.c:1497 gives a load's consumer `insn_cost` 2, so the base insn is not in the ready set at the cycle where the argument setup is chosen; the scheduler drains `a0`, the gp load and `a1` first and only then issues the base. s4's framing (evidence.md:537 — "expand_call emits argument setup at the call statement ... so no source-level statement order can produce that inequality") is correct about statement order and does NOT bound the dependence graph. The cost measured here is the load itself (+2 insns) plus a six-slot overshoot, and the base is re-canonicalised from `addiu a2,s4,-K` to `lw a2,0x44(sp)` + `addiu a2,a2,-K`.
- probe: `tools/sweep_variants.py --func func_8005D554 --file text1b --variants tmp/grind/func_8005D554/s8/v3/` plus a disassembly of the applied d1 build (`tmp/grind/func_8005D554/s8/dis.sh`, `cmp.py 82 100`). Variants: d1 both random bases in `u32 rb[2]`; d2 only r4 in `u32 rb[1]`; d3 `S46C s` at block scope; d4 `(s32)&s.p0` as the first argument; d5 both bases read through pointer locals.
- result: d1 32/178 with the window order `addiu a0,sp,16 / lw a2,68(sp) / lw v1,0(gp) / move a1,zero / sw zero,32(sp) / sw s4,36(sp) / sw s1,28(sp) / addiu a2,a2,-12`; d2 6/176 (a one-element constant-indexed array is kept in a register by GCC 2.7.2, so the lever does not fire and the body is byte-identical to the control); d3 6/176; d4 6/176; d5 61/186. Banked at rejected/both-bases-in-local-array-load-consumer-scores-32.c and rejected/bases-read-through-pointer-scores-61.c.
- verdict: CONFIRMED

## [s8] Every source-level re-spelling of the a2 sum that keeps 176 instructions still lands on the s4 quantization {6, 15}; only a dependence-graph change moves the schedule.
- mechanism: cse and combine canonicalise `r4 - K + jitter` to the same two-insn pair regardless of how the source associates it, so the only thing a re-spelling can change is which of the two insns reads the loop-invariant register — which is the 15-scoring variant — or nothing at all, which is the 6-scoring variant. Forms that break the canonicalisation (`r4 - (K - jitter)`) cost an extra insn instead.
- probe: `tools/sweep_variants.py --func func_8005D554 --file text1b --variants tmp/grind/func_8005D554/s8/v4/`.
- result: e4 `(s32)(r4 - K)` 6/176; e2 `jitter - (K - (s32)r4)` 15/176; e3 `a2_offset = (s32)r4 + jitter; a2_offset -= K` 15/176; e5 `s.zero1C = ((s32)r4 - K) + jitter` with no local at all 15/176; e1 `(s32)r4 - (K - jitter)` 54/178. Banked at rejected/jitter-minus-const-minus-r4-scores-15.c, rejected/sum-then-subtract-const-scores-15.c, rejected/no-local-single-expr-zero1C-scores-15.c, rejected/base-minus-const-minus-jitter-scores-54.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4297dfd2, candidate.c chassis, floor 6/176; no FAKE constructs present in any variant.

## [s8] The candidate's floor and the five closest banked forms all reproduce their recorded scores exactly on the current chassis, and fake_ablate finds no FAKE construct in the candidate to ablate.
- mechanism: The mandated kill re-audit exists because an instance kill measured with a FAKE carrier occupying the contested pseudo is not a kill. The candidate carries no /* FAKE */ construct at all, so no lever in this ledger was measured behind one, and the chassis is unchanged from s7.
- probe: tools/fake_ablate.py --func func_8005D554 --file text1b --candidate memory/grind/func_8005D554/candidate.c, then tools/sweep_variants.py over tmp/grind/func_8005D554/s8/reaudit/ (the five lowest-scoring banked forms).
- result: ablate: 'no FAKE-annotated constructs found ... nothing to ablate'. Sweep: stores-before-a2-init 6/176, a2-init-adjacent-to-plus-eq 7/176, second-invariant-local-for-r4 8/177, shared-a2-base-direct-store 10/176, tail-store-order-transposed 10/176 - every score identical to its banked value. Control floor re-measured 6/176 with 176/176 instructions.
- verdict: CONFIRMED

## [s8] m2c's independent re-derivation types the callee as three-argument (func_80073728(&s, 0, a2)), but adding that third argument measures 6/176 on the candidate chassis, the same as the 2-argument control, and 75/176 on the m2c chassis, the same as its own 2-argument control.
- mechanism: m2c infers the third argument only because $a2 is written in the insn before the jal and it cannot prove $a2 dead - the same inference it makes for the definitely-spurious rand(temp_a0) two insns earlier. GCC 2.7.2 expand_call evaluates every argument expression into a pseudo BEFORE emitting the hard-register moves, so an argument whose value the source already computed for a struct store adds no insn and no new birth position; the move to $a2 is coalesced because local-alloc already seats that pseudo in $a2.
- probe: python3 tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax asm/funcs/func_8005D554.s; then tools/sweep_variants.py over tmp/grind/func_8005D554/s8/v/ (m2c body, 2-arg vs 3-arg) and tmp/grind/func_8005D554/s8/v2/ (candidate body, 2-arg vs 3-arg). The 3-arg forms carry a block-scope 'extern s32 func_80073728();' so the 2-argument sibling call in func_8005D46C still compiles.
- result: m2c body 75/176 (2-arg) and 75/176 (3-arg); candidate body 6/176 (control) and 6/176 (3-arg). Banked at rejected/three-argument-call-reading-inert-scores-6.c. The 3-argument reading may still be the true signature of func_80073728; it is simply not a lever on the schedule.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4297dfd2, candidate.c chassis and the fresh m2c chassis, floor re-measured 6/176; no FAKE constructs present in any variant.

## [s8] The full m2c re-derivation (single-assignment temps, no pointer locals, no constant holders, hoisted sp40 base) scores 75/176 against the control's 6/176, and the same body carrying func_8005D46C's field-store order scores 86/176.
- mechanism: Dropping the p_b2e0/p_b388/p_b390 pointer locals forces the two %hi/%lo symbol bases to be re-materialised rather than held in $fp/$s7 across the loop, and dropping the c100/c1 holders changes which pseudos survive to local-alloc; the callee-saved seating that the candidate matches byte-for-byte is lost. The opcode multiset stays at 176, so this is a register-and-order divergence, not a count divergence.
- probe: tools/sweep_variants.py --func func_8005D554 --file text1b --variants tmp/grind/func_8005D554/s8/v/
- result: v3_m2c_shape 75/176, v1_m2c_3arg 75/176, v2_sibling_field_order 86/176. Banked at rejected/m2c-fresh-decompile-shape-scores-75.c and rejected/m2c-shape-sibling-field-order-scores-86.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4297dfd2, fresh-m2c chassis, control floor 6/176; no FAKE constructs present.

## [s8] The matched sibling func_8005D46C's field-store order (zero1C written before zero18) transplanted onto the candidate chassis scores 28/178.
- mechanism: func_8005D46C writes both offset fields as literal zeros, so its field order is free; ours must consume the second and third rand results in a fixed sequence, so writing zero1C first forces BOTH offsets to be staged in locals across the third rand call. Two extra values live across a call cost a callee-saved seat and two instructions, and the seating change propagates through the whole loop body.
- probe: tools/sweep_variants.py --func func_8005D554 --file text1b --variants tmp/grind/func_8005D554/s8/v2/, variant c2_cand_sibling_field_order.
- result: 28/178 against the control's 6/176. Banked at rejected/sibling-8005D46C-field-order-scores-28.c. func_8005D46C is now a SPENT sibling: it shares the struct, the callee and the file, and its only transferable spelling is measured and worse.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4297dfd2, candidate.c chassis, floor 6/176; no FAKE constructs present.

## [s8] Making the a2 base insn a data consumer of a LOAD moves it from before the call's argument setup to six slots after it - the first form in eight sessions that emits addiu a2,...,-K later than addiu a0,sp,0x10 and addu a1,zero,zero.
- mechanism: This is the dependence graph, not INSN_LUID. sched.c:1497 gives a load's consumer insn_cost 2, so the base insn is not in the ready set at the cycle where the argument setup is chosen; the scheduler drains a0, the gp load and a1 first and only then issues the base. s4's framing (evidence.md:537, 'expand_call emits argument setup at the call statement ... so no source-level statement order can produce that inequality') is correct about statement order and does not bound the dependence graph.
- probe: tools/sweep_variants.py over tmp/grind/func_8005D554/s8/v3/, then applying d1 to src/text1b.c, running sandbox, disassembling the sandbox object (tmp/grind/func_8005D554/s8/dis.sh) and aligning it to asm/funcs/func_8005D554.s (cmp.py 82 100).
- result: d1 (u32 rb[2] holding both random bases) 32/178, window order 'addiu a0,sp,16 / lw a2,68(sp) / lw v1,0(gp) / move a1,zero / sw zero,32(sp) / sw s4,36(sp) / sw s1,28(sp) / addiu a2,a2,-12' - the base insn six slots past the argument setup, versus the control's slot-88 position before it. Cost: the load itself (+2 insns) and the overshoot; the base is re-canonicalised to lw + addiu a2,a2,-K. d2 (u32 rb[1], r5 left scalar) 6/176 - the lever does not fire, GCC 2.7.2 keeps a one-element constant-indexed array in a register. d3 (S46C s at block scope) 6/176, d4 ((s32)&s.p0 as first argument) 6/176, d5 (bases read through pointer locals) 61/186.
- verdict: CONFIRMED

## [s8] Every source-level re-spelling of the a2 sum that keeps 176 instructions still lands on the s4 quantization {6, 15} on this chassis.
- mechanism: cse and combine canonicalise r4 - K + jitter to the same two-insn pair regardless of how the source associates it, so a re-spelling can only change which of the two insns reads the loop-invariant register (the 15-scoring variant) or nothing at all (the 6-scoring variant). Forms that break the canonicalisation, such as r4 - (K - jitter), cost an extra instruction instead.
- probe: tools/sweep_variants.py --func func_8005D554 --file text1b --variants tmp/grind/func_8005D554/s8/v4/
- result: e4 (s32)(r4 - K) 6/176; e2 jitter - (K - (s32)r4) 15/176; e3 a2_offset = (s32)r4 + jitter then a2_offset -= K 15/176; e5 s.zero1C = ((s32)r4 - K) + jitter with no local at all 15/176; e1 (s32)r4 - (K - jitter) 54/178. Banked at rejected/jitter-minus-const-minus-r4-scores-15.c, rejected/sum-then-subtract-const-scores-15.c, rejected/no-local-single-expr-zero1C-scores-15.c, rejected/base-minus-const-minus-jitter-scores-54.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 4297dfd2, candidate.c chassis, floor 6/176; no FAKE constructs present in any variant.


## s9 (forensics, 2026-09-09) — hypotheses

Chassis for every entry below: HEAD main @ 7dcbdc8e, `memory/grind/func_8005D554/candidate.c`
applied to `src/text1b.c`, control floor RE-MEASURED at 6/176. No FAKE constructs present in the
control or in any variant measured this session.

### H-s9-1 (CONFIRMED) — the 6-point residual is one boolean on one insn
STATEMENT: at sched1 the entire 3-slot-per-half residual is decided by a single ready-list pick;
if the a2-base insn were selected at that one point instead of the a1 argument move, the rest of
the window reproduces the target byte-for-byte with no other change.
MECHANISM: `schedule_block` is backward (sched.c:4036-4038, selected-first = emitted-last). At the
divergence the ready list is `[242(p=3,l=43) 240(p=3,l=42) 211(p=3,l=31)]`; priority ties,
last-scheduled class ties (s7), `potential_hazard` ties structurally (sched.c:2708-2723, strict
comparison keeps `ready[0]`), so `rank_for_schedule`'s final criterion
`INSN_LUID(tmp) - INSN_LUID(tmp2)` (sched.c:2462) picks the highest LUID, 242. Selecting 211 there
leaves {242,240} plus the one-cycle-blocked lw 205, which re-enters with LAUNCH_PRIORITY and is
picked next, then 242 (l=43), then 240 (l=42) — emitted `240,242,205,211` = the target.
PROBE: instrumented cc1 with `BB2_SCHED_DEBUG=1`; `tmp/grind/func_8005D554/s9/sched_window_half1.log`.
RESULT: measured, not inferred — the full pick/ready trace for both halves is banked.

### H-s9-2 (KILLED, instance) — the two loop halves are not separable
STATEMENT: applying a lever to half 2 alone does not fix half 2 alone; the two halves' sched1
windows are structurally identical on this chassis (same insn kinds, same priority level, same
sign and comparable size of the LUID gap), so half 2's window offers no load or hazard the first
half's lacks.
MECHANISM: half 2's window is 316 (a2 base, l=76), 334/337/340 (stores, l=84/85/86), 346 (l=88),
348 (l=89), 310 (the lw); all priority 6; selection order `340,337,334,348,310,346,316`. The lw is
`SELBLOCK`ed for one cycle in both halves. s8's premise that only half 2's window contains a load
is refuted by the trace.
PROBE: `BB2_SCHED_DEBUG=1` trace, `tmp/grind/func_8005D554/s9/sched_window_half2.log`.
MEASURED_ON: HEAD main @ 7dcbdc8e, candidate.c chassis, control floor 6/176; no FAKE constructs.

### H-s9-3 (KILLED, instance) — combine manufactures no insn in the contested window
STATEMENT: the `.loop`-to-`.combine` insn-set diff for this function shows exactly one
combine-created insn, `(insn 393 (use (reg:SI 119)))` attached to the first `rand()` call at the
top of the function, and none in block 6 or in either call window, so the s7 route of an a2 base
manufactured by combine at a late i3 position has no instance here.
MECHANISM: `combine.c` places a combined insn at i3's position; with no combination occurring in
this region there is no i3 to inherit a late position from.
PROBE: `tmp/grind/func_8005D554/s9/insns.py loop combine`; output
`tmp/grind/func_8005D554/s9/combine_created_insns.txt`.
MEASURED_ON: HEAD main @ 7dcbdc8e, candidate.c chassis, floor 6/176; no FAKE constructs.

### H-s9-4 (KILLED, class) — no INSN_PRIORITY lever can place the a2 base on the target slot
STATEMENT: every lever that raises the a2-base insn's INSN_PRIORITY above 3 also raises it above
the three struct stores, which sit at priority 3 in the same window, so the insn is selected
before them and emitted six slots past the target position; the required rank is strictly between
two insns that share priority 3, and `rank_for_schedule` compares priority before anything else
(sched.c:2417), so no priority value can express it.
MECHANISM: `adjust_priority` (sched.c:2542) raises a ready insn to `max_priority` when
`birthing_insn_p` (sched.c:2505, `reg_n_sets[dest] == 1`) holds; a load producer instead gives
`priority(x) + insn_cost - 1 = 3 + 2 - 1 = 4` (sched.c:1497). Both land the insn in a strictly
higher group than the stores 228/231/234, reproducing the six-slot overshoot s8 measured with
`u32 rb[2]`. Within one priority group the only remaining discriminators are `potential_hazard`
(structural tie: `addsi3_internal` vs `addsi3_internal`/`movsi_internal2` on one unit) and LUID.
PROBE: `BB2_SCHED_DEBUG=1` ADJPRI/SELBEST trace plus the sched.c predicates; corroborated by s8's
measured `rb[2]` overshoot (six slots, +2 insns, 32/178).
PREDICATE_CITE: tools/gcc-2.7.2/sched.c:2417
MEASURED_ON: HEAD main @ 7dcbdc8e, candidate.c chassis, floor 6/176; no FAKE constructs.

### H-s9-5 (KILLED, instance) — splitting the a2 base constant does not lengthen its chain
STATEMENT: spelling the a2 base as two constant steps (`a2_offset = (s32)r4 - 0x8;
a2_offset -= 4;` in both halves) does not add a dependent insn to the a2 chain and does not change
any scheduling decision.
MECHANISM: cse/combine re-fold the two `addiu` constants into one before sched1 sees the block, so
`reg_n_sets` and the chain length are unchanged.
PROBE: applied and measured with `sandbox func_8005D554 --disable all`.
RESULT: 6/176, byte-identical to the control. Banked at
`rejected/a2-base-constant-split-refolds-by-cse-scores-6.c`.
MEASURED_ON: HEAD main @ 7dcbdc8e, candidate.c chassis, control floor 6/176; no FAKE constructs.

### H-s9-6 (CONFIRMED) — the s8 candidate.c did not compile as banked
STATEMENT: candidate.c as written by s8 contained a FAKE annotation with its comment delimiters
inside the leading block comment, which terminated that comment early and made `src/text1b.c` fail
to parse; the sandbox reported this as `func_8005D554 not found in ...text1b.o`, not as a compile
error.
PROBE: hand compile via `tmp/grind/func_8005D554/s9/cc.sh` -> `text1b.c:2769: parse error before
'in'`. Fixed in place; the floor then measured 6/176 as recorded.

## [s9] The entire 3-slot-per-half residual is decided by a single sched1 ready-list pick: at clock 64 the ready list is [242(p=3,l=43) 240(p=3,l=42) 211(p=3,l=31)] (the two expand_call argument moves and the a2 base); selecting 211 there, and nothing else, reproduces the target's emission order 240,242,205,211 exactly.
- mechanism: GCC 2.7.2 schedule_block is a BACKWARD list scheduler (sched.c:4036-4038 prepends each selected insn, so selected-first == emitted-last). At that pick INSN_PRIORITY ties at 3, the last-scheduled class ties 3-vs-3 (s7), and schedule_select's potential_hazard ties structurally (sched.c:2708-2723, strict comparison keeps ready[0]), so rank_for_schedule falls through to its final criterion INSN_LUID(tmp) - INSN_LUID(tmp2) (sched.c:2462) and takes the highest LUID, 242. With 211 taken instead, the one-cycle-SELBLOCKed lw 205 (load-consumer cost, sched.c:1497) re-enters with LAUNCH_PRIORITY and is picked next, then 242 (l=43), then 240 (l=42). sched2 is a no-op here: its LUIDs (211=25, 240=26, 205=27, 242=28) already equal our emitted order.
- probe: Ran tools/gcc-2.7.2/cc1 with BB2_SCHED_DEBUG=1 (SCHEDDBG PICK/SELBEST/SELBLOCK/ADJPRI hooks) over src/text1b.c with candidate.c applied; extracted the full pick-and-ready trace for both call windows.
- result: Measured selection order 234,231,228,242,205,240,211 -> emitted 211,240,205,242,228,231,234, which is byte-for-byte our asm; the target needs selection 234,231,228,211,205,242,240. The residual is therefore the single boolean INSN_LUID(a2 base) > INSN_LUID(a1 argument move). Trace banked at tmp/grind/func_8005D554/s9/sched_window_half1.log.
- verdict: CONFIRMED

## [s9] The two loop halves are not separable: their sched1 windows are structurally identical on this chassis (same insn kinds, same priority level, same sign and comparable size of the LUID gap), so half 2 offers no load or hazard that half 1's window lacks.
- mechanism: Half 2's window is 316 (a2 base, l=76), 334/337/340 (the three struct stores, l=84/85/86), 346 (l=88) and 348 (l=89), all at INSN_PRIORITY 6, with the lw 310 SELBLOCKed for one cycle exactly as 205 is in half 1. Selection order 340,337,334,348,310,346,316 mirrors half 1 pick for pick. s8's frontier premise that only half 2's window already contains a load is refuted by the trace.
- probe: Same BB2_SCHED_DEBUG=1 trace, half-2 window extracted to tmp/grind/func_8005D554/s9/sched_window_half2.log.
- result: Both halves fail on the same single decision with the same LUID gap sign (31 vs 42/43 and 76 vs 88/89). No asymmetric form can move one half without the other on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 7dcbdc8e, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s9] Combine manufactures no insn in either contested window of this function, so the s7 route of an a2 base placed late by combine at an i3 position has no instance on this chassis.
- mechanism: combine.c places a combined insn at i3's position; the .loop-to-.combine insn-set diff shows exactly one insn added across the whole function, (insn 393 (use (reg:SI 119))) attached to the first rand() call at the top, and none removed in block 6.
- probe: Wrote tmp/grind/func_8005D554/s9/insns.py to parse both -da dumps into insn-UID sets and diff them; output banked at tmp/grind/func_8005D554/s9/combine_created_insns.txt.
- result: ONLY IN combine: [393]. ONLY IN loop: []. Nothing combine creates sits in or near either call window.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 7dcbdc8e, candidate.c chassis, floor 6/176; no FAKE constructs present.

## [s9] No INSN_PRIORITY lever can place the a2 base on the target slot, because the required rank is strictly between insns that all carry priority 3 and rank_for_schedule compares priority before every other criterion.
- mechanism: The a2 base must rank BELOW the three struct stores 228/231/234 and ABOVE the two argument moves 240/242, and all five carry INSN_PRIORITY 3. adjust_priority (sched.c:2542) raises a ready insn to max_priority when birthing_insn_p (sched.c:2505, reg_n_sets[dest] == 1) holds, and a load producer gives priority(x) + insn_cost - 1 = 3 + 2 - 1 = 4 (sched.c:1497); either lands the a2 base in a strictly higher group than the stores, so it is selected before them and emitted six slots past the target. Within one priority group the only remaining discriminators are potential_hazard (a structural tie: addsi3_internal against addsi3_internal and movsi_internal2 on one function unit) and LUID.
- probe: BB2_SCHED_DEBUG=1 ADJPRI/SELBEST trace (ADJPRI insn=211 deaths=0 birth=0 pri=3, while 205/206/217/219/220/222/223 all carry p=2130706433) read against the sched.c predicates; corroborated by s8's measured u32 rb[2] result, six-slot overshoot at 32/178.
- result: The priority axis is a group key, not a tunable slot delay; every spelling that raises the a2 base's priority reproduces s8's six-slot overshoot. Only LUID can express the required rank.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 7dcbdc8e, candidate.c chassis, floor 6/176; no FAKE constructs present.
- predicate_cite: tools/gcc-2.7.2/sched.c:2417

## [s9] Splitting the a2 base constant into two steps (a2_offset = (s32)r4 - 0x8; a2_offset -= 4;) in both halves does not lengthen the a2 dependence chain and changes no scheduling decision.
- mechanism: cse and combine re-fold the two addiu constants into a single addiu before sched1 sees the block, so reg_n_sets and the chain length are unchanged and adjust_priority still reports birth=0, pri=3.
- probe: Applied the variant to src/text1b.c and measured with sandbox func_8005D554 --disable all.
- result: 6/176, byte-identical to the control. This was the mandated kill re-audit of the closest banked instance kill (s8's a2-sum quantization to {6,15}); the s8 kill reproduces on the current chassis. Banked at memory/grind/func_8005D554/rejected/a2-base-constant-split-refolds-by-cse-scores-6.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 7dcbdc8e, candidate.c chassis, control floor 6/176; no FAKE constructs present.

## [s9] The candidate.c banked by s8 did not compile: it carried a FAKE annotation with its comment delimiters inside the leading block comment, which closed that comment early.
- mechanism: The inner comment terminator ends the header block comment at what s8 intended as prose, so the remaining header text is parsed as C. The sandbox surfaces this not as a compile error but as 'score unavailable: func_8005D554 not found in tmp/sandbox/func_8005D554/text1b.o', whose suggested cause (a sibling index-based reorder rule truncating the pipeline) is misleading.
- probe: Hand-compiled the preprocessed TU with tools/gcc-2.7.2/cc1 via tmp/grind/func_8005D554/s9/cc.sh.
- result: text1b.c:2769: parse error before 'in'. Fixed in candidate.c; the floor then measured 6/176. Future sessions seeing the 'not found in text1b.o' sandbox message should hand-compile the TU first.
- verdict: CONFIRMED

## s10 (forensics, 2026-09-09) — hypotheses

## [s10] The candidate's floor reproduces at 6/176 on HEAD main @ c09c5da8 and `fake_ablate` again finds no FAKE-annotated construct in it, so every banked lever in this ledger was measured with the a2-site pseudo unoccupied by a carrier. **CONFIRMED.**
- probe: re-applied `memory/grind/func_8005D554/candidate.c` to `src/text1b.c`, ran `sandbox func_8005D554 --disable all` (score 6, target_insns 176, build_insns 176), then `python3 tools/fake_ablate.py --func func_8005D554 --file text1b --candidate memory/grind/func_8005D554/candidate.c`.
- result: score 6/176; fake_ablate prints "no FAKE-annotated constructs found ...; nothing to ablate".

## [s10] `potential_hazard` cannot separate the a2 base from the two argument moves at the deciding clock-64 pick, because MIPS type `arith` matches no `define_function_unit` and `potential_hazard` returns 0 for an insn whose `insn_unit` is negative. **KILLED (class).**
- mechanism: `potential_hazard` (sched.c:1327) only produces a positive cost when `insn_unit (insn) >= 0` (sched.c:1335) and `function_units[unit].max_blockage > 1` (sched.c:1338); when `insn_unit` returns -1 the fallback loop at sched.c:1360 iterates zero times. `mips.md` (lines 153-260) defines units only for `load`/`store`/`xfer` ("memory"), `hilo`/`imul`/`idiv` ("imuldiv") and the FP types; `arith` is absent, so `addiu a2,s4,-K`, `addiu a0,sp,0x10` and `move a1,$zero` all tie at hazard 0 and `schedule_select`'s strict `>` scan keeps the LUID-sorted head. The same mechanism, with the opposite sign, is what lets the three struct stores (unit 0 "memory", `maxb=3`) beat the higher-LUID argument moves at clocks 61-63.
- probe: BB2_SCHED_DEBUG=1 trace of the control (`tmp/grind/func_8005D554/s10/sched.log`), plus reading `tools/gcc-2.7.2/sched.c:1327-1366` and the `define_function_unit` table in `tools/gcc-2.7.2/config/mips/mips.md:153-260`.
- result: at clock 64 the trace prints `SELBEST clock=64 insn=242 pos=1` over ready `[242(p=3,l=43) 240(p=3,l=42) 211(p=3,l=31)]` — the hazard scan returns the group head, i.e. the tie is total. Any hazard-winning spelling of the a2 base would have to be a memory or imuldiv insn, which is a different opcode from the target's `addiu $a2, $s4, -0xC`.
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/sched.c:1338
- measured_on: HEAD main @ c09c5da8, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s10] The last-scheduled CLASS criterion of `rank_for_schedule` is 3-vs-3 for the a2 base and both argument moves under EVERY possible `last_scheduled_insn`, not just the ones our stream produces. **KILLED (class).**
- mechanism: `rank_for_schedule` assigns class 3 whenever `link == 0 || insn_cost (tmp, link, last_scheduled_insn) == 1` (sched.c:2429 and its twin at 2437), and `insn_cost` (sched.c:1372) clamps its result to 1 for any insn whose `result_ready_cost` is below 1 — which is every insn with no function unit, i.e. every `arith`/`move`. The a2 base and both argument moves are arith, so they are class 3 regardless of what was scheduled last, including the call insn itself. This upgrades s7's finding (class delta 0 across 203 measured comparisons) from an instance observation to a predicate.
- probe: read `tools/gcc-2.7.2/sched.c:2420-2445` and `sched.c:1372-1398`; cross-checked against the RANKDBG/SCHEDDBG traces in `tmp/grind/func_8005D554/s10/sched.log`.
- result: no comparison in the trace produces a nonzero class delta, and the source shows why: the class-3 branch is taken for any arith producer.
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/sched.c:2429
- measured_on: HEAD main @ c09c5da8, candidate.c chassis, control floor 6/176; no FAKE constructs present.

## [s10] No source-side placement of the a2 base can give it an INSN_LUID above the call's hard-register argument moves, because `expand_call` emits those moves after every argument expression has been evaluated and after every preceding statement's expansion. **KILLED (class).**
- mechanism: `expand_call`'s final register-load loop calls `emit_move_insn (reg, args[i].value)` at calls.c:1880, under the comment "Their expressions were already evaluated" (calls.c:1845), and the call insn is emitted immediately after. Every insn produced by expanding a source statement that must complete before the call — which includes any value stored into the argument struct — therefore carries a strictly lower LUID. `rank_for_schedule` falls through to `INSN_LUID (tmp) - INSN_LUID (tmp2)` (sched.c:2462) and takes the largest, so the argument moves win the clock-64 pick unconditionally.
- probe: read `tools/gcc-2.7.2/calls.c:1843-1900`; measured the control trace (LUID 42/43 for the two argument moves, 41 for the highest other window insn, 31 for the a2 base) and then built form **v1** (`tmp/grind/func_8005D554/s10/v1.c`) with both halves rewritten so the two a2 statements sit at the last possible pre-call position — immediately before the `s.zero1C` store and after the `zero10`/`one14`/`ret` stores.
- result: v1 scores **6/176**. Its a2 base is uid 220 (`(set (reg/v:SI 6 a2) (plus:SI (reg/v:SI 20 s4) (const_int -12)))`, verified in `tmp/grind/func_8005D554/s10/sched2_seg.txt`), its LUID rose from 31 to 34, and the argument moves stayed at exactly 42/43. The emitted rotation is unchanged: `[addiu a2,s4,-12][addiu a0,sp,16][lw v1][move a1,zero]`. Banked at `rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c`. This also disposes of the s9 frontier's cse-operand-order route: cse does not move insns across statements, so it cannot lift the base past a bound statement placement itself cannot reach.
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/calls.c:1880
- measured_on: HEAD main @ c09c5da8, candidate.c chassis, control floor 6/176 and v1 6/176; no FAKE constructs present in either.

## [s10] Reload inserts no instruction into either contested window of this function, so the s9 frontier's "a2 base materialised by reload after the argument moves" route has no instance. **KILLED (instance).**
- mechanism: reload1.c inserts reload insns at the use point, and sched2 renumbers LUIDs over the post-reload stream — but only if a pseudo actually fails to get a hard register. `tmp/grind/func_8005D554/s10/greg_seg.txt` (the `func_8005D554` segment of `text1b.greg`) shows 18 pseudos to allocate and 18 register dispositions, zero `Reloads for insn` entries and zero `reload_in`/`reload_out` entries; the only reload activity in the whole function is `Spilling reg 7` (hard `$a3`) for insn 133, outside both windows. A reload insertion for the a2 base pseudo would in any case be a memory reference against its stack slot — an extra instruction beyond 176 and a different opcode from the target's `addiu a2,s4,-K`.
- probe: ran the instrumented cc1 with `-da` (`tmp/grind/func_8005D554/s10/sched_dump.sh`), extracted the function's segment from `text1b.greg`, and counted reload markers.
- result: 0 reload insertions in the function; 18/18 pseudos allocated.
- kill_scope: instance
- measured_on: HEAD main @ c09c5da8, candidate.c chassis, control floor 6/176; no FAKE constructs present.

## [s10] func_80073728 is a TWO-argument function: it reads $a0 and $a1 at entry and never reads $a2 before overwriting it, so the caller's $a2 is pure scratch and m2c's three-argument reading (s8) is wrong. **CONFIRMED.**
- probe: grepped `asm/funcs/func_80073728.s` for `$a0`/`$a1`/`$a2` uses.
- result: `addu $s2, $a0, $zero` (line 4) and `addu $s6, $a1, $zero` (line 17) are the only entry reads of argument registers; the first appearance of `$a2` is `mflo $a2` (line 194), a write. The a2 chain in func_8005D554 is therefore the value of the `s.zero1C` struct field only, and no argument-passing route can move its birth position.

## [s10] The candidate's floor reproduces at 6/176 on HEAD main @ c09c5da8 and fake_ablate finds no FAKE-annotated construct in it, so no banked lever in this ledger was measured behind a carrier occupying the a2-site pseudo.
- mechanism: Kill re-audit required by the brief: an instance kill measured while a FAKE carrier sits on the target pseudo is not a kill. tools/fake_ablate.py enumerates FAKE annotations in the candidate and ablates them one at a time.
- probe: Re-applied memory/grind/func_8005D554/candidate.c to src/text1b.c; ran `sandbox func_8005D554 --disable all`; ran `python3 tools/fake_ablate.py --func func_8005D554 --file text1b --candidate memory/grind/func_8005D554/candidate.c`.
- result: score 6, target_insns 176, build_insns 176. fake_ablate: 'no FAKE-annotated constructs found in memory/grind/func_8005D554/candidate.c; nothing to ablate'.
- verdict: CONFIRMED

## [s10] potential_hazard cannot separate the a2 base insn from the two expand_call argument moves at the deciding clock-64 pick, because MIPS type arith matches no define_function_unit and potential_hazard returns 0 for an insn whose insn_unit is negative.
- mechanism: potential_hazard (sched.c:1327) yields a positive cost only when insn_unit(insn) >= 0 (sched.c:1335) and function_units[unit].max_blockage > 1 (sched.c:1338); for a negative unit the fallback loop at sched.c:1360 iterates zero times. mips.md:153-260 defines units only for load/store/xfer ('memory'), hilo/imul/idiv ('imuldiv') and the FP types - arith is absent. The same mechanism with the opposite sign is why the three struct stores (unit 0 'memory', maxb=3 in the BLOCKAGE trace lines) beat the higher-LUID argument moves at clocks 61-63.
- probe: BB2_SCHED_DEBUG=1 instrumented-cc1 trace of the control (tmp/grind/func_8005D554/s10/sched.log) plus reading tools/gcc-2.7.2/sched.c:1327-1366 and the define_function_unit table in tools/gcc-2.7.2/config/mips/mips.md:153-260.
- result: At clock 64 the trace prints `SELBEST clock=64 insn=242 pos=1` over ready [242(p=3,l=43) 240(p=3,l=42) 211(p=3,l=31)] - the hazard scan returns the head of the LUID-sorted priority group, i.e. all three tie at cost 0. A hazard-winning spelling of the a2 base would have to be emitted as a memory or imuldiv insn, a different opcode from the target's `addiu $a2, $s4, -0xC`.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ c09c5da8, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.
- predicate_cite: tools/gcc-2.7.2/sched.c:1338

## [s10] The last-scheduled CLASS criterion of rank_for_schedule assigns class 3 to the a2 base and to both argument moves under every possible last_scheduled_insn, because insn_cost clamps to 1 for any insn with no function unit.
- mechanism: rank_for_schedule takes the class-3 branch whenever `link == 0 || insn_cost (tmp, link, last_scheduled_insn) == 1` (sched.c:2429, twin at 2437). insn_cost (sched.c:1372-1398) computes result_ready_cost and clamps anything below 1 up to 1, which is every arith/move insn. So no choice of last_scheduled_insn - including the call insn itself - can demote the argument moves below class 3. This upgrades s7's 0/203 measured class deltas from an instance observation to a predicate.
- probe: Read tools/gcc-2.7.2/sched.c:2420-2445 and sched.c:1372-1398; cross-checked against the SCHEDDBG/RANKDBG trace in tmp/grind/func_8005D554/s10/sched.log.
- result: No comparison in the trace produces a nonzero class delta, and the source shows the class-3 branch is taken for any arith producer.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ c09c5da8, candidate.c chassis, control floor 6/176; no FAKE constructs present.
- predicate_cite: tools/gcc-2.7.2/sched.c:2429

## [s10] No source-side placement or operand order can give the a2 base insn an INSN_LUID above the call's hard-register argument moves, because expand_call emits those moves after every argument expression has been evaluated and after every preceding statement has expanded.
- mechanism: expand_call's final register-load loop calls emit_move_insn (reg, args[i].value) at calls.c:1880, under the comment 'Their expressions were already evaluated' (calls.c:1845), and emits the call insn immediately after. Every insn from expanding a source statement that must complete before the call - which includes any value stored into the argument struct - carries a strictly lower LUID, and rank_for_schedule's final term is INSN_LUID (sched.c:2462), largest wins.
- probe: Read tools/gcc-2.7.2/calls.c:1843-1900. Measured the control trace (argument moves at LUID 42 and 43, highest other window insn 41, a2 base 31), then built form v1 (tmp/grind/func_8005D554/s10/v1.c) with both halves rewritten so the two a2 statements sit at the last possible pre-call position - after the zero10/one14/ret stores, immediately before the zero1C store - and re-ran the sandbox and the SCHEDDBG trace.
- result: v1 scores 6/176. Its a2 base is uid 220 (verified in the sched2 RTL: `(set (reg/v:SI 6 a2) (plus:SI (reg/v:SI 20 s4) (const_int -12)))`), its LUID rose 31 to 34, and the argument moves stayed at exactly 42/43; the emitted rotation is unchanged. Banked at rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c. This also disposes of the s9 frontier's cse-operand-order route, since cse moves no insn across statements and so cannot beat a bound that statement placement itself cannot reach.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ c09c5da8, candidate.c chassis, control 6/176 and v1 6/176; no FAKE constructs present in either.
- predicate_cite: tools/gcc-2.7.2/calls.c:1880

## [s10] Reload inserts no instruction into either contested window of this function, so the s9 frontier's route of an a2 base materialised by reload after the argument moves has no instance on this chassis.
- mechanism: reload1.c only inserts at a use point when a pseudo fails to get a hard register; sched2 then renumbers LUIDs over the post-reload stream. If nothing is reloaded in the window, sched2 sees the sched1 order.
- probe: Ran the instrumented cc1 with -da (tmp/grind/func_8005D554/s10/sched_dump.sh) and extracted the func_8005D554 segment of text1b.greg to tmp/grind/func_8005D554/s10/greg_seg.txt.
- result: 18 pseudos to allocate, 18 register dispositions, zero 'Reloads for insn' entries, zero reload_in/reload_out entries. The only reload activity in the whole function is `Spilling reg 7` (hard $a3) for insn 133, outside both windows. A reload insertion for the a2 base pseudo would in any case be a memory reference against its stack slot - an extra instruction beyond 176 with a different opcode from `addiu a2,s4,-K`.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ c09c5da8, candidate.c chassis, control floor 6/176; no FAKE constructs present.

## [s10] func_80073728 is a two-argument function: it reads $a0 and $a1 at entry and never reads $a2 before overwriting it, so the caller's $a2 is pure scratch and m2c's three-argument reading recorded in s8 is wrong.
- mechanism: If the callee never reads $a2, no argument-passing route can move the a2 chain's birth position out of the struct-store region and into expand_call's argument evaluation.
- probe: Grepped asm/funcs/func_80073728.s for uses of $a0/$a1/$a2.
- result: `addu $s2, $a0, $zero` (line 4) and `addu $s6, $a1, $zero` (line 17) are the only entry reads of argument registers; the first appearance of $a2 is `mflo $a2` (line 194), a write.
- verdict: CONFIRMED

## s11 (rederive, 2026-09-09, HEAD main @ e4c60089) — control re-measured 6/176

### Kill re-audit (mandated: floor flat >= 3 sessions)
- `python3 tools/fake_ablate.py --func func_8005D554 --file text1b --candidate
  memory/grind/func_8005D554/rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c`
  → "no FAKE-annotated constructs found; nothing to ablate".  The closest-to-target banked form
  (s10's maximal pre-call birth point, 6/176) carries no FAKE carrier, so its inert verdict was
  not measured behind one.  Control body re-applied to src/text1b.c and re-measured: **6/176**,
  build_insns 176 == target_insns 176.  Chassis unchanged from s10.

### H32 — the unspent in-file sibling func_8005FA98 (KILLED, instance)
- statement: Transplanting the store order of the matched in-file sibling func_8005FA98
  (src/text1b.c:2710 — same S46C type, same callee func_80073728, two back-to-back calls with a
  re-used stack struct) onto the candidate chassis scores 60/180 on this chassis.
- mechanism: 8005FA98 stores c20,c24,p0,byte28,zero1C,zero18,zero10,one14,p1,ret — zero1C early
  and ret last, unlike both our chassis (zero1C last) and the s8-spent sibling func_8005D46C
  (zero1C before zero18).  Preserving the rand() sequence forces all three rand calls and both
  offset computations ahead of the ten stores; half 2's p0 store must stay before its rands
  because it READS D_800A3418.
- probe: `tmp/grind/func_8005D554/s11/vB_sibling_8005FA98_order.c`, applied to src/text1b.c,
  `sandbox func_8005D554 --disable all`.
- result: **60/180** (+4 insns).  a0_offset and a2_offset are simultaneously live across a rand
  call, so the allocator needs two extra callee-saved seats and the frame/prologue grows.  The
  third and last structural sibling in this file is now SPENT (8005D46C at s8, 8005FA98 here).
- verdict: KILLED (instance) — banked at
  rejected/sibling-8005FA98-store-order-all-rands-first-scores-60.c

### H33 — frontier item 1: pointer local used ONLY at the call argument (KILLED, instance)
- statement: Declaring `S46C *ps;`, assigning `ps = &s;` in the loop preheader and passing
  `func_80073728((s32)ps, 0)` while leaving every field store spelled `s.field` scores 21/179.
- mechanism: s10's frontier hoped that giving the first argument a pseudo producer would turn
  expand_call's hard-register move into a coalescable pseudo-to-hard-reg copy (calls.c:1880
  always emits the move last, but WHAT it copies is source-determined), so that the insn holding
  LUID 42 would vanish before sched2 while still having occupied that LUID during sched1.
- probe: `tmp/grind/func_8005D554/s11/vC_ps_call_site_only.c` (distinct from the banked
  struct-via-pointer-local-scores-60.c, which routed EVERY field access through ps).
- result: **21/179** (+3 insns).  ps is live across both calls, so it takes a callee-saved seat;
  the preheader gains `addiu ps,sp,0x10`; and because ps is still live at each call the copy
  `a0 = ps` is NOT coalesced away — both `move a0,ps` insns survive in addition to the hoisted
  addiu.  The coalescing premise of the frontier item does not hold for a value that is live at
  more than one call site.
- verdict: KILLED (instance) — banked at
  rejected/ps-pointer-local-call-argument-only-scores-21.c

### H34 — frontier item 3: splitting the loop body into more basic blocks (KILLED, instance)
- statement: Respelling half 2's `s.p0 = (u8 *)base_offset + (D_800A3418 & 1) * 0xC` as a real
  if/else splits the loop body into four basic blocks at a cost of ONE instruction (53/177) and
  leaves the residual rotation byte-identical in BOTH halves.
- mechanism: schedule_block operates per basic block (schedule_insns' block loop, sched.c:4937),
  so both the LUID numbering and the ready-list contents are block-local; s10's frontier hoped a
  different block shape would change the clock-64 window.
- probe: `tmp/grind/func_8005D554/s11/vF_half2_block_split.c`; emitted code disassembled to
  `tmp/grind/func_8005D554/s11/fn.txt`.
- result: **53/177**.  fn.txt:90 and fn.txt:136 show both windows as
  `addiu a2,s4,-K` / `addiu a0,sp,0x10` / `lw v1,0(gp)` / `move a1,zero` — the control's
  rotation exactly.  The if/else JOIN dominates the entire a2 chain and the call, so the call's
  block still contains all four window insns in the same relative order.  Note the useful
  by-product: a real two-armed branch here costs only +1 insn, so block shape is cheap — it is
  simply not a lever on this window.  Structural consequence (not itself a measurement): the a2
  base is a program-order predecessor of the call, so any block boundary placed between them
  puts the base in a PREDECESSOR block, and cross-block emission order follows program order —
  the base would be emitted BEFORE the argument setup, which is the side the control already has.
- verdict: KILLED (instance) — banked at
  rejected/half2-p0-branch-block-split-scores-53.c

### H35 — frontier item 2: sched2 as an escape from the sched1 LUID bound (analysis, not a probe)
- statement: For the four-insn window whose INSN_PRIORITY, last-scheduled class and
  potential_hazard are all measured tied (s7/s9/s10), rank_for_schedule's final tie-break
  `return INSN_LUID (tmp) - INSN_LUID (tmp2);` (sched.c:2464) is a STABLE-SORT tie-break, and
  sched2 recomputes INSN_LUID from the current insn chain (sched.c:2198).  So for a tied group
  sched2's output order equals its input order, and the only sched1 emission order whose sched2
  output is the target order 240,242,205,211 is that order itself.
- status: NOT measured this session — recorded as the reasoning that de-prioritises the s10
  frontier item 2 sched_solver inversion, not as a kill.  s9 measured sched2 as a no-op on the
  control, which is the one data point consistent with it.  A future session that wants to spend
  the inversion should first check whether post-reload physical-register anti-deps break the
  priority tie in sched2 (they are the one term s7/s9/s10 measured only PRE-reload).

## [s11] Transplanting the store order of the previously unnamed in-file sibling func_8005FA98 (src/text1b.c:2710 - matched, same S46C, same callee func_80073728, same re-used stack struct) onto the candidate chassis scores 60/180.
- mechanism: func_8005FA98 stores c20,c24,p0,byte28,zero1C,zero18,zero10,one14,p1,ret - zero1C early and ret last, unlike both our chassis (zero1C last) and the s8-spent sibling func_8005D46C (zero1C before zero18). Preserving the rand() sequence forces all three rand calls and both offset computations ahead of the ten stores, so a0_offset and a2_offset are live across a rand call simultaneously and the allocator needs two extra callee-saved seats.
- probe: tmp/grind/func_8005D554/s11/vB_sibling_8005FA98_order.c applied to src/text1b.c, sandbox func_8005D554 --disable all.
- result: 60/180 (+4 instructions vs the 6/176 control). All three in-file S46C siblings are now spent: func_8005D46C (s8, 28/178), func_8005FA98 (s11, 60/180), main in src/ings.c (s10). Banked at rejected/sibling-8005FA98-store-order-all-rands-first-scores-60.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ e4c60089, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s11] A pointer local S46C *ps = &s; hoisted to the loop preheader and used ONLY as the first call argument, with every field store left spelled s.field, scores 21/179.
- mechanism: s10's frontier item 1: calls.c:1880 always emits the hard-register argument move last, but WHAT it copies is source-determined, so a pseudo producer for the first argument was expected to make a0 = pseudo a coalescable copy that vanishes before sched2 while still having occupied LUID 42 during sched1.
- probe: tmp/grind/func_8005D554/s11/vC_ps_call_site_only.c - distinct from the banked struct-via-pointer-local-scores-60.c, which routed every field access through ps.
- result: 21/179 (+3 instructions). ps is live at BOTH call sites, so it takes a callee-saved seat, the preheader gains addiu ps,sp,0x10, and neither move a0,ps is coalesced away. The coalescing premise does not hold for a value live at more than one call. Banked at rejected/ps-pointer-local-call-argument-only-scores-21.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ e4c60089, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s11] Respelling half 2's s.p0 = (u8 *)base_offset + (D_800A3418 & 1) * 0xC as a real if/else splits the loop body into four basic blocks for one extra instruction (53/177) and leaves the emitted window rotation byte-identical in both halves.
- mechanism: s10's frontier item 3: schedule_block operates per basic block (schedule_insns' block loop, sched.c:4937), so LUID numbering and ready-list contents are block-local and a different block shape was expected to change the clock-64 window.
- probe: tmp/grind/func_8005D554/s11/vF_half2_block_split.c; emitted code disassembled to tmp/grind/func_8005D554/s11/fn.txt.
- result: 53/177. fn.txt:90 and fn.txt:136 both show addiu a2,s4,-K / addiu a0,sp,0x10 / lw v1,0(gp) / move a1,zero - the control's rotation exactly. The if/else join dominates the whole a2 chain and the call, so the call's block still holds all four window insns in the same relative order. Useful by-product: a two-armed branch costs only +1 insn here, so block shape is cheap on this chassis. Banked at rejected/half2-p0-branch-block-split-scores-53.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ e4c60089, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## s12 (rederive, 2026-09-09, HEAD main @ 0c19707c)

**H35 (KILLED, instance).** The kill re-audit: the closest banked form
(`rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c`) still measures 6/176 on the
current chassis and carries no FAKE construct (`tools/fake_ablate.py`), so nothing in this
ledger's kill bank was measured behind a carrier.  Fourth consecutive passing re-audit.

**H36 (KILLED, instance).** Passing the a2 base as an extra call argument through a
function-local K&R declaration does nothing, because src/text1b.c:2642 already declares
`extern s32 func_80073728(s32, s32);` at file scope and GCC 2.7.2 discards the surplus argument.
Measured: a loop-VARIANT third argument produced output byte-identical to the control
(6/176, 176 insns, no insn emitted for it).  This retires s8's stated MECHANISM for the same
observation; the observation itself was a front-end drop, not an expand_call precompute.

**H37 (KILLED, instance).** A genuine third argument, reached by bypassing the prototype with a
local function-pointer cast, does not move the four-insn window.  Measured 17/178: the argument
value is materialised as `addiu a2,a3,-12` after both argument moves, but only because it
consumes the accumulate; the window itself emits
`addiu a3,s4,-12` / `addiu a0,sp,16` / `lw v1,0(gp)` / `move a1,zero`, the control's rotation with
the base reseated from $a2 to $a3.  Argument position is not a lever, because the zero1C value is
consumed before the call.

**H38 (KILLED, instance).** Giving the a2 base no consumer other than a call argument does not
birth it late; it deletes it.  `(s32)r4 - K` is loop-invariant, and single-set/single-use it
becomes a loop.c movable and is hoisted to the preheader: measured 66/162, fourteen instructions
below the target.  The accumulate is load-bearing for keeping the base insn in the loop body.

**H39 (KILLED, class).** Raising `INSN_PRIORITY` of the a2 base above the two argument moves
cannot produce the target window, whatever the instruction count.  `schedule_select` walks the
ready list in maximal equal-priority groups (sched.c:2674) and only advances to the next group
when the current one is fully queued (sched.c:2704); the three struct stores `sw zero,0x20`,
`sw s6,0x24`, `sw s1,0x1C` sit at priority 3 and cannot be lifted (they set no register, so
`adjust_priority`'s `birthing_insn_p` boost does not apply to them, and their only successor is
the call at cost 1).  So a strictly-higher-priority base is selected before the stores' group;
selection order is reverse emission order, so it is EMITTED after those three stores, whereas the
target emits it before them (4DEC0 vs 4DEC4/4DEC8/4DECC).  This closes s11's frontier item 3 (the
"lengthen the a2 base's downstream chain so its priority reads 4" probe) analytically, and it
explains rather than merely records s8's six-slot overshoot at 32/178.  Predicate: sched.c:2674.

**Where the residual stands after s12.** All four terms of the deciding clock-64 pick are closed:
priority (H39, sched.c:2674), class (sched.c:2429, arith always class 3), potential_hazard
(sched.c:1359, `insn_unit == -1` returns 0), and INSN_LUID (calls.c:1881 bound, s10).  The only
configuration that reproduces the target is `priority(stores) >= priority(a2 base) >
priority(moves)` with the stores held at 3 -- i.e. the base must reach priority 4 while the stores
stay at 3 AND still be emitted before them, which sched.c:2674 forbids -- or `LUID(a2 base) >
LUID(a1 move)`, which calls.c:1881 forbids for any value consumed before the call.  The next
sessions' search space is therefore: (i) a chassis in which the s.zero1C value is NOT consumed
before the call (so its base can be born inside expand_call), (ii) a chassis in which the three
struct stores are not priority-3 members of the same block, or (iii) a post-reload (sched2)
difference, which remains the one term never measured on the physical-register stream.

## [s12] The closest banked form (a2 statements at the maximal pre-call birth point) still measures 6/176 on the current chassis and carries no FAKE-annotated construct, so no banked lever in this ledger was measured behind a carrier.
- mechanism: Mandated kill re-audit: a lever measured inert while a FAKE carrier occupies its target pseudo is not a kill, so the closest score-6 form is re-run on the current chassis with tools/fake_ablate.py first.
- probe: tools/fake_ablate.py --func func_8005D554 --file text1b --candidate memory/grind/func_8005D554/rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c, then apply that form to src/text1b.c and run sandbox func_8005D554 --disable all.
- result: fake_ablate reported no FAKE-annotated constructs found, nothing to ablate. Sandbox: score 6, build_insns 176, target_insns 176 - identical to the banked score. Fourth consecutive passing re-audit.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 0c19707c, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s12] Adding a third argument to the func_80073728 call through a function-local K&R declaration emits no instruction for it, because src/text1b.c:2642 declares extern s32 func_80073728(s32, s32) at file scope and GCC 2.7.2 discards the surplus argument.
- mechanism: A function-local K&R extern does not override an in-scope file-scope prototype; with a 2-parameter prototype visible the front end drops the third actual argument before expand_call ever runs, so no RTL is produced for it.
- probe: Form p2 (tmp/grind/func_8005D554/s12/p2_variant_third_arg.c): control body plus a genuinely loop-VARIANT third argument a2_offset - 0xC and a2_offset - 0x19; measured with sandbox and disassembled with mipsel-linux-gnu-objdump.
- result: Score 6, build_insns 176 - byte-identical to the control - and the disassembled body contains no insn for the third argument. This retires the MECHANISM s8 recorded for the same observation (s8 attributed it to expand_call precomputing an already-computed value); the observation was a front-end argument drop, so it is not evidence about argument-position birth.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 0c19707c, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s12] A genuine third call argument, reached by bypassing the file-scope prototype with a local function-pointer cast, does not change the four-insn window rotation.
- mechanism: expand_call evaluates argument expressions before emitting the hard-register moves at calls.c:1881, and the zero1C value is consumed (accumulated, then stored) before the call, so its base insn is expanded before the moves however the call is spelled.
- probe: Form p3 (tmp/grind/func_8005D554/s12/p3_fnptr_third_arg.c): a local function pointer cast to a 3-parameter signature, called as f3((s32)&s, 0, a2_offset - 0xC) in both halves; measured and disassembled (tmp/grind/func_8005D554/s12/fn.txt).
- result: Score 17, build_insns 178. The third argument IS materialised, as addiu a2,a3,-12 at 0x35cc - after addiu a0,sp,16 at 0x3598 and move a1,zero at 0x35a0 - but only by dependence on the accumulate addu a3,a3,v0, not by rank. The window itself emits addiu a3,s4,-12 / addiu a0,sp,16 / lw v1,0(gp) / move a1,zero: the control rotation with the base merely reseated from $a2 to $a3.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 0c19707c, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s12] Removing the accumulate so that the a2 base has no consumer other than a call argument does not birth the base late - it removes the insn from the loop entirely.
- mechanism: (s32)r4 - K is loop-invariant. Once it is single-set and single-use it becomes a loop.c movable and is hoisted to the loop preheader, so the window addiu disappears and the body loses instructions.
- probe: Form p1 (tmp/grind/func_8005D554/s12/p1_base_as_third_arg.c): s.zero1C stores the scaled rand value alone and (s32)r4 - 0xC / (s32)r4 - 0x19 is passed as the call's extra argument; measured and disassembled.
- result: Score 66, build_insns 162 - fourteen instructions BELOW the 176-insn target - and no addiu $a2,$s4,-K appears anywhere in the loop body. The accumulate is load-bearing for keeping the base insn inside the loop at all.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 0c19707c, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s12] Giving the a2 base a strictly higher INSN_PRIORITY than the two argument moves places it on the wrong side of the three struct stores in the emitted stream, at any instruction count.
- mechanism: schedule_select walks the ready list in maximal equal-priority groups (sched.c:2674) and advances to the next group only when the current group is fully queued (sched.c:2704). The three struct stores sw zero,0x20 / sw s6,0x24 / sw s1,0x1C sit at priority 3 and cannot be lifted: they set no register, so the birthing_insn_p boost in adjust_priority does not reach them, and their only successor is the call at cost 1. A base at priority 4 is therefore selected before the stores group, and because schedule_block selects backwards (selected first equals emitted last) it is EMITTED after them - while the target emits addiu $a2,$s4,-0xC at 4DEC0, before all three stores at 4DEC4/4DEC8/4DECC.
- probe: Read schedule_select and rank_for_schedule end to end in tools/gcc-2.7.2/sched.c (2400-2726) and potential_hazard (1325-1364); cross-checked against the already-banked instance rejected/both-bases-in-local-array-load-consumer-scores-32.c, the s8 load-producer form that reaches priority 4 via a load consumer and measures 32/178 with a six-slot overshoot in exactly the predicted direction.
- result: Closes the s11 frontier item that proposed lengthening the a2 base downstream chain so its priority reads 4 at 176 insns: the instruction count is irrelevant, the group ordering alone puts the base after the stores. Together with the class term (sched.c:2429, arith is class 3 under every last_scheduled_insn), the hazard term (sched.c:1359, insn_unit -1 makes potential_hazard 0 for any arith spelling) and the LUID bound (calls.c:1881), all four terms of the deciding clock-64 pick are now closed with predicates.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 0c19707c, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.
- predicate_cite: sched.c:2674

## s13 (structural, 2026-09-09, HEAD main @ 5daf178d)

**Control re-measured 6/176** on candidate.c before any probe.

**H40 (CONFIRMED).** The whole 6-point residual is the `birthing_insn_p` LAUNCH-priority boost,
not INSN_PRIORITY depth, not INSN_LUID, not class, not hazard.
- mechanism: `adjust_priority` (tools/gcc-2.7.2/sched.c:2584) raises an insn's INSN_PRIORITY to
  `max_priority` when it has no REG_DEAD notes and `birthing_insn_p` (sched.c:2505) holds --
  i.e. the pattern is a SET whose dest is a REG that is live in `bb_live_regs` and whose
  `reg_n_sets[REGNO] == 1`.  Also newly established from the PRIODBG capture: GCC 2.7.2's
  `priority()` (sched.c:1434-1520) walks LOG_LINKS, i.e. PREDECESSORS, so INSN_PRIORITY is
  DEPTH FROM THE BLOCK START, not distance to the block end.  That is why half 1's window ties
  at 3 and half 2's at 6 -- both halves are in the SAME basic block (block 6, 92 insns).
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) run with BB2_SCHED_DEBUG=1 AND BB2_PRIO_DEBUG=1
  over the control body and over rejected/judge-failed-fresh-multiwrite-nv-nw-carrier-scores-0.c;
  block-6 traces extracted to tmp/grind/func_8005D554/s13/blk6.log and .../nv_blk6.log.
- result: CONTROL -- a2 base = insn 211, luid 31, pri 3, `ADJPRI ... birth=0`; at clock 64 the
  ready list is [242(p=3,l=43) 240(p=3,l=42) 211(p=3,l=31)], rank_for_schedule falls through
  priority and class to INSN_LUID (sched.c:2464) and picks 242; 211 is picked last at clock 67,
  so in the backward scheduler it is EMITTED FIRST -- the observed rotation.
  SCORE-0 BODY -- a2 base = insn 198, luid 25 (born before the s.zero18 store and before the
  third rand call), pri 1, `ADJPRI insn=198 deaths=0 birth=1 maxpri=2130706433`, boosted and
  picked at clock 61.  Same file, same block, same registers; the only difference is the boost.
- verdict: CONFIRMED

**H41 (KILLED, instance).** Borrowing the existing pre-loop locals v0 and v3 as the per-half
early carriers, at the EXACT positions and with the EXACT `carrier = ret; s.ret = carrier;`
second write of the score-0 body, measures 63/178 -- the same score the s3-era `v0 = 0` variant
reached, so the second write's VALUE is not the lever.
- mechanism: v0 and v3 are set and read before the loop, so a second live range inside the loop
  gives each pseudo one interference-spanning range; local/global alloc seats them in s0 and s1
  (confirmed in the disassembly, tmp/grind/func_8005D554/s13/p1.txt: frame 120 and ten register
  saves, same as the target, so the +2 is inside the loop, not a tenth callee-saved seat).
- probe: tmp/grind/func_8005D554/s13/p1_v0v3_ret_restage.c, sandbox --disable all, then objdump.
- result: 63/178.  Banked at rejected/v0v3-borrow-at-score0-carrier-positions-scores-63.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 5daf178d, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

**H42 (KILLED, instance).** Letting `a2_offset` itself carry the base across the third rand call
(the base statement moved above `s.zero18 = a0_offset;`, no extra carrier) measures 31/178.
- mechanism: a2_offset's live range then spans the third `rand()` call, so the allocator must
  keep the value in a call-saved seat and two extra insns appear inside the loop.  a2_offset also
  stays multi-set, so `birthing_insn_p` still fails and the base insn still gets no boost.
- probe: tmp/grind/func_8005D554/s13/p3_a2early_only.c
- result: 31/178.  Banked at rejected/a2offset-carries-base-across-third-rand-scores-31.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 5daf178d, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

**H43 (KILLED, instance).** The a0_offset/a2_offset ROLE SWAP -- the a0 value computed into
a2_offset and the a2 base staged early into a0_offset, i.e. the score-0 shape spelled entirely
with EXISTING loop-local variables -- measures 30/178 both with and without the
`a0_offset = ret; s.ret = a0_offset;` second write.
- mechanism: a0_offset is multi-set, so no boost; and its borrowed range now spans the third rand
  call, which costs the same +2 as H42.  The existing-local borrow quadrant therefore cannot buy
  the early-birth position at 176 instructions on this chassis.
- probe: tmp/grind/func_8005D554/s13/p5_role_swap_existing_locals.c and p6_role_swap_no_ret_restage.c
- result: 30/178 and 30/178.  Banked at rejected/role-swap-a0offset-carries-base-scores-30.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 5daf178d, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

**H44 (KILLED, instance).** The `carrier = ret; s.ret = carrier;` restage on its own, at the
control's statement positions and through the existing local a0_offset (dead after its
`s.zero18 = a0_offset;` store), is BYTE-INERT: 6/176, identical to the control.
- mechanism: combine folds the copy pair back to `s.ret = ret` and deletes the extra set, so
  neither the insn stream nor the dependence graph changes.  This isolates the score-0 body's
  lever to the EARLY BASE plus the resulting reg_n_sets == 1, not to the restage statement.
- probe: tmp/grind/func_8005D554/s13/p4_a0borrow_ret_only.c
- result: 6/176.  Banked at rejected/a0offset-ret-restage-byte-inert-scores-6.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 5daf178d, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

**H45 (KILLED, class).** No carrier that is single-set in the SOURCE can supply the boost on this
chassis, because the same predicate that enables the boost makes the loop-invariant base a LICM
movable.
- mechanism: `birthing_insn_p` requires `reg_n_sets[dest] == 1` (sched.c:2505).  loop.c's movable
  acceptance requires `n_times_set[REGNO (SET_DEST (set))] == 1` for an invariant source
  (loop.c:705), and the guard immediately above it (loop.c:695-700) only REJECTS a candidate when
  all three of (i) `!maybe_never && !loop_reg_used_before_p`, (ii) dest is neither a user variable
  nor a loop-test reg, and (iii) `reg_in_basic_block_p` are false.  This loop body is a single
  basic block (block 6) with no backward jump, so (i) is true for any carrier that is not read
  before its set -- the invariant `(s32)r4 - K` is therefore always a movable and is hoisted to
  the preheader (measured repeatedly at 54/...).  Hence: single-set in source => hoisted;
  multi-set in source => no boost.  The only measured escape is a source-level second set that
  COMBINE deletes, which is precisely the fresh multi-write carrier the Judge FAILed on
  2026-09-08.
- probe: read loop.c:660-760 and sched.c:2490-2600 end to end against the two PRIODBG/SCHEDDBG
  traces captured this session; cross-checked against the already-banked
  rejected/fresh-single-set-perhalf-base-licm-hoisted-scores-54.c and
  rejected/fresh-singleset-a2-base-adds-2-insns-scores-54.c.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 5daf178d, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.
- predicate_cite: loop.c:705

## [s13] The whole 6-point residual is the birthing_insn_p LAUNCH-priority boost applied by adjust_priority, not INSN_PRIORITY depth, not INSN_LUID, not the class or hazard terms.
- mechanism: adjust_priority (tools/gcc-2.7.2/sched.c:2584) raises an insn's INSN_PRIORITY to max_priority when it carries no REG_DEAD notes and birthing_insn_p (sched.c:2505) holds - a SET whose REG dest is live in bb_live_regs and whose reg_n_sets[REGNO] == 1. The same capture also establishes that GCC 2.7.2's priority() walks LOG_LINKS, i.e. PREDECESSORS, so INSN_PRIORITY is depth from the BLOCK START; both loop halves sit in one basic block (block 6, 92 insns), which is why half 1 ties at 3 and half 2 at 6.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) run with BB2_SCHED_DEBUG=1 and, for the first time, BB2_PRIO_DEBUG=1, over both the control body and the banked score-0 body; block-6 traces extracted to tmp/grind/func_8005D554/s13/blk6.log and nv_blk6.log.
- result: CONTROL: a2 base = insn 211, luid 31, pri 3, ADJPRI birth=0; at clock 64 the ready list is [242(p=3,l=43) 240(p=3,l=42) 211(p=3,l=31)], priority and class tie, INSN_LUID (sched.c:2464) takes 242, and 211 is picked last at clock 67 - emitted first in the window, which is exactly our rotation. SCORE-0 BODY: a2 base = insn 198, luid 25 (born before the s.zero18 store and before the third rand call), pri 1, ADJPRI insn=198 deaths=0 birth=1 maxpri=2130706433, picked at clock 61. Same block, same registers; the only difference is the boost.
- verdict: CONFIRMED

## [s13] Borrowing the existing pre-loop locals v0 and v3 as the per-half early carriers, at the exact positions and with the exact 'carrier = ret; s.ret = carrier;' second write of the score-0 body, measures 63/178.
- mechanism: v0 and v3 are set and read before the loop, so a second live range inside the loop gives each pseudo one interference-spanning range; the allocator seats them in s0 and s1 and two extra instructions appear inside the loop.
- probe: tmp/grind/func_8005D554/s13/p1_v0v3_ret_restage.c, sandbox --disable all, then mipsel-linux-gnu-objdump (tmp/grind/func_8005D554/s13/p1.txt).
- result: 63/178 - the same score the s3-era 'v0 = 0' variant reached, so the second write's VALUE is not the lever. The disassembly shows frame 120 and the same ten register saves as the target, so the +2 is loop-body cost, not a tenth callee-saved seat. Banked at rejected/v0v3-borrow-at-score0-carrier-positions-scores-63.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 5daf178d, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s13] Letting a2_offset itself carry the a2 base across the third rand call, with the base statement moved above the s.zero18 store and no extra carrier, measures 31/178.
- mechanism: a2_offset's live range then spans the third rand() call, so the value must occupy a call-saved seat and two extra instructions appear in the loop; a2_offset also stays multi-set, so birthing_insn_p still fails and the base insn still gets no boost.
- probe: tmp/grind/func_8005D554/s13/p3_a2early_only.c (and p2_a2early_a0borrow_ret.c, which adds the a0_offset ret restage on top).
- result: 31/178 for both, so the ret restage is byte-neutral on top of the early base. Banked at rejected/a2offset-carries-base-across-third-rand-scores-31.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 5daf178d, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s13] The a0_offset/a2_offset role swap - the a0 value computed into a2_offset and the a2 base staged early into a0_offset, i.e. the score-0 shape spelled entirely with existing loop-local variables - measures 30/178 with and without the ret restage.
- mechanism: a0_offset is multi-set so no boost fires, and its borrowed range now spans the third rand call, costing the same +2 instructions as the a2_offset variant.
- probe: tmp/grind/func_8005D554/s13/p5_role_swap_existing_locals.c and p6_role_swap_no_ret_restage.c
- result: 30/178 and 30/178. Banked at rejected/role-swap-a0offset-carries-base-scores-30.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 5daf178d, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s13] The 'carrier = ret; s.ret = carrier;' restage on its own, at the control's statement positions and through the existing local a0_offset, is byte-inert at 6/176.
- mechanism: combine folds the copy pair back to 's.ret = ret' and deletes the extra set, so neither the insn stream nor the dependence graph changes. This isolates the score-0 body's lever to the early base and the reg_n_sets == 1 it leaves behind, not to the restage statement.
- probe: tmp/grind/func_8005D554/s13/p4_a0borrow_ret_only.c
- result: 6/176, identical to the control. Banked at rejected/a0offset-ret-restage-byte-inert-scores-6.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 5daf178d, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s13] A carrier that is single-set in the SOURCE cannot supply the boost on this chassis, because the predicate that enables the boost is the same one that makes the loop-invariant base a LICM movable.
- mechanism: birthing_insn_p requires reg_n_sets[dest] == 1 (sched.c:2505). loop.c's movable acceptance requires n_times_set[REGNO (SET_DEST (set))] == 1 for an invariant source (loop.c:705), and the guard immediately above it (loop.c:695-700) rejects a candidate only when all three of (i) !maybe_never && !loop_reg_used_before_p, (ii) dest is neither a user variable nor a loop-test reg, and (iii) reg_in_basic_block_p are false. This loop body is one basic block with no backward jump and no carrier read before its set, so (i) is true for every natural carrier and the invariant (s32)r4 - K is always hoisted to the preheader.
- probe: Read loop.c:660-760 and sched.c:2490-2600 end to end against the two PRIODBG/SCHEDDBG traces captured this session; cross-checked against the already-banked rejected/fresh-single-set-perhalf-base-licm-hoisted-scores-54.c and rejected/fresh-singleset-a2-base-adds-2-insns-scores-54.c, both 54.
- result: Single-set in source implies hoisted; multi-set in source implies no boost. The only measured escape is a source-level second set that combine deletes ('nv = ret; s.ret = nv;' folding to 's.ret = ret'), which is exactly the fresh multi-write carrier the Judge FAILed on 2026-09-08.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 5daf178d, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.
- predicate_cite: loop.c:705

## [s14] loop.c's movable guard (loop.c:695-700) is a three-way OR, and disjunct C (reg_in_basic_block_p, loop.c:1062) alone keeps the invariant a movable, so falsifying only C does not stop the hoist.
- mechanism: scan_loop rejects a candidate movable only when ALL of A = (!maybe_never && !loop_reg_used_before_p), B = (!REG_USERVAR_P && !REG_LOOP_TEST_P) and C = reg_in_basic_block_p are false. Every C-level local is a user variable, so B is false for free; A is true whenever no jump or label precedes the base's set inside the loop body.
- probe: base1 = (s32)r4 - 0xC set in half 1 and read again in half 2 as base2 = base1 - 0xD, with half 2's p0 selection respelled as a real if/else so a CODE_LABEL sits between base1's set and its last use. tmp/grind/func_8005D554/s14/fB_base1_reused_across_join.c, disassembled to tmp/grind/func_8005D554/s14/fB.dis.
- result: 57/179, and the disassembly shows addiu a2,a0,-12 at 0x352C and addiu a0,a0,-25 at 0x3534, both BEFORE the loop top at 0x353C -- still hoisted. Banked at rejected/base1-reused-across-ifelse-join-still-hoisted-scores-57.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 46867ae6, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s14] The plain non-guard-duplicated while chassis does not set maybe_never for the loop body, because GCC rotates the exit test out of the loop.
- mechanism: scan_loop sets maybe_never only when its forward scan passes a CODE_LABEL or JUMP_INSN inside the loop (loop.c:919-930). A source-level while loop is rotated by expand_end_loop into a guarded do-while, so the only conditional jump is the guard (outside the loop) and the bottom test (which follows every base statement).
- probe: fresh single-set per-half base carriers under a plain while chassis. tmp/grind/func_8005D554/s14/fF_while_chassis_freshA.c, disassembled to tmp/grind/func_8005D554/s14/fF.dis.
- result: 64/176. The guard sits at 0x351C and the loop top at 0x3534, with addiu a2,v1,-12 at 0x3528 and addiu s8,v1,-25 at 0x3530 -- both hoisted, and the first is additionally SPILLED (sw a2,64(sp)) and reloaded inside the loop (lw a2,64(sp)), which is why the instruction count lands on 176 without being any closer to the target. Banked at rejected/plain-while-chassis-rotated-no-maybe-never-scores-64.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 46867ae6, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s14] Combining the while chassis with the half-2 join (both of s13's suggested maybe_never and reg_in_basic_block_p levers at once) still hoists both bases.
- mechanism: the rotation of the while chassis means the only in-body jump is still the half-2 if/else, which sits AFTER half 1's base statement; half 1's base therefore keeps disjunct A true and stays a movable, and half 2's base keeps disjunct C true because its set and its only use share a basic block.
- probe: tmp/grind/func_8005D554/s14/fE_while_chassis_join.c, disassembled to tmp/grind/func_8005D554/s14/fE.dis.
- result: 70/177; addiu a2,v1,-12 at 0x3524 and addiu v1,v1,-25 at 0x352C, loop top at 0x3534 -- both in the preheader. Banked at rejected/while-chassis-plus-join-still-hoisted-scores-70.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 46867ae6, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s14] Making the base's SOURCE non-invariant with a no-op set of r4 does not reach loop.c at all: cse runs before loop and deletes the move.
- mechanism: invariant_p (loop.c:2756) returns true for (plus (reg r4) (const)) exactly when n_times_set[r4] == 0 inside the loop, so an in-loop set of r4 would make the base insn ineligible as a movable while leaving the base carrier single-set (reg_n_sets == 1) for birthing_insn_p. But cse_main runs before loop_optimize in the pass order, and it deletes both a literal self-assign and a set whose source folds back to the destination.
- probe: two forms -- r4 = r4; at the top of the loop body, and r4 = (u32)(base1 + 0xC); immediately after half 1's accumulate (which folds to r4 = r4 once cse propagates base1 = r4 - 0xC). tmp/grind/func_8005D554/s14/fC2_r4_selfassign.c and fC_r4_reestablished.c.
- result: 54/178 and 54/178 -- byte-identical to the plain fresh-single-set form fA (54/178), i.e. the sets left no trace whatsoever. Banked at rejected/r4-self-assign-cse-deletes-scores-54.c and rejected/r4-reestablished-from-base1-cse-deletes-scores-54.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 46867ae6, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s14] CORRECTION to s12: the target does not rank the a2 base above the three struct stores, so the priority family is not self-defeating by overshoot.
- mechanism: GCC 2.7.2's schedule_block is a backward list scheduler (sched.c:4036-4038), so emission order is the reverse of selection order. The target's window (asm/funcs/func_8005D554.s, 4DEB4-4DECC) is addiu a0,sp,0x10 / addu a1,zero,zero / lw v1,gp / addiu a2,s4,-0xC / sw zero / sw s6 / sw s1. Reversing it gives the selection order sw, sw, sw, a2 base, lw, a1, a0 -- the three stores at clocks 61-63 and the a2 base at clock 64, exactly the slot where our control loses the INSN_LUID tie.
- probe: read the target asm window directly.
- result: The banked score-0 body boosts the base to max_priority and is picked at clock 61 (ahead of the stores) and still scores 0, so a later pass completes the rotation. s12's conclusion that any priority lift overshoots by construction is therefore not a reason to abandon the priority family; what the target's sched1 needs is only that the base win the clock-64 three-way tie.
- verdict: CONFIRMED

## [s14] Falsifying only loop.c's reg_in_basic_block_p disjunct - base1 set in half 1 and read again in half 2 as base2 = base1 - 0xD, with half 2's p0 selection respelled as a real if/else so a CODE_LABEL sits between base1's set and its last use - leaves both bases hoisted to the preheader and measures 57/179.
- mechanism: scan_loop (loop.c:695-700) rejects a candidate movable only when ALL THREE of A = (!maybe_never && !loop_reg_used_before_p), B = (!REG_USERVAR_P && !REG_LOOP_TEST_P) and C = reg_in_basic_block_p (loop.c:1062) are false. Every C-level local is a user variable, so B is false for free, but A stays true whenever no jump or label precedes the base's set inside the loop body - and the half-2 join sits after half 1's base statement.
- probe: tmp/grind/func_8005D554/s14/fB_base1_reused_across_join.c, sandbox --disable all, then mipsel-linux-gnu-objdump to tmp/grind/func_8005D554/s14/fB.dis.
- result: 57/179. The disassembly shows addiu a2,a0,-12 at 0x352C and addiu a0,a0,-25 at 0x3534, both before the loop top at 0x353C, so LICM still hoisted them. Banked at rejected/base1-reused-across-ifelse-join-still-hoisted-scores-57.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 46867ae6, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s14] The plain non-guard-duplicated while chassis does not buy maybe_never for the loop body: with fresh single-set per-half base carriers it measures 64/176 with both bases in the preheader and the first one additionally spilled to the frame and reloaded inside the loop.
- mechanism: scan_loop sets maybe_never only when its forward scan passes a CODE_LABEL or JUMP_INSN inside the loop (loop.c:919-930). A source-level while loop is rotated by expand_end_loop into a guarded do-while, so the only conditional jump before the body is the guard, which sits OUTSIDE the loop; the disassembly places the guard blez at 0x351C and the loop top at 0x3534.
- probe: tmp/grind/func_8005D554/s14/fF_while_chassis_freshA.c, sandbox --disable all, then objdump to tmp/grind/func_8005D554/s14/fF.dis.
- result: 64/176. addiu a2,v1,-12 at 0x3528 and addiu s8,v1,-25 at 0x3530 are both hoisted; sw a2,64(sp) in the preheader and lw a2,64(sp) inside the loop show the in-loop instruction is a reload, not the addiu, so the 176 count is spill-plus-reload arithmetic rather than proximity. Banked at rejected/plain-while-chassis-rotated-no-maybe-never-scores-64.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 46867ae6, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s14] Combining the while chassis with the half-2 join - s13's two suggested levers applied together - still hoists both bases and measures 70/177.
- mechanism: the while rotation leaves the half-2 if/else as the only in-body jump, so half 1's base keeps disjunct A true, and half 2's base keeps disjunct C true because its set and its only use share a basic block.
- probe: tmp/grind/func_8005D554/s14/fE_while_chassis_join.c, sandbox --disable all, then objdump to tmp/grind/func_8005D554/s14/fE.dis.
- result: 70/177; addiu a2,v1,-12 at 0x3524 and addiu v1,v1,-25 at 0x352C sit before the loop top at 0x3534. Banked at rejected/while-chassis-plus-join-still-hoisted-scores-70.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 46867ae6, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s14] Making the base's source non-invariant with a no-op set of r4 never reaches loop.c: both r4 = r4 and r4 = (u32)(base1 + 0xC) measure 54/178, byte-identical to the plain fresh-single-set form, because cse runs before loop_optimize and deletes the move.
- mechanism: invariant_p (loop.c:2756) treats (plus (reg r4) (const)) as invariant exactly when n_times_set[r4] == 0 inside the loop, so an in-loop set of r4 would disqualify the base insn as a movable while leaving the base carrier single-set for birthing_insn_p (sched.c:2505). But cse_main precedes loop_optimize in the pass order and deletes both a literal self-assign and a set whose source folds back to the destination.
- probe: tmp/grind/func_8005D554/s14/fC2_r4_selfassign.c and tmp/grind/func_8005D554/s14/fC_r4_reestablished.c, sandbox --disable all.
- result: 54/178 and 54/178, identical to the fresh-single-set control form fA (54/178) - the sets left no byte trace at all. Banked at rejected/r4-self-assign-cse-deletes-scores-54.c and rejected/r4-reestablished-from-base1-cse-deletes-scores-54.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 46867ae6, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s14] The target's own sched1 picks the a2 base at clock 64 and the three struct stores at clocks 61-63, so s12's claim that any INSN_PRIORITY lift overshoots by construction is not a class argument against the priority family.
- mechanism: schedule_block is a backward list scheduler (sched.c:4036-4038), so emission order is the reverse of selection order. The target window at asm/funcs/func_8005D554.s 4DEB4-4DECC is addiu a0,sp,0x10 / addu a1,zero,zero / lw v1,gp / addiu a2,s4,-0xC / sw zero,0x20(sp) / sw s6,0x24(sp) / sw s1,0x1C(sp); reversed that is sw, sw, sw, a2 base, lw, a1, a0.
- probe: read the target asm window directly and reversed it against s9's recorded control pick sequence (clock61=234, 62=231, 63=228, 64=242, 65=205, 66=240, 67=211).
- result: The single divergent decision is the clock-64 three-way tie [242(l43) 240(l42) 211(l31)], which the target resolves to 211. The banked score-0 body boosts its base to max_priority and is picked at clock 61, AHEAD of the stores, and still scores 0 - so a later pass completes the rotation and an overshoot at sched1 is survivable. The priority family is therefore re-opened, not closed.
- verdict: CONFIRMED

## [s15] Every spelling that gives the a2 expression's multiply, shift or base its own named per-half local is LICM-hoisted, collapsing the build from 176 to 153 instructions at score 42.
- mechanism: a per-half named local for a sub-expression of (s32)r4 - K + ((u32)(D_800A3418 * M) >> 0xF) is set exactly once, so n_times_set is 1 and scan_loop accepts the SET as a movable (loop.c:705) whenever any of its three guard disjuncts (loop.c:695-700) holds; every C-level local is a user variable, so disjunct B is false for free, but disjunct A stays true because no jump or label precedes the set inside this single-basic-block loop body. The invariant base and, through it, the whole chain are hoisted to the preheader.
- probe: a 48-form symmetric cross product over the a2 expression's naming space (multiply named/inlined x shift named/inlined x base named/inlined x base-declaration position x final-add operand order x multiply operand order), the SAME spelling applied to both halves. tmp/grind/func_8005D554/s15/genA.py, variants in s15/enumA, histogram in s15/sweepA.json.
- result: 44 of 48 variants score 42 at 153 build instructions (23 BELOW the target's 176); the only variants that keep 176 are the four fully-inlined spellings, which score 15. Banked at rejected/named-singleset-mul-sh-base-locals-all-licm-hoisted-scores-42.c
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 36291a08, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.
- predicate_cite: loop.c:705

## [s15] The base-last association moves the a2 base instruction out of the pre-call window and makes addiu a0,sp,0x10 win the window's first slot, but combine reassociates the constant off s4 onto the multiply result, so this spelling measures 15/176.
- mechanism: writing the half as a2_offset = ((u32)(D_800A3418 * M) >> 0xF); a2_offset += (s32)r4 - K; expands the r4 - K subtraction AFTER the seven-instruction multiply-shift chain, raising the base insn's INSN_LUID past the chain; but the resulting RTL is (plus (reg m) (plus (reg r4) (const -K))) and combine folds the constant into the already-available multiply result, emitting addiu v0,v0,-12 followed by addu v0,v0,s4 instead of the target's addiu a2,s4,-0xC followed by addu a2,a2,v0.
- probe: tmp/grind/func_8005D554/s15/v/v03_both_a2_baselast.c and the enumA fully-inlined spellings; disassembled from tmp/sandbox/func_8005D554/text1b.o.
- result: 15/176. The emitted pre-call window becomes [addiu a0,sp,0x10][lw v1,0(gp)][move a1,zero][sw zero][sw s6][sw s1] at 0x3594-0x35A8 -- addiu a0,sp,0x10 occupies the target's slot-1 position (4DEB4), which the control never achieves -- while the base pair sits at 0x35C4/0x35C8 with the constant on the multiply result. Single-half base-last forms measure 8/176. Banked at rejected/a2-base-last-association-combine-moves-const-off-s4-scores-15.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 36291a08, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s15] Staging the multiply-shift through the dead existing local a0_offset costs 23 points and costs exactly the same whether the a2 base statement precedes or follows the staged value.
- mechanism: a0_offset is dead after s.zero18 = a0_offset;, so reusing it for the half's own multiply-shift result extends no live range and adds no instruction (both forms build 176). The identical score for the two statement orders shows the base statement's position relative to the staged value leaves the emitted stream unchanged, matching s6's predicate-backed closure of statement placement (sched.c:2464).
- probe: tmp/grind/func_8005D554/s15/enumB/E_mul_via_a0offset_base_late.c and F_mul_via_a0offset_base_first.c
- result: 23/176 and 23/176. Staging through a single combined statement measures 18/176. Banked at rejected/mulshift-staged-through-dead-a0offset-scores-23.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 36291a08, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s15] Building the a2 base in three steps (copy r4, subtract the constant, accumulate the multiply-shift) is byte-inert at the floor.
- mechanism: combine folds the register copy and the immediate subtraction into a single addiu with the same destination, so the RTL reaching sched1 is identical to the control's two-statement form.
- probe: tmp/grind/func_8005D554/s15/enumB/D_copy_sub_acc.c
- result: 6/176, identical to the control. Banked at rejected/a2-base-built-copy-sub-acc-byte-inert-scores-6.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 36291a08, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s15] Hoisting the s.zero10/s.one14/s.ret stores above the a2 chain so that the a2 chain is the last thing before the call does not help, and it is the most expensive store move measured.
- mechanism: the three stores are scheduled at clocks 61-63 in both bodies because their memory function unit gives them a nonzero potential_hazard (sched.c:1338) that the arithmetic window insns lack; moving their source statements earlier only lowers their INSN_LUIDs, which is the term they already win on.
- probe: tmp/grind/func_8005D554/s15/v/v08_control_stores_hoisted.c (control chain) and v07_baselast_stores_hoisted.c (base-last chain)
- result: 32/176 and 41/176. Banked at rejected/zero10-one14-ret-stores-hoisted-above-a2-chain-scores-32.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 36291a08, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s15] Every spelling that gives the a2 expression's multiply, shift or base its own named per-half local is LICM-hoisted, collapsing the build from 176 to 153 instructions at score 42.
- mechanism: A per-half named local for a sub-expression of (s32)r4 - K + ((u32)(D_800A3418 * M) >> 0xF) is set exactly once, so n_times_set is 1 and scan_loop accepts the SET as a movable (loop.c:705) whenever any of its three guard disjuncts (loop.c:695-700) holds. Every C-level local is a user variable, so disjunct B is false for free, but disjunct A stays true because no jump or label precedes the set inside this single-basic-block loop body, so the invariant base and its whole chain are hoisted into the preheader.
- probe: A 48-form symmetric cross product over the a2 expression's naming space (multiply named/inlined x shift named/inlined x base named/inlined x base-declaration position x final-add operand order x multiply operand order), the same spelling applied to both halves. tmp/grind/func_8005D554/s15/genA.py, variants in s15/enumA, histogram in s15/sweepA.json.
- result: 44 of 48 variants score 42 at 153 build instructions - 23 BELOW the target's 176. The only variants that keep 176 instructions are the four fully-inlined spellings, which score 15. This reproduces s13's multi-set requirement from a completely independent direction: a2_offset must stay multi-set or the whole chain leaves the loop. Banked at rejected/named-singleset-mul-sh-base-locals-all-licm-hoisted-scores-42.c
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 36291a08, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.
- predicate_cite: loop.c:705

## [s15] The base-last association moves the a2 base instruction out of the pre-call window and gives addiu a0,sp,0x10 the window's first slot, but combine reassociates the constant off s4 onto the multiply result, and this spelling measures 15/176.
- mechanism: Writing the half as a2_offset = ((u32)(D_800A3418 * M) >> 0xF); a2_offset += (s32)r4 - K; expands the r4 - K subtraction after the seven-instruction multiply-shift chain, raising the base insn's INSN_LUID past that chain. The resulting RTL is (plus (reg m) (plus (reg r4) (const -K))) and combine folds the constant into the already-available multiply result, emitting addiu v0,v0,-12 then addu v0,v0,s4 instead of the target's addiu a2,s4,-0xC then addu a2,a2,v0.
- probe: tmp/grind/func_8005D554/s15/v/v03_both_a2_baselast.c plus the four fully-inlined enumA spellings and the single-half forms v01/v02; disassembled from tmp/sandbox/func_8005D554/text1b.o.
- result: 15/176 for both halves base-last, 8/176 for one half only. The emitted pre-call window becomes [addiu a0,sp,16][lw v1,0(gp)][move a1,zero][sw zero][sw s6][sw s1] at 0x3594-0x35A8, so addiu a0,sp,0x10 occupies the target's slot-1 position (4DEB4) for the first time in fifteen sessions - the control emits the a2 base there instead. The base pair lands at 0x35C4/0x35C8 with the constant on the multiply result. Across all 61 forms measured this session, no spelling produced both the late base instruction and the constant attached to s4. Banked at rejected/a2-base-last-association-combine-moves-const-off-s4-scores-15.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 36291a08, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s15] Staging the half's multiply-shift through the dead existing local a0_offset costs 23 points and costs exactly the same whether the a2 base statement precedes or follows the staged value.
- mechanism: a0_offset is dead after s.zero18 = a0_offset;, so reusing it for the half's own multiply-shift result extends no live range and adds no instruction - both forms build 176. The identical score for the two statement orders shows the base statement's position relative to the staged value leaves the emitted stream unchanged, replicating s6's closure of statement placement (sched.c:2464).
- probe: tmp/grind/func_8005D554/s15/enumB/E_mul_via_a0offset_base_late.c and F_mul_via_a0offset_base_first.c, plus the single-combined-statement variant G.
- result: 23/176 and 23/176; the combined-statement variant measures 18/176. Banked at rejected/mulshift-staged-through-dead-a0offset-scores-23.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 36291a08, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s15] Building the a2 base in three steps - copy r4, subtract the constant, accumulate the multiply-shift - is byte-inert at the floor.
- mechanism: combine folds the register copy and the immediate subtraction into a single addiu with the same destination, so the RTL reaching sched1 is identical to the control's two-statement form.
- probe: tmp/grind/func_8005D554/s15/enumB/D_copy_sub_acc.c
- result: 6/176, identical to the control - a second distinct spelling that sits exactly at the floor. Banked at rejected/a2-base-built-copy-sub-acc-byte-inert-scores-6.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 36291a08, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s15] Hoisting the s.zero10, s.one14 and s.ret stores above the a2 chain, so the a2 chain is the last thing before the call, is the most expensive store move measured this session.
- mechanism: The three stores are selected at clocks 61-63 in both bodies because their memory function unit gives them a nonzero potential_hazard (sched.c:1338) that the arithmetic window insns lack; moving their source statements earlier only lowers their INSN_LUIDs, which is a term they already win.
- probe: tmp/grind/func_8005D554/s15/v/v08_control_stores_hoisted.c (control chain) and v07_baselast_stores_hoisted.c (base-last chain).
- result: 32/176 and 41/176. Banked at rejected/zero10-one14-ret-stores-hoisted-above-a2-chain-scores-32.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 36291a08, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s15] All four contested window instructions draw their INSN_PRIORITY of 3 from a single shared predecessor - the third jal rand - so no operand respelling of the a2 base can raise its priority without making it a consumer of the in-block load.
- mechanism: The PRIODBG capture shows insns 205 (lw), 211 (a2 base), 240 (addiu a0,sp,0x10) and 242 (addu a1,zero,zero) each take pred=201 kind=14 pred_pri=3 cost=1 contrib=3, and the three struct stores 228/231/234 also finish at 3. The only priority-4 insns in the window are the load's own data consumers 206 and 208, which get 3 + 2 - 1 via the load's ready cost (sched.c:1497). The a2 base's value is (s32)r4 - K and r4 is set outside the loop, so it has no in-block data producer to inherit depth from.
- probe: tmp/grind/func_8005D554/s13/sched.log lines 54365-56111 extracted to tmp/grind/func_8005D554/s15/p1.log, grepped for the window uids and for every final_pri above 3.
- result: Confirmed against the compiler's own trace; this is the reason the 48-form naming sweep could not move the priority term without also adding an instruction or triggering the LICM hoist.
- verdict: CONFIRMED

## [s16] KILL RE-AUDIT: the three closest banked forms reproduce their recorded scores exactly on the current chassis, and none of them carries a FAKE construct.
- mechanism: `tools/fake_ablate.py --func func_8005D554 --file text1b --candidate rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c` reports "no FAKE-annotated constructs found; nothing to ablate", so no banked lever in this ledger was ever measured while a FAKE carrier occupied the a2 pseudo. The banked scores are therefore chassis-relative only, and the chassis is unchanged.
- probe: tmp/grind/func_8005D554/s16/reaudit swept with tools/sweep_variants.py.
- result: a2-statements-at-maximal-pre-call-birth-point 6/176, a2-base-built-copy-sub-acc 6/176, a2-base-last-association-combine-moves-const-off-s4 15/176 - all identical to their banked values. Control (candidate.c) re-measures 6/176.
- verdict: CONFIRMED

## [s16] A MULTI-SET existing local as the a2 base's carrier blocks combine's reassociation and keeps addiu a2,s4,-K with the constant on s4 at 176 instructions, but the base insn is still emitted in window slot 1.
- mechanism: combine.c will not substitute a value through a pseudo whose REG_N_SETS exceeds 1, so with the base staged into the dead existing local a0_offset the nested plus (plus m (plus r4 -K)) is never folded into (plus (plus m -K) r4); the base is materialised on its own as addiu <reg>,s4,-K. a0_offset already carries four real sets per iteration, so loop.c:705's n_times_set test also fails and the invariant set is not hoisted. Neither fact touches sched1's clock-64 ready-list pick, which is still decided by INSN_LUID (sched.c:2462), and the base's LUID is still below the expand_call argument moves' (calls.c:1880).
- probe: tmp/grind/func_8005D554/s16/enumA, 36 forms = 13 shapes (control, base-last plain, base-last via a0_offset in four associations, sum folded into the s.zero1C store, via v0, via v3, stores hoisted above the chain, staged into ret after the s.ret store, copy-then-subtract into a0_offset, a0_offset accumulates a2_offset, cast-free base) crossed with {both halves, half 1 only, half 2 only}. Histogram in s16/sweepA.json; S2_both, S6_both, S11_both and the control disassembled from tmp/sandbox/func_8005D554/text1b.o.
- result: best new form 12/176 (S11_both). S2_both scores 16/176 and its half-1 window at 0x3594 is [addiu a2,s4,-12][addiu a0,sp,16][lw v1,gp][move a1,zero] - the control's rotation verbatim, with the target's operand form on the base. No form in the sweep emitted the base after the argument setup while keeping 176 instructions. Banked at rejected/a2-base-via-dead-a0offset-carrier-const-kept-but-emitted-first-scores-16.c and rejected/a0offset-carries-base-a2offset-accumulated-onto-it-scores-12.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 07f3c383, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s16] s15's base-last association never won a scheduling decision - it moved the base only by making it a data consumer of the multiply chain, which is the same edit that moved the constant off s4.
- mechanism: with the constant folded onto the multiply result the surviving insn is addiu v0,v0,-12, a dependent of the seven-instruction multiply-shift chain, so dependence alone forces it past the argument setup. Restoring the constant to s4 (by a multi-set carrier, s16 above) restores the base to an independent insn whose only predecessor is the jal anti-dependence, and it returns to slot 1. The two properties are not independent axes; they are one axis with two ends.
- probe: side-by-side disassembly of s16/enumA/S1_both.c (15/176, base at 0x35C4 with the constant on the multiply result) and s16/enumA/S2_both.c (16/176, base at 0x3594 with the constant on s4).
- result: CONFIRMED against the compiler. This closes s15's frontier item 1 without needing a second consumer of the r4 - K value: the second consumer was only ever a way to block the reassociation, and blocking the reassociation is measured to restore the early emission.
- verdict: CONFIRMED

## [s16] Staging the a2 base through the dead local v0 is byte-inert inside the half-1 window and costs its 15 points elsewhere in the function.
- mechanism: v0 is dead inside the loop but its live range before the loop makes the merged pseudo a longer-lived allocno, so local-alloc seats the loop's other values differently; the window insns themselves are unchanged.
- probe: tmp/grind/func_8005D554/s16/enumA/S6_both.c, S6_h1.c, S7_both.c, S7_h1.c, disassembled at 0x3564-0x35bc.
- result: S6_both 15/176 with the half-1 window byte-identical to the control's; S6_h1 43/178 and S7_h1 37/178 (the single-half borrows each cost an instruction). Banked at rejected/a2-base-staged-through-dead-v0-scores-15.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 07f3c383, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s16] The LICM escape for a SINGLE-SET base carrier is reachable in ordinary C, and every C spelling that reaches it puts a CODE_LABEL between the base's set and its use, which moves the base out of the call's basic block and emits it before the argument setup.
- mechanism: loop.c:695-700 accepts a movable when any of A (not maybe_never and not loop_reg_used_before_p), B (dest not a user variable) or C (reg_in_basic_block_p) holds. B is false for every C-level local. reg_in_basic_block_p returns 0 either when regno_first_uid for the carrier is not this insn (loop.c:1068 - an earlier reference anywhere in the function) or when the forward scan from the set reaches a CODE_LABEL or BARRIER before the carrier's last use (loop.c:1093). maybe_never becomes 1 once the loop scan has passed any CODE_LABEL or JUMP_INSN (loop.c:930). The first route requires a single-set local to be read before it is written on the first iteration, which is not a semantics-preserving spelling of this function; the second route requires labels on both sides of the set. schedule_insns runs per basic block (sched.c:4937), so those labels put the base and its multiply chain in a region that is not the call's block, and the base is necessarily emitted before the call's argument setup - the side the control already has.
- probe: tmp/grind/func_8005D554/s16/enumB - half 2's p0 selection split into `odd = D_800A3418 & 1; if (odd) s.p0 = base_offset + 0xC;` before the base and `if (!odd) s.p0 = base_offset;` after it, with the base carried in a fresh single-set local (B1, B2), in the multi-set a0_offset (B6), and with the base placed before both ifs (B3), after both ifs (B4) and after a plain if/else join (B5); plus the two conditional chassis alone (B7, B8). Disassembled from tmp/sandbox/func_8005D554/text1b.o.
- result: B1 66/178 - base2 is single-set and is NOT hoisted (addiu a0,s5,-25 emitted at 0x365c inside the loop, in the second branch's delay slot), so the escape is real, but the half-2 window collapses to [addu a2,a0,v0][addiu a0,sp,0x10][move a1,zero][sw][sw][sw] at 0x367c-0x3690 with the base already spent. B6 scores the identical 66/178, so the carrier's set count is invisible in this shape. B3/B4/B5 (only one disjunct falsified) 38/179. B7 and B8 53/177 - the conditional chassis itself costs exactly one instruction whether spelled as two ifs or as an if/else, reproducing s11 item 3. Banked at rejected/licm-escape-split-p0-ifs-base-between-labels-block-split-scores-66.c and rejected/split-p0-into-two-ifs-control-chain-scores-53.c.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 07f3c383, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.
- predicate_cite: loop.c:1093

## [s16] The three closest banked forms carry no FAKE-annotated construct and reproduce their recorded scores exactly on the current chassis.
- mechanism: tools/fake_ablate.py reports 'no FAKE-annotated constructs found; nothing to ablate' for rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c, so no banked lever in this ledger was ever measured while a FAKE carrier occupied the a2 pseudo; the banked scores are chassis-relative only and the chassis is unchanged since s15.
- probe: tmp/grind/func_8005D554/s16/reaudit swept with tools/sweep_variants.py; fake_ablate run on the closest form.
- result: a2-statements-at-maximal-pre-call-birth-point 6/176, a2-base-built-copy-sub-acc 6/176, a2-base-last-association-combine-moves-const-off-s4 15/176 - all identical to their banked values. Control candidate.c 6/176. Seventh consecutive passing re-audit.
- verdict: CONFIRMED

## [s16] Staging the a2 base through a MULTI-SET existing local (the dead a0_offset, or v0, or v3, or ret) keeps addiu a2,s4,-K with the constant on s4 at 176 instructions but still emits the base in window slot 1, ahead of addiu a0,sp,0x10 / lw / move a1, in all 36 forms measured.
- mechanism: combine.c will not substitute a value through a pseudo whose REG_N_SETS exceeds 1, so the nested plus (plus m (plus r4 -K)) is never folded into (plus (plus m -K) r4) and the base is materialised on its own; a0_offset's four real sets per iteration also fail loop.c:705's n_times_set test, so nothing is hoisted. Neither fact touches sched1's clock-64 ready-list pick, which is still decided by INSN_LUID (sched.c:2462) with the base's LUID bounded below the expand_call argument moves' by calls.c:1880.
- probe: tmp/grind/func_8005D554/s16/enumA - 13 shapes (control, base-last plain, base-last via a0_offset in four associations, sum folded into the s.zero1C store, via v0, via v3, stores hoisted above the chain, staged into ret after the s.ret store, copy-then-subtract into a0_offset, a0_offset accumulates a2_offset, cast-free base) crossed with both-halves / half-1-only / half-2-only; histogram in s16/sweepA.json; S0/S1/S2/S6/S11 disassembled from tmp/sandbox/func_8005D554/text1b.o.
- result: Histogram 6 (1), 8 (2), 12 (1), 15 (3), 16 (8), 19 (12), 21 (2), 22 (2), 26 (2), 33 (1), 37 (2), 43 (2); nothing below 6. Best new form 12/176 (a0_offset carries the base, a2_offset accumulated onto it). S2_both 16/176 has the target's operand form addiu a2,s4,-12 at 0x3594 but in the control's slot 1. Banked at rejected/a2-base-via-dead-a0offset-carrier-const-kept-but-emitted-first-scores-16.c, rejected/a0offset-carries-base-a2offset-accumulated-onto-it-scores-12.c, rejected/a2-base-staged-through-dead-v0-scores-15.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ 07f3c383, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.

## [s16] s15's base-last association moved the a2 base only by making it a data dependent of the multiply chain, and that is the same edit that moved the constant off s4 - so the two properties are one axis with two ends, not two independent axes.
- mechanism: With the constant folded onto the multiply result the surviving insn is addiu v0,v0,-12, a dependent of the seven-instruction multiply-shift chain, and dependence alone drags it past the argument setup. Blocking the fold with a multi-set carrier restores an independent insn whose only predecessor is the jal anti-dependence, and it returns to slot 1.
- probe: Side-by-side disassembly of s16/enumA/S1_both.c (15/176, base at 0x35C4 as addiu v0,v0,-12) and s16/enumA/S2_both.c (16/176, base at 0x3594 as addiu a2,s4,-12).
- result: Confirmed against the compiler. This closes s15's frontier item 1 without needing a second real consumer of the r4 - K value: the second consumer was only ever a way to block the reassociation, and blocking the reassociation is measured to restore the early emission.
- verdict: CONFIRMED

## [s16] Every C spelling that falsifies both loop.c guard disjuncts A and C for a single-set a2-base carrier places a CODE_LABEL between the carrier's set and its use, which puts the base outside the call's basic block and emits it before the argument setup.
- mechanism: loop.c:695-700 accepts a movable when any of A (not maybe_never and not loop_reg_used_before_p), B (dest not a user variable) or C (reg_in_basic_block_p) holds; B is false for every C-level local. reg_in_basic_block_p returns 0 either at loop.c:1068 (an earlier reference to the carrier anywhere in the function - which for a single-set local means reading it before it is written on the first iteration, not a semantics-preserving spelling here) or at loop.c:1093 (a CODE_LABEL or BARRIER reached between the set and the last use). maybe_never, which falsifies A, is set at loop.c:930 by the first CODE_LABEL or JUMP_INSN the loop scan passes. schedule_insns runs per basic block (sched.c:4937), so the labels required by loop.c:1093 necessarily separate the base from the call.
- probe: tmp/grind/func_8005D554/s16/enumB - half 2's p0 selection split into two ifs around the base's set (odd = D_800A3418 & 1; if (odd) s.p0 = base_offset + 0xC; ... base2 = (s32)r4 - 0x19; if (!odd) s.p0 = base_offset; a2_offset = base2 + m), with the base in a fresh single-set local (B1, B2), in the multi-set a0_offset (B6), before both ifs (B3), after both ifs (B4), after a plain if/else join (B5), plus the two conditional chassis alone (B7, B8). Disassembled from tmp/sandbox/func_8005D554/text1b.o.
- result: B1 66/178 - the escape is REAL (base2 is single-set and is not hoisted; addiu a0,s5,-25 is emitted at 0x365c inside the loop, in the second branch's delay slot) but the half-2 window collapses to [addu a2,a0,v0][addiu a0,sp,0x10][move a1,zero][sw][sw][sw] at 0x367c-0x3690 with the base already spent. B6 scores the identical 66/178, so carrier set-count is invisible through a block boundary. B3/B4/B5 (only one disjunct falsified) 38/179. B7 and B8 both 53/177, so the two-ifs spelling is free relative to the conditional and the conditional itself costs exactly one instruction. Banked at rejected/licm-escape-split-p0-ifs-base-between-labels-block-split-scores-66.c and rejected/split-p0-into-two-ifs-control-chain-scores-53.c.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD main @ 07f3c383, candidate.c chassis, control floor re-measured 6/176; no FAKE constructs present.
- predicate_cite: loop.c:1093

## s17 (enumerate, 2026-09-09, HEAD main @ fa84454f) — H40-H42

Chassis: candidate.c, control re-measured 6/176. No FAKE constructs present in the control or in
either of the two re-audited banked forms (fake_ablate reports none). 66 spellings measured.

**H40 — KILLED (instance).** *Statement:* Borrowing an existing dead local (arg0, arg1, v0, v3,
or a mixed pair) as the a2 base carrier in the score-0 body's early slot — the set placed before
the `s.zero18` store, so it spans the third `rand` call — builds 178 or 179 instructions in all
18 measured forms and scores 43-63, never 176.
*Mechanism:* birthing_insn_p (sched.c:2505) only boosts an insn whose SET survives combine with
`reg_n_sets == 1`; that requires the carrier's own set to be the base insn, which requires the
set to precede the third rand. The resulting live range crosses a call, so the allocator adds
the spill/copy pair that s13 measured for a0_offset and a2_offset. s17 extends that measurement
from the two loop locals to the two parameters and the two preheader temporaries.
*Measured on:* HEAD main @ fa84454f, candidate.c chassis, control 6/176, no FAKE constructs.
*Probe:* tmp/grind/func_8005D554/s17/gen.py -> enumA (36 forms), swept with sweep_variants.

**H41 — KILLED (instance).** *Statement:* The combine-deletable second write that raises the
borrowed carrier's loop-time set count is byte-inert in this function in both its value and its
position: the `ret`, `one14`, `zero10`, `c20` and `p1` restages all produce identical scores for
a given carrier pair and base-set position, across 33 measured forms.
*Mechanism:* the second write exists only to make loop.c's `n_times_set` 2 (loop.c:705) and is
deleted by combine before sched1, so it contributes no insn and no dependence; only the carrier's
identity and the base set's position reach the emitted bytes.
*Measured on:* HEAD main @ fa84454f, candidate.c chassis, control 6/176, no FAKE constructs.
*Probe:* enumA (3 second-write values x 12 carrier/position cells) + enumB (3 anchor positions x
10 carrier/shape cells).

**H42 — KILLED (instance).** *Statement:* With the borrowed carrier's base set at the control
slot (after the third rand), the shape in which a2_offset consumes it — copy+accumulate, a single
sum, a multiply-first accumulate, a direct store into `s.zero1C`, or a carrier self-accumulate —
is byte-inert: all fifteen v0/v3 forms score 19-21 at 176 instructions and the half-1 window at
0x3594 disassembles to the control rotation with the base reseated from a2 to a3.
*Mechanism:* at the control slot combine folds the carrier copy into the accumulate, so the
surviving base insn's dest is `a2_offset` again (four sets) and birthing_insn_p's
`reg_n_sets == 1` precondition fails; what is left is a pure register-seat tax from extending the
borrowed local's live range.
*Measured on:* HEAD main @ fa84454f, candidate.c chassis, control 6/176, no FAKE constructs.
*Probe:* tmp/grind/func_8005D554/s17/genB.py -> enumB (30 forms), plus the 0x3594 disassembly of
B_v0v3_ret_ctrl (tmp/grind/func_8005D554/s17/dis.sh).

### Frontier after s17

R1. The carrier's second loop-time set has never been supplied by RESTRUCTURING an existing
    multi-set local's own real chain — every form measured so far (fresh nv/nw, and all of s17's
    borrows) adds a redundant restage statement. `a0_offset` already has two real sets per half
    (`= (s32)r5 - K` and `+= m`); the untested question is whether an ordinary-C restructuring of
    that chain can leave the a2 base as a2_offset's ONLY surviving set at sched1 without adding
    a statement. Probe: enumerate rewrites of the a0/a2 chains as a single shared accumulator
    whose per-half sets are all real values, gate on build_insns == 176, and capture
    BB2_SCHED_DEBUG=1 ADJPRI lines for the base insn to see whether birth flips to 1.
R2. (unchanged) Header-canonical aggregate declaration for the 0x3C-stride particle-template
    table at D_8009B2E0 and the pair D_8009B388/D_8009B390 — the last untried non-spelling axis.
    Needs an integration handoff for the header edit.
R3. (unchanged) INSN_PRIORITY 4 for the base insn at 176 instructions: make the base a data
    consumer of an insn already present in the block at zero added cost (sched.c:1497 gives
    insn_cost 2 only to a load's consumer).

## [s17] Borrowing an existing dead local (arg0, arg1, v0, v3, or a mixed pair) as the a2 base carrier in the score-0 body's early slot - the set placed before the s.zero18 store, so it spans the third rand call - builds 178 or 179 instructions in all 18 measured forms and scores 43-63, never 176.
- mechanism: birthing_insn_p (sched.c:2505) only boosts an insn whose SET survives combine with reg_n_sets == 1; that requires the carrier's own set to BE the base insn, which requires the set to precede the third rand call. The resulting live range crosses a call, so the allocator adds the copy/spill pair s13 measured for a0_offset and a2_offset. s17 extends that measurement from the two loop locals to the two parameters and the two preheader temporaries, i.e. to every local this function contains that is dead inside the loop.
- probe: tmp/grind/func_8005D554/s17/gen.py generated 36 forms (6 carrier pairs x 3 combine-deletable second writes x 2 base-set positions) into s17/enumA; swept with tools/sweep_variants.py, histogram s17/enumA.json. Every early-position cell built 178 or 179 instructions.
- result: Round A histogram 21 (6 forms), 23 (3), 28 (4), 30 (3), 40 (2), 43 (2), 50 (4), 55 (2), 60 (1), 61 (4), 63 (5). All 18 early-position forms build 178/179. arg0/arg1 pay +3, v0/v3 pay +2, mixed pairs +2 or +3. Representative banked at rejected/param-borrow-early-birth-slot-costs-three-insns-scores-50.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ fa84454f, memory/grind/func_8005D554/candidate.c chassis, control re-measured 6/176; fake_ablate reports no FAKE-annotated construct in the control or in either re-audited banked form.

## [s17] The combine-deletable second write that raises a borrowed carrier's loop-time set count is byte-inert in this function in both its value and its position: the ret, one14, zero10, c20 and p1 restages produce identical scores for a given carrier pair and base-set position across 33 measured forms.
- mechanism: The second write exists only to make loop.c's n_times_set equal 2 so the invariant base is not accepted as a movable (loop.c:705), and combine deletes it before sched1 because it is a redundant copy into a store. It therefore contributes no instruction and no dependence edge; only the carrier's identity and the base set's position reach the emitted bytes.
- probe: enumA crossed 3 second-write values over 12 carrier/position cells; enumB crossed 3 anchor positions (s.ret after the a2 chain, s.c20 and s.p1 both ahead of it) over 10 carrier/shape cells. Histograms s17/enumA.json and s17/enumB.json.
- result: Identical scores within every cell - all three v0/v3 ctrl forms score 21/176; the arg0/arg1 ret_ctrl and zero10_ctrl forms both score 40/179; and the two earlier anchors reproduce the s.ret score exactly for every v0/v3 round-B shape (21/176).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ fa84454f, candidate.c chassis, control re-measured 6/176; no FAKE constructs present.

## [s17] With the borrowed carrier's base set at the control slot (after the third rand), the shape in which a2_offset consumes it - copy+accumulate, a single sum, a multiply-first accumulate, a direct store into s.zero1C, or a carrier self-accumulate - is byte-inert: all fifteen v0/v3 forms score 19-21 at 176 instructions and the half-1 window at 0x3594 disassembles to the control rotation with the base reseated from a2 to a3.
- mechanism: At the control slot combine folds the carrier copy into the accumulate, so the surviving base insn's dest is a2_offset again (four sets) and birthing_insn_p's reg_n_sets == 1 precondition fails. What remains is a pure register-seat tax from extending the borrowed local's live range into the loop.
- probe: tmp/grind/func_8005D554/s17/genB.py generated 30 forms (2 carrier pairs x 5 consumption shapes x 3 second-write anchors) into s17/enumB; swept with sweep_variants.py (histogram s17/enumB.json) and the best cell disassembled with s17/dis.sh.
- result: Round B histogram 19 (2 forms), 21 (13), 26 (2), 28 (9), 40 (4). Disassembly of B_v0v3_ret_ctrl at 0x3594: addiu a3,s4,-12 / addiu a0,sp,16 / lw v1,gp / move a1,zero - the control rotation verbatim. Best new form overall 19/176, banked at rejected/existing-local-borrow-selfacc-c20-restage-scores-19.c; the 21/176 representative at rejected/existing-local-borrow-ctrl-position-base-reseated-a3-scores-21.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main @ fa84454f, candidate.c chassis, control re-measured 6/176; no FAKE constructs present.
