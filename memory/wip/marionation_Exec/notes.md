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

## Region-3 DECOMPOSED (session 9b) — two sub-problems, both walled
**(3a) the steal:** eager fill walks arm-2 [sb LOSE(trap+oppmem), la
LOSE(inelig), move WINNER — $a1 dead at after_blocks (20fc0000)] → steals.
BB2_NO_FT_STEAL what-if (env-gated knob in gccdbg reorg.c) confirms:
blocking it restores the NOP+insn-count but exposes (3b). Predicates
structural for ANY same-shape RTL; only stoppers = labels (mid-walk
own_thread←0 @3322, or head own_fallthrough=0) — no byte-free C label
construct found (jump2 cross-jump is the only post-flow label creator,
runs AFTER sched2 per toplev.c:3142, but arm-2's mid-block spot is not a
merge tail). get_branch_condition handles both branch senses; MIPS
define_delay has no likely-conditions; no mips.md peepholes.
**(3b) the transposition:** target arm-2 straightline = [sb, MOVE, la,
SEQ(beqz-a1, li)]; ours (steal blocked) = [sb, la, MOVE, SEQ]. sched2 is a
measured luid-no-op here (flat pri=1, ready sorted luid-desc — SCHEDDBG
block-23). All C orders placing the move ≤2 luids earlier ROTATE the RA:
arg1's global-alloc pri (~32000/L truncated) vs the i1494/96 pair
(~56000/150=373) has a ONE-POINT margin — L=86 ok (372), L=85 crosses
(376). Measured: T4/[store;dst2;src;i] → L=84 → arg1→s1; B → rotation;
v2/v3 noop-compensation (`chk2 = check`) FAILS because check/chkb cross
the test→arm-2 block boundary ⟹ NO local qty ⟹ no tie ⟹ real move
(chk2→a3/v0). In-arm-2 noop impossible (nothing born before the move).
⟹ target's L(arg1)/pair-L geometry MUST differ upstream: the RA-insn
count between prologue and arm-2 has slack vs bytes ONLY via macro-insn
respellings (la/mem-symbol macros = 1 RA insn / 2 bytes). NEXT: instrument
global.c allocno_priority (BB2_GLOB_DEBUG: pri/live_length/refs table),
compute the exact (pair-L, arg1-L) windows, then search head/poll/printf
respellings that shift RA-insn counts byte-identically.
Also: permuter now RUNS on marionation (tmp/perm_mar random,
tmp/perm_mar2 = PERM_LINESWAP 17280-order enumeration; workspaces fixed:
pre-preprocessed base.c + sanitized asm-renames; compile.sh = honest
pipeline). Directed enumeration found nothing below the score-6 plateau.

## Region-1 ledger (printf lbu5/sll4) — the two-state trap
sched1 normalizes ALL statement orders to exactly TWO streams (QTYDBG,
tmp/mar_qtydbg.sh; indices = block-local luids):
- **State A** (mul-before-arg5; CURRENT candidate): sll4@14 → temp
  [14..24] L=10 < val5 [20..26] L=6 density → val5→v1, temp→a0, 11D5→v0
  = TARGET RA ✓ but order [.., sll4, sll5, addu5, lw5] ✗ (4 masked lines).
- **State B** (any arg5-before-mul): sll4@18 → ORDER ✓ (pure register-
  substitution diffs) but temp [18..24] L=6 TIES val5 L=6 → qty_compare_1
  tie → smaller qty number = earlier birth = temp (18<20) → temp steals
  v1, val5→a0 ✗.
Target = state-B order + state-A registers ⟹ val5 must win the tie:
temp L≥7 (sink addu4 past 24) or val5 born first (impossible: lw5 depends
on addu5@18; sll4>18 required for order). REFUTED sinks: seg3-inline into
the call arg (score 16 — combine merges the chain, [20..32] 6-ref qty);
named t3 for seg3 (G1/G2: local-alloc ties it into a 6-ref [18..32] qty
→ v1 ✗); named t3 for the shift (G4: normalizes back to state B); pp
position (no effect — sched1 normalizes); copy-back `t0 = t3` (eliminated
pre-RA, same tables). Key mechanism: addu4 (`t0 = base + t0`) is
NON-BIRTHING (t0 multi-set) → drifts early (24) in backward sched1; sw
(call-gen'd, luid after everything) lands 26. To flip addu4 past sw its
dest must be single-set, but every naming that does so re-ties the chain.
NEXT: permuter directed sweep on the block (tools/permuter_annotate.py);
spellings that keep t0 2-set AND stretch temp span; a3-arg cost>2 forms
that trigger calls.c:1618 precompute WITHOUT combine-merging.

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
