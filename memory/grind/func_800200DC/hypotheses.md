# Hypothesis ledger — func_800200DC

## s1 (recon, 2026-07-27) — baseline + precise diff map

- H-s1-a: "The 14-insn residual is one monolithic rotation" — KILLED (refined).
  Normalized objdump diff (tmp/grind/func_800200DC/s1/diff_named.txt) shows TWO
  separable sub-rotations:
  - **Rotation A (6 insns):** dy dest at the subu — target `subu $v0,$v1,$v0`
    (dy reuses the subtrahend/arg0[1] reg), ours `subu $v1,$v1,$v0` (dy reuses
    the minuend/arg1[1] reg). Propagates: target mflo $v1 (arg2^2) + sll $s0,$v0,1;
    ours mflo $v0 + sll $s0,$v1,1; disc addu operands follow.
  - **Rotation B (8 insns):** clean $v1<->$a1 two-register swap — target: sq
    (call-result copy) `move $v1,$v0`, quotient `mflo $a1` + bgez/sra/addiu/sra
    rounding on $a1, sq re-use `subu $v0,$a2,$v1`; ours has sq in $a1 and the
    quotient in $v1.
- H-s1-b (OPEN, mechanism lead): in Rotation A's disc compute, target burns
  **$t1** for the arg3*dy2 mflo (`mflo $t1; addu $v1,$v1,$t1`) while ours reuses
  the freed v-reg (`mflo $v1; addu $v1,$v0,$v1`). If MIPS REG_ALLOC_ORDER is
  low-first, target choosing $t1 over free $v0/$a0/$a1/$t0 implies those were
  occupied in target's RTL at that point — i.e. dy (in $v0) may still be LIVE
  past the sll in the original's RTL, or the allocno order differs. This is the
  concrete question for an ALLOCDBG/-da greg dump probe.
- H-s1-c: m2c shape (tmp/grind/func_800200DC/s1/m2c.c) confirms structure is
  fully correct: `var_a0 = 0x12C` hoisted before the disc>=0 test (GCC puts it
  in the bltz delay slot — our else-arm `a0 = 300;` already produces this), and
  the arg4[2] store is a cross-jumped shared tail (ours matches at 168=168).
  No structural lever remains; the gap is 100% RA.
- H-s1-d: no sibling/duplicate analog exists — tools/find_duplicates.py full
  scan (206 pairs) has zero entries involving func_800200DC.

## [s1] The 14-insn residual is one monolithic coupled rotation
- mechanism: register allocation across the mid-function sqrt call
- probe: normalized objdump diff of sandbox .o vs asm/funcs/func_800200DC.s (syntax + maspsx nops filtered)
- result: Residual decomposes into Rotation A (6 insns: dy dest $v0 target vs $v1 ours, propagating through the disc compute) + Rotation B (8 insns: clean $v1<->$a1 swap of sq and the division quotient after the second call). No insert/delete/reorder diffs.
- verdict: KILLED

## [s1] Structure may still have a gap (delay-slot a0=300 preset, shared tail store)
- mechanism: m2c reconstruction vs our C
- probe: m2c --valid-syntax on target asm; compare against src form
- result: m2c shape matches our C exactly: a0=300 preset lands in the bltz delay slot from our else-arm assignment; arg4[2] tail store is cross-jump-shared; both sides 168 insns. Gap is 100% RA.
- verdict: CONFIRMED

## [s2] Rotation B roots in a reused variable: disc = sqrt(disc<<10)
- mechanism: reusing `disc` for the call result merges the disc and sq pseudos
  into one high-ref allocno (≈6 refs) that wins $v1 in global-alloc, exactly
  matching target where disc AND sq AND the a2-sq subu all use $v1
- probe: replace `s32 sq = func_8007E11C(disc << 10);` with
  `disc = func_8007E11C(disc << 10);` (a2 decl moved up, assignments after)
- result: sandbox 14 → 6; the entire 8-insn Rotation B ($v1<->$a1 sq/quotient
  swap) is gone; residual is purely Rotation A
- verdict: CONFIRMED

## [s2] dy's dest-reg flip is reachable by respelling the subtraction
- mechanism: named-temp splits / split-init / load-order around dy
- probe: y1,y0 named temps (14); y0-first (17); split-init dy (14)
- result: named splits along expression boundaries are RTL-neutral; load-order
  flip perturbs the schedule. dy's $v1 preference comes from set_preference
  taking the MINUEND's reg (global.c XEXP(src,0)) — unreachable by spelling
  the subtraction differently while keeping arg1[1] first
- verdict: KILLED

## [s2] Named-temp/CSE respellings of dy2, a2, product, quotient move the allocation
- mechanism: pseudo renumbering via named locals or CSE-owned temps
- probe: named t=(a2+sq)*dist (14); named q=.../dy2 (14); inline arg2<<5 both
  arms (14); inline dy*2 all sites (39 — CSE fails across the call); inline dy
  (15); sum-order sq+a2 (14 — canonicalized); dy2*arg3 (7 — mult operand order
  NOT canonicalized, 1 worse); split-init disc (16); unconditional a0 preset
  (29); arm swap dy!=0 (67)
- result: every same-boundary respelling is neutral; every statement-structure
  change diverges. Rotation A does not yield to source-level renumbering of
  the existing statement set
- verdict: KILLED

## [s2] (OPEN — next frontier) Rotation A flips when the arg2^2/arg3*dy2 mult
   temps are allocated BEFORE dy and land $v1/$t1
- mechanism: ours allocates dy (allocno 87, prefers $v1) before temp 106; target
  must allocate the temps first (or give dy a $v1 conflict), evicting dy to $v0.
  The greg dump shows 87's priority sits between 107 and 106; a small ref/length
  change on either temp's pseudo flips the order. $t0 is the reload spill reg
  ("Spilling reg 8"), so $t1 is the natural temp landing spot once low regs fill
- probe: permuter campaign on the score-6 base (directed PERM_* around the disc
  statement family), or BB2_ALLOC_DEBUG instrumented cc1 to read the exact
  allocno_compare inputs for 87/106/107 and derive which C change raises the
  temps' priority without adding insns
- verdict: (open)

## [s1] A sibling/duplicate function could donate a matched allocation shape
- mechanism: byte/structure similarity scan
- probe: tools/find_duplicates.py full scan (206 pairs) + tmp/duplicates_leads.txt grep
- result: Zero entries involving func_800200DC in either output.
- verdict: KILLED

## [s2] Rotation B roots in a reused variable: the original wrote disc = func_8007E11C(disc << 10) instead of a fresh sq local
- mechanism: merging disc+sq into one pseudo (105) gives it ~6 refs; it wins $v1 in global-alloc, matching target where disc, the call-result copy, and the a2-sq subu all sit in $v1; the arm-1 quotient then falls to $a1 as in target
- probe: single sandbox run with the reuse spelling
- result: sandbox 14 -> 6; Rotation B (8 insns) fully closed; residual diff is purely Rotation A
- verdict: CONFIRMED

## [s2] dy's dest-register tiebreak (Rotation A root) flips via respelling the subtraction (named temps, split-init, load order)
- mechanism: GCC 2.7.2 global.c set_preference takes XEXP(src,0) for non-copy sets, so dy always prefers the MINUEND's reg ($v1); dump confirms 87 prefers 3 and does not conflict with $v0
- probe: y1/y0 named temps; y0-loaded-first; sanctioned split-init dy=arg1[1]; dy-=arg0[1]
- result: 14 / 17 / 14 — named splits RTL-neutral, load-order flip perturbs the schedule (+3); preference unreachable from the subtraction spelling
- verdict: KILLED

## [s2] Named-temp or CSE-owned respellings of dy2/a2/product/quotient renumber allocnos enough to flip either rotation
- mechanism: pseudo birth-order and ref-count changes via naming, inlining, split-init, operand order, arm order
- probe: 10 sandbox probes: named t (14), named q (14), inline arg2<<5 (14), inline dy*2 (39), inline dy (15), sq+a2 (14), dy2*arg3 (7), split-init disc (16), unconditional a0=300 preset (29), arm swap dy!=0 (67)
- result: every same-boundary respelling neutral; every statement-structure change diverges; named dy2 is load-bearing (CSE fails to keep one call-crossing sll); m2c's pre-if a0 preset is post-reorg appearance only
- verdict: KILLED

## [s3] Rotation A roots in dy's set_preference minuend pref; killing it in C flips the whole rotation
- mechanism: set_preference gives dy the minuend local's hard reg ($v1); find_reg's
  preference-override steals dy from its natural $v0; staging the minuend through a
  cross-block pseudo (unrenumbered at scan time) records no pref
- probe: FINDREG/ALLOCDBG instrumented dumps at score-6 base + staged variants,
  sandbox-measured: disc-carrier dy-only (7), disc-carrier both-minuends (5),
  y1-relay both-minuends (0), neg-carrier (26)
- result: CONFIRMED — dy=$v0, arg2^2=$v1 and the reload-$t1 all cascade from the
  pref-kill; the y1 form reaches sandbox 0
- verdict: CONFIRMED

## [s3] Same-statement-set respellings can never flip Rotation A (search-space kill)
- mechanism: regs_someone_prefers[dy] is structurally empty (all dy-conflicting
  allocnos are call-crossing -> prune empties their prefs; arg2^2 is LO-preferred-
  class -> prune wipes merged GP prefs), so dy's $v1 override always wins in any
  spelling that keeps the raw `arg1[1] - arg0[1]` minuend shape
- probe: global.c source analysis (allocno_compare/find_reg/set_preference/
  expand_preferences/prune_preferences) + FINDREG dumps of the actual sets
- result: explains every s1/s2 neutral probe; the only C-reachable vector is the
  minuend's pseudo-vs-local status (or an owner-sanctioned relay)
- verdict: CONFIRMED

## [s3] The residual-5 ($a0 arm-2 quotient) is disc's call-arg {4} pref flowing down
   the dy->dy2->divres->quotient expand-merge chain
- mechanism: combine folds `disc << 10` into the call-arg set giving disc pref {4};
  disc dies at the staged dy-subu -> merge into dy -> chain to the quotient, whose
  override takes free $a0 (target: pref-less scan -> $v1)
- probe: FINDREG 120/104 dumps; y1-relay (pref-less carrier) removes it -> 0
- result: CONFIRMED; no existing $v1-colorable carrier without the {4} exists
  (disc is the only one; neg mis-colors to $a1)
- verdict: CONFIRMED

## [s3b] a2 (existing var) can carry the staged minuends into $v1
- mechanism: empty pref set (arg2 unrenumbered at set_preference), first
  segment overlaps arg1-ptr blocking the {5} leak
- probe: sandbox + FINDREG/ALLOCDBG (a2c.*)
- result: 8; downstream allocation all-target but carrier lands $a2 — pass-0
  avoids $v1 (someone_prefers = disc's {3,4,7}; a2 conflicts with disc via the
  arm-2 (a2-disc) use) and the disc conflict makes $v1 unreachable in ANY order
- verdict: KILLED

## [s3b] neg's $a1 landing is pass-0 politeness (fixable by allocation order)
- mechanism: assumed someone_prefers blocked $v1
- probe: FINDREG negc_fr77 (judge-mandated explanation)
- result: someone_prefers EMPTY; hard conflict {3} — globalizing neg removes
  the local-pool $v1 blocker, the zero-arm quotient-1 local takes $v1 inside
  neg's range. Self-defeating, not order-fixable
- verdict: KILLED

## [s3b] Restoring the zero-arm equilibrium (quot1 split through a0) rescues
   the neg carrier
- mechanism: a0-var (global, $a0 home) removes quot1 from the local pool;
  target zero-arm bytes identical under the split spelling
- probe: sandbox 18 + nega0 dumps
- result: zero arm matches, but the else-arm products become locals@$v1 ->
  literal {3} set_preferences on both /32 temps via divmod single_set edges ->
  $v1 stolen, disc loses {3}, cascade
- verdict: KILLED

## [s3b] divmodsi4 is invisible to set_preference/expand_preferences
- mechanism: parallel with 2 sets assumed to fail single_set
- probe: rtlanal.c single_set source
- result: single_set IGNORES REG_UNUSED-dest sets (line 601) — the dead mod
  half always qualifies, so divmods ARE full pref/merge edges (dividend's
  local renumbering leaks into the quotient's prefs)
- verdict: KILLED (corrects the s2/s3 chain model)

## [s3b] (OPEN — the load-bearing unknown) disc's {3} pref provenance in
   intact-zero-arm forms
- mechanism: unexplained by set_preference on disc's own sets or by any
  modeled expand merge; correlates perfectly with zero-arm local-neg@$v1
  across 6 dumped forms; disc{3} is what shields $v1 (someone_prefers) and
  wins disc $v1
- probe next: A/B minimal experiments on the floor-5 base (flip zero-arm
  neg/denom statement order so local-neg colors off $v1; predict disc loses
  {3} and score breaks) to localize the channel; if inconclusive, surface an
  owner request to extend the instrumented cc1 with a set_preference/expand
  event log (tools/ change = owner-only)
- verdict: (open)

## [s3b] (OPEN) {3}-injection into the arm-2 chain closes the last 5
- mechanism: fr120={4} only; find_reg override scans low-first, so {3,4} on
  120 picks $v1 (target) before $a0. The chain 115->116->120 is fed only by
  unrenumbered globals today; a spelling that makes one operand a
  $v1-renumbered block-local would inject {3} via divmod/mult set_preference
- probe next: re-block the (a2-disc) temp / arm-2 dividend into a local scoped
  so local-alloc colors it $v1 (zero-arm and arm-1 untouched); measure
- verdict: (open)

## [s3] Existing var a2 can carry the two staged minuends into $v1
- mechanism: empty pref set (arg2 unrenumbered at set_preference time); first stage segment overlaps arg1-ptr, blocking the {5} entry-copy leak
- probe: sandbox + FINDREG/ALLOCDBG dumps (s3/a2c.*)
- result: 8. Downstream allocation ALL-target (dy=$v0, /32 temps $a1/$v1, disc $v1) but the carrier lands $a2: find_reg pass-0 avoids $v1 (regs_someone_prefers = disc's {3,4,7}; a2 CONFLICTS with disc because its real segment spans arg2<<5 through the arm-2 (a2-disc) use) and that same conflict makes $v1 unreachable under any allocation order
- verdict: KILLED

## [s3] neg's prior $a1 landing (26) was pass-0 politeness, fixable by lowering neg's allocation priority below disc's
- mechanism: assumed someone_prefers blocked $v1 at neg's find_reg turn
- probe: FINDREG dump of the reconstructed neg-carrier form (judge-mandated explanation; s3/negc_fr77.txt)
- result: someone_prefers is EMPTY; the blocker is a HARD conflict {3}: globalizing neg removes it from the zero-arm local pool, and local-alloc then gives the zero-arm quotient-1 local $v1 inside neg's live range (intact forms: local-neg@$v1 blocks quot1 to $a0 = target). neg-carrier is structurally self-defeating, not order-fixable
- verdict: KILLED

## [s3] Restoring the zero-arm equilibrium by splitting quotient-1 through a0 (a0=neg*dx; a0=a0/denom) rescues the neg carrier
- mechanism: a0 is global with an $a0 home, so the quotient leaves the local pool while emitting target-identical zero-arm bytes (mflo $a0/div/mflo $a0/sw)
- probe: sandbox + nega0 dumps (s3/neg_a0split_18.diff.txt)
- result: 18. Zero arm now matches, but the (a2+/-disc)*dist products become block-locals colored $v1 (nega0 locals 94,98 in 3) and both /32 rounding temps acquire LITERAL {3} set_preferences through the divmod edges (greg ';; 112/118 preferences: 3'), stealing $v1; disc drops to {4,7} and lands $a3, the carrier lands $a1
- verdict: KILLED

## [s3] divmodsi4 parallels are invisible to set_preference/expand_preferences (2-set parallel fails single_set)
- mechanism: assumed structural single_set failure
- probe: rtlanal.c single_set source read
- result: single_set IGNORES sets whose dest carries REG_UNUSED (rtlanal.c:601) and the dead mod-half always does — divmods ARE full preference/merge edges, leaking the dividend's local-alloc renumbering into the quotient's prefs. Corrects the prior 'dy->dy2->divres->quotient' chain model: the floor-5 {4} actually flows disc{4} ->(a2-disc) subu death-merge-> 115 ->mult-> 116 ->divmod-> 120, and dy/dy2 never carry {4} (the dy-relay merge precedes disc's {4} acquisition in expand's single forward pass)
- verdict: KILLED

## [s3] The target coloring is a whole-function equilibrium anchored on disc holding pref {3} and the zero-arm neg staying a $v1 block-local
- mechanism: disc{3} wins disc $v1 AND shields $v1 at the arm-1 /32 temp's ord-0 turn via regs_someone_prefers (that temp conflicts with disc since disc lives through arm-1 into arm-2); local-neg@$v1 blocks the zero-arm quotient off $v1 and correlates with disc{3} across all 6 dumped forms
- probe: greg pref tables + FINDREG across staged2/a2c/y1f (intact: disc {3,4,7}) vs negc/nega0 (broken: disc {4,7})
- result: confirmed as an invariant; consequently only a variable that is global, dead outside the two stage segments, and absent from every local pool can host the stages — no existing variable qualifies (disc=5 {4}-leak, a2=8 conflict, neg=26/18 pool-membership, dx/dz/dist/dy/dy2/a0 wrong stage-lw home bytes, denom pool-membership)
- verdict: CONFIRMED

## [s4] {3}-injection into the arm-2 chain closes the last 5 (s3b OPEN hypothesis)
- mechanism: an existing-variable dest on the arm-2 (a2-disc) subu receives
  disc's prefs (incl. {3}) via expand_preferences at disc's death and passes
  them down the mult/divmodsi4 pref edges to the /32 quotient; find_reg's
  low-first override then picks $v1 (target) over $a0
- probe: directed permuter campaign from the floor-5 base (tmp/perm_200DC_s4);
  hand refinement measured natural=5, fresh named temp=5, disc-reuse=2,
  dy-reuse=0 (sandbox --disable all, 168/168, 14 rules stripped)
- result: dy-reuse form reaches sandbox 0; independent rule-free pipeline diff
  vs target empty; candidate in src/
- verdict: CONFIRMED

## [s4] disc-reuse as the arm-2 temp closes the function
- mechanism: same pref-merge route, but the temp coalesces into disc's pseudo
  (105) whose home is $v1
- probe: permuter score-10 find applied to src; sandbox = 2 (subu/mult dest
  $v1 vs target $v0; quotient/rounding cluster all match)
- result: quotient fixed but temp reg wrong; superseded by dy-reuse
- verdict: KILLED

## [s4] The zero-arm denom/neg decl-order flip is load-bearing for the closure
- mechanism: suspected local-pool rotation change (s3b frontier A/B)
- probe: dy-reuse form with original neg-first order vs flipped
- result: 0 both ways — flip not load-bearing; original order kept
- verdict: KILLED
