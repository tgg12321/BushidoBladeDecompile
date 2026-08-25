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

## [s8] 2026-08-25 (SYNTHESIS modality — merged attack + frontier reset)

Chassis re-established first: the 07:54 layer-1 FAIL banned the [s5]/[s6]
candidate, so the submittable body is the [s7] non-banned form. Re-measured
this session: `sandbox func_800460E4 --disable all` = **9** (245/248,
rules_dropped=10), twice, and again at end of session. candidate.c now carries
that body (the banned 0-scoring form stays banked at
rejected/layer1-fail-0825-0754.c).

H27 — "The `bb_live_regs` half of `birthing_insn_p` ([s3] frontier item 2) can
be defeated by some C spelling, killing the boost while the carrier stays a
block-local single-set pseudo."
Mechanism as filed: sched.c:2505 `birthing_insn_p` returns 0 outright when the
SET_DEST pseudo is not live in `bb_live_regs` at the scheduling point,
independently of `reg_n_sets`.
Probe: read the function verbatim (tools/gcc-2.7.2/sched.c:2504-2537) and
checked it against the block-19 ready-list traces of two different chassis
(tmp/grind/func_800460E4/s8/base.sched and s8/vC.sched — every 0x7f000001 entry
in both).
Verdict: **KILLED — structurally unreachable.** The list scheduler runs
BACKWARD: an insn only becomes ready once all of its block-local consumers are
already scheduled, and scheduling a consumer is exactly what puts the consumed
pseudo INTO `bb_live_regs`. So for any ready insn whose pattern is
`SET (REG, ...)`, the liveness test is satisfied by construction and
`birthing_insn_p` degenerates to `reg_n_sets[dest] == 1`. The only patterns
that fail the test are ones whose SET_DEST is not a REG (a store, a SUBREG),
which is not a spelling choice for a loaded value. Frontier item (2) is closed:
there is no liveness lever, only the reg_n_sets lever, and the reg_n_sets lever
is what the 07:19 ruling and the 07:54 layer-1 FAIL close.

H28 — "[s3] frontier item (1) is real: an atom-set change INSIDE block 19 that
leaves the final atom multiset unchanged can flip the T-6 decision and give
target's adjacent loads, with NO multiply-assigned carrier anywhere."
Mechanism: sched1 sees the pre-reload insn stream. Extra insns that exist at
sched1 shift each chain's ready cycle and the block's unit-hazard state, so
`schedule_select`'s tie-break between the two 0x7f000001 insns resolves the
other way — the dependence graph, the alias behaviour and reg_n_sets are all
untouched.
Probe: 38 spellings measured through tmp/grind/func_800460E4/s4b2/pr.sh (exact
cpp|cc1|prologue_fix|maspsx|multu_pad pipeline, scored positionally against
target with s4/score.py): a 24-point full cross of {load order} x {detour
completion order} x {consumption order} x {store position}, plus 14 rebase-
detour shapes (`- (s32)s0` / `+ (s32)s0`, the reverse sign, `^ (s32)s0` twice,
`- s3` / `+ s3`, `+ 4` / `- 4`, one-word and two-word variants). cc1 `-da`
dumps taken for the baseline and for the winning shape.
Verdict: **CONFIRMED.** With a value-neutral rebase detour on BOTH header words
the two loads issue back to back exactly as target does, and both header values
land in target's exact hard registers (`lw a0,-4(v1)` / `lw v1,-8(v1)`: the -8
word in $v1, the -4 word in $a0). This is the first non-banned construct in
eight sessions to reach target's load adjacency. Dump evidence for the
mechanism: baseline block 19 T-6 ready list is
`331 (1) 298 (1) 310 (7f000001) 322 (7f000001)` and the scheduler takes 322;
under the detour the same cycle reads
`346 (1) 298 (1) 332 (7f000001) 329 (7f000001)` followed by GCC's own line
`;; insn 329 has a greater potential hazard, now 329 332 346 298` — the boost
is NOT removed (both insns still carry 0x7f000001, i.e. reg_n_sets is still 1
for both dests), the tie simply resolves the other way because the detour's
insns changed the cycle at which each chain became ready. This CORRECTS the
guess in the [s3] frontier that the only way to influence T-6 is to remove a
boost.
Scores (positional diffs / insn count, target = 248): baseline 9/245;
`-s0/+s0` detour both words, s6-then-s4 consumption, store last 10/246;
same with s4-then-s6 consumption 10/248; `^s0` detour, s4-then-s6, store last
10/248; same, store between 10/248; `+4/-4` detour 9/248 (the constant folds
completely, so no atoms reach sched1 and the schedule reverts to baseline —
the clean control that proves the effect comes from atoms surviving into
sched1, not from the source text); `-s3/+s3` 40/246 (clobbers s3's seat).

H29 — "Load adjacency is sufficient to close case 3."
Probe: the winning detour forms above, diffed instruction by instruction
against target.
Verdict: **KILLED.** With adjacency achieved and both header values in
target's registers, the residual is 10, not 0, and is exactly two things:
(a) the ADDRESS pseudo is seated $v1 where target uses $v0 (which also forces
our pair to load -4 before -8, because $v1 is simultaneously the -8 word's
seat), and (b) each ALIGN4 shift chain refines through $v0 instead of refining
IN PLACE in the value's own register the way target does
(`srl v1,v1,0x2; sll v1,v1,0x2` / `srl a0,a0,0x2; sll a0,a0,0x2`). (b) is
[s5] H23 restated: in-place refinement is the second, INDEPENDENT requirement,
and the only spelling known to produce it is a carrier written more than once.

**Modality verdict: SYNTHESIS complete.** The residual is now fully decomposed
into two independent requirements with separate levers, one of which is newly
open:
  (A) load adjacency  — reachable by a sched1 atom-set change (H28, measured),
      no multi-set carrier needed, no alias analysis touched;
  (B) in-place ALIGN4 refinement + the address pseudo in $v0 — only ever
      achieved so far by a multiply-assigned carrier (banned/refused), so this
      is the open half.
Every previously-open frontier item is now resolved: item (2) killed (H27),
item (1) confirmed and half-spent (H28/H29).

Frontier (in order, for the next ladder pass):
(1) SOLVER modality on requirement (B) ONLY, at the H28 chassis (the
    adjacency-achieved 248-insn body, 10 diffs, seats otherwise correct):
    run tools/ra_solver inverse_compose.py classify on the two remaining seat
    facts — address pseudo -> $v0, and each ALIGN4 result pseudo -> its own
    input register — for a typed REACHABLE/FORECLOSED verdict with ranked
    C-lever vectors. This is a much smaller and much better-posed question than
    any solver run this function has had before, because everything else in the
    block is already byte-exact.
(2) ATOM-SET modality continued on requirement (A) with an HONEST spelling:
    the rebase detours that produce adjacency are research forms only (T1/T2
    failures — no semantic purpose, no human would write them). The open
    question is whether any spelling a human WOULD write leaves an extra insn
    in block 19 at sched1 that disappears before final: candidates not yet
    swept are a genuinely used second consumer of one header word that
    cross-jump or reload later removes, and sub-expression spellings whose
    extra atoms are copies that reload coalesces.
(3) Only if (1) returns FORECLOSED for the address/in-place seats AND (2) finds
    no honest adjacency spelling is the instruction-selection level exhausted;
    the residual would then be a fidelity/routing question for an escalation
    packet. Do NOT re-ask any family question: the aggregate merge (04:46), the
    fresh multi-set carrier (07:19) and the hoisted pre-existing carrier
    (07:54) are all closed, and re-asking is auto-reject class.

## [s4] The bb_live_regs half of sched.c birthing_insn_p ([s3] frontier item 2) can be defeated by a C spelling, killing the boost while the carrier stays a block-local single-set pseudo.
- mechanism: sched.c:2504-2537 returns 0 outright when the SET_DEST pseudo is not live in bb_live_regs at the scheduling point, independently of reg_n_sets
- probe: Read birthing_insn_p verbatim in tools/gcc-2.7.2/sched.c and checked it against every 0x7f000001 ready-list entry in two chassis block-19 traces (tmp/grind/func_800460E4/s8/base.sched, s8/vC.sched)
- result: The list scheduler runs BACKWARD: an insn only becomes ready once all its block-local consumers are scheduled, and scheduling a consumer is exactly what puts the consumed pseudo INTO bb_live_regs. So for any ready insn with a SET(REG,...) pattern the liveness test holds by construction and birthing_insn_p degenerates to reg_n_sets[dest]==1. Only non-REG SET_DESTs (store, SUBREG) fail it, which is not a spelling choice for a loaded word.
- verdict: KILLED

## [s4] [s3] frontier item (1) is real: an atom-set change inside block 19 that leaves the final atom multiset unchanged can flip the T-6 tie and give target adjacent loads, with no multiply-assigned carrier anywhere and no change to alias analysis or the dependence graph.
- mechanism: sched1 sees the pre-reload stream; extra insns shift each chain ready cycle and the block unit-hazard state, so schedule_select tie-break between the two 0x7f000001 insns resolves the other way
- probe: 38 spellings through the new fast harness tmp/grind/func_800460E4/s8/pr.sh (exact cpp|cc1|prologue_fix|maspsx|multu_pad pipeline, scored positionally against target): a 24-point cross of load order x detour-completion order x consumption order x store position, plus 14 rebase-detour shapes; cc1 -da dumps for the baseline and the winning shape
- result: Value-neutral rebase detours on both header words put target two loads back to back AND give both header values target exact hard registers (-8 word in $v1, -4 word in $a0) - the first non-banned construct in eight sessions to reach adjacency. Dumps: baseline T-6 ready list '331 (1) 298 (1) 310 (7f000001) 322 (7f000001), now 322 310 ...' vs detour '346 (1) 298 (1) 332 (7f000001) 329 (7f000001)' plus GCC own line ';; insn 329 has a greater potential hazard, now 329 332 346 298'. The boost is NOT removed (both still 0x7f000001) - the tie simply resolves differently, which corrects the [s3] frontier assumption that only boost-removal can move T-6.
- verdict: CONFIRMED

## [s4] Load adjacency is sufficient to close case 3.
- mechanism: if the T-6 decision was the whole residual, fixing it should reach 0
- probe: instruction-by-instruction diff of the adjacency-achieving forms against target (tmp/grind/func_800460E4/s8/x2.dis, s8/vC.dis)
- result: Residual is 10, not 0, and is exactly two facts outside the tie: (a) the address pseudo is seated $v1 where target uses $v0 (and because $v1 is also the -8 word seat, our pair is forced to load -4 first), and (b) each ALIGN4 shift chain refines through $v0 instead of in place in the value own register as target does. (b) is [s5] H23 restated: in-place refinement is an INDEPENDENT requirement whose only known spelling is a carrier written more than once.
- verdict: KILLED

## [s4] The rebase-detour effect comes from the source text rather than from atoms surviving into sched1.
- mechanism: control for the atom-set hypothesis - a constant rebase (+4/-4) folds completely before sched1
- probe: the +4/-4 variant measured through the same harness and its case-3 block diffed against the baseline
- result: 9/248 with a case-3 schedule identical to the baseline, while the -s0/+s0 and ^s0 rebases measure 10/246 and 10/248 with adjacency. Corollary: combine runs before sched1 (toplev.c:3004 vs :3033), so any construct whose whole effect combine folds cannot influence this tie; only constructs whose extra atoms survive combine and are removed later (reload coalescing, jump2) can be both effective and byte-neutral.
- verdict: CONFIRMED



## [s9] 2026-08-25 — SOLVER modality

H26 (KILLED, and it kills the axis it came from). "Requirement (B) — the address
pseudo to $v0 plus in-place ALIGN4 refinement — is an INDEPENDENT requirement
decidable by the RA solver." FALSE. Probe: `local_extract.py text1a_c2` at the
[s7] candidate chassis + hand replay of the validated `qty_compare` /
`find_free_reg` arithmetic on TARGET's block-19 instruction order
(tmp/grind/func_800460E4/s5b/block19_localalloc.md). The model reproduces OUR
seats and TARGET's seats exactly, and target's seats fall out of target's ORDER
alone: adjacency shrinks the address quantity's live span from 7 insns to 3,
lifting its priority 7142 -> 16666 so it ranks first and takes $v0; $v1 and $a0
then follow mechanically. There is no separate seat question and no need for a
multiply-assigned carrier — the goal the 07:19 ruling and the 07:54 layer-1 FAIL
were both chasing does not exist.

H27 (KILLED). "There is an honest C spelling whose extra block-19 atoms survive
combine into sched1, buy adjacency, and are removed afterwards byte-neutrally."
FORECLOSED BY CONSTRUCTION. Probe: `local_extract.py` re-run with the vC
xor-rebase body in src. Block 19's address quantity still carries refs5 / span 14
(pri 7142, allocated last, seated $v1) despite the loads being adjacent. Extra
atoms that stay live across the two loads necessarily keep the ADDRESS live
across them too, so no member of that family can win the qty_compare race it
must win. [s8] frontier item 1 is closed; do not sweep it again.

H28 (CONFIRMED). "The entire remaining residual is the MEM_IN_STRUCT_P bit on the
two case-3 header-word loads." Mechanism: `expr.c:4567-4577` sets /s on an
INDIRECT_REF iff its address subtree is a PLUS_EXPR; `sched.c:831-839`
true_dependence then proves a varying /s load disjoint from the non-varying,
non-/s store to `D_8009947A`, so sched1 is free to hoist `li 1 / sh` between the
loads. Probe: the pointer-variable spelling (banned #4, research measurement
only) measures 248 insns / 0 diffs at this chassis — nothing else in the function
is wrong.

H29 (CONFIRMED, enumeration). "The set of C spellings that yield a non-/s load is
(a) plain pointer variable, (b) integer/pointer-cast byte-offset deref,
(c) aggregate COMPONENT_REF — and all three are already banned for this
function." (a) = banned #4/#5; (b) = the 06:39 ruling + the standing "no inlined
integer-cast byte-offset derefs in any form" constraint; (c) sets /s
unconditionally (expr.c:4888) and is closed by the 04:46 aggregate-merge ruling.
No fourth spelling exists in GCC 2.7.2's INDIRECT_REF expansion.

H30 (open — the only remaining shape of a question). The residual is no longer a
search problem or a family-sanction problem; it is a FIDELITY question about the
original source at one site, with a mechanically enumerated answer space. The
next modality that can move it is `escalation`, whose packet asks a
fidelity/routing question (what the original C at case 3 was, and whether the
already-banned pointer-variable spelling is the honest reconstruction of it
rather than an alias-defeat trick) — NOT a family-sanction or standard-lowering
question, which is auto-reject class. Nothing in solver / permuter / structural /
synthesis space remains untried.

## [s5] [s8] requirement (B) - the address pseudo to $v0 plus in-place ALIGN4 refinement - is an INDEPENDENT requirement, decidable by the RA solver, and only ever produced by a multiply-assigned carrier.
- mechanism: local_alloc (block_alloc) qty_compare priority floor_log2(refs)*refs*size/(death-birth)*10000 with tie to lower qty, then ascending find_free_reg over regs live in [birth,death).
- probe: tools/ra_solver/local_extract.py text1a_c2 at the [s7] candidate chassis (block 19 = case 3), plus a hand replay of the same validated arithmetic on TARGET's block-19 emission order; table banked at tmp/grind/func_800460E4/s5b/block19_localalloc.md.
- result: The model reproduces OUR seats exactly (address refs5 span14 pri 7142 ranks LAST -> $a0; -4 word refs6 span8 pri 15000 ranks FIRST -> $v0) and reproduces TARGET's seats exactly from TARGET's order alone (address refs5 span6 pri 16666 ranks FIRST -> $v0; -8 word -> $v1; li 1 -> $v0; -4 word span18 -> $a0). Adjacency of the two loads shortens the address quantity from 7 insns to 3 and is by itself sufficient for every seat in the block.
- verdict: KILLED

## [s5] There is an honest C spelling whose extra block-19 atoms survive combine into sched1, buy load adjacency, and are removed afterwards byte-neutrally by reload coalescing or jump2 ([s8] frontier item 1).
- mechanism: atoms present at sched1 shift ready cycles and flip schedule_select's tie-break without touching the dependence graph or reg_n_sets.
- probe: local_extract.py re-run with the vC xor-rebase body (rejected/s8-xor-rebase-detour-248-10-seats-right-address-v1.c) installed in src/text1a_c2.c; read block 19's quantity rows.
- result: FORECLOSED BY CONSTRUCTION, not by search. On the adjacency-achieved detour chassis the address quantity STILL carries refs5 / span 14 (pri 7142, allocated last, seated $v1). Any construct whose extra atoms stay live across the two loads necessarily keeps the ADDRESS live across them too, so it can never win the qty_compare race it must win. This is the mechanical explanation of [s8.4]'s '$v1 where target has $v0' and it closes the entire rebase/atom-set family.
- verdict: KILLED

## [s5] The entire remaining 9-instruction residual is the MEM_IN_STRUCT_P (/s) flag on the two case-3 header-word loads.
- mechanism: tools/gcc-2.7.2/expr.c:4567-4577 sets MEM_IN_STRUCT_P on an INDIRECT_REF iff its address subtree is a PLUS_EXPR ('If address was computed by addition, mark this as an element of an aggregate') - a purely syntactic GCC 2.7.2 property, nothing to do with structs. tools/gcc-2.7.2/sched.c:831-839 true_dependence then suppresses the conflict between a varying /s load and the non-varying non-/s store to D_8009947A, so sched1 is free to hoist li 1 / sh between the two loads; target's original carried the edge and could not.
- probe: Read both compiler sources; then measure the pointer-variable spelling (banned construct #4, research measurement only) through tmp/grind/func_800460E4/s8/pr.sh at the current chassis.
- result: 248 insns, 0 diffs (tmp/grind/func_800460E4/s4b2/s5pm2.dis). Nothing else in this function is wrong: the honest floor of 9 and the single /s bit are the same fact. Banked as research only at rejected/s9-research-only-banned-pm-ptr-form-measures-0-at-s7-chassis.c - NOT proposed.
- verdict: CONFIRMED

## [s5] The set of C spellings that produce a non-/s load of those two words is exactly {plain pointer variable, integer/pointer-cast byte-offset deref, aggregate COMPONENT_REF}, and all three are already banned for this function.
- mechanism: expr.c:4567-4577 enumerates the /s condition (PLUS_EXPR address subtree, SAVE_EXPR of one, or aggregate type); expr.c:4888 sets /s unconditionally for COMPONENT_REF.
- probe: Source enumeration against the function's banned_constructs list and standing judge constraints.
- result: (a) pointer variable = banned_constructs #4/#5; (b) integer/pointer-cast byte-offset deref = the 06:39 ruling plus the standing constraint 'do not respell the header-word reads as inlined integer-cast byte-offset derefs in any form'; (c) aggregate = the 04:46 aggregate-merge ruling. There is no fourth spelling in GCC 2.7.2's INDIRECT_REF expansion.
- verdict: CONFIRMED

## [s5] inverse_compose.py classify can be run directly on this function to triage the residual.
- mechanism: mkasm_honest.sh derives <stem>.tgt.s by applying regfix/regfix_stage2/asmfix to whatever is currently in src/.
- probe: Ran classify with the candidate body in src (fictional target), then re-ran with .tgt.s rebuilt from HEAD's rule-era src and .hon.s from the candidate.
- result: The naive run reports a confident RA verdict describing a clean $17/$18/$19 three-cycle rotation that DOES NOT EXIST - the 10 regfix rules re-apply on top of a body whose registers are already right. With the target pinned correctly the verdict is PRE-RA (honest 237 vs target 240), and the 3-insn gap is jump2 cross-jumping, which the classifier's funnel does not model. Procedure banked in evidence.md [s9.1] for every future solver session on a rule-carrying function.
- verdict: CONFIRMED

## [s6] The /s + sched1 account of the residual is an RTL fact, not an inference, and it can be read directly out of the dependence lists.
- mechanism: expr.c:4567-4577 sets MEM_IN_STRUCT_P at expand time from the tree shape; sched.c:831-839 (true_dependence / anti_dependence) then exempts a varying /s SImode MEM against a non-varying non-/s MEM, so sched1 has no edge to respect between the case-3 header loads and the D_8009947A store.
- probe: cc1 -O2 -da on the candidate body (tmp/grind/func_800460E4/s6/dumps_cand/); read t.flow for the MEM flags and t.sched for the emitted order plus each insn's dependence insn_list.
- result: t.flow shows insn 308 = (mem/s:SI (plus (reg 138) (const_int -8))), insn 322 = (mem/s:SI (plus (reg 138) (const_int -4))), insn 331 = (set (mem:HI (symbol_ref "D_8009947A")) ...) with no /s. t.sched emits 308 -> 329 (li 1) -> 331 (sh) -> 309 -> 310 -> 322, and insn 322's dependence list names ONLY insn 304 (its address) — the memory edge is visibly absent. Pass attribution final: born in expr.c expand, taken by sched1.
- verdict: CONFIRMED

## [s6] Only ONE of the two case-3 header reads has to lose /s — the -4 (second) one. The pair was never the unit.
- mechanism: the store follows both loads in source order, so it is anti-dependent on whichever loads it is not exempt from. Blocking it against the LAST load alone already keeps it out of the inter-load gap; exempting or not exempting the FIRST load changes nothing about where it can go.
- probe: two one-sided variants at the current chassis through s8/pr.sh — (A) -4 read via a pointer variable with -8 left as ptr[-2]; (B) -8 via a pointer variable with -4 left as ptr[-1].
- result: (A) 248 insns / 0 diffs — the whole function closes. (B) 245 insns / 9 diffs — completely inert. Banked research-only (banned #4 family) at rejected/s6-research-only-banned-only-m1-ptr-248-0.c and rejected/s6-research-only-only-m2-ptr-245-9-inert.c. Any future fidelity question is about ONE lvalue, not a two-read construct.
- verdict: CONFIRMED

## [s6] The D_80099478/D_8009947A aggregate model is not merely unproven — the shipped binary affirmatively contradicts it.
- mechanism: prong (a) of the 2026-08-17 aggregate-merge rule requires base-register or stride evidence independent of the byte chase. If the two halfwords were members of one aggregate, GCC would materialize the base once and offset the second access.
- probe: survey every access to both symbols across asm/funcs/*.s (7 functions, 12 accesses).
- result: all 12 accesses use their own independent lui %hi / %lo pair. func_8004668C stores to BOTH symbols back to back and emits a SECOND lui $at for the second store rather than reusing the first base with a +2 offset. No base-register or stride evidence exists anywhere in the binary. Prong (a) fails on evidence, not only on procedure. Artifact: tmp/grind/func_800460E4/s6/aggregate_evidence_survey.txt.
- verdict: KILLED

## [s6] Walking pointers (*hp++ / *--hp) — the strongest honest, project-precedented construct for this residual — close the /s bit but cannot match, on instruction count.
- mechanism: INDIRECT_REF(POSTINCREMENT_EXPR) / (PREDECREMENT_EXPR) is not a PLUS_EXPR, so expr.c:4567-4577 never sets /s: the technique clears the flag with no cast, no volatile, no invented scalar, and a plain human reading. Precedent: .claude/rules/walking-pointer-serializes-parallel-loads.md (cheat-reviewer + owner sign-off 2026-06-15, two confirmed COMPLETED-C cases). BUT target's base register holds s0+(s3<<2) with literal -8/-4 load offsets, while a walking pointer must hold the dereferenced address at offset 0 — which costs one extra addiu to bias the base.
- probe: three spellings through s8/pr.sh — post-increment from &s0[s3-2]; the same with the store hoisted; pre-decrement from &s0[s3].
- result: 249 insns / 4 diffs; 246 / 25; 249 / 5. Target is 248. The extra addiu is structural and no schedule removes it. Banked at rejected/s6-walking-pointer-postinc-249-4-extra-addiu.c, rejected/s6-walking-pointer-predec-249-5-extra-addiu.c, rejected/s6-walking-pointer-store-first-246-25.c.
- verdict: KILLED

## [s6] The [s9.7] exhaustion enumeration is closed by measurement as well as by syntax: the two requirements intersect in exactly one construct, and it is banned.
- mechanism: (i) the base register must stay at s0+(s3<<2) so the loads keep literal -8/-4 offsets, else +1 addiu ([s6.4]); (ii) the -4 read's INDIRECT_REF operand must not be a PLUS_EXPR, else /s stays set ([s6.1]/[s6.2]). Requirement (i) forbids the dereferenced address from living in the pointer being dereferenced, which excludes every post/pre-increment and index-0 shape; requirement (ii) excludes every indexed shape.
- probe: the four measurements above plus the [s9.6] pm-form control, all at the same chassis.
- result: the intersection is the single construct `s32 *pm1 = ptr - 1; ... *pm1` (banned_constructs #4/#5). No fourth spelling and no fourth shape exists. The residual is a fidelity question about one lvalue, not a search problem.
- verdict: CONFIRMED

## [s6] GCC 2.7.2 portable fact (useful beyond this function): p[0] / *p on a pointer variable is /s = 0; p[k] for k != 0 is /s = 1; and /s survives CSE address folding.
- mechanism: build_array_ref folds PLUS_EXPR(p, 0) away, so expr.c:4567-4577's PLUS_EXPR test never fires for index 0. The flag is set once at expand time from the tree and is carried on the MEM thereafter regardless of how later passes rewrite the address.
- probe: read t.flow for this TU's own mainline reads.
- result: s0[0] and a0_ptr[0] are (mem:SI (reg/v:SI 74)) / (mem:SI (reg/v:SI 109)) with no /s; a0_ptr[-1] and a0_ptr[1] are (mem/s:SI (plus:SI ...)); insns 170 and 246 are (mem/s:SI (reg ...)) — bare-register addresses that still carry /s from a PLUS_EXPR birth.
- verdict: CONFIRMED

## [s6] The case-3 residual is produced by MEM_IN_STRUCT_P (/s) on the two header-word loads plus the sched.c:831-839 exemption against the non-/s, non-varying store to D_8009947A, and this can be read directly out of cc1's own RTL dumps rather than inferred from compiler source.
- mechanism: expr.c:4567-4577 sets MEM_IN_STRUCT_P at expand time iff the INDIRECT_REF address subtree is a PLUS_EXPR; sched.c:831-839 (true_dependence / anti_dependence) then suppresses the conflict between a varying /s SImode MEM and a non-varying non-/s MEM, so sched1 has no edge to respect.
- probe: cc1 -O2 -da on the candidate body (tmp/grind/func_800460E4/s6/dumps_cand/); read t.flow for the MEM flags and t.sched for the emitted order and per-insn dependence insn_lists.
- result: t.flow: insn 308 = (mem/s:SI (plus (reg 138) (const_int -8))), insn 322 = (mem/s:SI (plus (reg 138) (const_int -4))), insn 331 = (set (mem:HI (symbol_ref "D_8009947A")) ...) with no /s. t.sched: 308 -> 329 (li 1) -> 331 (sh) -> 309 -> 310 -> 322, and insn 322's dependence list names ONLY insn 304 (its address) — the missing memory edge is visible in the dump. Pass attribution final: born in expr.c expand, taken by sched1.
- verdict: CONFIRMED

## [s6] Only ONE of the two case-3 header reads has to lose /s for the function to close, and it is the -4 (second) read; the pair was never the unit.
- mechanism: The store follows both loads in source order, so it is anti-dependent on whichever loads it is not exempt from. Blocking it against the LAST load alone already keeps it out of the inter-load gap; the FIRST load's exemption status is irrelevant to where the store can be placed.
- probe: Two one-sided variants at the current chassis through tmp/grind/func_800460E4/s8/pr.sh — (A) -4 read via a pointer variable with -8 left as ptr[-2]; (B) -8 via a pointer variable with -4 left as ptr[-1].
- result: (A) 248 insns / 0 diffs — the whole function closes. (B) 245 insns / 9 diffs — completely inert. Both banked research-only (banned #4 family, NOT proposed) at rejected/s6-research-only-banned-only-m1-ptr-248-0.c and rejected/s6-research-only-only-m2-ptr-245-9-inert.c.
- verdict: CONFIRMED

## [s6] The D_80099478/D_8009947A aggregate model has independent, pre-existing evidence somewhere in the binary that would satisfy prong (a) of the aggregate-merge rule.
- mechanism: Prong (a) (no-new-park-categories.md, 2026-08-17) requires base-register or stride evidence independent of the byte chase. If the two halfwords were members of one aggregate, GCC would materialize the base once and offset the second access.
- probe: Surveyed every access to both symbols across asm/funcs/*.s — 7 functions (func_800460E4, func_800464C4, func_8004659C, func_8004668C, func_800466C0, func_80046798, func_800467A8), 12 accesses.
- result: All 12 accesses use their own independent lui %hi / %lo pair. Decisive negative: func_8004668C stores to BOTH symbols back to back and emits a SECOND lui $at for the second store rather than reusing the first base with a +2 offset. No base-register or stride evidence exists anywhere in the binary — the aggregate model is affirmatively contradicted, not merely unproven. Artifact: tmp/grind/func_800460E4/s6/aggregate_evidence_survey.txt.
- verdict: KILLED

## [s6] The walking-pointer technique (*hp++ / *--hp) — honest, cast-free, volatile-free and already project-sanctioned — closes case 3, because a post/pre-increment INDIRECT_REF operand is not a PLUS_EXPR and therefore clears /s.
- mechanism: expr.c:4567-4577 only sets /s for PLUS_EXPR (or SAVE_EXPR-of-PLUS / aggregate) address subtrees, so INDIRECT_REF(POSTINCREMENT_EXPR) is /s = 0. Precedent: .claude/rules/walking-pointer-serializes-parallel-loads.md (independent cheat-reviewer + owner sign-off 2026-06-15, two confirmed COMPLETED-C cases). Counter-mechanism: target's base register holds s0+(s3<<2) with literal -8/-4 load offsets, while a walking pointer must hold the dereferenced address at offset 0.
- probe: Three spellings through s8/pr.sh — post-increment from &s0[s3-2]; the same with the store hoisted to the top of the block; pre-decrement from &s0[s3].
- result: 249 insns / 4 diffs; 246 / 25; 249 / 5. Target is 248: biasing the base costs one extra addiu, and the extra instruction is structural — no schedule removes it. The /s flip works; the instruction count does not. Banked at rejected/s6-walking-pointer-postinc-249-4-extra-addiu.c, rejected/s6-walking-pointer-predec-249-5-extra-addiu.c, rejected/s6-walking-pointer-store-first-246-25.c.
- verdict: KILLED

## [s6] The [s9.7] exhaustion enumeration might have a fourth member: some non-PLUS_EXPR spelling the syntactic enumeration missed.
- mechanism: A closing form must satisfy two independent requirements simultaneously — (i) the base register stays at s0+(s3<<2) so the loads keep literal -8/-4 offsets, and (ii) the -4 read's INDIRECT_REF operand is not a PLUS_EXPR. Requirement (i) forbids the dereferenced address from living in the pointer being dereferenced, which excludes every post/pre-increment and index-0 shape; requirement (ii) excludes every indexed shape.
- probe: The four measurements above plus the [s9.6] pm-form control, all at the same chassis, cross-checked against the RTL dump for what actually sets /s.
- result: The intersection of (i) and (ii) is exactly one construct — `s32 *pm1 = ptr - 1; ... *pm1` — which is banned_constructs #4/#5. The enumeration is now closed by measurement as well as by syntax: no fourth spelling and no fourth SHAPE exists. The residual is a fidelity question about one lvalue, not a search problem.
- verdict: CONFIRMED

## [s6] GCC 2.7.2 portable fact (banked for other functions): p[0] / *p on a pointer variable yields a MEM without /s, p[k] for k != 0 yields /s, and /s survives CSE address folding.
- mechanism: build_array_ref folds PLUS_EXPR(p, 0) away, so expr.c:4567-4577's PLUS_EXPR test never fires for index 0. The flag is set once at expand time from the tree shape and is carried on the MEM thereafter regardless of how later passes rewrite the address.
- probe: Read t.flow for this TU's own mainline reads and for the CSE-folded ((s32 *)arg1)[s3] loads.
- result: s0[0] and a0_ptr[0] are (mem:SI (reg/v:SI 74)) / (mem:SI (reg/v:SI 109)) with no /s; a0_ptr[-1] and a0_ptr[1] are (mem/s:SI (plus:SI ...)); insns 170 and 246 are (mem/s:SI (reg ...)) — bare-register addresses that still carry /s from a PLUS_EXPR birth.
- verdict: CONFIRMED

## [s7] 2026-08-25 — FORENSICS modality (session 7)

### H31 — the sched.c anti_dependence exemption can be broken from the STORE side, not only the LOAD side
**Statement.** [s6.5]/[s9.7] declared the closing set to be exactly one (banned)
construct on the ground that the `-4` header load must lose MEM_IN_STRUCT_P. That
enumeration covered only one term of a five-term conjunction in
`anti_dependence` (tools/gcc-2.7.2/sched.c:843-864). Setting MEM_IN_STRUCT_P on
the *store's* MEM breaks the same exemption and restores the dependence edge,
with the loads left untouched.
**Mechanism.** sched.c:855-859 — the exemption requires
`MEM_IN_STRUCT_P(load) && rtx_addr_varies_p(load) && mode != QImode &&
!MEM_IN_STRUCT_P(store) && !rtx_addr_varies_p(store)`. expr.c:4567-4577 / 4888 /
4329 / 5788 set /s on a MEM from `AGGREGATE_TYPE_P` of the accessed object's
type, so an aggregate-typed global gives its store /s while emitting the same
single `sh` through the assembler's `%hi/%lo` macro.
**Probe.** `extern s16 D_8009947A[1];` with the four accesses spelled
`D_8009947A[0]`, everything else identical to the [s7]/[s8]/[s9] candidate body;
scored through tmp/grind/func_800460E4/s8/pr.sh.
**Result.** 248 insns / 19 diffs (base 245 / 9). Case 3 becomes **byte-exact**
against target lines 131-145 — adjacent `lw -8` / `lw -4`, store after both. The
19 diffs are a global $s2/$s3 seat swap and a case-34 order inversion.
**Verdict: CONFIRMED.**

### H32 — with the store carrying /s, case 34 must be written load-before-store, and that reorder is byte-inert on its own
**Statement.** The 19 residual diffs of H31 are not a case-3 problem; they follow
from case 34, where the same store now has a live anti-dependence against
`s0[5]`, so source order decides an ordering that used to be a free scheduler
choice.
**Mechanism.** Same exemption clause; target's case 34 emits `lw v0,20(s0)`
before `li v1,1; sh v1`, so the source statement order must be
`s4 = ...ALIGN4(s0[5]); D_8009947A = 1;`.
**Probe.** (a) case-34 reorder alone on the plain-scalar chassis; (b) array-typed
decl + case-34 reorder together.
**Result.** (a) **245 / 9 — completely inert** (so the reorder carries no
coercion content of its own); (b) **248 / 0**, and
`sandbox func_800460E4 --disable all` independently prints `score: 0,
build_insns: 248, target_insns: 248, rules_dropped: 10, cheat_asm_stripped: 0`.
**Verdict: CONFIRMED.**

### H33 — the closing construct is a first reach of an unsanctioned family, so it may not be submitted
**Statement.** Re-typing `extern s16 D_8009947A;` as `extern s16 D_8009947A[1];`
is not covered by any sanctioned family.
**Probe.** Read `.claude/rules/header-type-correction-from-use-sites.md` in full;
grepped `docs/reference/sotn-construct-index.md`; re-checked the symbol map
around 0x8009947A.
**Result.** Prong (a) of header-type-correction fails on its own terms (no
independent use site *requires* the array type — every `[0]` is authored by this
diff). The SOTN construct index has no scalar-to-array re-typing entry (only two
`&g_Entities[1]` pointer aliases into a genuine array, one PSX one PSP). The
symbol map shows three separately named neighbours (`g_stage_id` 0x80099478,
`g_stage_variant` 0x8009947A, `g_stage_data` 0x8009947C) and no indexed or
base-register access anywhere in the binary. **Verdict: CONFIRMED (it is a first
reach).** Disposition: ruling-request; the form is banked in
rejected/s7-ruling-pending-array-typed-store-248-0.c and NOT promoted to
candidate.c.

### H34 — [s6.3]'s double-`lui` argument is not sound in isolation
**Statement.** func_8004668C re-materialising `$at` with a second `lui` for
back-to-back stores to the two symbols does not discriminate between "two scalars"
and "one aggregate".
**Mechanism.** GCC emits one RTL store per member; the assembler's `sh sym+k`
macro expands each into its own `lui $at` / `%lo` pair regardless of object
identity.
**Verdict: CONFIRMED** (the aggregate-merge conclusion still stands, on the 04:46
ruling and on the absence of any indexed/base-register access — but future
sessions must not cite the double-`lui` as the reason).

### Dead on this session's evidence — do NOT re-run
- L2 (make the case-3 load address non-varying): impossible, it is `s0 + (s3<<2)`.
- L3 (make the load QImode): impossible, the header words are 32-bit.
### H35 — L5 (make the store's address varying, via a C-level pointer alias to the global) is DEAD on measurement
**Statement.** The remaining store-side term, `!rtx_addr_varies_p(store)`, can be
broken by routing the four `D_8009947A` accesses through a function-scope pointer
alias `s16 *vp = &D_8009947A;` — a form that would sit in the SANCTIONED
pointer-alias family (`.claude/rules/pointer-alias-fake-exception.md`) rather than
needing a new family grant. If it closed, it would be strictly preferable to H31.
**Mechanism.** `(mem:HI (reg))` has a varying address, so the exemption's fifth
term fails and the anti-dependence edge is restored — the same end state as H31,
reached without re-typing anything.
**Probe.** `s16 *vp = &D_8009947A;` declared with the other locals; all four
accesses spelled `*vp`. Measured with and without the H32 case-34 reorder.
**Result.** **246 / 58** and **246 / 43** (base 245 / 9; target 248). Holding the
global's address in a pointer costs an extra address materialisation and
re-shuffles allocation across the whole function — the failure mode is identical
in kind to the walking-pointer kill in [s6.4]: the lever works, the instruction
budget does not. **Verdict: KILLED.**
Banked: rejected/s7-store-pointer-alias-246-58.c and
rejected/s7-store-pointer-alias-case34-246-43.c.

### The conjunction is now fully enumerated
L1 load-/s: only the banned pm1 construct ([s6.5]). L2, L3: impossible by
construction. L4 store-/s: CLOSES at 248/0 but needs an unsanctioned re-typing
(H31/H32/H33). L5 store-address-varying: sanctioned family available but dead on
instruction count (H35). No sixth term exists.
