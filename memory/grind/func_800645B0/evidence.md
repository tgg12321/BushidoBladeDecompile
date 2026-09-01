## Session s16 (2026-09-01, escalation/disposition) -- the s15 register-seat frontier taken to a TYPED verdict with the owner's Ruling-C instrument: the seat is FORECLOSED, and borrowing idx is what costs the two insns

(Driver numbering: this is grind session s16, scratch `tmp/grind/func_800645B0/s16/`.
The block below self-labels "Session 16" but is the PREVIOUS session -- the ledger's
own numbering drifted; trust the scratch-directory names.)

- **Chassis re-measured FIRST.**  SB body (`candidate.c`) pasted over the
  `INCLUDE_ASM` line: `sandbox func_800645B0 --disable all` = **score 1,
  target_insns 78, build_insns 78, rules_dropped 0**.  The floor of 1 is current
  on this tree; the h form re-measured **2 / 78** unchanged.

- **H69 (CONFIRMED) -- the h residual is RA-stage and its seat is FORECLOSED.**
  The owner's 2026-09-01 Ruling-C `--target-object` escape is implemented and
  works on this INCLUDE_ASM function.  `inverse_compose.py classify text1b
  func_800645B0 --target-object build/src/text1b.o --ours-object
  tmp/sandbox/func_800645B0/text1b.o` on the h build prints **FIRST DIVERGENCE:
  RA** (honest 78 / target 78; same instructions, different registers -- only
  `andi`/`sh`).  `goal_from_tgt.py goal --scope` narrows `$s0 -> $v0` to
  **UNIQUE pseudo 74** and emits goal `{"74": 2}`.  `inverse.py global
  --goal {"74":2} --depth 2 --top 8` over a 126-atom space in 6 classes
  (refs / live span / birth order / conflicts / preferences / calls-crossed)
  returns **FORECLOSED + NEGATIVE RESULT**, naming the mechanism: *"pseudo 74
  crosses 1 call(s) and $v0 is call-used, so prune_preferences (global.c:897)
  strips it from this allocno's preferences before find_reg ever sees it."*
  The extracted model corroborates it independently: `flow["74"].calls_crossed
  = 1` and `hard_conflicts["74"] = [2, 17, 29]` -- $v0 (reg 2) is a HARD
  conflict, so no preference or priority perturbation can reach it.
  Transcripts: `tmp/grind/func_800645B0/s16/solver_h_classify.txt`,
  `solver_h_inverse.txt`; model `h.model.json`, goal `h.goal.json`.

- **H70 (KILLED) -- frontier item 2 ("some other real value may seat better")
  dies as a FAMILY, not as a spelling.**  Routing the occupancy-bit OR result
  through idx instead of the masked random (`idx = val | mask;` consumed by
  `D_800A3444 = idx;`, with the halfword store taking `last & 7` inline) also
  measures **2 / 78 at 78 insns**, and the residual is the IDENTICAL two-insn
  shape one statement later: `59 OURS or s0,v1,s2 / TGT or v1,v1,s2`,
  `60 OURS sw s0,0(gp) / TGT sw v1,0(gp)`.  Note what this proves in passing:
  the masked random DOES land in $v0 as soon as it is not written into idx.  So
  the cost is not *which* value is borrowed -- it is the borrow.  idx is live
  across the `last = rand()` call, so its allocno is seated callee-saved for its
  whole range and every borrowed value inherits $s0, while the target computes
  every value in this arm caller-saved.  The three remaining candidate values
  (the `(rand() & 0xFF) - 0x7F` store operands, `idx2`, `mask`) are all consumed
  inside the same call-straddled region and inherit the same seat.  Banked:
  `rejected/or-result-routed-through-idx-same-callee-saved-seat-2of78.c`.

- **H71 (KILLED) -- the one C lever on the call-crossing is a target-contradicted
  direction.**  Ending idx's live range before the call (move `last = rand();`
  from before the *3 sum to after it) DOES free the seat -- idx drops to a
  caller-saved register -- but to **$a0, not $v0**, dragging `j` to $a1, and it
  measures **11 / 78**: with the sum available early, reorg.c fills the
  `jal rand` delay slot with the sum instead of the target's `sll s1,s0,0x1`,
  rewriting the whole loop head (10, 11, 14, 17, 18/19 swapped, 20, 63, 65).
  Decisively, the TARGET's own stream has `jal` at 18, `sll s1,s0,1` in the
  delay slot at 19 and `addu s0,s1,s0` at 20 -- i.e. **the target's idx IS live
  across that call**, which is exactly why the target seats it callee-saved.
  The call-crossing that forecloses $v0 is a property of the original code, not
  of our spelling.  Banked:
  `rejected/rand-moved-after-sum-steals-the-jal-delay-slot-11of78.c`.

- **H72 (KILLED) -- the multiply spelling of the *3 offset is not a lever.**
  `idx = idx * 3;` (an entirely ordinary C spelling of the word offset, and the
  one form whose synth_mult expansion could plausibly emit target's
  `addu dst, <idx<<1>, idx` operand order without a fresh destination) is
  **byte-identical to the SB floor**: 1 / 78 with the same single residual at
  index 20.  GCC 2.7.2 reduces `idx * 3` to `(plus (ashift idx 1) idx)`, CSE
  unifies the ashift with the live `idx2`, and the PLUS reaches the same
  `expand_binop` call with `target == op1`, so optabs.c:400-419 swaps
  identically.  H24/H58's operand-order wall is spelling-invariant for every
  commutative-PLUS form whose destination is the idx pseudo.  Banked:
  `rejected/mul3-strength-reduce-folds-to-the-same-addu-operand-order.c`.

- **The k (byte-offset) chassis is now fully typed too.**  Re-measured 12 / 78
  at 78 insns with EVERY OPCODE EXACT -- a pure register permutation
  (`goal_from_tgt`: `$s1->$s0 x9, $s0->$s1 x3, $v1->$s0 x2, $a0->$v1 x2`, goal
  `{"74":16, "78":16, "73":3}`).  Its goal requires two distinct allocnos (74 =
  idx, 78 = the sum) to share $s0, i.e. the target's sum destination coalesces
  into idx's seat.  `inverse.py global` returns NEGATIVE for the full goal AND
  for each narrowed sub-goal (`{"74":16}`, `{"74":16,"73":3}`, `{"78":16}`),
  each time reporting that the needed preference atoms cannot be emitted at all
  ("callee-saved registers cannot appear in pre-RA RTL from any C at all, so no
  spelling reaches this -- only a forbidden register-asm pin would").
  Transcripts: `solver_k_inverse.txt`, `solver_k_inverse_narrow.txt`.

- **Both endgame-lock gates re-run this session and both FAILED.**
  (a) `python3 tools/scan_hand_coded.py --single func_800645B0` = **tier=LOW
  score=0/8**, "no strong hand-coded indicators" (S1..S8 all clear; 78 insns,
  5 spills, 7 distinct registers).  (b) Precedent census re-confirmed NEGATIVE
  on the uncapped `docs/reference/sotn-construct-index.md` (pin
  `aa53500226ee84be763f3e8702b27de06456b3a7`, index generated 2026-09-01, 2746
  lines): the only PSX-tagged reuse evidence is still the five
  `// fake reuse of i?` cutscene hits, all of which BORROW an existing loop
  index -- and this session's H70 proves the borrow is precisely what costs the
  two insns here, so even a granted borrow would not close the function.
  Disposition filed: `## 2026-09-01 -- func_800645B0 -- RESOLVED BY STANDING
  RULING (2026-07-27): FORECLOSED` in `docs/grind/decisions.md`.

- **Artifacts.** `tmp/grind/func_800645B0/s16/` -- `apply.py`, `diff.py`,
  `m1_rand_after_sum.c`, `v1_mul3.c`, `t2_or_into_idx.c`, `h.model.json`,
  `h.goal.json`, `k.model.json`, `k.goal.json`, `solver_h_classify.txt`,
  `solver_h_inverse.txt`, `solver_k_inverse.txt`, `solver_k_inverse_narrow.txt`,
  `inv.sh`, `inv2.sh`.  `src/text1b.c` restored to HEAD at end of session.


## Session 16 (2026-09-01, forensics) -- the session-6 scheduling/optabs lock is BROKEN: a non-copy second real write to `idx` wins both halves; the residual is now a register seat

- **Chassis re-measured first.** SB body (`candidate.c`) pasted over the
  `INCLUDE_ASM` line: `sandbox func_800645B0 --disable all` = **score 1,
  target_insns 78, build_insns 78, rules_dropped 0**.  The floor of 1 is current.

- **H68 (CONFIRMED) -- the headline.**  H59 (s12) declared the 1-vs-3 trade
  closed-form on the premise that "the function computes nothing else that lands
  in `idx`".  That premise is FALSE: the halfword store consumes `rand() & 7`,
  and `idx` is dead from the *3 sum onward, so `idx = last & 7;` is a second
  write that is both semantically real and a COMPUTATION (not a copy, so no pass
  deletes it).  With the WD chassis' fresh-destination sum (`wid = idx2 + idx;`,
  which optabs.c:398-421 requires for target's operand order) plus that second
  write, the honest sandbox measures **2 / 78 at 78 insns** -- index 20 EXACT,
  inner-loop head 11/12 EXACT, back-edge delay slot 65 EXACT.  The whole residual
  is two register names: `55 OURS andi s0,v0,7 | TGT andi v0,v0,7` and
  `58 OURS sh s0,X(at) | TGT sh v0,X(at)`.  A second non-copy spelling
  (`idx = wid << 2;`, the byte-offset scaling) reproduces the target's ENTIRE
  opcode sequence at 78 insns with all twelve differences being register names.
  **The remaining problem on this function is register allocation, not
  scheduling and not RTL expansion.**

- **H67 (KILLED) -- which second writes do NOT work, and why.**  `idx = rand();`
  measures 3 / 78 (the WD residual verbatim) because a call return landing in a
  local is a HARD-REG COPY `(set (reg/v:SI 74) (reg:SI 2 v0))` and **combine.c
  deletes it**.  Per-pass census of the dumps
  (`tmp/grind/func_800645B0/s15b/g.{rtl,cse,cse2,flow,combine,sched}.txt`):
  2 sets of reg 74 at rtl / cse / cse2 / flow, **1 set at combine and at sched**,
  insn 118 gone.  Together with s15's cse.c finding (pseudo->pseudo copies die in
  cse) the rule is: **a second write to the carrier must COMPUTE something; any
  spelling whose RHS is a bare register is deleted before the scheduler.**

- **H66 (KILLED) -- the mirror attack on the scheduler tie, closed as a FAMILY.**
  `rank_for_schedule` (sched.c:2408-2465) falls through to `INSN_LUID` only when
  priorities tie, so granting the const-1 `li` the same `birthing_insn_p`
  max-priority lift the loop-top `addu` gets would hand the loop head to source
  statement order.  That needs `reg_n_sets[val] == 1` -- which is ALSO loop.c's
  invariant-hoist precondition.  Dump proof (`s15b/e.loop.txt`, loop.c's own
  dump): `Insn 41: regno 78 (life 1), move-insn savings 1  moved to 188` then
  `Insn 188: regno 78 (life 52), ... halved since already moved  moved to 190`,
  with `(insn 190 (set (reg/v:SI 78) (const_int 1)))` sitting BEFORE the outer
  `NOTE_INSN_LOOP_BEG`.  The hoisted constant is live across `jal rand`, so RA
  seats it callee-saved and the function pays `sw $20,32($sp)` + `lw $20,32($sp)`
  = +2 insns (`.frame regs=6` vs the target's 5).  Four spellings measured:
  A (SB + fresh `flags`) 13 / 80, B (WD + fresh `flags`) 12 / 80,
  C (WD + `mask` carrier) **14 / 77**, E (WD + `idx` carrier) 15 / 80.
  Corollary: the SB chassis' reuse of `val` for the D_800A3444 RMW is a LICM
  DEFEAT, not merely a scheduling choice.

- **Artifacts.** `tmp/grind/func_800645B0/s15b/` -- variant bodies
  `a_sb_valsplit.c`, `b_wd_valsplit.c`, `c_wd_mask_carrier.c`,
  `e_wd_idx_carrier.c`, `g_wd_idx_borrowed_for_rand.c`,
  `h_wd_idx_second_set_nonco.c`, `k_wd_idx_byteoffset.c`; dumps
  `e.{loop,rtl,sched}.txt` and `g.{rtl,cse,cse2,flow,combine,loop,lreg,sched}.txt`.
  Banked forms: `rejected/both-halves-idx-second-real-nonco-andi-regseat-2of78.c`,
  `rejected/idx-second-set-byte-offset-seat-permutation-12of78.c`,
  `rejected/idx-second-set-from-rand-is-a-copy-deleted-by-combine.c`,
  `rejected/once-set-const1-*-licm-hoist-*.c` (four).
  `src/text1b.c` restored to HEAD at end of session.



## Session 15 (2026-09-01, forensics) -- the Ruling-A named probe executed; cse.c is the reason the amended once-written family cannot close this function

- **Chassis re-measured this session.** SB body (`candidate.c`) applied over the
  `INCLUDE_ASM` line in `src/text1b.c`: `sandbox func_800645B0 --disable all` =
  **score 1, target_insns 78, build_insns 78, rules_dropped 0**.  The ledger
  floor of 1 is current on today's tree.

- **The owner-directed probe (2026-09-01 Ruling A, reopen note).**  "Build and
  measure the two-distinct-once-written-locals spelling -- each fresh local
  written exactly once, textually and semantically distinct from every banned
  entry."  Executed in both directions:
  - **P1** `wid = i + j; idx = wid;` at the inner-loop top, `wid2 = idx2 + idx;`
    for the *3 sum, the three word stores reading `wid2` = **3 / 78**, 78 build
    insns.
  - **P2** (mirror) `idx = i + j;` at the loop top, `wid = idx;` +
    `wid2 = idx2 + wid;` inside the if-arm = **3 / 78**, 78 build insns.
  Both residuals are the identical three objdump positions and are the WD
  fresh-destination residual verbatim: 11 (ours `addiu v1,zero,1` / target
  `addu s0,s3,a0`), 12 (the reverse), 65 (the inner loop's back-edge delay
  slot -- the target steals the `addu`, we steal the `li`).  The operand-order
  half of the 1-vs-3 lock is won by the fresh destination; the loop-head half
  is lost.

- **NEW MECHANISM, dump-proven -- `cse.c` deletes the copy before any pass that
  counts sets.**  This is the general finding of the session and it is what
  makes the amended family unusable HERE.  A once-written named intermediate
  at the inner-loop top can only hold a COPY of the carrier (the only real
  value at that point is `i + j` itself), and GCC 2.7.2's first post-RTL pass
  removes it:
  - P1: `tmp/grind/func_800645B0/s15/p1.rtl.txt` carries
    `(insn 41 38 44 (set (reg/v:SI 74) (reg/v:SI 79)))` -- the `idx = wid` copy.
    `p1.cse.txt` has **0** occurrences of `insn 41` and **0** references to
    `reg/v:SI 74`; `cse_insn` substituted reg79 into both `1 << idx` and
    `idx << 1` and deleted the copy insn.
  - P2: `p2.rtl.txt` carries `(insn 60 57 63 (set (reg/v:SI 79) (reg/v:SI 74)))`
    -- the `wid = idx` copy.  `p2.cse.txt` has **0** occurrences of `insn 60`
    and **0** references to `reg/v:SI 79`, and insn 63 reads
    `(plus:SI (reg/v:SI 75) (reg/v:SI 74))` directly.
  Consequence: `loop.c`'s `count_loop_regs_set` and `sched.c`'s
  `birthing_insn_p` (H61/H62) see reg_n_sets == 1 for the surviving carrier in
  BOTH probes, the max-priority lift is applied to the loop-top `addu`, and the
  loop-head placement is lost.  **Raising reg_n_sets above 1 in this function
  requires a second, differently-valued write to the same carrier -- a
  multi-WRITE carrier -- which is precisely this function's standing banned
  construct and is explicitly outside the amended named-intermediate family
  (`.claude/rules/no-new-park-categories.md:227-228`, "Multi-WRITE carriers
  remain NOT this entry").**  The reopen's own rule applies: the ban wins and
  the probe FAILs.

- **Gate (b) re-censused against the AMENDED class** (per the reopen note), on
  the uncapped index rebuilt 2026-09-01 at pin `aa535002`.  Still FAILS.  The
  PSX reuse evidence in the index is the frozen variable-reuse family
  (`// fake reuse of i?` at `src/boss/mar/cutscene.c:172`,
  `src/st/cen/cutscene.c:211`, `src/st/lib/cutscene.c:153`,
  `src/st/no3/cutscene.c:360`, `src/st/top/cutscene.c:143` -- all BORROW an
  existing loop index, which `staged-value-reused-variable.md` bound 2 bars us
  from inventing), and `src/weapon/w_037.c:300` is the once-written shape just
  measured at 3 / 78.  The `new_var_temp` class
  (`docs/reference/sotn-construct-index.md:1425`) carries declaration lines
  only and cannot evidence a write count.  No PSX-tagged precedent for an
  invented multi-write carrier exists in the index.

- **Artifacts.** `tmp/grind/func_800645B0/s15/p1.c`, `p2.c`, `apply.py`,
  `p1.rtl.txt`, `p1.cse.txt`, `p2.rtl.txt`, `p2.cse.txt`, `p2.combine.txt`;
  full pass dumps in `tmp/grind/func_800645B0/dumps/` (last written for P1).
  Banked forms: `rejected/two-once-written-locals-copy-deleted-by-cse.c`,
  `rejected/two-once-written-locals-mirror-copy-deleted-by-cse.c`.

- [s15] Floor re-measured first thing this session: SB chassis (memory/grind/func_800645B0/candidate.c) pasted over the INCLUDE_ASM line = score 1, target_insns 78, build_insns 78, rules_dropped 0. The ledger floor of 1 is current on today's tree.

- [s15] The remaining problem on func_800645B0 is REGISTER ALLOCATION, not scheduling and not RTL expansion: on the h form every instruction and every position matches and only two register names differ ($s0 where the target keeps the rand-return seat $v0); on the k form all 78 opcodes and their order match and twelve register names differ.

- [s15] loop.c hoists a once-set loop-invariant scalar out of BOTH loops here, and because the value is then live across `jal rand` it costs a callee-saved save/restore pair (+2 insns). This is why the SB chassis' reuse of `val` for the D_800A3444 RMW is load-bearing: it is a LICM defeat (.claude/rules/defeat-licm-hoist-var-reuse.md), not merely a scheduling choice.

- [s15] A reg-reg copy can never raise reg_n_sets at schedule time in this function: pseudo->pseudo copies die in cse.c (s15), hardreg->pseudo call-return copies die in combine.c (this session, per-pass dump census).

- [s15] Variant C (WD + `mask` as the RMW carrier) is the only form ever measured on this function at 77 insns - one SHORT of the target's 78 - at score 14. Banked for the record.

- [s15] src/text1b.c was restored to HEAD at end of session; main continues to carry INCLUDE_ASM("asm/funcs", func_800645B0). No engine/tools/rules files were touched.

- [s15] The 2026-09-01 11:09 discarded-session FORECLOSED span in docs/grind/decisions.md is superseded on the merits; a PROGRESS NOTE recording that (not a disposition, no question to the owner) was appended to docs/grind/decisions.md this session.

- [s16] Chassis re-measured FIRST on this session's tree: SB body (memory/grind/func_800645B0/candidate.c) pasted over the INCLUDE_ASM line gives sandbox func_800645B0 --disable all = score 1, target_insns 78, build_insns 78, rules_dropped 0. The floor of 1 is current; the h form re-measured 2 / 78 unchanged.

- [s16] The owner's 2026-09-01 Ruling-C --target-object escape is IMPLEMENTED in tools/ra_solver/inverse_compose.py and works on this INCLUDE_ASM function; build/src/text1b.o is a valid target stream (78 insns, matches asm/funcs alignment).

- [s16] inverse_compose classify on the h build: FIRST DIVERGENCE = RA, 78 vs 78, identical instruction multiset, two register differences only (andi/sh, $s0 vs target $v0).

- [s16] goal_from_tgt scope-narrowed attribution: $s0 -> $v0 narrows to UNIQUE pseudo 74; inverse.py global returns FORECLOSED + NEGATIVE at depth 2 naming global.c:897 prune_preferences (pseudo 74 crosses 1 call, $v0 is call-used). Model: flow[74].calls_crossed = 1, hard_conflicts[74] = [2,17,29].

- [s16] The k (byte-offset) chassis is 12 / 78 with EVERY OPCODE EXACT - a pure register permutation ($s1->$s0 x9, $s0->$s1 x3, $v1->$s0 x2, $a0->$v1 x2). Its goal requires two distinct allocnos (idx and the sum) to share $s0; inverse.py returns NEGATIVE for the full goal and every narrowed sub-goal.

- [s16] Borrowing idx for ANY real value costs exactly the same two instructions: the masked-random borrow (h) and the OR-result borrow (t2) both measure 2 / 78 with the same residual shape, and the masked random lands in $v0 as soon as it is not written into idx.

- [s16] The target's own instruction stream proves its index is live across the jal rand (jal at 18, sll s1,s0,1 in the delay slot at 19, addu s0,s1,s0 at 20), so the call-crossing that forecloses the $v0 seat is a property of the original code and cannot be spelled away.

- [s16] Endgame-lock gate (a) FAILED: tools/scan_hand_coded.py --single func_800645B0 = tier=LOW score=0/8, no strong hand-coded indicators (S1..S8 all clear; 78 insns, 5 spills, 7 distinct registers).

- [s16] Endgame-lock gate (b) FAILED: precedent census re-run on the uncapped docs/reference/sotn-construct-index.md (pin aa53500226ee84be763f3e8702b27de06456b3a7, generated 2026-09-01, 2746 lines) - only the five `// fake reuse of i?` cutscene borrow hits, and H70 shows a borrow precedent would not close this function anyway.

- [s16] Disposition filed by this session at the end of docs/grind/decisions.md: `## 2026-09-01 - func_800645B0 - **RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED**`. src/text1b.c restored to HEAD; main still carries INCLUDE_ASM("asm/funcs", func_800645B0).
