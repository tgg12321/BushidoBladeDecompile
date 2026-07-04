# marionation_Exec — CANDIDATE MASKED 4 (mh5); region-3 refusal mechanism FOUND in natural C (2026-07-04 s6)

## TL;DR
Floor unchanged (masked 4 = region-1 pair 2 + region-3 2; candidate.c = mh5).
SESSION-6: permuter finds all honest-verified and REJECTED (mar6 200-1/2 = 8,
200-3 = 5, csmd4 40-2 = 6; mh5 stands — deep local optimum confirmed).
Region-3 BREAK: a goto-referenced label in check2's fall-through (between sb
and the move) REFUSES the steal → natural NOP, CONFIRMED in minis
(tmp/mar_trigger_gen3.py h1). 5b's own_fallthrough closure ("needs a label —
bytes") was WRONG: labels are zero-width; USER labels delete to
NOTE_INSN_DELETED_LABEL (jump.c delete_insn). The one unsolved slot: the
label's REFERENCER must be byte-free — its lifecycle is now precisely
characterized by the two-pass timing calculus below.

## Region-3 — the sharpened decision tree (session-6, source-verified)
- dbr_schedule runs {fill_simple ×2; fill_eager; relax} TWICE (MAX_REORG_
  PASSES=2, "try everything twice"). ⇒ ANY scaffolding that refuses the pass-1
  steal but dies in pass-1 relax gets UNDONE by the pass-2 fill retry.
  Scaffolding must SURVIVE pass-2 fill_eager and die in/after pass-2 relax.
- Pass-2-relax deleters (the only byte-free removal window): branch-to-next
  (reorg.c ~4032, needs adjacency — but jump1 kills adjacent-jump-to-next
  pre-flow; adjacency must ARISE inside reorg via insns stolen into slots);
  jump-threading redirects orphaning labels (→NOTE). TODO next session: read
  relax 4030-4120 (filled-SEQUENCE handling) for more deleters.
- everything-live (find_basic_block==-1) ⇔ check2's target label minted
  POST-FLOW. Verified in flow.c: EVERY flow-era CODE_LABEL is its own
  basic_block_head entry (consecutive labels each get a block) — no deletion/
  retarget trick on flow-known labels can give -1. Post-flow minters:
  (a) jump2 do_cross_jump — mints MID-BLOCK (fall-through-reachable) ⇒
      find_basic_block walks up past reachable insns to a far barrier ⇒
      returns b_far, then does a control-flow-blind SET-marks-live scan
      [L_far..target) — NOT -1; a1 live only if SET in that stretch w/o
      label-committed death.
  (b) jump2 USE-hoist (jump.c 679-697) — mints BEFORE the old label (wrong
      side; jump's still real bytes).
  (c) relax redundant-first-insn chain (reorg.c 3992-4006, mint at 4002,
      `next=insn; continue` re-walks) — CAN yield a barrier-adjacent mint:
      re-computed `check = *(idx_1496-1) & new_var3;` at after_blocks =
      lbu+andi both redundant along check2's path → chain-mints down to the
      beqz-s7 insn = exactly .L812C4. Orphaned lbu/andi + dead after_blocks
      label are cleaned BYTE-FREE by delete_insn's label-cascade (jump.c
      3509-3532: deleting a barrier-preceded label deletes the unreachable
      code after it; user labels become NOTEs). BLOCKED ALONE: pass-1
      fill_eager runs BEFORE pass-1 relax → steals first (h2 measured).
- CONFIRMED refusal (c-mechanism): mid-thread referenced label/jump in the
  fall-through blocks the walk/own_thread → slot stays NOP (h1 mini, natural
  C, `if (G) goto midlab;` + `midlab:` at the sb). Referencer = bytes → must
  die per the timing calculus.
- MEASURED DEAD: `void *q = &&midlab;` (dead take) — eliminated at TREE level,
  label absent by .dbr, steal proceeds (k1/k2/k4). LABEL_PRESERVE_P route
  needs a SURVIVING take = bytes/data. Round-1/2 topology batteries (15+12
  variants: loops/duptails/switch/etc. at after_blocks): steal in ALL
  call-context variants; loop-top labels die pre-flow (back-edge folded).
- COMPOSITE BLUEPRINT (next session's target): pass-1 refusal via mid-thread
  label (referencer J) + pass-1 relax redundancy-chain (c) redirects check2's
  beqz onto the barrier-adjacent MINT (+cascade cleanup) → pass-2 fill sees
  find_basic_block==-1 → refused → NOP; J may die as early as pass-1 relax
  since pass-2 is covered by the mint. UNSOLVED: J's byte-free death (its
  redirect keeps bytes; deletion paths need adjacency or unreachability).
  NEXT PROBES: (i) BB2_DBR_DEBUG per-pass fill trace on the real candidate —
  which fills naturally land in pass 2 (parameterizes the calculus);
  (ii) compose h1+h2 in minis with J-lifecycle variants; (iii) finish the
  relax deleter catalog; (iv) check whether jump1's jump-around-jump
  inversion can be DEFERRED past flow by any construction (would let
  [bnez→L1; j after; L1: sb…] reach reorg; relax-era inversion deletes the j).

## Arm-2 transposition (knob-world residual, 2 masked lines)
Unchanged from s5 — see git history (d1/d2 tie at 952, ref-bump program
CLOSED as byte-exact route; u3+y1+iq+ix = exact s-alloc but no byte-clean
instantiation).

## Region-2 SOLVED — recipe unchanged (in candidate.c)

## Region-1 pair — permuter verdicts (session-6)
- perm_mar6 output-200-1/2 (arg5 value-staging via status): honest masked 8 —
  staging breaks the register story (lbu v1/lw s0 vs lbu a0/lw v1). REJECTED.
- output-200-3 (goto-loop moved into the arm): masked 5, registers in region-1
  all correct but the pair becomes a 3-insn rotation {sll a0 | sll v0, addu}
  and region-3 unchanged. REJECTED (worse than mh5's 4).
- csmd4 output-40-2: masked 6 vs floor 2. REJECTED.
- Conclusion: the g3-family residual (v1/a0 qty exchange) remains the
  region-1 frontier per 5b; the blind search keeps confirming mh5's basin.

## Target ground truth (asm/funcs/marionation_Exec.s)
- Regs: status s0, saved s1, i1494 s2, i1496 s3, arg1 s4, tbl s5, i1495 s6,
  arg0 s7; check a2, src a0, i v1, b v0, dst/dst2 a1.
- arm-2: sb -1(s3); move a1,s4; la F19A8; beqz a1→.L812BC slot=li v1,7;
  li a3,-1; loop; .L812BC: j .L812CC; move v0,a2. check2 beqz slot = NOP.
- .L812C4: beqz s7→.L810A4 slot=move v0,0; falls into epilogue (.L812CC).
- Tail has TWO identical [j .L812CC; slot move v0,a2] (arm-1 end + .L812BC):
  not cross-jumped because jump-vs-jump find_cross_jump needs minimum=2
  matching insns and only 1 matches. Arm-1's beqz s4 DID steal its move.
- Base ALLOCDBG s-order: p82 952 / p76 933 / p78 933 / p73 930 / p81 657 /
  p77 405 / p72 256.

## Known gotchas
- 42 rules index-anchored; end gate = retire-all-42 + full SHA1. Twin csmd4
  (:388) shares text — marionation-unique anchors. Declare new_var/new_var3.
- d1/d2 stmt reorders flip s-regs via the 952 tie (do NOT re-derive).
- knob uids shift with any C change — tmp/mar_alllive_full.py re-discovers
  them by fixpoint.

## Tools (all local/gitignored; regenerate from here if lost)
- tmp/gccdbg/cc1 + cc1_alllive: BB2_DBR_DEBUG, BB2_ALLLIVE_LABEL, ALLOC/QTY/
  SCHED dumps. Source: tools/gcc-2.7.2 (read-only reference).
- Session-6 empirical harness: tmp/mar_trigger_gen.py (topology battery 1),
  _gen2.py (loop-note battery), _gen3.py (h1 mid-thread label CONFIRMED NOP;
  h2 recheck steals), _gen4.py (&&label DEAD), tmp/mar_trace_labels.sh
  (per-pass label lifecycle for a variant). Minis in tmp/mar_trig/.
- Verify kit: tmp/perm_finds_verify.py (splice+sandbox permuter finds),
  tools/mar_test_candidate.sh, tmp/mar_floor_check.sh.
