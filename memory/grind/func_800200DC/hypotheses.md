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
