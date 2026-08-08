
## s3 (permuter, 2026-08-07) — CFG/fold coupling law measured; both fresh-seed basins dry

Floor unchanged at 1 (candidate.c body re-applied to src/text1b.c, sandbox
--disable all = 1, 47/47, at session start; src/text1b.c restored to HEAD at
session end — oracle stays green).

- [s3] **THE STRUCTURAL LAW (5 hand probes, artifacts in
  tmp/grind/func_80048AD0/s3/): the andi folds IFF the snd_LoadBgm call block
  is fall-through-reachable from the lbu/compare block on cse1's path; target's
  CFG places the call block at a conditional-branch target (bne a0,v0,0x44),
  which is NOT on any cse1 path.** Probe matrix:
  - chassis_inverted_gate.c / p1_outer_early_inner_ne.c (`if (sound != 0xFF)
    { call... return 1; } return 0;`): andi GONE, loop+allocation fully match,
    but layout is wrong — call is fall-through of a beq, the merged return-0
    block sits at the END (target: between check2 and the call), 45 vs 47 insns.
    Banked as rejected/andifree-fallthrough-wrong-cfg.c (honest form, measured
    dead on CFG, NOT a cheat).
  - p2_goto_body.c (`if (sound != 0xFF) goto body; return 0; body: ...`):
    target CFG exact, andi PRESENT (sole diff) — same class as base.
  - p6_goto_diamond.c (`if (sound == 0xFF) goto ret0; goto body; ret0: return
    0; body: ...`): jump1 inverts the branch-over-jump BEFORE cse1 runs →
    identical to base (andi present, CFG exact).
  - p8_deadstore_gotopair.c (goto diamond + `delta = 0;` dead-store blocker
    between beq and goto, meant to delay the inversion past cse1): the .jump
    dump (p8_deadstore_gotopair.c.jump, kept in s3/) PROVES jump1 already
    normalized the layout — branch inverted to (ne)->label52, return-0 inline
    as fall-through, body label carries NOTE_INSN_DELETED_LABEL, the
    zero_extend truncation (insn 61) sits in the branch-target block, the dead
    store rides at the target block head and dies later. cse1 therefore sees
    target layout and does not fold. Result: andi present, CFG exact.
  - p7_dupread_stmt_PROBEONLY.c (duplicate-read STATEMENT `sound =
    (&D_80099BCC)[idx];` at the call-block head — banned family, compiled as a
    MECHANISM PROBE ONLY, never proposable): andi PRESENT. This KILLS the
    "any same-bb producer lets combine fold (and P 0xff)" theory — the fold is
    cse-path-based, not LOG_LINK/combine-based, AND it shows the banned
    argument form worked only because the u8-typed argument expression avoided
    the truncation at EXPAND time (no truncation ever emitted), not by fold.
- [s3] Consequence: block layout is FIXED at jump1 (GCC 2.7.2 never reorders
  blocks afterwards); jump1 has exactly two stable normal forms for this
  diamond — (i) target layout with the call at a branch target (no cse fold
  observed on any measured path), (ii) call-fall-through layout with the fold
  but the return-0 block at the end (2-insn structural miss that no later pass
  repairs). Every source spelling measured lands in one of the two.
- [s3] Campaign 1 (tmp/perm_48AD0_p1, seed = the andi-free P1 chassis, base
  score 460, body-confined settings inherited from s2): ONE find in 64,450
  iterations / 28 min — output-430-1, a `new_var = 0xFF` constant-holder +
  do-while(0) wrap of the sound load (score 430, -30). Cheat-family flavored
  (named-local constant-holder / do-while0 outside its carve-out scope) AND
  nowhere near 0. Banked rejected/permuter-newvar-dowhile-430.c. No other
  novelty in ~18 quiet minutes → harvest --stop.
- [s3] Campaign 2 (tmp/perm_48AD0_m1, seed = the m1 idx-reuse chassis, u8
  sound, base score 10 = the two lh-dest register diffs): ZERO finds in
  68,142 iterations / 28 min → harvest --stop. The score-10 basin has NO
  downhill neighbor under body mutation — corroborates the s2 analytic kill
  of the fresh-counter {$a0}-pref lever.
- [s3] Remaining crack, explicitly UNMEASURED this session (forensics
  modality): cse.c's cse_end_of_basic_block takes follow_jumps and
  skip_blocks parameters — whether any condition lets the cse path extend
  ACROSS the 2-insn return-0 fall-through block INTO the conditional-branch
  target (the call block) is the precise question that decides if any pure-C
  spelling can fold the andi under target layout. If cse.c proves it never
  extends into a conditional-branch target (or only under conditions this
  function cannot meet), then every fold route under target layout is dead,
  and the only andi-free routes are expand-time (u8-typed argument
  expression) — all spellings of which are banned/refused — making the
  function escalation-shaped exactly as the s2 frontier anticipated.
- Artifacts: tmp/grind/func_80048AD0/s3/{chassis_inverted_gate.c,
  chassis_m1_idx_reuse.c, p1_outer_early_inner_ne.c, p2_goto_body.c,
  p6_goto_diamond.c, p7_dupread_stmt_PROBEONLY.c, p8_deadstore_gotopair.c,
  p8_deadstore_gotopair.c.jump (+7 sibling dumps),
  find_score430_newvar_dowhile.c, campaign_p1_log_tail.txt,
  campaign_m1_log_tail.txt} + workspaces tmp/perm_48AD0_p1/, tmp/perm_48AD0_m1/.

- [s3] Floor re-measured at 1 this session: candidate.c body applied to src/text1b.c, sandbox --disable all = 1 (47/47, cheat_asm_stripped replaced the 4 HEAD pins); src/text1b.c restored to HEAD at session end, oracle stays green

- [s3] STRUCTURAL LAW: the andi folds IFF the snd_LoadBgm call block is fall-through-reachable from the lbu/compare block on cse1's path; target's CFG places the call at a conditional-branch target (bne a0,v0,0x44) which is on no cse1 path — measured across 5 semantically-equal spellings

- [s3] jump1 (not jump2) fixes block layout: the P8 .jump dump shows the goto-diamond + dead-store blocker already normalized before cse1 (branch inverted, body label NOTE_INSN_DELETED_LABEL, truncation insn 61 in the branch-target block), so no pass-order trick can present cse1 a fall-through call that later becomes a branch target

- [s3] The inverted-gate/P1 andi-free form is honest and 45/47 (loop+allocation fully match target; residual is purely the return-0 block placed at the end instead of between check2 and the call) — banked as rejected/andifree-fallthrough-wrong-cfg.c since no GCC 2.7.2 pass reorders blocks after jump1

- [s3] P7 duplicate-read-statement probe (banned family, probe-only) leaves the andi in place — kills the combine/LOG_LINK fold theory and proves the banned argument form worked via expand-time truncation avoidance, not a fold

- [s3] Campaign telemetry: perm_48AD0_p1 base 460, 64,450 iters, one -30 cheat-flavored find, stopped; perm_48AD0_m1 base 10, 68,142 iters, zero finds, stopped — both harvested --stop in-session, no orphans
