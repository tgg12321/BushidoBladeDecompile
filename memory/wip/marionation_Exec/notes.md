# marionation_Exec — CANDIDATE MASKED 4 (mh5); region-3 mechanism DECODED (2026-07-04)

## TL;DR
Floor unchanged (masked 4 = region-1 pair 2 + region-3 2; candidate.c = mh5).
SESSION-5 DECODED REGION-3 END-TO-END: the missing nop requires reorg.c's
fill_slots_from_thread to refuse stealing `move a1,s4`, and the ONLY
byte-compatible refusal is mark_target_live_regs' conservative everything-live
answer for check2's branch target (find_basic_block()==-1). KNOB-PROVEN
byte-exact (BB2_ALLLIVE_LABEL): steal refused, nop appears, 179/179. Remaining:
(a) a NATURAL-C trigger for that conservatism, (b) the arm-2 move/la
transposition (coupled to a p73 livelen tie), (c) the region-1 pair (unchanged).

## Region-3 — the decision tree (all measured, session-5)
- Steal site: jump_insn(beqz a2) steals `(set a1 s4)` from its OWN fall-through
  thread [sb; la; move] — trial passes refset/setset/setneed/setsopp/trap.
- Prediction (mostly_true_jump) is IRRELEVANT: predicted-taken still retries
  the fall-through on failure (reorg.c:3748). Empirically: `do{}while(0);`
  before after_blocks (LOOP_BEG note → prediction 2) changes NOTHING.
- own_fallthrough=0 impossible (needs a label between beqz and sb — bytes).
- $a1 genuinely live at .L812C4 impossible (bytes: a1 is written before any
  read on both paths out of after_blocks).
- ⇒ ONLY door: mark_target_live_regs everything-live = target label unknown to
  flow-era basic_block_head (find_basic_block==-1). Simulated via the new env
  knob → arm-2 becomes [beq a2; NOP; sb; la; move; beq a1; li-slot] ✓ nop ✓.
- Natural-C trigger = a POST-FLOW-minted label at check2's target. Session-5b
  CLOSED the remaining derivation doors: block-0 fallback (linear walk kills
  a1 at calls/labels), mid-block-label pending-death trick (barrier pins the
  head), jump1/jump2 canonicalize-on-last constructions (the two-referencer
  obstruction: after_blocks has ONE jumper; every keeper-jump is either
  unreachable (deleted jump1) or real bytes), reorg-era label replacement
  (only check2 targets it; fills process in stream order). NEXT-SESSION
  PROTOCOL (the strongest remaining tool): EMPIRICAL TRIGGER DISCOVERY —
  generate ~50 synthetic mini-functions reproducing the steal shape
  [beqz; sb; move(dead-on-taken); la; beqz] x label/goto/loop/duplicated-tail
  topologies around the target block; compile each; grep .s for nop-in-slot;
  any natural NOP = the trigger, reverse-engineer it. Cheap, mechanical,
  falsifies-or-finds. (Harness sketch: tmp/mar_alllive_full.py's fixpoint
  runner + a topology generator.)

## Arm-2 transposition (knob-world residual, 2 masked lines)
Knob-world order [sb; la; move] vs target [sb; move; la] = sched2 following C
stmt order. Reordering C (dst2=a1 before src=…, d1/d2) fixes the order BUT
shortens p73's live range 86→84 luids → pri 2*4*1e4/84 = 952 = EXACT TIE with
p82(saved, 952) → ascending-pseudo tiebreak → p73 steals s1 (s-regs flip).
Counter = raise p82 (+2 refs → 3636) and, because p73 then sits at 952 > 933,
also raise p76/p78 above it — the full bump program below.

## Byte-free ref-bump program — CLOSED as a byte-exact route (session-5/5b)
Target order needs p82 > p76 >= p78 > p73 strictly (find_reg = lowest-free-s).
WORKING (with byte costs): p78 iq-split `idx_1496=idx_1494; idx_1496+=2;`
(9r/1800, refs SURVIVE combine, byte-clean); p82 y1 3-stmt split (4r/3636,
alloc right, 2-line cost: combine merges load+move -> lbu s1); p76 ix
adjacent-symbol split (+4 insns: cse latches the symbol). BATTERY-FAILED:
tp-shield (185 insns, breaks iq), y2 `saved=3;saved&=t` + y5 `t&=0xF` (cse
const-prop collapses both; score 31). Only sets gain refs; const-sourced
splits die in cse; reg-sourced survive (iq precedent).
PROOF OF CONCEPT: u3+y1+iq+ix = EXACT target s-alloc (all 8) — the 3-point
wall is arithmetic, but no byte-clean instantiation exists; u3-guard path
also loses on guard position (beqz early w/ sb slot vs target late beqz a1).

## Region-2 SOLVED — recipe unchanged (in candidate.c)
Per-arm dst/dst2 split + arm-1 dst-EARLY (see git history for mechanics).

## Region-1 pair — session-5b corrections (READ BEFORE acting on 5a's redirect)
- CAVEAT on the "in-place = byte-proven" claim: RA colors a fresh single-set
  temp onto its dying operand's register, producing IDENTICAL bytes to an
  in-place chain — the g3-family inference is plausible, NOT proven.
- MEASURED (twin): all in-place chain forms (addu-in-place W1, +shl W2,
  target-order W3, g3-hybrid W4) score 15-16 vs ctl 2 — with no launches the
  whole head re-times; h5/mh5 is a deep local optimum. Do NOT re-derive.
- Permuter find output-40-1 (temp=arg5 staging) honest-verified 6 — rejected,
  but it confirms the search is in the arg5-qty neighborhood.

## Region-1 pair (2 masked lines) — FRONTIER REDIRECTED to g3 (session-5)
{sll a0 <-> addu v0,v0,s5} @56-57: both LAUNCH (sched1 dump: sll dest = fresh
single-set p109 split from the t0 chain; addu dest = fresh p111), luid-pinned.
NEW: t0<<=2 at the same position (in-place, kills p109's launch) scores 9
(twin 7) — the *=4 temp's launch is load-bearing for the h5 head timing; the
pair cannot be flipped inside the h5 equilibrium (re-confirms the ledger).
DECISIVE BYTE FACT: target's t0 chain is fully in-place in $a0
(lbu a0; sll a0,a0; addu a0,a0,s5) => the ORIGINAL's sll never launched =>
the original source was the g3 FAMILY (in-place chain, pair naturally right).
=> The search belongs on g3's residual (the v1/a0 qty exchange: t0-qty p113
6r/24 pri 5000 allocates before arg5-qty p100 2r/6 pri 3333), NOT on h5's
pair. arg5-qty needs pri >= 5000 (refs 3/span6 ties at 5000; birth order
favors arg5) — the twin ledger's vehicle classes all died; untried: multi-set
arg5 VALUE-staging shapes that keep the lw v1,0(v0) dest split (arg5 var
separate from address var). Permuters own the blind search meanwhile:
tmp/perm_mar6 (mh5) + tmp/perm_csmd4 (twin g3) — verify finds via
tmp/perm_finds_verify.py. Twin crack transfers.

## Target ground truth (asm/funcs/marionation_Exec.s)
- Regs: status s0, saved s1, i1494 s2, i1496 s3, arg1 s4, tbl s5, i1495 s6,
  arg0 s7; check a2, src a0, i v1, b v0, dst/dst2 a1.
- arm-2: sb -1(s3); move a1,s4; la F19A8; beqz a1→.L812BC slot=li v1,7;
  li a3,-1; loop; .L812BC: j .L812CC; move v0,a2. check2 beqz slot = NOP.
- after_blocks .L812C4: beqz s7→.L810A4 slot=move v0,0; falls into epilogue.
- Base ALLOCDBG s-order: p82 952 / p76 933 / p78 933 / p73 930 / p81 657 /
  p77 405 / p72 256 (u3 guard lifts p73 to 5r/1176 — the old (3a) wall).

## Known gotchas
- 42 rules index-anchored; end gate = retire-all-42 + full SHA1. Twin csmd4
  (:388) shares text — marionation-unique anchors. Declare new_var/new_var3.
- d1/d2 stmt reorders flip s-regs via the 952 tie (do NOT re-derive).
- knob uids shift with any C change — tmp/mar_alllive_full.py re-discovers
  them by fixpoint (watch for false stealer matches in prologue).

## Tools (all local/gitignored; regenerate from here if lost)
- tmp/gccdbg/cc1: BB2_DBR_DEBUG, BB2_NO_FT_STEAL, BB2_ALLOC/QTY/SCHED/SLL/FLOW.
- tmp/gccdbg/cc1_alllive (NEW): + BB2_ALLLIVE_LABEL=<uid,uid,...> — forces
  mark_target_live_regs everything-live for those target uids (simulates
  find_basic_block==-1). Source: ~/gcc272src/reorg.c (= tools/gcc-2.7.2/
  reorg.c, kept in sync); rebuild: make cc1 in ~/gccdbg-build.
- Session-5 probes: tmp/mar_alllive_test.sh, tmp/mar_alllive_full.py,
  tmp/mar_r3_refs_sweep.py (ref-bump ALLOCDBG sweep), tmp/mar_dw_test.py,
  tmp/mar_u3_alloc3.py. Kit: tools/mar_test_candidate.sh, tmp/mar_diff2.sh,
  tmp/mar_floor_check.sh.
