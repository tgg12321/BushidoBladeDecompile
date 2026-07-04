# marionation_Exec — HONEST SCORE 6; regions 1+3 remain (foundation-level)

## NEW DOORS from the twin's 2026-07-03 campaign (READ cpu_side_move_dir_4 notes FIRST)
The twin's session-3 mechanism map opens region-1 differently:
(a) CROSS-BLOCK UNIONS: w = printf-chain ∪ arm-walker(s) (a0-family),
    i = val5-carrier ∪ the arms' counters (v1; CARRY = `i = tbl[idx1]`
    makes lw5 non-birthing → no LAUNCH → tail re-times to target order),
    m1 = shared -1 holder across BOTH arms (a3; pri ~2×the twin's single).
(b) The a3-pref on w (set_preference MEM-unwrap at the lw-a3 homing) needs
    an a3-holder allocated BEFORE w — marionation's DOUBLE -1 gives m1
    pri ~10909 vs w∪1-walker ~11142 (within 2% — measure, don't derive).
(c) FIRST MEASUREMENT (tmp/mar_m1_test.py, masked 30): m1 landed a2 (no
    pref, ascending scan — a2 was FREE because the check-var went t0!) and
    the arm-2 structure drifted (-1 insn). NEXT: make the check-var hold
    a2 before m1 (its natural target reg) — check-var vs m1 priority is
    the knob; then w's pref-mask follows. The head shows the t2-class trap
    (lbu5/pp/w-sll rotation) — see the twin's staging theorem + traces.

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

## Region-1 ledger — the tie
State B/220 (arg5-early ≡ arg5v-split): ORDER correct; residue = the
v1↔a0 swap of [temp+t0-global]↔val5 (5 substs, masked-8). QTYDBG: temp
[18..24]=6 TIES val5 [20..26]=6 → birth order → temp ✗. addu4 already
sinks; only the birth tie remains; sched1 normalizes all C orders
(N1-N5 identical). sched2 NOT a no-op here (sw↔sll-11D5 swap measured)
so stream/bytes can split. Pins ignored. State A = RA ✓ order ✗ (base
6). Refuted sinks (s9): seg3-inline, named-t3, pp-moves, copy-back.
**VETTING (11c):** order-floor 220 (17280 enumerated). mar5 "180" =
FALSE POSITIVE (semantic goto-loop). Vet: semantics + uniform j-deltas +
full-diff. do-while-0 neutral.
**State-B RTL mapped (11d/e, marB.i.lreg via tmp/mar_b_uidmap.py):**
qty-114 IDENTIFIED = the ANON SHIFT TEMP: insn 127 `(set 114 (ashift
105 2))` + insn 132 `(set 105 (plus 114 tbl))` — 105 = the t0 user-var
(multi-set → global); 114 = [sll..addu] = the [18..24] tie-winner.
val5 = 104 (lw5 = insn 120; has REG_EQUIV (mem sp+16) — its stack home).
Post-RA both 105+114 → v1 (in-place look); val5 → a0; TARGET swapped.
The tie: 114-birth (insn 127's stream slot) vs 104-birth (insn 120's).
Failed: arg3v naming (AF1-3, 15-16 — pulls the 11D5 chain); clock-12
luids favored the flip but hazard-greedy select overrides (theorem).
**SCHEDULER THEOREM (11h — mechanism COMPLETE):** rank_for_schedule
(pri → dep-class → luid; BB2_RANK_DEBUG: clock-12 = class-3 tie, val=0)
THEN schedule_select (sched.c:2643): within a same-priority group it
queues blocked insns and picks the LARGEST potential_hazard first ⟹
loads beat ALU backward ⟹ [ALU-forward-before-load] IS FORCED for
LAUNCH-tied birthing pairs. Corollaries: state-B's [sll4, lw5] + temp-
first birth = a theorem; breaking it needs a PRIORITY-GROUP SPLIT
(strip one side's LAUNCH bump = dest multi-set/not-live); val5-reuse
(VR1-3: index-then-value) strips it but relocates the index-lbu reg
(10-11); `t0 <<= 2` (no anon temp — synth_mult only fires for `*= 4`)
= continuous 6-ref qty stealing v1 (48*2500/22 ≫ 13333).
**GROUP-SPLIT CLOSED (11i):** the index var MUST stay multi-set/global
to land a0 (any single-set index qty gets v0/v1 first in find_free
order — the relocation curse, measured in VR1-3/t4-split/t2 analyses)
⟹ t0's multi-set is pinned ⟹ the anon temp + both LAUNCH bumps are
pinned ⟹ NO priority-group split exists within the register
constraints ⟹ region-1's local family is CLOSED analytically. Open:
whole-foundation respellings (poll/head/w9 geometry) + campaign luck
outside the modeled family. CAUTION: sched-log dumps interleave
functions — cross-check uids vs marB.i.lreg.

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
