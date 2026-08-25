# Hypothesis ledger — func_800460E4

## [s1] 2026-08-25 (recon)

H1 — "The four addu operand-order rules (@76/@102/@129/@165) correspond to the
scaled-index `(s3 << 2)` address computations, and the offset-first order is
produced by spelling them as an integer sum with the shift term first."
Probe: rewrote the four sites; measured.
- Sub-result: `&s0[s3]` spelling measured FLAT (base-first) — KILLED as a
  spelling; `(s32 *)((s3 << 2) + (s32)s0)` measured to flip all four sites.
Verdict: **CONFIRMED** (35 → 32 combined with H2's alias drop; diff inspection
shows all four sites now offset-first, matching target).

H2 — "Dropping the p/p2 param-alias locals (direct `((s32*)arg1)[s3]` reads)
is byte-neutral and raises arg1's pseudo ref count."
Probe: dropped both; measured + .greg/solver extract.
Verdict: **CONFIRMED byte-neutral; NOT sufficient for the rotation** (residual
32 still the pure 3-seat rotation; arg1 pri 3703 < s2 pri 4444).

H3 — "The 3-way rotation exists because the original had arg1 and the s1 local
as ONE variable (the param reused as the func_80045600-argument carrier); the
merged pseudo's ref count lifts its global.c priority above the s2 pointer so
it is allocated first and takes $s1, cascading s2→$s2, s3→$s3."
Probe: ra_solver simulate (17/17 ground-truth match) + inverse (REACHABLE, all
22 vectors refs_up on 73/80; honest split ref-counts already equal target's
visible uses, so a split-pseudo original is arithmetically excluded) → merged
the variable in C; measured.
Verdict: **CONFIRMED — sandbox 0** (35 → 0 total this session).

## [s3] 2026-08-25 (recon, after the layer-1 FAIL banned H3's merge construct)

H4 — "With the banned merge reverted (s1 restored) but the [s1] spelling wins
kept, the honest floor is the pure 3-seat rotation."
Probe: reverted src; measured. Verdict: **CONFIRMED — sandbox 32** (248/248).

H5 — "The duplicated-statement-into-arms family can add the +2 byte-dead refs
on pseudo 73/80."
Probe: mapped every $s1 ref in target asm against our pseudo refs. All `s1 =
s2;` stores are arm HEADS; cross-jump merges only TAILS; no mergeable
real-statement site involving s1/arg1 exists. Verdict: **KILLED (structural —
no site; see evidence.md [s3])**.

H6 — "A combine-foldable chain-extender (dead-store-fake-exception scope
extension, rule line 32) on s1's default init adds exactly the +2 reg_n_refs
the ra_solver inverse demands, folds to zero bytes, and flips the rotation."
Probe: `s1 = (s32*)((s32)s4 - (s32)s0); s1 = (s32*)((s32)s1 + (s32)s0);`
measured + dumps (flow: refs 11→13; greg: 73→$17, 78→$18, 79→$19, 80→$17 =
target seating; insns 248/248, score 0).
Verdict: **CONFIRMED — sandbox 0, measured twice (with and without the FAKE
annotation).**

## [s4] 2026-08-25 (recon, after the second layer-1 FAIL banned the case-3 volatile)

H7 — "The 3 insns the case-3 volatile preserved are deleted by jump2
find_cross_jump (case-3 tail merged into case 34's), downstream of sched1
placing li/sh between the two loads (no alias edge on the mem/s loads) and the
resulting seat assignment."
Probe: novol disassembly (j .L31 into case 34's srl/sll/addu tail) + dump .s +
.sched (sh insn has no load deps at the mem/s chassis).
Verdict: **CONFIRMED** (full chain in evidence.md [s4]).

H8 — "A statement-order respelling of case 3 can reproduce target's
loads-adjacent/store-after order." Probe: P1-P4 sweep + sched_solver perturb
(all atoms depth 2, luid atoms depth 3, block 19 pass 1).
Verdict: **KILLED — unreachable**; the original RTL must differ structurally.

H9 — "Fresh pointer intermediates (pm2/pm1) make the loads non-MEM_IN_STRUCT_P,
so sched.c raises anti-dependence edges load→store(D_8009947A), forcing target
order by REAL dependence; seats and the cross-jump defeat follow."
Probe: spelled, measured, dumped (REG_DEP_ANTI 312/315 on insn 320).
Verdict: **CONFIRMED — sandbox 0 (248/248), measured twice** (before/after the
FAKE annotation). The banned volatile is REMOVED; the [s3] chain-extender is
independently still required (removing it → 32).

Frontier: (empty — function at sandbox 0 with BOTH banned constructs absent;
remaining work is integration: rule retirement via the normal retire path +
layer-1/Judge gates on the two annotated FAKE constructs.)
