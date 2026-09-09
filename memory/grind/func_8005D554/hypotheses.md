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
