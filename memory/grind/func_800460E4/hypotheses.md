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

## [s5] 2026-08-25 (recon, after the third layer-1 FAIL banned pm2/pm1)

H10 — "MEM_IN_STRUCT_P is a TREE-SHAPE property (expr.c:4570: set iff the
INDIRECT_REF operand is a PLUS_EXPR / SAVE_EXPR-of-PLUS / aggregate ref), so
a direct deref of a CAST integer-arithmetic address — the function's own
established scaled-index idiom `(s3 << 2) + (s32)s0` — yields a non-/s MEM
with NO invented locals, giving sched.c the real anti-dependence edges that
force target's load/load/store order in case 3."
Probe: respelled case 3 fully inlined (store last, case-13 order); measured;
dumped. Loads 306/318 non-/s; store 327 carries REG_DEP_ANTI 306+318.
Verdict: **CONFIRMED — sandbox 0 (248/248), canonical distance 0.** All
three banned constructs absent; zero new FAKE constructs.

H11 — "Statement order still matters at the non-/s chassis: store-first
inlined form loses the anti-dep pinning benefit."
Probe: store-first variant measured 24 (245/248 — cross-jump returns).
Verdict: **CONFIRMED (store-last is required)** — banked as
rejected/case3-inlined-store-first.c.

Frontier: (empty — function at sandbox 0 with ALL THREE banned constructs
absent and only ONE FAKE construct (the [s3] s1 chain-extender, already
ruled legitimate by the 03:53 layer-1 review); remaining work is
integration: rule retirement via the normal retire path + layer-1/Judge
gates.)

## [s6] 2026-08-25 (recon, after the fourth layer-1 FAIL banned every load-side non-/s spelling for case 3)

H12 - "The original dependence edge came from the STORE side, not the loads:
D_80099478/D_8009947A are one aggregate (struct { s16 id; s16 variant; } =
g_stage_id/g_stage_variant), so the variant store is a /s COMPONENT_REF MEM
and sched.c's struct/non-struct exemption (anti_dependence, sched.c:855-863,
which requires the OTHER ref be non-struct) cannot dismiss the load->store
conflict - the anti-dependence edges form from real dependence analysis with
case 3 keeping case 13's exact ptr[-2]/ptr[-1] spelling."
Probe: TU-local struct probe, all 7 pair-refs respelled; measured + full
instruction diff (tmp/grind/func_800460E4/s6_ours2.dis).
Verdict: **CONFIRMED mechanically - every instruction matches target
(sandbox 4 = exactly the four variant-store reloc spellings D_80099478+2 vs
D_8009947A+0, which alias the same address at link; %hi/%lo arithmetic proven
identical to target's emitted halfwords). Sanction question (aggregate-merge
prong (a)) is open -> ruling-request.**

H13 - "Under the /s store, case 34 must be spelled store-LAST (s4 compute
then variant store), mirroring target's lw/li/sh order; store-first forces
the lw after the sh via true_dependence and cascades a whole-function
C-s2/C-s3 seat swap."
Probe: measured both orders (21 store-first vs 4 store-last).
Verdict: **CONFIRMED** - banked rejected/case34-store-first-under-struct.c.

H14 - "g_stage_variant qualifies for the legitimate-volatile-interrupt-touched
carve-out as the alternative store-side mechanism."
Probe: censused all 16 access sites across 8 functions - every consumer is a
synchronous stage-machine function; no IRQ handler touches the pair.
Verdict: **KILLED (two-prong gate unmet; no IRQ evidence).**

Frontier: (1) obtain the aggregate-merge ruling for the g_stage_id/g_stage_variant
struct (the ONLY open question between the current form and COMPLETED-C);
(2) if granted, complete the merge per prongs (b)-(e) (game.h canonical decl,
respell src/sound.c getters + this TU's other two functions) and resolve the
sandbox named-symbol-addend-alias artifact at the driver/operator level
(engine/score.py is session-forbidden); (3) if refused, the case-3 residual
returns to floor 9 with every measured mechanism banned or unmet - next
modality would need a genuinely new axis (none currently known).
[RESOLVED by the 2026-08-25 04:46 ruling: REFUSED - branch (3) is live.]

## [s7] 2026-08-25 (recon re-baseline after the 04:46 aggregate-merge refusal)

H15 - "The indexed rvalue spelling `s0[s3-2]`/`s0[s3-1]` (the function's own
first-switch idiom; ARRAY-index tree, /s preserved, NOT an alias-defeat)
produces a structurally different RTL (per-read address arithmetic, different
pseudo/LUID layout pre-cse) that could reorder sched1's li/sh placement in
case 3."
Probe: respelled case 3 (then case 13 too) as
`s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2])); s4 = ... s0[s3 - 1] ...;`
store last; measured; disassembly diffed against the block-local-ptr build.
Verdict: **KILLED as a lever - sandbox flat at 9 and the object is
BYTE-IDENTICAL to the ptr-spelled build** (cse canonicalizes both to one
shared (s3<<2)+s0 pseudo with -8/-4 displacements; tmp/grind/func_800460E4/
s7/ ours_s7.dis vs ours_s7_prev.dis, diff empty). Retained in candidate.c
purely as the more natural uniform spelling (byte-neutrality proven).

New forensic constraints banked (evidence.md [s7]): target's case-3 li
reuses the dead address register $v0, so target seats+order are ONE RIGID
SOLUTION (li-in-$v0 requires li after lw2); target's own case 34 has li/sh
inside the load-delay window, so no store barrier existed in the original;
target's case-3 atom multiset is identical to ours - divergence is
order+seats only.

Frontier (mechanism-grounded, in order):
(1) SOLVER modality at this chassis: model PASS 2 explicitly - post-reload,
    with target's seats, `li $v0,1` carries a genuine hard-reg
    anti-dependence on `lw $a0,-4($v0)` and cannot lift, so target order is
    sched2-self-consistent GIVEN target seats; the decidable technical
    question is whether any honest ATOM-SET change (not luid perturbation -
    that space is swept dead) lets sched1/local-alloc reach those seats.
    Run inverse_compose.py classify on the case-3 seat pair (address
    pseudo -> $v0, m1 pseudo -> $a0) at the floor-9 chassis for a typed
    REACHABLE/FORECLOSED verdict with ranked C-lever vectors.
(2) REDERIVE modality: whole-function fresh derivation (m2c + sibling
    idioms + Kengo naming cross-reference) hunting a structurally different
    global shape that changes the case-3 block's atom set or live-in state -
    the only space the perturbation sweep does not cover.
(3) If (1) returns FORECLOSED and (2) dies measured, the complete exhaustion
    chain (solver order-unreachability + every dependence-edge mechanism
    banned/refused/killed with citations) is escalation-packet material for
    a fidelity/routing question ONLY - the aggregate-merge refusal is final
    and any family re-ask is auto-reject-class.

## [s1] H15: the indexed rvalue spelling s0[s3-2]/s0[s3-1] (the function's own first-switch idiom, /s preserved, not an alias-defeat) yields structurally different RTL that could reorder sched1's li/sh placement in case 3
- mechanism: different tree derivation (per-read ARRAY-index address arithmetic) could produce different pseudo/LUID structure feeding sched1
- probe: respelled case 3 and case 13 to ALIGN4(s0[s3-2])/ALIGN4(s0[s3-1]); sandbox; objdump diff vs the block-local-ptr build
- result: flat at 9 and BYTE-IDENTICAL object (cse canonicalizes both to one shared (s3<<2)+s0 pseudo with -8/-4 displacements); adopted in candidate.c purely as the more natural uniform spelling
- verdict: KILLED

## [s1] Baseline validity: the [s6]-reported floor-9 non-banned baseline reproduces at the current chassis
- mechanism: chassis re-measurement per the dispatch warning (HEAD floor was 'measurement unavailable')
- probe: sandbox at HEAD rule-era body, then at the rebuilt [s5]-minus-banned-case-3 form
- result: HEAD = 35 (248/248, rules_dropped 10); floor-9 form = 9 (245/248), measured twice
- verdict: CONFIRMED

## [s2] 2026-08-25 (structural)

H16 - "A whole-function structural change (splitting the dual-purpose `s7`
local into a separate early-return flag and the `s7 = 7` channel id - the
spelling a human writing from spec would use) alters case 3's block entry
state and therefore its schedule/seats."
Probe: split spelled, measured, object disassembled and case 3 diffed against
the baseline build.
Verdict: **KILLED, twice over.** (a) As a candidate: sandbox 46 (240/248) vs
baseline 9 - the `s7` reuse is load-bearing whole-function (with the split the
first switch's flag lands in $s2, not target's $s7). (b) As a mechanism, and
this is the important half: case 3's emitted block is BYTE-IDENTICAL to the
baseline's under this drastic whole-function perturbation
(tmp/grind/func_800460E4/s2/pA.dis:130-141). Case-3 codegen does not see
whole-function shape. Banked rejected/s7-flag-split-whole-function.c.

H17 - "The one case-3 statement slot [s4]'s P1-P4 sweep never covered at the
mem/s chassis - the store `D_8009947A = 1;` placed FIRST, before both header
reads - reaches target's li/sh-after-both-loads order."
Probe: spelled store-first; measured; disassembled. Also measured `s1 = s2;`
moved to the END of case 3.
Verdict: **KILLED - 9/245 with a case-3 block BYTE-IDENTICAL to store-last**
(s2/pB.dis:132-143). Store position is inert: cse + sched1 canonicalize every
position onto li/lui/sh in the FIRST load's delay slot. Banked
rejected/case3-store-first-at-mem-s-chassis.c.

H18 - "The 3 insns jump2 find_cross_jump eats are an independent component of
the 9, so defeating the merge lowers the floor."
Probe: computed s4 (the -4 word) BEFORE s6 (the -8 word) in case 3, at all
three store positions (D/E/F); measured each; disassembled D.
Verdict: **KILLED - the merge is SCORE-NEUTRAL.** All three variants measure
sandbox 9 with build_insns 248 == target_insns: the merge is defeated (our
tail becomes `addu s6,s0,v0`, no longer identical to case 34's `addu
s4,s0,v0`) and the score does not move. This CORRECTS the [s4]/[s7] ledger
account of "9 = 3 merged insns + seat swap": all 9 are the seat/order
divergence; the fold merely relocated 3 already-wrong instructions. Not
adopted (zero score gain, and it introduces a case-3-vs-case-13
statement-order divergence motivated only by defeating a jump2 fold - the
smell the 04:28 layer-1 FAIL banned for the address-form choice). Banked
rejected/case3-assign-order-swap-defeats-crossjump.c.

**Modality verdict: STRUCTURAL is EXHAUSTED for this function.** Case 3's atom
multiset is fixed (= target's, [s7]), its live-in set is fixed, its schedule
is order-invariant (H17 + [s4] P1-P4 + sched_solver UNREACHABLE), and its
block is insensitive to whole-function shape (H16). The seats are a
consequence of the schedule, not an independent lever. The only remaining
input is the dependence graph, and every honest spelling that changes it is
banned (load side: 03:53/04:17/04:28) or refused (store side: 04:46) or
gate-failed (H14 volatile).

Frontier (unchanged in substance from [s7], now with structural eliminated):
(1) SOLVER modality - tools/ra_solver inverse_compose.py classify on the
    case-3 seat pair (address -> $v0, -4 word -> $a0) plus an explicit sched
    pass-2 model, for a typed REACHABLE/FORECLOSED verdict. This session's
    order-invariance measurements are strong prior evidence for FORECLOSED;
    a typed verdict is what an escalation packet would need.
(2) REDERIVE modality - a whole-function fresh derivation is now known NOT to
    reach case 3 by shape alone (H16); it is only worth running if it changes
    case 3's OWN statements (i.e. finds a different set of atoms), which
    [s7]'s atom-multiset identity argues against.
(3) If (1) returns FORECLOSED, the exhaustion chain is escalation-packet
    material for a fidelity/routing question ONLY (the aggregate-merge refusal
    is final; any family re-ask is auto-reject-class).

## [s2] A whole-function structural change (splitting the dual-purpose s7 local into a separate early-return flag and the s7=7 channel id - the spelling a human writing from spec would use) alters case 3's block entry state and therefore its schedule/seats.
- mechanism: different pseudo set / conflicts / live-in state entering block 19 -> different local-alloc seating even with unchanged block-local atoms (the [s7] frontier item 2 mechanism)
- probe: spelled the split; sandbox --disable all; objdump -M no-aliases of the sandbox object; diffed the emitted case-3 block against the baseline build (tmp/grind/func_800460E4/s2/pA.dis:130-141)
- result: sandbox 46 (240/248) vs baseline 9 (245/248) - the s7 reuse is load-bearing whole-function (the first switch's flag moves from target's $s7 to $s2). AND the emitted case-3 block is BYTE-IDENTICAL to the baseline's despite that drastic global change.
- verdict: KILLED

## [s2] The one case-3 statement slot the [s4] P1-P4 sweep never covered at the mem/s chassis - the store D_8009947A=1 placed FIRST, before both header reads - reaches target's li/sh-after-both-loads order.
- mechanism: sched1's input-stream order feeds the list scheduler's tie-breaks; a store-first stream might not be sunk past the loads
- probe: spelled store-first in case 3; sandbox; disassembled and diffed case 3 (tmp/grind/func_800460E4/s2/pB.dis:132-143). Also measured 's1 = s2;' relocated to the END of case 3.
- result: store-first = 9 (245/248) with a case-3 block BYTE-IDENTICAL to store-last; s1=s2-last = 9 (245/248). cse+sched1 canonicalize every position onto li/lui/sh in the FIRST load's delay slot.
- verdict: KILLED

## [s2] The 3 insns jump2 find_cross_jump eats (case 3's tail merged into case 34's) are an independent component of the 9, so defeating the merge lowers the floor.
- mechanism: find_cross_jump merges byte-identical block suffixes; making case 3's tail 'addu s6,s0,v0' instead of 'addu s4,s0,v0' removes the identity
- probe: computed s4 (the -4 header word) BEFORE s6 (the -8 word) in case 3, at all three store positions (probes D/E/F); measured each; disassembled D (tmp/grind/func_800460E4/s2/pD.dis:132-146)
- result: all three measure sandbox 9 with build_insns 248 == target_insns - the merge IS defeated and the score does not move at all. The 3-insn deletion is score-neutral; all 9 diffs are the case-3 seat/order divergence. Not adopted (zero gain, plus a case-3-vs-case-13 statement-order divergence motivated only by defeating a jump2 fold - the smell the 04:28 layer-1 FAIL banned for the address-form choice).
- verdict: KILLED
