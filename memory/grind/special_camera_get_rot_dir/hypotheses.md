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

## KILLED (s4) — permuter modality measured dead; escalated owner-gated
- P1 (directed/random permuter finds a livelen arrangement the manual sweep missed):
  KILLED. Clean single-function workspace (target.o at offset 0). Random mode (never run
  before) from TWO structurally-distinct seeds — score-10 hoisted-copy_end and score-9
  block-local — ran ~46k combined iterations. Chassis 1: 27.5k iters, best 318 = pure
  reorder-penalty reduction, register rotation unchanged (copy_end<->cam 2-cycle intact).
  Chassis 2: 18.5k iters, best 113, every find a DIFFERENT wrong permutation of the 5
  callee-saved values, never target {index=s2,cam=s3,const=s4,copy_end=s5,buf2=s1}. No
  score-0 in either. The permuter's statement/decl-reorder + var-permutation search covers
  exactly the def/use-placement + allocno-tiebreak axes s2/s3 proved dead; it empirically
  confirms the proof. copy_end->s5 requires it to be lowest-priority (longest-lived), but its
  sole use is the earliest of the four callee-saved invariants — structurally unreachable.
- P2 (canonical-asm authorization for a hand-coded construct): KILLED. scan_hand_coded LOW
  2/8, no STRONG signal; the residual is an ordinary allocno-priority RA tiebreak. A byte-0
  form provably requires a register pin (asm("s5") on copy_end), which is a forbidden cheat
  with no SOTN precedent — authorizing canonical-asm would launder the pin (inline-asm-injection trap).

## DISPOSITION (s4): owner-gated
Every grind-advanceable axis is measured dead: structural (s1-s3, instrumented-cc1 proof),
permuter (s4, ~46k iters/2 chassis). Both endgame-lock AND-gates fail (canonical-asm LOW 2/8;
no SOTN precedent for a register-rotation pin). This is the marionation_Exec /
cpu_side_move_dir_4 allocno-priority-tie wall class. Filed OWNER-ESCALATION in
docs/grind/decisions.md (2026-07-23, special_camera_get_rot_dir) presenting the two AND-gates;
returned owner-gated. NB: no cheat is present (0 rules) — unlike cpu_side_move_dir_4 there is
nothing to "retain"; the clean floor-9 candidate is the best committable pure-C form and is
+2 insns short of the target (missing the s5 save/restore pair), so it is NOT a COMPLETED-C match.

## [s4] Random-mode permuter over a clean single-function workspace, seeded from the score-10 hoisted-copy_end form, finds a livelen/statement arrangement the manual sweep missed and reaches a byte match.
- mechanism: decomp-permuter randomizes statement order, declaration order, and variable permutation — exactly the def/use-placement + allocno-tiebreak axes. target.o built at offset 0 (asm/funcs + prelude, .set gp=64 dropped) so the score is the real per-function diff.
- probe: 27,499 iters, --stop-on-zero. Best novel find output-318-1 (perm-score 318 vs base 328); compiled + objdump-diffed vs target.
- result: Only find is pure reorder-penalty reduction (li s4,128 moved); the copy_end(s3)<->cam(s5) 2-cycle is UNCHANGED. No score-0.
- verdict: KILLED

## [s4] Random-mode permuter seeded from the structurally-distinct score-9 block-local form (copy_end in caller-saved t0) reaches the target register permutation from a different basin.
- mechanism: Different starting allocation (copy_end non-hoisted, 70 insns) gives the permuter a different neighborhood to bridge to target's hoisted-s5 form.
- probe: 18,554 iters, --stop-on-zero. Best find output-113-1 (perm-score 113, 72 insns) compiled + diffed vs target.
- result: Every find is a DIFFERENT wrong permutation of the 5 callee-saved values (best 113: index->s3, cam->s4, const->s5, copy_end->s1, buf2->s2), never target {index=s2,cam=s3,const=s4,copy_end=s5,buf2=s1}. No score-0 in ~46k combined iters.
- verdict: KILLED

## [s4] The copy_end->s5 residual qualifies for canonical-asm authorization (hand-written-asm construct).
- mechanism: endgame-lock-disposition AND-GATE 1: canonical-asm only if scan_hand_coded shows STRONG (S1/S2/S6) signals.
- probe: tools/scan_hand_coded.py --single special_camera_get_rot_dir.
- result: tier LOW, score 2/8 (only S4 front-loads + spurious S5 self-cluster to func_80037348 = same 0x80037348 address). No STRONG signal. The residual is ordinary GCC allocno-priority RA output; the only byte-0 form is a forbidden register pin (inline-asm-injection trap).
- verdict: KILLED

## s5 (synthesis) — FRONTIER RESET. The residual is a live-length CHAIN with two routes; s1-s4 only ever attacked one of them.

The five-session framing "copy_end must land in callee-saved s5" is correct but under-specified,
and the under-specification is what made it look like a wall. The exact condition (derived from an
8/8-exact ra_solver model of THIS function, not estimated) is a total order on live lengths:

      L(buf2) < L(index) < L(cam) <= L(const) <= L(copy_end)      [all nrefs=3]

Two chassis satisfy different prefixes of it, and each opens a DIFFERENT route:
  * top-def (score 12): 31 < 34 < 66 <= 76 — cam/const already correct; copy_end at L32 is the
    single defect. Route A only (needs L(copy_end) >= 76). Route B foreclosed here.
  * V_cefirst_constlast (score 10): 32 < 34, then 66 > 62 > 38 — copy_end already ABOVE index at
    L38. Route B only (needs cam and const pulled down into [34,38]). Route A also open in principle.

## KILLED (s5)
- S1 (Route A' — reach the target by giving copy_end nrefs=2, which would relax its live-length
  requirement from >=76 to >=51): KILLED by mechanism. `reg_n_refs` is loop-depth weighted
  (flow.c:2081); copy_end's 2 textual RTL refs weigh 3 because its def is at loop depth 1 (retry
  preheader) and its only use is the `bne` at loop depth 2 (inside the copy loop). nrefs=2 requires
  the copy loop not to be a loop. Also disposed of the underlying premise: the TARGET's $s5 has the
  same 2 machine refs, so the target is NOT reaching s5 via a lower ref count.
- S2 (the ra_solver inverse solver hands us a spellable lever vector): KILLED as posed. Every
  minimal (2-atom) vector it returns requires `refs 3->1` on copy_end, which no C spells. Its
  live-length atoms are bounded to +/-{2,4,8} (inverse.py:206-220), so the -28/+44 deltas this
  function actually needs are outside the searched space. Use the direct simulate.py sweeps instead
  (banked). The run's FORECLOSED section is a genuine positive result: it mechanically re-derives
  the s3 G2 kill — no callee-saved preference for $s3/$s4/$s5 can exist in pre-RA RTL from any C.

## CONFIRMED (s5)
- C1: ra_solver's forward model is EXACT for special_camera_get_rot_dir (8/8 dispositions + sort
  order on two chassis). Reachability questions here are calculations. Sweep first, compile second.
- C2: the reachability spec above (exhaustive simulation, 364 hits = exactly the chain).
- C3: chassis choice determines which route is even available (see above); the floor-9 candidate is
  a third basin that satisfies neither chain.

## OPEN FRONTIER (for s6+) — ranked
- **R1 (PRIMARY, never attempted in five sessions): Route B — shrink cam and const.** Seed from
  `rejected/copyend-first-const-last-score10.c` (score 10, re-measured this session). Goal: pull
  L(cam) from 66 into [34,38] and L(const) from 62 into [L(cam),38], leaving copy_end alone at L38.
  Both are loop-invariant values held across the retry loop; the whole surplus is the span from
  their (hoisted) defs down to their last uses. Probes worth running IN THIS ORDER:
  (a) segment `tmp/grind/.../s5/flowdbg.py` per function (it currently reports TU-wide counts) and
      get the per-block live-count breakdown for pseudos 73/74/76/77 — that names exactly which
      blocks contribute cam's 66 and const's 62 versus index's 34, and therefore what has to stop
      being live where;
  (b) attack whichever block dominates the surplus with ordinary C (recompute rather than hold,
      re-associate `index + cam_base` so cam's last use moves earlier, sink the def past the first
      call), re-extracting the model after each edit — the simulator says immediately whether the
      chain closed, without needing the sandbox score to move monotonically.
  NB s2 already measured one const-shortening probe (literal 0x80 at the final call) as inert:
  nrefs fell 3->2 but livelen stayed 76 because CSE still held the value. That is one spelling, not
  the axis.
- **R2: Route A revisited only if R1's per-block data shows copy_end's cap is softer than L38.**
  The requirement is L(copy_end) >= 76 in the top-def chassis. s2/s3 killed this on def-placement
  grounds; do not re-run the def-placement sweep. It reopens only if the per-block liveness trace
  shows copy_end is being counted in fewer block visits than it structurally should be (i.e. the
  cap is a flow-counting artifact rather than a last-use fact).
- **R3: pre-RA scheduling (sched1) as the live-length lever.** Live lengths are measured on the
  post-sched1 stream while the emitted order is set by sched2, so two C forms with identical bytes
  can present different pre-RA live lengths. `tools/sched_solver` models both scheduler passes
  order- and clock-exactly and has never been pointed at this function. This is the only remaining
  mechanism that could move a live length without moving a statement.

## DISPOSITION (s5): progress
Not owner-gated. The 2026-07-23 OWNER-ESCALATION entry (docs/grind/decisions.md:1579) and the
2026-07-27 ruling recorded the function as REFUSED / OWNER-ACCEPTED INCOMPLETE on the strength of
"every grind-advanceable axis is measured dead". That claim is now FALSE: Route B (R1) and the
sched_solver axis (R3) are both live and unmeasured, and the owner's 2026-08-24 escalation-not-parked
ruling already returned the function to active grinding with exactly this instruction (use the
solver before a deep re-grind). The function is grindable; the ladder should continue.

## [s5] The target register assignment is reachable iff the six allocnos' live lengths satisfy L(buf2) < L(index) < L(cam) <= L(const) <= L(copy_end), all at nrefs=3 — i.e. the wall is a chain condition, not the single proposition 'copy_end must be lowest priority'.
- mechanism: global.c assignment is pure allocno-priority order (pri = floor_log2(n)*n*10000/L, tie -> lower pseudo number), and find_reg takes the lowest available hard reg. With all four callee-saved invariants at nrefs=3 the priority order is fully determined by live length, so the target permutation {72 s0, 78 s1, 73 s2, 74 s3, 76 s4, 77 s5} is exactly one total order on live lengths.
- probe: tools/ra_solver/extract.py + simulate.py on both chassis (8/8 dispositions and sort order MATCH each time), then exhaustive simulation over cam/const/copy_end live lengths in [28,46) on the score-10 model (tmp/grind/special_camera_get_rot_dir/s5/sweep3.py).
- result: 364 hits, all and only the chain. Current chains: score-10 = 32 < 34 < 66 > 62 > 38 (two inversions); top-def = 31 < 34 < 66 <= 76 with copy_end at 32 (one inversion).
- verdict: CONFIRMED

## [s5] copy_end can be driven to nrefs=2, which would relax its live-length requirement from >=76 to >=51 and make Route A reachable.
- mechanism: reg_n_refs enters the priority numerator as floor_log2(n)*n, so 3->2 is a 1.5x priority cut — a bigger lever than any live-length move measured in s1-s4. The target's $s5 carries only 2 machine refs, which made a lower ref count look like the target's own mechanism.
- probe: Counted textual RTL refs per pseudo in the .flow/.combine/.lreg dumps (tools/grinder/dump.ps1) and compared against the instrumented-cc1 ALLOCDBG nrefs; then read tools/gcc-2.7.2/flow.c:2081 and flow.c:1685/2087.
- result: copy_end has exactly 2 textual RTL refs but nrefs=3 because reg_n_refs is LOOP-DEPTH WEIGHTED (flow.c:2081, reg_n_refs[regno] += loop_depth) while reg_live_length is a plain per-insn-visit count: the def is at loop depth 1 (retry preheader) and the bne use at loop depth 2 (inside the copy loop). nrefs=2 requires the copy loop not to be a loop. The same weighting explains pseudos 81 (6 textual / 8 weighted) and 82 (7 / 10). Corollary: the target's $s5 has the identical 2-textual profile, so the target also allocates copy_end at nrefs=3 and reaches s5 by satisfying L(copy_end) >= L(const).
- verdict: KILLED

## [s5] ra_solver's inverse solver (owner directive 2026-08-24: run the solver before a deep re-grind) yields a spellable C-lever vector for this residual.
- mechanism: inverse.py enumerates minimal perturbations of the modelled RA inputs that reach a goal assignment and maps each to catalogued C techniques.
- probe: python3 tools/ra_solver/inverse.py global <model> --goal '{"74":19,"76":20,"77":21}' --depth 2 --top 10 on the score-10 model.
- result: Minimal solution size 2 atoms, 6 vectors, but EVERY vector pairs a cam/const live_shrink with 'refs_down pseudo 77: refs 3->1', which no C spells (a 1-ref pseudo is def-only or use-only). The solver missed both real routes because its live-length atom space is hard-bounded to +/-{2,4,8} (tools/ra_solver/inverse.py:206-220) while the deltas needed here are -28 (cam) and +44 (copy_end). Its FORECLOSED section is a genuine positive: all 24 preference atoms naming $s3/$s4/$s5 are unreachable because callee-saved registers cannot appear in pre-RA RTL from any C — a mechanical re-derivation of the s3 G2 (copy-preference) kill.
- verdict: KILLED

## [s5] Chassis choice is load-bearing: the s3/s4 'best' score-10 form and the score-12 top-def form open OPPOSITE routes to the target, so the four-session conclusion 'every grind-advanceable axis is dead' was drawn from one branch only.
- mechanism: Route B (pull cam and const below copy_end) needs L(copy_end) > L(index); Route A (lift copy_end above const) needs L(copy_end) >= L(const). The score-10 chassis has copy_end at L38 > index L34 (Route B open, Route A needs +38); the top-def chassis has copy_end at L32 < index L34 (Route B foreclosed, Route A needs +44).
- probe: Re-measured all three forms on the current HEAD chassis (sandbox --disable all) and swept each model with simulate.py: sweep2.py (const x copy_end), sweep3.py (cam x const x copy_end), sweep_topdef.py (route A/A'/B on the top-def model).
- result: candidate=9 (70 insns), score-10 form=10 (72), top-def=12 (72) — all chassis-valid. Route B reaches the target from the score-10 chassis for every L(cam) in [34,38] with L(const) in [L(cam),38], copy_end untouched at L38; Route B has ZERO hits from the top-def chassis; Route A needs L(copy_end)>=76 (nrefs 3) or >=51 (nrefs 2) from top-def.
- verdict: CONFIRMED

# FRONTIER RESET (s6) — supersedes the s5 frontier

The residual is no longer "a live-length chain that C statement placement must satisfy". The
def-order chassis that satisfies the chain EXISTS and is banked
(`rejected/routeC-seed-deforder-basewindow-score16.c`, base lengths 38/37/36/32/31). Exactly one
thing stands between it and a byte match: `local-alloc.c:1064` doubles the live length of pseudos
74 (cam) and 76 (const) because each carries a REG_EQUIV note. Undoubling them — verified on an
exact model — yields the target register assignment, with a 28-pair tolerance window.
Work ONLY that question. Do not re-open Route A, Route A', the statement-shuffle form of Route B,
the permuter, or the structured-loop retry chassis; all are killed with mechanisms.

## [s6-F1 PRIMARY] A zero-instruction-cost C spelling exists that makes reg_n_sets >= 2 for the cam and const pseudos, removing their REG_EQUIV and therefore the x2 doubling, and it lands the byte match from the def-order chassis.
- mechanism: `update_equiv_regs` (tools/gcc-2.7.2/local-alloc.c ~1015-1030) bails out with
  `continue` before granting REG_EQUIV whenever `reg_n_sets[regno] != 1`, and the x2 at
  local-alloc.c:1064 is gated on that note. `reg_n_sets` is counted by flow's life_analysis, which
  runs BEFORE combine/sched/local-alloc; cross-jumping (jump2) and reload's no-op-move deletion run
  AFTER. So a second SET that exists at flow time can still be absent from the emitted stream.
- next probe: seed src from `rejected/routeC-seed-deforder-basewindow-score16.c` and iterate with
  `bash tmp/grind/special_camera_get_rot_dir/s6/alloc.sh <tag>` (one call, prints the exact allocno
  table off src/ — the true gradient; sandbox score is a lagging summary). Success signal, checkable
  in that one table: 74 and 76 report livelen in [32,38] with L(74) <= L(76) and copy_end 77 at 38,
  at 72 build insns. Two ranked spellings: (a) two identical assignments placed in
  cross-jumpable tails (both `if (v0 != 0) goto retry;` sites are candidates — cross-jump merges
  identical tails after reload, so the flow-time count is 2 and the emitted count may be 1);
  (b) initialise via a REG-to-REG copy across an EBB boundary so cse attaches no constant REG_EQUAL
  (cse.c:6923 requires `GET_CODE (src_const) != REG`), letting local-alloc's optimize_reg_copy_1/2
  tie the pseudos and reload delete the move. Always confirm the insn count is still 72 — the s6
  probe `rejected/twoset-kills-regequiv-plus2insns.c` proves the doubling dies but cost +2 insns.

## [s6-F2] The doubled length can instead be pulled back INTO the window by local-alloc's direct live-length arithmetic, without removing the REG_EQUIV at all.
- mechanism: `optimize_reg_copy_1` (called from the same update_equiv_regs insn loop,
  local-alloc.c:1003-1015) writes `reg_live_length[sregno] -= length` and
  `reg_live_length[dregno] += d_length` at local-alloc.c:820-831 — the only other site in the whole
  compiler that edits a live length after flow. cam at 72 needs to land anywhere in [32,38] and
  const at 74 anywhere in [L(cam),38]; a copy whose source is cam or const and which is coalesced
  away would subtract exactly this kind of delta.
- next probe: from the same def-order chassis, introduce a reg-reg copy consuming cam (and const)
  whose source is not dead at the copy — the precondition for optimize_reg_copy_1 at
  local-alloc.c:1003-1007 — and read the resulting livelen off `alloc.sh`. This is a pure
  measurement: it either moves the two numbers into the window at 72 insns or it does not.

## [s6-F3] Route R3 (pre-RA scheduling moves a live length without moving a C statement) survives s6 untouched and is now much better targeted.
- mechanism: live lengths are measured on the post-sched1 stream while emitted byte order is set by
  sched2, so two byte-identical C forms can present different pre-RA live lengths.
  tools/sched_solver models both passes order- and clock-exactly and has still never been pointed at
  this function.
- next probe: run sched_solver on the def-order chassis and ask only one question — can any legal
  sched1 ordering move base(cam)/base(const) or their doubled values into [32,38] while leaving
  copy_end at 38 and the emitted stream byte-identical? Because the doubling is applied after flow,
  a scheduling change that shortens the BASE by k shortens the doubled value by 2k, so this axis has
  twice the leverage here that it would have on an undoubled pseudo.

## [s6] The cam(74) and const(76) live-length surplus (66 and 62 against copy_end's 38) is program liveness that C statement placement can shorten by ~28 insns (the s5 'Route B' frontier).
- mechanism: s5 modelled the residual as a live-length chain L(buf2) < L(index) < L(cam) <= L(const) <= L(copy_end) and proposed reaching it by shrinking cam and const with ordinary C (recompute rather than hold, re-associate, sink the def past the first call).
- probe: Segmented the BB2_FLOW_DEBUG per-insn liveness data per function (new tmp/grind/special_camera_get_rot_dir/s6/isolate.py rewrites the preprocessed TU down to a single function body, which the s5 ledger flagged as the unsolved blocker), then compared raw flow counts against the instrumented-cc1 ALLOCDBG livelens.
- result: Raw flow counts are 72:45 73:39 74:38 76:36 77:38 78:37 and every pseudo maps to its final livelen at ~0.86x EXCEPT 74 and 76, which map at exactly 2x0.86. Their TRUE liveness is 36 and 37 — already inside the target window. The surplus is not liveness at all: it is a x2 applied after flow by tools/gcc-2.7.2/local-alloc.c:1064 to any pseudo carrying a REG_EQUIV note. The .lreg dump confirms exactly two REG_EQUIV notes in this function: (symbol_ref "SpecialCam") and (const_int 128).
- verdict: KILLED

## [s6] The residual is a single binary condition: with the REG_EQUIV x2 removed from pseudos 74 and 76, the def-order chassis reaches the target register assignment.
- mechanism: global.c assigns purely by allocno priority (floor_log2(n)*n*10000/L, tie -> lower pseudo number) and find_reg takes the lowest available hard reg, so with all five callee-saved allocnos at nrefs=3 the target permutation is exactly one total order on live lengths. Undoubled base live length is a strict function of definition order (earliest def = longest).
- probe: Built the assignment order copy_end, constant_80, cam_base, index(jal), buf2_ptr (tmp/grind/.../s6/v_ce_const_cam.c, banked as rejected/routeC-seed-deforder-basewindow-score16.c), read its exact allocno table with the new s6/alloc.sh, extracted an exact ra_solver model (s6/cecc.model.json) and ran s6/sim_undouble.py with the doubling reverted.
- result: The chassis produces base lengths copy_end 38 / const 37 / cam 36 / index 32 / buf2 31 — EXACTLY the chain the target needs — with 74 and 76 doubled to 72 and 74. The simulator returns {72 s0, 78 s1, 73 s2, 77 s3, 74 s4, 76 s5} as measured for the doubled baseline, and {72 s0, 78 s1, 73 s2, 74 s3, 76 s4, 77 s5} = the TARGET assignment when 74 and 76 are set to 36 and 37. The tolerance is 28 distinct (L(cam), L(const)) pairs: any L(cam) in [32,38] with L(const) in [L(cam),38] at copy_end L38.
- verdict: CONFIRMED

## [s6] The REG_EQUIV doubling can actually be removed from these two pseudos by C-level means.
- mechanism: update_equiv_regs (local-alloc.c ~1015-1030) bails out with continue before granting REG_EQUIV whenever reg_n_sets[regno] != 1, and the x2 at local-alloc.c:1064 is gated on that note. cse.c:6923-6934 attaches the enabling REG_EQUAL note to EVERY single-SET constant store to a REG unconditionally, so a second SET is the only C-visible escape.
- probe: rejected/twoset-kills-regequiv-plus2insns.c — a second assignment of cam_base and constant_80 placed after the copy block, where cse's extended-basic-block table cannot prove the store redundant. Measured with s6/alloc.sh plus an insn count off the emitted cc1 asm.
- result: Both doublings die exactly as predicted: pseudo 74 goes nrefs 3->4 livelen 72->31 and pseudo 76 stays nrefs 3 with livelen 74->31. Cost: both re-materialisations survive to the output (la $16,SpecialCam and li $18,0x80 at insns 78-79 of 83), so this spelling is 74 insns against the target's 72 and cannot match. The lever is real and reproducible; only this spelling is priced out.
- verdict: CONFIRMED

## [s6] The alternative world in which cam and const stay doubled is reachable by lifting copy_end above them (any residual form of Route A).
- mechanism: The chain would then need L(copy_end) >= 2*base(const) >= ~62; copy_end could only get there by being doubled itself.
- probe: Bounded the maximum achievable live length (pseudo 72 is live from function entry and measures 38; the ceiling is the ~45 insns visited in the final propagate_block pass), then checked both REG_EQUIV grant paths against copy_end's RTL.
- result: FORECLOSED by mechanism. copy_end's set src is (plus (reg 30 $fp) (const_int 80)) — sp-relative, never CONSTANT_P, so the constant path cannot fire; the alternate MEM path (local-alloc.c:1049-1055) requires reg_basic_block[regno] >= 0, i.e. use confined to one basic block, which copy_end fails. Route A and A' are now killed for a second, stronger reason and the all-undoubled solution is the ONLY surviving one.
- verdict: KILLED

## [s6] Spelling the retry loop as a real C loop (for (;;) with continue), which reproduces the target's two-back-edges-to-the-header shape, improves the permutation.
- mechanism: The target's control flow is one loop whose header receives both back edges; a structured C loop was never tried in five sessions.
- probe: rejected/forloop-continue-loopdepth-worse.c measured with s6/alloc.sh.
- result: Strictly worse. The front-end loop note raises loop_depth over the whole body and reg_n_refs is loop-depth weighted (flow.c:2081), so every count rises (buf2 5, index 5, cam 5, const 5, copy_end 4) while the doubling persists (74 L72, 76 L74) and the permutation is unchanged. The goto-retry spelling is the correct chassis; do not re-try the structured loop.
- verdict: KILLED

## [s7] The residual is a reference-count problem, not a live-length problem: making the retry a real C loop while spelling the inner copy loop with a backward goto lands the full target register permutation.
- mechanism: global.c:allocno_compare ranks by floor_log2(n_refs)*n_refs*10000/live_length
  (ties -> lower pseudo) and find_reg takes the lowest free hard register, so with all six
  callee-saved allocnos mutually conflicting the hard-register assignment IS the allocation
  order. flow.c weights each reference by loop_depth, and a goto-spelled loop emits no
  NOTE_INSN_LOOP_BEG and therefore contributes no depth. A real `for(;;)` retry loop lifts
  all five callee-saved pseudos from 3 refs to 5 (and dest from 4 to 7); a goto-spelled
  INNER copy loop keeps copy_end's single use at depth 2 instead of 3, holding it alone at
  3 refs. Its priority collapses to 937, below constant_80's 1315, so it is allocated last
  and takes $s5.
- probe: reproduced allocno_compare in tmp/grind/special_camera_get_rot_dir/s7/manifold.py,
  enumerated the four realistic loop-spelling reference vectors against the live lengths
  measured on the target-emission chassis, found exactly one solution, spelled it
  (tmp/grind/.../s7/p1_loopretry_gotocopy.c) and measured with s6/alloc.sh + sandbox + build.
- result: allocno table came out exactly as predicted (dest 7/38, buf2 5/31, index 5/34,
  cam 5/66, const 5/76, copy_end 3/32) giving order 72,78,73,74,76,77 = the target
  permutation. `sandbox --disable all` = **0** at 72/72 insns; full clean build SHA1 ==
  62efab4f73f992798c43e8c730aa43baa10bb4fa. Zero rules, zero inline asm, zero pins, zero
  volatile, zero dead locals, zero FAKE constructs.
- verdict: CONFIRMED

## [s7] The REG_EQUIV x2 at local-alloc.c:1064 on cam_base and constant_80 must be KEPT, not removed — s6's entire frontier pointed the wrong way.
- mechanism: the doubling takes their live lengths from bases 33/38 to 66/76, which is what
  ranks them BELOW index (34) — exactly where the target wants them ($s3/$s4 vs index's $s2).
  Removing it would put cam at 33 < index at 34 and invert that pair.
- probe: the matching form's measured allocno table carries livelen 66 and 76 for pseudos 74
  and 76, i.e. both REG_EQUIVs alive, and it matches.
- result: all three s6 frontier hypotheses (F1 zero-cost second SET to kill REG_EQUIV, F2
  optimize_reg_copy_1 live-length subtraction, F3 sched1 base shortening) are moot: they all
  aimed at destroying or shrinking a doubling that the match requires.
- verdict: KILLED

## [s7] s6's def-order chassis (rejected/routeC-seed-deforder-basewindow-score16.c) is byte-unreachable regardless of register assignment.
- mechanism: it emits `addu $19,$sp,80` (copy_end) and `la $20,SpecialCam` (cam_base) before
  the `jal func_80036EA8`; the target emits both after it (0x80037380-0x80037388). A register
  permutation cannot reorder instructions, so no allocation makes that chassis match.
- probe: read the cc1 output of both chassis against asm/funcs/special_camera_get_rot_dir.s.
- result: the chassis whose emission order matches the target is
  rejected/topdef-score12-routeA-seed-chassis.c (const, jal/index, cam, copy_end, buf2).
  Every s6 model, simulation and "28-pair tolerance window" was computed on the wrong
  chassis, and s6's pseudo->variable map was transposed on top of that.
- verdict: KILLED

## [s7] Route R3 (a pre-RA scheduling difference moving live lengths without moving a C statement) is closed.
- mechanism: `inverse_compose.py classify` aligns the honest stream against the target stream
  and types the first divergence. It reported RA — identical instructions, register
  substitutions only — which means neither sched pass contributes to the residual.
- probe: `python3 tools/ra_solver/inverse_compose.py classify code6cac_b2_post special_camera_get_rot_dir`.
- result: FIRST DIVERGENCE: RA. No sched_solver run needed; the axis is not live.
- verdict: KILLED
