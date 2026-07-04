# marionation_Exec — HONEST SCORE 6; the masked-4 v0-idx form REJECTED by review (policy Q pending)

## SESSION-4 (2026-07-03): the v0-idx discovery + the review outcome
A form reaching masked 4 (vs the 6 floor) was found and twice FAILed
layer-1 review: rejected/v0-idx-staging-scheduler-pass.c — the index
staged through the dead-here v0 var strips the load LAUNCH priority
(sched.c adjust_priority/birthing_insn_p, reg_n_sets>=2) and re-times
the head to target (51-54+58-67 byte-match; t0=a0 via the p109 a0-pref
chain, FINDREGDBG-verified). Reviewer: live store = neither dead-store
nor named-local carve-out fits; the scheduler-pass mechanism is outside
the SOTN-evidenced LICM/RA variable-reuse scope -> needs fresh SOTN
evidence or a USER POLICY RULING (surfaced). If sanctioned, the form is
ready: residual would be [55-57 sll4-rotation, luid-pinned] + region-3.
Candidate REVERTED to the honest masked-6 (verified). The bare FAKEs
(idx_1495/D_800F19C0/pp) now carry full mechanism annotations per the
review. Lever-exhaustion ledger for the block (all sandbox-measured):
plain 6; REU 10; carriers i/status/cnt/temp 9-30; unions M1/M3 9-30;
18+ order sweeps 4-37; split-addu 8+drift; m2c-inline 14. Permuter
(tmp/perm_mar6, honest splice-metric): 200x2/220x3/225 finds = the
X-hoist family, sandbox-verified WORSE (weighted-vs-masked divergence);
grinding on.

## (session-3 door-map folded: see cpu_side_move_dir_4 notes + git history)

## STATE (READ FIRST)
candidate.c = the score-6 form (`bash tools/mar_test_candidate.sh` applies
+ scores + restores; copy to tmp/mar_candidate.c first). 178/179 insns;
ALL 8 callee-saved + block regs target. Remaining:
- **Region-1 (printf lbu5/sll4 order, 4 masked lines)** — ledger below.
- **Region-3 (check2 slot steal + arm-2 move/la transposition, 2 lines +
  missing nop)** — decomposed + locally EXHAUSTED below.

## Region-2 SOLVED — recipe (in candidate.c)
Per-arm dst/dst2 split + arm-1 dst-EARLY. make_regs_eqv:853 (last-uid
rule) keeps a1 canonical → beqz s4; dbr backward scan takes the move,
sb protected. check-1 NOP: sb trap+oppmem, la never splits, SEQ stops,
1723 refuses conditional-SEQ steals.

## Region-3 (9b/9c)
**(3a) steal:** twin-proven — arm-head label (two-entry ==2||==5) ⟹
own_fallthrough=0 ⟹ fill_eager skips (3764). Marionation's single-beqz
admits no visible second entry; L1/L2/L4/L6 all normalize away (6).
**(3b) the transposition — RA-pinned, measured to ±1, EXHAUSTED.** Allocno
table: pair i1494/96 refs7/L150/pri 933; arg1 refs4/L86/930; saved 952.
Every earlier-arg1 order rotates (941→s2 / 952-tie→s1). No RA-legal
pre-dbr order yields [sb, move, la, SEQ]. Noop-compensation catch-22
(same-block copies cse-fold; cross-block get no qty). sched2 = luid-noop;
no peepholes. Tie-copy vehicles: none (i2 propagated, dst2 past death,
check cross-block). Sandwich: no integer solution. Family sweep ≥9.
NOTE: the M1-union transform CHANGES the whole allocno table — (3b)'s
exhaustion is w.r.t. the OLD foundation; re-measure after region-1 lands.
**Permuter: REAL METRIC** (tmp/perm_mar3; recipe tools/mar_perm_
workspace.sh + mar_perm_compile.sh: full-TU compile + awk-extract to
offset-0 .o; reduced TU shifts codegen 55 lines — don't). Base = 300
(was 238800 noise). perm_inline=0. Refuted: Kengo (rewrite); -1-holder/
status&2-named; old 42 rules = same steal wall cheated through.
**CONCLUSION:** local search around this foundation = exhausted with
measured negatives; head-chain respellings byte-pinned (H1-H6 + O1-O3:
sched1 normalizes stores, la-reorders drop refs). The 6 points need a
foundation the derivation hasn't conceived — the REAL-metric permuter
campaigns are the search. DONE leads: twin's C, corpus, m2c, Kengo (✗).

## Region-1 ledger (LARGELY SUPERSEDED by session-4's v0-idx fix; full text in git)
The old tie ([temp+t0]↔val5 birth-order) and the scheduler theorem
(rank_for_schedule → schedule_select hazard-greedy; LAUNCH-tied pairs
force ALU-before-load) remain the mechanism reference — see git history
and the twin's notes. The v0-idx staging resolved the head; only the
55-57 sll4-rotation survives (luid-pinned, see session-4 block).

## Target ground truth (asm/funcs/marionation_Exec.s)
- Regs: status s0, saved s1, i1494 s2, i1496 s3, arg1 s4, tbl s5,
  i1495 s6, arg0 s7; tail check a2, src a0, i v1, b v0, dst/dst2 a1.
- check-1: lbu 0(s3), andi a2, beqz→check2, NOP. arm-1: sb 0(s3), la
  F19B0, beqz s4→.L812BC, slot=move; li v1,7; li a3,-1; loop; j; move.
- check2: lbu -1(s3), andi, beqz→after_blocks, NOP. arm-2: sb -1(s3),
  MOVE, la F19A8, beqz a1→.L812BC, slot=li v1,7; li a3,-1; loop;
  .L812BC: j .L812CC; move v0,a2. after_blocks: beqz s7→loop, slot=
  move v0,zero. Two [j; move] sites unmerged (ours ✓).

## Known gotchas
- 42 rules index-anchored; end gate = retire-all-42 + full SHA1. Twin
  csmd4 (:388) shares text — marionation-unique anchors. Declare
  new_var/new_var3 in the final form (undeclared-at-:555; bytes fine).
  csmd4's last 5 rules are THIS printf block; mechanisms transfer.

## Tools
- tmp/gccdbg/cc1: BB2_DBR_DEBUG, BB2_NO_FT_STEAL, BB2_ALLOC_DEBUG,
  QTY/SCHED/SLL/FLOW. Runners: tmp/mar_{dbrdbg,allocdbg,qtydbg,
  cand_sched}.sh; mar_dbr_tail.py; mar_i2_trace.py.
- Sweeps: tmp/mar_qty_sweep{,2,3,4}.py, mar_family_sweep.py,
  mar_{cross_sweep,nv2_sweep,head_order,perm_leads}.py.
- Kit: mar_test_candidate.sh + mar_diff2.sh (restore src).
