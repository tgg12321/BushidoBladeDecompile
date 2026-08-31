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

## [s6] SYNTHESIS — frontier reset. The score-10 chassis was the wrong chassis.
Everything below supersedes the s3/s4/s5 framing ("both residuals are cc1
coin-flips below C reach / no pure-C-0 form exists"). That framing was derived
on the score-10 HEAD body, which is one insn SHORT of target. On the ARM-SPLIT
chassis the instruction multiset is EXACT (108 vs 108, register-blanked
multisets identical; `goal_from_tgt.py classify` = "FIRST DIVERGENCE: RA",
9 replace + 1 moved). Work the arm-split chassis from now on.

## [s6] The tail residual ("Gap B", ~9 diffs) is ONE missing instruction — CONFIRMED
- statement: on the arm-split chassis, adding the SI temp
  `{ s32 t = a0 + 3; s1[11] = t; }` removes the HImode truncation of a0 that
  created the shared (HI)a0 pseudo cse reuses, so the tail stores a0 directly
  from s2 exactly as target does; the only remaining tail difference is the
  single leading base copy `addu v0,s1,zero`.
- mechanism: expander truncates a0 to HImode BEFORE the +3 when the store is
  `s1[11] = a0 + 3` (s2's RTL insns 213/215); cse then reuses that (HI)a0
  pseudo for the three `s1[N] = a0` HImode stores, which is what pinned the
  base in s1 and produced the extra `move v0,s2`. An SI-mode named
  intermediate blocks the early truncation.
- probe: src = arm-split + SI temp; sandbox --disable all + masked objdump of
  the tail.
- result: score 11, build_insns 107. Tail = `addiu v0,s2,3 / sh v0,22(s1) /
  li v0,0x8000 / sh s2,4(s1) / sh s5,8(s1) / sh s2,20(s1) / sh s2,16(s1) /
  sw v0,24(s1)` vs target `move v0,s1 / addiu v1,s2,3 / sh v1,22(v0) /
  li v1,0x8000 / sh s2,4(v0) / sh s5,8(v0) / sh s2,20(v0) / sh s2,16(v0) /
  sw v1,24(v0)`. Delta = exactly the base copy + the renames it forces.
- verdict: CONFIRMED. Chassis banked at
  rejected/chassis-armsplit-si-temp-tail-matches-except-basecopy.c.
- NOTE: s2 measured this same SI temp on the score-10 HEAD chassis, saw
  107->106, and filed it "wrong direction". That mis-scoping is what sent
  s3/s4/s5 down the coin-flip/permuter path.

## [s6] `s16 *p = s1;` tail alias materialises the base copy on the NEW chassis — KILLED
- mechanism: hoped the different tail cse state (no shared (HI)a0 pseudo) would
  let a single-set pointer copy survive copy-prop.
- probe: arm-split + SI temp + whole tail rewritten through `s16 *p = s1;`.
- result: score 11, build_insns 107 — byte-identical to the chassis without the
  alias. cse copy-propagates the single-set copy regardless of tail shape.
- verdict: KILLED. rejected/tail-p-alias-on-armsplit-si-chassis.c.

## [s6] The else-arm rotation is a sched1 `potential_hazard` tie the C cannot win by SPELLING — CONFIRMED (foreclosure)
- statement: our `addiu s3,s2,3` is placed 3 slots early because
  `schedule_select` overrides rank_for_schedule's (already correct) ordering.
- mechanism: sched.c:2661-2727 picks, within an equal-INSN_PRIORITY group, the
  insn with the largest `potential_hazard` (sched.c:1327-1364:
  `ncost = minb*0x40 + maxb; ncost *= (unit_n_insns[unit]-1)*0x1000 + unit`).
  The two `sh` are on function unit "memory" (mips.md:161) with bmin=1 bmax=3
  and unit_n_insns[memory]=2 -> cost 0x43*0x1000; the addiu has insn_unit=-1
  -> cost 0. An arith insn can never win that tie.
- probe: `pwsh tools/grinder/dump.ps1` (.sched) + instrumented
  `BB2_SCHED_DEBUG=1 tools/gcc-2.7.2/cc1` on the arm-split chassis.
- result: block 2, n_insns=15, `units n0=2`; insn 75 (the addiu) has ZERO
  LOG_LINKS, unit=-1, pri=1; ready list sorted pos0=75 (target's answer) then
  `SELBEST clock=1 insn=72 pos=1` overrides it. Block 2 contains ZERO loads,
  and so does target's else arm — so escape E1 (priority>=2 via a load-fed
  dependence, insn_cost 2 on r3000) has no material to work with in either
  build. Escape E3 is impossible (backward scheduler; both stores have ref=0).
- verdict: CONFIRMED as a foreclosure for statement-level spelling/ordering
  (which is exactly why s4's exhaustive directed line-swap sweep plateaued).
  The one surviving escape is E2: the recompute and the two stores must be in
  DIFFERENT basic blocks when sched runs, so that the recompute's block has
  unit_n_insns[memory] <= 1.

## [s6] The score-10 HEAD body is the right chassis to grade progress on, and the residual contains a genuine PRE-RA (missing-instruction) gap that no RA/sched perturbation can reach.
- mechanism: s5 concluded a reachability/RA-class wall from measurements taken on the score-10 body, which is 107 insns vs target's 108.
- probe: Applied the arm-split chassis (build_insns 108) and ran tools/ra_solver/goal_from_tgt.py classify text1a_c func_80045878 (object-level path, required because the function is INCLUDE_ASM-routed) — the owner-directed solver step, never run before.
- result: ours 108 / target 108, register-blanked multisets IDENTICAL; FIRST DIVERGENCE: RA ($v0->$v1 x1, $s1->$v0 x1); align = 98 equal, 9 replace, 0 delete, 0 insert, 1 moved. There is no missing-instruction gap on this chassis; the whole residual is 9 replaced + 1 moved insn in two disjoint regions (else-arm rotation, tail store block).
- verdict: KILLED

## [s6] The ~9-diff tail ('Gap B') is an irreducible local_alloc coin-flip requiring a base copy that C cannot produce.
- mechanism: s2/s3 attributed the tail to a local_alloc live-range split of the 2-pred join .L800459DC after cse materialised a shared (HI)a0 pseudo.
- probe: Re-ran s2's SI-temp probe ({ s32 t = a0 + 3; s1[11] = t; }) on the ARM-SPLIT chassis (s2 had only measured it on the score-10 HEAD chassis) and disassembled the tail with engine.score.normalized_insns.
- result: score 11, build_insns 107; the tail becomes addiu v0,s2,3 / sh v0,22(s1) / li v0,0x8000 / sh s2,4(s1) / sh s5,8(s1) / sh s2,20(s1) / sh s2,16(s1) / sw v0,24(s1) — a0 is now stored DIRECTLY from s2 three times exactly as target does, and the ONLY remaining tail difference is target's single leading base copy `addu v0,s1,zero` plus the renames it forces. The ~9-diff block is now one instruction.
- verdict: CONFIRMED

## [s6] A C-level pointer alias over the tail materialises the base copy once the shared (HI)a0 pseudo is gone (i.e. the WIP/s2 kill was chassis-specific).
- mechanism: Different cse state in the tail was hoped to let a single-set `s16 *p = s1;` survive copy-propagation.
- probe: arm-split + SI temp + whole tail rewritten through `s16 *p = s1;`; sandbox --disable all.
- result: score 11, build_insns 107 — byte-identical to the same chassis without the alias. cse copy-propagates the single-set copy regardless of the surrounding tail shape.
- verdict: KILLED

## [s6] The else-arm 4-insn rotation ('Gap A' residual) is reachable by some C statement ordering/spelling that lowers the recompute's scheduling priority.
- mechanism: s3 GUESSED 'the recompute feeds the next call arg so it has genuinely higher launch priority'. Checked against the dumps this session instead of guessed.
- probe: pwsh tools/grinder/dump.ps1 (.sched) plus an instrumented BB2_SCHED_DEBUG=1 tools/gcc-2.7.2/cc1 run on the arm-split chassis; then read sched.c schedule_select / potential_hazard / priority and mips.md's define_function_unit entries.
- result: Block 2 (the else arm), n_insns=15, units n0=2. The recompute (insn 75) has ZERO LOG_LINKS, insn_unit=-1, INSN_PRIORITY=1; the two `sh` are unit 0 with bmin=1 bmax=3 and INSN_PRIORITY=1. rank_for_schedule ALREADY sorts the recompute to pos 0 (= target's answer); schedule_select (sched.c:2661-2727) then overrides it because potential_hazard (sched.c:1327-1364) gives the stores 0x43*((2-1)*0x1000+0) = 274432 and gives an insn with no function unit 0. SELBEST clock=1 insn=72 pos=1. An arith insn can therefore never win a same-priority block-end tie against a store — which is exactly why s4's exhaustive directed line-swap sweep plateaued. Escape E1 (INSN_PRIORITY >= 2 via a predecessor with insn_cost 2, i.e. a load) has no material: block 2 contains zero loads and so does target's else arm (0x800458BC-0x800458F8). Escape E3 is impossible (backward scheduler; both stores have ref=0).
- verdict: KILLED


## [s7] `inverse_compose.py classify` mis-routes this function to the TEXT-stream path and reports a spurious PRE-RA verdict - CONFIRMED
- statement: the modality's mandated first step (`inverse_compose.py classify
  text1a_c func_80045878`) prints "PATH: text-stream classifier ...
  func_80045878 is not `replace_with_asmfile`-wired", "honest 105 insns,
  target 106", "FIRST DIVERGENCE: PRE-RA", "ours only: addu $#,$#,3 / target
  only: addiu $#,$#,3 x2" - i.e. the one verdict that means "stop, no RA or
  scheduler perturbation can reach this".
- mechanism: exactly the failure documented in `goal_from_tgt.py`'s own module
  docstring - `.hon.s` is maspsx assembler SOURCE and the target block is
  DISASSEMBLY, so the same instruction is spelled `addu $r,$r,3` on one side
  and `addiu $r, $r, 0x3` on the other and every register-blanked skeleton
  compares unequal. Since func_80045878 is INCLUDE_ASM-routed (not
  `replace_with_asmfile`-wired) the compose classifier does NOT auto-route to
  the object-level path.
- probe: ran both classifiers back-to-back on the identical arm-split chassis.
- result: text-stream = PRE-RA / 105-vs-106; object-level
  (`goal_from_tgt.py classify`) = "ours 108 insns, target 108 insns
  [object-level: replace_with_asmfile-safe]  FIRST DIVERGENCE: RA  $v0 -> $v1
  x1, $s1 -> $v0 x1".
- verdict: CONFIRMED. On this function ALWAYS use `goal_from_tgt.py classify`.
  Do not spend a session acting on the compose classifier's PRE-RA verdict.

## [s7] The tail base copy dies because cse.c:826 `make_regs_eqv` never makes a JOIN-LOCAL pseudo canonical - CONFIRMED (mechanism, read from source)
- statement: a fresh `s16 *p = s1;` at the join is copy-propagated for a
  precise, C-visible reason, and the condition that would keep it is
  satisfiable from C.
- mechanism: `make_regs_eqv (new=p, old=s1)` (tools/gcc-2.7.2/cse.c:826) makes
  NEW the canonical replacement for the quantity only if
      (uid_cuid[regno_last_uid[new]]  > cse_basic_block_end
    || uid_cuid[regno_first_uid[new]] < cse_basic_block_start)
    && (uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]])
  For a pseudo whose first AND last mention are both inside the tail block,
  both halves of the first clause are false, so `s1` stays canonical,
  `canon_reg` rewrites every use of p back to s1, and the copy becomes dead.
  `regno_first_uid` is filled by `reg_scan` (regclass.c), which runs BEFORE
  cse - so a mention of p in ANY earlier basic block flips the clause, even
  one whose insn cse itself later deletes.
- probe: source read + three measured spellings (below).
- verdict: CONFIRMED. This retro-explains the s2 and s6 pointer-alias kills
  (tail-mixed-p-s1-basecopy-propagated.c,
  tail-p-alias-on-armsplit-si-chassis.c) without any appeal to "coin flip".

## [s7] Giving the tail-base pointer an EARLY mention materialises target's join copy - CONFIRMED
- statement: adding a use of p in an earlier basic block keeps the join copy.
- probe/result, three spellings measured on the s6 armsplit+SItemp chassis:
  * `p = s1;` at the END of BOTH arms of the third if - score 16,
    build_insns 108. Copy materialises but sched1 hoists the else-arm one
    above that arm's last three stores and the then-arm one survives as a
    second copy. rejected/tail-base-p-duplicated-into-both-arms.c.
  * early mention in the ELSE arm (its last three stores through p) -
    score 15, build_insns 109; that copy is NOT removable, so it costs an
    extra insn. rejected/tail-base-p-early-mention-in-else-arm-copy-survives.c.
  * early mention in the THEN arm (`p = s1; p[3] = 0;`) + `p = s1;` at the
    join + tail through p - **score 12, build_insns 108**, the then-arm copy
    propagated away and deleted, the join copy standing in target's exact
    position. Banked as
    rejected/chassis-p4-thenarm-p-tail-pure-rename.c - THE NEW CHASSIS.
- verdict: CONFIRMED.

## [s7] On the P4 chassis the WHOLE tail residual (R2) is a two-register RA rename - CONFIRMED
- statement: after P4 the build aligns index-for-index with target and only
  two divergences remain.
- probe: object-level side-by-side (tmp/grind/func_80045878/s7/sbs_p4.txt).
- result: idx 89-97 ours `move a0,s1 / addiu v0,s2,3 / sh v0,22(a0) /
  li v0,0x8000 / sh s2,4(a0) / sh s5,8(a0) / sh s2,20(a0) / sh s2,16(a0) /
  sw v0,24(a0)` vs target the same nine instructions with $a0 -> $v0 and
  $v0 -> $v1. Plus idx 50: ours fills the `beq v1,v0` delay slot with
  `move a0,s3` (duplicated from idx 53) where target has `nop` (reorg.c).
  Nothing else differs anywhere in 108 instructions.
- verdict: CONFIRMED. R1 (the s6 else-arm 4-insn rotation) is GONE on this
  chassis - the arm-split already fixed it; the only ordering residual left
  is the single delay-slot fill at idx 50.

## [s7] The $a0->$v0 flip for the tail base is reachable by perturbing global.c's modelled inputs - KILLED (typed FORECLOSED, depth 2)
- statement: ra_solver's inverse search can name a C lever that gives the tail
  base pseudo $v0.
- mechanism/measurements: `extract.py` + `simulate.py` on the P4 body =
  **8/8 dispositions, sort order MATCH** (the model is exact for this body).
  `local_extract.py` + `local_alloc.py text1a_c --func 80045878` = **no qty
  rows**, i.e. local-alloc allocates NOTHING here; every pseudo is global.c's.
  Trace:
      a=78 pri=30000 calls=0 hard_conf=[2,29] someone=[] best=4 prefs=[4,5]
      a=76 pri=10000 calls=0 hard_conf=[29]   someone=[] best=2 prefs=[2,4,5]
  pseudo 78 = the tail base p, pseudo 76 = the tail scratch. p is allocated
  first (pri 30000) and is BARRED from $v0 by a HARD conflict with hard reg 2,
  then takes $a0 off its own preference list [$a0,$a1]; the scratch takes $v0
  after it.
- probe: `inverse.py global tmp/ra_solver_work/func_80045878.model.json
  --goal '{"78": 2, "76": 3}' --depth 2 --top 10` (and the analogous
  single-goal run on the reuse-v0 body).
- result: **NEGATIVE / FORECLOSED** - "no perturbation of any modelled input,
  up to depth 2, reaches the target assignment" over refs (+12/-6), live
  length (+/-2,4,8), birth order, conflicts, preferences and calls-crossed.
  It additionally reports 8 preference atoms as mechanically unreachable
  (prune_preferences, global.c:897, strips $v0 preferences from every
  call-crossing allocno; $v1 never appears as a hard reg in this function's
  pre-RA RTL so set_preference can never record a preference for it).
- verdict: KILLED for the modelled inputs. The two inputs that must move are
  NOT in the atom space: (i) pseudo 78's HARD conflict with $v0, and (ii) its
  $a0/$a1 preference set. Both are properties of where p's live range sits and
  what hard-reg copies touch it - attack them structurally, from the C, and
  re-extract; do not re-run the same inverse search.

## [s7] `inverse_compose.py classify text1a_c func_80045878` (the modality's mandated first step) gives a trustworthy triage verdict for this function.
- mechanism: func_80045878 is INCLUDE_ASM-routed, not `replace_with_asmfile`-wired, so inverse_compose takes the TEXT-stream path: it compares `.hon.s` (maspsx assembler SOURCE, `addu $r,$r,3`) against the disassembled target (`addiu $r, $r, 0x3`). Every register-blanked skeleton compares unequal, so it reports a wholly different instruction multiset. This is the exact blindness documented in goal_from_tgt.py's own module docstring.
- probe: Ran `inverse_compose.py classify` and `goal_from_tgt.py classify` back-to-back on the identical arm-split chassis; saved both reports.
- result: inverse_compose: 'honest 105 insns, target 106', FIRST DIVERGENCE: PRE-RA, 'ours only addu $#,$#,3 / target only addiu $#,$#,3 x2' -- the one verdict meaning 'stop, no RA/sched perturbation can reach this'. goal_from_tgt: 'ours 108 insns, target 108 insns [object-level]', FIRST DIVERGENCE: RA, $v0->$v1 x1, $s1->$v0 x1. The object-level verdict is the correct one and was confirmed by the whole rest of the session.
- verdict: KILLED

## [s7] The tail base copy `addu v0,s1,zero` dies for an identifiable, C-reachable reason rather than being an irreducible allocation coin-flip (s2/s3/s6 attributions).
- mechanism: cse.c:826 `make_regs_eqv(new=p, old=s1)` makes NEW the canonical register of the quantity only if `(uid_cuid[regno_last_uid[new]] > cse_basic_block_end || uid_cuid[regno_first_uid[new]] < cse_basic_block_start) && uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]`. A fresh `s16 *p = s1;` at the join has its first AND last mention inside the tail block, so both halves of the first clause are false, s1 stays canonical, canon_reg rewrites every use of p back to s1, and the copy becomes dead. regno_first_uid is filled by reg_scan (regclass.c), which runs BEFORE cse, so a mention of p in ANY earlier basic block flips the clause -- even one whose insn cse itself later deletes.
- probe: Read tools/gcc-2.7.2/cse.c make_regs_eqv end-to-end, then measured three spellings that place p's early mention in different blocks.
- result: All three spellings materialise the base copy, which no prior session had ever produced: `p=s1` at the end of BOTH arms -> score 16 / 108 insns (two copies, else-arm one hoisted by sched1); early mention in the ELSE arm -> score 15 / 109 insns (second copy not removable); early mention in the THEN arm (`p = s1; p[3] = 0;`) -> score 12 / 108 insns with the then-arm copy propagated away and deleted and the join copy in target's EXACT position.
- verdict: CONFIRMED

## [s7] With the base copy materialised, the remaining residual of func_80045878 is larger than a register rename.
- mechanism: s6 left two regions (R1 else-arm 4-insn rotation, R2 ~9-insn tail). The arm-split already closes R1; the early-mention lever closes R2's shape.
- probe: Object-level index-for-index side-by-side of the P4 body against target (tmp/grind/func_80045878/s7/sbs_p4.txt).
- result: 108 vs 108 instructions, aligned index-for-index, with exactly two divergences in the whole function: idx 89-97 = target's nine tail instructions in target's exact order and shapes differing ONLY by $a0->$v0 and $v0->$v1; idx 50 = ours fills the `beq v1,v0` delay slot with `move a0,s3` (duplicated from idx 53) where target leaves a `nop` (reorg.c). The s6 R1 rotation is gone.
- verdict: KILLED

## [s7] ra_solver's inverse search can name a C lever that gives the tail base pseudo $v0 (and the tail scratch $v1), i.e. closes R2.
- mechanism: global.c allocno priority + find_reg. simulate.py reproduces this body's allocation 8/8 dispositions with sort order MATCH, and local_alloc.py reports NO qty rows for func_80045878, so every pseudo is a global.c allocno and the model is authoritative here.
- probe: `extract.py` + `simulate.py --trace` + `local_extract.py`/`local_alloc.py` on the P4 body, then `inverse.py global tmp/ra_solver_work/func_80045878.model.json --goal '{"78": 2, "76": 3}' --depth 2 --top 10` (and the single-goal analogue on the reuse-v0 body).
- result: NEGATIVE RESULT / FORECLOSED: no perturbation of refs (+12/-6), live length (+/-2,4,8), birth order, conflicts, preferences or calls-crossed reaches target at depth 2; 8 preference atoms additionally reported mechanically unreachable (prune_preferences global.c:897 strips $v0 preferences from every call-crossing allocno, and $v1 never appears as a hard reg in this function's pre-RA RTL so set_preference can never prefer it). The trace names the two blocking inputs precisely: pseudo 78 (tail base p) pri=30000 calls=0 hard_conf=[2,29] prefs=[4,5] -> $a0; pseudo 76 (tail scratch) pri=10000 calls=0 hard_conf=[29] prefs=[2,4,5] -> $v0. p is allocated first and is BARRED from $v0 by a hard conflict with hard reg 2, then takes $a0 off its own preference list.
- verdict: KILLED

## [s7] Reusing the existing local `v0` (the func_8004574C result) as the tail base is a cheaper way to buy the early mention than introducing a new pointer.
- mechanism: v0 is genuinely used at the top, so regno_first_uid is early for free and cse.c:826 keeps the join copy.
- probe: Applied the reuse-v0 body; sandbox --disable all; object-level side-by-side; `inverse.py global --goal '{"76": 2}' --depth 2`.
- result: score 15, build_insns 109 vs target 108. The tail comes out as the same pure rename of target, so the cse mechanism works -- but the reuse costs a whole extra instruction (`move a0,v0` at idx 12): the v0 pseudo now spans the call-result region where $v0 is hard-conflicted, so global.c gives the whole pseudo $a0 and must copy the return value into it. inverse.py returned FORECLOSED for that pseudo -> $v0 as well.
- verdict: KILLED

## [s8] The tail base pseudo can be BOTH cse-canonical (copy survives) and single-block (so local_alloc gives it $v0) - KILLED (closed-form, from cc1 source + 3 measurements)
- statement: some C spelling of the tail base makes the join copy
  `addu v0,s1,zero` survive AND lets the base win $v0, closing residual R2.
- mechanism: (a) toplev.c pass order is flow_analysis (2983) -> combine
  (3004) -> sched (3033) -> regclass+local_alloc (3049) -> global_alloc
  (3077); REG_BASIC_BLOCK is written ONLY in flow.c (2072-2075, 2508-2511),
  before combine. (b) The two tail scratch pseudos are born and die in the
  tail block, so local_alloc claims them first and find_free_reg gives them
  $v0 (.lreg: `Register 101/102 ... in block 13`, `;; Register 101 in 2.`,
  `;; Register 102 in 2.`); that is the entire origin of pseudo 78's HARD
  conflict with hard reg 2 (.greg: `;; 78 conflicts: 72 73 78 2 29`) which
  s7 named as blocking and inverse.py reported outside its atom space.
  Target's shape therefore requires the BASE to be block-local too, so
  local-alloc allocates it first. (c) But cse.c:826 make_regs_eqv keeps the
  copy only if the base is canonical, which for the join EBB requires
  `uid_cuid[regno_first_uid[p]] < cse_basic_block_start` - a mention in an
  EARLIER basic block (the sibling disjunct `regno_last_uid > cse_basic_block_end`
  is unreachable: the tail block is the function's last block). (d) Any such
  mention sets REG_BLOCK_GLOBAL at flow time, before combine deletes it -
  measured: the P4 then-arm copy insns 121/124 are still in .flow and gone
  only in .combine, yet .lreg reports pseudo 78 with no `in block N`.
- probe: read toplev.c/flow.c/cse.c/local-alloc.c; re-measured P4 (12/108);
  measured the dead-mention variant (then-arm store through s1) = 11/107;
  measured the FIRST-if else-arm placement = 14/109; disassembled
  tmp/sandbox/func_80045878/text1a_c.o for both to get index-for-index truth.
- result: canonical IFF multi-block; $v0 IFF single-block. Mutually
  exclusive. Every placement of the early mention is now measured: third-if
  THEN arm 12/108 (s7, the only free one), third-if ELSE arm 15/109 (s7),
  both arms 16/108 two copies (s7), FIRST-if else arm 14/109 (s8), dead
  mention 11/107 (s8), reuse of local `v0` 15/109 (s7).
- verdict: KILLED. Do NOT re-spell the pointer copy; the space is closed.

## [s8] R1' (the idx-50 delay-slot fill) and R2 (the tail register pair) are independent residuals that can be closed separately - KILLED (they are anti-coupled through one C decision)
- statement: s7 recorded R1' and R2 as two separate frontier items, implying
  independent levers.
- mechanism: reorg.c fills the `beq v1,v0` delay slot from the successor
  block only when a safe copy is available there. Writing the third-if
  then-arm store through `p` (P4) leaves `move a0,s3` available and reorg
  takes it; writing the same store through `s1` leaves the slot empty.
- probe: object-level diff of the two sandbox builds (p4.dis vs dead.dis in
  tmp/grind/func_80045878/s8/).
- result: with the then-arm store through s1 the slot is `nop`, EXACTLY as
  target - R1' CLOSED - but that same spelling removes p's early mention and
  the join base copy dies (107 insns), reopening R2 worse than before.
  With the store through p, R2's shape is right and R1' is wrong.
- verdict: KILLED as independent items. One C decision (the base of the
  then-arm store) controls both, in opposite directions.

## [s8] `pwsh tools/grinder/dump.ps1 func_80045878` reliably reflects the current src/text1a_c.c - KILLED (tool defect)
- statement: the mandated pass-attribution dump tool tracks source edits.
- probe: dumped P4, edited one statement (`p[3] = 0;` -> `s1[3] = 0;`),
  re-dumped; then measured both variants with the sandbox and disassembled
  the sandbox objects.
- result: the two dumps were BYTE-IDENTICAL (.greg 468146 bytes both times,
  identical .s function body including `move $4,$17`) while the sandbox
  objects differed by a whole instruction (108 vs 107) and by the presence of
  that very copy. The dump taken immediately after the P4 measurement is
  valid; the one taken after the edit is stale.
- verdict: KILLED. Take the dump immediately after the sandbox measurement it
  is meant to explain, and cross-check any dump-based claim against
  `mipsel-linux-gnu-objdump -d tmp/sandbox/<func>/<stem>.o`
  (tmp/grind/func_80045878/s8/dis.sh).

## [s8] Some C spelling of the tail base makes the join copy `addu v0,s1,zero` survive AND lets that pseudo win $v0, closing residual R2.
- mechanism: toplev.c pass order is flow_analysis(2983) -> combine(3004) -> sched(3033) -> regclass+local_alloc(3049) -> global_alloc(3077), and REG_BASIC_BLOCK is written ONLY in flow.c (2072-2075, 2508-2511), before combine. The two tail scratch pseudos are born and die inside the tail block, so local_alloc claims them first and find_free_reg gives them $v0 (.lreg 'Register 101/102 ... in block 13' plus ';; Register 101 in 2.' / ';; Register 102 in 2.'); that is the whole origin of pseudo 78's HARD conflict with hard reg 2 (.greg ';; 78 conflicts: 72 73 78 2 29') - the input s7 named as blocking and inverse.py reported outside its atom space. Target's tail (base=$v0, scratch=$v1) therefore requires the BASE to be block-local too, so local_alloc allocates it first. But cse.c:826 make_regs_eqv keeps the copy only if the base is canonical, which at the join EBB requires uid_cuid[regno_first_uid[p]] < cse_basic_block_start, i.e. a mention in an EARLIER basic block (the sibling disjunct regno_last_uid > cse_basic_block_end is unreachable because the tail block is the function's last block). Any such mention sets REG_BLOCK_GLOBAL at flow time, before combine deletes it.
- probe: Read toplev.c / flow.c / cse.c / local-alloc.c; re-measured the P4 chassis (sandbox --disable all = 12, build_insns 108); measured the dead-mention variant (then-arm store through s1) = 11 / 107; measured the FIRST-if else-arm placement = 14 / 109; disassembled tmp/sandbox/func_80045878/text1a_c.o for both variants for index-for-index truth.
- result: Canonical IFF multi-block; $v0 IFF single-block - mutually exclusive. Every placement of the early mention is now measured: third-if THEN arm 12/108 (s7, the only cost-free one), third-if ELSE arm 15/109 (s7), both arms 16/108 with two copies (s7), FIRST-if else arm 14/109 (s8), dead mention 11/107 (s8), reuse of the existing `v0` local 15/109 (s7). On P4 the then-arm copy insns 121/124 are still present in .flow and gone only in .combine, yet .lreg reports pseudo 78 as 'used 9 times across 9 insns' with no 'in block N' suffix - flow.c:2074 had already made it global.
- verdict: KILLED

## [s8] R1' (the idx-50 delay-slot fill) and R2 (the tail register pair) are independent residuals closable by separate levers, as s7's frontier assumed.
- mechanism: reorg.c fills the `beq v1,v0` delay slot from the successor block only when a safe copy is available there. Writing the third-if then-arm store through `p` leaves `move a0,s3` available and reorg takes it; writing the same store through `s1` leaves the slot empty.
- probe: Object-level diff of the two sandbox builds (tmp/grind/func_80045878/s8/p4.dis vs dead.dis, produced by disassembling tmp/sandbox/func_80045878/text1a_c.o after each measurement).
- result: With the then-arm store through s1 the slot comes out as `nop`, exactly as target has it - R1' CLOSED - but that same spelling removes p's early mention, the join base copy dies, and the build is 107 insns (one SHORT of target), reopening R2 worse than before. With the store through p, R2's shape is right and R1' is wrong. One C decision (the base of that store) controls both, in opposite directions.
- verdict: KILLED

## [s8] `pwsh tools/grinder/dump.ps1 func_80045878` reliably reflects the current src/text1a_c.c, so its dumps can be trusted for pass attribution.
- mechanism: The dump script recompiles the TU with -da and writes the pass dumps under tmp/grind/<func>/dumps/.
- probe: Dumped with P4 in place, edited exactly one statement (`p[3] = 0;` -> `s1[3] = 0;`), re-dumped, then measured both variants with the sandbox and disassembled the sandbox objects.
- result: The two dumps were byte-identical (.greg 468146 bytes both times; identical .s function body including the `move $4,$17` join copy) while the sandbox objects differed by a whole instruction (108 vs 107) and by the presence of that very copy. The dump taken immediately after the P4 measurement is valid and is what every P4 dump citation rests on; the one taken after the edit is stale.
- verdict: KILLED


## [s9] 2026-08-30 (escalation modality - floor 10 -> 4)

- H-s9-1 (CONFIRMED, and it kills the s8 foreclosure). Making BOTH tail scratch
  values non-block-local removes pseudo 78's hard conflict with hard reg 2 and
  global.c then seats the tail base in $v0 and the scratch in $v1, exactly as
  target has them. Mechanism: local-alloc.c:472 gates on
  reg_basic_block[i] >= 0; local_alloc runs before global_alloc and takes the
  lowest free GR for any tail-block-local pseudo. Probe: V3 (hoist both into
  the arms) -> the tail renders "move v0,s1 / sh v1,22(v0) / ...". The s8 claim
  that hard-reg preferences [4,5] would still force $a0 is FALSE - it was never
  measured.
- H-s9-2 (CONFIRMED). One scratch is not enough. V2 (only 0x8000 hoisted,
  score 13) and V4 (only a0+3 hoisted, score 13) both leave a block-local
  scratch that takes $v0 and pushes the base back to $a0.
- H-s9-3 (CONFIRMED). A single reused local can carry BOTH tail scratches and
  that is what target's single $v1 seat wants. The carrier must be (a)
  mentioned in an earlier basic block, (b) never live across a call, and (c)
  not the hard-$v0 call-return value. Violating (b) gives $s0 (V5, score 7);
  violating (c) costs a "move v1,v0" (V6, 109 insns, score 7).
- H-s9-4 (CONFIRMED). Which condition temp is used matters, because the
  carrier's earlier use fixes its seat. s1[4]'s temp must be $v0 in target, so
  carrying through it scores 6 (V7); s1[3]'s temp is $v1 in target, and
  carrying through it scores 4 (V8 = candidate.c).
- H-s9-5 (KILLED). "Source statement order in the tail selects which of
  p = s1 / c = a0+3 is emitted first." Probe V9 swapped them; output
  byte-identical. sched.c re-derives the order from priority, not RTL order.
- H-s9-6 (KILLED). "Give the base pseudo a second set so birthing_insn_p's
  reg_n_sets == 1 test fails and the LAUNCH_PRIORITY bump is denied", spelled
  as routing the third-if else arm's three record stores through p. Probe V11:
  score 20 / 109 insns - the else arm gains an instruction and the block is
  wrecked. A cheaper spelling of the same idea is still open (see frontier).
- H-s9-7 (CONFIRMED, mechanism only). Both remaining residuals are the same
  scheduler phenomenon: an "addiu rX,s2,3" emitted at the wrong end of its
  block because its producer lost the adjust_priority (sched.c:2543)
  LAUNCH_PRIORITY bump that birthing_insn_p (sched.c:2570) grants only to live
  single-set destinations. Trace line:
  ";; ready list at T-8: 222 (1) 225 (7f000001), now 225 222".

## [s9] Making BOTH tail scratch values non-block-local removes pseudo 78's hard conflict with hard reg 2, and global.c then seats the tail base in $v0 and the scratch in $v1 (target's assignment), contradicting s8's claim that hard-reg preferences [4,5] would still force $a0.
- mechanism: local-alloc.c:472 only claims pseudos with reg_basic_block[i] >= 0; local_alloc runs before global_alloc (toplev.c 3049 vs 3077) and hands the lowest free GR ($v0) to any tail-block-local scratch, which is the sole origin of pseudo 78's hard_conf=[2,29]. Remove the block-local scratches and global.c is free to seat the base in $v0.
- probe: V3 (tmp/grind/func_80045878/s9/v3.c): both scratches assigned in the third-if arms. objdump of the tail renders 'move v0,s1 / addiu v1,s2,3 / sh v1,22(v0)' - target's registers.
- result: score 10 at 109 insns; tail base = $v0, scratch = $v1 for the first time in nine sessions
- verdict: CONFIRMED

## [s9] One scratch is not enough - if EITHER tail scratch stays block-local it takes $v0 and pushes the base back to $a0.
- mechanism: local_alloc claims each block-local pseudo independently and find_free_reg takes the lowest free GR, so a single surviving tail-local quantity re-creates the hard conflict with reg 2.
- probe: V2 (only the 0x8000 constant hoisted) and V4 (only the a0+3 value hoisted), both measured with sandbox --disable all and objdump.
- result: V2 score 13/108 (li stays, base $a0); V4 score 13/108 (addiu stays, base $a0)
- verdict: CONFIRMED

## [s9] A single reused local can carry both tail scratches (matching target's single $v1 seat), but it must be multi-block, never live across a call, and not the hard-$v0 call-return value.
- mechanism: The carrier's earlier live range merges into one allocno: calls_crossed > 0 forces a callee-save seat in global.c, and reusing the call-return local forces the return value out of hard $v0 and costs an extra copy insn.
- probe: V5 (carrier = existing s0, whose else-arm range crosses func_800455AC/func_80044ED8) and V6 (carrier = existing v0 call-return local).
- result: V5 score 7/108 with the tail rendered 'addiu s0,s2,3 / li s0,0x8000' (callee-save); V6 score 7 at 109 insns with an added 'move v1,v0' at the top
- verdict: CONFIRMED

## [s9] Which of the two condition temps is reused as the carrier decides the seat, because the earlier use pins it: target seats the s1[4] temp in $v0 and the s1[3] temp in $v1, and the tail scratch must be $v1.
- mechanism: One pseudo gets one hard register for its whole (merged) live range, so the carrier's condition-site register and its tail-site register are the same seat.
- probe: V7 (carrier = the s1[4] condition temp) vs V8 (carrier = the s1[3] condition temp), both sandbox-measured and word-aligned against asm/funcs/func_80045878.s.
- result: V7 score 6/108 (carrier lands in $v1 at the condition where target has $v0); V8 score 4/108 with every register in the function matching target
- verdict: CONFIRMED

## [s9] Source statement order in the tail selects which of 'p = s1' and 'c = a0 + 3' is emitted first.
- mechanism: If the scheduler were order-preserving for a priority tie, swapping the two statements would swap the emitted pair and close residual idx 89/90.
- probe: V9: 'c = a0 + 3;' written before 'p = s1;' in the tail; RTL order confirmed swapped in the .flow/.combine dumps.
- result: output byte-identical to V8 (score 4, 108 insns) - sched.c re-derives the order from INSN_PRIORITY, not from RTL order
- verdict: KILLED

## [s9] Giving the tail base pseudo a second set defeats birthing_insn_p's reg_n_sets == 1 test, denies it the LAUNCH_PRIORITY bump, and flips the idx 89/90 pair - spelled as routing the third-if else arm's record stores through p.
- mechanism: birthing_insn_p (sched.c:2570) returns true only when the destination is live AND reg_n_sets[dest] == 1; adjust_priority (sched.c:2543) raises only birthing predecessors to max_priority (LAUNCH_PRIORITY, sched.c:187/4049).
- probe: V11: 's1[3] = 1; *(s1+0x24) = 0; *(s1) = 0;' rewritten as 'p = s1; p[3] = 1; *(p+0x24) = 0; *(p) = 0;' in the else arm.
- result: score 20 at 109 insns - the else arm gains an instruction and its schedule is wrecked; this spelling is dead, the underlying idea is not
- verdict: KILLED

## [s10] The else-arm `addiu s3,s2,3` at idx 32 is NOT a second source assignment: it is the single join-block statement, with reorg.c having duplicated it into the `beqz` delay slot at idx 13 and advanced the then-arm `j` past the original.
- mechanism: reorg.c fill_slots_from_thread copies the first insn of the branch target block into the delay slot and redirects the jump label past the copied insn. Direct evidence in asm/funcs/func_80045878.s: the `j` at idx 15 targets func+0x84 == idx 33, skipping idx 32. Nine prior sessions read the two emitted addiu as two source statements (one per arm of the first if) and spent the whole structural/permuter/solver ladder trying to fix the resulting block order with scheduler levers.
- probe: R7 (tmp/grind/func_80045878/s10/r7.c) - delete both arm assignments, write ONE `s3 = a0 + 3;` immediately after the first if/else and before `if (func_8004574C(s3))`.
- result: score 4 -> 2 at 108 insns; Gap A gone, only the idx 89/90 pair left
- verdict: CONFIRMED

## [s10] The join-block placement is load-bearing: hoisting the single `s3 = a0 + 3;` ABOVE the first if leaves only the delay-slot materialisation and loses an instruction.
- mechanism: with the statement in the entry block there is nothing for reorg.c to duplicate-and-skip; the addiu is emitted once, in the entry block, and reorg then fills the beqz slot from it.
- probe: R2 (r2.c) - `s3 = a0 + 3;` before `v0 = func_8004574C(a0);`
- result: score 3 at 107 insns (target 108) - one insn short, the idx-32 addiu is simply absent
- verdict: CONFIRMED

## [s10] Source-statement order inside the first-if else arm is byte-neutral for the Gap A pair; the block order is decided by schedule_select's potential-hazard rule, not by LUID.
- mechanism: sched.c schedule_select works down the ready list in equal-priority groups and moves the insn with the largest potential_hazard to the front; the two `sh` stores beat the `addiu` on that test regardless of LUID, so the addiu is picked last (T-4) and emitted first. Trace: ";; insn 72 has a greater potential hazard, now 72 75 69" (block 2, this session's dump).
- probe: R3 (s3 assignment first in the arm), R5 (s3 between the two stores), R6 (`a0 + 3` instead of `a0 - -3`)
- result: all three score 4 at 108 insns - byte-identical to the s9 baseline
- verdict: KILLED (this axis is dead; Gap A was never a scheduler problem)

## [s10] Residual B (idx 89/90) closes by denying the tail base pointer the birthing_insn_p LAUNCH_PRIORITY bump, spelled as reusing the EXISTING local `v0` (the func_8004574C result pointer, dead after the first if) as the tail base.
- mechanism: sched.c:2543 adjust_priority raises a newly-ready predecessor to max_priority (0x7f000001, sched.c:187/4049) iff sched.c:2570 birthing_insn_p holds - dest live AND reg_n_sets[dest] == 1. Two sets of the base kill the test; both insns then sit at priority 1, rank_for_schedule falls through to INSN_LUID, the addiu has the higher LUID and schedule_select finds no hazard difference between two ALU insns, so the addiu is picked at T-8 and emitted second. Trace before the fix: ";; ready list at T-8: 219 (7f000001) 222 (1), now 219 222".
- probe: U2/U3 (u2.c raw, u3.c cleaned spelling) - `s16 *v0;` reused, tail writes `v0 = s1; ... v0[11] = c; ...`
- result: score 0 at 108 insns, rules_dropped 0 - THE FUNCTION IS BYTE-EXACT
- verdict: CONFIRMED

## [s10] Every OTHER way of giving the tail base a second set costs at least one instruction.
- mechanism: a second `p = s1` copy that cse cannot fold is a real `move` insn; one that cse CAN fold leaves reg_n_sets at 1 and the bump intact.
- probe: S1 (p set before the third if, condition read through p), S4 (p set before the third if, then-arm store through p), S3 (p set in the third-if else arm), S2 (p carried through both arms with no tail set), U1 (p reused for the existing 0x1A88 address computation).
- result: 109 / 109 / 110 / 109 / 108 insns at scores 12 / 12 / 57 / 49 / 2 - only the `v0` borrow is free, and U1 (the 0x1A88 borrow) leaves the pair unswapped at score 2
- verdict: KILLED (as alternatives)

## [s10] The fresh call-free carrier `c` is still mandatory on the new chassis - no existing local can replace it, because every existing local and parameter in this function crosses a call.
- mechanism: global.c seats an allocno with calls_crossed > 0 in a callee-save register, and local-alloc.c:472 hands $v0 to any pseudo that is block-local to the tail.
- probe: U4/U5 (carrier dropped entirely, tail scratch values written inline), U6 (`s0` borrowed as the carrier), T1/T3 (one scratch carried, the other block-local).
- result: U4/U5 110 insns score 14; U6 108 insns score 6 with the tail rendered `addiu s0,s2,3` (callee-save); T1/T3 108 insns score 10
- verdict: CONFIRMED

## [s10b] 2026-08-30 (rederive modality - floor 0-with-banned-construct / 4 legal)

## [s10b] The carrier's non-block-local status only has to exist at flow.c time, so its earlier reference may be combine-folded away for free and does NOT have to inherit that site's register seat.
- mechanism: reg_basic_block[] is written only by flow.c's life_analysis (flow.c:2072 and flow.c:2508 promote a pseudo to REG_BLOCK_GLOBAL the first time it is referenced from a second block) and is never recomputed; toplev.c calls combine_instructions at 3004, schedule_insns at 3033, and regclass()/local_alloc() only at 3051-3052. local-alloc.c:472 (and its comment at local-alloc.c:194) gate on reg_basic_block[i] >= 0, so a pseudo that WAS multi-block at flow time is skipped by local_alloc even if combine has since deleted the second reference.
- probe: E1 (tmp/grind/func_80045878/s10/e1.c, banked as memory/grind/func_80045878/alt_anchor_e1.c) - carrier `c` anchored on the first-if else arm's call argument `c = 0x1A88 + (s32)s1; func_80045600(a0, c);`, with the third `if` restored to plain `... && (s1[3] != -2)`.
- result: score 0 at 108 insns, rules_dropped 0 - byte-exact, and the anchor site still emits target's single `addiu $a1, $s1, 0x1A88` (no $a1 seat leaked into the tail, no extra copy insn)
- verdict: CONFIRMED

## [s10b] The anchor SITE is not unique - any earlier, call-free, live reference works - but a site whose value survives to RA propagates its seat and costs points.
- mechanism: if combine cannot fold the earlier reference away, the carrier is one allocno spanning both sites and gets ONE hard register; target's earlier value at that site then dictates the tail seat.
- probe: E2 (anchor = `c = -1; s1[4] = c;` in the first-if else arm) and E3 (anchor = the natural named intermediate for the else arm's shift arithmetic, `c = (((u32)((s32*)s0)[*(s32*)s0]) >> 2) << 2; s0 = s0 + c;`), both with the third-if condition left plain.
- result: E2 score 2 at 108 insns; E3 score 2 at 108 insns (target computes both of those values in $v0, and the carrier drags $v0 into the tail). Working anchors so far: the s1[3] condition read (candidate.c, score 0) and the 0x1A88 argument (E1, score 0). Failing anchors: s1[4] condition read (s9 V7, score 6), -1 constant (E2, 2), shift temp (E3, 2), existing local s0 (s9 V5 / s10 U6, score 6-7, callee-save), existing local v0 (s9 V6, 109 insns).
- verdict: CONFIRMED

## [s10b] The sanctioned duplicated-statement-into-arms family cannot supply the non-block-local scratches: duplicating the two tail scratch STORES (not their values) into both arms of the third `if` is not re-merged and costs an instruction.
- mechanism: cross-jumping runs in the post-reload jump pass, and the two copies do not form an identical tail (the then-arm copy is followed by a jump, the else-arm copy by fall-through, and both address through $s1 while target's tail addresses through the $v0 base copy), so jump.c never merges them.
- probe: D1 (rejected/s10b-dupstores-into-arms-109-score11.c) - `s1[11] = a0 + 3; *(s32*)((s32)s1 + 0x18) = 0x8000;` appended to BOTH arms, join keeps `v0 = s1;` plus the other four stores, no carrier local at all.
- result: score 11 at 109 insns
- verdict: KILLED

## [s10b] STRUCTURAL THEOREM (rederive): every byte-exact form of this function needs a fresh local written MORE THAN ONCE; no existing variable and no single-write variable can serve.
- mechanism: (1) both tail scratch values must sit in non-block-local pseudos or local_alloc hands one of them $v0 ahead of the base (s9 V2/V4, s10 T1/T3); (2) a non-block-local pseudo needs a live reference in a second basic block at flow time; (3) that reference must not be live across a call or global.c seats it callee-save (s9 V5); (4) a SINGLE-write carrier holds one value from its birth to its last use, so its earlier reference and its tail reference must want the SAME value - the tail wants `a0 + 3` and `0x8000`, `0x8000` occurs nowhere else in the function, and `a0 + 3` already exists as `s3`, whose live range provably crosses calls (it is target's $s3); (5) the complete list of pre-existing variables is s0/s1/s3/v0/a0/a1/a2 and each is disqualified by a measured or structural property - s0/s3/a0/a2 cross calls (callee-save seats), s1 and a1 are still live in the tail (target seats them $s1/$s5, different registers from the tail scratch's $v1), v0 is the hard-$v0 call return and is already spent on the accepted tail base.
- probe: the s9 + s10 + s10b measurement table in evidence.md; this session added E1/E2/E3/D1 and the flow.c/toplev.c source reads that pin the reg_basic_block lifetime.
- result: no legal (single-write, or existing-variable) spelling exists in the searched space; the two byte-exact forms found so far (candidate.c and alt_anchor_e1.c) both carry a 3-write fresh local, which the Judge FAILed on 2026-08-30 21:30
- verdict: CONFIRMED (as an elimination result; it is what the next escalation-modality session should carry into a decision packet)

## [s10] The carrier's non-block-local status only has to exist at flow.c time, so its earlier reference may be combine-folded away for free and does not leak that site's register seat into the tail.
- mechanism: reg_basic_block[] is written only by flow.c life_analysis (flow.c:2072 / flow.c:2508 promote a pseudo to REG_BLOCK_GLOBAL on its first second-block reference) and is never recomputed; toplev.c calls combine_instructions at 3004, schedule_insns at 3033, and regclass()+local_alloc() only at 3051-3052, while local-alloc.c:472 skips any pseudo with reg_basic_block[i] < 0.
- probe: E1 (tmp/grind/func_80045878/s10/e1.c): carrier anchored on the first-if else arm's argument 'c = 0x1A88 + (s32)s1; func_80045600(a0, c);', third if restored to plain '... && (s1[3] != -2)'; sandbox func_80045878 --disable all.
- result: score 0 at 108 insns, rules_dropped 0 - byte-exact; the anchor site still emits target's single 'addiu $a1,$s1,0x1A88', with no extra copy and no $a1 seat in the tail. Banked as memory/grind/func_80045878/alt_anchor_e1.c.
- verdict: CONFIRMED

## [s10] The anchor site is fungible, but a site whose value survives to RA propagates its own register seat into the tail and costs points.
- mechanism: If combine cannot fold the earlier reference away, the carrier is one allocno spanning both sites and receives one hard register, so target's value at that site dictates the tail seat.
- probe: E2 (anchor = the else arm's -1 constant, 'c = -1; s1[4] = c;') and E3 (anchor = a natural named intermediate for the else arm's >>2<<2 arithmetic), both with the third-if condition left plain.
- result: E2 score 2 at 108 insns; E3 score 2 at 108 insns - target computes both of those values in $v0 and the carrier drags $v0 into the tail. Working anchors: the s1[3] condition read (score 0) and the 0x1A88 argument (score 0); failing anchors: s1[4] read (6), -1 (2), shift temp (2), existing local s0 (6-7), existing local v0 (109 insns).
- verdict: CONFIRMED

## [s10] The sanctioned duplicated-statement-into-arms family can supply the non-block-local tail scratches with no carrier local at all.
- mechanism: Duplicating a real statement into both arms makes its pseudo multi-block at flow time; post-reload cross-jumping was expected to re-merge the two copies byte-neutrally.
- probe: D1 (memory/grind/func_80045878/rejected/s10b-dupstores-into-arms-109-score11.c): both tail scratch STORES appended to both arms of the third if, no carrier local, join keeps 'v0 = s1' plus the other four stores.
- result: score 11 at 109 insns - jump.c never merges them: the then-arm copy is followed by a jump, the else-arm copy falls through, and both address through $s1 while target's tail addresses through the $v0 base copy.
- verdict: KILLED

## [s10] Some legal spelling exists - either an existing variable or a single-write fresh local can serve as the tail carrier (the Judge's suggested remedy).
- mechanism: Both tail scratches must be non-block-local or local_alloc hands one of them $v0 ahead of the base; a non-block-local pseudo needs a live second-block reference at flow time that is not itself live across a call.
- probe: Complete enumeration of the function's variables against those requirements, plus this session's E1/E2/E3/D1 measurements on top of s9/s10's V2/V4/V5/V6/V7 and T1/T3/U4/U5/U6 table.
- result: No such spelling exists. A single-write carrier would have to want the same value at both of its references, but the tail wants 'a0 + 3' and '0x8000'; 0x8000 occurs nowhere else in the function, and a0+3 already exists as s3, whose live range crosses calls (target seats it $s3). Every pre-existing variable is independently disqualified: s0/s3/a0/a2 cross calls (callee-save seats, measured score 6-7); s1 and a1 are still live in the tail with different target seats ($s1/$s5 vs the scratch's $v1); v0 is the hard-$v0 call return and is already spent on the accepted tail base.
- verdict: KILLED

## [s11] 2026-08-30 (rederive modality - chassis re-measured, floor 0-with-banned-construct / 4 legal)

## [s11] A `static inline` tail helper can make the base copy and the two tail scratches non-block-local through integrate.c's parameter pseudos, with no source-level variable reuse at all. (frontier item 3, inherited from s10)
- mechanism: integrate.c copies the callee body into the caller and creates fresh pseudos for the parameters; the parameter copy is set in the caller's block and read in the inlined body's blocks, which would be a flow-time second-block reference.
- probe: I1 (tmp/grind/func_80045878/s11/i1.c, banked rejected/s11-inline-tail-helper-basecopy-seated-a0-109-score13.c) - the six tail stores moved into `static inline void func_80045878_set(s16 *p, s32 hi, s32 id, s32 kind, s32 flags)`, called once as `v0 = s1; func_80045878_set(v0, a0 + 3, a0, a1, 0x8000);`, NO carrier local anywhere. I2 (rejected/s11-inline-tail-helper-param-copy-cse-folded-107-score9.c) - same helper called with `s1` directly.
- result: I1 = 109 insns / score 13; I2 = 107 insns / score 9. The premise is FALSE: the argument copies are emitted in the CALLER's block and the branch-free inlined body is spliced into that same block, so every parameter pseudo is still block-local. In I1 the first scratch therefore takes $v0 from local_alloc, the multi-block base allocno `v0` is pushed to $a0 by global_alloc, and an extra `move a0,v0` appears at the top for the first call's return value. In I2 cse folds the parameter copy away and the base copy insn vanishes (107).
- verdict: KILLED (frontier item 3 closed)

## [s11] The inline-helper form nevertheless reproduces target's tail INSTRUCTION SEQUENCE exactly, with no carrier local: the residual is a pure 2-register rename plus one insn.
- mechanism: integrate.c splices the helper body verbatim, so the sched1 ordering that the `c` carrier was needed to produce (the `li 0x8000` sitting four insns ahead of its use, between `sh ...,0x16` and `sh ...,0x4`) falls out of the source order of the helper body by itself.
- probe: I1's disassembly (tmp/grind/func_80045878/s11/ diff vs asm/funcs/func_80045878.s).
- result: ours `move a0,s1 / addiu v0,s2,3 / sh v0,22(a0) / li v0,0x8000 / sh s2,4(a0) / sh s5,8(a0) / sh s2,20(a0) / sh s2,16(a0) / sw v0,24(a0)` vs target's byte-identical stream in $v0/$v1. Everything else in the function matches except the extra top-of-function `move a0,v0` and the delay slot of the third if's last branch (ours fills it with the else arm's `move a0,s3` and retargets, target leaves a nop) - both downstream of the same seat decision.
- verdict: CONFIRMED (the carrier is needed ONLY for the register seats, never for the instruction order)

## [s11] Prong (1) of the s10b structural theorem re-proved from a second, independent mechanism: any block-local pseudo in the tail takes $v0 ahead of the base, unconditionally.
- mechanism: config/mips/mips.h does NOT define REG_ALLOC_ORDER, so local-alloc.c:2249-2272 falls into the `#ifndef REG_ALLOC_ORDER` arm and scans hard regs in ASCENDING regno order; $2 ($v0) is the first allocatable GP register. toplev.c runs local_alloc BEFORE global_alloc, so a block-local tail quantity always wins $v0 from a multi-block base allocno, whatever their relative qty priorities.
- probe: source read of tools/gcc-2.7.2/local-alloc.c:2249-2272 + the absence of REG_ALLOC_ORDER anywhere under tools/gcc-2.7.2/config/mips/; corroborated by I1 (block-local scratch = $v0, base = $a0) and by s9/s10's T1/T3/U4/U5.
- result: independent confirmation of the s10b theorem's prong (1) - the scratches MUST be non-block-local.
- verdict: CONFIRMED

## [s11] The theorem could be side-stepped from the other end: if the BASE copy were block-local it would outrank the scratches on local-alloc.c's qty_compare priority and take $v0 first, leaving $v1 for the scratches and removing the non-block-local requirement entirely.
- mechanism: local-alloc.c:1641 qty_compare ranks by log2(n_refs)*n_refs*qty_size/(qty_death-qty_birth); the tail base has 7 refs over ~9 insns (pri ~15555) against the scratch's 2 refs over 2 insns (pri ~10000), so the base would be allocated first.
- probe: P4 (rejected/s11-block-local-multiset-base-copy-folded-108-score9.c) - a fresh block-local base local written twice from the same source (`p = s1; p[11] = a0+3; p = s1; ...`) to make it multi-set (the property that keeps the accepted `v0 = s1` copy alive) while staying inside one block. Mechanism probe only; it carries a fresh multi-write local and is not shippable.
- result: 108 insns / score 9 - BOTH copies are folded by cse and the second set is deleted as a dead store; all six stores address through $s1 and an unrelated `move v0,s2` appears. A surviving base copy needs its two sets to carry DIFFERENT values in DIFFERENT blocks, which is exactly the multi-block `v0` reuse - and multi-block means global_alloc, the configuration that loses $v0 to a block-local scratch.
- verdict: KILLED (the reframing is unconstructible; the theorem's prong (1) has no back door)

## [s11] Sibling/transplant re-derivation: another BB2 function registers an entity through the same func_80045600/func_80045230/func_80045694 trio and would show the original tail idiom in already-matched C.
- mechanism: rederive modality - a matched sibling's C is direct evidence of the original source shape.
- probe: `grep -rl func_80045694 asm/funcs src` gives exactly six callers (func_80045878, func_80045B68, func_800460E4, func_800467B8, func_8004695C, func_800469C4) - every one of them is still INCLUDE_ASM, i.e. none is matched. None of the five siblings contains the tail idiom either (`0x8000` stored at +0x18 with a `+3` half-word at +0x16 appears in no other asm/funcs body).
- result: no sibling evidence exists; the transplant arm of the rederive ladder is empty for this function.
- verdict: KILLED

## [s11] A `static inline` tail helper makes the base copy and the two tail scratches non-block-local through integrate.c's parameter pseudos, with no source-level variable reuse at all (s10 frontier item 3, the last untried shape on the rederive ladder).
- mechanism: integrate.c copies the callee body into the caller and creates fresh pseudos for the parameters; a parameter copy set in the caller's block and read in the inlined body was expected to be a flow-time second-block reference, satisfying local-alloc.c:472's reg_basic_block gate without any multi-write local.
- probe: I1 (rejected/s11-inline-tail-helper-basecopy-seated-a0-109-score13.c): the six tail stores moved into `static inline void func_80045878_set(s16 *p, s32 hi, s32 id, s32 kind, s32 flags)`, called once as `v0 = s1; func_80045878_set(v0, a0+3, a0, a1, 0x8000);`, no carrier local anywhere. I2 (rejected/s11-inline-tail-helper-param-copy-cse-folded-107-score9.c): same helper called with `s1` directly. Both measured with `sandbox func_80045878 --disable all`.
- result: I1 = 109 insns / score 13; I2 = 107 insns / score 9. The premise is false: integrate.c emits the argument copies in the CALLER's block and splices the branch-free helper body into that same block, so every parameter pseudo stays block-local. I1 lands in the known failing configuration (first scratch takes $v0 from local_alloc, multi-block base allocno `v0` pushed to $a0 by global_alloc, extra `move a0,v0` at function top). In I2 cse folds the parameter copy and the base copy insn disappears entirely.
- verdict: KILLED

## [s11] The inline-helper form nevertheless reproduces target's tail INSTRUCTION SEQUENCE and ORDER exactly with no carrier local, so the carrier is needed only for register seats.
- mechanism: integrate.c splices the helper body verbatim, so the sched1 ordering the `c` carrier was believed to produce (the `li 0x8000` sitting four insns ahead of its use, between the 0x16 and 0x4 stores) falls out of the helper's source order by itself.
- probe: Normalized disassembly diff of I1's text1a_c.o against asm/funcs/func_80045878.s (tmp/grind/func_80045878/s11/).
- result: Ours `move a0,s1 / addiu v0,s2,3 / sh v0,22(a0) / li v0,0x8000 / sh s2,4(a0) / sh s5,8(a0) / sh s2,20(a0) / sh s2,16(a0) / sw v0,24(a0)` vs target's identical stream in $v0/$v1. The whole function differs in exactly three places, all downstream of one seat decision: the extra top-of-function `move a0,v0`, the base/scratch two-register rename, and the third if's last-branch delay slot (ours fills it with the else arm's `move a0,s3` and retargets the branch past it; target leaves a nop).
- verdict: CONFIRMED

## [s11] Prong (1) of the s10b structural theorem (both tail scratches must be non-block-local) holds for a second, independent reason: any block-local pseudo in the tail takes $v0 ahead of the base, unconditionally and regardless of qty priority.
- mechanism: config/mips/mips.h does not define REG_ALLOC_ORDER anywhere, so local-alloc.c:2249-2272 takes the `#ifndef REG_ALLOC_ORDER` arm and scans hard registers in ascending regno order, where $2 ($v0) is the first allocatable GP register; toplev.c runs local_alloc before global_alloc.
- probe: Source read of tools/gcc-2.7.2/local-alloc.c:2249-2272 plus a REG_ALLOC_ORDER grep over tools/gcc-2.7.2/config/mips/ (no hits); corroborated by I1's seats and by s9/s10's T1/T3/U4/U5 measurements.
- result: Independent confirmation. The theorem's prong (1) no longer rests only on the local-alloc.c:472 reg_basic_block gate.
- verdict: CONFIRMED

## [s11] The theorem can be side-stepped from the other end: if the BASE copy were block-local it would outrank the scratches on local-alloc.c's qty_compare priority, take $v0 first, and leave $v1 for the scratches - removing the non-block-local requirement on the scratches entirely.
- mechanism: local-alloc.c:1641 qty_compare ranks by log2(n_refs)*n_refs*qty_size/(qty_death-qty_birth); the tail base has 7 refs over ~9 insns (priority ~15555) against a scratch's 2 refs over 2 insns (priority ~10000).
- probe: P4 (rejected/s11-block-local-multiset-base-copy-folded-108-score9.c), a MECHANISM PROBE ONLY (it carries a fresh multi-write local and is not shippable): a fresh block-local base given two sets from the same source (`p = s1; p[11] = a0+3; p = s1; ...`) to reproduce the multi-SET property that keeps the accepted `v0 = s1` copy alive while staying inside one block.
- result: 108 insns / score 9. cse folds both copies and the second set is deleted as a dead store; all six stores address through $s1 and an unrelated `move v0,s2` appears. A surviving base copy requires its two sets to carry DIFFERENT values in DIFFERENT blocks - which is exactly the multi-block `v0` reuse, which is exactly the configuration that loses $v0 to a block-local scratch. The loophole has no back door.
- verdict: KILLED

## [s11] A matched sibling function registering an entity through the same func_80045600/func_80045230/func_80045694 trio would show the original tail idiom in already-matched C (the transplant arm of the rederive ladder).
- mechanism: Rederive modality: a matched sibling's C is direct evidence of the original source shape.
- probe: `grep -rl func_80045694 asm/funcs src` plus an idiom search over asm/funcs for a 0x8000 store at +0x18 next to a +3 half-word store at +0x16.
- result: func_80045694 has exactly six callers (func_80045878, func_80045B68, func_800460E4, func_800467B8, func_8004695C, func_800469C4) and every one of them is still INCLUDE_ASM; no other asm/funcs body contains the tail idiom. The sibling/transplant arm is empty for this function - no future session should re-run that search.
- verdict: KILLED

## s12 (2026-08-30, structural modality)

Mandated modality: structural (block-local var splits, declaration order, type
narrowing, statement re-association). Owner directive from the queue item
(escalation-batch ruling 10, "ledger states nothing pends; active with modality
change") was already discharged by s11 and is discharged again here: the
function was worked, not re-escalated.

### H-S12-A — "an EXISTING local can carry a tail scratch if it is RE-SET in the tail"
Statement: re-setting `s3` in the tail (`s3 = a0 + 3; v0[11] = s3;`) supplies the
a0+3 scratch with a variable that is already multi-block, is ordinary compliant C
(a real value that is really read), and would cut the fresh carrier `c` from three
writes to two.
Mechanism predicted: s3 is already mentioned in several blocks, so it never reaches
local_alloc; global_alloc would rank it against the base and might seat it $v1.
Probe: candidate.c with only that substitution; sandbox --disable all.
Result: score 20 at 108 insns (chassis = 0).
Verdict: **KILLED.** s3 crosses 3 calls, and GCC 2.7.2 keeps one allocno per pseudo
for the whole function (no live-range splitting), so the tail set lengthens s3's
range to the last insn rather than creating a short caller-save range. The
re-ranking is global, not local: the regression is 20 points spread over the body.
Banked: rejected/s12-s3-reset-in-tail-perturbs-whole-fn-score20.c.

### H-S12-B — "the compliant plain tail's residual is purely register seating"
Statement: with the carrier deleted (the fully Judge-compliant tail), the residual
should be register seats only, so a seating lever alone would close it.
Probe: re-measure + objdump the emitted tail.
Result: score 14 at 110 insns; emitted tail seats the base at $a0 and BOTH scratches
at $v0, and carries an extra `move v0,s2`. Not seats only — two extra instructions.
Verdict: **KILLED** as stated, but CONFIRMED as the mechanism demonstration: it is the
first direct observation, in emitted code rather than by inference, of local_alloc
claiming $v0 for every block-local tail quantity and pushing the multi-block base
allocno out to $a0. Banked: rejected/s12-compliant-plain-tail-110-score14.c,
tmp/grind/func_80045878/s12/m2_tail_objdump.txt.

### H-S12-C — "a block-local base copy is constructible after all" (the s11 frontier's own escape)
Statement: s11 left open that a surviving block-local base copy would dissolve the
whole problem (its qty_compare priority, 7 refs over ~9 insns, outranks a 2-ref
scratch, so it would take $v0 and leave $v1 for the scratches) and recorded only that
it was empirically "unconstructible".
Probe: read the pass that decides it — cse.c `make_regs_eqv` (cse.c:836-864), plus the
`.cse` dump for the live candidate (insn 213 survives).
Result: the canonicalization test is
`(uid_cuid[regno_last_uid[new]] > cse_basic_block_end
  || uid_cuid[regno_first_uid[new]] < cse_basic_block_start)
 && uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]`,
with the uids computed by reg_scan over the WHOLE function before cse runs. A copy's
destination survives only if it is mentioned outside the current extended basic block;
a mention outside the block is exactly what makes flow.c mark the pseudo
REG_BLOCK_GLOBAL and route it to global_alloc.
Verdict: **KILLED — and upgraded from empirical to closed-form.** "Base copy survives"
and "base pseudo is block-local" are mutually exclusive by construction. No source-level
spelling can hold both. This retires the s11 frontier item that asked for a lever moving
the base allocno onto $v0 while a block-local scratch exists: with a block-local scratch
present, local_alloc has already taken $v0 before global_alloc sees the base, and the only
way to deny it is a hard-$v0 conflict across the tail, which this void, call-free tail
block cannot produce without inventing a dead read or write (a cheat, not a lever).

### H-S12-D — "some existing variable other than v0 is a legal carrier"
Statement: the search for a compliant carrier has been spelling-driven; a census would
say whether any candidate exists at all.
Probe: read the `.lreg` register table for the function.
Result: reg 72 (a0) crosses 14 calls, reg 73 (a1) 14, reg 74 (a2) 9, reg 75 (s0) 8,
reg 77 (s1) 12, reg 78 (s3) 3. Exactly two user pseudos are call-free and multi-block:
reg 76 (`v0`) and reg 79 (the fresh carrier `c`). `v0` is consumed by the base.
Verdict: **KILLED, exhaustively.** There is no existing local or parameter left. This is
a complete enumeration, not a sample, so it closes the axis rather than narrowing it.

### Net position after s12
The five-step foreclosure in evidence.md [s12] shows the Judge's 2026-08-30 constraint
("no fresh local written more than once as a carrier") admits NO satisfying form for this
residual: steps (1) and (2) are properties of cse.c and local-alloc.c, step (5) is a
complete variable census, and step (4) is measured twice. The structural modality is
therefore closed with a proof rather than a plateau. The function still has two
independent byte-exact pure-C forms (candidate.c, alt_anchor_e1.c); the only open item is
the family/routing question the s11 frontier already named, now with a much stronger
packet: it is no longer "we could not find another spelling", it is "no other spelling
exists".

## [s12] An EXISTING local can carry the tail's a0+3 scratch if it is RE-SET in the tail (s3 = a0 + 3; v0[11] = s3;), which is ordinary compliant C and would cut the fresh carrier from three writes to two.
- mechanism: s3 is already multi-block so it never reaches local_alloc; global_alloc would rank it against the base allocno and might seat it $v1, which is target's seat for that value.
- probe: candidate.c with only that substitution applied over the INCLUDE_ASM line; sandbox func_80045878 --disable all.
- result: score 20 at 108 insns (chassis with candidate.c = score 0 at 108/108). s3 crosses 3 calls per the .lreg table; GCC 2.7.2 keeps one allocno per pseudo for the whole function with no live-range splitting, so the tail set lengthens s3's range to the last insn instead of creating a short caller-save range, and the re-ranking perturbs seats across the whole body rather than just the two tail insns. Banked rejected/s12-s3-reset-in-tail-perturbs-whole-fn-score20.c.
- verdict: KILLED

## [s12] With the fresh carrier deleted (the fully Judge-compliant plain tail) the residual is purely register seating, so a seating lever alone would close the function.
- mechanism: If the only damage were which hard register each tail value lands in, the instruction count would stay at 108 and only register fields would differ.
- probe: Re-measure the carrier-free form on today's chassis and objdump the emitted tail out of tmp/sandbox/func_80045878/text1a_c.o.
- result: score 14 at 110 insns, so not seats only. Emitted tail: move a0,s1 / addiu v0,s2,3 / sh v0,22(a0) / move v0,s2 / sh v0,4(a0) / sh v0,20(a0) / sh v0,16(a0) / li v0,0x8000 / sh s5,8(a0) / sw v0,24(a0). BOTH block-local scratches take $v0 and the multi-block base is pushed out to $a0 - the first direct observation in emitted code, rather than by inference, of local_alloc claiming $v0 ahead of global_alloc. Banked rejected/s12-compliant-plain-tail-110-score14.c.
- verdict: KILLED

## [s12] A surviving BLOCK-LOCAL tail base copy is constructible after all (s11 left this open as the one lever that would dissolve the problem, since local-alloc.c qty_compare ranks a 7-ref base above a 2-ref scratch and it would take $v0 first).
- mechanism: s11 recorded only an empirical 'unconstructible'; the deciding pass had never been read.
- probe: Read cse.c make_regs_eqv (tools/gcc-2.7.2/cse.c:836-864) and check the .cse dump of the live candidate for the surviving copy insn.
- result: The canonicalization test is (uid_cuid[regno_last_uid[new]] > cse_basic_block_end || uid_cuid[regno_first_uid[new]] < cse_basic_block_start) && uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]], with the uids computed by reg_scan over the whole function before cse runs. A copy's destination stays canonical - and the copy insn therefore survives - only if the pseudo is mentioned outside the current extended basic block. That same outside mention is exactly what makes flow.c mark the pseudo REG_BLOCK_GLOBAL and route it to global_alloc. Confirmed in the dump: .cse insn 213 (set (reg/v:SI 76) (reg/v:SI 77)) survives for the existing variable v0, while every prior fresh tail-only p was folded away. 'Base copy survives' and 'base pseudo is block-local' are mutually exclusive by construction.
- verdict: KILLED

## [s12] Some existing variable other than v0 is a legal (call-free, multi-block) carrier for the tail scratches - the search so far has been spelling-driven rather than exhaustive.
- mechanism: global_alloc seats any allocno that crosses a call in the callee-save class, so a carrier must be call-free; a complete census of the function's pseudos answers the question outright.
- probe: Read the .lreg register table for func_80045878 (pwsh tools/grinder/dump.ps1, tmp/grind/func_80045878/s12/f.lreg).
- result: reg 72 (a0) crosses 14 calls, reg 73 (a1) 14, reg 74 (a2) 9, reg 75 (s0) 8, reg 77 (s1) 12, reg 78 (s3) 3. Exactly two user pseudos are call-free and multi-block: reg 76 (v0) and reg 79 (the fresh carrier c). v0 is already consumed by the tail base copy. This is a complete enumeration, not a sample, so the axis is closed rather than narrowed.
- verdict: KILLED

## [s12] The Judge's 2026-08-30 constraint (no fresh local may be written more than once to serve as a carrier) admits at least one satisfying byte-exact form, findable by structural respelling.
- mechanism: The constraint explicitly directed the next session to find the remaining anchor via an existing variable or a structural respelling, which presumes such a form exists.
- probe: Compose the four results above into a foreclosure argument and test each step against a measurement or against the compiler source.
- result: Unsatisfiable, in five steps: (1) the tail base copy survives cse only if its pseudo is mentioned outside the tail block, hence it is multi-block, hence global_alloc; (2) any block-local tail scratch is taken by local_alloc, which runs first and takes $v0, so the base cannot then hold $v0 - measured, score 14 this session and score 10 in s10; (3) therefore both tail scratches must be non-block-local; (4) both tail values must be produced IN the tail, because the tail is a join block where cse's table is reset and target recomputes addiu $v1,$s2,3 there - producing either earlier folds a0+3 into s3 or relocates the 0x8000 insn out of the tail, both measured in s10; (5) no existing call-free variable remains per the census. So the carrier must be a fresh local written once per tail value plus mentioned once outside the tail - more than one write. Steps (1) and (2) are properties of cse.c and local-alloc.c, not of the source text, so no spelling, ordering, typing, or block-structuring change escapes them.
- verdict: CONFIRMED
