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

## [s3] 2026-08-25 (structural)

H19 — "The pre-sched insn STREAM ORDER (LUID) is the lever: if the two header
loads sit adjacent in the stream, sched1 will leave them adjacent."
Probe: named the two words into block-local `w0`/`w1` in case 3, which makes
the .combine stream literally equal target's final order; measured; diffed the
object against baseline.
Verdict: **KILLED, in the strongest available form** — sandbox 9 and the
emitted object is BYTE-IDENTICAL to the baseline. Handing sched1 target's exact
order as input does not survive the pass. Banked
rejected/case3-both-loads-adjacent-in-stream-byte-identical.c.

H20 — "The whole 9 is one scheduler tie: at T-6 of block 19 the second load
(insn 319) and the first shift of the s6 chain (insn 322) are both lifted to
`max_priority` by sched.c `adjust_priority`/`birthing_insn_p`, and
`schedule_select` breaks the tie by `potential_hazard` in favour of the load.
`birthing_insn_p` requires `reg_n_sets == 1`, so holding the header words in
variables assigned in more than one place removes the boost and yields target's
order."
Probe: read the ready-list trace in the -da .sched dump (block 19, T-1..T-14);
then four spellings — P2 (scratch pair shared case 3 + case 13), P3 (only the
-4 word shared), P4 (second sites in the mainline + case 34), P6 (P2 plus
in-place `>>= 2; <<= 2;` on a `u32` scratch so the shift chain writes the
value's own register). Measured each; disassembled each.
Verdict: **CONFIRMED as the mechanism.** The model predicts all three outcomes,
including P3's swapped loads. P2/P4 reproduce target's case-3 first EIGHT
instructions byte-exactly (address seat, both load seats, li reusing the dead
address register, store placement). P6 brings case 3 to 6 differing
instructions (baseline 9), differing from target ONLY by the two scratch
variables' hard-register seats being swapped. Floors: P2 17, P3 22, P4 22,
P5 19, P6 18, P7 18 — all WORSE than the floor 9 overall, because the required
`reg_n_sets > 1` forces the value pseudo into global_alloc (one hard register
for the whole function) while target seats the same semantic value differently
in case 3 ($v1/$a0) than in case 13 ($v0/$v1) — proof that in target those
values are block-local single-set pseudos, which is precisely the condition
that GRANTS the boost. Not adopted; candidate.c unchanged at floor 9.

H21 — "Declaration order of the scratch pair flips their global-alloc seats."
Probe: `u32 hdr_m1, hdr_m2;` vs `u32 hdr_m2, hdr_m1;` at the P6 chassis.
Verdict: **KILLED — inert, sandbox 18 both ways.** Global-alloc seating here is
usage/priority-driven, not declaration-order-driven.

**Modality verdict: STRUCTURAL is exhausted for statement order, declaration
order, whole-function shape and variable-splitting — but this session moved the
residual from "seats and order diverge, mechanism unknown" to a single named,
dump-quoted scheduler tie with a proven lever and a proven reason the lever
cannot be spent under the current dependence graph.**

Frontier (in order):
(1) ATOM-SET modality inside block 19: the tie only exists because BOTH the
    second load and the s6 chain's first shift become ready at T-6 with the
    boost. Any honest change that makes one of the two chains not newly-ready
    at that cycle (a different-length chain, a different consumer) breaks the
    tie without touching the dependence graph. [s7]'s atom-multiset identity
    argues the final atoms are fixed, but it says nothing about atoms that are
    present at sched1 and folded away afterwards.
(2) `bb_live_regs` half of `birthing_insn_p`: the boost also requires the
    destination pseudo to be LIVE at the scheduling point. A spelling in which
    the -4 word's only consumer is scheduled such that its dest is not yet live
    at T-6 would defeat the boost while keeping the pseudo block-local and
    single-set — the one combination P2..P7 could not achieve.
(3) If (1) and (2) die measured, the exhaustion chain is complete at the
    instruction-selection level and the residual is a fidelity/routing question
    only (the aggregate-merge refusal is final; any family re-ask is
    auto-reject-class).

## [s3] The pre-sched insn stream order (LUID) is the lever: if the two case-3 header loads sit adjacent in the pre-sched stream, sched1 leaves them adjacent as target does.
- mechanism: rank_for_schedule falls through to INSN_LUID when priority and dependence class tie, so the input stream order is the final tie-break
- probe: Named the two header words into block-local w0/w1 in case 3, which makes the .combine RTL stream literally equal target's final instruction order (insns 298,303,305,309,319,321,322,324,326,327,329,334,336); sandbox --disable all; objdump diff vs baseline (tmp/grind/func_800460E4/s3/pP1.dis)
- result: sandbox 9 (245/248) and the emitted object is BYTE-IDENTICAL to the baseline - sched1 hoists li/lui/sh into lw(-8)'s delay slot regardless of being handed target's exact order
- verdict: KILLED

## [s3] The whole 9 is one scheduler tie: at T-6 of block 19 the second load (insn 319) and the first shift of the s6 chain (insn 322) both carry sched.c's adjust_priority/birthing_insn_p max_priority boost, and schedule_select's same-priority group rule breaks the tie by potential_hazard in favour of the memory-unit insn; birthing_insn_p requires reg_n_sets==1, so a header-word variable assigned in more than one place loses the boost and yields target's order and seats.
- mechanism: sched.c:2545-2593 adjust_priority -> birthing_insn_p (sched.c:2505, returns reg_n_sets[dest]==1 when the dest is live) lifts a newly-ready insn to max_priority; schedule_select then picks within the equal-priority group by potential_hazard, which is nonzero for the memory unit and zero for the ALU
- probe: Read GCC's own ready-list trace in tmp/grind/func_800460E4/dumps/text1a_c2.sched:368-407 (T-1..T-14 for block 19, including the literal dump line ';; insn 319 has a greater potential hazard'); then spelled and measured four boost-killing forms: P2 scratch pair shared by case 3 and case 13, P3 only the -4 word shared, P4 second set sites in the mainline and case 34, P6 = P2 with ALIGN4 spelled as in-place >>=2 / <<=2 updates on a u32 scratch; disassembled each
- result: The model predicts every outcome. P2 and P4 reproduce target's case-3 FIRST EIGHT instructions byte-exactly (sll/addu address in $v0, lw $v1,-8($v0), lw $a0,-4($v0), addiu $v0,1, lui, sh - including target's li reusing the then-dead address register). P3 (only one boost killed) swaps the two loads exactly as predicted. P6 brings case 3 to 6 differing instructions vs the baseline's 9, differing from target ONLY by the two scratch variables' hard-register seats being swapped. Overall floors: P2 17, P3 22, P4 22, P5 19, P6 18 - all worse than 9, because reg_n_sets>1 forces the value pseudo into global_alloc (one hard register function-wide) while target seats the same value in $v1/$a0 in case 3 and $v0/$v1 in case 13
- verdict: CONFIRMED

## [s3] Declaration order of the scratch pair flips their global-alloc seats, which is the last thing separating P6's case 3 from byte-exact.
- mechanism: global.c allocno ordering ties broken by allocno number, which follows declaration order
- probe: u32 hdr_m1, hdr_m2; vs u32 hdr_m2, hdr_m1; at the P6 chassis; sandbox --disable all
- result: sandbox 18 both ways - inert
- verdict: KILLED

## [s4] The [s3] frontier holds: an atom-set change inside case 3's block that makes one of the two competing insns NOT newly-ready at reverse-cycle T-6 breaks the birthing-boost tie and yields target's order AND all of its seats.
- mechanism: sched.c adjust_priority's birthing boost (0x7f000001) is granted per newly-ready insn; the divergence exists only because the second header load and the first shift of the other ALIGN4 chain become ready in the SAME cycle. Desynchronising the two chains removes the tie entirely rather than trying to win it.
- probe: decomp-permuter campaign perm_a (seed = inherited floor-9 candidate, 41334 iterations) produced output-110-1, whose case 3 is semantically identical to the baseline but routes the -4 header word through `nv = &s0[s3 - 1]`; reproduced by hand as q_a_repro, measured with `sandbox func_800460E4 --disable all`, and both spellings' cc1 `-dS` .sched dumps compared block-for-block (tmp/grind/func_800460E4/s4/dumps_bs vs dumps_qa, ready-list group 19)
- result: floor-9 spelling ready list at T-6 = `332/331 (1) 298 (1) 310 (7f000001) 322 (7f000001)` (two boosted insns, tie); winning spelling = `332 (1) 298 (1) 310 (7f000001)` (one boosted insn, no tie). The object reaches target's exact case-3 order and every seat, including `li v0,1` reusing the dead address register after both loads. sandbox --disable all = 2 (249/248) - the only residual is GCC materialising the `&s0[s3-1]` base a second time (`addu v1,v0,s0`) plus that load's seat.
- verdict: CONFIRMED

## [s4] The duplicated base can be removed while keeping the desynchronisation, by reading the header word through a byte-offset cast-deref off the same address expression instead of through a second pointer object.
- mechanism: `*(s32 *)((s32)&s0[s3] - 4)` yields one address pseudo (both loads at -8/-4 off it, exactly target's 15-atom block) while still not being an array-ref MEM off the same base as the other read, so the two ALIGN4 chains stay desynchronised
- probe: directed sweep tmp/grind/func_800460E4/s4/probe2.py + probe3.py (13 spellings of the two header-word reads, scored against target with tmp/grind/func_800460E4/s4/score.py), then the winner re-measured in src/text1a_c2.c with `sandbox func_800460E4 --disable all`
- result: **sandbox --disable all = 0 (248/248, rules_dropped=10, cheat_asm_stripped=0)**. Applying the same byte-offset read to BOTH header words at BOTH sites (case 3 and case 13) also measures 0, and that fully-uniform form is what candidate.c carries.
- verdict: CONFIRMED (bytes) — LICENCE OPEN: the construct respells state.json banned_constructs #5, so the session returned `ruling-request` rather than `candidate-ready`

## [s4] Pointer-local spellings of the same address (`p = &s0[s3]; p[-2]; p[-1]`) do NOT desynchronise the chains.
- mechanism: an array-ref MEM off a pointer local produces the same dependence/readiness structure as the direct `s0[s3-2]` index, so the T-6 tie survives
- probe: q_d / q_g / r1 / r2 / r4 in probe2.py / probe3.py (pointer local at &s0[s3], at &s0[s3-1], at &s0[s3-2], with the two reads distributed every way), diffed against target
- result: 245-246 instructions, 9 diffs - identical to the floor-9 baseline in every case
- verdict: KILLED (and this is the strongest counter-argument for the ruling: the same address written as a pointer local closes nothing, so the winning construct's only observable difference is codegen)

## [s4] Re-indexing case 3 through a decremented s3 (`s3 -= 2; s0[s3]; s0[s3+1]`) is catastrophic, not neutral.
- mechanism: s3 is live into the post-switch region's register allocation; clobbering it re-seats s1/s2/s3 across the whole tail
- probe: p1_s3minus2 / p2_s3minus2_s4first in probe.py
- result: 246 / 249 instructions, 135 / 139 diffs
- verdict: KILLED

## [s4] Permuter-seed calibration for this function (methodology, not a codegen claim).
- mechanism: the permuter's weighted metric charges 100 per inserted/deleted instruction, so the jump2 cross-jump merge in the floor-9 candidate costs 300 points that carry no information
- probe: launched the same search from the floor-9 candidate (perm_a) and from its merge-free twin with case 3's s4 assigned before s6 (perm_b, also sandbox 9)
- result: base scores 550 vs 260 for two bodies at the same honest distance. Seed the merge-free twin in future campaigns on this function.
- verdict: CONFIRMED


## [s4b] 2026-08-25 (permuter)

- H-s4b-1 - the 06:39 ruling's open route (byte-neutral, once-written fresh named
  intermediate holding a real consumed value/address) can close case 3.
  Mechanism: named-intermediate prongs; a real consumed value in the target's bytes.
  Probe: probe4 (8 value-local spellings), probe5 (7 pointer-local spellings incl.
  hdr = &s0[s3] at both source orders and both scopes), probe9 k4.
  Result: all 22 measure diffs=9; 248-insn byte-neutral members exist and are still 9.
  Verdict: KILLED.

- H-s4b-2 - the birthing boost can be cleared by making the carrier multiply assigned,
  and that alone closes the block.
  Mechanism: sched.c adjust_priority -> birthing_insn_p boosts only pseudos with
  reg_n_sets[regno] == 1; writing a carrier twice removes the boost, so only ONE boosted
  insn is ready at block 19's reverse-cycle T-6 and the tie disappears.
  Probe: permuter perm_d output-95-1 (found it), then probe6 x2 (one chain split: 7),
  probe7 y3 (both chains split: 0), probe9 k1/k2/k3/k6 (0), probe6 x1/x3/x4/x6/x7/x8
  (same splits with once-written carriers: 9).
  Result: both carriers must be twice-written; then sandbox --disable all = 0 (248/248).
  Verdict: CONFIRMED.

- H-s4b-3 - the multiply-assigned carrier can be an EXISTING local (staged-value-reused-
  variable bound 2) rather than a fresh one.
  Mechanism: same reg_n_sets mechanism; s6/s4 are overwritten here so their old values are
  dead, s3 dies after the two loads.
  Probe: probe10 b1/b2/b4 (s6/s4 as their own carriers), b3 (s3 + one value local).
  Result: 246 insns diffs=8, and 248 insns diffs=40. No existing-local borrow reaches 0.
  Verdict: KILLED - this is why the closing form needs a ruling rather than a family claim.

- H-s4b-4 - the staged spelling can be applied uniformly at case 13 to remove the
  two-spellings-in-one-function divergence.
  Probe: probe8 z3 / z4.
  Result: case 13 goes from matching to diffs=13 (44 with the FAKE chain also dropped).
  Verdict: KILLED - the divergence is byte-forced.

- H-s4b-5 - the inherited FAKE s1 chain-extender is now redundant under the new case-3 form.
  Probe: probe8 z2 (replace with plain s1 = s4;).
  Result: 0 -> 32 diffs. Verdict: KILLED (it stays load-bearing).


## [s5] 2026-08-25 (permuter)

H22 - "The birthing-boost mechanism (H20 / H-s4b-2) can be spent with carriers
that are PRE-EXISTING locals - the route the 2026-08-25 07:19 ruling left open
when it refused the fresh twice-written carrier - because the prior session's
carrier census was incomplete: it tried only the destination pointers (s6/s4)
and the two live pointers (s3/s2), never the function's mainline scratch OFFSET
locals."
Mechanism: sched.c adjust_priority -> birthing_insn_p boosts only pseudos with
reg_n_sets[regno] == 1; a local that is already assigned elsewhere in the
function carries the multi-set property WITHOUT any invented extra write, and
(unlike s6/s4/s3/s2) an offset temp that is dead across every call can be
seated by global_alloc in the caller-saved registers target uses ($v1/$a0).
Probe: directed sweeps tmp/grind/func_800460E4/s4/s5a.py (7 borrow spellings),
s5b.py (5 in-place variants), s5c.py/s5d.py (10 refinement spellings), s5g.py
(7 carrier-choice x spelling-uniformity variants), each scored against target
with s4/score.py; the winner re-measured with `sandbox --disable all`.
Result: **sandbox 0 (248/248)**. The closing form (s5g j4) borrows off_a (the
mainline stage-block offset, value s0[1], already consumed into s6) for the -8
header word and off_b (the early switch's sub-block offset) for the -4 word,
refining each in place. Intermediate data points: borrowing s6/s4 themselves =
8 diffs at 246 insns (callee-saved carriers, shift chain runs through $v0);
borrowing the mainline s2-offset local = 22; borrowing the mainline off3 local
= 2 (case 3 exact, off3's own site refines through a second register) and 8 when
that site is split too (every instruction's shape correct, carrier lands in $a1
where target uses $a0). Verdict: **CONFIRMED**.

H23 - "The in-place refinement (x = x >> 2; x = x << 2) is required at the
carrier's OTHER site too, not just in case 3."
Mechanism: local-alloc quantity tying - writing the refinement back into the
same pseudo lets both shifts take the carrier's own hard register, which is what
target does at every ALIGN4-into-argument site (`lw a0,4(a0); srl a0,a0,2;
sll a0,a0,2`); a one-expression ALIGN4 there refines through a second register.
Probe: s5g j3/j4 (early site staged) vs j5/j6 (early site flat).
Result: staged = 0, flat = 3 (the three instructions of that site's shift
chain). Verdict: **CONFIRMED**.

H24 - "Hoisting every local declaration to the top of the function (C89/PsyQ
house style) is byte-neutral, so the borrow does not depend on a targeted
scope widening."
Probe: hoisted off3/a0_ptr/the s2-offset temp as well (off_c/off_d/a0_ptr) and
re-measured. Result: sandbox 0 before and after. Verdict: **CONFIRMED**.

H25 - "A permuter campaign seeded on the closest known non-banned chassis (the
s6/s4-staged 246-insn body, 8 diffs) finds the closing form."
Probe: campaign perm_e, label s5-b1-staged-s6s4-246, base score 435, 6 jobs,
16658 iterations, harvested and stopped in-session.
Result: 20+ finds, best new score 80, NO zero and nothing structurally novel -
the closing form came from the directed carrier sweep instead. Verdict:
**KILLED as the productive route for this residual** (banked as the modality's
data point: this basin does not yield to random search, it yields to enumerating
which pre-existing local is borrowed).

Frontier: (empty at the C level - the function measures sandbox 0 with zero
banned constructs and every construct inside a sanctioned family. Remaining work
is acceptance + integration: layer-1/Judge review of the staged-value-reused-
variable claim, then retirement of the 10 regfix rules via the normal retire
path and a full-build SHA1 verify, which are operator/driver surfaces.)

## [s6] 2026-08-25 (permuter modality — re-measure + resubmission)

H26 — "The s5 closing form still measures 0 at the current chassis, i.e. the
discard was a self-vet WORDING failure and not a C or chassis problem."
Mechanism: the driver's pre-Judge tripwire (`grindlib._ban_trips`) matches a
banned entry's content words as substrings inside the vet's `CONSTRUCTS:` block
only; a positive declaration that happens to reuse the banned entry's domain
vocabulary trips it even when the declared construct is a different one.
Probe: re-applied memory/grind/func_800460E4/candidate.c to src/text1a_c2.c from
a clean HEAD tree; `sandbox func_800460E4 --disable all` twice; then ran
`grindlib.check_banned_constructs` directly against the rewritten vet and printed
the per-entry hit lists.
Result: HEAD body = 35; candidate body = **0 (248/248, rules_dropped=10,
cheat_asm_stripped=0)**, both measurements reproduced. Ban check now returns
`(True, '')` with per-entry hits of 1/2/6/0/0/1 content words (entry #3's
threshold is 20, so 6 is far under). All four PRECEDENT paths resolve to the
claimed text.
Verdict: **CONFIRMED.**

Frontier: (empty at the C level — the function measures sandbox 0 with zero
banned constructs, every construct inside a sanctioned family with a resolving
precedent, and a format-valid self-vet. Remaining work is acceptance +
integration: layer-1 review of the staged-value-reused-variable claim (the one
live question is whether hoisting the borrowed temps out of inner braces
disqualifies bound 2 — disclosed, not hidden), then the Judge, then retirement
of the 10 regfix rules via the normal retire path with a full-build SHA1 verify,
which are operator/driver surfaces this session may not touch.)
