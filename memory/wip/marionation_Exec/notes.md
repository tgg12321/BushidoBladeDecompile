# marionation_Exec — HONEST SCORE 6; regions 1+3 remain (foundation-level)

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
**(3b) the transposition — RA-pinned, measured to ±1.** Exact allocno
table (BB2_ALLOC_DEBUG, tmp/mar_allocdbg.sh): pair i1494/96 = refs 7,
L=150, pri 933/933; arg1 = refs 4, L=86, pri 930; saved = 952. arg1's
death = arm-2's move; EVERY order placing it earlier: L=85→941 (>933,
takes s2; measured s_i_d_src), L=84→952 (ties saved, allocno-order wins,
takes s1; measured T4/s_d_i_src). Dbr fill enumeration: NO RA-legal
pre-dbr order yields target's [sb, move, la, SEQ] (from [sb,la,li,move]
the li-fill leaves [sb, la, move]; every sb-before-branch order with the
move earlier gets sb-or-li stolen or rotates). Noop-at-RA compensation is
a catch-22: same-block copies get cse-propagated away (R1a/b/c measured:
qty tables identical to no-noop), cross-block copies get no local qty ⟹
no tie ⟹ real moves (v2/v3: chk2→a3/v0). sched2 = luid-no-op (flat
pri=1, luid-desc ties — SCHEDDBG block-23); no mips.md peepholes; jump2
moves nothing. ⟹ target's geometry must differ UPSTREAM (arg1-refs=3
via a construct keeping beqz-s4?? merged-test spelling gives beqz-dst ✗;
or pair-refs=8 via floor_log2 jump — no byte-free 8th ref found).
**(3b) EXHAUSTED (10):** tie-copy hole has NO arm-2 vehicle (i/src
constants — i2=i cse1-propagated, tmp/mar_i2_trace.py; dst2-chains past
arg1 death; check cross-block; others don't die). saved-split refuted
by bytes. Sandwich has no integer solution. L counts LIVE positions
(topology inert). Family sweep all ≥9.
**Permuter: REAL METRIC** (tmp/perm_mar3; recipe tools/mar_perm_
workspace.sh + mar_perm_compile.sh: full-TU compile + awk-extract to
offset-0 .o; reduced TU shifts codegen 55 lines — don't). Base = 300
(was 238800 noise). perm_inline=0. Refuted: Kengo (rewrite); -1-holder/
status&2-named; old 42 rules = same steal wall cheated through.
**CONCLUSION:** local search around this foundation = exhausted with
measured negatives; head-chain respellings byte-pinned (H1-H6 + O1-O3:
sched1 normalizes stores, la-reorders drop refs). The 6 points need a
foundation the derivation hasn't conceived — the REAL-metric permuter
campaigns (perm_mar3/4) are the active search. DONE leads: twin's C
(:388, ==2||==5 two-entry label), corpus (OR-conditions), m2c (no pp in
original; arm-2 dst-early in bytes), Kengo (rewrite, useless).

## Region-1 ledger (session 11b) — the tie, sharply
State B/220-form (arg5-early or the arg5v-split — SAME state): ORDER
fully correct; remaining = a pure v1↔a0 swap of [temp+t0-global]↔val5
(5 subst lines, masked-8). QTYDBG: temp [18..24]=6 TIES val5 [20..26]=6
→ qty-order (birth) → temp → v1 ✗. Target needs val5 first. addu4
ALREADY sinks to 24 in this state; only the BIRTH tie (sll4@18 < lw5@20)
remains, and sched1 normalizes every C order to sll4-first (arg5v/N1-N5
all identical tables). KEY NEW MECHANISM (SCHEDDBG runs 16/17): sched2
is NOT a no-op in the printf block (it swaps sw↔sll-11D5 between
passes!) ⟹ luid-order [lw5<sll4] + byte-order [sll4<lw5] is REACHABLE
in principle via post-RA anti-dep differences — no C spelling found yet
that gives sched1 the lw5-first luids. Pins ignored (RA fights them).
State A (mul-before-arg5) = RA ✓ order ✗ (the base-6). REFUTED sinks
(session 9): seg3-inline (combine merges), named t3 (re-ties), pp moves
(normalized), copy-back (pre-RA-eliminated).
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
**CLOCK-12 (11f):** RTL luids already favor the flip (lw5=l9 < sll4=
l11); the backward pick@12 takes the load anyway — explained by the
theorem below (hazard-greedy select), NOT a luid/class anomaly.
Failed: arg3v naming (AF1-3, 15-16 — pulls the 11D5 chain).
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
= continuous 6-ref qty stealing v1 (48*2500/22 ≫ 13333). In-family
levers exhausted; the campaigns own the residual space. CAUTION:
sched-log dumps interleave functions — cross-check uids vs marB.i.lreg.

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
