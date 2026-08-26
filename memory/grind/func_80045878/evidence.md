> **2026-08-24 MIGRATION NOTE:** HEAD is now `INCLUDE_ASM` (migrated in
> 4faaa384 (2026-08-19 batch 2)); rules retired, in-source cheat-asm removed. "HEAD"
> claims below describe the pre-migration tree (`retired-chassis-2026-08/body.c`).

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

## s6 (synthesis, 2026-08-26)
CHASSIS RE-MEASURE: score-10 body (candidate.c) re-applied to src and
re-measured on today's HEAD — sandbox --disable all score=10, build_insns 107
vs target 108, 0 rules. The ledger floor is intact; every banked spelling
conclusion remains chassis-valid.

### THE CENTRAL s6 FINDING — the score-10 chassis is the WRONG chassis
`goal_from_tgt.py classify text1a_c func_80045878` was run for the first time
(owner directive 2026-08-24: solver before deep re-grind). Run on the ARM-SPLIT
chassis (rejected/armsplit-…, sandbox 11, build_insns 108) it reports:

    ours 108 insns, target 108 insns   [object-level: replace_with_asmfile-safe]
    FIRST DIVERGENCE: RA        $v0 -> $v1  x1 ,  $s1 -> $v0  x1
    align ours->tgt: |A|=108 |B|=108 {'equal': 98, 'replace': 9, 'delete': 0,
                                      'insert': 0, 'moved': 1}

i.e. on the arm-split chassis our build has the EXACT target instruction
multiset (register-blanked multisets identical, 108 vs 108) and the whole
residual is 9 replaced + 1 moved instruction in two disjoint regions. The
PRE-RA gap that s5 used to justify "no pure-C-0 form exists / reachability
wall" DOES NOT EXIST on this chassis — it is an artifact of grading on the
score-10 HEAD body, which is one insn SHORT. sandbox score is NOT the right
ordering here: 11-with-the-right-multiset is strictly closer than
10-with-a-missing-insn. **All future work starts from the arm-split chassis.**

Region-aligned residual (objdump, masked, s6):
    idx  ours                     target
    29   addiu s3,s2,3            li    v0,-1          <- R1: 4-insn rotation
    30   li    v0,-1              sh    v0,8(s1)
    31   sh    v0,8(s1)           sh    zero,6(s1)
    32   sh    zero,6(s1)         addiu s3,s2,3
    89   addiu v0,s2,3            move  v0,s1          <- R2: tail block
    90   sh    v0,22(s1)          addiu v1,s2,3
    91   move  v0,s2              sh    v1,22(v0)
    92   sh    v0,4(s1)           li    v1,0x8000
    93   sh    v0,20(s1)          sh    s2,4(v0)
    94   sh    v0,16(s1)          sh    s5,8(v0)
    95   li    v0,0x8000          sh    s2,20(v0)
    96   sh    s5,8(s1)           sh    s2,16(v0)
    97   sw    v0,24(s1)          sw    v1,24(v0)

### R2 (tail) COLLAPSED to ONE missing insn — new chassis armsplit+SItemp
Adding the SI temp `{ s32 t = a0 + 3; s1[11] = t; }` to the arm-split chassis
(s2 measured this ONLY on the score-10 HEAD chassis, saw 107->106 and filed it
"wrong direction" — that mis-scoping cost s3/s4/s5) removes the HImode
truncation of a0 that created the shared (HI)a0 pseudo cse was reusing. Result
(sandbox: score 11, build_insns 107) — the tail becomes:

    addiu v0,s2,3 / sh v0,22(s1) / li v0,0x8000 / sh s2,4(s1) / sh s5,8(s1)
    / sh s2,20(s1) / sh s2,16(s1) / sw v0,24(s1)

which is target's tail EXCEPT for the single leading `move v0,s1` base copy
(and the renames that follow from having it). a0 is now stored DIRECTLY from
s2 three times, exactly as target does. The entire ~9-diff "Gap B" is now ONE
missing instruction. Banked as
rejected/chassis-armsplit-si-temp-tail-matches-except-basecopy.c (a CHASSIS,
not a dead end).
KILLED on that chassis: `s16 *p = s1;` alias over the whole tail — still
107 / score 11, byte-identical to the chassis without it (cse copy-propagates
the single-set copy regardless of tail shape).
rejected/tail-p-alias-on-armsplit-si-chassis.c.

### R1 (else-arm rotation) — FORECLOSURE PROOF from cc1 source + instrumented run
Pass attribution done by DUMP, not guess (`pwsh tools/grinder/dump.ps1`, plus
an instrumented `BB2_SCHED_DEBUG=1 tools/gcc-2.7.2/cc1` run; artifacts
tmp/grind/func_80045878/s6/{scheddbg.txt,block2_sched1.txt}). The else arm is
basic block 2, n_insns=15. Ground truth from the instrumented run:

    SCHEDDBG units n0=2 n1=0 ...            (memory unit has exactly the 2 `sh`)
    node insn=75 (addiu s3,s2,3) unit=-1 icost=0 pri=1 ref=0 bmin=-1 bmax=-1
                 -- and NO dep lines at all: zero LOG_LINKS
    node insn=69 (sh v0,8(s1))   unit=0  icost=0 pri=1 bmin=1 bmax=3
    node insn=72 (sh zero,6(s1)) unit=0  icost=0 pri=1 bmin=1 bmax=3
    initial ready list (sorted by SCHED_SORT): pos=0 insn=75, pos=1 insn=72,
                                               pos=2 insn=69
    SELBEST clock=1 insn=72 pos=1   ->  PICK clock=1 picked=72

Reading (sched.c is backward: the insn picked at clock 1 becomes the block's
LAST insn). rank_for_schedule ALREADY sorts our recompute (highest LUID = last
in source) into pos 0 — i.e. target's answer. It is then overridden by
`schedule_select` (sched.c:2661-2727), which, within a group of equal
INSN_PRIORITY, picks the insn with the largest `potential_hazard`
(sched.c:1327-1364):

    ncost = minb * 0x40 + maxb;                      /* 1*0x40+3 = 0x43 */
    ncost *= (unit_n_insns[unit] - 1) * 0x1000 + unit;   /* (2-1)*0x1000 */

For the two stores (function unit "memory", mips.md:161) that is 0x43*0x1000 =
274432. For the recompute, `insn_unit` is -1 (no function unit) so
potential_hazard returns 0. **An arith insn can therefore NEVER win a
same-priority tie against a store in the block-end slot.** The only C-visible
escapes are:
  E1  give the recompute INSN_PRIORITY >= 2 so it forms its own, earlier
      group. priority() (sched.c:1432-1500) is
      `max(priority(pred) + insn_cost(pred) - 1, 1)`, so this needs a
      predecessor with cost 2 — on r3000 that is a LOAD (mips.md:157-159,
      load ready-delay 2). MEASURED FACT: block 2 contains ZERO loads (every
      other insn is a call or arith, unit=-1), and target's else arm
      (asm/funcs/func_80045878.s, 0x800458BC-0x800458F8) also contains zero
      loads. So no in-block load exists to hang the recompute off.
  E2  make `unit_n_insns[memory] == 1` for the block — then the factor
      `(1-1)*0x1000 + 0` is 0, all costs tie at 0, and pos 0 (the recompute)
      wins. Requires one of the two `sh` to leave the block.
  E3  make both stores unavailable at the block-end slot — impossible: the
      scheduler is backward, readiness means "all dependents scheduled", and
      both stores have ref=0 (no dependents at all).
Since target's else block has the same 15 insns, the same 2 stores, and no
loads, the target's order is NOT producible by sched1/sched2 from a block with
this shape. => target's `addiu` was NOT in this basic block when the scheduler
ran, or the block was split at that point. E2 (block splitting) is the live
lever, not spelling.

### Disposition
NOT owner-gated: the 2026-07-23 escalation for this function was already ruled
(2026-07-27, option (b)) and the owner kicked the function back to active on
2026-08-24; re-filing the same "no pure-C form exists" packet is the
auto-reject class, and it is now positively CONTRADICTED by the multiset-exact
arm-split classification above. Floor unchanged at 10; src/text1a_c.c restored
to HEAD (INCLUDE_ASM) at end of session.

- [s6] Chassis re-measure 2026-08-26: the score-10 body re-applied to src/text1a_c.c still scores sandbox --disable all = 10, build_insns 107 vs target 108, 0 rules dropped. Ledger floor intact; banked spelling conclusions remain chassis-valid.

- [s6] goal_from_tgt.py classify on the arm-split chassis: ours 108 / target 108, register-blanked multisets identical, FIRST DIVERGENCE = RA ($v0->$v1 x1, $s1->$v0 x1), align 98 equal / 9 replace / 0 delete / 0 insert / 1 moved. The s5 'no pure-C-0 form exists, reachability/RA-class wall' conclusion was an artifact of grading on a chassis that is one insn short.

- [s6] Residual is exactly two disjoint regions on the arm-split chassis: R1 = idx 29-32, a 4-insn rotation (ours addiu/li/sh/sh vs target li/sh/sh/addiu); R2 = idx 89-97, the tail store block.

- [s6] arm-split + SI temp ({ s32 t = a0 + 3; s1[11] = t; }) makes the tail store a0 directly from s2 three times exactly as target does; the whole ~9-diff Gap B reduces to the single missing `addu v0,s1,zero` base copy. Banked as memory/grind/func_80045878/rejected/chassis-armsplit-si-temp-tail-matches-except-basecopy.c (a CHASSIS, not a dead end).

- [s6] s2 measured that same SI temp only on the score-10 HEAD chassis (107->106, score unchanged) and filed it 'wrong direction'; that mis-scoping is what routed s3/s4/s5 into the coin-flip/permuter dead end.

- [s6] `s16 *p = s1;` over the tail is copy-propagated on the new chassis too (107 / score 11, byte-identical) — the base copy is not reachable by any C-level pointer copy measured to date.

- [s6] sched1 ground truth (instrumented cc1, BB2_SCHED_DEBUG=1): block 2 n_insns=15, units n0=2; insn 75 (addiu s3,s2,3) unit=-1 icost=0 pri=1 ref=0 and NO dep lines at all; insns 69/72 (the two sh) unit=0 bmin=1 bmax=3 pri=1; initial SCHED_SORT ready list pos0=75 pos1=72 pos2=69; SELBEST clock=1 insn=72 pos=1.

- [s6] Mechanism named from cc1 source, not guessed: sched.c:2661-2727 schedule_select picks the largest potential_hazard within an equal-INSN_PRIORITY group; sched.c:1327-1364 computes ncost = (minb*0x40 + maxb) * ((unit_n_insns[unit]-1)*0x1000 + unit), which is 274432 for a store on the 'memory' unit (mips.md:161) when the block holds two memory insns, and 0 for any insn with insn_unit == -1. unit_n_insns is cleared per block (sched.c:3251), so it is a per-block, C-visible quantity.

- [s6] sched.c:1432-1500 priority() = max(priority(pred) + insn_cost(pred) - 1, 1), so INSN_PRIORITY >= 2 requires a predecessor of cost 2 — on r3000 a LOAD (mips.md:157-159, load ready-delay 2). Measured: block 2 has zero loads, and target's else arm 0x800458BC-0x800458F8 has zero loads.

- [s6] Target's else arm and ours contain the same 15 instructions and the same 2 stores, so target's order is NOT producible by sched1/sched2 from a block of this shape — the recompute must have been in a DIFFERENT basic block when the scheduler ran. That makes block SPLITTING (escape E2), not spelling, the live lever.

- [s6] Target's THEN arm ends `lw s1,4(v0); j .L800458FC; nop` — an UNFILLED delay slot at 0x800458B8 that reorg would normally have filled from the preceding lw. Our arm-split build reproduces insns 0-28 byte-identically including that j+nop, so the divergence is localised strictly inside the else arm.

- [s6] Disposition note: owner-gated is NOT available here. The 2026-07-23 escalation was already ruled (2026-07-27, option (b)) and the owner returned the function to active on 2026-08-24; re-filing the same 'no pure-C form exists' packet is the auto-reject class, and it is now positively contradicted by the multiset-exact classification.

- [s6] src/text1a_c.c restored to HEAD (INCLUDE_ASM) at end of session; no rules/pipeline/engine files touched; no permuter campaigns launched, none left alive.


## s7 (2026-08-26, solver modality)

- [s7] Chassis re-measure: the s6 arm-split body scores sandbox --disable all
  = 11 at build_insns 108 vs target 108; the score-10 candidate.c body still
  scores 10 at build_insns 107. Ledger floor 10 intact and re-verified this
  session with the edits in place.
- [s7] TOOL DEFECT for this function: `inverse_compose.py classify text1a_c
  func_80045878` takes the TEXT-stream path (the function is INCLUDE_ASM-routed,
  not `replace_with_asmfile`-wired) and returns a spurious "FIRST DIVERGENCE:
  PRE-RA" with a 105-vs-106 insn count, because `.hon.s` is maspsx source and
  the target side is disassembly (`addu $r,$r,3` vs `addiu $r, $r, 0x3`). The
  correct tool is `goal_from_tgt.py classify`, which reports 108/108 and
  FIRST DIVERGENCE: RA. Reports saved at
  tmp/grind/func_80045878/s7/classify_armsplit.txt vs classify_obj_armsplit.txt.
- [s7] cse.c:826 `make_regs_eqv` is the pass that eats the tail base copy, and
  its canonicality condition is C-reachable: a pseudo needs a mention in an
  EARLIER basic block (regno_first_uid comes from reg_scan, which runs before
  cse) for `regno_first_uid[p] < cse_basic_block_start` to hold. That single
  fact converts three sessions of "the base copy is copy-propagated, mechanism
  unknown" into a spelled lever.
- [s7] NEW CHASSIS "P4": s6 armsplit+SItemp + `s16 *p;` whose EARLY mention is
  the then-arm store (`p = s1; p[3] = 0;`) + `p = s1;` at the join + the whole
  tail through p. sandbox --disable all = 12, build_insns 108 == target 108,
  alignment index-for-index. Banked at
  memory/grind/func_80045878/rejected/chassis-p4-thenarm-p-tail-pure-rename.c.
  The then-arm copy is propagated away and deleted (it costs nothing); the
  join copy survives and lands in target's exact position.
- [s7] On P4 the ENTIRE remaining residual is: (R2) idx 89-97, nine
  instructions in target's exact order and shapes, differing ONLY by
  $a0 -> $v0 and $v0 -> $v1; and (R1') idx 50, where ours fills the
  `beq v1,v0` delay slot with `move a0,s3` (duplicated from idx 53) and target
  leaves a `nop` (reorg.c). The s6 "R1 else-arm 4-insn rotation" is CLOSED on
  this chassis. Side-by-side: tmp/grind/func_80045878/s7/sbs_p4.txt.
- [s7] ra_solver ground truth on the P4 body: `extract.py` + `simulate.py`
  reproduce the allocation 8/8 dispositions with sort order MATCH (the model
  is exact here, so its negatives are trustworthy).
  `local_extract.py`/`local_alloc.py text1a_c --func 80045878` report NO QTY
  ROWS - local-alloc allocates nothing in this function, so the tail base and
  tail scratch are both global.c allocnos.
- [s7] The blocking inputs, named exactly (simulate.py --trace):
  pseudo 78 (tail base p) pri=30000 calls=0 hard_conf=[2,29] prefs=[4,5]
  -> best=$a0; pseudo 76 (tail scratch) pri=10000 calls=0 hard_conf=[29]
  prefs=[2,4,5] -> best=$v0. p is allocated first and is barred from $v0 by a
  HARD conflict with hard reg 2, then takes $a0 off its preference list.
- [s7] `inverse.py global --goal '{"78": 2, "76": 3}' --depth 2` = NEGATIVE
  RESULT / FORECLOSED across refs, live length, birth order, conflicts,
  preferences and calls-crossed; 8 preference atoms additionally reported
  mechanically unreachable (prune_preferences global.c:897 strips $v0 prefs
  from call-crossing allocnos; $v1 never appears as a hard reg in the pre-RA
  RTL so set_preference can never prefer it). Report:
  tmp/grind/func_80045878/s7/inverse_p4.txt.
- [s7] Reusing the existing local `v0` as the tail base (instead of a new p)
  reproduces target's tail shape too but costs +1 insn (109): the v0 pseudo
  then spans the call-result region, is hard-conflicted with $v0 there, gets
  $a0 for its whole range and needs `move a0,v0` at idx 12.
  rejected/tail-base-reuse-v0-hard-conflicts-with-v0.c;
  `inverse.py global --goal '{"76": 2}'` on that body = FORECLOSED too.
- [s7] Placing p's early mention in the ELSE arm instead of the THEN arm leaves
  a second, non-removable copy (109 insns, score 15).
  rejected/tail-base-p-early-mention-in-else-arm-copy-survives.c.
- [s7] No FAKE/coercion construct was written or measured this session; every
  probe is plain C (a named pointer local used for real stores).
  src/text1a_c.c restored to HEAD (INCLUDE_ASM) at end of session; no
  rules/pipeline/engine files touched; no permuter campaigns launched, none
  left alive.

- [s7] Floor re-verified this session with edits in place: the score-10 candidate.c body still scores sandbox --disable all = 10 at build_insns 107; the s6 arm-split body scores 11 at build_insns 108 == target 108.

- [s7] TOOL ROUTING for this function: `inverse_compose.py classify` takes the text-stream path (INCLUDE_ASM-routed, not replace_with_asmfile-wired) and returns a spurious PRE-RA verdict with a 105-vs-106 insn count; `goal_from_tgt.py classify` is the correct entry point and returns 108/108 FIRST DIVERGENCE: RA.

- [s7] cse.c:826 make_regs_eqv is the pass that eats the tail base copy; its canonicality condition needs regno_first_uid[p] < cse_basic_block_start, and regno_first_uid comes from reg_scan which runs before cse -- so an early mention in ANY prior basic block suffices, even one cse itself later deletes.

- [s7] NEW CHASSIS 'P4' (memory/grind/func_80045878/rejected/chassis-p4-thenarm-p-tail-pure-rename.c): s6 armsplit+SItemp + `s16 *p;` with its early mention as the then-arm store (`p = s1; p[3] = 0;`) + `p = s1;` at the join + the whole tail through p. sandbox --disable all = 12, build_insns 108 == target 108, aligned index-for-index. Supersedes the s6 chassis.

- [s7] On P4 the ENTIRE remaining residual is (R2) idx 89-97 -- nine instructions in target's exact order and shapes differing only by $a0->$v0 and $v0->$v1 -- plus (R1') idx 50, where ours fills the `beq v1,v0` delay slot with `move a0,s3` and target leaves a nop. The s6 else-arm 4-insn rotation is CLOSED on this chassis.

- [s7] ra_solver model is exact on the P4 body: extract.py + simulate.py = 8/8 dispositions, sort order MATCH. local_extract.py/local_alloc.py report NO qty rows for func_80045878, so local-alloc allocates nothing here and every pseudo is global.c's -- the global model's negatives are trustworthy for this function.

- [s7] Blocking inputs named exactly (simulate.py --trace on P4): pseudo 78 = tail base p, pri=30000 calls=0 hard_conf=[2,29] prefs=[4,5] -> best=$a0; pseudo 76 = tail scratch, pri=10000 calls=0 hard_conf=[29] prefs=[2,4,5] -> best=$v0.

- [s7] inverse.py global --goal {78:2, 76:3} --depth 2 = FORECLOSED over refs/livelen/birth-order/conflicts/preferences/calls-crossed; the two inputs that must move (pseudo 78's HARD conflict with hard reg 2, and its $a0/$a1 preference set) are NOT in the atom space, so they must be attacked structurally from the C and the model re-extracted.

- [s7] Placing p's early mention in the ELSE arm instead of the THEN arm leaves a second, non-removable copy (109 insns, score 15); duplicating `p = s1` into both arms gives 108 insns but two copies with the else-arm one hoisted by sched1.

- [s7] No FAKE/coercion construct was written or measured this session -- every probe is plain C (a named pointer local used for real stores). src/text1a_c.c restored to HEAD (INCLUDE_ASM); no rules/pipeline/engine files touched; no permuter campaigns launched, none left alive.


## s8 (2026-08-26, escalation modality)

- [s8] Chassis re-measure with edits in place: the P4 body
  (rejected/chassis-p4-thenarm-p-tail-pure-rename.c) applied to
  src/text1a_c.c scores sandbox --disable all = 12, build_insns 108 ==
  target 108. Confirms s7's chassis on today's HEAD. Ledger floor 10
  (the score-10 candidate.c body) unchanged this session.
- [s8] PASS-ORDER GROUND TRUTH (read from tools/gcc-2.7.2/toplev.c, not
  guessed): 2983 flow_analysis -> 3004 combine_instructions -> 3033 sched ->
  3049 regclass + local_alloc -> 3077 global_alloc. `REG_BASIC_BLOCK` is
  written ONLY in flow.c (flow.c:2072-2075 and flow.c:2508-2511), i.e.
  strictly BEFORE combine.
- [s8] ORIGIN OF PSEUDO 78's HARD CONFLICT WITH $v0 (the input s7 named as
  blocking and inverse.py could not perturb): local_alloc gets there first.
  P4 .lreg shows `Register 101 used 2 times across 2 insns in block 13` and
  `Register 102 used 2 times across 4 insns in block 13`, and the
  local-alloc disposition lines `;; Register 101 in 2.` / `;; Register 102 in
  2.` -- both tail scratch pseudos (the a0+3 value and the 0x8000 constant)
  are BLOCK-LOCAL, so local_alloc allocates them before global_alloc runs and
  find_free_reg hands them $v0. Pseudo 78 (the tail base p) is then a global
  allocno that conflicts with hard reg 2 for that reason alone. P4 .greg:
  `;; 78 conflicts: 72 73 78 2 29` / `;; 78 preferences: 4 5` /
  dispositions `76 in 2  78 in 4`.
- [s8] THEREFORE target's tail assignment (base=$v0, scratch=$v1) requires the
  tail BASE to be a block-13-LOCAL quantity with a longer live range than the
  scratches, so that local_alloc allocates it first ($v0) and pushes both
  scratches to $v1. Target's 0x800459DC-0x800459FC is exactly that shape.
- [s8] BUT cse.c:826 make_regs_eqv can only keep the join copy alive if the
  base pseudo has a mention in an EARLIER BASIC BLOCK (the other disjunct,
  regno_last_uid > cse_basic_block_end, is unreachable because the tail block
  is the last block of the function), and flow.c:2074 turns any such mention
  into REG_BLOCK_GLOBAL before combine can delete it. Measured directly: on
  P4 the then-arm copy insns 121/124 are STILL PRESENT in the .flow dump and
  are gone only in .combine, yet .lreg reports pseudo 78 as
  `used 9 times across 9 insns` with NO `in block N` suffix.
- [s8] => STRUCTURAL CONTRADICTION, closed form: the tail base pseudo is
  cse-canonical (its copy survives) IFF it is multi-block, and it can win $v0
  IFF it is single-block. No C spelling satisfies both. This retires the
  three-session framing of R2 as an RA tie-break that "some spelling" might
  win.
- [s8] NEW MEASUREMENT (early mention made dead): P4 with the third-if
  then-arm store written `s1[3] = 0;` instead of `p[3] = 0;` gives sandbox
  --disable all = 11, build_insns 107. cse re-canonicalises the store onto p
  when p is set first, so the "dead store" is not actually dead in P4; when
  the store genuinely uses s1 the join copy dies and the build is one insn
  SHORT of target. Banked:
  rejected/thenarm-store-through-s1-closes-delayslot-kills-basecopy.c.
- [s8] NEW MEASUREMENT (R1' is anti-coupled to R2): in that same 107-insn
  build the `beq v1,v0` delay slot at idx 50 comes out as `nop`, EXACTLY as
  target has it -- residual R1' is CLOSED by writing the then-arm store
  through s1. On P4 (store through p) reorg.c fills the slot with
  `move a0,s3`. So the ONE C decision that closes R1' is the one that opens
  R2, and vice versa; no measured spelling closes both.
- [s8] NEW MEASUREMENT (last untried early-mention placement): early mention
  in the FIRST if's ELSE arm (`p = s1; p[4] = -1; p[3] = 0;`) = sandbox 14,
  build_insns 109 -- the block-2 copy is not propagated away and costs a whole
  extra instruction, the same failure mode as s7's third-if else-arm probe.
  Banked: rejected/firstelse-arm-p-early-mention-extra-copy.c. The third-if
  THEN arm (P4) remains the ONLY placement where the early mention is free.
- [s8] TOOL DEFECT (record before trusting a dump again): `pwsh
  tools/grinder/dump.ps1 func_80045878` emitted BYTE-IDENTICAL .greg/.lreg/.s
  output for two source variants that demonstrably compile to different
  objects (108 vs 107 insns under the sandbox, verified by disassembling
  tmp/sandbox/func_80045878/text1a_c.o). The dump taken immediately after the
  P4 measurement IS valid and is what every P4 dump citation above rests on,
  but the tool did not pick up the following edit. Cross-check any dump-based
  claim against `mipsel-linux-gnu-objdump -d tmp/sandbox/<func>/<stem>.o`
  (helper: tmp/grind/func_80045878/s8/dis.sh).
- [s8] scan_hand_coded.py --single func_80045878 re-run: tier=LOW score=0/8,
  108 insns, 7 spills, 11 distinct regs, S1-S8 all negative. AND-gate #1
  fails, unchanged since s5.
- [s8] DISPOSITION: both endgame-lock AND-gates fail and there is no closing
  construct of any family to seek a precedent for, so the owner's standing
  2026-07-27 auto-ruling was APPLIED and filed at docs/grind/decisions.md
  (2026-08-26 entry) as REFUSED / OWNER-ACCEPTED INCOMPLETE, with the
  closed-form proof above written into the entry. src/text1a_c.c restored to
  HEAD (INCLUDE_ASM) at end of session; no rules/pipeline/engine files
  touched; no permuter campaigns launched, none left alive.

- [s8] P4 chassis re-measured on today's HEAD with edits in place: sandbox --disable all = 12, build_insns 108 == target 108. Ledger floor 10 (the score-10 candidate.c body) unchanged this session.

- [s8] toplev.c pass order confirmed by source read: flow_analysis(2983) -> combine_instructions(3004) -> sched(3033) -> regclass+local_alloc(3049) -> global_alloc(3077).

- [s8] REG_BASIC_BLOCK is assigned only inside flow.c (flow.c:2072-2075 and flow.c:2508-2511), i.e. strictly before combine - so a copy that combine later deletes has already forced the pseudo to REG_BLOCK_GLOBAL.

- [s8] P4 .lreg: the two tail scratch pseudos are block-local ('Register 101 used 2 times across 2 insns in block 13', 'Register 102 ... in block 13') and local_alloc gives BOTH of them $v0 (';; Register 101 in 2.', ';; Register 102 in 2.'). That, and nothing else, is the source of pseudo 78's hard conflict with hard reg 2.

- [s8] P4 .greg: ';; 78 conflicts: 72 73 78 2 29', ';; 78 preferences: 4 5', dispositions '76 in 2  78 in 4' - the tail base takes $a0 off its own preference list after being barred from $v0.

- [s8] cse.c:826 make_regs_eqv's first disjunct (regno_last_uid[p] > cse_basic_block_end) is unreachable for this function because the tail block is the last basic block, so only an earlier-basic-block mention can keep the join copy alive.

- [s8] NEW: P4 with the third-if then-arm store written through s1 instead of p = sandbox 11, build_insns 107 - the join copy dies. Banked as memory/grind/func_80045878/rejected/thenarm-store-through-s1-closes-delayslot-kills-basecopy.c.

- [s8] NEW: in that same 107-insn build the idx-50 `beq v1,v0` delay slot comes out as `nop`, exactly as target has it - residual R1' is closed by that spelling, and only by it.

- [s8] NEW: early mention placed in the FIRST if's else arm ('p = s1; p[4] = -1; p[3] = 0;') = sandbox 14, build_insns 109 - the block-2 copy is not propagated away. Banked as memory/grind/func_80045878/rejected/firstelse-arm-p-early-mention-extra-copy.c. The third-if THEN arm remains the only free placement.

- [s8] AND-gate #1 re-run this session: tools/scan_hand_coded.py --single func_80045878 = tier=LOW score=0/8 (108 insns, 7 spills, 11 distinct regs), S1-S8 all negative. Canonical-asm is not supportable.

- [s8] AND-gate #2 fails vacuously: there is no closing construct of any family - sanctioned, unsanctioned or forbidden - to seek a SOTN precedent for, because the two properties target's tail requires are mutually exclusive under this compiler.

- [s8] TOOL DEFECT: tools/grinder/dump.ps1 produced byte-identical dumps for two source variants that compile to different objects; cross-check dump-based claims against mipsel-linux-gnu-objdump -d tmp/sandbox/<func>/<stem>.o (helper tmp/grind/func_80045878/s8/dis.sh).

- [s8] src/text1a_c.c restored to HEAD (INCLUDE_ASM) at end of session; no rules/pipeline/engine files touched; no permuter campaigns launched, none left alive.

- [s8] Disposition filed by this session at docs/grind/decisions.md, entry '## 2026-08-26 - func_80045878 (src/text1a_c.c) - OWNER-ESCALATION - RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE'.
