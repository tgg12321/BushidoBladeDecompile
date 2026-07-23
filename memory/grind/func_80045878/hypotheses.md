# Hypothesis ledger — func_80045878

Floor: 10 (sandbox --disable all; build_insns 107 vs target 108). Verdict C.
Two coupled residual gaps (see evidence.md). m2c target shape:
tmp/grind/func_80045878/s1/m2c_target.c.

## H-A (Gap A — else recompute) — CONFIRMED mechanism, residual = scheduling
Statement: target recomputes `addiu s3,s2,3` (=a0+3) at the END of the else arm
(0x800458F8); our build folds it into the pre-if `s3=a0+3` (cse.c) => 107.
Probe s1: arm-split (set s3 in BOTH the taken arm and the else arm, drop the
pre-if init). RESULT: build_insns 107->108 (recompute MATERIALIZES) but score
10->11 — sched1 hoists the fresh single-def recompute 3 slots early (build 2c6c,
before `li v0,-1; sh; sh`) because s3 feeds the next call arg. Target keeps it
last. => Gap A insn is reachable; residual is ANCHORING it last.
Next probe: a form that keeps the recompute AND anchors it after the two stores
without a barrier — e.g. keep s3 live-in (redefinition anti-deps hold position)
while structurally defeating the cse fold; or reorder so the else def is not the
launch-priority winner. Read sched.c launch-priority + cse.c fold conditions.

## H-B (Gap B — tail base copy) — biggest chunk (~9 diffs)
Statement: target's final store block copies s1 into a caller-save base
(`addu v0,s1,zero`), stores through v0, uses s2 (=a0) DIRECTLY for the a0
stores, holds scratch (a0+3, 0x8000) in v1. Our build stores through s1 with an
extra `move v0,s2`. Single-set `s16 *p = s1` alias copy-props away (WIP-ruled).
Mechanism: local-alloc assigns a short-lived tail-base pseudo to caller-save v0;
need s1 kept as the long-lived record pseudo while a separate tail base
materializes and survives copy-prop.
Next probe: keep s1 referenced/live past the tail base copy, or multi-set the
base per param-reuse-base-copy-cse-canon; inspect .greg dump to see s1's pseudo
disposition. NOT yet attempted beyond the ruled-out single-set alias.

## H-C (param types) — KILLED (s2)
m2c infers (s16 arg0, s16 arg1, u32 *arg2). Tried: score 10->43, build 107->112.
s16 params inject sign-extends on every a0/a1 use; target has ZERO (params flow
straight into s2/s5). Original params are register-width s32. Dead axis.

## A+B COUPLING (frontier item 3) — KILLED (s2)
Hypothesis: correcting types / fixing one gap shifts RA for both. FALSIFIED:
applied the arm-split (Gap A materialized, build 108) and disassembled the tail
— byte-identical to HEAD's tail (base=s1, move v0,s2). Gap A's presence does not
touch Gap B. The gaps are INDEPENDENT; each needs its own solution.

## [s4] Directed permuter on a clean single-function target.o closes one/both residuals (frontier item 1) — KILLED
Statement: a directed permuter over the whole function (clean single-fn target.o at
offset 0) finds a pure-C spelling that flips the Gap-B base-copy choice and/or anchors
the Gap-A recompute last, reaching sandbox 0.
Mechanism (premise): both residuals are RA/sched ties; permuter's structural mutations
explore orderings/spellings outside the hand-search space.
Probe: FOUR permuter chassis, --stack-diffs, --stop-on-zero, fresh-seed discipline:
HEAD-random (base 580, 45.9k iters), arm-split-random (540, 45k), arm-split-perm_inline=0
(540, 45k), and a DIRECTED chassis (this session): PERM_LINESWAP over the Gap-A else tail
+ PERM_GENERAL recompute spelling, AND PERM_LINESWAP over the 6 Gap-B tail stores +
PERM_GENERAL for s1[11] — exhaustively enumerating every ordering of BOTH residual
regions (~26k combos, 21.5k iters).
Result: every chassis plateaus (250 HEAD / 210 arm-split); NO chassis reaches a
sub-plateau find, NO zero. Best forms are re-finds of the known arm-split plateau +
permuter synthetic temps (inline_fn/new_var), all at the same 210 — noise-equivalent,
not closing forms. C-statement ordering/spelling is NOT the lever for either coin-flip.
verdict: KILLED. Permuter modality measured dead. Both residuals are decided by cc1
local-alloc/sched1 tie-breakers below the reach of any C-source mutation the permuter
produces — consistent with the s3 structural verdict. The matching C (if it exists per
difficult-is-not-impossible) is NOT reachable by permuter's C-source search; the remaining
avenue is a novel structural insight not yet found, or (given ALL sanctioned axes —
structural s2/s3 + permuter s4 — now measured dead and the function byte-matches only via
10 regfix rules a few insns short in pure C) the endgame-lock disposition
([[endgame-lock-disposition]] / owner policy 2026-07-20) at owner discretion.

## [s3] Structural levers exhausted for BOTH gaps — measured dead; residuals are RA/sched coin-flips
- Gap B re-derived at greg RTL: ONE callee->caller copy, flipped choice.
  Fork `move v0,s2` (a0 dies mid-tail, base stays s1); target `addu v0,s1,zero`
  (s2 stored direct, base copied). cse materializes shared (HI)s2 (insn 213).
- s1[11]=s3 (reuse s3 / pressure probe): build 107->105, score 10, wrong bytes
  (target recomputes; s3 dead pre-tail). KILLED.
- Tail-store reorder: build 107, score 10 == HEAD (cse re-clusters (HI)s2
  order-insensitively). KILLED.
- Register-pressure at .L800459DC join: no legitimate hook (tail uses only
  s1/s2/s5+consts). Dead axis. KILLED.
- arm-split recompute (Gap A) sched-early is genuinely higher priority (feeds
  next call arg), NOT a tie; no clean structural lever lowers it. worth ~1
  (10->9) if anchored. -> directed permuter.
- verdict: KILLED (structural modality exhausted). Next modality = directed
  permuter on clean single-function target.o (frontier item 3).

## [s2] Gap A is a cse deletion, Gap B is a local_alloc live-range split — both nailed at RTL, clean structural levers measured dead
- Gap A: else recompute = RTL insn 74 `(set reg75 (plus reg72 3))`; cse deletes
  it (reg75 already holds reg72+3, reg72=a0 preserved across else). Pre-if init
  (sched anchor) is what makes cse fold => catch-22. No value-neutral spelling
  survives (cse folds all a0+3 spellings pre-availability-check).
- Gap B: fork truncates a0->HI for the `s1[11]=a0+3` store then cse reuses that
  (HI)a0 for the 3 a0 stores (base stays s1). Target adds 3 to full s2, stores
  s2 directly, copies base to v0. The v0 base copy is a local_alloc live-range
  split of the 2-pred join block; cse copy-propagates any C-level `p=s1`.
- verdict: KILLED (structural modality's clean levers exhausted; NOT proven
  impossible — see frontier for the remaining non-structural avenues).

## [s1] Target recomputes `addiu s3,s2,3` (=a0+3) at the end of the else arm; our build folds it into the pre-if s3 init (cse.c), leaving build 1 insn short (107 vs 108).
- mechanism: cse.c: pre-if `s3=a0+3` dominates the else; a0 and s3 (callee-save) are unchanged across the else calls, so the else `s3=a0-(-3)` is a redundant available expression and is folded.
- probe: Arm-split: drop the pre-if init, set s3=a0+3 in the taken arm and s3=a0-(-3) in the else arm; sandbox --disable all.
- result: build_insns 107->108 (recompute materializes) but score 10->11: sched1 hoists the fresh single-def recompute 3 slots early (build 2c6c, before li v0,-1; sh; sh) because s3 feeds the next call arg; target keeps it last (0x800458F8).
- verdict: CONFIRMED

## [s1] Target's final store block copies s1 into a caller-save base (addu v0,s1,zero), stores through v0, uses s2(=a0) directly, and holds scratch (a0+3, 0x8000) in v1; our build stores directly through s1 with an extra move v0,s2 (~9 of the 10 diffs).
- mechanism: local-alloc assigns a short-lived tail-base pseudo to caller-save v0; a single-set `s16 *p=s1` alias is copy-propagated away (s1 dead after) so no base copy materializes.
- probe: Not re-run this session (WIP already ruled out single-set alias at 107/floor 10); mapped only.
- result: Confirmed as the dominant residual chunk via objdump alignment of the tail store block against target.
- verdict: CONFIRMED

## [s2] Narrowing the signature to m2c's inferred types (s16 a0, s16 a1, u32 *a2) shifts RA to close the gaps.
- mechanism: narrower param types change value-tracking and RA pressure.
- probe: Changed signature + cast a2 uses; sandbox --disable all.
- result: score 10->43, build_insns 107->112. s16 params inject sign-extends (sll/sra) on every a0/a1 use; target has ZERO sign-extends (a0/a1 flow straight into s2/s5). Original params are register-width s32.
- verdict: KILLED

## [s2] Gap A and Gap B are coupled: materializing the else recompute (Gap A) shifts RA and also fixes the tail base copy (Gap B).
- mechanism: frontier item 3 premise: one structural change alters allocation for both.
- probe: Applied arm-split (Gap A materialized, build 108) and disassembled the tail.
- result: Tail is BYTE-IDENTICAL to HEAD's tail (base=s1, move v0,s2, sh v0/s5 stores). Gap A presence does not touch Gap B. The two gaps are fully independent.
- verdict: KILLED

## [s2] Gap B tail base copy (addu v0,s1,zero) can be materialized by keeping s1 the record pseudo while a separate C-level tail base pointer survives copy-prop.
- mechanism: local-alloc gives a short-lived tail-base pseudo a caller-save; a kept-live spelling forces the copy (param-reuse-base-copy-cse-canon).
- probe: s16 *p=s1 for the 5 HI stores + s1 referenced in the trailing word store (mixed, both live); also SI-temp s32 t=a0+3;s1[11]=t.
- result: Mixed p/s1: score 10, build 107 (cse copy-propagates p=s1 regardless; copy vanishes, == WIP single-set result). SI-temp: build 106 (drops the (HI)a0 truncation) but score stays 10. Target's base copy is a local_alloc live-range split of the 2-pred join block, not a C-level pointer copy.
- verdict: KILLED

## [s2] Gap A: our fork drops the else recompute `addiu s3,s2,3` (folded), leaving build 1 insn short (107 vs 108).
- mechanism: cse deletes RTL insn 74 (set reg75 (plus reg72 3)) because reg75 already holds reg72+3 (reg72=a0 preserved across the else calls, reg75 untouched); the pre-if init that anti-dep-anchors the redefinition for sched is exactly what feeds cse the available expression.
- probe: cc1 -da dumps: insn 74 present in dump.i.rtl, DELETED in dump.i.cse (insn 71 links straight to code_label 76). Confirmed no value-neutral spelling escapes (cse constant-folds a0-(-3), (a0+1)+2, etc. to (plus a0 3) before the availability check).
- result: Root cause confirmed. Structural spellings that keep the pre-if init all fold; dropping it (arm-split) materializes the insn but sched1 hoists it 3 slots early (score 11). Catch-22 between cse-fold and sched-anchor stands.
- verdict: CONFIRMED

## [s3] Keeping s3 (=a0+3) live into the tail by using it for s1[11] raises register pressure at the 2-pred join and triggers the target's s1->v0 base-copy split.
- mechanism: local-alloc splits a callee-save pseudo into a caller-save block-local copy under pressure; forcing s3 (reg19) live across .L800459DC adds a live value.
- probe: s1[11] = s3; (instead of a0+3); sandbox --disable all + greg reasoning.
- result: build_insns 107->105, score stayed 10. Reuses s3 (WRONG bytes: target recomputes addiu v1,s2,3; s3 is REG_DEAD before the tail) and build got SHORTER, not the extra base copy. No split triggered.
- verdict: KILLED

## [s3] Reordering the tail store statements breaks cse's clustering of the three (subreg:HI s2) a0 stores, stopping the shared (HI)s2 materialization (greg insn 213) that keeps the base in s1.
- mechanism: cse materializes the repeated (HI)s2 into one reg (v0) and reuses it, so a0 dies mid-tail and s1 stays as base with a `move v0,s2`.
- probe: Interleaved order s1[2];s1[11];s1[10];word;s1[8];s1[4]; sandbox --disable all.
- result: build_insns 107, score 10 — IDENTICAL to HEAD. cse re-clusters the HI stores and re-materializes the shared (HI)s2 regardless of source order. Statement order is not a lever for this CSE.
- verdict: KILLED

## [s3] Legitimately raising register pressure across the .L800459DC 2-pred join forces s1's tail use into a block-local caller-save copy (the target's addu v0,s1,zero).
- mechanism: more simultaneously-live values at the join push local-alloc to split s1's live range.
- probe: Enumerated the tail's live set: it consumes only s1(base), s2(a0), s5(a1) + constants. Any additional live value would have no semantic purpose.
- result: No legitimate hook exists — a 4th live value at the join would be a dead (cheat) value. Dead axis for the structural modality.
- verdict: KILLED

## [s3] Arm-split (set s3 in both arms, drop pre-if init) materializes the Gap-A else recompute at the target's last position, closing Gap A.
- mechanism: no pre-if dominating def => cse cannot fold the else recompute; scheduler should place it after the two else stores.
- probe: Applied rejected/armsplit form; sandbox --disable all + disassembly of the else block.
- result: build_insns 108 (recompute present), score 11. The recompute lands at build 2c6c (position N-3, before li v0,-1; sh v0,8(s1); sh zero,6(s1)); target keeps it LAST (0x800458F8). It is NOT a priority tie: the fresh single-def recompute feeds the next block's call arg (a0=s3), so sched1 gives it genuinely higher launch priority. No clean structural lever lowers that priority without changing bytes.
- verdict: KILLED

## [s4] A directed permuter over a clean single-function target.o finds a pure-C spelling that flips the Gap-B base-copy choice and/or anchors the Gap-A else recompute last, reaching sandbox 0 (frontier item 1).
- mechanism: Both residuals are RA/scheduling ties; permuter's structural mutations were expected to explore orderings/spellings outside the hand-search space. Directed chassis: PERM_LINESWAP over the Gap-A else tail {s1[4]=-1;s1[3]=0;s3=recompute} + PERM_GENERAL over the recompute spelling (a0--3 / a0+3 / (a0+1)+2), AND PERM_LINESWAP over the 6 Gap-B tail stores + PERM_GENERAL for s1[11] value (a0+3 / live s3) — exhaustively enumerating every ordering of BOTH residual regions (~26k combos).
- probe: Built clean workspace tmp/perm_80045878_dir (full-TU honest-pipeline base.c, func region extracted, clean target.o from asm/funcs/func_80045878.s at offset 0, prelude minus .set gp=64 for r3000). Launched via permuter_campaign.py --stack-diffs --stop-on-zero -j8; waited one in-turn ~8.5-min window; harvested+stopped.
- result: 21519 iterations, base_score 540, BEST permuter-score 210 (== plain arm-split floor), NO zero, NO sub-plateau find. Corroborated by the 3 prior random chassis (HEAD base 580 best 250; arm-split base 540 best 210; arm-split perm_inline=0 base 540 best 210), ~135k iters combined, none reaching zero. Best forms across all chassis are re-finds of the arm-split plateau + permuter synthetic temps (inline_fn/new_var), all noise-equivalent at 210, none a closing form.
- verdict: KILLED

## [s5] A fresh-seed reseed of the arm-split chassis (5th chassis) escapes the 210 plateau to a sub-plateau / zero form — KILLED
Statement: per fresh-seed discipline, a genuinely fresh seed on the closest chassis
(arm-split, base 540 / floor 210) explores a novel basin that flips the Gap-B base
copy or anchors the Gap-A recompute last, reaching a sub-210 (ultimately zero) form.
- mechanism (premise): s4's two arm-split runs may have exhausted only their seed's
  basins; a fresh seed samples a structurally distinct trajectory of the same random
  mutation set.
- probe: Relaunched tmp/perm_80045878_arm (arm-split base.c, no PERM annotations = pure
  random mutation) via permuter_campaign.py launch --stop-on-zero -j8, fresh seed;
  waited in-turn (~7-min window); harvested --stop.
- result: 13,769 iterations, base_score 540, best_new_score 210, 147 finds, MIN score
  210 — NO sub-plateau, NO zero. Byte-for-byte the same plateau as s4's two arm-split
  runs. A THIRD fresh arm-split seed confirms the basin is dry.
- verdict: KILLED. Permuter modality re-confirmed dead across a 5th chassis. Combined
  with s2/s3 (structural) and s4 (4 chassis), EVERY sanctioned grind axis is now
  measured dead for a committable COMPLETED-C form. scan_hand_coded = LOW 0/8 (ordinary
  GCC RA/sched output) rules out canonical-asm. No pure-C sandbox-0 form exists
  (reachability/RA-class wall, sibling of func_800611A4). => OWNER-ESCALATION filed
  (docs/grind/decisions.md, 2026-07-23); disposition INCOMPLETE-owner-accepted per
  endgame-lock-disposition; outcome owner-gated.

## [s5] A fresh-seed reseed of the arm-split chassis (5th chassis) escapes the 210 plateau to a sub-plateau/zero form.
- mechanism: s4's two arm-split runs may have exhausted only their seed's basins; a fresh seed samples a structurally distinct trajectory of the same pure-random mutation set over the Gap-A-materializing (build 108) chassis.
- probe: Relaunched tmp/perm_80045878_arm (arm-split base.c, no PERM annotations = pure random) via permuter_campaign.py launch --stop-on-zero -j8, fresh seed; waited in-turn ~7-min window; harvest --stop.
- result: 13,769 iterations, base_score 540, best_new_score 210, 147 finds, MIN score 210 - NO sub-plateau, NO zero. Byte-for-byte the same plateau as s4's two arm-split runs. A third fresh arm-split seed confirms the basin is dry.
- verdict: KILLED

## [s5] func_80045878 qualifies for canonical-asm (original was hand-written asm).
- mechanism: endgame-lock-disposition: STRONG hand-coded signals (S1/S2/S6) would authorize inline-asm as the finished form.
- probe: tools/scan_hand_coded.py --single func_80045878 (read-only).
- result: tier=LOW score=0/8 (108 insns, 7 spills, 11 regs; no S1 multu pacing, no S2 empty branch, no S4 front-load burst, no S5 sibling cluster, no S6 BIOS jumptable, no S7 unsaved $sN, no S8 redundant mask). Ordinary GCC 2.7.2 RA/sched output; canonical-asm unsupportable.
- verdict: KILLED
