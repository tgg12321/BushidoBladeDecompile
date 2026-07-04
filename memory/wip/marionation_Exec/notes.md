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
- Natural-C trigger = a POST-FLOW-minted label at check2's target. jump2's
  cross_jump (runs after reload) mints labels — and duplicated-statement-into-
  arms (owner-sanctioned 2026-07-01) explicitly covers byte-neutral cross-jump
  re-merge. Blocker: the after_blocks tail has only ONE instance/predecessor,
  so there is nothing to merge. NEXT: read jump.c label lifecycle (cross_jump
  get_label_before; adjacent-label merge keep-direction; any other post-flow
  label create/delete) and hunt a shape that re-mints that label.

## Arm-2 transposition (knob-world residual, 2 masked lines)
Knob-world order [sb; la; move] vs target [sb; move; la] = sched2 following C
stmt order. Reordering C (dst2=a1 before src=…, d1/d2) fixes the order BUT
shortens p73's live range 86→84 luids → pri 2*4*1e4/84 = 952 = EXACT TIE with
p82(saved, 952) → ascending-pseudo tiebreak → p73 steals s1 (s-regs flip).
Counter = raise p82 (+2 refs → 3636) and, because p73 then sits at 952 > 933,
also raise p76/p78 above it — the full bump program below.

## Byte-free ref-bump program (ALLOCDBG-verified arithmetic, session-5)
Target order needs p82 > p76 >= p78 > p73 strictly (find_reg = lowest-free-s).
- p78 (idx_1496): split-init `idx_1496 = idx_1494; idx_1496 += 2;` → 9r/1800;
  combine merges back to one addiu; REFS SURVIVE COMBINE (measured). ✓
- p82 (saved): 3-stmt `{u8 t; t=*D_800A147C_2; saved=t; saved&=3;}` → 4r/3636
  and allocation ✓, BUT combine merges load+move → `lbu s1` (2-line byte cost;
  target has lbu v0 / andi s1,v0,3). UNTESTED byte-clean spellings:
  y2 `saved=3; saved&=t;` (cse const-prop risk), y5 `t&=0xF;` chain.
- p76 (idx_1494): needs +2 → 1800 (ties p78; ascending keeps p76 first).
  Adjacent-symbol split `idx_1494=&D_800A1496; idx_1494-=2;` FAILED: cse
  latches the intermediate symbol for idx_1495/19C0 derivations (+4 insns,
  u3_all=16). Only sets gain refs (fold-away reads just relocate the read).
  UNTESTED: cse-shielded 3-stmt `{u8*tp; tp=&D_800A1496; idx_1494=tp;
  idx_1494-=2;}` (combine 3-insn merge → la D_800A1494; refs counted at flow).
- PROOF OF CONCEPT: u3(guard) + sv + iq + ix = EXACT target s-alloc
  (p83 s0, p82 s1, p76 s2, p78 s3, p73 s4, p81 s5, p77 s6, p72 s7) — first
  steal-refusing shape ever to hold all 8; cost = ix prologue damage + guard
  position (masked 16). The u3-guard path is INFERIOR to the knob-trigger path
  (guard beqz sits early w/ sb in its slot vs target's late beqz a1).

## Region-2 SOLVED — recipe unchanged (in candidate.c)
Per-arm dst/dst2 split + arm-1 dst-EARLY (see git history for mechanics).

## Region-1 pair (2 masked lines) — UNCHANGED
{sll a0 <-> addu v0,v0,s5} @56-57: both LAUNCH, luid-pinned; flip breaks the
p106/val5 qty birth-tie (6). ~45 forms ledgered (git). Permuters own the
search: tmp/perm_mar6 (mh5) + tmp/perm_csmd4 (twin g3) — running, honest
splice metric; verify finds via tmp/perm_finds_verify.py. Twin crack transfers.

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
