# Hypothesis ledger — func_80019568

## [s1] 2026-08-25 — recon

H1 (CONFIRMED, mechanism-level): the committed rule-era body's structure (register pins,
explicit walking pointers, pointer-alias tail) is NOT the original shape. Fresh indexed
derivation (v2) reaches 34 honest vs the chassis's 42, with build insn count == target.

H2 (CONFIRMED): loop 1's target shape is INDEXED accesses over one counter i —
`sp.output[i]`, `sp.output[i+2]`, `packets[i*8+k]` — giving loop.c combined givs whose
reduced walkers match target a2/a3 exactly, and preserving the sh->lhu store-reload
(pointer-deref spelling folds the reload; indexed spelling does not). Probe: v1 (49) vs
v2 (34) + diff. 

H3 (CONFIRMED): the single dominant residual mechanism at v2 is loop.c biv elimination
of counter i (final-value path + compare rewrite via an always-computable out-class giv).
Blocking it (diagnostic `while (i+1<3)`) drops 34 -> 28 and aligns the entire register
file (i->t0, mask->t1, const4->t2, jtbl->t3, loop-2 counter->t0). Probe: dumps + score.
The diagnostic spelling itself is NOT submittable (2 extra insns; coercion-flavored).

H4 (KILLED): s16-typed counter to fail the giv `v->mode == mode` check — score 74,
build 159, sign-extends everywhere.

## Frontier for s2+

F1 (primary): find the HONEST spelling under which `while (i < 2)` survives biv
elimination emitting plain `slti v0,t0,2`. Mechanism-grounded angles, in order:
  a. Identify WHICH giv qualified for the compare rewrite (out-class, mult 2,
     add reg191, always_computable despite all output sites being inside arms).
     Read loop.c record_giv/not_every_iteration handling, or use the instrumented cc1
     (tools/gcc-2.7.2/cc1, BB2_*_DEBUG hooks) to log the qualifying giv's insn. Then
     find the C statement it comes from and see whether an honest respell of THAT
     statement makes it !always_computable (every giv ineligible -> i survives).
  b. Alternate axis: make one giv of i fail the worthwhile test (loop.c:3824
     lifetime*threshold*benefit < insn_count) -> all_reduced=0 -> no elimination.
     Check whether that ALSO suppresses the a2/a3 walkers (would break H2's match —
     measure before investing).
  c. Census the OTHER solved jtbl-cluster members (owner directive: "same pure-C route
     as the other jtbl members") — grep docs/grind/decisions.md and closed ledgers for
     how their loop counters survived; a proven sibling idiom beats derivation.
F2: keep `li 1` inside the loop (target: addiu v0,zero,1 between lhu and addiu -1,
    filling the load-delay slot; ours hoists it as a movable). Suspect coupled to F1's
    register pressure; re-measure after F1. If independent: loop.c scan_loop movables
    ("move-insn savings 1") — find what makes the original's 1-pseudo unmovable
    (e.g. conditional-arm placement that scan_loop refuses; ours moved regardless).
F3: tail block — honest producer for the materialized &D_80102790 (lui+addiu, lw/sw via
    0(v0)) + early D_80102790 store + store order 9C,94,98. Investigate the aggregate
    hypothesis (D_80102788..D_8010279C one struct) with independent evidence
    (split-scalars-hide-aggregate recipe: adjacent .data words, sibling functions,
    cross-TU stride) BEFORE any pointer spelling; the rule-era pointer local is the
    FAKE-family fallback, last resort only.

## [s1] The committed rule-era body (pins, hardcoded asm, pointer-alias tail) reflects rule calibration, not original structure
- mechanism: cheat-asm stripping leaves a structurally broken object (extra callee-save s1, split output[2] pointer, lbu through sp+43)
- probe: sandbox --disable all on committed body (42) vs fresh clean derivations (v1 49, v2 34 with build_insns==target_insns==141)
- result: fresh v2 beats chassis and matches insn count exactly
- verdict: CONFIRMED

## [s1] Loop 1's original shape is indexed accesses (sp.output[i], sp.output[i+2], packets[i*8+k]) over one counter, not walking pointers
- mechanism: loop.c combine_givs merges index-derived givs into one walker per array (a2 offsets 0/4, a3 offsets 0..3) matching target; pointer-deref spelling instead splits per-offset walkers AND lets cse fold the sh->lhu store-reload that target keeps
- probe: v1 (explicit pointers) 49 with split walkers and folded reload; v2 (indexed) 34 with target-shaped walkers and surviving lhu
- result: v2 matches target addressing and reload
- verdict: CONFIRMED

## [s1] The dominant v2 residual is loop.c eliminating biv i (exit test rewritten to slt a2,limit)
- mechanism: strength_reduce final-value path (dumps/code6cac.loop:885-886) + compare rewrite via an always-computable out-class giv (loop.c:6127+); blocking the rewrite cascades the entire register file to target (i->t0, mask->t1, const4->t2, jtbl->t3, loop-2 counter->t0)
- probe: diagnostic exit test `while (i + 1 < 3)` (REG nested in PLUS fails maybe_eliminate_biv_1) scored 28 vs 34
- result: elimination blocked, registers align; diagnostic itself costs +2 insns (combine will not fold signed < across PLUS) and is not submittable
- verdict: CONFIRMED

## [s1] A narrow (s16) counter blocks elimination via the giv mode check honestly
- mechanism: loop.c:6131 requires v->mode == biv mode
- probe: s16 i measured
- result: 74 (build 159), sign-extends everywhere
- verdict: KILLED

## [s2] 2026-08-25 - structural

H5 (CONFIRMED): biv `i` survives loop.c elimination when a per-iteration record pointer
(`u8 *p = &packets[i*8]; s16 *o = &sp.output[i];`) is hoisted to the top of the loop body.
- mechanism: DEST_ADDR givs are unconditionally always_computable (loop.c:4386) so they can
  ALWAYS rewrite the compare; the only reachable gate is `all_reduced == 1`
  (strength_reduce:4034). Collapsing each scale group to a single "add 0" DEST_REG giv
  leaves that leader with benefit 2 - add_cost(2)*biv_count(1) = 0, which trips the
  worthwhile test at loop.c:3824 -> ignore=1 -> all_reduced=0 -> elimination skipped.
- probe: v4/v5 measured; .loop dump shows "giv of insn 51 not worth while, 0 vs 51",
  "giv of insn 45 not worth while, 0 vs 51", no "biv 75 was eliminated", exit test `slt $8,2`.
- result: 34 -> 20, register file (t0/t1/t2/t3, a2, a3) identical to target.
- verdict: CONFIRMED

H6 (CONFIRMED): the hoisted `li 1` is suppressed by giving its pseudo two non-consecutive
sets inside the loop (reusing `voice` for the voice id and then for the constant 1).
- mechanism: scan_loop only creates a movable when n_times_set==1 or the sets are
  consecutive (loop.c:705-708); the desirability test (loop.c:1631) can never reject one
  because threshold = 2*(1+n_non_fixed_regs) dwarfs insn_count.
- probe: v5; build_insns 142 -> 141 == target; `li v1,1; sh v1,4(a2)` now sits between the
  lhu and the addiu -1, exactly target's slot.
- result: structure matches; only a v0<->v1 seat swap remains in that block.
- verdict: CONFIRMED (family-gated: variable-reuse, FAKE required for submission)

H7 (KILLED): explicit walking pointers (out++/pk += 8) with a separate counter reproduce
target's a2/a3 walkers.
- mechanism: with a pointer biv, MEM(biv) is NOT a giv but MEM(biv+k) IS, so every nonzero
  offset reduces into its own register.
- probe: v3 measured 46 (build 146) - walkers at sp+16, sp+20, sp+40 AND sp+43(-2..0).
- verdict: KILLED (rejected/pointer-walkers-split-givs.c)

H8 (KILLED): the tail's materialized &D_80102790 comes from an aggregate spanning
D_80102788..D_8010279C.
- mechanism/probe: sibling func_800194F4 writes all six of those words with plain
  per-symbol %hi/%lo macros, which is what a scalar declaration produces; and target's
  OTHER three tail stores (94/98/9C) are themselves plain %lo stores off $at. Only
  D_80102790 gets a register-held address, so the producer is address-taken, not
  aggregate-based.
- verdict: KILLED as an aggregate question; F3 is a pointer-alias family question.

H9 (KILLED): reordering the four tail stores to target's emission order (90, 9C, 94, 98)
recovers the tail shape.
- probe: v8 measured, still 20, no diff change. The scheduler chooses the store order.
- verdict: KILLED (rejected/tail-store-order-9c-before-94-inert.c)

## Frontier for s3+

F2b (family question, cheap, do FIRST): find an HONEST spelling that gives the `1`-holding
  pseudo two non-consecutive sets in the loop, so H6's win does not depend on the
  FAKE-gated variable-reuse family. Not yet measured: a real flag local written in BOTH
  arms (`flag = 1` in the if-arm, `flag = 0` in the else-arm) and stored inside each arm -
  combine should fold the 0 arm into `sh zero,4(a2)` (the MIPS movhi pattern takes
  const_int 0 via $zero) while the 1 arm keeps its `li`. If that also measures 141/20 the
  whole loop-1 candidate becomes ordinary C with no family claim at all.
  ALTERNATIVELY get a ruling that the `voice` reuse sits inside the frozen
  "variable reuse for codegen control" family and just annotate it.

F1b (R1, ~6 insns): if-arm v0<->v1 seat swap. Target ties `voice` to the lbu temp's seat
  (srl v0,v0,4) and the lhu temp to v1; ours is the mirror. Declaration-order swaps
  (voice before/after voice2) measured INERT. This is a pure local-alloc question - run
  tools/ra_solver (inverse_compose.py classify) on the .lreg dump before spending more
  measurements on spelling guesses.

F3 (R2, ~13 insns): tail block. The aggregate hypothesis is KILLED (H8) and statement
  reordering is KILLED (H9). What remains: what honest C makes GCC 2.7.2 hold
  &D_80102790 in a register across a load and a store of the same scalar global?
  Probe order: (a) read the .cse/.combine dumps for why the two (mem (symbol_ref))
  accesses did not share an address register; (b) census SOTN master via
  docs/reference/sotn-construct-index.md for a load+store-through-one-address-register
  global RMW precedent; (c) read .claude/rules/pointer-rmw-global-sanctioned.md BEFORE
  assuming this needs the pointer-alias FAKE family
  (.claude/rules/pointer-alias-fake-exception.md).

## [s2] Biv i survives loop.c elimination when a per-iteration record pointer (u8 *p = &packets[i*8]; s16 *o = &sp.output[i];) is hoisted to the top of the loop body.
- mechanism: record_giv (loop.c:4386) makes DEST_ADDR givs always_computable=1 UNCONDITIONALLY with mode SImode, so an indexed access always yields a giv eligible to rewrite the exit compare (loop.c:6160-6190) -- s1's 'all giv sites are in conditional arms' angle is a dead lead. The only reachable gate is strength_reduce:4034's all_reduced==1. Hoisting the record pointer collapses the two i*8 and three i*2 DEST_REG 'add 0' givs to ONE each; combine_givs only accumulates benefit into leaders and the worthwhile loop skips combined members (loop.c:3782), so a lone leader has benefit 2, minus add_cost(2)*biv_count(1) = 0, tripping loop.c:3824 -> ignore=1 -> all_reduced=0 -> elimination skipped entirely. The ignored intermediates are dead once the add-reg givs reduce, so they cost no insns.
- probe: v4/v5 applied to src/code6cac.c, sandbox --disable all, plus pwsh tools/grinder/dump.ps1 and a read of tmp/grind/func_80019568/dumps/code6cac.loop
- result: 34 -> 20. Dump shows 'giv of insn 51 not worth while, 0 vs 51' and 'giv of insn 45 not worth while, 0 vs 51', NO 'biv 75 was eliminated' line, and the emitted exit test is slt $8,2 (= target slti v0,t0,2). Register file identical to target: i->t0, mask->t1, const4->t2, jtbl->t3, a2 = sp+16 walker offsets 0/4, a3 = sp+40 walker offsets 0..3.
- verdict: CONFIRMED

## [s2] The hoisted 'li 1' for sp.output[i+2]=1 stays inside the loop if its pseudo gets two NON-consecutive sets (voice = p[1]>>4; o[0] = voice; voice = 1; o[2] = voice;).
- mechanism: scan_loop only creates a movable when invariant_p(src) and (n_times_set==1 or consec_sets_invariant_p) -- loop.c:702-716. The desirability test (loop.c:1631, threshold*savings*lifetime >= insn_count with threshold = 2*(1+n_non_fixed_regs), loop.c:532) can never reject an existing movable here, so preventing CREATION is the only lever.
- probe: v5 measured; objdump diff against asm/funcs/func_80019568.s via tmp/grind/func_80019568/s2/nd.py
- result: build_insns 142 -> 141 == target 141; 'li v1,1; sh v1,4(a2)' now sits between the lhu and the addiu -1, exactly target's load-delay slot. Only a v0<->v1 seat swap remains in that block. FAMILY-GATED: this is the variable-reuse family (defeat-licm-hoist-var-reuse.md / staged-value-reused-variable.md) and is NOT cleared for submission as spelled.
- verdict: CONFIRMED

## [s2] Explicit walking pointers (out++/pk += 8) plus a separate counter reproduce target's single a2 and a3 walkers.
- mechanism: With a pointer biv, MEM(biv) is not a giv while MEM(biv+k) is, so every nonzero offset strength-reduces into its OWN register instead of folding into the MEM offset.
- probe: v3 measured with sandbox --disable all and the generated .s read
- result: 46 (build 146). Four walkers: sp+16 and sp+20 for the output, sp+40 and a second packet walker anchored sp+43 with offsets -2..0. Only the indexed spelling combines all offsets onto one register.
- verdict: KILLED

## [s2] The tail's materialized &D_80102790 (lui+addiu, lw/sw via 0(v0)) comes from an aggregate spanning D_80102788..D_8010279C (s1 frontier F3's leading idea).
- mechanism: An aggregate declaration would not change how GCC addresses a fixed-offset member: it still emits per-access lui at,%hi / sw ,%lo(at).
- probe: Read asm/funcs/func_800194F4.s (the sibling writer of the same cluster) and re-read target's own tail
- result: func_800194F4 writes D_80102788/8A and D_80102790/94/98/9C with plain per-symbol %hi/%lo macros, and target's own tail stores to 94/98/9C are likewise plain %lo stores off $at. ONLY D_80102790 gets a register-held address, so the producer is address-taken, not aggregate-based. F3 is a pointer-alias question, not an aggregate question.
- verdict: KILLED

## [s2] Reordering the four tail stores into target's emission order (90, 9C, 94, 98) recovers the tail block shape.
- mechanism: Source statement order feeding the scheduler's store ordering.
- probe: v8 measured
- result: still 20, diff unchanged -- the scheduler picks the order; the entire tail residual is the address materialization.
- verdict: KILLED

## [s2] Computing voice2 before the o[2] store is a real improvement (it measures 17).
- mechanism: Without an intervening store to the same base, cse folds the sh -> lhu reload into the stored value.
- probe: v7 measured and diffed
- result: 17 but build_insns 138 vs target 141 -- target's 'lhu v1,0(a2); addiu v1,v1,-1; sll 16; sra 16' chain is deleted. A FALSE minimum: the o[2] store sitting between the o[0] store and the reload is load-bearing for the match.
- verdict: KILLED

## [s3] 2026-08-25 - structural

H10 (CONFIRMED): the honest replacement for the s2 FAKE-gated variable-reuse spelling is a
real per-slot flag local written 1 in the valid arm and 0 in the invalid arm, stored
inside each arm.
- mechanism: loop.c:702-716 - scan_loop creates a movable only when invariant_p(src) and
  (n_times_set==1 or consec_sets_invariant_p). Two sets in two different arms are
  non-consecutive, so no movable exists for the `li 1` and it stays in the loop filling
  target lhu load-delay slot. Additionally, because `enable` is a DISTINCT pseudo from
  the shifted voice id, local-alloc seats voice in $v0 (the lbu temp seat) and the lhu
  reload in $v1 - target assignment - instead of the s2 mirror image.
- probe: v9 applied to src/code6cac.c and measured with sandbox --disable all, then two
  ablations with the s3 tail in place: bare literal (no local) and the s2 voice-reuse.
- result: 20 -> 12 with the s2 tail; with the s3 tail, flag local = 0, bare literal = 8
  (build 142, li hoisted), voice-reuse = 8 (build 141, v0/v1 swapped). The flag local is
  the unique closer.
- verdict: CONFIRMED

H11 (KILLED): dropping the `old_mask` local and reading D_80102790 directly three times
(store last) makes cse share one address register for the load and the stores.
- mechanism: cse would have to hoist a (symbol_ref) address into a pseudo.
- probe: v10 measured.
- result: 11, build 141. cse folds the three READS into one `lw`, but each access is still
  a (mem (symbol_ref)) assembled as lui $at/%lo, so no `la` is ever emitted and the
  D_80102790 store still sinks below the nor/and chain.
  rejected/tail-direct-multiread-no-la-11.c
- verdict: KILLED

H12 (CONFIRMED): the tail register-held &D_80102790 comes from a pointer local used for
a read-modify-write of that global.
- mechanism: on MIPS a MEM whose address is a bare symbol_ref is already a legitimate
  address, so no pass materialises it; only a pointer VALUE forces
  `lui/addiu` (la) and lets both the load and the store use 0($v0). It also anchors the
  store early, ahead of the nor/and chain, matching target order.
- probe: v11 = v9 + `s32 *p = &D_80102790; old_mask = *p; *p = sp.voice_mask;`
- result: 12 -> **0**, build_insns 141 == target_insns 141. sandbox --disable all score 0.
- verdict: CONFIRMED

H13 (KILLED as a live question): R1, the if-arm v0<->v1 local-alloc seat swap, needs a
tools/ra_solver classification.
- mechanism: it was never an allocator tie; it was a consequence of the C-level variable
  partition (the s2 spelling made ONE variable carry both the voice id and the constant 1,
  which forced the mirror seats).
- probe: the H10 flag local; the swap disappeared with no solver run and no
  declaration-order games.
- verdict: KILLED (question dissolved, not answered)

## Frontier for s4+ (only if this candidate is bounced)

The function is at sandbox distance 0 with build_insns == target_insns. There is no
measured residual left. The only open items are dispositional, not technical:
 1. C2 (`enable`) is claimed under .claude/rules/named-local-fake-exception.md
    (constant-holder / named scalar local) with a FAKE annotation. It is a two-valued
    per-arm flag rather than that rule canonical `s32 k = 1;`-held-across-a-call
    example. If layer 1 or the Judge holds that the family does not reach this shape,
    the correct next outcome is a ruling-request on C2 ALONE - C1 (record pointers,
    ordinary C) and C3 (pointer RMW, separately user-sanctioned) are unaffected and the
    byte-proof stands. Do NOT re-grind: the measured alternatives are exhausted and
    banked (bare literal 8/142, variable-reuse 8/141).
 2. Integration: func_80019568 still carries 5 regfix rules calibrated to the old
    rule-era body. They must be retired by the operator/driver (`retire func_80019568`)
    before/with the full-build SHA1 verify. This session touched no rule file.

## [s3-permuter] 2026-08-25 - permuter modality (after the layer-1 FAIL on C2)

H14 (CONFIRMED): a BARE LITERAL `o[2] = 1;` can never survive LICM in this loop, for a
reason that is structural rather than tuning-dependent.
- mechanism: scan_loop's three-way movable gate (tools/gcc-2.7.2/loop.c:695-701).
  `maybe_never` is already 1 inside either arm (set at the loop's first JUMP_INSN,
  loop.c:918-930), so condition 1 is dead; but a literal's destination pseudo is a
  COMPILER TEMP, which satisfies condition 2 (`! REG_USERVAR_P && ! REG_LOOP_TEST_P`)
  unconditionally. The movable is therefore always created and always hoisted.
- probe: `rejected/bare-literal-o2-li-hoisted-8.c` measured on the s3 chassis.
- result: 8, build_insns 142 (one hoisted `li $t4,1` plus the v0/v1 seat mirror and the
  `nop` that fills the lhu load-delay slot in target).
- verdict: CONFIRMED. Corollary: the honest closer MUST be a named user variable; the
  only remaining doors are `reg_in_basic_block_p` returning 0 (loop.c:1062-1100) or the
  n_times_set/consec_sets test failing.

H15 (KILLED): computing the flag honestly before the branch
(`s32 enable = (rec[0] == 0); if (enable) { ... o[2] = enable; } else { ... o[2] = enable; }`)
reproduces target.
- mechanism: a comparison result is not `invariant_p`, so no movable can exist at all,
  and the flag register is genuinely recomputed each iteration.
- probe: `rejected/computed-flag-sltiu-10.c` measured.
- result: 10, build_insns 142. GCC materialises the comparison (`sltiu`) and inverts the
  branch sense; target branches directly on the loaded byte (`lbu; nop; bnez`).
- verdict: KILLED.

H16 (CONFIRMED - the closer): a DEFAULT-INITIALISED named flag local
(`s32 enable = 0;` at the top of the loop body, `enable = 1;` in the valid arm, the
write-out `o[2] = enable;` duplicated into both arms) keeps the `addiu $v0,$zero,1`
inside the loop AND lands the target's v0/v1 seats.
- mechanism: the loop-top default is the pseudo's first mention, so
  `reg_in_basic_block_p` returns 0 at the in-arm set (loop.c:1067), killing condition 3;
  condition 1 is dead via `maybe_never`; condition 2 is dead because the pseudo is
  REG_USERVAR_P. Independently, the two sets are non-consecutive so the
  `n_times_set == 1 || consec_sets_invariant_p` test (loop.c:706-709) also fails. No
  movable is created. Because the flag is a DISTINCT pseudo from the shifted voice id,
  local-alloc seats the voice temp in $v0 and the lhu reload in $v1 (target), instead of
  the mirror the carrier-reuse spellings produce. The else arm reads the default, and
  combine folds that store to `sh $zero,0x4($a2)` (target line 55).
- probe: v15 applied to src/code6cac.c; `sandbox func_80019568 --disable all`.
- result: **0**, build_insns 141 == target_insns 141.
- verdict: CONFIRMED. Family posture: the flag local itself is ordinary C (the
  initialiser is READ on the else path, so nothing is dead); the duplicated write-out is
  claimed under .claude/rules/duplicated-statement-into-arms.md and FAKE-annotated. This
  is NOT the banned two-armed `enable = 0; o[2] = enable;` shape - that form's dead
  in-arm assignment is absent here.

H17 (KILLED): the write-out can sit once after the if/else join instead of being
duplicated into the arms.
- mechanism: if the duplication were byte-neutral (cross-jump re-merge), the single-copy
  spelling would be preferable and no family claim would be needed at all.
- probe: `rejected/flag-store-after-join-21.c` measured.
- result: 21, build_insns 136 - FOUR target instructions deleted. Target genuinely has
  two separate `sh` stores to `0x4($a2)` (lines 34 and 55 of the target listing), one per
  arm; the join spelling emits one.
- verdict: KILLED. The duplication is load-bearing, which is why it carries the FAKE
  annotation and the family claim rather than being dropped.

H18 (KILLED): borrowing an EXISTING loop variable as the 1-carrier reaches 0.
- mechanism: variable-reuse gives the carrier two non-consecutive sets, so the `li` does
  stay in the loop - but the carrier is then the SAME pseudo as the value it borrows
  from, and local-alloc mirrors the v0/v1 seats.
- probe: `voice` carrier (s2, `rejected/voice-reuse-instead-of-flag-8.c`) and `bits`
  carrier (this session, permuter-found, `rejected/bits-carrier-reuse-seat-mirror-6.c`
  and `rejected/bits-carrier-reuse-indexed-uses-6.c`).
- result: 8/141 and 6/141 respectively. A second permuter campaign seeded from the
  score-6 `bits` form (5,207 iterations) found NO improvement - the seat swap is not
  reachable from the carrier-reuse basin.
- verdict: KILLED. Target's 1-holder is a pseudo distinct from every other value in the
  arm; only a fresh named local reproduces it.

## Frontier for s4+ (only if this candidate is bounced)

The function is at sandbox distance 0 with build_insns == target_insns and no measured
residual. Remaining items are dispositional:
 1. If layer 1 or the Judge holds that the duplicated write-out does not sit inside
    .claude/rules/duplicated-statement-into-arms.md (e.g. because the two copies store
    DIFFERENT values and so are not literally "the SAME statement"), the correct next
    outcome is a ruling-request on that single point - not a re-grind. The alternative
    spellings are exhausted and banked above (H14/H15/H17/H18); the honest floor without
    the flag is 8.
 2. Integration: func_80019568 still carries 5 regfix rules calibrated to the rule-era
    body. They must be retired by the operator/driver (`retire func_80019568`) with the
    full-build SHA1 verify. This session touched no rule file.

## [s3b-permuter] 2026-08-25 — permuter modality, second pass (after the driver
## discarded the first s3-permuter session for re-declaring the banned `enable` shape)

Chassis re-verified FIRST, not inherited: `memory/grind/func_80019568/candidate.c`
(the H16 default-initialised flag form) applied to src/code6cac.c and measured with
`sandbox func_80019568 --disable all` → **score 0, build_insns 141 == target_insns 141**
on today's HEAD chassis (rules_dropped 5, cheat_asm_stripped 28). Every number below is
from the same chassis.

H19 (KILLED): inverting the arm order — `if (rec[0] != 0) { o[0] = 4; o[2] = 0; bits = 0; }
else { ...valid... }` — reaches target's block layout and lets the `1` survive.
- mechanism: the store of `1` would then sit in the fall-through (else) block rather than
  the taken arm; if scan_loop's `maybe_never` bookkeeping (loop.c:918-930) or the basic-block
  ordering were what let target keep the `addiu $v0,$zero,1` in the loop, swapping which arm
  is the fall-through would expose it.
- probe: `tmp/grind/func_80019568/s3v/vA_inverted_arms.c` (bare-literal chassis + s3 tail),
  sandbox --disable all.
- result: **17, build_insns 142** — worse than the 8 of the un-inverted bare-literal form.
  The `li` is still hoisted (142) and the branch sense now diverges from target as well.
- verdict: KILLED. Block ordering is not the lever; the movable is created either way,
  confirming H14's structural reading (a literal's destination is a compiler temp, which
  satisfies scan_loop condition 2 unconditionally).

H20 (KILLED): giving the enable slot its own record pointer
(`s16 *e = &sp.output[i + 2]; ... *e = 1; / *e = 0;`) instead of `o[2]` changes the
1-source's pseudo class and/or the local-alloc seats.
- mechanism: a second DEST_REG giv leader for the i*2 scale group; the ledger's H5 result
  (one leader per scale group → benefit 0 → all_reduced=0) predicted this would ALSO add a
  walker, so this probe doubles as a re-test of H5's mechanism.
- probe: `tmp/grind/func_80019568/s3v/vB_sep_enable_ptr.c`, sandbox --disable all.
- result: **32, build_insns 146** — five extra instructions; the second pointer materialises
  its own walker exactly as H5/H7 predict, and the `li` is still hoisted.
- verdict: KILLED. Confirms H5's mechanism a second time from the opposite direction: the
  single-leader-per-scale-group property of the record-pointer chassis is load-bearing and
  fragile, and any extra address local costs ~5 instructions.

H21 (see result line below): a DIRECTED permuter campaign over the 1-source spelling and
its placement finds an honest 0 that needs no named flag local at all.
- mechanism: the two prior campaigns (`tmp/perm_80019568`, bare-literal seed, 13,728
  iterations; `tmp/perm_80019568_b`, bits-carrier seed, 5,207 iterations) were RANDOM
  searches from two chassis. Neither had directed alternatives at the exact divergence.
  This campaign (`tmp/perm_80019568_c`, label `directed-literal-placement`) seeds the
  bare-literal chassis with `PERM_LINESWAP` over the two arm bodies, `PERM_GENERAL` over
  four honest spellings of the 1-store (`o[2] = 1;`, `sp.output[i + 2] = 1;`, a
  block-scoped `s16 *e = o + 2; *e = 1;`, and an explicitly `(s16)`-cast literal), two
  spellings of the 0-store, and `PERM_RANDOMIZE` re-opened over the mask accumulate.

H22 (KILLED): an ADDRESS-typed named local in the valid arm
(`{ s16 *e = o + 2; *e = 1; }`, block-scoped inside the arm — the alternative the directed
campaign selected in its first same-score find) reaches target.
- mechanism: if REG_USERVAR_P anywhere in the store's dataflow were enough to defeat
  scan_loop's movable creation, an address-typed user local would do it as cheaply as a
  value-typed one, and with no family question at all (a pointer to a local slot is
  ordinary C).
- probe: `rejected/blockscope-enable-pointer-8.c`, sandbox --disable all.
- result: **8, build_insns 142** — bit-for-bit the bare-literal residual. Block-scoping the
  pointer inside the arm also avoids the extra walker H20 paid for (142, not 146), so the
  address local is free — it simply does nothing.
- verdict: KILLED. The user-variable property has to sit on the pseudo that HOLDS THE
  VALUE 1, not anywhere else in the store.

H23 (KILLED — and it sharpens the mechanism): a SINGLE-SET named value local declared
inside the valid arm (`s32 enabled = 1; ... o[2] = enabled;`) is the closer.
- mechanism: H16 offered two independent reasons the movable is not created —
  (a) REG_USERVAR_P kills scan_loop condition 2 (loop.c:695-701), and (b) the two
  non-consecutive sets fail `n_times_set == 1 || consec_sets_invariant_p`
  (loop.c:706-709). This probe separates them: a single-set user local satisfies (a) but
  not (b).
- probe: `rejected/armscope-single-set-named-local-8.c`, sandbox --disable all.
- result: **8, build_insns 142** — the `li` is hoisted exactly as for the bare literal.
- verdict: KILLED, and the mechanism attribution in H16 is hereby CORRECTED for the
  record: user-variable-ness ALONE is NOT sufficient. The operative gate is
  loop.c:706-709 — the value pseudo must have TWO NON-CONSECUTIVE SETS inside the loop.
  That is the same gate H6 (variable reuse) and H18 (the `bits` carrier) exploited; the
  difference is only WHICH pseudo carries them, and that difference is worth 6-8 points
  because a carrier shared with another live value mirrors the $v0/$v1 seats.
  CONSEQUENCE FOR DISPOSITION: the honest floor for this function is 8 unless the value
  written to `sp.output[i + 2]` comes from a named local with a default set at the top of
  the loop body and an override set inside the valid arm. There is no third spelling left
  at this divergence — literals (H14), computed comparisons (H15), joined stores (H17),
  borrowed carriers (H18), inverted arms (H19), address locals (H20/H22) and single-set
  locals (H23) are each measured and banked.

H24 (KILLED): a MEMORY-level default (`o[2] = 0;` unconditionally at the top of the loop
body, `o[2] = 1;` in the valid arm only, no flag variable anywhere) reproduces target.
- mechanism: this is the flag candidate's control structure with the default living in
  memory instead of a register — the same "default then override" program logic, no named
  local, therefore no family question of any kind. If GCC kept the `1` in the loop for it,
  the whole disposition problem would dissolve.
- probe: `rejected/default-store-then-override-13.c`, sandbox --disable all + objdump diff.
- result: **13, build_insns 141 == target 141** (the count matches only by accident: the
  extra top-of-loop `sh $zero,0x4($a2)` pays for the deleted in-loop `li`). The diff shows
  `li $t4,1` STILL hoisted into the prologue, a `sh zero,4(a2)` inserted at build[24] that
  target does not have, `nop` where target has `addu $v1,$zero,$zero` (build[28]), and the
  whole `lhu/addiu -1/sll/sra` chain seated in $v0 instead of target's $v1 — i.e. the seat
  mirror is back too.
- verdict: KILLED. A memory default does not give any REGISTER pseudo two sets, so
  loop.c:706-709 is untouched; and with no distinct flag pseudo, local-alloc mirrors the
  seats again. Both halves of the residual return.
- H21 RESULT (KILLED): the directed campaign ran **21,147 iterations / 651 s on 8 jobs**
  and produced exactly ONE output, `output-530-1`, at the SAME score as the base (530 in
  permuter units — the standalone-TU scale, not the sandbox scale). Its only content is
  the `{ s16 *e = o + 2; *e = 1; }` alternative plus cosmetic reformatting; measured in the
  engine it is 8/142 (see H22). Harvested with `--stop`; artifacts
  `tmp/grind/func_80019568/s3/campaign_c_meta.json`, `campaign_c_tail.log`,
  `campaign_c_base.c`.
- verdict: KILLED. Permuter search on this function is now exhausted across three distinct
  seeds and ~40k iterations (13,728 random from bare-literal + 5,207 random from the bits
  carrier + 21,147 directed from bare-literal with explicit alternatives at the exact
  divergence). No spelling other than a named local with two non-consecutive sets reaches 0.

## H25 [s4, 2026-08-25] — the s3b byte proof is chassis-independent
- statement: the s3b candidate body still reaches distance 0 on the current HEAD chassis,
  so the banked byte proof can be spent rather than re-derived.
- mechanism: the candidate's three constructs are all C-level and touch no rule surface;
  the only chassis coupling would be a change in code6cac.c's surrounding compilation
  context or in the pipeline flags, neither of which moved.
- probe: apply candidate.c to src/code6cac.c:365-499, run
  `sandbox func_80019568 --disable all`.
- result: score 0, target_insns 141, build_insns 141, rules_dropped 5.
- verdict: CONFIRMED.

## H26 [s4, 2026-08-25] — a permuter campaign can still contribute at floor 0
- statement: the mandated permuter modality has remaining value on this function.
- mechanism: decomp-permuter descends a nonzero score; with build_insns == target_insns
  and score 0 there is no residual and no gradient, so the search space it explores is
  empty by construction. Independently, the s3/s3b campaigns already returned the KILLED
  verdict across three seeds and ~40k iterations.
- probe: none run — launching one would burn ~30 min of wall clock to search for a
  spelling that scores below 0, which does not exist. Recorded as a reasoned negative so
  a later session does not re-spend the modality.
- verdict: KILLED (modality closed for this function in both directions).

## H27 [s4, 2026-08-25] — the layer-1 FAIL was a citation defect, repairable without a
##     construct change
- statement: the 2026-08-25 12:52 layer-1 FAIL can be cleared by re-filing C2 under the
  correct family rather than by respelling or removing any construct.
- mechanism: the FAIL text names exactly one defect — C2 "does not fit inside
  named-local-fake-exception's exact sanctioned scope and has no other precedent covering
  it". The 13:45 ruling supplied the covering precedent
  (duplicated-statement-into-arms) and held the flag itself to be ordinary C, so the
  premise of the FAIL is discharged by citation, not by code.
- probe: rewrote self_vet.md with FAMILY duplicated-statement-into-arms, the verbatim
  scope sentence from .claude/rules/duplicated-statement-into-arms.md:13-15, precedent
  .claude/rules/duplicated-statement-into-arms.md:63, and each of the family's five
  prerequisites answered against this diff; added the family name and the ruling pointer
  to the in-source FAKE annotation (comment-only, so codegen is untouched); re-measured.
- result: score 0, 141/141 after the annotation edit — the repair is byte-inert.
- verdict: CONFIRMED.
