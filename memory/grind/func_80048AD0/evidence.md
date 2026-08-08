
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


## s4 (forensics, 2026-08-07) — fold mechanism named verbatim: combine's label_tick-scoped nonzero_bits, NOT cse1 path reachability; carrier-copy family closed

Floor re-measured at 1 this session (candidate.c body applied to src/text1b.c,
sandbox --disable all = 1, 47/47; src restored to HEAD, tree clean).

- [s4] **The s3 frontier question is ANSWERED, and the s3 law re-attributed.**
  cse.c cse_end_of_basic_block (tools/gcc-2.7.2/cse.c:8007-8207) DOES follow the
  taken edge of a conditional branch: the follow_jumps arm (8102-8148) requires
  only (a) SET/IF_THEN_ELSE jump, (b) LABEL_NUSES(target)==1, (c) target label
  preceded by a BARRIER. In the target-layout (pre-jump2) stream BOTH branches
  qualify (check1 beqz -> label24 behind barrier23, check2 bne -> label52 behind
  barrier51), and the P8 .cse dump prints ";; Processing block from 2 to 94" —
  ONE cse path spans the lbu, the compare, and the call block containing the
  truncation. The truncation still survives cse1. The s3 "cse1 fall-through
  reachability" law was a correct empirical correlation with a WRONG mechanism.
- [s4] **cse1/cse2 cannot fold this truncation in ANY layout** (measured): the
  zero_extend:SI(subreg:QI(sound)) is intact post-cse1 and post-cse2 in both the
  target layout (P8) and the andi-free fall-through layout (P1, freshly dumped
  this session under -da). In P1 the truncation dies at COMBINE (.combine shows
  the truncation insn deleted and the arg copy folded to a plain a0=sound).
- [s4] **The true fold mechanism (verbatim combine.c):**
  expand_compound_operation (combine.c:4778-4886) rewrites
  zero_extend(subreg(sound)) into a shift pair -> simplify_and_const_int ->
  nonzero_bits(sound). nonzero_bits' REG case (combine.c:6882-6923) has three
  routes: (1) fast path 6887: requires reg_last_set_value valid AND
  (reg_n_sets==1 OR reg_last_set_label==label_tick). `sound` has THREE sets
  (lbu load, counter init `sound=0`, increment `sound++` — the counter-reuse
  construct), so the fast path demands the load and the truncation share a
  label_tick region, i.e. NO CODE_LABEL between them. Target layout interposes
  the call-block label -> fails. (2) get_last_value: same label test
  (combine.c:10034) -> fails. (3) global fallback reg_nonzero_bits (6920):
  recorded by set_nonzero_bits_and_sign_copies (combine.c:717-790) ONLY for
  reg_n_sets>1 regs, and it ORs nonzero_bits over ALL set sources — the
  increment source (sound+1, evaluated with nonzero_sign_valid=0 during the
  init scan) contributes the full SI mask -> reg_nonzero_bits[sound] =
  0xFFFFFFFF -> no fold. P1 folds precisely because check2 falls through into
  the call block with no label between def and truncation (label_tick disjunct
  passes, fast path returns 0xFF).
- [s4] **The construct coupling is exact and cruel:** the SAME reuse that wins
  the {$a0} allocation (sound==counter => multi-set) is what disables both
  label-independent nonzero_bits routes. A single-set `sound` (fresh counter)
  WOULD fold label-independently via the reg_n_sets==1 disjunct — but the fresh
  counter provably cannot receive {$a0} (s2 kill: no REG_DEAD merge site on
  (set i 0)/(set i i+1) for global.c expand_preferences).
- [s4] **NEW family measured dead — single-set SImode carrier copy**
  (`bgm = sound;` passed as the snd_LoadBgm argument). k1 (copy before the
  gate) and k2 (copy at call-block head) both compile BYTE-IDENTICAL to base
  (sole diff = the andi; the copy vanishes). The k1 -da dumps prove why: cse1
  records bgm==sound in one quantity class and canon_reg rewrites every later
  use (including the truncation operand) back to sound's pseudo (oldest reg in
  class); the copy goes dead and is deleted BEFORE combine runs (k1 .cse: copy
  insn 41 gone, truncation reads reg74). Structurally unavoidable: recording
  0xFF for the carrier requires the copy in the load's label region = label-free
  to the load = the SAME cse region, where canonicalization always reverts it.
  Post-gate placement fails twice over (also canon-reverted on the extended cse
  path; and across the label anyway). Banked as
  rejected/carrier-copy-cse-canon-reverted.c.
- [s4] **Complete partition of andi-elimination routes under target layout —
  all dead:** (i) cse1/cse2 fold: measured impossible in any layout; (ii)
  combine fold with the reused counter: label_tick-blocked, fallback poisoned
  (verbatim code + measurements); (iii) combine fold with single-set sound:
  fold works but the {$a0} chain is lost (~7 reg diffs; s1/s2 kills + the s3
  68k-iter dry campaign); (iv) carrier copies: cse1 canon-reverted (measured
  k1/k2 + dumps); (v) expand-time truncation avoidance (u8-typed argument
  expression): the banned/refused (A) family. Consequence for rederive: since
  target has `nop` in the delay slot AND registers exactly matching the reuse
  allocation, the ORIGINAL source must have avoided emitting the truncation at
  EXPAND — an honestly-u8-typed argument value — while getting the counter
  allocation from a shape outside this chassis. That is exactly the rederive
  frontier, now sharpened: search u8-sound shapes with a structurally different
  counter/callee-save arrangement (the m1 u8-basin scored 10, not 0, so the
  shape must differ more deeply than variable retyping).
- Artifacts: tmp/grind/func_80048AD0/s4/{mkdumps.sh, mkdumps_k1.sh,
  p1_outer_early_inner_ne.c + full -da dump set (.cse/.combine/etc.),
  k1_pregate_carrier.c + full -da dump set, k2_postgate_carrier.c,
  text1b_head_backup.c}.

- [s4] Floor re-measured at 1 this session: candidate.c body applied to src/text1b.c, sandbox --disable all = 1 (47/47, 347 cheat-asm lines stripped); src/text1b.c restored to HEAD, tree clean

- [s4] cse1's follow_jumps arm (cse.c:8102-8148) follows conditional-branch taken edges into barrier-preceded single-use labels: the P8 target-layout stream has BOTH branches followed ('Processing block from 2 to 94'), overturning the s3 assumption that the call block is off cse1's path

- [s4] cse1/cse2 never fold the truncation in any layout (measured P1+P8); the fold is combine's: expand_compound_operation -> simplify_and_const_int -> nonzero_bits(sound)

- [s4] nonzero_bits REG routes for multi-set sound (3 sets from counter reuse): fast path (6887) label_tick-blocked under target layout; get_last_value (10034) same; reg_nonzero_bits fallback (6920) poisoned to full mask by the increment source ORed in at init scan (717-790)

- [s4] The coupling is exact: counter reuse (needed for the {$a0} chain, s1-confirmed) is what makes sound multi-set; single-set sound folds label-independently but the fresh counter provably cannot receive {$a0} (s2 kill) — the ~7-reg-diff basin

- [s4] Carrier-copy family dead: k1/k2 byte-identical to base; cse1 canon_reg reverts every same-region SImode copy of sound before combine (k1 .cse dump); banked rejected/carrier-copy-cse-canon-reverted.c

- [s4] Complete route partition under target layout, all dead: cse fold (measured), combine+reuse (label-blocked+poisoned), combine+single-set (RA loss), carrier copies (canon-reverted), expand-time u8 avoidance (banned/refused family)

- [s4] Rederive sharpening: target's delay-slot nop + exact reuse-allocation registers mean the original source avoided emitting the truncation at EXPAND with an honestly-u8-typed argument value while getting the counter allocation from a shape OUTSIDE this chassis (the m1 u8-basin scored 10, so the difference is structural, not a retype)
