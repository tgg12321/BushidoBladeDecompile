# Evidence bank — func_80045878

## s1 (recon, 2026-07-23)
- Baseline reconfirmed: canonical verdict C; sandbox --disable all score=10,
  build_insns=107, target_insns=108, 10 rules dropped.
- m2c target shape captured: tmp/grind/func_80045878/s1/m2c_target.c. Confirms
  a record-init/dispatch fn; struct pointer var_s1; s3=a0+3 written TWICE
  (pre-if + else). HEAD build dump: .../s1/build_head_score10.txt.
- Gap decomposition CONFIRMED via objdump: (A) 1 missing insn = folded else
  recompute `addiu s3,s2,3`; (B) tail store block base-copy/register mismatch
  (~9 diffs) — target `addu v0,s1,zero` + stores via v0 + `sh s2,..` direct +
  scratch in v1; ours stores via s1 with an extra `move v0,s2`.
- PROBE (arm-split, set s3 in both arms, drop pre-if init): build 107->108
  (Gap A recompute MATERIALIZES) BUT score 10->11 — sched1 hoists the fresh
  recompute 3 slots early (feeds next call arg) vs target's last position.
  => Gap A insn is REACHABLE in pure C; residual is scheduling-anchor only.
  Saved rejected/armsplit-s3-materializes-but-sched-early.c (building block).
- Duplicates: no useful analog — text1a_c near-clone report lists func_80045878
  under no lead; nearest COMPLETED-C siblings are unrelated shapes.

- WIP rejected_form: {'form': 's16 *p = s1; tail stores via p', 'score': 10, 'reason': 'Not a cheat, no effect: GCC copy-propagates the single-set alias (s1 dead after the block), so no `addu v0,s1,zero` materializes. build stayed 107. Need a form that keeps the copy live.'}

- == imported from memory/wip notes.md ==
# func_80045878 (text1a_c.c) — WIP, BLOCKED, register/codegen cluster (10 rules)

## TL;DR (2026-06-14)
HEAD byte-matches only via 10 regfix rules = 1 lost-codegen `insert_after
"addiu $19,$18,3" @30` + a 9-rule tail-store `subst` cluster (idx 86-94).
Two coupled gaps toward COMPLETED-C; no pure-C form found this session.

## Gap A — folded recompute (build is 1 insn short: 107 vs 108)
Target recomputes `addiu $s3,$s2,3` (= a0+3) TWICE. Source has `s3=a0+3` (top,
line 1877) and `s3 = a0 - -3` (else, line 1891) — GCC folds them to one. The
`insert_after @30` injects the missing one. Need a structural change so the
else recompute survives (shared-end-label family).

## Gap B — tail store base copy
Target's final store block (`.L800459DC`):
```
addu  v0, s1, zero          ; copy s1 base into caller-save $v0
addiu v1, s2, 3             ; v1 = a0+3
sh    v1, 0x16(v0)          ; s1[11] = a0+3
ori   v1, zero, 0x8000      ; v1 = 0x8000
sh    s2, 0x4(v0)           ; s1[2]  = a0
sh    s5, 0x8(v0)           ; s1[4]  = a1   (s5 = a1)
sh    s2, 0x14(v0)          ; s1[10] = a0
sh    s2, 0x10(v0)          ; s1[8]  = a0
sw    v1, 0x18(v0)          ; *(s1+0x18) = 0x8000
```
My build stores directly through `$s1` (no copy). A single-set `s16 *p = s1;`
alias is copy-propagated away (s1 dead after the block) → no `addu v0,s1,zero`
materializes, build stayed 107. Need the copy to survive: keep s1 live past
the copy, reuse the param as a cursor, or name the copy as a kept value (see
param-reuse-base-copy-cse-canon).

## Resume steps
1. Keep HEAD body. Work Gap A (recompute survival) and Gap B (materialized
   base copy) in pure C.
2. VERIFY via full build: the masked sandbox CANNOT see the $v0-vs-$s1 base
   choice; need build_insns 108 AND retire SHA1 == oracle.

## Ruled out (do not re-derive)
- `s16 *p = s1;` tail alias: copy-propagated, no effect (still 107 / floor 10).

## Pointers
- `.claude/rules/param-reuse-base-copy-cse-canon.md` (materialize a base copy)
- `.claude/rules/lost-codegen-insert-cheat.md` + `shared-end-label.md` (Gap A)
- Same hard cluster class as this batch's motion_SetMotion / func_8006156C.


- [s1] Baseline: canonical verdict C; sandbox --disable all score=10, build_insns=107, target_insns=108, 10 rules dropped.

- [s1] m2c target shape captured (tmp/grind/func_80045878/s1/m2c_target.c): record-init/dispatch fn, struct ptr var_s1, s3=a0+3 written twice (pre-if + else).

- [s1] Gap A (1 insn) = folded else recompute `addiu s3,s2,3`; Gap B (~9 insns) = tail store base-copy/register mismatch.

- [s1] PROBE arm-split: build 107->108 materializes Gap A recompute (Gap A is pure-C-reachable) but score 10->11 due to sched1 hoisting the recompute 3 slots early; saved rejected/armsplit-s3-materializes-but-sched-early.c as a building block.

- [s1] Duplicates scan: no useful analog for func_80045878 in the near-clone report.

- [s1] src/text1a_c.c reverted to the score-10 HEAD form; sandbox re-verified score=10. No rules/pipeline/engine files touched.

## s2 (structural, 2026-07-23)
- Full target asm captured (asm/funcs/func_80045878.s). Confirms HEAD source is
  STRUCTURALLY the target up to Gap A: pre-if `s3=a0+3` lands in the beqz delay
  slot (0x800458AC == HEAD 2c2c), else recompute at 0x800458F8 (else last).
- Gap A mechanism NAILED via cc1 -da dumps (tmp/grind/func_80045878/s2/): the
  else recompute is RTL insn 74 `(set reg75 (plus reg72 3))` — CSE DELETES it
  (gone in dump.i.cse: insn 71->code_label 76 directly) because reg75 already
  == reg72+3 (reg72=a0 in callee-save s2, unchanged across the else calls;
  reg75 untouched). Pre-if init (needed to anti-dep-anchor the redefinition for
  sched) is EXACTLY what feeds cse the available expression => catch-22. No
  value-neutral spelling escapes: cse constant-folds `a0-(-3)`, `(a0+1)+2`, etc.
  to (plus a0 3) BEFORE the availability check.
- Gap B root cause NAILED via RTL: our fork expands `s1[11]=a0+3` by truncating
  a0 to HImode FIRST (insn 213 `reg100=(HI)a0`; insn 215 `reg101=(SI)reg100+3`),
  then cse REUSES reg100 for the three `s1[N]=a0` HI stores (insns 220/226/229
  substitute reg100 for (subreg:HI reg72)). Net: base kept in s1, a0-HI CSE'd
  into v0, `move v0,s2`. TARGET instead adds 3 to full s2 (`addiu v1,s2,3`),
  stores s2 DIRECTLY 3x, and COPIES base `addu v0,s1,zero`.
- KILLED H-C (param types s16/s16/u32*): score 10->43, build 107->112. s16
  params inject sign-extends on every a0/a1 use; target has ZERO. Original params
  are register-width s32. m2c's narrow inference is wrong here.
- KILLED frontier item 3 (A+B coupling): applied arm-split (Gap A materialized,
  build 108) and disassembled the tail — BYTE-IDENTICAL to HEAD's tail
  (`sh v0,4(s1)...; li v0,0x8000; sh s5,8(s1); sw v0,24(s1)`, base=s1). Gap A's
  presence does NOT change Gap B. The two gaps are FULLY INDEPENDENT.
- KILLED Gap B lever H-B (mixed p/s1 base copy): `s16 *p=s1; p[..]=..;` for the
  HI stores + s1 referenced in the trailing word store -> score 10, build 107.
  cse copy-propagates `p=s1` regardless (both are the same available value).
  Target's base copy is a local_alloc live-range-split of the join block
  (.L800459DC, 2 preds), NOT a C-level pointer copy — unreproducible by any
  clean `p=s1`. Probed SI-temp for a0+3 (`s32 t=a0+3; s1[11]=t;`) -> build 106
  (drops the (HI)a0 truncation) but score stays 10; wrong direction.
- Artifacts: tmp/grind/func_80045878/s2/{dump.i.rtl,dump.i.cse,dump.i.greg,
  f_rtl.txt,f_cse.txt,f_greg.txt,dump.sh,dis.sh}.

- [s2] Full target asm (asm/funcs/func_80045878.s): HEAD source is structurally the target up to Gap A; pre-if s3=a0+3 lands in the beqz delay slot (0x800458AC), else recompute at 0x800458F8 (else last).

- [s2] Gap A = cse deletion of RTL insn 74 (else recompute); confirmed via dump.i.rtl (present) vs dump.i.cse (gone). reg72=a0 stays in callee-save s2 across the else calls so a0+3 is an available expression.

- [s2] Gap B = fork truncates a0->HImode for the s1[11]=a0+3 store (RTL insn 213 reg100=(HI)a0; insn 215 (SI)reg100+3), then cse reuses reg100 for the three s1[N]=a0 HI stores (base kept in s1, move v0,s2). Target adds 3 to full s2 and stores s2 directly, with base copied `addu v0,s1,zero`.

- [s2] Gap A and Gap B are INDEPENDENT: arm-split tail is byte-identical to HEAD tail.

- [s2] Param narrowing (s16/s16/u32*) is dead: original params are register-width s32 (target has zero sign-extends).

- [s2] Target's tail base copy is a local_alloc live-range split of the 2-predecessor join block (.L800459DC), not reproducible by a C-level p=s1 copy (cse always copy-propagates it).

## s3 (structural, 2026-07-23)
- Re-derived Gap B at the greg RTL (full dump this session): the fork's
  divergence is ONE callee->caller copy with the OPPOSITE choice of which
  value gets it. Fork: `move v0,s2` materializes (HI)a0 (greg insn 213,
  REG_DEAD s2) and reuses v0 for the 3 s1[N]=a0 HI stores => a0 dies mid-tail,
  base stays in s1. Target: stores s2 DIRECTLY (`sh s2,off(v0)`, s2 lives)
  and copies base s1->v0 (`addu v0,s1,zero`), a0+3 into v1. Same one copy,
  flipped. A cse+local-alloc coin-flip, not a value/structure bug.
- KILLED (measured) three structural Gap-B/A levers:
  * s1[11]=s3 (reuse live s3 for a0+3, pressure probe): build 107->105,
    score 10. Reuses s3 (target recomputes; s3 dead pre-tail) => wrong bytes,
    shorter build, no base-copy triggered. rejected/tail-s3-reuse-and-reorder.
  * Tail-store reorder (interleave a0/a0+3/word): build 107, score 10 —
    IDENTICAL to HEAD. cse re-clusters the 3 (subreg:HI s2) stores and
    re-materializes the shared (HI)s2 temp regardless of source order.
    Statement order is NOT a lever for the (HI)s2 CSE.
  * Register-pressure at the .L800459DC join: no legitimate hook — the tail
    only consumes s1,s2,s5 + constants; any 4th live value would be a dead
    (cheat) value. Dead axis for structural.
- Re-confirmed arm-split (Gap A materialize): build 108, score 11; the else
  recompute lands at build 2c6c (position N-3, before `li v0,-1; sh; sh`),
  target keeps it LAST (0x800458F8). It is NOT a priority tie — the fresh
  single-def recompute feeds the next block's call arg (a0=s3), giving it
  genuinely higher sched1 launch priority; no clean structural lever lowers
  that priority without changing bytes. Anchoring it last would be worth
  ~1 (10->9); it is a scheduling-priority tie -> directed-permuter axis.
- Artifacts: tmp/grind/func_80045878/s3/armsplit.c; reused s2 dis.sh/greg.
- VERDICT: structural modality's clean levers are now exhaustively measured
  dead for BOTH gaps. Both residuals are RA/scheduling coin-flips (Gap B:
  which callee-save gets the caller-save copy; Gap A: recompute launch
  priority). Per difficult-is-not-impossible the matching C exists; the
  named remaining tool is the directed permuter on a clean single-function
  target.o (frontier item 3), a permuter-modality session.

- [s3] Gap B is ONE callee->caller copy with flipped choice: fork copies a0 (move v0,s2, base stays s1); target copies base (addu v0,s1,zero, s2 direct). cse+local-alloc coin-flip.
- [s3] s1[11]=s3 probe: build 107->105 score 10, reuses s3 (wrong bytes, target recomputes), no base-copy triggered. KILLED.
- [s3] Tail-store reorder: build 107 score 10 identical to HEAD; cse re-clusters the (HI)s2 stores order-insensitively. KILLED.
- [s3] Register pressure at 2-pred join has no legitimate semantic hook (tail uses only s1/s2/s5+consts); dead axis for structural.
- [s3] arm-split recompute sched-early is NOT a tie — recompute feeds next call arg -> genuinely higher launch priority; no clean structural lever lowers it. Anchoring worth ~1 (10->9) but is a scheduling-priority tie -> permuter.

- [s3] Floor reconfirmed: sandbox --disable all score=10, build_insns=107 vs target 108, 10 rules dropped.

- [s3] Gap B re-derived at greg RTL (full dump this session): the fork/target divergence is ONE callee->caller copy with the OPPOSITE choice of which value gets it. Fork: `move v0,s2` materializes (HI)a0 (greg insn 213, REG_DEAD s2) and reuses v0 for the three s1[N]=a0 HI stores, so a0 dies mid-tail and the base stays in s1. Target: stores s2 DIRECTLY (`sh s2,off(v0)`, s2 stays live), copies the base s1->v0 (`addu v0,s1,zero`), and puts a0+3 in v1.

- [s3] The shared (HI)s2 temp (greg insn 213) is what forces base-in-s1; it is created by cse from the three repeated (subreg:HI s2) a0 stores and is order-insensitive (tail reorder left build/score unchanged).

- [s3] s1[11]=s3 reuse: build 107->105, score 10 (wrong bytes; target recomputes a0+3, s3 dead before tail).

- [s3] arm-split: build 108, score 11; else recompute at position N-3 (build 2c6c) vs target LAST (0x800458F8); higher sched1 launch priority from feeding the next call arg, not a tie.

- [s3] Both residuals are RA/scheduling coin-flips; per difficult-is-not-impossible the matching C exists and the named remaining tool is the directed permuter on a clean single-function target.o.

## s4 (permuter, 2026-07-23)
- Clean single-function permuter workspace stood up (tmp/perm_80045878{,_arm,_arm2,_dir}):
  full-TU honest-pipeline base.c (no regfix/asmfix), func region extracted .ent..end,
  clean target.o from asm/funcs/func_80045878.s at offset 0 (prelude minus `.set gp=64`
  for r3000). base_score 580 (HEAD chassis) / 540 (arm-split chassis). Setup + validate
  scripts in tmp/grind/func_80045878/s4/.
- FOUR chassis measured, --stack-diffs on, --stop-on-zero, all harvested+stopped:
  * HEAD (fulltu random):      ~45.9k iters, base 580, BEST 250, no zero.
  * arm-split (random):        ~45k iters,   base 540, BEST 210, no zero.
  * arm-split perm_inline=0:   ~45k iters,   base 540, BEST 210, no zero.
  * DIRECTED (this session):   21.5k iters,  base 540, BEST 210, no zero.
- The DIRECTED chassis (frontier item 1, the axis the prior random-only runs skipped):
  PERM_LINESWAP over the Gap-A else tail {s1[4]=-1; s1[3]=0; s3=recompute} +
  PERM_GENERAL over the recompute spelling (a0--3 / a0+3 / (a0+1)+2), AND PERM_LINESWAP
  over the 6 Gap-B tail stores + PERM_GENERAL for s1[11] value (a0+3 / live s3). This
  EXHAUSTIVELY enumerates every statement ordering of BOTH residual regions (~26k combos).
  Result: plateau 210 (== arm-split floor), NO sub-plateau find, NO zero.
- The best forms across all chassis are re-finds of the known arm-split plateau (Gap A
  recompute materialized-but-sched-early) + permuter synthetic temps (inline_fn wraps,
  named `new_var`/`new_var2` intermediates — none of which are closing forms; all sit
  at the same 210 plateau, i.e. noise-equivalent, not cheat-forms that matched).
- CONCLUSION: the permuter (random over 3 chassis @~135k iters + directed exhaustive
  ordering sweep) does NOT close either residual. This CONFIRMS the s3 verdict at the
  search level: both residuals (Gap A recompute launch-priority; Gap B base-copy
  callee->caller choice) are decided by cc1 local-alloc/sched1 tie-breakers BELOW the
  reach of any C-statement ordering/spelling the permuter can produce. C-source mutation
  is not the lever. Permuter modality is now measured dead for this function.
- Artifacts: tmp/grind/func_80045878/s4/{setup_perm.sh,setup_arm.sh,setup_arm2.sh,
  setup_dir.sh,monitor.sh,show_best.sh,dbg*.sh}; permuter workspaces
  tmp/perm_80045878{,_arm,_arm2,_dir}/ (base.c, compile.sh, target.o, output-*);
  telemetry in metrics/events.jsonl (4 permuter-launch + 4 permuter-harvest events).

- [s4] Directed PERM_LINESWAP+PERM_GENERAL exhaustive sweep of BOTH residual regions (~26k orderings, 21.5k iters) plateaued at 210 == arm-split floor; no zero. C-statement ordering is not the lever for either coin-flip.
- [s4] Random permuter over 3 structurally-distinct chassis (~135k iters total) + the directed sweep all top out at the same plateau (250 HEAD / 210 arm). Permuter modality measured dead: both residuals are cc1 local-alloc/sched1 tie-breaks below C-source-mutation reach.

- [s4] Floor unchanged: sandbox --disable all score 10 (build_insns 107 vs target 108), src/text1a_c.c untouched this session.

- [s4] Directed PERM_LINESWAP+PERM_GENERAL exhaustive sweep of BOTH residual regions (~26k orderings, 21.5k iters, -j8) plateaued at permuter-score 210 == arm-split floor; no zero. C-statement ordering/spelling is not the lever for either coin-flip.

- [s4] Four permuter chassis total (HEAD-random, arm-split-random, arm-split-perm_inline=0, directed), all --stack-diffs --stop-on-zero, all harvested+stopped: best 250/210/210/210, ~156k iters combined, ZERO closing forms found.

- [s4] Best-scoring permuter outputs are re-finds of the known arm-split plateau plus synthetic named temps (inline_fn, new_var/new_var2) — all sit at the same 210 plateau (noise-equivalent), none are cheat-forms that matched, none close the gap.

- [s4] This CONFIRMS the s3 structural verdict at the search level: Gap A (else-recompute launch priority) and Gap B (base-copy callee->caller choice) are decided by cc1 local_alloc/sched1 tie-breakers below the reach of any C-source mutation the permuter can produce. Permuter modality is measured dead for this function.

- [s4] All 4 permuter-launch + 4 permuter-harvest events logged to metrics/events.jsonl. No permuter processes left alive; no build-pipeline/rules/engine files touched.

## s5 (permuter, 2026-07-23)
- Baseline reconfirmed: sandbox --disable all score=10, build_insns=107 vs
  target 108, 10 rules dropped. src/text1a_c.c untouched (permuter runs in a
  separate honest-pipeline workspace, not src).
- FRESH-SEED reseed on the arm-split chassis (the closest, permuter base 540 /
  floor 210) per fresh-seed discipline — a 5th chassis beyond s4's four:
  launched via permuter_campaign.py --stop-on-zero -j8, waited in-turn (~7 min
  window), harvested+stopped. Result: 13,769 iters, base_score 540,
  best_new_score 210, 147 finds, MIN score 210 — NO sub-plateau, NO zero.
  Distinct scores {210,215,220,250,260,...}; the 210 floor is a re-find of the
  known arm-split plateau (Gap A recompute materialized-but-sched-early). This
  is BYTE-FOR-BYTE the same plateau s4's two arm-split random runs hit (both
  210) — a THIRD fresh arm-split seed confirms the basin yields nothing new.
- scan_hand_coded.py --single func_80045878 = tier=LOW score=0/8 (108 insns):
  no S1 multu pacing, no S2 empty branch, S3 7 spills/11 regs (ordinary), no
  S4 front-load burst, no S5 sibling cluster, no S6 BIOS jumptable, no S7
  unsaved $sN, no S8 redundant mask. => ordinary GCC 2.7.2 RA/sched output;
  canonical-asm is NOT supportable (no hand-written-asm evidence). Per
  endgame-lock-disposition the disposition is INCOMPLETE-owner-accepted.
- CONCLUSION: permuter modality re-confirmed dead across a 5th chassis; every
  sanctioned grind axis (structural s2/s3 + permuter s4/s5) is now measured
  dead for a committable COMPLETED-C form. No pure-C sandbox-0 form exists
  (unlike the func_80061658 cluster) — this is a reachability / RA-class wall,
  sibling to func_800611A4 / func_80049A2C / InitHiraRmd_80047FBC. Filed
  OWNER-ESCALATION in docs/grind/decisions.md (2026-07-23, func_80045878);
  outcome = owner-gated.
- Artifacts: tmp/grind/func_80045878/s5/{launch_arm_freshseed.sh,monitor_loop.sh,
  harvest_final.json,scan_hand_coded.txt,status_raw.txt}; workspace
  tmp/perm_80045878_arm/ (output-* finds); permuter-launch + permuter-harvest
  events in metrics/events.jsonl. No campaign left alive; no build-pipeline/
  rules/engine files touched.

- [s5] Fresh-seed arm-split reseed (5th chassis, 13,769 iters, base 540) plateaued at permuter-score 210 == s4 arm-split floor; MIN 210, no sub-plateau, no zero. Third fresh arm-split seed confirms the basin is dry — permuter modality re-confirmed dead.
- [s5] scan_hand_coded --single func_80045878 = LOW 0/8 (ordinary GCC RA/sched output); canonical-asm unsupportable, disposition = INCOMPLETE-owner-accepted per endgame-lock-disposition.
- [s5] All sanctioned grind axes (structural s2/s3 + permuter s4/s5) measured dead; no pure-C sandbox-0 form exists (reachability/RA-class wall, sibling of func_800611A4). Filed OWNER-ESCALATION; outcome owner-gated. src/text1a_c.c untouched; no campaign left alive.

- [s5] Floor reconfirmed: sandbox --disable all score=10 (build_insns 107 vs target 108, 10 rules dropped). src/text1a_c.c untouched this session.

- [s5] s5 fresh-seed arm-split reseed (5th chassis): 13,769 iters, base 540, best_new_score 210, MIN score 210, 147 finds, no sub-plateau, no zero - identical to s4's two arm-split runs (both 210). Permuter modality re-confirmed dead.

- [s5] scan_hand_coded.py --single func_80045878 = LOW 0/8 -> canonical-asm not supportable (no hand-written-asm evidence); disposition INCOMPLETE-owner-accepted per endgame-lock-disposition.

- [s5] Both residuals are cc1 local_alloc/sched1 coin-flips (RTL-proven s2/s3): Gap A = cse.c available-expression fold (else recompute deleted) + sched1 launch-priority (arm-split materializes it but hoists 3 slots early, genuinely higher priority feeding next call arg); Gap B = local_alloc live-range split of the 2-pred join .L800459DC (cse copy-props any C-level p=s1).

- [s5] All sanctioned grind axes measured dead: structural s2/s3 (param types s16->score43, arm-split, tail reorder, s1[11]=s3 pressure, mixed p/s1 base copy, SI-temp) + permuter s4 (4 chassis ~156k iters, plateau 250/210) + permuter s5 (5th chassis). No pure-C sandbox-0 form exists (reachability/RA-class wall), unlike the func_80061658 cluster.

- [s5] OWNER-ESCALATION filed in docs/grind/decisions.md (2026-07-23, func_80045878) with two owner options: (a) canonical-asm NOT supportable (LOW 0/8); (b) REFUSE -> OWNER-ACCEPTED INCOMPLETE as sibling func_800611A4 et al. No campaign left alive; no build-pipeline/rules/engine files touched.

- [s5] PID-reuse noise: permuter_campaign status shows recycled PID 417 as alive across many historical entries, but only armsplit-freshseed-s5 was registered ACTIVE and it was harvested --stop (0 active after).
