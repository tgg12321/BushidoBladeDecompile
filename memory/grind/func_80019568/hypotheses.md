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
