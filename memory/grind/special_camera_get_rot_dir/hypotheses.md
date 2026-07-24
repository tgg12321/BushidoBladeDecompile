# Hypothesis ledger — special_camera_get_rot_dir

## CONFIRMED (s1)
- H0: masked sandbox is a valid gradient (score == rotation-diff count).
  CONFIRMED — 3 edits moved score {12->14,9,22}. sandbox 0 <=> full match.
- H1: copy_end has anomalously HIGHEST allocno priority (grabs s2); the whole
  rotation is copy_end being 1st instead of last. CONFIRMED via greg + the
  global.c:604 formula (copy_end n_refs=2 with short live_length beats the
  n_refs=3 trio). Block-local copy_end drops it out of s2 -> floor 12->9.

## KILLED (s2) — with exact instrumented-cc1 measurements
- F1 (extend copy_end live range across one call -> s5 while keeping trio in
  s2/s3/s4): KILLED. copy_end's livelen is capped ~38 (sole use = mid-function
  do-loop; nothing after the loop uses the end pointer — verified: Triple-copy
  reuse extends range by only 1 insn and breaks tail bytes). 38 << const(76),
  so copy_end pri (>=789) can never drop below const's 394. Assignment is pure
  priority order (find_reg takes lowest available reg; no cost bias to higher
  regs; copy_end has no copy-preference lever, being a leaf addiu). => copy_end
  cannot reach s5 by any def/use placement.
- F2 (nudge copy_end to lowest priority via livelen + allocno tiebreak): KILLED.
  A tiebreak win needs pri(copy_end)==pri(const). Requires livelen equality;
  copy_end max ~38, const locked at 76 (needed at both calls; literal-const
  probe left livelen=76). Neither can move to meet. => no tie possible.
- F3 (compute the numbers): DONE. Built scratch instrumented cc1; exact
  nrefs/livelen/pri banked for all 5 forms. The computation PROVES F1/F2 dead.

## OPEN FRONTIER (for s3+)  — NOT def/use placement (that axis is EXHAUSTED)
The sub-problem stays crisp: make **copy_end land in callee-saved s5** (currently
t0). s2 PROVED this is unreachable through allocno PRIORITY (copy_end livelen
capped ~38 << const's locked 76; find_reg has no cost bias to higher regs;
copy_end is a leaf addiu with no copy-preference). So the remaining levers are
NOT about copy_end's def/use position or its live length. Genuinely-untested:

- G1 (pre-alloc RTL shape via sched1): livelen is measured on the RTL AFTER
  cse/loop/flow/combine/sched1 but BEFORE reload. Two C sources that emit the
  same final bytes (sched2/reorg restores them) can present DIFFERENT pre-alloc
  livelens. Find a C form whose sched1 output makes copy_end's OR const's
  pre-alloc range differ (e.g. a form that keeps the do-loop's copy_end-use
  scheduled later in the pre-alloc stream than in the final stream). Probe:
  inspect the .sched1/.combine dumps (cc1 -da) for both candidate and target-
  reconstructed C to see whether the do-loop's position in pre-alloc RTL can be
  moved without moving the emitted loop. HIGH uncertainty; may be a dead end too.

- G2 (induce a copy-preference legitimately): copy_end -> s5 could come from a
  hard_reg_copy_preference (find_reg global.c:1046+), NOT priority. That needs a
  real copy insn linking copy_end to a value that lands in s5 — WITHOUT a
  register pin (pins are cheats). Explore whether the ORIGINAL loop expressed the
  end pointer via a copy from another pointer (e.g. end = src_base derived by a
  move) that GCC would copy-preference. Must stay byte-neutral. Uncertain a
  legitimate C form exists.

- G3 (re-derive the s5 identity): re-check the assumption that s5 == copy_end is
  the only decomposition. m2c the target and read the loop shape; confirm the
  original didn't express the bound as a down-counter or a dest-relative end
  that changes which value is long/short-lived. (Target DOES compare src->s5, so
  s5 is a src-relative end pointer — but verify no coalescing/aliasing changes
  the allocno set.)

NB for s3: this is the global.c:624 allocno-priority wall class
(marionation_Exec / cpu_side_move_dir_4). If G1/G2/G3 also measure dead, the
next session's mandated modality (per the ladder) is likely permuter or
escalation — do NOT re-run the def/use placement sweep (banked dead above).

## Ruled out (rejected/)
- copy_end def 2nd -> 14.  - copy_end assigned inside retry -> 22.
- reorder index/cam_base before constant_80 -> breaks prologue (s0).

## [s1] The masked sandbox (--disable all) is a valid gradient for this function, contradicting the s0/WIP 'sandbox is blind' claim.
- mechanism: Score == exact count of rotation-differing instructions; the sandbox strips register PINS (so pin edits are inert) but DOES reflect real C-structure changes to GCC's allocation.
- probe: Made 3 structural edits; sandbox score moved 12 -> {14, 9, 22}.
- result: score responds monotonically to C structure; 9 is a genuine floor drop
- verdict: CONFIRMED

## [s1] The entire honest gap is a pure 4-register rotation of {index,cam_base,constant_80,copy_end} over callee-saved s2/s3/s4/s5; copy_end has the highest allocno priority but must be lowest.
- mechanism: global.c:604 priority = floor_log2(n_refs)*n_refs/live_length (no loop weighting). copy_end n_refs=2 with a short live_length beats the n_refs=3 trio, so it grabs s2. It should be s5 (largest live_length / tiebreak).
- probe: objdump diff of build vs target + cc1 -da greg dispositions (pseudo 77->s2=copy_end, 73->s3=index, 74->s4=cam_base, 76->s5=constant_80).
- result: rotation fully explained by copy_end priority; the other three are in correct relative order
- verdict: CONFIRMED

## [s1] Defining copy_end block-locally (in the copy block) drops it out of s2 and correctly lands index->s2, cam_base->s3, constant_80->s4.
- mechanism: Shortening/localizing copy_end's live range removes the call-crossing, so GCC picks a caller-saved temp (t0) for it, freeing s2 for index; the trio then falls into target slots.
- probe: Moved 'copy_end = &sp_buf[0x40]' into the copy block just before the do-loop; sandbox=9, build_insns=70.
- result: floor 12->9; residual is only copy_end being in t0 (caller-saved) vs target's s5 (callee-saved, +2 save/restore)
- verdict: CONFIRMED

## [s2] F1: extend copy_end's live range across one call so it becomes callee-saved s5 while keeping index/cam/const in s2/s3/s4.
- mechanism: Allocno priority pri=floor_log2(nrefs)*nrefs/livelen*10000; find_reg (global.c:1011-1044) assigns lowest available reg in ascending order (no MIPS REG_ALLOC_ORDER, no cost bias to higher regs). For copy_end->s5 it must be LOWEST priority => livelen>76 (const's).
- probe: Instrumented scratch cc1 (BB2_ALLOC_DEBUG, relinked into tmp/ from in-tree global.o). Swept copy_end def placement: block-local L5->t0(score9); top-def L32->s2(12); def-first L38->s3(14). Confirmed copy_end's sole use is the mid-function do-loop; reusing it in the post-loop Triple copy extended range by only 1 insn and broke tail bytes (score12).
- result: copy_end livelen maxes at ~38 (def moved to function entry) << cam(64)/const(74-76); it can never drop below cam/const in priority, so never s5. Assignment is pure priority order; copy_end (leaf addiu, no copy insn) has no preference lever.
- verdict: KILLED

## [s2] F2: tie copy_end's priority with const so the allocno-number tiebreak (global.c:624, higher pseudo last) hands copy_end s5.
- mechanism: A tie needs pri(copy_end)==pri(const) => equal livelen (both nrefs 3). Lower const's livelen toward copy_end's ~38, or raise copy_end's toward 76.
- probe: Passed literal 0x80 (not constant_80) as the final func_800372F4 arg to shorten const's live range. Measured with instrumented cc1.
- result: const_80 nrefs dropped 3->2 but livelen STAYED 76 (CSE still holds 0x80 in a callee-saved reg entry->final-call); pri 263, score 10 (worse). const's livelen is structurally locked at 76; no tie is reachable.
- verdict: KILLED

## [s2] F3: obtain exact allocno nrefs/livelen/priority for all four competitors to turn F1/F2 from guess-and-check into a calculation.
- mechanism: The in-tree tools/gcc-2.7.2/global.o already carries the BB2_ALLOC_DEBUG fprintf; the shipped build/cc1 was linked before it. Relink cc1 from in-tree .o's into tmp/ (touching nothing in tools/).
- probe: Built tmp/grind/.../s2/cc1_dbg; ran on 5 forms; extracted special_camera's allocno batch each time.
- result: Exact table banked (top-def: dest s0 pri2105, buf2 s1 967, copy_end s2 937, index s3 882, cam s4 454, const s5 394). Computation proves F1/F2 dead and identifies the pure-priority-order assignment rule.
- verdict: CONFIRMED

## KILLED (s3) — structural axis exhausted with measurements
- G1 (pre-alloc RTL / sched1 gives copy_end a longer or const a shorter pre-alloc
  livelen while bytes stay identical): KILLED for the def-placement + statement-
  reassociation sub-axes. Measured that flow.c reg_live_length counts loop-carry (copy_end
  IS live in the bottom block 4, 19 insns = const's), so the deficit is block-0 (def
  position) only. Loop-invariant def-placement IS a real lever (const 76->62, copy_end
  ->38; drove score 12->11->10, fixing index+const slots) but copy_end's livelen is
  hard-capped ~38 by its earliest-last-use (inner-loop bne); it can't exceed cam(66)/
  const(62), and no byte-neutral later reference exists (tail copy must read via src, not
  copy_end). So copy_end can never be lowest-priority -> never s5 via priority.
- G2 (induce a hard_reg_copy_preference to pull copy_end to s5): KILLED by the target asm
  itself — target has NO copy/move insn to $s5; copy_end reaches s5 purely by lowest allocno
  priority. Copy-preference is not the target's mechanism, so it's not a valid route.
- G3 (re-derive s5 identity via m2c): DONE/CONFIRMED — same single-loop decomposition,
  s5 = loop-invariant src-relative end pointer hoisted to preheader. No alternate allocno set.

## OPEN FRONTIER (for s4+) — structural axis is EXHAUSTED
The sub-problem is unchanged: copy_end must land in callee-saved s5. s3 proved (with the
new def-placement lever + flow-trace + m2c) that NO pure-C statement arrangement makes
copy_end simultaneously call-crossing (callee-saved) AND longest-lived (lowest priority),
because its only use is structurally the earliest of the four callee-saved invariants and
byte-neutrality forbids a later reference. Remaining, genuinely-untested modalities:
- Permuter (directed): PERM_* over statement order / invariant-def placement seeded from the
  score-10 V_cefirst_constlast form (index+const already correct). Random mode never run on
  this fn. Low expectation (the 2-cycle is priority-locked) but not measured.
- Escalation: this is the marionation_Exec / cpu_side_move_dir_4 allocno-priority-tie wall
  class. If permuter also dead, escalation modality (owner ruling on the endgame-lock
  disposition policy) is the ladder's next step. NOT owner-gated this session (structural
  modality; permuter still ungrinded = still grindable).

## [s3] G3: an alternate loop/allocno decomposition (down-counter or dest-relative bound) makes the long-lived value take s4 and the short one s5 naturally.
- mechanism: m2c reconstruction + coalescing/aliasing check for a different reg_may_share set.
- probe: Ran m2c on the target; inspected loop shape.
- result: Target body is a single loop (one header, two back-edges) identical to my `retry` form; s5 = loop-invariant src-relative end pointer (sp+0x50) hoisted to the outer preheader. Same decomposition as candidate; no alternate allocno set.
- verdict: KILLED

## [s3] G2: induce a hard_reg_copy_preference (find_reg global.c:1046+) that pulls copy_end to s5 without a register pin.
- mechanism: A copy insn linking copy_end to a value already in s5 overrides best_reg to s5, bypassing priority.
- probe: Inspected the target asm for any move/copy insn to $s5.
- result: Target has NO copy/move insn to $s5 whatsoever; copy_end reaches s5 purely via lowest allocno priority. Copy-preference is not the target's mechanism, so it is not a valid route.
- verdict: KILLED

## [s3] G1: a pure-C statement arrangement gives copy_end a longer (or const a shorter) pre-alloc livelen, flipping copy_end to lowest priority (s5) while keeping bytes identical.
- mechanism: flow.c reg_live_length (per-insn live count) sets allocno priority = floor_log2(nrefs)*nrefs*10000/livelen; the 4 callee-saved values (index, cam, const, copy_end) are all loop-invariant-hoisted, so their livelen is set by last-use position.
- probe: Swept loop-invariant def-placement with instrumented cc1 + BB2_FLOW_DEBUG per-insn liveness. Forms: top-def(12), const-last(11, const L76->62), copyend-first+const-last(10, index->s2 & const->s4 correct), const-reassign-in-loop(17, breaks bytes).
- result: Def-placement IS a real lever (score 12->11->10) but flow counts loop-carry (copy_end live in bottom block4 = const's 19 insns); copy_end's deficit is block-0 only. copy_end livelen hard-caps ~L38 by its earliest last-use (inner-loop bne); it cannot exceed cam(L66)/const(L62), and no byte-neutral later reference exists (tail copy must read via src, not copy_end). So copy_end can never be lowest-priority -> never s5.
- verdict: KILLED
