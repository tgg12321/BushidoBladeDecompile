# marionation_Exec — HONEST SCORE 6; region-2 solved, regions 1+3 remain

## SESSION 9 STATE (READ FIRST)
candidate.c = the score-6 form (`bash tools/mar_test_candidate.sh` applies +
scores + restores; needs tmp/mar_candidate.c — committed mirror is this
candidate.c). 178/179 insns; ALL 8 callee-saved + all block regs target.
Remaining = TWO regions:
- **Region-1 (printf lbu5/sll4 order, 4 masked lines)** — see ledger below.
- **Region-3 (check2 slot: ours steals arm-2's `move a1,s4`, target NOP —
  also the missing insn)** — see paradox below.

## Region-2 SOLVED (this session) — the recipe
`dst` split into per-arm `dst`/`dst2` AND arm-1 dst-EARLY (`dst = a1;`
BEFORE `if (a1 != 0)` — line 100 of candidate.c). Mechanism:
- cse.c `make_regs_eqv:853`: copy dest becomes qty-canonical iff its
  `regno_last_uid` > the old reg's. Shared dst (lives to arm-2's loop) beat
  a1 → test rewritten to dst → refs 4→3 → RA rotation (= every old
  "dst-early costs 2" failure). Per-arm dst dies before a1's arm-2 use →
  a1 stays canonical → beqz s4 ✓. RTL: [sb, la, move-324, beqz-327].
- dbr fill_simple backward scan (reorg.c:2907): trial-1 = the move
  (conflict-free vs needed={s4}, eligible) → TAKEN, breaks after 1 slot →
  sb never reached → bytes [sb, la, beqz, move-slot] = target. Arm-1 order
  [store; dst; src] equivalent (same score).
- check-1 stays NOP: eager walk loses sb (may_trap + oppmem=1 always,
  mark_target_live_regs:2463), loses la (2-word macro, NEVER splits),
  stops at SEQ; steal_from_fallthrough:1723 refuses conditional SEQs.

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
**Region-1 status:** same catch-22 (noop between sll4/lw5 cse-folded).
Two-state trap stands. The +2-common-span-insn window (arg1 919 / pair
921 order-preserving) computed but no legal insn source exists.
**Permuter:** harness WORKS (tmp/perm_mar random, tmp/perm_mar2 directed;
pre-preprocessed sanitized base.c; honest pipeline compile.sh). 17280
directed lineswap orders: nothing below score 6. Random best uses
illegitimate inline_fn mutations — vet any output against the cheat
catalog before use.
**NEXT round:** (i) solve region-1 and region-3 TOGETHER — the razor
constants (933/930 margins) shift if the printf-block spelling changes
refs/L upstream; enumerate t0-chain spellings × arm-2 orders jointly
(the per-region local searches are provably exhausted); (ii) find the
byte-free label for (3a) — study how the TWIN's own candidate C (its 5
remaining rules are THIS block) is currently spelled in src/system.c and
what IT does about the label; (iii) decomp.me corpus scrape for this tail
idiom (beqz+NOP+sb+move) in gcc2.7.2-psx scratches.

## Region-1 ledger (printf lbu5/sll4) — the two-state trap
sched1 normalizes ALL statement orders to TWO streams (QTYDBG):
- **State A** (mul-before-arg5; CURRENT): sll4@14 → temp [14..24] L=10 <
  val5 [20..26] L=6 density → val5→v1, temp→a0, 11D5→v0 = TARGET RA ✓
  but order ✗ (4 masked lines).
- **State B** (any arg5-before-mul): sll4@18 → ORDER ✓ but temp [18..24]
  L=6 TIES val5 → qty_compare_1 tie → earlier birth = temp → steals v1 ✗.
Target = B-order + A-registers ⟹ val5 must win: temp L≥7 (sink addu4
past 24) or val5 born first (impossible: lw5 needs addu5@18; sll4>18
required for order). REFUTED sinks: seg3-inline into the call arg (16 —
combine merges); named t3 seg3 (G1/G2 re-tie → v1); named-shift t3 (G4 =
state B); pp position (normalized); copy-back (pre-RA-eliminated).
Key mechanism: addu4 (`t0 = base + t0`) is
NON-BIRTHING (t0 multi-set) → drifts early (24) in backward sched1; sw
(call-gen'd, luid after everything) lands 26; single-set namings re-tie
the chain. Remaining: a3-arg cost>2 precompute forms (calls.c:1618)
WITHOUT combine-merging; joint search with region-3 (see NEXT above).

## Target ground truth (asm/funcs/marionation_Exec.s)
- Regs: status s0, saved s1, i1494 s2, i1496 s3, arg1 s4, tbl s5,
  i1495 s6, arg0 s7; tail check a2, src a0, i v1, b v0, dst/dst2 a1.
- check-1 (71A24): lbu 0(s3), andi a2 0xFF, beqz→check2, NOP.
- arm-1: sb 0(s3), la F19B0, beqz s4→.L812BC, slot=move a1,s4; li v1,7;
  li a3,-1; loop; j .L812CC; move v0,a2.
- check2 (71A74): lbu -1(s3), andi, beqz→after_blocks(.L812C4), NOP.
- arm-2: sb -1(s3), move a1,s4, la F19A8, beqz a1→.L812BC, slot=li v1,7;
  li a3,-1; loop; .L812BC: j .L812CC; move v0,a2.
- after_blocks: beqz s7→loop, slot=move v0,zero. Epilogue .L812CC.
- The two [j; move v0,a2] sites stay unmerged (ours too ✓).

## Known gotchas
- 42 rules index-anchored: mid-derivation full builds meaningless; end
  gate = retire-all-42 + full SHA1. Sandbox masked (register-blind).
- Twin csmd4 (~:399) shares text — anchors must be marionation-unique.
- Declare new_var/new_var3 in the final form (undeclared-at-:555 pipe
  swallows cc1 error-recovery exit; bytes fine but fix before close-out).
- csmd4 payoff: its last 5 rules are THIS block; every mechanism transfers.

## Tools (this session's additions)
- tmp/gccdbg/cc1 now has BB2_DBR_DEBUG (reorg.c: fill_simple trials/elig,
  fill_slots_from_thread entry+trials+WINNER/LOSE, mark_target_live_regs
  block). Runner: tmp/mar_dbrdbg.sh (dumps kept in tmp/rtl/marD.*);
  uid→insn map: tmp/mar_dbr_tail.py [dumpfile].
- QTYDBG sweeps: tmp/mar_qty_sweep{,2,3,4}.py (statement-order × qty
  tables); tmp/mar_sweep_printf2.py (order × score).
- Prior kit: mar_test_candidate.sh, mar_qtydbg.sh, mar_cand_sched.sh,
  probe_mar.py, mar_diff2.sh (all restore src).
