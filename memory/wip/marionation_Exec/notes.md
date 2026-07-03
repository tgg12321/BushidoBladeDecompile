# marionation_Exec — HONEST SCORE 6; regions 1+3 remain (foundation-level)

## STATE (READ FIRST)
candidate.c = the score-6 form (`bash tools/mar_test_candidate.sh` applies
+ scores + restores; copy to tmp/mar_candidate.c first). 178/179 insns;
ALL 8 callee-saved + block regs target. Remaining:
- **Region-1 (printf lbu5/sll4 order, 4 masked lines)** — ledger below.
- **Region-3 (check2 slot steal + arm-2 move/la transposition, 2 lines +
  missing nop)** — decomposed + locally EXHAUSTED below.

## Region-2 SOLVED — recipe (in candidate.c)
Per-arm `dst`/`dst2` split + arm-1 dst-EARLY (`dst = a1;` before the
guard, candidate line ~100). cse.c make_regs_eqv:853 promotes a copy
dest to qty-canonical iff regno_last_uid[new] > old's — per-arm dst dies
before a1's arm-2 use → a1 stays canonical → beqz s4 ✓. dbr backward
scan takes the move (trial-1), sb protected → [sb, la, beqz, move-slot]
= target. check-1 NOP: sb loses (trap+oppmem=1 always), la inelig
(never splits), SEQ stops; 1723 refuses conditional-SEQ steals.

## Region-3 DECOMPOSED (sessions 9b/9c) — the ledger
**(3a) the steal — MECHANISM FOUND VIA THE TWIN.** cpu_side_move_dir_4's
target tail (71A9C..): its check-branch (bne a2,v0) slot = NOP with
[li-2, sb, MOVE, la, SEQ(beqz-a1, li-v1)] straightline — because its arm
head carries .L80080FBC, targeted by the FIRST of two chained tests
(`chk==2 || chk==5`) ⟹ own_thread_p sees the CODE_LABEL ⟹
own_fallthrough=0 ⟹ fill_eager SKIPS the fall-through fill (reorg.c:3764
gates on own_fallthrough) ⟹ NOP. Marionation's check2 needs the same but
its bytes show NO label at arm-2's head — a label alive at pass-1-eager
and dead by final needs a user deleted post-eager byte-free; L1/L2/L4/L6
label structures (else-goto, redundant second test, switch, bnez-form)
ALL normalize away pre-dbr (measured, all score 6).
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
**(3b) EXHAUSTION COMPLETED (session 10):** the tie-copy hole (src born
same-block + dies at copy + dest cross-block) has NO vehicle in arm-2:
i/src are CONSTANTS (i2=i: cse1 const-propagates — tmp/mar_i2_trace.py,
copy in .jump, gone in .cse; L=85 → s2 rotation); dst2-chains sit past
arg1's death; check/chkb cross-block-born; arg0/tbl/i1496 don't die at
the copy. saved-split refuted by bytes (lbu→v0 = unsplit). Sandwich
(saved 952 > pair > arg1) has no integer solution (floor_log2 jumps
overshoot). L counts LIVE positions only (arm-1 loop + done-island are
arg1-dead — topology games inert). Family sweep (twin-form/u8/array-ptr/
named-arg4/direct) all ≥9.
**Permuter (session 11): REAL METRIC ACHIEVED.** tmp/perm_mar3 (recipe:
tools/mar_perm_workspace.sh + tools/mar_perm_compile.sh): full-TU compile
(context-faithful — a reduced TU shifts marionation by 55 lines!!) then
awk-extract marionation from the maspsx'd asm (directives at col 0!) →
offset-0 .o vs target.o from asm/funcs+prelude. Base score = 300 (was
238800 of address noise — old runs' rankings were GARBAGE: their best
scored 55 through the sandbox). perm_inline=0 via settings weight_overrides.
Runs: perm_mar3 random + perm_mar4 (lineswap 17280 orders) vs real metric.
Also refuted: Kengo marionation_Exec = PS2 rewrite (useless); the -1
constant-holder + status&2-named leads (K1/K2 rotate, K3 neutral); the
42 old rules confirm the same steal (subst addu->nop @127 + insert @130
= the previous workers cheated through the identical wall).
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
- tmp/gccdbg/cc1: BB2_DBR_DEBUG (fill decisions), BB2_NO_FT_STEAL
  (what-if), BB2_ALLOC_DEBUG (allocno table), QTY/SCHED/SLL/FLOW.
  Runners: tmp/mar_{dbrdbg,allocdbg,qtydbg,cand_sched}.sh;
  tmp/mar_dbr_tail.py (uid map); tmp/mar_i2_trace.py (copy tracer).
- Sweeps: tmp/mar_qty_sweep{,2,3,4}.py, mar_family_sweep.py,
  mar_cross_sweep.py (csv), mar_sweep_{printf2,tail,arm2,label}.py.
- Prior kit: mar_test_candidate.sh, mar_diff2.sh,
  probe_mar.py (all restore src).
