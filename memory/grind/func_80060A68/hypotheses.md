# Hypothesis ledger — func_80060A68

## Frontier after s1 (floor 2, candidate.c banked)

The residual is ONE adjacent swap of two independent, equal-priority (both 3) loads:
stage load `lw $5,16($3)` (insn 25) must move from slot 11 to slot 12, past copy2's
address load `lw $4,12($3)` (insn 35). Everything else — all 64 other instructions,
every register, the frame — matches.

### H1 (primary): change insn 35's readiness/hazard path so it schedules at backward
cycle T-47 instead of T-45.
Mechanism: 35 currently jumps ahead via the stall-filler launch ("launching 35 before
32 with no stalls") triggered by copy1-store's stall window; 25 is plain-class and
places only when alone-ready. Honest levers to measure: respell copy1 so its
store/data-load chain has no stall at that cycle (e.g. split copy1's data load into a
named temp; or reorder copy1/copy2 data-vs-addr chains); respell copy2's address read
(named local reused for copy2 only — but beware: target reloads 12($3) three times,
so a SHARED local is wrong; a per-statement fresh temp keeps the reload).
Probe: sandbox-measure each respelling; check the .sched trace for whether 35 still
launches before 32.

### H2: permuter campaign from the floor-2 base.
The base is an excellent seed (score 2, pure ordering residual). Directed PERM on the
copy-block statement forms + temp declaration order. Run via tools/permuter_campaign.py
with the single-function target per [[difficult-is-not-impossible]] §3 (build
target.o from asm/funcs/func_80060A68.s so the function sits at offset 0).
Closing forms are PROPOSALS — vet against the checklist before banking.

### H3 (only if H1/H2 measure dead): re-read [[sched-rank-class-tie-wall]] and read
rank_for_schedule (tools/gcc-2.7.2/sched.c:2399-2456) against insns 25/35's actual
dependence classes to determine whether this tie is structurally closed for pure C.
If the wall rule's closure argument applies verbatim, the ledger must say so with the
sched.c line-level reasoning before anyone claims exhaustion — the residual is 2, and
2026-05-12's "plateau" claim on this function already died once this session.

## Killed this session (do not re-derive)
- K1: caching the dispatch index in a shared local (old body) — CSE holds the
  D_800F10D0 address in a register; 39-level wall. Killed by v2 (fresh re-read).
- K2: fresh single-set pointer local (`p10`) in ANY source position — load lands
  slot 5 (before the sw $0 fence) or slot 23/26 (after); never slot 12. Killed by
  v3/v4/v5/v6 grid.
- K3: `u16` typed dispatch-index local — emits a redundant `andi` target lacks.
  Killed by v2→v3 (s32 fixes it).
- K4: gp-store `D_800A3478 =` written between the two u16 copy statements with
  inline reads — store-order fence pins read2's load below the gp store; target
  needs it above. Killed by v8; the split-read (temp2) is the required shape.
- K5: staging the 0x10 pointer through `idx` — schedules early but allocates $6,
  cascading renames (score 7). The stage variable must be temp_a1 (target's
  `lhu $5,4($5)` self-overwrite). Killed by v10.
- K6: source-position of the temp_a1 stage statement (before copy1 / after copy1 /
  after copy2) — all three produce IDENTICAL emit. Killed by v11-v13.

## [s1] The 2026-05-12 'RA/scheduling plateau' was actually CSE holding the D_800F10D0 address across the body; a fresh re-read of *(u16*)outer (sibling func_80060B70's idiom) collapses it
- mechanism: cse.c address reuse vs target's per-use lui/addu/%lo rematerialization
- probe: v2 rewrite: drop $3 pin + _frame_pad, inline first idx read, fresh second read; sandbox
- result: 39 -> 10; frame 0x20 returned naturally without the pad
- verdict: CONFIRMED

## [s1] Target's store order (sh 0x18 < sw gp D_800A3478 < sh 0x1A) with read2's load ABOVE the gp store forces a split-read temp (read before gp store, store after), same shape as temp_a1/D_800A347C
- mechanism: sched.c write_dependence: gp-symbol store vs base-reg memory refs cannot be disambiguated, so stores emit in source order and later-in-source loads cannot cross them
- probe: v8 (pair between stores, inline read) vs v9 (split-read temp2); sandbox + disasm
- result: v8=10/67; v9=2/65 with the whole middle block byte-matching
- verdict: CONFIRMED

## [s1] The early 0x10($3) pointer load at target slot 12 requires a multi-set stage variable, and the stage variable is temp_a1 itself (target's lhu $5,4($5) self-overwrite)
- mechanism: sched.c load-late launch priority for reg_n_sets==1 destinations ([[staged-value-reused-variable]], sanctioned 2026-07-03); fresh single-set local measured at slot 5 or slot 23/26 in every position, never slot 12
- probe: position grid v3-v6 (single-set) then v10 (stage via idx) then v11 (stage via temp_a1); sandbox + disasm
- result: single-set: 8/10/6 never slot 12; via idx: 7 (wrong reg $6); via temp_a1: 2 with ALL registers matching
- verdict: CONFIRMED

## [s1] Source position of the stage statement moves the contested load past copy2's address load
- mechanism: LUID/source-order tie-break in rank_for_schedule
- probe: three positions (before copy1, after copy1, after copy2), sandbox + disasm each
- result: identical emit all three; both insns priority 3 (equal) in the .sched dump; copy2's load jumps via stall-filler launch, stage load places only when alone-ready
- verdict: KILLED

## [s2] Structural modality — H1 KILLED with a line-level mechanism; new frontier below

### KILLED this session (do NOT re-derive)
- **K7 (this kills all of s1's H1): respelling the 0xC copy triple cannot move the
  contested pair.** Eight distinct spellings measured (reused temp for all three / for
  copy1 only / copy2 only / copy3 only / copy2+copy3; copy1's data load split into a
  named temp; address-and-data both through one reused temp; and the v21 spelling
  crossed with two stage-statement positions). Every spelling that preserved the 66-insn
  shape emitted slots 10/11/12 IDENTICALLY to baseline, or moved the stage load the
  WRONG way (earlier, to slot 10). Mechanism: at sched1 the pair is never decided by
  anything the copy block controls — insn 25 loses every cycle to `LAUNCH_PRIORITY`
  (sched.c:187/4049), and at sched2 it loses the `rank_for_schedule` LUID tie-break
  (sched.c:2464) to an insn chain order sched1 already fixed.
- **K8: the 4th source position for the stage statement (between copy2 and copy3) is
  also byte-identical** (v29). Together with s1's K6 this closes the position axis by
  measurement; the mechanism section of evidence.md [s2] explains why ALL positions must
  be identical, so the axis is closed by argument as well, not just by sampling.
- **K9: the tail-block statement order does not reach the pair.** Moving `idx =
  *(u16*)outer` above the staged `+4` read is byte-identical (v30); moving the
  `*(u16*)(outer+0x1C)` store above the `D_800A347C` gp store costs 3 (v31/v33, the
  s1-K4 store-order fence again).
- **K10 (generalises s1's K2): the staged MULTI-SET variable has the same two-position
  lattice as a fresh single-set local.** Hoisting the stage read above the
  `sw $0,%lo(D_800F10D0)` store (v32) puts the load in slot 5's delay slot (65 insns,
  the nop consumed) while making slots 10-33 byte-identical to target; leaving it below
  the store puts the load at slot 11. The store is the block's only memory fence, so
  those are the only two emitted positions available to pure C. Target's slot 12 is
  neither. s1 had only measured this lattice for the single-set spelling.

### CONFIRMED this session
- **The residual is a `rank_for_schedule` LUID tie-break at sched2 (sched.c:2464),
  seeded by the insn chain order sched1 emitted (28 → 25 → 30 → 35).** Insns 25 and 35
  are structurally identical loads (same mode, same base reg/v 72, same LOG_LINKS
  {9, 22}, same `INSN_PRIORITY == 3`, same dependence class 3 vs `last_scheduled_insn`),
  so priority (sched.c:2418) and class (sched.c:2457) both fall through. Full trace and
  RTL identities in evidence.md [s2]. This also CORRECTS s1's finding-4 insn map: slot
  10 is insn 28 (copy1 addr), not a neighbour of 25.

## Frontier after s2

### H2 (now the primary — untouched by s1 and s2): permuter campaign from the floor-2 seed.
Still the best-shaped remaining lever: the residual is a pure two-insn ordering swap with
every register, the frame, and 64 of 66 instructions already correct. Run
`tools/permuter_campaign.py` against a single-function target.o built from
`asm/funcs/func_80060A68.s` (offset-0 discipline), seeded with candidate.c, honouring the
fresh-seed stopping rule ([[permuter-fresh-seed-discipline]]). Any closing form is a
PROPOSAL — vet it against the 6-test checklist before banking, and note that s2's
mechanism analysis predicts a closing form must change **sched1's emitted chain order**
(25 after 35), so a form that only permutes statements is predicted dead and a form that
changes the dependence structure of insn 25 or 35 is the only thing that can work.

### H3: attack insn 25's `INSN_PRIORITY` rather than its position.
s2 showed priority and class both tie at 3. If insn 25's dependence depth could honestly
be made 4 while 35 stays 3, sched.c:2418 decides before the LUID fall-through. Worked
example from the trace: with priority(25)=4 the T-44 ready list becomes {25(4), 35(3),
30(4)} and the class test (30 is data-dependent on `last_scheduled_insn` 32 → class 1;
25 independent → class 3) picks 25 at T-44, giving emission 28/35/30/25 — CLOSE but NOT
target (28/35/25/30). So a naive +1 on 25's depth is predicted to miss; what is needed is
for **30** to win T-44 and **25** to win T-45. Next probe: enumerate honest C shapes that
raise insn 25's LOG_LINKS depth without adding an instruction, and check each against the
predicted pick order before spending a sandbox run.

### H4 (only if H2 and H3 measure dead): the sched.c:2464 closure argument.
s2 has already done most of the work the [[sched-rank-class-tie-wall]] rule asks for — the
tie is localised to one line, both insns are proven structurally identical in every field
`rank_for_schedule` inspects, and the LUID that breaks the tie is proven to be sched1
output rather than source order. What is still MISSING before any exhaustion claim is a
demonstration that no honest C shape can make sched1 emit 25 after 35. Do NOT claim
exhaustion on this function until H2 and H3 are measured — the floor moved 39 → 2 in one
session and both remaining hypotheses are live.

## [s2] Respelling the 0xC copy triple (reused named temps, split data load) changes which of insn 25 (stage load) / insn 35 (copy2 address load) is emitted first.
- mechanism: s1's H1: insn 35 was thought to jump ahead only because copy1-store's stall window opens at T-44, so removing or moving that stall should re-rank the pair.
- probe: v20-v26 and v28 - eight copy-block spellings (all three copies via one reused temp; copy1 only; copy2 only; copy3 only; copy2+copy3; copy1's data load split into a named temp; address and data both through one reused temp; the v21 spelling crossed with a second stage-statement position), each sandbox-measured with --disable all and each disassembled at slots 9-15.
- result: Every spelling that kept the 66-insn shape emitted slots 10/11/12 identically to baseline (insn 28 / insn 25 / insn 35, score 2). v21 and v28 moved the stage load the WRONG way, to slot 10, still score 2. v20 folded two address loads (score 3, 65 insns); v23 destroyed the shape (score 13, 69 insns); v27 scored 4.
- verdict: KILLED

## [s2] The stage statement's source position (the fourth slot, between copy2 and copy3, that s1's K6 never tried) or the tail-block statement order can move the contested pair.
- mechanism: LUID / source-order tie-break in rank_for_schedule.
- probe: v29 (stage statement between copy2 and copy3), v30 (idx read hoisted above the staged +4 read), v31 and v33 (the 0x1C store moved above the D_800A347C gp store); sandbox plus disassembly for each.
- result: v29 and v30 are byte-identical to baseline (score 2, 66 insns). v31 and v33 regress to 5, which is the s1-K4 store-order fence again. Source position provably cannot matter here: the sched1 trace shows LUID is never consulted for this pair (see the next hypothesis), so K6 generalises from three sampled positions to every position.
- verdict: KILLED

## [s2] The residual pair is decided by rank_for_schedule's final LUID fall-through at the post-reload scheduler, and the LUID it uses is sched1's emitted chain order rather than source order - which is why no source-level reordering reaches it.
- mechanism: tools/gcc-2.7.2/sched.c:2464, 'return INSN_LUID (tmp) - INSN_LUID (tmp2);', reached only after the INSN_PRIORITY test (sched.c:2418) and the dependence-class test (sched.c:2457) both tie. GCC 2.7.2 schedules each basic block BACKWARD, so the later-picked insn is emitted earlier.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) run with -da plus BB2_SCHED_DEBUG and BB2_RANK_DEBUG; read the ready-list trace and the post-schedule RTL out of tmp/grind/func_80060A68/s2/dumps/text1b.sched and text1b.sched2, then read rank_for_schedule (sched.c:2408-2465) line by line.
- result: CONFIRMED. sched2 T-44 reads 'ready list at T-44: 25 (3) 35 (3) 30 (4), now 30 35 25' and T-45 reads 'ready list at T-45: 35 (3) 25 (3), now 35 25'. Insns 25 and 35 are structurally identical loads - same mode, same base register (reg/v 72), same LOG_LINKS {insn 9, insn 22}, same INSN_PRIORITY 3, same dependence class 3 relative to last_scheduled_insn 30 - so priority and class both fall through and LUID decides. sched1's output chain is 28 -> 25 -> 30 -> 35, so LUID(25) < LUID(35) and sched2 can only ever emit our order. At sched1 the pair is not decided by LUID either: insn 25 becomes ready at T-29 and sits for 18 cycles at honest priority 3 while every competitor arrives from insn_queue displaying 0x7f000001 = LAUNCH_PRIORITY (sched.c:187, assigned at sched.c:4049), which beats priority 3 unconditionally at sched.c:2418.
- verdict: CONFIRMED

## [s2] The staged multi-set variable has a richer emitted-position lattice than the fresh single-set local s1 killed in K2, so hoisting its read above the D_800F10D0 store might land it at target's slot 12.
- mechanism: Removing insn 25's anti-dependence on the 'sw $0,%lo(D_800F10D0)' store frees it to schedule anywhere in the block.
- probe: v32 - stage read hoisted above the D_800F10D0 store; sandbox plus full disassembly of slots 1-34.
- result: KILLED, and informatively. v32 makes slots 10 through 33 BYTE-IDENTICAL to target, but the stage load lands in slot 5 - the load-use delay slot after 'lhu $2,0($3)', where target has a nop - consuming that nop and dropping to 65 insns (score 2). That store is the block's only memory fence, so the stage read has exactly two honest source positions and exactly two emitted slots: above the store gives slot 5, below the store gives slot 11. Target's slot 12 is neither. s1's K2 lattice therefore holds for the multi-set staged spelling as well as the single-set one, which was the untested half.
- verdict: KILLED

## [s3] Structural modality — CLOSED. Floor 2 -> 0.

### CONFIRMED this session
- **H5 (new, and it closed the function): the two-instruction residual is decided by
  `birthing_insn_p`'s `reg_n_sets[regno] == 1` gate on the LAUNCH_PRIORITY bump in
  `adjust_priority` (tools/gcc-2.7.2/sched.c:2504-2535 and 2540-2592), not by the
  `rank_for_schedule` LUID fall-through at sched.c:2464.** An insn that kills no
  register and whose destination pseudo has exactly one set is bumped to
  `0x7f000001` the moment it becomes ready; because blocks are scheduled backward
  and the pick is `ready[0]`, a bumped insn is picked early and therefore EMITTED
  LATE. Copy 2's address load was bumped (single-set temp) and landed at slot 12;
  the staged 0x10 load was not (twice-assigned `temp_a1`) and landed at slot 11.
  Target wants the reverse. The LUID tie-break only decides between insns that are
  all unbumped — which is precisely the state the fix creates.
- **The fix that follows from it (measured, not searched): give every one of the
  three contested loads a multiply-assigned destination, then let source order do
  the ordering.** Copy 1's source pointer and loaded word stage through the
  pre-existing `result` local; copies 2 and 3 share one `src` pointer scratch; the
  0x10 read keeps s1's `temp_a1` staging and moves to just after copy 2 so its LUID
  sits above copy 2's address load. `sandbox --disable all` = **0**, 66/66.

### KILLED this session (do NOT re-derive)
- **K11: the copies-2+3 scratch cannot be a borrow of a pre-existing local.** v44
  (`result` for copy 1 + `idx` for copies 2/3) and v46 (fresh `d1` for copy 1 +
  `idx` for copies 2/3) both score 11 at 67 insns — `idx` is needed in a different
  hard register at that point, so the borrow cascades the allocation. The pairing
  in v46 isolates the failure to the `idx` borrow specifically: copy 1's borrow of
  `result` is free (v45 == v42 == 0).
- **K12: all three copies through one scratch is over-reuse.** v43 (three sets of
  one `cp`) scores 3 at 65 insns — cse folds two of the address loads.
- **K13: sharing the copies scratch with the first 0x10 read** (v41) scores 9 at
  67 insns.
- **K14 (retrospective, and it retires s2's H3 and H4 outright): raising or
  lowering INSN_PRIORITY by deepening dependence chains was never the lever, and
  the sched.c:2464 "tie wall" was never a wall.** s2's H3 (attack insn 25's
  dependence depth) and H4 (write the sched.c:2464 closure argument before claiming
  exhaustion) are both moot: the pair never reaches the LUID comparison at sched1
  at all, because one of the two is carrying LAUNCH_PRIORITY. Had this function
  been escalated on the strength of the s2 analysis it would have been escalated
  wrongly — the durable lesson is that a tie-break line is only the answer once you
  have proved the competitors actually arrive at it with equal priority.

## Frontier after s3
None for the C. The function is bytes-matched in the cheat-invisible sandbox.
What remains is an INTEGRATION step on a surface a grind session may not touch:
`asmfix.txt:109` (delete_between anchored on `^lhu\t\$4,0\(\$3\)$`) and
`asmfix.txt:110` (a 43-instruction `insert_before` that splices the whole target
body) must be retired in the same change as the C, because the matched C's first
body instruction is `lhu $2,0($3)` and the stale anchor would otherwise mis-fire
and duplicate the body in a full build. Operator/driver steps: apply the C (already
in src/text1b.c), `engine retire func_80060A68`, `engine verify-oracle`, then a
fresh layer-2 cheat-reviewer on the C before `queue done`.

## [s3] The two-instruction residual is a rank_for_schedule tie that no pure-C shape can break (the standing s2 reading).
- mechanism: s2 attributed the emitted order to rank_for_schedule's LUID fall-through at sched.c:2464, seeded by the chain order sched1 emitted, and predicted that only a form changing insn 25's or insn 35's dependence structure could close it.
- probe: Regenerated the full cc1 -da dump set for the floor-2 body into tmp/grind/func_80060A68/s3/dumps/, read the sched1 and sched2 ready-list traces for the function block, and then read tools/gcc-2.7.2/sched.c:2490-2600 and 3790-4060 line by line instead of reasoning from the trace alone.
- result: KILLED as an attribution. The LUID fall-through is downstream. The deciding code is birthing_insn_p (sched.c:2504-2535), whose test is 'return (reg_n_sets[i] == 1);', reached from adjust_priority (sched.c:2540-2592); a single-set, non-killing destination is bumped to LAUNCH_PRIORITY 0x7f000001 (sched.c:187, set at sched.c:4049) as soon as the insn becomes ready. Backward scheduling turns that bump into 'emitted late'. Copy 2's address load was bumped and the staged 0x10 load was not, which is the whole two-insn swap.
- verdict: KILLED
## [s3] Removing the birthing_insn_p LAUNCH_PRIORITY bump from copy 2's address load, and placing the staged 0x10 read below it, moves the stage load from slot 11 to target's slot 12.
- mechanism: reg_n_sets[dest] > 1 disables the bump (sched.c:2504-2535); with both loads unbumped the sched2 LUID tie-break (sched.c:2464) orders them by RTL/source position, so putting the staged read after copy 2 gives it the higher LUID.
- probe: v40 - copies 2 and 3 share one two-set 'cp' scratch, staged 0x10 read moved to just after copy 2; sandbox --disable all plus disassembly of slots 9-16.
- result: CONFIRMED, exactly as predicted before the run. Slot 12 becomes 'lw a1,16(v1)' (target) and slots 13-16 match target as well. Score stays 2 with 66 insns because a NEW adjacent swap appears at slots 10/11 - copy 1's address load is still a single-set destination and is still bumped.
- verdict: CONFIRMED
## [s3] Staging copy 1's source pointer and loaded word through a multiply-assigned local removes the last LAUNCH_PRIORITY bump and closes the function.
- mechanism: same reg_n_sets gate; with copy 1's, copy 2's and the stage load's destinations all multiply-assigned, none of the three is bumped and all three emit in LUID (source) order, which is target's order.
- probe: v42 (fresh two-set local 'd1'), v45 (borrowing the pre-existing 'result' local instead), v47 (v45 renamed 'src' and FAKE-annotated); sandbox --disable all for each, plus v43/v44/v46 as controls.
- result: CONFIRMED. v42, v45 and v47 all print score 0, build 66 / target 66. Controls: v43 (three sets of one scratch) = 3 at 65 insns, cse folds two address loads; v44 and v46 (borrowing 'idx' for copies 2+3) = 11 at 67 insns. v47 is banked as candidate.c and is the body currently in src/text1b.c.
- verdict: CONFIRMED

## [s3-permuter] A permuter campaign from the floor-2 seed closes the last two instructions.
- mechanism: the residual is a pure two-insn ordering swap with every register, the frame, and 64 of 66 instructions already correct, so the ordering search space around the copy block is tiny and a basin search should reach it.
- probe: built and VALIDATED a minimal-TU permuter workspace (tmp/grind/func_80060A68/s3/perm; its extracted function region is byte-identical to the sandbox's full-TU text1b.o region for the same body), then ran campaign s3-min-tu-floor2 with -j 8 --stop-on-zero --stack-diffs from the floor-2 seed, and harvested it with --stop inside the session.
- result: KILLED. 27,212 iterations in 973 s, base score 20, ZERO finds at any score. Random mutation does not reach the closing form, because closing requires a specific reg_n_sets change on one load's destination rather than any statement permutation the randomizer emits.
- verdict: KILLED

## [s3-permuter] Making BOTH contested loads single-set (hence both bumped) lets the LUID tie-break order them by source position.
- mechanism: birthing_insn_p bumps only single-set destinations; if both competitors are bumped they tie at 0x7f000001, so rank_for_schedule falls through to the sched.c:2464 LUID comparison, which is RTL / source order.
- probe: w2 - split the 0x10 staging into a fresh single-set local p10 (temp_a1 then holds only the halfword), leaving copy 2's address load single-set; sandbox --disable all plus a target disassembly diff.
- result: KILLED. Score 5 at 67 insns. Slots 10/11 do become correct, but a bumped insn is picked the instant it becomes READY, not according to its LUID, and the stage load becomes ready ~18 cycles early (s2's trace), so it overshoots to ~slot 22 and costs an extra reload of 0x10 plus a nop.
- verdict: KILLED

## [s3-permuter] Copy 2's address load can be unbumped by a PRE-EXISTING local, so the banned `src` scratch is not necessary.
- mechanism: reg_n_sets[dest] > 1 disables the birthing_insn_p bump. The borrowing local must be allocatable to $a0 at that point, which is the constraint that killed the `idx` borrow (K11) and rules out `result` (pinned to $v0 by the dispatch call's return value). `temp2` satisfies it, because its own later value - the 0x1A halfword - lives in $a0 in target (lhu a0,2(a0) -> sh a0,26(v1)).
- probe: w3 - widen temp2 from u16 to s32 and let it carry copy 2's source pointer before it is rewritten with the halfword; then w4 - w3 plus copy 1 staged through the pre-existing `result` local. Controls: w1 (sibling destination-local idiom) and w5 (copy 2 self-overwriting).
- result: CONFIRMED. w3 = 2 at 66 insns with the stage load at target's slot 12 and a new swap at 10/11, exactly as predicted; w4 = 0, build 66 / target 66. Controls: w1 = 2 (byte-identical to baseline), w5 = 4 (target puts copy 2's loaded word in $v0, not in temp2's $a0).
- verdict: CONFIRMED

## Frontier after s3-permuter
None for the C. The function is bytes-matched in the cheat-invisible sandbox with a
body that contains no new local, no pin, no volatile, no asm and no banned
construct. What remains is an INTEGRATION step on a surface a grind session may not
touch: asmfix.txt:109 (delete_between anchored on the old first body instruction
lhu $4,0($3)) and asmfix.txt:110 (a 43-instruction insert_before that splices the
whole target body) must be retired in the same change as the C, because the matched
C's first body instruction is lhu $2,0($3) and the stale anchor would otherwise
mis-fire and duplicate the body in a full build. Operator / driver steps: apply the
C (already in src/text1b.c), `engine retire func_80060A68`, `engine verify-oracle`,
then a fresh layer-2 cheat-reviewer on the C before `queue done`.

## [s3b] 2026-08-19 — permuter modality, second pass

- **KILLED (definitively, on two chassis): "a permuter campaign from the floor-2
  seed closes the last two instructions."** This was the ledger's headline live
  frontier. It is now measured dead: 27,212 iterations from the floor-2 seed (s3,
  base score 20) and 38,233 iterations from the w3 seed one adjacent swap from the
  match (s3b, base score 10) produced ZERO outputs between them — 65,445
  iterations, two structurally different basins, no find at all. Mechanism for the
  negative result: the residual is decided by GCC 2.7.2's birthing_insn_p
  (sched.c:2504-2535), a per-pseudo reg_n_sets[regno] == 1 predicate. The only C
  edit that moves that predicate is "route this value through a DIFFERENT EXISTING
  local", which is not in the stock permuter's mutation vocabulary — it reshapes
  expressions, splits and merges temporaries and permutes statements, all of which
  either leave the predicate alone or introduce a fresh single-set temporary (the
  wrong direction). Do not spend another campaign on this function; a search-based
  attack here would need a mutation that rebinds values onto the function's
  existing locals.

- **CONFIRMED: temp2 holds an independent, load-bearing job, so the closing borrow
  is not resting on a manufactured pretext.** Probe w6 — the candidate with the
  0x1A halfword read inlined, i.e. temp2 reduced to nothing but copy 2's pointer
  carrier — scores 10 at 68 insns against the candidate's 0 at 66. This is the
  measured answer to bound 2 of staged-value-reused-variable, and it is the reason
  the s3b vet can argue construct (2) on evidence rather than on assertion.

- **OPEN, and the only genuinely undecided thing left — a rules question, not a
  search question.** temp2 is pre-existing with respect to the floor-2 body but was
  introduced by session s1 as the named intermediate that forces the 0x1A halfword
  read above the D_800A3478 store. Bound 2 of staged-value-reused-variable forbids
  "inventing a new variable just to have something to borrow"; temp2 was not
  invented to be borrowed (it predates the borrow, and w6 shows it earns its keep),
  but it was introduced by a grind session rather than shipped in the original
  body. If a reviewer reads bound 2 to exclude any grind-introduced local, then
  every honest closing form for this function is excluded and the correct
  disposition is a ruling. The alternatives are already measured dead: idx as the
  carrier scores 11 at 67 insns (K11, v44, v46); result is unavailable for copies
  2/3 because the dispatch call's return value pins it to the wrong register; the
  both-loads-bumped alternative (w2) scores 5 at 67. There is no fourth
  pre-existing local in this function.
  NEXT PROBE if this returns for another session: none in the measurement space —
  the next move is layer-1's ruling on construct (2), not another probe.

## [s4] 2026-08-19 — forensics modality. The carrier space is now CLOSED by measurement + code reading.

s4 did not look for a new closing form (two are already known and both are BANNED for
this function). It answered the question the ruling actually turns on: **is there any
pure-C shape at all that closes this function without introducing a local, and if not,
exactly why not.** Answer: no, and the "why not" is now attributed to two named GCC
passes rather than argued.

### CONFIRMED (code reading of tools/gcc-2.7.2/sched.c:2504-2535, not inference)
`birthing_insn_p` has THREE exits, not one, and s3 only documented the third:
  1. `if (GET_CODE (pat) == SET && GET_CODE (SET_DEST (pat)) == REG)` — a destination
     that is not a bare REG (a SUBREG, i.e. a DImode/paired-mode spelling) never
     reaches the test at all. That escape is the forbidden "DImode chain for
     scheduling" family, so it is not available.
  2. `if (bb_live_regs[offset] & bit)` — the dest must be LIVE at the moment the insn
     becomes ready. It always is: an insn becomes ready only when its consumer has just
     been scheduled, and the emission loop's `attach_deaths_insn` (sched.c:3955) makes
     every register used by the just-scheduled insn live before the next `adjust_priority`
     runs. The only way the bit is clear is that a LATER set of the same pseudo was
     already scheduled — which is the multiply-set case again.
  3. `return (reg_n_sets[i] == 1);` — the documented lever.
  Also note `adjust_priority` only consults `birthing_insn_p` in the `n_deaths == 0`
  arm, and GCC's own comment at sched.c:2551 says REG_DEAD notes are gone by then, so
  the death-count arms are unreachable and cannot be used as an alternative lever.
  CONCLUSION: unbumping copy 2's address load requires `reg_n_sets[dest] > 1`, i.e. a C
  variable assigned more than once. There is no fourth door.

### CONFIRMED — the carrier must satisfy THREE constraints simultaneously
A carrier for copy 2's address load must be (a) multiply-assigned, (b) dead across the
window from copy 2's address load to copy 2's data load (target slots 11-15), and
(c) allocatable to $a0, which is where target holds that pointer.

### KILLED this session — every pre-existing local, measured
The function's pre-existing locals (HEAD body, src/text1b.c:3321 before s1 touched it)
are exactly `outer`, `idx`, `temp_a1`, `result`. `outer` is live for the whole body.
The other three are now each measured as copy-2 carriers:
- **K15: `idx` — fails in global-alloc, not in the scheduler (probe x1, score 9/67).**
  The schedule comes out RIGHT (slots 10-13 = `lw v0,12` / `lw a2,12` / `lw a1,16` /
  `lw a0,12`): copy 2's address load is unbumped and precedes the staged 0x10 read
  exactly as the birthing_insn_p model predicts. The failure is allocation. Merging
  copy 2's pointer into `idx` makes pseudo 73 span the copy block to the call
  ("used 6 times across 17 insns ... dies in 2 places" in text1b.lreg), local-alloc
  declines it, and global.c records `73 conflicts: 73 2 3 4 5 29` — a conflict with
  hard reg 4 = $a0 — so it gets reg 6 = $a2 and the body costs one extra insn. In the
  score-0 candidate the same pseudo 73 is local-alloc'd straight to $a0 (`73 in 4`) and
  the conflicting global pseudo is 74, whose conflict set `74 2 3 5 29` does NOT include
  4. This upgrades s3's K11 from "idx is needed in a different hard register" to a named
  pass and a printed conflict set.
  Variants also killed: x2 (idx additionally carries the top D_800F10D0 index — 11/67),
  x6 (idx carrier with copy 1 left inline — 10/67).
- **K16: `result` — dies on an anti-dependence, score 4/67 (probe x4).** `result` is
  $v0 (the dispatch call's return value coalesces it there, and copy 1's staged word
  is in $v0 too). Copy 2's address load is then NOT HOISTED AT ALL — slots 10-12 emit
  `lw v0,12` / `lw a1,16` / `lw v0,0(v0)` and copy 2's address load appears at slot 14 —
  because copy 1's word is still live in $v0 until the 0x20 store, so the second set of
  `result` cannot cross it. Constraint (b) fails.
- **K17: `temp_a1` — score 7/66 (probe x3).** Its staged 0x10 pointer must stay live from
  target slot 12 to slot 29, so it violates constraint (b) as well.

### The residue — a RULES question, not a search question
Every measured score-0 form (s3's `src`, s3-permuter/s3b's `temp2`) closes the function
by introducing or extending a local that the ORIGINAL body did not have, and both are
now BANNED for this function. s4's measurements show that is not a failure of
imagination: of the four pre-existing locals, one is live throughout and the other three
are each measured non-viable, with the failure mechanism named (global-alloc $a0
conflict for `idx`; $v0 anti-dependence for `result`; live-range overlap for `temp_a1`).
So the closing lever is unique up to naming, and the question of whether it is a
sanctioned family or a cheat cannot be settled by any further measurement. s4 therefore
returns `ruling-request` rather than another respelling.

### Frontier after s4
No measurable frontier remains on the carrier axis; do not re-run carrier probes. If the
ruling comes back "a purpose-introduced multiply-assigned carrier is NOT sanctioned
here", the remaining honest dispositions are the canonical-asm grant path or
owner-accepted-incomplete, and the evidence for that entry is already complete in this
ledger (s2's sched trace, s3's mechanism, s3b's 65,445 permuter iterations across two
chassis with zero finds, s4's carrier-space closure). If it comes back "sanctioned",
memory/grind/func_80060A68/candidate.c is a measured score-0 body on the current chassis
(re-verified twice this session) and the only remaining work is the asmfix.txt:109/110
integration handoff described at the end of the s3 section.

## [s5] 2026-08-19 — forensics modality. Chassis re-measured, ban-free floor established, function disposed.

s5 was dispatched in `forensics` modality after the Judge's 2026-08-19 10:21 ruling
(docs/grind/decisions.md:6622) closed the only known closing lever. It deliberately did
NOT re-run any carrier probe (s4 closed that space) and did not re-open the permuter axis
(65,445 iterations, two chassis, zero finds). What it DID do is the one thing the brief
says must never be inherited: it re-measured the chassis-relative numbers from scratch,
because every banked spelling conclusion in this ledger is chassis-relative.

### CONFIRMED — the chassis is unchanged and the ban-free floor is 2, not 39
- HEAD body (register-pinned `outer` + `volatile s32 _frame_pad[2]`, 2 asmfix rules):
  `sandbox func_80060A68 --disable all` => **score 39, build 64 / target 66**.
- The s1/s2 ban-free floor-2 body re-applied over src/text1b.c:3321 =>
  **score 2, build 66 / target 66**. Re-measured this session; this is now the banked
  contents of candidate.c, replacing the BANNED temp2-dual-role score-0 body (preserved
  at rejected/banned-temp2-dual-role-score0-layer1-and-judge-FAIL.c).
  This matters for the escalation record: the function is parked at an honest distance of
  2 with a body that carries no rule, no pin, no volatile, no asm and no dead local — not
  at HEAD's 39-with-two-asmfix-rules.
- `scan_hand_coded.py --single func_80060A68` re-run this session: **tier=LOW score=1/8**,
  S4 (front loads: 6 loads in an 8-insn window @ insn 9) the only signal; S1/S2/S6 all
  clear. Endgame-lock gate 1 therefore fails on a measurement taken this session, not on a
  quoted one.
- Instrumented-cc1 -da dump set regenerated for the current chassis into
  tmp/grind/func_80060A68/dumps/ (.combine/.cse/.loop/.lreg/.greg/.sched/.sched2/.flow/
  .jump2/.rtl/.dbr) — the pass-attribution substrate behind s2/s3/s4 is reproducible on
  demand and was not disturbed by any chassis drift.

### The disposition, and why it is terminal rather than a pause
Gate 1 (canonical-asm): FAILS — LOW 1/8, measured above; endgame-lock-disposition calls a
LOW tier dispositive for a pure scheduling artifact.
Gate 2 (cited SOTN-master precedent for the closing family): FAILS — the Judge searched
for one and found none; the fresh-AND-multiply-written carrier is an *excluded quadrant*
of the frozen list (staged-value-reused-variable bound 2 forbids inventing a variable to
borrow; the named-intermediate 2026-08-17 clarification admits a fresh local only
once-written/once-read), and scarcity of carriers is explicitly not a licensing condition.
Both gates failing is the owner's pre-decided case, so the 2026-07-27 standing auto-ruling
applies immediately and with no owner wait: **terminal OWNER-ACCEPTED INCOMPLETE**. Entry
filed this session in docs/grind/decisions.md.

### Frontier after s5
None that is a measurement. The C axis is closed by the ruling, not by a lack of ideas;
the search axis is closed by 65,445 zero-find iterations; the carrier axis is closed by
s4's four named-mechanism failures; canonical-asm is refused by the scanner. The only
thing that could ever reopen this function is a RULES change — specifically, an owner
extension of the frozen list to admit a purpose-introduced multiply-assigned carrier. If
that ever happens, candidate.c's header records the exact closing body (now in rejected/)
and the two-line asmfix.txt:109/110 integration step it needs.

## [s6] 2026-08-19 — forensics. s5's "the carrier axis is closed" was PREMATURE.

s5 concluded the function was terminal on the strength of s4's four named-mechanism
carrier failures (`idx`, `result`, `temp_a1`, `outer`). That enumeration was over
CARRIERS, not over the function's whole job/local assignment, and it silently assumed
every other local keeps the job it happens to have in candidate.c. Once the assignment is
enumerated as a partition — seven jobs from target's own register flow, five locals, two
forced pairings, three jobs left for two locals, therefore exactly three possible sharings
— one seat had never been measured. It measures score 0. Details and the full table in
evidence.md [s6]; the body is
memory/grind/func_80060A68/ruling-y3-role-permutation-score0.c.

**H-s6-1 — CONFIRMED.** The carrier's failure mode is register allocation, not
scheduling, and it is decided by the local_alloc/global_alloc split.
  mechanism: a pseudo whose live range is confined to the copy block is taken by
  local-alloc, which hands out $a0 to five disjoint short pseudos in this function; a
  pseudo whose range reaches the late-index job (which itself needs $a0 as the call's
  arg 0) is deferred to global_alloc, whose conflict set for it already contains 4.
  probe: y1 vs y2 — one line of C apart. y2 (carrier also holds the late index) → pseudo
  73 to global_alloc, `;; 73 conflicts: 73 2 3 4 5 29`, $a2, score 11/67. y1 (late index
  moved off the carrier) → `73 in 4`, $a0, conflict set loses 4, score 8/66.

**H-s6-2 — KILLED.** The {c,f} sharing (one local carrying copies 2+3's source pointer
AND the late character index) closes the residual.
  probe: y2 = score 11, 67 insns. Banked as
  rejected/y2-idx-carries-copies23-plus-late-index-range-escapes-global-alloc-score11.c.

**H-s6-3 — KILLED.** Moving the late index onto `result` (so the carrier is free) is a
net win.
  probe: y1 = score 8, 66 insns. The carrier does get $a0, but `result` then spans copy
  1's staging + the late index + the call return and loses $v0 to $a2 (`76 in 6`). Banked
  as rejected/y1-late-index-in-result-carrier-gets-a0-but-result-loses-v0-score8.c.

**H-s6-4 — CONFIRMED (unruled).** The {e,f} sharing — `temp2` holds the 0x1A halfword and
then the late character index (both $a0, adjacent disjoint ranges), which frees `idx` to
carry copies 2 and 3's source pointer — closes the function.
  probe: y3 = **score 0, build 66 / target 66**, plus the `-da` dump set in
  tmp/grind/func_80060A68/s3/f2/y3dump/ showing both carrier pseudos local-allocated to
  hard reg 4. NOT submitted: the `idx` block is banned construct 1 with the identifier
  changed, while nothing in the body is fresh (same five locals, all read). Session
  returns ruling-request.

**H-s6-5 — KILLED (door closed, never re-open).** A birthing insn can be bumped to
something smaller than LAUNCH_PRIORITY, via the SCHED_GROUP_P path at sched.c:4068-4075
which deliberately omits the LAUNCH_PRIORITY assignment.
  probe: read sched.c. SCHED_GROUP_P is set at exactly two sites — sched.c:1856 (the
  `HAVE_cc0` arm, not compiled for MIPS) and sched.c:2168 (the USE chain immediately
  before a CALL_INSN). The contested pair's consumers are ordinary stores, so neither
  applies and `max_priority` is 0x7f000001 for every relevant bump.

### Frontier after s6
1. **The ruling on y3.** If a pure role permutation across the existing locals is an
   acceptable form, the function is DONE at score 0 today and only needs the asmfix.txt
   integration step. If it is a respelling of ban 1, then the partition argument in
   evidence.md [s6] is complete and the function is genuinely closed on the C axis — but
   that is the Judge's call, not a session's.
2. If y3 is refused, the remaining unexplored direction is not a new carrier but a
   different TARGET register flow: every argument above takes target's assignment of
   values to $v0/$a0/$a1 as given. Nobody has yet asked whether a differently-shaped body
   reaches the same 66 instructions with the contested pair produced by a different pair
   of pseudos entirely.
3. Do NOT re-open: the SCHED_GROUP_P bump path (H-s6-5), the {c,f} sharing (H-s6-2), the
   permuter (65,445 iterations, two chassis, zero finds), canonical-asm (LOW 1/8).

## [s7] 2026-08-19 — forensics. Two hypotheses measured; the function is NOT exhausted.

**H-s7-1 — CONFIRMED.** The residual is decided by the BUMP STATE of the two contested
loads' destination pseudos, and only the all-unbumped cell reproduces target's order; the
identity of the hosting local is downstream of that.
  mechanism: adjust_priority/birthing_insn_p (tools/gcc-2.7.2/sched.c:2504-2592) bumps a
  ready insn to LAUNCH_PRIORITY iff its destination pseudo has reg_n_sets == 1; blocks are
  scheduled backward, so bumped = picked when ready = emitted late, and unbumped = lingers
  in the ready list = emitted early, ordered among themselves by the LUID (source-order)
  fall-through at sched.c:2464.
  probe: completed the 2x2 by measuring the missing cell. z1 = y3 with the stage split
  into two single-set locals (p10 pointer, h4 halfword), everything else identical.
  result: score 9, 66 insns. Copy 2's address load lands at target's slot 11 (unbumped,
  correct) while the bumped stage load drifts to slot ~19. The other three cells were
  already measured: (bumped, bumped) 8-10 in s1 K2, (bumped, unbumped) 2 at baseline,
  (unbumped, unbumped) 0 in v42/v45/v47/y3. Banked as
  rejected/z1-copy2-unbumped-stage-singleset-bumped-stage-drifts-to-slot19-score9.c.
  This retires s6 frontier item 2: no differently-shaped body can reach the same 66
  instructions with a different pair of pseudos, because the requirement is a property of
  the bump state, not of the pseudo.

**H-s7-2 — CONFIRMED (unruled).** `reg_n_sets > 1` on copy 2's address-load destination
can be obtained WITHOUT banned construct 1's redundant reload, by same-variable split-init
accumulation on a pointer that is loaded exactly once.
  mechanism: `cp = *(s32*)(outer+0xC); cp += 4;` is the shape the owner sanctioned
  provisionally on 2026-06-13 ([[split-init-accumulation-sanctioned]], commit ad11a8c8,
  func_80049C24 in this same file). combine folds the `+= 4` into the load displacement so
  the emitted code is unchanged (66 insns, `lw $v0,4($a0)`), but the pseudo still behaves
  as multiply-set for birthing_insn_p - the stale-count behaviour the sanction documents
  for reg_n_refs, observed here on reg_n_sets.
  probe: z2 (split-init block alone on top of candidate.c) = score 2, 66 insns, stage load
  at target slot 12 for the first time, residual now copy1-vs-copy2 address loads. z3 (z2 +
  copy 1 staged through the pre-existing `result`) = **score 0, build 66 / target 66**,
  measured twice, disassembly and -da dump set banked. z6 (control: same `cp` local, split
  line removed, offset folded back into the deref) = score 2, isolating the split-init line
  as the entire lever. z5 (split-init hosted on the pre-existing `idx`) = score 9 / 67
  insns, so a fresh local is required for this seat.
  NOT SUBMITTED: `cp` is a fresh local written twice, which the Judge's 2026-08-19 10:21
  ruling calls an excluded quadrant, and the split-init sanction is not on the frozen SOTN
  family list. Session returns ruling-request.

### Frontier after s7
1. **The ruling on z3's `cp`.** If same-variable split-init accumulation on a
   once-loaded pointer is admissible here, the function is DONE at score 0 today and needs
   only the standing asmfix.txt:109/:110 integration step. If it is refused, then combined
   with H-s7-1 the C axis really is closed - and the closure argument to record is the
   bump-state truth table, not s4's carrier enumeration.
2. If `cp` is refused but copy 1's `result` staging is not, the only unexplored question
   left is whether ANY once-loaded, twice-written spelling of copy 2's pointer exists on a
   PRE-EXISTING local. s7 measured `idx` (score 9, global-alloc $a0 conflict, same failure
   as s4 K15) and s4 measured `result` (taken by copy 1) and `temp_a1` (taken by the
   stage); `temp2` is banned and `outer` is live throughout. That enumeration is complete,
   so the honest answer is probably no - but it has not been written out as a partition
   under the split-init spelling.
3. Do NOT re-open: the carrier-identity enumeration (s4), the SCHED_GROUP_P bump path
   (H-s6-5), the permuter (65,445 iterations, two chassis, zero finds), canonical-asm
   (LOW 1/8), and every cell of the H-s7-1 truth table.

## [s8] 2026-08-19 — forensics. One hypothesis measured and KILLED; the mechanism space is
## now closed at the GCC-source level.

### KILLED this session (do NOT re-derive)

**H-s8-1 — `birthing_insn_p`'s SUBREG exit closes the function without a multiply-assigned
carrier.** Mechanism: sched.c:2510-2517 tests `GET_CODE (SET_DEST (pat)) == REG` BEFORE it
ever reaches `reg_n_sets[i] == 1`, so a load whose destination is only part of a multi-word
pseudo escapes the LAUNCH_PRIORITY bump for a reason that has nothing to do with how many
times the C variable is written — which would sidestep the entire banned carrier axis.
Probe: d1 = candidate.c with copy 2's source pointer hosted in a `long long`
(`cpq = (long long)(u32)*(s32 *)(outer + 0xC); *(s32 *)(outer + 0x24) = *(s32 *)((s32)cpq + 4);`),
measured with `sandbox func_80060A68 --disable all`. Result: **score 31, build 70 /
target 66**. The door fires (tmp/grind/func_80060A68/s3/d1_disasm.txt: copy 2's
`lw a0,12(a2)` is hoisted to slot 10, ahead of the stage load `lw a1,16(a2)` — the exact
reordering the carrier axis was chasing) but the DImode local materialises four
instructions that no pass folds away: `move v0,a0` (low half), `move v1,zero` (high half,
which flow keeps although it is never read), `addiu v0,v0,4`, and a reloaded
`lw a0,12(a2)`. VERDICT: KILLED, and killed on BYTES rather than on rules — every
wide-typed spelling (long long, double, two-word struct or union) pays the identical cost,
because the cost is the multi-word representation that creates the SUBREG.

### CONFIRMED this session

- The chassis is unchanged: candidate.c = score 2, build 66 / target 66, and the residual
  is the same adjacent load swap it has been since s1.
- `birthing_insn_p` has THREE exits, not the single one the ledger recorded through s7.
  Two are now closed by measurement/argument (SUBREG: costs 4 instructions;
  dest-not-live: reachable only for a dead dest, which flow deletes). The third,
  `reg_n_sets[i] == 1`, is the banned carrier axis.

### Frontier after s8

There is no un-measured mechanism cell left. The remaining route to score 0 is known and
reproducible (s7's z3 and s6's y3 both measured score 0, build 66 / target 66) and it is
banned by ruling, not blocked by ignorance: it requires copy 1's and copy 2's address-load
destinations to be multiply-assigned C variables, which the Judge closed on 2026-08-19 at
10:21, 10:48 and 11:07 "under any name, family label, or second-write spelling".
Canonical-asm is refused (`scan_hand_coded --single func_80060A68` = tier LOW 1/8, S4
only). Both endgame-lock gates therefore fail and the owner's 2026-07-27 standing
auto-ruling applies: terminal OWNER-ACCEPTED INCOMPLETE, filed in docs/grind/decisions.md
by this session.

## [s9] 2026-08-19 — forensics. The divergence is RE-ATTRIBUTED from sched1 to **sched2**,
## and the closure argument is now anchored on a different GCC line than the ledger recorded.

*(Driver session index 3 of the current dispatch; ledger-local index s9. The prior session
was DISCARDED by the driver validator — it returned `owner-gated` from `forensics`
modality, which the validator rejects — so nothing it filed in docs/grind/decisions.md
survives, and this session reverted that uncommitted entry. Its MEASUREMENTS, banked in
the [s7]/[s8] sections above and in rejected/, are unaffected and were not re-run.)*

### Chassis re-measured this session
`sandbox func_80060A68 --disable all` with candidate.c applied to src/text1b.c:3321 =
**score 2, build 66 / target 66**. Residual is the same adjacent pair it has been since
s1: target emits `lw v0,12(v1)` / `lw a0,12(v1)` / `lw a1,16(v1)`; we emit
`lw v0,12(v1)` / `lw a1,16(v1)` / `lw a0,12(v1)`.

### CONFIRMED — H-s9-1: the final emission order is decided by **sched2**, not sched1.
Mechanism: `birthing_insn_p` returns 0 immediately when `reload_completed == 1`
(tools/gcc-2.7.2/sched.c:2508-2509), so the LAUNCH_PRIORITY bump the ledger has blamed
since s2 exists ONLY in sched1. sched2 re-runs `schedule_block` on the post-reload chain
with every insn at its natural depth priority, and it is sched2's pick order that reaches
the assembler.
Probe: full instrumented-cc1 `-da` dump set for the candidate.c body
(tmp/grind/func_80060A68/s3/n9/, cc1 = tools/gcc-2.7.2/cc1, BB2_SCHED_DEBUG=1
BB2_RANK_DEBUG=1), reading `.sched` and `.sched2` side by side.
Result, verbatim from the traces:
  * `.sched` (sched1) emits the chain `22, 25, 39, 27, 32, 29, ...`
    (trace_sched.txt: T-49=22, T-48=25, T-47=39, T-46=27, T-45=32; larger T-n = earlier in
    emission). At sched1's T-45 the ready list is `39 (3) 32 (7f000001) 27 (7f000001)` —
    insns 32 and 27 carry the birthing bump, insn 39 does not, so 39 loses every contest
    and is chained near the front.
  * `.sched2` emits `..., 25, 39, 32, 27, ...` — **a different order from sched1's**, and it
    is this order that the assembler gets (it matches the objdump exactly). At sched2's
    T-45 the ready list is `32 (3) 39 (3)`: NO bump anywhere, both insns at natural
    priority 3.
Insn identities (from the .sched RTL; same numbering in both dumps):
  * insn 25 = `(set (reg 83) (mem (plus (reg/v 72) (const_int 12))))` — copy 1's address
    load, `lw v0,12(v1)`.
  * insn 32 = `(set (reg 85) (mem (plus (reg/v 72) (const_int 12))))` — copy 2's address
    load, `lw a0,12(v1)`.
  * insn 39 = `(set (reg/v 75) (mem (plus (reg/v 72) (const_int 16))))` — the staged
    `temp_a1 = *(s32 *)(outer + 0x10)` load, `lw a1,16(v1)`.
  * insn 27 = `(set (reg 84) (mem (reg 83)))` — copy 1's value load, `lw v0,0(v0)`.
VERDICT: CONFIRMED. The ledger's line-level attribution ("rank_for_schedule's LUID
fall-through at sched.c:2464, seeded by sched1's emitted chain") is right about the LINE and
wrong about the PASS: the fall-through that decides the bytes is the one taken in **sched2**,
and sched1's bump reaches it only indirectly, by fixing the INSN_LUID order sched2 inherits.

### CONFIRMED — H-s9-2: at sched2's deciding tie, all three tests above the LUID
### fall-through are exactly tied, and each is structurally pinned.
Mechanism, read off `rank_for_schedule` (sched.c:2408-2465) against the T-45 state:
  1. **INSN_PRIORITY (sched.c:2418).** Both 32 and 39 are priority 3. GCC 2.7.2's
     `priority()` is a BACKWARD depth: `priority(insn) = max over LOG_LINKS producers of
     (priority(producer) + insn_cost(producer, link, insn))`. Insns 25, 32 and 39 have
     IDENTICAL producer sets — `{9, 22}`, i.e. `lw v1,0(gp)` (priority 1) and the
     `sw zero,0(at)` D_800F10D0 store (priority 3) — and the store→load memory link costs
     0 (proved by insn 25 also landing on 3, while insn 27, a load→load address use of
     insn 25, costs 1 and lands on 4). So 25, 32 and 39 are locked together at 3: anything
     that deepens insn 22 lifts all three equally and changes no order.
  2. **Dependence class (sched.c:2426-2444).** `last_scheduled_insn` at T-45 is insn 27
     (it takes T-44 on priority 4). The class test asks whether the candidate appears in
     `LOG_LINKS (27)`; `LOG_LINKS (27) = {22, 25}`. Neither 32 nor 39 is a producer of 27
     — and neither CAN be, because 27 precedes both in the sched1 chain that sched2's
     dependence graph is built over. Both score class 3.
  3. **INSN_LUID (sched.c:2464).** The comparator returns `LUID(y) - LUID(x)`, so under
     qsort the HIGHER LUID sorts to ready[0] and is scheduled first — i.e. emitted LAST.
     The sched1 chain gives LUID(39) < LUID(32), so 32 takes T-45 and is emitted after 39.
     Target needs the opposite.
VERDICT: CONFIRMED by trace. To flip the pair, sched2 needs EITHER priority(39) = 4
(structurally blocked by (1): 39's only reachable deeper producers are insns 27 and 29,
both of which target emits AFTER insn 39, so depending on either would force the wrong
order anyway), OR `LUID(39) > LUID(32)`, i.e. **sched1 must chain insn 32 before insn 39**.

### KILLED — H-s9-3: source-statement position of the staged read can supply that LUID order.
Mechanism under test: if sched1's chain order tracked C statement order, moving
`temp_a1 = *(s32 *)(outer + 0x10);` would move insn 39's LUID and flip the sched2 tie.
Probe: p1 = candidate.c with that statement moved from between copy 2 and copy 3 to
immediately after copy 1 (tmp/grind/func_80060A68/s3/n9/p1_stage_after_copy1.c, banked at
rejected/p1-stage-after-copy1-byte-identical-source-position-inert-score2.c).
Result: **score 2, build 66 / target 66, and the objdump is BYTE-IDENTICAL to candidate.c**
(tmp/grind/func_80060A68/s3/n9/p1_disasm.txt — the only diff against target is still the
same one adjacent pair).
VERDICT: KILLED, and it kills the whole statement-permutation family with a named
mechanism rather than by enumeration. In sched1 insn 39 is the ONLY unbumped insn in its
contest window; a priority-3 insn is picked only when the ready list holds nothing else,
which happens at T-47, so its chain position is set by READINESS, not by LUID and therefore
not by source order. This is the mechanism behind s2's 14 dead structural respellings and
it retires that axis by argument instead of by sampling.

### CONFIRMED — H-s9-4: the "bump the staged load instead" escape fails inside sched2, and
### the reason is now named (it was previously banked only as a score).
Probe: re-ran rejected/split-stage-single-set-bumped-overshoots-slot22-score5.c (a fresh
single-set local `p10` holds the 0x10 pointer, so insn 39's destination is single-set and
DOES take sched1's bump) with the full dump set (tmp/grind/func_80060A68/s3/n9b/).
Result: **score 5, build 67 / target 66**, and the disassembly shows the intended half of
the effect actually landing: slots 10-11 are `lw v0,12(v1)` / `lw a0,12(v1)`, i.e. copy 2's
address load reaches TARGET'S SLOT 11 for the first time. What breaks is the staged load
itself — it lands at slot 25 instead of slot 12, which also strands a load-delay `nop` at
slot 22 that target fills with `lw a0,16(v1)`, hence 67 instructions.
Mechanism (tmp/grind/func_80060A68/s3/n9b/trace_sched2.txt): with the bump, sched1 picks
insn 39 at its first ready cycle and chains it between insns 56 and 58, far from the front.
sched2 then reaches T-30 with insn 39 as the ONLY ready insn — insns 56, 53, 51, 46, 44,
36, 34, 32, 29, 27, 25 and 22 are all still blocked, and the trace shows insn 56 being
launched only once 39 is scheduled. So sched2 has no choice at T-30 and cannot float the
staged load back toward the front the way it does in the unbumped body. The overshoot is a
sched2 READINESS PIN created by sched1's placement, not a sched1 priority artifact.
VERDICT: CONFIRMED (the axis stays dead as spelled, but for a newly named reason, and with
the valuable half-result recorded: the bumped-stage family is the ONLY measured family that
puts copy 2's address load on target's slot 11 without a banned carrier).

### Frontier after s9
1. **Bumped stage + break the sched2 readiness pin.** The bumped-stage body already wins
   slots 10-11 (target) and loses only the staged load's own slot. The pin is
   `sched2 T-30, insn 39 sole ready insn`. Any honest change that leaves insn 39 with a
   live competitor at T-30 lets sched2 sink it back toward slot 12. Next probe: from
   rejected/split-stage-single-set-bumped-overshoots-slot22-score5.c, dump `.greg` and read
   which hard register `p10` gets, and which insn's release the sched2 trace attributes to
   insn 39 at T-30/T-31; then look for a spelling of the 0x1A halfword read that removes
   insn 56's `lw v0,16(v1)` from between insns 53 and 58 (target reuses ONE `lw a0,16(v1)`
   there). This is the first frontier item on this function that is NOT a carrier construct.
2. **priority(39) = 4 in sched2.** Blocked only by the fact that every priority-4-or-deeper
   producer available to insn 39 (insns 27 and 29) is emitted AFTER it in target. Next
   probe: check whether any priority-3 producer can reach insn 39 through a link whose
   `insn_cost` is 1 rather than 0 — i.e. a load→address-use link like 25→27 rather than a
   store→load memory link like 22→39. That needs the staged read's ADDRESS to be a loaded
   value, which the semantics do not obviously allow; confirm or kill in one read of
   `insn_cost` (sched.c) plus one probe.
3. Do NOT re-open: statement permutation (killed above with a mechanism, superseding s2's
   sampling), the DImode/SUBREG door (s8, costs 4 insns), the carrier axis in all three
   partition seats (banned by Judge 2026-08-19 10:21/10:48/11:07), the permuter (65,445
   iterations, two chassis, zero finds), canonical-asm (`scan_hand_coded` LOW 1/8).

## [s3] The final emission order of the two contested loads is decided by sched1's birthing_insn_p LAUNCH_PRIORITY bump, as every ledger section from s2 onward records.
- mechanism: birthing_insn_p (tools/gcc-2.7.2/sched.c:2503-2535) bumps a single-set destination's INSN_PRIORITY to max_priority; the ledger's model is that this pushes copy 2's address load past the staged 0x10 load, with rank_for_schedule's LUID fall-through (sched.c:2464) as the tiebreak inside sched1.
- probe: Full instrumented-cc1 -da dump set for the candidate.c body (cc1 = tools/gcc-2.7.2/cc1, BB2_SCHED_DEBUG=1 BB2_RANK_DEBUG=1) written to tmp/grind/func_80060A68/s3/n9/, then .sched and .sched2 traces read side by side against the objdump.
- result: sched1's post-pass chain is 22, 25, 39, 27, 32, 29 ... while sched2 emits ..., 25, 39, 32, 27, ... and it is sched2's order that matches objdump exactly. birthing_insn_p returns 0 at its first statement once reload_completed == 1 (sched.c:2508-2509), so the bump exists only in sched1: sched1's ready lists carry (7f000001) priorities on insns 25/27/32/34/42/44/49/51/56, sched2's carry only natural depth priorities. The bump therefore reaches the bytes only indirectly, by fixing the INSN_LUID order sched2 inherits.
- verdict: KILLED

## [s3] The byte-deciding comparison is sched2's rank_for_schedule at T-45 between insn 32 (copy 2's address load, lw a0,12(v1)) and insn 39 (the staged temp_a1 = *(s32 *)(outer+0x10) load, lw a1,16(v1)), and all three tests above the LUID fall-through are exactly tied and structurally pinned.
- mechanism: sched.c:2418 compares INSN_PRIORITY; sched.c:2426-2444 compares dependence class against last_scheduled_insn; sched.c:2464 falls through to INSN_LUID, whose comparator returns LUID(y)-LUID(x) so the HIGHER LUID sorts to ready[0], is scheduled first in the backward pass, and is therefore emitted LAST.
- probe: Read the sched2 trace state at T-45 (tmp/grind/func_80060A68/s3/n9/trace_sched2.txt) plus the post-sched RTL for insns 25/27/29/32/39, and calibrated GCC 2.7.2's priority() as a backward depth against the block's own numbers.
- result: At sched2 T-45 the ready list is '32 (3) 39 (3)' with last_scheduled_insn = insn 27. (1) INSN_PRIORITY 3 == 3: insns 25, 32 and 39 have IDENTICAL producer sets {9 = lw v1,0(gp) at priority 1, 22 = sw zero,0(at) at priority 3} and the store-to-load memory link costs 0 (calibrated by insn 27, a load-to-address-use of insn 25, costing 1 and landing on 4), so they are locked together and deepening insn 22 lifts all three equally. (2) class 3 == 3: LOG_LINKS(27) = {22, 25} contains neither candidate, and neither can, since 27 precedes both in the chain sched2's dependence graph is built over. (3) LUID(39) < LUID(32) from the sched1 chain, so 32 takes T-45 and is emitted after 39. Flipping the pair requires priority(39) = 4 -- blocked, because the only priority-4-or-deeper producers reachable by insn 39 are insns 27 and 29, both emitted AFTER it in target -- or LUID(39) > LUID(32), i.e. sched1 must chain insn 32 before insn 39.
- verdict: CONFIRMED

## [s3] Source-statement position of the staged read can supply the LUID order sched2 needs.
- mechanism: If sched1's chain order tracked C statement order, moving the staged read would move insn 39's LUID and flip sched2's T-45 tie.
- probe: p1 = candidate.c with 'temp_a1 = *(s32 *)(outer + 0x10);' moved from between copy 2 and copy 3 to immediately after copy 1; measured with sandbox func_80060A68 --disable all and diffed against target (tmp/grind/func_80060A68/s3/n9/p1_stage_after_copy1.c, n9/p1_disasm.txt).
- result: score 2, build 66 / target 66, and the objdump is BYTE-IDENTICAL to candidate.c -- the only diff against target is still the same adjacent pair. Mechanism: in sched1 insn 39 is the only UNBUMPED insn in its contest window, and a priority-3 insn is picked only when the ready list is otherwise empty (T-47), so its chain position is fixed by READINESS, not by LUID and therefore not by source order. This kills the whole statement-permutation family with a named mechanism and supersedes s2's 14-respelling sampling argument.
- verdict: KILLED

## [s3] The 'bump the staged load instead of unbumping copy 2' escape fails for a sched1 priority reason (the ledger banked it only as a score: 'overshoots to slot 22, score 5').
- mechanism: Making the 0x10 pointer live in a fresh single-set local gives insn 39's destination reg_n_sets == 1, so it takes sched1's LAUNCH_PRIORITY bump; the ledger's model was that the bump alone carries it too far.
- probe: Re-ran rejected/split-stage-single-set-bumped-overshoots-slot22-score5.c with the full dump set (tmp/grind/func_80060A68/s3/n9b/) and read sched2's trace.
- result: score 5, build 67 / target 66 -- but slots 10-11 are exactly target's 'lw v0,12(v1)' / 'lw a0,12(v1)', the FIRST measured body on this function to place copy 2's address load correctly without a multiply-assigned carrier. The failure is the staged load's own slot (25 instead of 12), which also strands a load-delay nop at slot 22 that target fills with 'lw a0,16(v1)'. Named mechanism: sched2 reaches T-30 with insn 39 as the SOLE ready insn (';; ready list at T-30: 39 (3), now 39'; insns 56, 53, 51, 46, 44, 36, 34, 32, 29, 27, 25, 22 all still blocked, and insn 56 is launched only after 39 is scheduled). sched1's bump had chained insn 39 between insns 56 and 58 and the post-reload dependence graph pins it there. So the overshoot is a sched2 READINESS pin created by sched1's placement, attackable by changing what sits between insns 53 and 58, not by changing insn 39's priority.
- verdict: CONFIRMED

## [s10] 2026-08-19 — forensics. The s2-s9 residual (the sched2 LUID tie) is GONE from the new candidate; a different 2-instruction residual replaces it.

Two of the three s9 frontier items are resolved this session. Frontier item 2
(`priority(insn 39) = 4`) is KILLED with the compiler's own priority stream. Frontier item
1 (bumped stage + break the sched2 readiness pin) is PARTLY CONFIRMED: the pin is real,
its proximate cause is now named at register-allocation level, and a neighbouring source
position of the same bumped local sidesteps the pin entirely and puts all three contested
loads on target — at the price of a new, different 2-instruction gap.

## [s10] priority(insn 39) can be raised to 4 by giving it a deeper producer, flipping sched2's T-45 rank test above the LUID fall-through.
- mechanism: priority() at tools/gcc-2.7.2/sched.c:1497 is max over LOG_LINKS producers x of (priority(x) + insn_cost(x, link, insn) - 1), so a cost-1 link contributes +0 and a cost-2 (load-result) link contributes +1. rank_for_schedule tests INSN_PRIORITY first (sched.c:2418), so priority(39)=4 against priority(32)=3 wins the tie outright and emits insn 39 at target's slot 12.
- probe: Re-ran the s9 instrumented -da recipe on the floor-2 body with BB2_PRIO_DEBUG=1 added (tmp/grind/func_80060A68/s4/n1/, cc1 = tools/gcc-2.7.2/cc1), split this function's sched1/sched2 debug streams out of the 10 MB stderr by the SCHEDDBG FUNC func=func_80060A68 pass=N markers (s4/n1/pass1.txt, pass2.txt), and read every PRIODBG line for insns 9, 22, 25, 27, 32, 39.
- result: In sched2, insn 39 has exactly two producers - insn 9 (lw v1,0(gp), priority 1, cost 2, contrib 2) and insn 22 (the sw zero,0(at) D_800F10D0 store, priority 3, cost 1, contrib 3) - so SET insn=39 final_pri=3. Raising it needs a cost-2 link from a priority-3 producer or a cost-1 link from a priority-4 producer. Every producer must be emitted BEFORE insn 39, and target emits insn 39 at slot 12, so the producer pool is exactly target slots 1-11, whose priorities are 1, 2, 3, 3, 3, 3 - there is NO priority-4 insn in the pool, killing the cost-1 route. The cost-2 route needs insn 39's address operand to be the RESULT of insn 25 or insn 32, the only priority-3 loads in the pool; both load *(s32 *)(outer + 0xC) while insn 39's address is outer + 0x10, and no honest C form makes one the other. Deepening the shared producer insn 22 lifts insns 25, 32 and 39 by exactly the same amount (all three take their max from an identical pred=22 cost=1 link), so the tie survives it.
- verdict: KILLED

## [s10] The sched2 T-30 readiness pin that overshoots the bumped staged load to slot 25 is caused by what sits between insns 53 and 58, and is attackable by respelling the 0x1A halfword read.
- mechanism: s9 recorded that with a bumped (single-set) p10 the staged load reaches target slots 10-11 but overshoots its own slot, because sched2 reaches T-30 with insn 39 as the sole ready insn. The s9 next-probe was to remove insn 56 (lw v0,16(v1)) from between insns 53 and 58.
- probe: Read the two sched2 traces side by side at T-28..T-33 (s3/n9/trace_sched2.txt = prior staged body, works; s3/n9b/trace_sched2.txt = the 3-load bumped body, pins), then measured three honest respellings of the 0x1A read on the pinning body: s32 temp2 (u1), s32 temp_a1 (u3), and a fresh single-set s32 q local hosting the 0x1A read's address so it lives in its own pseudo (u2).
- result: The pin is NOT about insn 56 existing; it is about which HARD REGISTER insn 56 gets. In the pinning body reload gives insn 56 the register v0, and insn 53 is sh v0,24(v1), so 53 carries a write-after-read anti-dependence on 56 and sched2 cannot release 53 until 56 is scheduled - T-30 then holds only insn 39 (';; ready list at T-30: 39 (3)') and the scheduler takes it rather than stall. In target and in the prior staged body insn 56 gets a0 (self-overwrite lw a0,16(v1) / lhu a0,2(a0)), 53 IS ready at T-30 (';; ready list at T-30: 39 (3) 53 (8), now 53') and beats insn 39 on priority 8 vs 3. All three respellings scored 5 / 67 insns - none moved insn 56 off v0, because the v0-vs-a0 choice is made in local-alloc/reload, before sched2 runs.
- verdict: CONFIRMED (the pin and its mechanism; the s9 respelling attack on it is dead as spelled, and the axis is re-aimed at register allocation)

## [s10] The bumped single-set staged local's SOURCE POSITION is inert, as it is for the unbumped one (the s9 statement-permutation kill generalises).
- mechanism: s9 killed statement permutation for the UNBUMPED staged load with a named mechanism (an unbumped priority-3 insn is placed by readiness, not by LUID). The same argument would predict a bumped load is placed by its first-consumer position and is likewise source-position-inert.
- probe: Measured all nine source positions of a single-set s32 p10 = *(s32 *)(outer + 0x10); statement (r1..r9, tmp/grind/func_80060A68/s4/), each with sandbox --disable all plus a disassembly diff against target.
- result: FALSE - the position is NOT inert, and the reason is CSE, not scheduling. Positions r2/r3/r4 (p10 above the copy-3 store) keep three independent 16(v1) loads and score 5 / 67; positions r5/r6/r7 (p10 below it) let cse fold one halfword read onto p10 and score 2 / 66; r1 scores 2 / 65 and r8/r9 score 3 / 66. The s9 kill stands for the emitted POSITION of an already-fixed insn, but moving the statement changes WHICH INSNS EXIST, and that is what moves the score.
- verdict: KILLED (as a general claim; the s9 mechanism is unaffected)

## [s10] A single-set p10 placed immediately after the copy-3 store reproduces target's slots 10, 11 and 12 - the residual that every session s1-s9 has been fighting.
- mechanism: p10 single-set means its load (insn 39) takes sched1's birthing_insn_p LAUNCH_PRIORITY bump and is chained late, supplying LUID(39) > LUID(32) so sched2's T-45 rank falls through to LUID in target's direction; and because no store separates p10's load from the 0x18 read, cse folds that read onto p10, giving insn 39 an early consumer at slot 22 that keeps sched1 from chaining it out past insn 56.
- probe: r5 = candidate body with the staged temp_a1 replaced by a single-set s32 p10 placed after the *(s32 *)(outer + 0x28) copy; sandbox --disable all plus a full disassembly diff against tmp/grind/func_80060A68/s3/tgt.txt.
- result: score 2, build 66 / target 66, and the entire prefix through slot 20 is BYTE-IDENTICAL to target, including slot 10 lw v0,12(v1), slot 11 lw a0,12(v1) and slot 12 lw a1,16(v1). The sched2 rank_for_schedule LUID tie at sched.c:2464 is absent from this body. The remaining 2-instruction gap is elsewhere: the 0x18 halfword read is CSE-folded onto p10's register (lhu v0,0(a1) where target has lw a0,16(v1) + lhu v0,0(a0)), so target's slot-23 load - which also fills the load-delay slot - is missing and shows as a nop. Promoted to candidate.c; it carries no multiply-assigned local of any kind, so it needs no sanctioned-family claim at all.
- verdict: CONFIRMED

## [s10] The three-independent-loads requirement and the unpinned slot-12 placement can be satisfied at the same time.
- mechanism: target has three independent lw ...,16(v1) loads (slots 12, 20, 23) AND places the first of them at slot 12 with its only consumer at slot 29. If a memory store can be made to separate p10's load from the 0x18 read while p10 still lands at slot 12, both halves hold.
- probe: t1 = split copy 3 into a named local so its store falls between the p10 statement and the 0x18 read; t2 = hoist the D_800A3478 gp store above the 0x1A read to break CSE there instead; t5 = write the 0x18 read explicitly through p10. All measured with sandbox --disable all and diffed against target.
- result: t1 restores the three loads and the T-30 pin returns with them, score 5 / 67. t2 scores 8 / 68 - the store-order fence drags the gp store forward out of target's slot 27. t5 is byte-identical to r5 (score 2 / 66), confirming r5's fold is genuine CSE and not a spelling artifact. Together with r1-r9 this traces a conservation law: three independent loads implies p10 has no early consumer implies sched1 chains insn 39 between insns 56 and 58 implies the sched2 T-30 pin; an early consumer unpins insn 39 onto slot 12 but costs one of the three loads.
- verdict: KILLED as spelled (every C-level lever measured; the way out is not source order but the register allocation of insn 56)

### Frontier after s10
1. **Steer insn 56 (the 0x1A read's address load) from v0 to a0 in the 3-load body.** That
   is the ONE thing separating the r4/t1 family (three correct loads, correct CSE
   structure, score 5 only because the sched2 T-30 pin forces insn 39 to slot 25) from a
   byte match. The pin exists solely because insn 53 (`sh v0,24(v1)`) reads v0 and insn 56
   writes it. Next probe: dump `.lreg` and `.greg` for t1 and for the prior staged body,
   find the pseudo that becomes insn 56's destination, and read WHY local-alloc gives it v0
   in one and a0 in the other (conflict set, allocno order, `reg_n_refs`) - then look for an
   honest C change that alters that pseudo's live range rather than its type (the three
   type-level attempts u1/u2/u3 are dead).
2. **Restore the missing third load in the r5 family without re-pinning insn 39.** r5 needs
   `lw a0,16(v1)` where it has a nop, i.e. the 0x18 read must not CSE onto p10 while p10
   keeps an early consumer. Every store-based separator measured (t1, t2) fails. Next probe:
   look for a spelling in which p10's early consumer is NOT one of the three halfword reads
   - e.g. a use of the pointer value that target also performs before slot 25 - or determine
   from `.cse` whether GCC 2.7.2 can be made to treat the two reads as distinct expressions
   without a store between them. If neither exists, KILL the r5 family with that argument.
3. Do NOT re-open: `priority(insn 39) = 4` (killed above from the PRIODBG stream), statement
   permutation of the UNBUMPED staged read (s9), the DImode/SUBREG door (s8), the
   multiply-assigned carrier axis in any of its three partition seats (Judge 2026-08-19
   10:21/10:48/11:07), the permuter (65,445 iterations, two chassis, zero finds),
   canonical-asm (`scan_hand_coded` LOW 1/8).

## [s4] priority(insn 39) can be raised to 4 by giving it a deeper producer, flipping sched2's T-45 rank test above the LUID fall-through (s9 frontier item 2).
- mechanism: priority() at tools/gcc-2.7.2/sched.c:1497 is max over LOG_LINKS producers x of (priority(x) + insn_cost(x, link, insn) - 1), so a cost-1 link contributes +0 and a cost-2 load-result link contributes +1. rank_for_schedule tests INSN_PRIORITY first (sched.c:2418), so priority(39)=4 against priority(32)=3 wins the tie outright and emits insn 39 at target's slot 12.
- probe: Re-ran the s9 instrumented -da recipe on the floor-2 body with BB2_PRIO_DEBUG=1 added (tmp/grind/func_80060A68/s4/n1/, cc1 = tools/gcc-2.7.2/cc1); split this function's sched1/sched2 debug streams out of the 10 MB stderr by the 'SCHEDDBG FUNC func=func_80060A68 pass=N' markers (s4/n1/pass1.txt, pass2.txt) and read every PRIODBG line for insns 9, 22, 25, 27, 32, 39.
- result: In sched2 insn 39 has exactly two producers: insn 9 (lw v1,0(gp), priority 1, cost 2, contrib 2) and insn 22 (the sw zero,0(at) D_800F10D0 store, priority 3, cost 1, contrib 3), so 'PRIODBG SET insn=39 final_pri=3'. Raising it needs a cost-2 link from a priority-3 producer or a cost-1 link from a priority-4 producer. Every producer must be emitted BEFORE insn 39 and target emits insn 39 at slot 12, so the producer pool is exactly target slots 1-11, whose priorities are 1, 2, 3, 3, 3, 3 -- there is no priority-4 insn in the pool, so the cost-1 route is empty. The cost-2 route needs insn 39's address operand to be the RESULT of insn 25 or insn 32, the only priority-3 loads in the pool; both load *(s32 *)(outer + 0xC) while insn 39's address is outer + 0x10. Deepening the shared producer insn 22 lifts insns 25, 32 and 39 by the same amount (all three take their max from an identical pred=22 cost=1 link), so the tie survives it.
- verdict: KILLED

## [s4] The sched2 T-30 readiness pin that overshoots the bumped staged load to slot 25 is caused by what sits between insns 53 and 58, and is attackable by respelling the 0x1A halfword read (s9 frontier item 1).
- mechanism: s9 recorded that with a bumped (single-set) staged local the load reaches target slots 10-11 but overshoots its own slot because sched2 reaches T-30 with insn 39 as the sole ready insn; the s9 next-probe was to remove insn 56 (lw v0,16(v1)) from between insns 53 and 58.
- probe: Read the two sched2 traces side by side at T-28..T-33 (s3/n9/trace_sched2.txt = prior staged body, works; s3/n9b/trace_sched2.txt = the 3-load bumped body, pins), then measured three honest respellings of the 0x1A read on the pinning body: s32 temp2 (u1), s32 temp_a1 (u3), and a fresh single-set s32 q local hosting the 0x1A read's address so it lives in its own pseudo (u2).
- result: The pin is not about insn 56 existing, it is about which HARD REGISTER insn 56 gets. In the pinning body reload gives insn 56 the register v0 and insn 53 is sh v0,24(v1), so 53 carries a write-after-read anti-dependence on 56 and sched2 cannot release 53 until 56 is scheduled; T-30 then holds only insn 39 (';; ready list at T-30: 39 (3)') and the scheduler takes it rather than stall. In target and in the prior staged body insn 56 gets a0 (self-overwrite lw a0,16(v1) / lhu a0,2(a0)), 53 IS ready at T-30 (';; ready list at T-30: 39 (3) 53 (8), now 53') and beats insn 39 on priority 8 vs 3. All three respellings scored 5 / 67 insns; none moved insn 56 off v0, because the choice is made in local-alloc/reload before sched2 runs.
- verdict: CONFIRMED

## [s4] The bumped single-set staged local's SOURCE POSITION is inert, as s9 proved it is for the unbumped one.
- mechanism: s9 killed statement permutation for the unbumped staged load with a named mechanism (an unbumped priority-3 insn is placed by readiness, not by LUID); the same argument would predict a bumped load is placed by its first-consumer position and is likewise source-position-inert.
- probe: Measured all nine source positions of a single-set 's32 p10 = *(s32 *)(outer + 0x10);' statement (r1..r9 in tmp/grind/func_80060A68/s4/), each with sandbox func_80060A68 --disable all plus a disassembly diff against tmp/grind/func_80060A68/s3/tgt.txt.
- result: FALSE, and the reason is CSE rather than scheduling. r2/r3/r4 (p10 above the copy-3 store) keep three independent 16(v1) loads and score 5 / 67; r5/r6/r7 (p10 below it) let cse fold one halfword read onto p10 and score 2 / 66; r1 scores 2 / 65 and r8/r9 score 3 / 66. The s9 kill stands for the emitted position of an already-fixed insn, but moving the statement changes WHICH INSNS EXIST, and that is what moves the score.
- verdict: KILLED

## [s4] A single-set p10 placed immediately after the copy-3 store reproduces target's slots 10, 11 and 12 -- the residual every session s1-s9 has been fighting.
- mechanism: p10 single-set means its load (insn 39) takes sched1's birthing_insn_p LAUNCH_PRIORITY bump and is chained late, supplying LUID(39) > LUID(32) so sched2's T-45 rank falls through to LUID in target's direction; and because no store separates p10's load from the 0x18 read, cse folds that read onto p10, giving insn 39 an early consumer at slot 22 that keeps sched1 from chaining it out past insn 56.
- probe: r5 = the candidate body with the staged temp_a1 replaced by a single-set s32 p10 placed after the *(s32 *)(outer + 0x28) copy; sandbox func_80060A68 --disable all plus a full disassembly diff against target.
- result: score 2, build 66 / target 66, and the entire prefix through slot 20 is byte-identical to target INCLUDING slot 10 lw v0,12(v1), slot 11 lw a0,12(v1) and slot 12 lw a1,16(v1). The sched2 rank_for_schedule LUID tie at sched.c:2464 is absent from this body. The remaining 2-instruction gap is elsewhere: the 0x18 halfword read is CSE-folded onto p10's register (lhu v0,0(a1) where target has lw a0,16(v1) + lhu v0,0(a0)), so target's slot-23 load, which also fills the load-delay slot, is missing and shows as a nop. Promoted to memory/grind/func_80060A68/candidate.c; it carries no multiply-assigned local of any kind.
- verdict: CONFIRMED

## [s4] The three-independent-loads requirement and the unpinned slot-12 placement can be satisfied at the same time.
- mechanism: Target has three independent lw ...,16(v1) loads (slots 12, 20, 23) AND places the first at slot 12 with its only consumer at slot 29; if a memory store can be made to separate p10's load from the 0x18 read while p10 still lands at slot 12, both halves hold.
- probe: t1 = split copy 3 into a named local so its store falls between the p10 statement and the 0x18 read; t2 = hoist the D_800A3478 gp store above the 0x1A read to break CSE there instead; t5 = write the 0x18 read explicitly through p10. All measured with sandbox --disable all and diffed against target.
- result: t1 restores the three loads and the T-30 pin returns with them, score 5 / 67. t2 scores 8 / 68 (the store-order fence drags the gp store forward out of target's slot 27). t5 is byte-identical to r5 at score 2 / 66, confirming r5's fold is genuine CSE and not a spelling artifact. With r1-r9 this traces a conservation law: three independent loads implies p10 has no early consumer implies sched1 chains insn 39 between insns 56 and 58 implies the sched2 T-30 pin; an early consumer unpins insn 39 onto slot 12 but costs one of the three loads.
- verdict: KILLED

## [s5] A fresh m2c decompile yields a structurally different, viable body (no local for the context pointer).
- mechanism: m2c renders every access as M2C_FIELD(D_800A3468, ...), i.e. re-reads the gp-relative global at every use instead of caching it in a local; if GCC 2.7.2 keeps the global in one register across the body, that spelling is equivalent to the `outer` local and its different liveness might move the +2 read's address pseudo off v0.
- probe: tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax on asm/funcs/func_80060A68.s, then four faithful transcriptions with `outer` deleted and `D_800A3468` inlined at every use, in m2c's own order and in the v6/candidate/v2 orders (g1-g4 in tmp/grind/func_80060A68/s5/v/), each measured with sandbox --disable all.
- result: 26 / 68, 26 / 68, 28 / 69, 29 / 70. GCC does NOT keep the global in one register: each interleaved store forces a reload of the gp word, adding 2-4 instructions and wrecking the whole prefix. The identical orderings WITH the `outer` local score 9, 3, 2 and 5. The no-local spelling is a decompiler artefact, not the original source. m2c's statement ORDER, by contrast, matches the campaign's order exactly (and independently corroborates it), except that m2c stores the +2 read directly with no temp2 -- measured as z1 at 9 / 68, so temp2 is load-bearing and honest.
- verdict: KILLED

## [s5] Struct-typed member access changes GCC's alias handling of the inner halfword reads and could break the CSE fold the campaign is fighting.
- mechanism: spelling the reads as (*(struct S3 **)(outer + 0x10))->a sets MEM_IN_STRUCT_P on the halfword MEMs, which GCC 2.7.2's true_dependence / cse invalidation treat differently from plain scalar MEMs; if struct MEMs survive an intervening store the fold pattern changes.
- probe: struct S3 { u16 a, b, c; } declared function-locally; four seats measured (x1 = v6 order with struct source reads; x2 = candidate order with a struct S3 *p10; x3 = both source and destination struct-typed; x4 = the u16* array-index spelling (*(u16 **)(outer + 0x10))[0..2]).
- result: x1 = 6 (the byte-identical raw-cast body v6 = 3), x2 = 2 (neutral), x3 = 8, x4 = 6. MEM_IN_STRUCT_P is not codegen-neutral here, and every seat that moves is a REGRESSION -- it makes GCC more willing to fold, not less. The struct-typing axis is dead in both the source-side and destination-side seats.
- verdict: KILLED

## [s5] Naming the +0 and/or +2 read's address in an explicit local alters that pseudo's live range and can move reload off v0 (s10's frontier item 1).
- mechanism: s10 established that in every 3-load body reload gives the +2 read's address pseudo hard register v0, creating a write-after-read anti-dependence against `sh v0,24(v1)` that forces insn 39 into the sched2 T-30 slot; s10 asked for an honest C change that alters the pseudo's LIVE RANGE rather than its type, since three type-level attempts were already dead.
- probe: four fresh seats on the v2 (3-load) body: r1 = named `s32 p0` for the +0 read's address; r2 = named `s32 p2` for the +2 read's address; r5 = both named; r8 = p10 typed `u16 *` and the +4 read spelled p10[2].
- result: all four score 5 / 67 and are byte-identical to the unnamed v2 body. Declaring a named C local for an address GCC already holds in a single-def/single-use pseudo does not change that pseudo's live range at all -- cse and local-alloc see exactly the same allocno. With s10's three type-level attempts this closes the declaration-level lever on insn 56's destination from seven seats. Anything that moves it must change WHAT ELSE IS LIVE across the +2 read, not how the address is spelled.
- verdict: KILLED

## [s5] In the three-fresh-inline-reads body the third 16(v1) load sits at slot 27 because the D_800A3478 gp store blocks it from hoisting.
- mechanism: a load may not be hoisted above a store whose address GCC cannot disambiguate; `sw v0,%gp_rel(D_800A3478)(gp)` and `lw ?,0x10($v1)` have different base registers, so sched.c must assume they conflict. Sinking the gp store below the +4 read should let the load hoist to target's slot 12.
- probe: w1 = the v6 body with the D_800A3478 assignment moved below the +4 read; also w3, w6 and x5 as three further placements of the same sink.
- result: w1 = 7 / 66. The load moved only from slot 27 to slot 25 -- not to slot 12 -- and sinking the gp store swapped it past `sh a0,0x1A($v1)`, which target emits AFTER it (GCC 2.7.2 never reorders two stores whose bases it cannot disambiguate, so the swap is permanent). w3 = 7 / 67, w6 = 7 / 66, x5 = 7 / 67. The gp store is a real barrier but not the binding one: the +4 load's list-scheduling priority, set by the distance from its sole consumer to the end of the block, is what keeps it low.
- verdict: KILLED

## [s5] Giving p10 an early consumer in a 3-load body yields a body distinct from both candidate.c and v6.
- mechanism: the s4 conservation law says three independent loads implies p10 has no early consumer implies the sched2 T-30 pin; if p10 is read early AND explicitly consumed by the +0 (or +2) read while the other two reads stay fresh in-line, both halves might hold at once.
- probe: q1 (p10 early, consumed by the +0 read; +2 and +4 fresh in-line) and q3 (p10 early, consumed by the +2 read; +0 and +4 fresh in-line), both measured and disassembled.
- result: both score 3 / 66 and are BYTE-IDENTICAL to v6, the body with no p10 local at all. cse propagates p10 forward into whichever read consumes it and then the load schedules with that read, so an early-consumed p10 is not a distinct body -- the local disappears entirely. Two more seats of "give p10 an early consumer" collapse onto the same output. The conservation law survives intact.
- verdict: KILLED

## [s5] The target's C shape can be DERIVED from its own instruction stream via the store-separator law, rather than searched for.
- mechanism: cse.c does not disambiguate two (plus pseudo const) addresses, so any intervening store forces a fresh load, while sched.c DOES disambiguate them (memrefs_conflict_p), so a load may be hoisted across those same stores. That asymmetry means a read of *(s32 *)(outer + 0x10) folds onto the previous one iff and only iff no store separates them in SOURCE order. Reading the target's stream under that law fixes the source.
- probe: apply the law to target's three 0x10 address loads (slots 12, 20, 23) and their consumers (slots 29, 22, 25) together with the fact that GCC never reorders two non-disambiguable stores; cross-check the conclusion by measuring every ordering the law admits (34 bodies, tmp/grind/func_80060A68/s5/v/).
- result: the +0 and +2 reads are separated by `sh v0,0x18(v1)` and are therefore two in-line reads; but the only stores following the +2 load are the gp store (slot 27) and `sh a0,0x1A(v1)` (slot 28), and since stores never reorder, both are also after the +2 read in source -- so NO store is available to separate the +2 read from the +4 read. The +4 pointer therefore cannot be a third in-line read; it must be an early, separately-read local surviving to slot 29. That body is exactly v2, which measures 5 / 67. The target's C is now derived, not guessed, and the entire remaining gap is v2's register allocation.
- verdict: CONFIRMED

## [s5] q5 is a same-floor body whose residual is strictly cleaner than candidate.c's.
- mechanism: candidate.c loses BOTH the slot-23 load and the register of the slot-22 lhu (it reads a1 where target reads a0); a body where p10 feeds the +2 and +4 reads while the +0 read stays fresh should get slot 22 right and isolate the residual to the missing load alone.
- probe: q5 = p10 read before the copy-3 store, +0 read fresh in-line, temp2 = *(u16 *)(p10 + 2), temp_a1 = *(u16 *)(p10 + 4); sandbox --disable all plus a full disassembly diff.
- result: score 2 / 66, same floor as candidate.c, but slot 22 is `lhu v0,0(a0)` -- matching target exactly where candidate.c emits `lhu v0,0(a1)`. The only differing slots are 23 (nop vs `lw a0,0x10(v1)`) and 25 (`lhu a0,2(a1)` vs `lhu a0,0x2(a0)`). Banked at rejected/s5-p10-early-feeds-plus2-and-plus4-plus0-fresh-score2-66insns.c as the better forensic seat for the next .lreg/.greg read, since its single missing insn is the same one v2 has and mis-allocates.
- verdict: CONFIRMED

## [s5] A fresh m2c decompile yields a structurally different, viable body (no local for the context pointer; every access re-reads the gp-relative global D_800A3468).
- mechanism: m2c renders every access as M2C_FIELD(D_800A3468, ...). If GCC 2.7.2 keeps the global in one register across the body that spelling is equivalent to the `outer` local, and its different liveness might move the +2 read's address pseudo off v0.
- probe: tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax on asm/funcs/func_80060A68.s, then four faithful transcriptions with `outer` deleted (g1-g4 in tmp/grind/func_80060A68/s5/v/), in m2c's own order and in the v6 / candidate / v2 orders, each measured with sandbox --disable all.
- result: 26/68, 26/68, 28/69, 29/70. GCC does NOT keep the global in one register: each interleaved store forces a reload of the gp word. The identical orderings WITH the `outer` local score 9, 3, 2 and 5. m2c's statement ORDER independently corroborates the campaign's order, except that m2c has no temp2 (stores the +2 read directly) -- measured as z1 at 9/68, so temp2 is load-bearing and honest.
- verdict: KILLED

## [s5] Struct-typed member access changes GCC's alias handling of the inner halfword reads (MEM_IN_STRUCT_P) and could break the CSE fold the campaign is fighting.
- mechanism: Spelling the reads as (*(struct S3 **)(outer + 0x10))->a sets MEM_IN_STRUCT_P on the halfword MEMs, which GCC 2.7.2's true_dependence / cse invalidation treat differently from plain scalar MEMs.
- probe: Function-local struct S3 { u16 a, b, c; }; four seats: x1 (v6 order, struct source reads), x2 (candidate order, struct S3 *p10), x3 (source AND destination struct-typed), x4 (u16* array-index spelling (*(u16 **)(outer + 0x10))[0..2]).
- result: x1 = 6 where the byte-identical raw-cast body v6 = 3; x2 = 2 (neutral); x3 = 8; x4 = 6. MEM_IN_STRUCT_P is not codegen-neutral here and every seat that moves is a REGRESSION -- it makes GCC more willing to fold, not less.
- verdict: KILLED

## [s5] Naming the +0 and/or +2 read's address in an explicit C local alters that pseudo's live range and can move reload off v0 (s10 frontier item 1).
- mechanism: s10 established that in every 3-load body reload gives the +2 read's address pseudo hard register v0, creating a write-after-read anti-dependence against `sh v0,24(v1)` that forces insn 39 into the sched2 T-30 slot; s10 asked for a change to the pseudo's LIVE RANGE rather than its type.
- probe: Four fresh seats on the v2 (3-load) body: r1 (named s32 p0 for the +0 read's address), r2 (named s32 p2 for the +2 read's address), r5 (both named), r8 (p10 typed u16 * and the +4 read spelled p10[2]).
- result: All four score 5 / 67 and are byte-identical to the unnamed v2 body. Declaring a named local for an address GCC already holds in a single-def/single-use pseudo does not change that pseudo's live range at all. With s10's three type-level attempts this closes the declaration-level lever on insn 56's destination from seven seats.
- verdict: KILLED

## [s5] In the three-fresh-inline-reads body (v6) the third 16(v1) load sits at slot 27 because the D_800A3478 gp store blocks it from hoisting to target's slot 12.
- mechanism: A load may not be hoisted above a store whose address GCC cannot disambiguate; sw v0,%gp_rel(D_800A3478)(gp) and lw ?,0x10($v1) have different base registers, so sched.c must assume they conflict.
- probe: w1 = the v6 body with the D_800A3478 assignment sunk below the +4 read; w3, w6 and x5 as three further placements of the same sink.
- result: w1 = 7/66: the load moved only from slot 27 to slot 25, NOT to slot 12, and sinking the gp store swapped it past `sh a0,0x1A(v1)` permanently (GCC 2.7.2 never reorders two stores whose bases it cannot disambiguate). w3 = 7/67, w6 = 7/66, x5 = 7/67. The gp store is a real barrier but not the binding one -- the +4 load's list-scheduling priority is.
- verdict: KILLED

## [s5] Giving p10 an early consumer while keeping three independent 0x10 loads yields a body distinct from both candidate.c and v6, breaking the s4 conservation law.
- mechanism: The s4 law says three independent loads implies p10 has no early consumer implies the sched2 T-30 pin. If p10 is read early AND explicitly consumed by the +0 (or +2) read while the other two reads stay fresh in-line, both halves might hold at once.
- probe: q1 (p10 early, consumed by the +0 read; +2 and +4 fresh in-line) and q3 (p10 early, consumed by the +2 read; +0 and +4 fresh in-line), measured and disassembled.
- result: Both score 3 / 66 and are BYTE-IDENTICAL to v6, the body with no p10 local at all -- cse propagates p10 forward into whichever read consumes it and the local disappears. Two more seats of 'give p10 an early consumer' collapse onto the same output; the conservation law survives intact.
- verdict: KILLED

## [s5] The target's C shape can be DERIVED from its own instruction stream via the store-separator law, rather than searched for.
- mechanism: cse.c does not disambiguate two (plus pseudo const) addresses, so any intervening store forces a fresh load, while sched.c DOES disambiguate them (memrefs_conflict_p), so a load may be hoisted across those same stores. A read of *(s32 *)(outer + 0x10) therefore folds onto the previous one if and only if no store separates them in SOURCE order.
- probe: Apply the law to target's three 0x10 address loads (slots 12, 20, 23) and their consumers (slots 29, 22, 25), together with the fact that GCC never reorders two non-disambiguable stores; cross-check by measuring every ordering the law admits (34 bodies in tmp/grind/func_80060A68/s5/v/).
- result: The +0 and +2 reads are separated by `sh v0,0x18(v1)` and are two in-line reads; but the only stores after the +2 load are the gp store (slot 27) and `sh a0,0x1A(v1)` (slot 28), and since stores never reorder, both follow the +2 read in source too -- so NO store is available to separate the +2 read from the +4 read. The +4 pointer cannot be a third in-line read; it must be an early, separately-read local surviving to slot 29. That body is exactly v2, which measures 5 / 67. The target's C is derived, not guessed, and the entire remaining gap is v2's register allocation.
- verdict: CONFIRMED

## [s5] q5 is a same-floor body whose residual is strictly cleaner than candidate.c's and is the better forensic seat for the next session.
- mechanism: candidate.c loses BOTH the slot-23 load and the register of the slot-22 lhu (a1 where target reads a0); a body where p10 feeds the +2 and +4 reads while the +0 read stays fresh should get slot 22 right and isolate the residual to the missing load alone.
- probe: q5 = p10 read before the copy-3 store, +0 read fresh in-line, temp2 = *(u16 *)(p10 + 2), temp_a1 = *(u16 *)(p10 + 4); sandbox --disable all plus a full disassembly diff against target.
- result: score 2 / 66, same floor as candidate.c, but slot 22 is `lhu v0,0(a0)` -- matching target exactly where candidate.c emits `lhu v0,0(a1)`. The only differing slots are 23 (nop vs `lw a0,0x10(v1)`) and 25 (register). Banked as the better seat for the next .lreg/.greg read, since its single missing insn is the same one v2 has and mis-allocates.
- verdict: CONFIRMED

## [s6] The target's C shape is FORCED to v2 by cse/sched separator arithmetic -- re-derived independently of s5, including the copy-store partition s5 never tried
- mechanism: cse.c does not disambiguate two `(plus (reg) (const_int))` addresses (so two reads of `*(s32 *)(outer + 0x10)` need an intervening STORE to stay two loads), while sched.c does disambiguate them off a common base (so loads hoist across the copy and halfword stores) but does NOT disambiguate a `(mem (symbol_ref))` store from a `(reg + const)` load (so nothing crosses the gp stores). Target needs three 0x10 loads and therefore two separators; the fixed store order plus "the +2 read must sit above the gp-3478 store" leaves exactly one legal placement.
- probe: enumerate the separator assignments, then measure the one family s5 never tried -- the +0 halfword VALUE hoisted into its own local above the copy-3 store so that copy 3 becomes the +0/+2 separator (x1, x2, x6), plus two store-order variations on it (x7, x8).
- result: x1 = 9/67, x2 = 13/66, x6 = 9/67, x7 = 11/67, x8 = 11/67. Every seat of the alternative partition regresses; the enumeration leaves v2 as the unique shape. CONFIRMED (v2 is the target's C).
- verdict: CONFIRMED

## [s6] Frontier item 2 is impossible: v6's third 0x10 load cannot be hoisted to slot 11 at ANY priority, because a gp-relative symbol_ref store is an absolute alias barrier for sched.c
- mechanism: v6 emits the +4 pointer's load at slot 26, immediately below `sw $v0,%gp_rel(D_800A3478)($gp)` at slot 25. sched.c's memory disambiguation can prove two `(plus (reg) (const_int))` references off the same base do not overlap, but cannot prove a `(mem (symbol_ref "D_800A3478"))` store does not alias a `(plus (reg) (const_int 16))` load, so the dependence is unconditional and no INSN_PRIORITY value can move the load across it.
- probe: re-measure v6 on today's chassis and take a full normalised disassembly diff against target (tmp/grind/func_80060A68/s6/v6.dis vs target.txt).
- result: v6 = 3 / 66. The two streams are identical except that ONE instruction -- the third `lw ?,0x10($v1)` -- sits at slot 26 instead of slot 11, with the intervening slots shifted by one and the consumer reading `$v0` instead of `$a1`. The blocking store is right above it. The v6 family is closed: it is not one hoist short of a priority threshold, it is on the wrong side of an alias barrier.
- verdict: KILLED

## [s6] BOTH of v2's defects (the missing slot-22 load and the $v0-vs-$a0 register) have a SINGLE cause: sched.c's birthing_insn_p LAUNCH_PRIORITY bump on insn 39, whose only C-reachable gate is reg_n_sets[p10] != 1
- mechanism: sched.c:4049 sets the currently-scheduling insn's INSN_PRIORITY to LAUNCH_PRIORITY (0x7f000001, sched.c:187) while it releases its predecessors; adjust_priority (sched.c:2541-2590) then raises every released insn for which birthing_insn_p is true to `max_priority`, which at that instant IS that LAUNCH_PRIORITY. `n_deaths` in adjust_priority is always 0 because REG_DEAD notes are already gone (GCC's own comment, sched.c:2551), so the birthing branch always runs. birthing_insn_p (sched.c:2505-2536) is true iff reload has not completed, the pattern is `(set (reg) ...)`, the dest is live, and `reg_n_sets[dest] == 1`. So every single-set pseudo birth unconditionally outranks every store.
- probe: `pwsh tools/grinder/dump.ps1 func_80060A68` with v2 in src/text1b.c; read text1b.lreg (line 29591) for the pseudo/insn map and text1b.sched (line 34035) for sched1's ready-list trace; then read tools/gcc-2.7.2/sched.c:2505-2590 and :4049 for the gate.
- result: trace shows `ready list at T-31: 39 (7f000001) 53 (8), now 39 53` -- insn 39 (p10's load) takes T-31 on the bump, pushing insn 53 (the 0x18 store, honest priority 8) to T-33 and insn 56 (the +2 address load) to T-32. Because sched.c schedules backward, that emits 53 BEFORE 56, so reg91 (the +0 halfword value, $v0) is dead by the time reg92 (the +2 address) is born and local-alloc re-uses $v0; target has 56 before 53, reg91 still live, and reg92 forced onto $a0. Remove insn 39's bump and T-31 goes to 53 (8 > 3), T-32 to 56, and 39 drifts to an early slot -- which is target's stream exactly. The two "separate" defects are one.
- verdict: CONFIRMED

## [s6] The only C-visible input to that gate is a multiply-written p10, i.e. the construct the Judge already banned -- and its cheapest spelling is DCE'd, so there is no accidental back door
- mechanism: birthing_insn_p's only source-controllable condition is `reg_n_sets[regno] == 1`. Making it 2 requires writing the p10 local twice with both writes live, which is the multiply-assigned pointer-staging carrier banned for this function (Judge, 2026-08-19, "banned ... regardless of which identifier hosts it"). A dead first write does not qualify because flow.c deletes it before sched1 counts sets.
- probe: v2 with `p10 = outer;` inserted before `p10 = *(s32 *)(outer + 0x10);`.
- result: 5 / 67, byte-identical to plain v2 -- the dead store is removed and reg_n_sets stays 1. Banked at rejected/s6-DIAGNOSTIC-dead-first-write-to-p10-is-DCEd-inert-score5.c. This closes the "maybe a harmless second write works" question: only a genuinely live second write moves the gate, and that is the banned shape.
- verdict: CONFIRMED

## [s6] Every single-statement move off the v2 statement order costs points -- v2 is a strict local optimum, and declaration-level dressing on it is inert
- mechanism: n/a (measurement sweep).
- probe: eleven seats on the v2 body -- w1 (+0 value in its own local), w2 (a `u16 *dst` local carrying the 0x18/0x1A/0x1C stores and the gp-3478 value, the sibling func_80060B70 `dst_u16` idiom), w3 (a second early pointer local for the +2 read), w6/w7 (p10 between copy 1 and copy 2 / above the copy triple), w4 (idx above the +4 read), w5 (0x1C store above the gp-347C store), y1 (+4 read below the gp-347C store), y5/y6 (idx read hoisted into the halfword group).
- result: w1, w2, w3, w6, w7 are byte-identical to v2 at 5 / 67 (inert); w4 = 7/67, w5 = 8/67, y1 = 8/68, y5 = 8/67, y6 = 8/67. The sibling `dst_u16` transplant is therefore also dead on the v2 body, not just on candidate.c.
- verdict: KILLED

## [s6] The target's C shape is FORCED to v2 by cse/sched separator arithmetic - re-derived independently of s5, including the copy-store partition s5 never tried.
- mechanism: cse.c does not disambiguate two `(plus (reg) (const_int))` addresses, so two reads of *(s32 *)(outer + 0x10) collapse unless a STORE separates them; sched.c DOES disambiguate them off a common base (loads hoist across the copy and halfword stores) but does NOT disambiguate a `(mem (symbol_ref "D_800A3478"))` store from a `(reg + const)` load, so nothing crosses the gp stores. Target needs three 0x10 loads = two separators; stores never reorder, the +0 read must precede the 0x18 store, and the +2 read must sit above the gp-3478 store. Exactly one separator assignment survives: +4 pointer read first into a local (separated from the +0 read by a copy store), +0/+2 separated by the 0x18 store. That is v2.
- probe: Enumerate separator assignments, then measure the family s5 never tried - the +0 halfword VALUE hoisted into its own `t0` local above the copy-3 store so copy 3 becomes the +0/+2 separator: x1 (p10 above copy2, t0 above copy3), x2 (p10 above copy1, t0 after copy1), x6 (t0 after copy2), x7 (0x1A store hoisted above the gp-3478 store), x8 (idx hoisted above the +4 read).
- result: x1 = 9/67, x2 = 13/66, x6 = 9/67, x7 = 11/67, x8 = 11/67 - every seat of the alternative partition regresses hard. The enumeration plus the measurements leave v2 as the unique legal shape. v2 itself re-measured 5/67 on today's chassis.
- verdict: CONFIRMED

## [s6] Frontier item 2 is impossible: v6's third 0x10 load cannot be hoisted to slot 11 at ANY INSN_PRIORITY, because a gp-relative symbol_ref store is an absolute alias barrier for sched.c.
- mechanism: In v6 the +4 pointer's load sits at slot 26, immediately below `sw $v0,%gp_rel(D_800A3478)($gp)` at slot 25. sched.c's memory disambiguation can prove two (reg + const) references off the same base do not overlap, but cannot prove a (mem (symbol_ref)) store does not alias a (plus (reg) (const_int 16)) load, so the dependence is unconditional and priority is irrelevant.
- probe: Re-measure v6 on today's chassis and take a full normalised disassembly diff against the target stream (tmp/grind/func_80060A68/s6/v6.dis vs target.txt).
- result: v6 = 3/66. The two streams are IDENTICAL except that exactly one instruction - the third `lw ?,0x10($v1)` - sits at slot 26 instead of slot 11, with the intervening slots shifted by one and the consumer reading $v0 instead of $a1. The blocking gp store is directly above it. The v6 family is closed; it is not one hoist short of a priority threshold, it is on the wrong side of an alias barrier. (This is also an independent proof of hypothesis 1: an in-line +4 read is necessarily below the gp store.)
- verdict: KILLED

## [s6] BOTH of v2's remaining defects (the missing slot-22 load and the $v0-vs-$a0 register on the +2 read's address) have a SINGLE cause: sched.c's birthing_insn_p LAUNCH_PRIORITY bump on insn 39, p10's load.
- mechanism: sched.c:4049 temporarily sets the currently-scheduling insn's INSN_PRIORITY to LAUNCH_PRIORITY (0x7f000001, sched.c:187) while it releases predecessors; adjust_priority (sched.c:2541-2590) then raises every released insn for which birthing_insn_p is true to max_priority, which at that instant IS that LAUNCH_PRIORITY. adjust_priority's n_deaths switch is dead code (REG_DEAD notes are already removed - GCC's own comment, sched.c:2551), so the birthing branch always runs. birthing_insn_p (sched.c:2505-2536) is true for any live single-set pseudo birth; its only source-controllable gate is `reg_n_sets[i] == 1`. Every such birth therefore outranks every store unconditionally.
- probe: `pwsh tools/grinder/dump.ps1 func_80060A68` with v2 applied; read text1b.lreg (function at line 29591) for the pseudo/insn map and text1b.sched (function at line 34035) for sched1's ready-list trace; then read tools/gcc-2.7.2/sched.c:2505-2590 and :4049.
- result: Trace: `;; ready list at T-31: 39 (7f000001) 53 (8), now 39 53`, then `;; ready list at T-32: 53 (8) 56 (7f000001), now 56 53`, then `T-33: 53 (8)`. insn 39 = p10's load, 53 = the 0x18 store (honest priority 8), 56 = the +2 read's address load. Because sched.c schedules BACKWARD (larger T = earlier position), 39 taking T-31 emits 53 BEFORE 56 - so reg91 (the +0 halfword value, $v0) is already dead when reg92 (the +2 address) is born and local-alloc re-uses $v0. Target's order is 51,56,53,58: reg91 still live, $v0 unavailable, reg92 forced onto $a0. Strip insn 39's bump and T-31 goes to 53 (8 > 3), T-32 to 56, and 39 drifts to an early slot - target's stream exactly. The two defects are one.
- verdict: CONFIRMED

## [s6] The only C-visible input to that gate is a multiply-WRITTEN p10 (the banned carrier), and its cheapest spelling is dead-code-eliminated, so there is no accidental back door.
- mechanism: birthing_insn_p's only source-controllable condition is reg_n_sets[regno] == 1. Making it 2 requires two LIVE writes of the p10 local - the multiply-assigned pointer-staging carrier the Judge banned for this function on 2026-08-19 ('banned regardless of which identifier hosts it'). A dead first write does not count because flow.c deletes it before sched1 counts sets.
- probe: DIAGNOSTIC ONLY (never proposed as a candidate): v2 with `p10 = outer;` inserted before `p10 = *(s32 *)(outer + 0x10);`.
- result: 5/67, byte-identical to plain v2 - the dead store is removed and reg_n_sets stays 1. Banked at rejected/s6-DIAGNOSTIC-dead-first-write-to-p10-is-DCEd-inert-score5.c. Only a genuinely live second write moves the gate, and that is exactly the banned shape. This also explains why every session since s1 re-converged on that construct: it is not one spelling among many, it is the unique C-visible input to sched.c:2536.
- verdict: CONFIRMED

## [s6] The v2 statement order is a strict local optimum and declaration-level dressing on it - including the sibling func_80060B70 `dst_u16` idiom - is inert.
- mechanism: n/a (measurement sweep).
- probe: Eleven seats on the v2 body: w1 (+0 value in its own local), w2 (`u16 *dst = (u16 *)(outer + 0x18)` carrying the 0x18/0x1A/0x1C stores and the gp-3478 value - the sibling idiom), w3 (a second early pointer local for the +2 read), w6/w7 (p10 between copy 1 and copy 2 / above the copy triple), w4 (idx above the +4 read), w5 (0x1C store above the gp-347C store), y1 (+4 read below the gp-347C store), y5/y6 (idx read hoisted into the halfword group).
- result: w1, w2, w3, w6, w7 are byte-identical to v2 at 5/67 (inert); w4 = 7/67, w5 = 8/67, y1 = 8/68, y5 = 8/67, y6 = 8/67. Every single-statement move off the v2 order costs 2-8 points. The sibling `dst_u16` transplant is dead on the v2 body as well as on candidate.c.
- verdict: KILLED

## [s7] Moving the CONSUMER statement (`temp_a1 = *(u16 *)(p10 + 4)`) rather than the p10 statement finds a strictly better body than v2
- mechanism: sched1 releases p10's load only when its consumer is scheduled, so the consumer's SOURCE position (not p10's) sets the cycle at which the load enters the ready list. No prior session moved the consumer; s6's w-series moved p10 and left the consumer fixed.
- probe: seven consumer seats on the v2 body (ca/cb/cc/cd/v2/ce plus s6's y1), sandbox --disable all each.
- result: ca 8/66, cb **4/66**, cc 7/66, cd 8/67, v2 5/67, ce 7/67, y1 8/68. cb (consumer immediately after the 0x18 store) is the best non-candidate body on record: 66 instructions, all three `lw ?,0x10($v1)` loads present, and a single defect. p10's own position is inert at that seat (cb1/cb2/cb = 4/66; only cb4, which re-enables the cse fold, regresses to 6/66).
- verdict: CONFIRMED

## [s7] cb's single defect is the SAME insn-39 LAUNCH_PRIORITY bump s6 named, and the target's stream geometry proves sched1 cannot produce the target under that bump
- mechanism: sched1 trace for cb: `launching 39 before 59 with no stalls at T-32` / `ready list at T-32: 53 (8) 39 (7f000001), now 39 53`. The bumped load always takes the FIRST cycle at which it is ready, and it becomes ready exactly when its consumer is scheduled — so a bumped single-set pointer load is ALWAYS emitted adjacent to its consumer. Target has p10's load at slot 11 and its consumer at slot 28, seventeen slots apart, which sched1 cannot do. Only sched2 (reload_completed == 1, never bumps, sched.c:2509) can open that gap, and only if the hard register is free across it: target's p10 is `$a1`; cb's is `$v0`, written at slot 20 and read at slot 22, so the hoist is blocked.
- probe: `pwsh tools/grinder/dump.ps1 func_80060A68` with cb applied; text1b.sched line 34035 ff.; normalised disassembly diff tmp/grind/func_80060A68/s7/cb.dis vs target.
- result: cb = 4/66 with a one-slot shift from slot 11 onward; p10's load emitted at slot 23 as `lw $v0,0x10($v1)` (consumer `lhu $a1,0x4($v0)` at slot 25) instead of slot 11 `lw $a1,0x10($v1)`. Confirms the s6 causal chain on an independent, better body, and adds the geometric proof that the target's compile did NOT bump its equivalent of insn 39.
- verdict: CONFIRMED

## [s7] birthing_insn_p's `bb_live_regs` gate (the last unprobed door, s6 frontier item 2) is CLOSED for every C shape
- mechanism: sched.c:2524-2531 returns 0 without consulting reg_n_sets when the destination bit is clear in bb_live_regs. But sched.c schedules BACKWARD: insn 39 is released only after an insn that READS its destination has been scheduled, and that read is precisely what sets the bit. A load whose destination is never live is dead and is removed by flow.c before sched1 counts anything.
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_SCHED_DEBUG=1 over the cb body; harness tmp/grind/func_80060A68/s7/adjpri.sh, log tmp/grind/func_80060A68/s7/adjpri.log; our block located via `SCHEDDBG block=0 n_insns=44 n_ready=1` (log line 55776) cross-checked against the per-insn priority list.
- result: `SCHEDDBG ADJPRI insn=39 deaths=0 birth=1 maxpri=2130706433 pri=3`. birth=1. (The flag is discriminating TU-wide — 3611 birth=0 vs 3009 birth=1 across 6620 records — and insn 53, a store, correctly reports birth=0, so this is a real reading, not a stuck flag.) `reg_n_sets[i] == 1` is now the ONLY C-visible input to the bump, and it is the banned multiply-written carrier.
- verdict: KILLED

## [s7] Declaration order, local types, and non-copy separators are all inert on the 3-load (cb) body
- mechanism: n/a (measurement sweep, closing the sanctioned-dressing families on the new seat).
- probe: cbd1/cbd2/cbd3/cbd4 (four declaration orders incl. p10 first and p10 last), cbt1 (`s32 temp_a1`), cbt2 (`u16 *p10` with `p10[2]`), cbt3 (`s32 temp2`), g1 (`D_800A347C` gp store hoisted between p10 and the +0 read as the cse separator), g2 (same with `D_800A3478`).
- result: cbd1-4 and cbt1-3 are all 4/66, i.e. byte-inert; g1 = 8/68 and g2 = 11/68, because a `(mem (symbol_ref))` store is an absolute scheduling barrier and destroys the whole matched prefix. The sanctioned declaration-order and named-intermediate-type families are therefore measured dead on the 3-load body as well as on candidate.c, and the copy stores are the only usable cse separators.
- verdict: KILLED

## [s7] Moving the CONSUMER statement (temp_a1 = *(u16 *)(p10 + 4)) rather than the p10 statement is an unexplored axis that finds a better body than v2.
- mechanism: sched1 releases p10's load only when its consumer is scheduled, so the CONSUMER's source position - not p10's - sets the cycle at which the load enters the ready list. Every prior session (s6's w-series, s10's r-series) moved p10 and left the consumer fixed.
- probe: Seven consumer seats on the v2 body (ca after the copy triple, cb after the 0x18 store, cc after the +2 read, cd after the D_800A3478 store, v2 after the 0x1A store, ce after the idx read, plus s6's y1 after the D_800A347C store); sandbox func_80060A68 --disable all on each.
- result: ca 8/66, cb 4/66, cc 7/66, cd 8/67, v2 5/67, ce 7/67, y1 8/68. cb is the best non-candidate body on record: 66 instructions (correct count), all three lw ?,0x10($v1) loads present, one defect. p10's own position is inert at that seat (cb1 4/66, cb2 4/66, cb 4/66); only cb4, which re-enables the cse fold of the +0 read, regresses to 6/66.
- verdict: CONFIRMED

## [s7] cb's single defect has the same cause s6 named on v2, and the target's stream geometry proves sched1 cannot have produced the target under the insn-39 LAUNCH_PRIORITY bump.
- mechanism: cb's sched1 trace: 'launching 39 before 59 with no stalls at T-32' then 'ready list at T-32: 53 (8) 39 (7f000001), now 39 53'. A bumped insn always takes the FIRST cycle at which it is ready, and it becomes ready exactly when its consumer is scheduled - so a bumped single-set pointer load is ALWAYS emitted adjacent to its consumer. The target puts p10's load at slot 11 and its consumer at slot 28, seventeen slots apart; only sched2 (reload_completed == 1, sched.c:2509, never bumps) can open that gap, and only when the hard register is free across it - $a1 in the target, $v0 in cb (written slot 20, read slot 22, so the hoist is blocked).
- probe: pwsh tools/grinder/dump.ps1 func_80060A68 with cb applied; read text1b.sched block-0 trace at dump line 34035 ff.; normalised disassembly diff tmp/grind/func_80060A68/s7/cb.dis against the target stream.
- result: cb = 4/66. The stream matches target exactly with a one-slot shift from slot 11 onward: p10's load emitted at slot 23 as lw $v0,0x10($v1) (consumer lhu $a1,0x4($v0) at slot 25) rather than slot 11 lw $a1,0x10($v1). Independent confirmation of the s6 causal chain on a better body, plus a body-independent argument that the target's compile did NOT bump its insn-39 equivalent.
- verdict: CONFIRMED

## [s7] s6 frontier item 2 - birthing_insn_p's bb_live_regs test could clear the bump without a second write - is false for every C shape.
- mechanism: sched.c:2524-2531 returns 0 without consulting reg_n_sets when the destination's bit is clear in bb_live_regs. But sched.c schedules BACKWARD: insn 39 is released only after an insn that READS its destination has been scheduled, and that read is exactly what sets the bit. A load whose destination is never live is a dead load and is deleted by flow.c before sched1 runs.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_SCHED_DEBUG=1 over the cb body via tmp/grind/func_80060A68/s7/adjpri.sh; block located in tmp/grind/func_80060A68/s7/adjpri.log by 'SCHEDDBG block=0 n_insns=44 n_ready=1' (line 55776) cross-checked against the per-insn priority list (insn 9 pri 1, insn 39 pri 3, insn 133 pri 2147483528).
- result: SCHEDDBG ADJPRI insn=39 deaths=0 birth=1 maxpri=2130706433 pri=3. birth=1. The flag is discriminating TU-wide (3611 birth=0 vs 3009 birth=1 across 6620 records) and insn 53, a store, correctly reports birth=0 - so this is a real reading, not a stuck flag. The gate is closed; reg_n_sets[i] == 1 is the sole remaining C-visible input to the bump.
- verdict: KILLED

## [s7] Sanctioned declaration-order / local-type dressing, and non-copy cse separators, might work on the 3-load (cb) body even though they were inert on candidate.c.
- mechanism: n/a - measurement sweep to close the sanctioned-dressing families on the new seat before the next session inherits it.
- probe: cbd1 (p10 declared first), cbd2 (p10 last), cbd3 (p10 second), cbd4 (temp_a1 first, full permutation); cbt1 (s32 temp_a1), cbt2 (u16 *p10 with p10[2] for the +4 read), cbt3 (s32 temp2); g1 (D_800A347C gp store hoisted between p10 and the +0 read as the cse separator), g2 (same with D_800A3478).
- result: cbd1-cbd4 and cbt1-cbt3 are all 4/66 - byte-inert, so pseudo numbering and local typing have no effect on p10's allocation. g1 = 8/68, g2 = 11/68: a (mem (symbol_ref)) store is an absolute scheduling barrier and destroys the matched prefix, so the copy stores remain the only usable cse separators.
- verdict: KILLED

## [s8 2026-08-19 - structural] The residual is NOT gated on birthing_insn_p's reg_n_sets test; it is gated on which HARD REGISTER local-alloc gives p10's pseudo, and that is movable by honest statement order.

- statement: s10's "conservation law" (three independent `lw ?,0x10($v1)` loads XOR an
  un-pinned p10 load) is false, and s6/s7's reduction of the whole residual to
  birthing_insn_p's `reg_n_sets[i] == 1` test is an over-reduction. The C-visible lever that
  actually decides the byte is REGISTER PRESSURE at local-alloc time: whether some pseudo is
  live in $v0 across p10's sched1 live range.
- mechanism: dump-verified this session against tmp/grind/func_80060A68/dumps/ with the cb
  body applied. text1b.lreg line 29591 shows p10 is pseudo 75 (`Register 75 used 2 times
  across 6 insns in block 0`), set by insn 39 `(set (reg/v:SI 75) (mem:SI (plus (reg 72)
  (const_int 16))))` and read by insn 56. text1b.greg line 25167 shows `;; 1 regs to
  allocate: 83` - so pseudo 75 is assigned by LOCAL-ALLOC, not global-alloc - and the
  dispositions line reads `75 in 2`, i.e. $v0. sched1's emitted order around it is
  51, 53, 39, 59, 56, 61, 64; $v0 is killed at insn 53 (`sh $v0,0x18($v1)`, REG_DEAD v0) and
  not re-born until insn 64, so $v0 is free across reg75's entire range and local-alloc's
  lowest-numbered-free-hard-reg scan takes it. In the final schedule $v0 is written at slot 20
  and read at slot 22, straddling slots 12..28, so sched2 cannot hoist the load out of slot 23.
  Keep ANY value live in $v0 across insns 39..56 and reg75 goes to $a1 - which is dead from
  slot 12 to slot 28 - and sched2 hoists the load to slot 12, `lw $a1,0x10($v1)`, exactly as
  in target. birthing_insn_p is not bypassed by any of this: p10 stays single-set, the bump
  still fires, sched1 still chains insn 39 next to insn 56. The bump decides sched1's ORDER;
  the register decides whether sched2 can undo it. They are separable, and s6/s7 fused them.
- probe: (1) read text1b.lreg / text1b.greg for the cb body and identify pseudo 75 and its
  disposition; (2) read sched.c:2505-2593 directly to confirm birthing_insn_p's gates and that
  adjust_priority's n_deaths switch is dead (REG_DEAD notes stripped before sched); (3) sweep
  59 statement orders on the cb and cand bodies through `sandbox --disable all`, disassembling
  each and recording the slot and hard register of every `lw ?,0x10($v1)`.
- result: `lw $a1,0x10($v1)` at target's slot 12 was produced in TWENTY distinct pure-C bodies
  with a single-set p10, by three independent pressure sources:
    (a) hoisting the `idx = *(u16 *)outer;` read above the p10 consumer statement - f3, f4,
        i3, i4, r1 (all 8 / 66) and r3 (10 / 66), r6 (7 / 67);
    (b) sinking copy 3's store below the +2 halfword read - m1 (9 / 66, two loads), n2
        (10 / 67, three loads), o4 / o5 (10 / 66, two loads), o7 (10 / 67, three loads);
    (c) splitting copy 3 into a named `c3` local whose store becomes the p10/0x18 separator -
        u5 (8 / 66, three loads), u9 (10 / 66), ua (8 / 66); and with the gp stores as the
        separator, u1 (8 / 68) and u2 (11 / 68).
  The f3 family is the strongest: three 0x10 loads, 66 instructions, and a prefix
  BYTE-IDENTICAL TO TARGET THROUGH SLOT 22 - eleven slots deeper than cb's, covering the whole
  copy triple. u5 is its complement: slots 26-34 byte-identical to target, including
  `lhu $a1,0x4($a1)` at slot 29 (p10's consumer, seventeen slots after its load, in $a1) and
  the late `lhu $a0,0x0($v1)` idx read at slot 30 - the first time the campaign has produced
  either. Neither beats the floor: the honest floor is unchanged at 2 (candidate.c, re-measured
  this session at score 2 / build 66 / target 66 on today's HEAD).
- verdict: CONFIRMED (the pressure lever exists and is honest); s10's conservation law and the
  s6/s7 "only lever is reg_n_sets" framing are both KILLED.

## [s8 2026-08-19 - structural] What still costs f3 its eight slots is scheduling, not allocation: the hoisted `idx` load takes slot 23, the slot target gives to the third 0x10 load.

- statement: in the f3 family the ONLY remaining defect is that the pressure-supplying
  instruction is itself schedulable into the load-delay slot at 23.
- mechanism: `idx = *(u16 *)outer;` compiles to `lhu $a0,0x0($v1)` and depends on nothing but
  $v1, which is live from slot 0. sched2 therefore has it ready at the cycle that fills the
  load-delay slot after `lhu $v0,0x0($a0)` at slot 22, and it wins that slot over the third
  `lw $a0,0x10($v1)`. Target puts the 0x10 load there and the idx load at slot 30. Everything
  in slots 23..31 is then a permutation of target and the score is 8.
- probe: twelve seats for the idx read on the cb order (i0..i11); re-association of the
  consumer and the +2 read to lower the idx read's LUID below the third load's (r1, r2, r3,
  r4, r5, r6); consumer-position family with both the p10 read and its consumer moved into or
  above the copy triple (ka, kb, kc, kd, ke, kf, kg); hoisting the +0 halfword VALUE into a
  `t0` local with the 0x18 store delayed (e1..e5); sinking copy 3 (m1..m5); `c3` splits
  (o4..o8, u1..u9, ua, ub); p10-position sweep on the cb order (pa, pb, pd, pe); halfword
  group permutations (h1..h8); n1..n6.
- result: KILLED for every source-level respelling tried. LUID re-association does not move the
  idx load (r1 = f3 exactly, 8 / 66). Seats 0 and 1 over-pressure and give p10 $a2 (9 / 66).
  Seats 6 and later drop back to $v0 (cb itself, seat 9, is the 4 / 66 floor seat). Moving the
  p10 consumer into the copy triple (ka 9/66, kb 11/67, kc 10/67, kd 8/66, ke 11/67, kf 10/67,
  kg 8/66) puts the load at slot 10 or 11 but in $a0, because $a0 is also free there -
  pressure has to cover $a0 as well as $v0. Delaying the 0x18 store through a `t0` local
  (e1 8/66, e2 7/65, e3 7/65, e4 7/65, e5 9/65) never reaches $a1 and usually costs a load.
- verdict: KILLED (no source spelling found that supplies the $v0 pressure without emitting an
  extra early-schedulable instruction into slot 23).

## [s8] s6/s7 are wrong that sched.c:2536 `return (reg_n_sets[i] == 1);` is the sole remaining C-visible input to the residual, and s10 is wrong that three independent `lw ?,0x10($v1)` loads and an un-pinned p10 load are mutually exclusive. The real lever is register pressure at local-alloc time.
- mechanism: Dump-verified, not guessed. tmp/grind/func_80060A68/dumps/text1b.lreg:29591 identifies p10 as pseudo 75 (insn 39 sets it from `(mem:SI (plus (reg 72) (const_int 16)))`, insn 56 is its only reader). text1b.greg:25167 says `;; 1 regs to allocate: 83`, so reg 75 is assigned by LOCAL-ALLOC, and the dispositions line reads `75 in 2` = $v0 (reg 76, temp_a1, is `76 in 5` = $a1). In sched1's emitted order (49, 46, 51, 53, 39, 59, 56, 61, 64, 66) $v0 is killed at insn 53 (`sh $v0,0x18($v1)`) and re-born at insn 64, so it is free across reg 75's whole range and local-alloc's lowest-numbered-free-hard-reg scan takes it; in the final schedule $v0 is written at slot 20 and read at slot 22, straddling 12..28, so sched2 cannot hoist the load out of slot 23. birthing_insn_p still fires and still pins insn 39 next to insn 56 in SCHED1's order - but sched2 runs with reload_completed == 1 and never bumps, so it is free to undo that adjacency whenever the hard register is dead across the gap. Two separable gates; only the register one has to move.
- probe: Read tools/gcc-2.7.2/sched.c:2495-2600 (birthing_insn_p + adjust_priority + schedule_insn) directly; read text1b.lreg / text1b.greg for the cb body; then sweep 59 statement orders through `sandbox func_80060A68 --disable all`, disassembling each and recording the slot and hard register of every `lw ?,0x10($v1)` (harness in tmp/grind/func_80060A68/s8/).
- result: Target's `lw $a1,0x10($v1)` at slot 12 was produced in TWENTY distinct pure-C bodies, p10 single-set in every one, by three independent honest pressure sources: (a) hoisting the `idx = *(u16 *)outer;` read above the p10 consumer statement - f3/f4/i3/i4/r1 all 8/66 with THREE 0x10 loads and a prefix byte-identical to target through slot 22 (cb diverges at 11), plus r3 10/66 and r6 7/67; (b) sinking copy 3's store below the +2 halfword read - n2 10/67 (three loads), m1 9/66, o4/o5 10/66, o7 10/67; (c) splitting copy 3 into a named `c3` local whose store becomes the p10/0x18 cse separator - u5 8/66 (three loads), ua 8/66, u9 10/66, u1 8/68, u2 11/68. u5 additionally reproduces target's slots 26-34 EXACTLY, including `lhu $a1,0x4($a1)` at slot 29 (p10's consumer, seventeen slots after its load, in $a1) and the late `lhu $a0,0x0($v1)` idx read at slot 30 - both campaign firsts. Floor unchanged: candidate.c re-measured 2 / 66 / 66 on today's HEAD.
- verdict: CONFIRMED

## [s8] The remaining 8-slot cost of the f3 family can be removed by re-seating or re-associating the pressure-supplying statement so that the third `lw $a0,0x10($v1)` wins slot 23 instead of the hoisted idx load.
- mechanism: `idx = *(u16 *)outer;` compiles to `lhu $a0,0x0($v1)` and depends on nothing but $v1, live from slot 0, so sched2 has it ready at the cycle that fills the load-delay slot after `lhu $v0,0x0($a0)` at slot 22 and it beats the third 0x10 load to slot 23. Target puts the 0x10 load at 23 and the idx load at 30. Slots 1-22 and 32-66 of f3 are already correct; 23-31 are a permutation.
- probe: Twelve source seats for the idx read on the cb order (i0..i11); LUID re-association putting the +2 read above the consumer so the idx read's LUID exceeds the third load's (r1..r6); consumer-and-p10 moved into or above the copy triple (ka..kh); +0 halfword value hoisted into a `t0` local with the 0x18 store delayed (e1..e5); copy-3 sinks (m1..m5); `c3` splits (o4..o8, u1..u9, ua, ub); p10-position sweep (pa, pb, pd, pe); halfword-group permutations (h1..h8); n1..n6.
- result: KILLED for every spelling tried. LUID re-association does not move the idx load (r1 is byte-identical to f3, 8/66). idx seats 0 and 1 over-pressure and give p10 $a2 (9/66); seats 2-5 give $a1 (8/66, the f3 family); seats 6+ fall back to $v0 (cb, seat 9, is the 4/66 floor seat). Moving the p10 consumer into the copy triple (ka 9/66, kb 11/67, kc 10/67, kd 8/66, ke 11/67, kf 10/67, kg 8/66, kh 10/67) does put the load at slot 10 or 11 but in $a0, because $a0 is free there too - a winning body must occupy $v0 AND $a0 across p10's sched1 range. Delaying the 0x18 store through a `t0` local (e1 8/66, e2 7/65, e3 7/65, e4 7/65, e5 9/65) never reaches $a1 and usually costs a load, because the delayed store stops separating the two 0x10 reads.
- verdict: KILLED

## [s9] The three-load structure and the $a1-at-slot-12 hoist are NOT mutually exclusive at 66 instructions; the residual is a single sched2 placement of the +4 address load
- mechanism: d8's order gets all three `lw ?,0x10($v1)` loads at 66 insns with slots 30-63
  byte-identical to target, leaving one defect: the +4 read's address load sits at slot 24 in $v0
  instead of slot 12 in $a1.
- probe: 28 bodies measured with tmp/grind/func_80060A68/s9/run2.ps1 + cmp.py; d8 = 4/66.
- result: CONFIRMED as a structure (d8 is the cleanest 66-instruction three-load body the campaign
  has produced) but it does NOT lower the floor: candidate.c remains 2/66.
- verdict: CONFIRMED (structure) / the floor stays at 2.

## [s9] A `*(s32 *)(outer + 0x10)` load whose only consumer is the +4 read cannot be hoisted to slot 12 - sched2 ready-list starvation pins it adjacent to that consumer
- mechanism: sched.c schedules backward and never idles a cycle while any insn is ready. The load
  becomes ready exactly when its single consumer is scheduled, and it is then the only ready insn.
  text1b.sched2 (b1 applied) prints `;; ready list at T-30: 39 (3), now 39`.
- probe: `pwsh tools/grinder/dump.ps1 func_80060A68` with b1 in place; read
  tmp/grind/func_80060A68/dumps/text1b.sched2 from line 36617.
- result: every one-consumer body measures the load at slot 24-25 (b1/b2/b3 5/67, d8 4/66, e5 7/67,
  e6 8/68, d2 9/67, d6 8/67); every body that reaches slot 12 in $a1 either gives the load a second
  consumer (candidate.c, q5) or adds an outside pressure source (f3, g1, c4, d1).
- verdict: CONFIRMED. This retires the s6/s7 `reg_n_sets` framing for good and refines s8's:
  occupancy decides the REGISTER, ready-list starvation decides the SLOT.

## [s9] An outside pressure source can supply the missing ready insn without stealing target's slot-23 load-delay slot
- mechanism: the pressure source must be ready at sched2's T-30 but must not be the cheapest filler
  for the load-delay slot at forward slot 23, which target gives to the third 0x10 load.
- probe: seven distinct sources measured on the b1 and d8 orders (idx read, `gp18 = outer + 0x18`,
  `gp20 = outer + 0x20`, the copy-2 split, the copy-3 split, the gp-347C store hoist, the +2 read
  hoist), each in two to four seats.
- result: dependence-free sources (idx, gp18, gp20) either steal slot 23 (g1 8/66, f3 8/66) or are
  inert on the placement (a2 6/66, g3 4/66, d4 5/67); sources with a dependence either break the
  copy triple (c4 8/66) or cost instructions (d1 8/68, c5 5/67).
- verdict: KILLED for every source tried. The frontier is a source that is BOTH dependent on
  something late in the block AND consumed early enough to be ready at T-30.

## [s9] The three-load structure and the $a1-at-slot-12 hoist are not mutually exclusive at 66 instructions; a body exists whose whole residual is the placement of the +4 read's address load.
- mechanism: Reading *(s32*)(outer+0x10) into p10 ABOVE the copy-3 store makes the copy-3 store a cse separator, so the +0 and +2 reads each get a fresh load and p10 serves only the +4 read; the 0x18 store separates +0 from +2.
- probe: d8 = copy1; copy2; p10; copy3; 0x18 store(+0 fresh); temp_a1=*(p10+4); temp2(+2 fresh); gp3478; 0x1A; idx; gp347C; 0x1C. Measured with tmp/grind/func_80060A68/s9/run2.ps1 and diffed slot-by-slot with s9/cmp.py.
- result: d8 = score 4 / build 66 / target 66, all three lw ?,0x10($v1) loads present, slots 30-63 byte-identical to target; sole defect is the +4 address load at slot 24 in $v0 instead of slot 12 in $a1, with slots 12-24 being target's stream shifted by one. It does NOT beat the floor (candidate.c stays 2/66).
- verdict: CONFIRMED

## [s9] A *(s32*)(outer+0x10) load whose only consumer is the +4 read cannot be hoisted to slot 12; sched2 pins it adjacent to that consumer by ready-list starvation.
- mechanism: sched.c schedules backward and never idles a cycle while any insn is ready. The load becomes ready exactly when its single consumer is scheduled, and it is then the only ready insn, so it is forced into the next cycle (= adjacent in forward order).
- probe: pwsh tools/grinder/dump.ps1 func_80060A68 with b1 applied; read tmp/grind/func_80060A68/dumps/text1b.sched2 from line 36617: ';; ready list at T-30: 39 (3), now 39' then ';; launching 56 before 39 with no stalls at T-31'. Cross-checked against every one-consumer body measured.
- result: Every one-consumer body puts the load at slot 24-25 (b1/b2/b3 5/67, d8 4/66, d2 9/67, d6 8/67, e5 7/67, e6 8/68); every body that reaches slot 12 in $a1 either gives the load a second consumer (candidate.c, q5) or adds an outside pressure source (f3, g1, c4, d1).
- verdict: CONFIRMED

## [s9] An outside pressure source can supply the missing ready insn at sched2's T-30 without stealing target's slot-23 load-delay slot.
- mechanism: The source must be ready at that cycle but must not be the cheapest filler for forward slot 23, which target gives to the third 0x10 load.
- probe: Seven sources measured on the b1 and d8 orders in two to four seats each: the idx read, a named gp18 = outer+0x18, a named gp20 = outer+0x20, copy 2 split into a c2 local, copy 3 split into a c3 local, the gp-347C store hoisted above the halfword group, and the +2 read hoisted (28 bodies total).
- result: Dependence-free sources either steal slot 23 (g1 8/66, f3 8/66) or are inert on the placement (a2 6/66, g3 4/66, d4 5/67); dependent sources break the copy triple (c4 8/66 - cse keeps copy 2's base in $a0 so target's lw $a0,0xC($v1) reload is lost) or cost instructions (d1 8/68, c5 5/67, a3 11/65). No source both supplies the ready insn and leaves slots 23-31 alone.
- verdict: KILLED

## [s10 2026-08-25 - escalation] Frontier item 1 ("a SECOND, EARLY consumer of the +0x10 pointer that is not one of the other two 0x10 reads") is CLOSED for honest C by a C-level argument, not by enumeration.
- statement: no pure-C body can hoist the +4 read's address load to slot 12 AND keep all three
  `lw ?,0x10($v1)` loads, because the only honest consumers of the +0x10 pointer that this
  function's semantics contain are the three halfword reads themselves.
- mechanism: GCC 2.7.2's cse.c folds a repeated `*(s32 *)(outer + 0x10)` onto the MOST RECENT
  live equivalent expression and any aliasing store in between invalidates it. Therefore
  "three separate loads" is exactly equivalent to "each of the three reads is separated from
  the previous one by an aliasing store", which is exactly equivalent to "each load has exactly
  one consumer". Give any one load a second consumer and that consumer's own load disappears
  (two loads, 66 insns, a nop where target's third load sits) and, additionally, that read
  inherits the hoisted load's hard register. The only way out would be a consumer of the
  pointer that is not a read of `p10[0]`, `p10[1]` or `p10[2]` - and the function computes no
  such value, so it would have to be fabricated (a discarded read / address-of / dead local),
  which is the banned dead-read family, not a lever.
- probe: two new bodies measured this session with tmp/grind/func_80060A68/s10/run2.ps1 and
  diffed slot-by-slot with s10/cmp.py: w1 (the +0 read taken into a `temp0` local ABOVE p10's
  definition, so p10's second consumer becomes the +2 read) and w2 (same, with the +2 read
  spelled as an inline re-read instead of `p10 + 2`).
- result: w1 = w2 = score 2 / build 66 / target 66, and BYTE-IDENTICAL to each other. Both are
  the exact mirror of candidate.c: `lw a1,16(v1)` still at slot 11 and the fresh `lw a0,16(v1)`
  still at slot 19, but now the +0 read is target-exact (`lhu v0,0(a0)`, which candidate.c gets
  wrong as `lhu v0,0(a1)`) and the +2 read is wrong instead (`lhu a0,2(a1)` vs target's
  `lhu a0,2(a0)`). The missing third load (line 23, a nop) is common to both. The residual
  count is invariant at 2 under the choice of second consumer - it only moves between reads.
- verdict: CONFIRMED (the conservation is a property of cse fold-to-latest plus the one-consumer
  starvation law from s9, not of any particular statement order). Frontier item 1 is CLOSED;
  only frontier item 2 (an OUTSIDE pressure source ready at sched2's T-30 whose own emission
  slot is not target's slot-23 load-delay slot) survives, and s9 measured seven such sources in
  two-to-four seats each, all dead.

## [s10] Frontier item 1 -- a SECOND, EARLY consumer of the +0x10 pointer that is not one of the other two 0x10 reads -- is unreachable in honest C, so no pure-C body can both hoist the +4 read's address load to slot 12 and keep all three lw ?,0x10($v1) loads.
- mechanism: GCC 2.7.2's cse folds a repeated *(s32 *)(outer + 0x10) onto the most recent live equivalent and any aliasing store invalidates it, so 'three separate loads' == 'each of the three halfword reads separated from the previous by an aliasing store' == 'every one of those loads has exactly one consumer'; s9's ready-list-starvation law (sched.c schedules backward and never idles a cycle while anything is ready; text1b.sched2:36617 ';; ready list at T-30: 39 (3), now 39') then pins a one-consumer load adjacent to its consumer. The only escape would be a consumer of the pointer that is not a read of p10[0], p10[1] or p10[2] -- a value this function's semantics do not compute, so it would have to be fabricated (discarded read / address-of / dead local = banned dead-read family).
- probe: Two new bodies measured with tmp/grind/func_80060A68/s10/run2.ps1 and diffed slot-by-slot with s10/cmp.py: w1 (the +0 read taken into a temp0 local ABOVE p10's definition so p10's second consumer becomes the +2 read) and w2 (same with the +2 read spelled as an inline re-read rather than p10 + 2). Both compared against candidate.c's disassembly and against s10/target.txt.
- result: w1 = w2 = score 2 / build 66 / target 66 and byte-identical to each other; the load layout is unchanged (lw a1,16(v1) at slot 11, fresh lw a0,16(v1) at slot 19) but the residual MOVES: line 22 becomes target-exact (lhu v0,0(a0), which candidate.c gets wrong as lhu v0,0(a1)) and line 26 breaks instead (lhu a0,2(a1) vs target lhu a0,2(a0)). The missing third load at line 23 (a nop) is common to both. The residual count is invariant at 2 under the choice of second consumer.
- verdict: CONFIRMED

## [s10] The endgame-lock gate 1 (canonical-asm) still fails for func_80060A68 on today's chassis.
- mechanism: scan_hand_coded scores S1..S8 hand-written-asm signals; only STRONG tiers with S1/S2/S6 qualify per .claude/rules/endgame-lock-disposition.md.
- probe: python3 tools/scan_hand_coded.py --single func_80060A68, re-run 2026-08-25.
- result: tier=LOW score=1/8, S4 (6 loads in an 8-insn window @ insn 9) the only signal; S1/S2/S6 all clear. Canonical-asm refused.
- verdict: CONFIRMED

## [s10] The endgame-lock gate 2 (an in-hand SOTN-master precedent for a closing construct) is not even in play for this function.
- mechanism: A precedent can only be cited for a proposed construct; s8/s9/s10 propose none.
- probe: Audit of every body measured in s8, s9 and s10 (95+ bodies): all plain C, every local written exactly once, no volatile, no pin, no inline asm, no dead local.
- result: No family is claimed, so no precedent exists to cite. Gate fails by absence. The standing bans (multiply-assigned pointer-staging carrier in all three partition seats; temp2's dual role) were not approached.
- verdict: CONFIRMED

## [s11] The +4 address load is pinned at sched2 T-30 by ready-list starvation, so an OUTSIDE pressure source ready at T-30 can free it (frontier item 2, inherited from s9/s10)

**Mechanism claimed by s9/s10:** insn 39 becomes ready only when its single consumer is
scheduled; nothing else is ready at that cycle; sched.c never idles; therefore the load is
forced adjacent to its consumer, and a second ready insn at that cycle would break the pin.

**Probe:** `pwsh tools/grinder/dump.ps1 func_80060A68` with the three-load body `d8` applied
(re-measured 4/66 this session); read the full sched2 priority table, the whole T-1..T-54 trace,
and the post-reload RTL of insn 39 including its dependence list. Dump banked at
`tmp/grind/func_80060A68/s11/sched2.d8` (function region line 30625).

**Result:** insn 39's dependence list is
`(insn_list 152 (insn_list:REG_DEP_OUTPUT 51 (insn_list:REG_DEP_ANTI ...`, and insn 51 is the
+0 halfword read `(set (reg:HI 2 v0) (mem:HI (reg:SI 4 a0)))`. Insn 39 is
`(set (reg/v:SI 2 v0) (mem:SI (plus (reg/v:SI 3 v1) (const_int 16))))`. Both write **$v0**; the
dependence is an OUTPUT dependence created by reload's register assignment, and it is what gives
insn 39 INSN_PRIORITY 8. Separately, the trace shows the displacement idea is arithmetically
impossible: insn 39 has priority 8, the only other pending priority-8 insn from T-30 downward is
insn 53, and every insn scheduled at T-33..T-45 has priority 7 or lower. sched2 picks the
max-priority ready insn every cycle, so extra ready insns can displace a priority-8 insn by at
most ONE cycle — target needs 13.

**Verdict: KILLED** (twice over — wrong mechanism, and impossible on its own terms). Frontier
item 2 is closed. The controlling quantity is not readiness but **which hard register reload
gives p10's pseudo in a three-load body**.

## [s11] Keeping $v0 (or $a0) busy across the p10 read via a named value/base local will push p10's pseudo off $v0 in the 66-instruction three-load body

**Mechanism:** local-alloc assigns hard registers in class order ($v0, $v1, $a0, $a1, ...); if
copy 1's or copy 2's loaded value (or base pointer) is still live across p10's read, $v0 and/or
$a0 are unavailable and p10 falls to $a1 — target's register — which removes the REG_DEP_OUTPUT
and lets the load schedule early.

**Probe:** seven bodies (x1, x2, x4, x6, x7, x8, x9) placing a named `s32` local on copy 1's
value, copy 2's value, copy 3's value, copy 1's base and copy 2's base, with `p10` at each seat
inside the copy triple; plus a nine-seat sweep of `p10` on the c2-local frame (y2..y10). Measured
with `tmp/grind/func_80060A68/s11/run.ps1` (apply + `sandbox --disable all`).

**Result:** the register lever WORKS and the instruction count does not. All seven x-bodies
measure **5 / 67** — one instruction over target — and x2's disassembly
(`tmp/grind/func_80060A68/s11/x2.dis`) confirms the mechanism: three `lw ?,0x10($v1)` loads with
the +4 load in **$a1**, slots 0-11 byte-identical to target (including target's adjacent
`lw $v0,0xC($v1); lw $a0,0xC($v1)` pair), but the $a1 load lands at slot 24 instead of 12 and a
`nop` is added. Every seat of `p10` inside the copy triple costs that instruction regardless of
which value or base carries the local, so the only 66-instruction three-load body is still the
bare `d8` seat, where p10 is in $v0. On the after-the-copy-triple seats (y6, y7, y8) the local is
codegen-inert: 2/66, candidate.c's byte class.

**Verdict: CONFIRMED as a register lever, KILLED as a floor lever.** It produces the first body
in the campaign that holds target's register for the +4 load in a three-load stream; it cannot
yet hold it at 66 instructions.

## [s11] The +4 address load is pinned at sched2 T-30 by ready-list starvation, so an OUTSIDE pressure source ready at T-30 would free it (frontier item 2, inherited from s9/s10).
- mechanism: s9/s10 claim: insn 39 becomes ready only when its single consumer is scheduled, nothing else is ready at that cycle, sched.c never idles a cycle, so the load is forced adjacent to its consumer; a second ready insn would break the pin.
- probe: pwsh tools/grinder/dump.ps1 func_80060A68 with the three-load body d8 applied (re-measured 4/66 this session); read the full sched2 priority table, the whole T-1..T-54 trace, and the post-reload RTL of insn 39 including its dependence list. Dump banked at tmp/grind/func_80060A68/s11/sched2.d8 (function region line 30625).
- result: Insn 39 is (set (reg/v:SI 2 v0) (mem:SI (plus (reg/v:SI 3 v1) (const_int 16)))) -- the +4 address load -- and its dependence list is (insn_list 152 (insn_list:REG_DEP_OUTPUT 51 (insn_list:REG_DEP_ANTI ...)). Insn 51 is (set (reg:HI 2 v0) (mem:HI (reg:SI 4 a0))), the +0 halfword read. BOTH ARE IN $v0. sched2 runs post-reload, so the dependence is an OUTPUT dependence manufactured by reload's register assignment, and it is what raises insn 39 to INSN_PRIORITY 8 and releases it only at T-30. Separately, the trace shows the displacement idea was arithmetically impossible: insn 39 has priority 8, insn 53 is the only other pending priority-8 insn from T-30 downward, and every insn scheduled at T-33..T-45 has priority 7 or lower; since sched2 picks the max-priority ready insn every cycle, extra ready insns can displace a priority-8 insn by at most ONE cycle, where target needs 13 (forward slot 12 is about T-43).
- verdict: KILLED

## [s11] Keeping $v0 or $a0 busy across the p10 read via a named value/base local will push p10's pseudo off $v0 in a 66-instruction three-load body, removing the output dependence and letting the +4 load schedule early.
- mechanism: local-alloc assigns hard registers in class order ($v0, $v1, $a0, $a1, ...); if copy 1's or copy 2's loaded value or base pointer is still live across p10's read, $v0 and/or $a0 are unavailable and p10 falls to $a1 -- target's register -- which removes the REG_DEP_OUTPUT identified above.
- probe: Seven bodies (x1 x2 x4 x6 x7 x8 x9) putting a named s32 local on copy 1's value, copy 2's value, copy 3's value, copy 1's base and copy 2's base, with p10 at each seat inside the copy triple; plus a nine-seat sweep of p10 on the c2-local frame (y2..y10). Measured via tmp/grind/func_80060A68/s11/run.ps1 (apply + sandbox --disable all); x2 and y6 disassembled.
- result: The register lever works; the instruction count does not. All seven x-bodies measure exactly 5 / 67. x2's disassembly (tmp/grind/func_80060A68/s11/x2.dis) confirms the mechanism directly: THREE lw ?,0x10($v1) loads, the +4 address load in $a1 (target's register, not $v0), and slots 0-11 byte-identical to target including target's adjacent lw $v0,0xC($v1); lw $a0,0xC($v1) pair at slots 10/11 -- but the $a1 load lands at slot 24 instead of 12 and one extra nop is emitted. Every p10 seat inside the copy triple costs that instruction regardless of which value or base carries the local, so the only 66-instruction three-load body is still the bare d8 seat where p10 is in $v0. On the after-the-copy-triple seats the local is codegen-inert: y6 / y7 / y8 = 2 / 66, candidate.c's exact byte class (slot 12 lw a1,0x10(v1) correct, slot 22 lhu v0,0(a1) vs target's lhu v0,0(a0), slot 23 nop vs target's third lw a0,0x10(v1)).
- verdict: CONFIRMED

## s12 (2026-08-30) — escalation / disposition modality

### H-s12-1 — KILLED. "A store the function already performs can act as the cse separator between p10's def and the 0x18 read for free."
Mechanism: cse folds a repeated `*(s32 *)(outer + 0x10)` onto the most recent equivalent
unless an aliasing store intervenes; the two gp stores (`D_800A3478 = outer + 0x18`,
`D_800A347C = outer + 0x20`) and copy 3's store are the only stores available in that region,
and moving one of them there would produce the third load without inventing anything.
Probe: bodies a1/a2/a3/a4/b1 (tmp/grind/func_80060A68/s12/bodies/), applied and measured with
`sandbox func_80060A68 --disable all`.
Result: a1 (3478) 11 / 68; a2 (347C) 8 / 68; a3, a4 (both, either order) 14 / 69; b1 (p10's
def hoisted above copy 3's store) 5 / 67; b4 8 / 68; b5 7 / 67. Every separator costs at
least one instruction and the gp separators cost two, because hoisting the store also hoists
its `addiu $v0,$v1,k` and forces a load-delay nop.
Verdict: KILLED. There is no free separator. Banked at rejected/s12-gp3478-*,
rejected/s12-gp347C-*, rejected/s12-both-gp-stores-*, rejected/s12-p10-def-above-copy3-store-*.

### H-s12-2 — KILLED. "The untried {+0,+2} partition puts the +4 load on its own while leaving p10 with an early consumer, so it reaches target's three-register layout at 66 instructions."
Mechanism: if the +0 and +2 reads are both spelled directly off the local `p10`
(`*(u16 *)(p10 + 0)`, `*(u16 *)(p10 + 2)`) then no cse fold is involved for them at all —
p10 is a variable, and intervening stores do not kill it — while the +4 read, written as
`*(u16 *)(*(s32 *)(outer + 0x10) + 4)` after the 0x18 and 0x1A stores, is forced to reload
the pointer. That yields a body with p10 consumed early AND a separate +4 load, which is the
combination s10's two partitions could not produce.
Probe: bodies c1 / c2 / c3, measured and c1 disassembled (tmp/grind/func_80060A68/s12/c1.dis).
Result: c1 = 4 / 66, c2 = 7 / 66, c3 = 5 / 66. c1's disassembly shows the separate +4 address
load emitted at slot 27 in `$v0` (`lw v0,16(v1)` / `lhu a1,4(v0)`), never at slot 12 in `$a1`.
Verdict: KILLED, and with it the whole PARTITION AXIS — all three groupings of the three
halfword reads onto two loads are now measured ({+0,+4} 2/66, {+2,+4} 2/66, {+0,+2} 4/66).
Banked at rejected/s12-partition-0-and-2-share-a-load-plus4-separate-score4-66insns.c.

### H-s12-3 — KILLED (as a lever; CONFIRMED as an explanation). "local-alloc's first-fit order is the steerable quantity: raising p10's qty priority would hand it $a0."
Mechanism: `tools/gcc-2.7.2/local-alloc.c:1649-1685` computes
`pri = floor_log2(refs) * refs * size / (death - birth) * 10000` and `block_alloc`
(local-alloc.c:1563, 1571-1580) allocates first-fit in decreasing-pri order.
Probe: read the formula out of the compiler source and evaluate it on the campaign's measured
qty table for the base body (qty 8 = pseudo 75, refs 3, birth 26, death 44; qty 11 = pseudo 74,
refs 2, birth 36, death 42).
Result: pri(qty 8) = 1666, pri(qty 11) = 3333 — qty 11 allocates first and takes $a0, qty 8
then takes $a1. This reproduces the observed assignment exactly, so the model is right, but
the only two ways to raise qty 8 above 3333 are (a) a fourth reference to the +0x10 pointer,
which is the fabricated-second-consumer axis closed by s10 and banned by the frozen family
list, or (b) a live range shorter than 9 insns, which is literally "move the +4 read early" —
measured as d8 (4/66) and c1 (4/66), both of which then put the load in $v0.
Verdict: KILLED as a lever. The campaign's open item ("profile inverse.py local, then depth-1
the qty8/qty11 decision") is answered analytically and needs no tool fix for THIS function.

### The general law s12 establishes (supersedes the s10 conservation statement)
A `lw ?,0x10($v1)` is emitted at slot 12 and allocated `$a1` **iff** it has an early
consumer; a load with no early consumer is emitted late and allocated `$v0`. Four
independent bodies exhibit both halves (base/y6, w1/w2, c1, d8). Target requires a load that
is simultaneously unshared (three loads) and early-in-$a1, which is exactly the combination
the law excludes at 66 instructions. Buying the early consumer with a separator store costs
an instruction (H-s12-1); buying it with a fabricated consumer is a banned family.

## [s12] A store the function already performs can act as the cse separator between p10's def and the 0x18 read for free, producing target's third lw ?,0x10($v1) at no instruction cost.
- mechanism: cse folds a repeated *(s32 *)(outer + 0x10) onto the most recent live equivalent unless an aliasing store intervenes; the only stores available in that region are D_800A3478 = outer + 0x18, D_800A347C = outer + 0x20 and copy 3's store.
- probe: bodies a1/a2/a3/a4/b1/b4/b5 in tmp/grind/func_80060A68/s12/bodies/, each spliced over src/text1b.c:3063 and measured with sandbox func_80060A68 --disable all
- result: a1 (3478) 11/68; a2 (347C) 8/68; a3 and a4 (both stores, either order) 14/69; b1 (p10's def above copy 3's store) 5/67; b4 8/68; b5 7/67. Every separator costs at least one instruction; the gp separators cost two because hoisting the store also hoists its addiu $v0,$v1,k and forces a load-delay nop.
- verdict: KILLED

## [s12] The never-measured {+0,+2} partition gives p10 an early consumer AND leaves the +4 read with its own load, reaching target's three-register layout at 66 instructions.
- mechanism: Spelling the +0 and +2 reads directly off the local p10 (*(u16 *)(p10 + 0), *(u16 *)(p10 + 2)) involves no cse fold at all -- p10 is a variable and intervening stores do not kill it -- while the +4 read written as *(u16 *)(*(s32 *)(outer + 0x10) + 4) after the 0x18/0x1A stores is forced to reload the pointer.
- probe: bodies c1/c2/c3 measured; c1 disassembled to tmp/grind/func_80060A68/s12/c1.dis and compared slot-by-slot with asm/funcs/func_80060A68.s
- result: c1 = 4/66, c2 = 7/66, c3 = 5/66. c1's separate +4 address load is emitted at slot 27 in $v0 (lw v0,16(v1) / lhu a1,4(v0)), never at slot 12 in $a1 -- the same pathology as the three-load body d8.
- verdict: KILLED

## [s12] local-alloc's first-fit order is a steerable quantity: raising p10's qty priority would hand it $a0 and fix our line 22.
- mechanism: tools/gcc-2.7.2/local-alloc.c:1649-1685 computes pri = floor_log2(refs)*refs*size/(death-birth)*10000 and block_alloc (local-alloc.c:1563, 1571-1580) allocates first-fit in decreasing-pri order.
- probe: read the formula out of the compiler source and evaluated it on the 2026-08-30 ra_solver campaign's measured qty table for the base body (qty 8 = pseudo 75, refs 3, birth 26, death 44; qty 11 = pseudo 74, refs 2, birth 36, death 42)
- result: pri(qty 8) = 1666 < pri(qty 11) = 3333, so qty 11 allocates first and takes $a0 and qty 8 then takes $a1 -- reproducing the observed assignment exactly. The only two ways above 3333 are a fourth reference to the +0x10 pointer (the fabricated-second-consumer axis, closed by s10 and a banned family) or a live range under 9 insns (= move the +4 read early, measured as d8 and c1, both 4/66 with the load in $v0).
- verdict: KILLED

## [s12] Endgame gate 1 (canonical-asm) might pass on today's post-migration chassis.
- mechanism: scan_hand_coded scores eight hand-written-asm signatures; a STRONG tier (S1/S2/S6) would put the function on the canonical-asm grant path instead of a refusal.
- probe: python3 tools/scan_hand_coded.py --single func_80060A68, re-run this session
- result: tier=LOW score=1/8, S4 only ('6 loads in 8-insn window @ insn 9'); S1, S2, S3, S5, S6, S7, S8 all clear.
- verdict: KILLED

## 2026-09-01 — operator reopen note (owner ruling 2026-09-01 (decisions.md FORECLOSED-BUCKET REVIEW entry))

Returned to active under Ruling A. Ground: the qty8/qty11 pairwise priority comparison replaced an inverse.py-local search that was killed at 100% CPU before finishing depth 1 (evidence.md:1926-1950) — first-fit over 23 quantities is a whole-order property. Named probes: (1) bounded/profiled inverse.py local --block 0 (or an 800324D0-style order-closure replay over block_alloc's sorted order) enumerating which orderings seat p10 in $a0; (2) local_extract.py func_80060A68 --suggest (instrument postdates the record) to confirm the suggestion sets are empty rather than assumed.

## s13 (2026-09-01) — the owner's Ruling A probes, executed and closed

**H-s13-1 (KILLED — as a tooling claim).** "inverse.py local's forward replay is
superlinear on 23-quantity blocks, so the escape set cannot be enumerated."
Mechanism claimed by the 2026-08-30 campaign note. FALSE: the hang is
`itertools.permutations(base)` over 23 quantities in `LocalBackend.atoms()`
(tools/ra_solver/inverse.py:484-489, the `len(base) <= 6` guard sits inside the loop).
With that atom class omitted the identical backend finishes depth 1 in 0.0 s and
depth 2 in 3.9 s (tmp/grind/func_80060A68/s13/bounded_local.py). The search that was
"killed at 100% CPU before depth 1" is now FINISHED. tools/ is outside a grind
session's edit surface — the guard hoist is reported for the operator lane, not applied.

**H-s13-2 (CONFIRMED).** "The $a1 seat is decided by first-fit over a `used` set in
which $a0 is contributed by TWO overlapping quantities, not one." QTYDBG/FFR ground
truth: qty8 is allocation call #24 of 25 with used={$zero,$at,$v0,$v1,$a0}; the only
allocated $a0 holders overlapping [26,44] are qty10 (32-36) and qty11 (36-42).
s12's qty8-vs-qty11 pairwise comparison was therefore necessary but not sufficient —
exactly the depth limitation the owner's 2026-09-01 review named.

**H-s13-3 (CONFIRMED, and it is the closure).** "qty8 takes $a0 iff its local-alloc
priority reaches 5000." pri = floor_log2(refs)*refs*size*10000/span, ties broken on
ascending qty number (8 < 10). Complete escape set: refs=3 & span<=6; refs=4 & span<=16;
refs=5 & span<=20; refs=6 & span<=24. Baseline refs=3 span=18 -> 1666.

**H-s13-4 (KILLED, measured twice on the real compiler).** "A reference lift can move
qty8 over 5000." It cannot, because a reference is an instruction and an instruction
inflates the span in the same units the priority divides by. PROBE A (refs 5, span 24,
pri 4166, got $a2) and PROBE B (refs 4, span 28, pri 2857, got $a1, sandbox 6/72/66)
bracket the axis; and since the build already sits at target's exact 66 instructions,
every added reference is a guaranteed +1 residual even in a branch where the seat flips.
The refs axis cannot pay for itself. Banked in rejected/s13-refs-lift-*.c.

**H-s13-5 (KILLED, structural).** "The sanctioned duplicated-statement-into-arms
ref-lift — the family that closed func_800324D0 and the owner's named probe for
func_80045294's refs_up — closes this seat too." It has no attachment point: the
contested span [26,44] is straight-line, and the function's only conditional is the
trailing `if (*(s32 *)D_800A3468 & 0x200000)` guard, which references neither p10 nor
$a0. The family requires a REAL pre-existing arm; this function has none in range.

**Standing caveat for any future session.** inverse.py local's forward model reproduces
only 14 of 23 QTYDBG seats on this block (interval-overlap conflicts vs local-alloc's
qty_conflict bitmaps). Treat its verdicts here as corroboration only; the QTYDBG/FFR
dump plus local-alloc.c:1649-1685 is the load-bearing evidence.

## [s13] inverse.py local's forward replay is superlinear on 23-quantity blocks, so the qty8/qty11 escape set cannot be enumerated (the 2026-08-30 campaign note's tool-wall diagnosis).
- mechanism: Claimed per-atom forward replay cost. Actual cause: LocalBackend.atoms() (tools/ra_solver/inverse.py:484-489) builds its ALLOC_ORDER class as `for perm in itertools.permutations(base): if list(perm) != base and len(base) <= 6:` — the size guard sits INSIDE the loop, so with 23 allocated quantities atoms() walks 23! permutations and never returns. The forward replay was never reached.
- probe: Reused the identical LocalBackend (same _alloc, same _pri, same model file) from tmp/grind/func_80060A68/s13/bounded_local.py with the atom list rebuilt minus the ALLOC_ORDER class; ran depth 1 over 510 atoms and depth 2 over the 464 non-qty8 atoms.
- result: Depth 1 completes in 0.0 s; depth 2 in 3.9 s. The block was never expensive — the search was never started. tools/ is outside a grind session's edit surface, so the guard hoist is reported for the operator lane, not applied.
- verdict: KILLED

## [s13] The $a1 seat is decided by a pairwise qty8-vs-qty11 priority comparison (the s12 analytic closure the owner's 2026-09-01 review flagged as depth-limited).
- mechanism: local-alloc block_alloc allocates first-fit ascending over a priority-sorted order; a quantity's `used` set accumulates the hard registers of every ALREADY-ALLOCATED conflicting quantity.
- probe: local_extract.py text1b --func func_80060A68 --suggest; read the QTYDBG order table and the SUGGDBG-FFR per-call `used`/`first_used` sets for blk 0 (artifact s13/qtydbg_baseline.txt).
- result: qty8 (reg1=75, birth 26, death 44, refs 3) is allocation call #24 of 25 with used = {$zero,$at,$v0,$v1,$a0} -> first free 5 = $a1. $v0/$v1 are blocked structurally (qty0 spans 2-52 plus the block's hard base pointer). $a0 is blocked by TWO overlapping quantities, qty10 (32-36, call #20) and qty11 (36-42, call #23) — s12 named only qty11, so both must be displaced, not one.
- verdict: KILLED

## [s13] qty8 takes $a0 iff its local-alloc priority reaches 5000, and that condition has a complete, finite solution set in (refs, span).
- mechanism: qty_compare priority = floor_log2(refs)*refs*size*10000/(death-birth) (tools/gcc-2.7.2/local-alloc.c:1649-1685), ties broken on ascending qty number, and 8 < 10.
- probe: Computed the measured priority table from the QTYDBG dump (qty8=1666, qty11=3333, qty10=5000 alongside eight further 5000s) and solved the inequality at size 1.
- result: Escape set: refs=3 & span<=6; refs=4 & span<=16; refs=5 & span<=20; refs=6 & span<=24. Baseline is refs=3, span=18. This is the set the 2026-08-30 campaign note recorded as 'underived'.
- verdict: CONFIRMED

## [s13] A reference lift on p10 can move qty8 over the 5000 threshold and reseat it in $a0.
- mechanism: Raising refs raises floor_log2(refs)*refs, but every added reference is an added instruction and an added instruction inflates the span the priority divides by — so the lift partly or wholly cancels itself, and on a body already at target's exact instruction count it also costs residual slots outright.
- probe: Two real builds. PROBE A: two extra p10 reads placed AFTER the temp_a1 read. PROBE B: the same two reads placed immediately after `p10 = *(s32 *)(outer + 0x10);` to protect the death point. QTYDBG re-extracted for each; sandbox scored for B.
- result: PROBE A: refs 3->5 but span 18->24, pri = 2*5*10000/24 = 4166 < 5000, still allocated after qty10, seat $a2. PROBE B: span 18->28 and only refs=4 (cse collapsed one base reference), pri 2857, seat $a1, sandbox 6 / build 72 / target 66 — six extra instructions on a body that already matches target's 66. Banked as rejected/s13-refs-lift-late-* and rejected/s13-refs-lift-early-*.
- verdict: KILLED

## [s13] The sanctioned duplicated-statement-into-arms ref-lift — the family that closed func_800324D0 and the owner's named probe for func_80045294's refs_up 72: 3->4 — supplies the honest reference this seat needs.
- mechanism: The family duplicates a REAL pre-existing statement into two or more arms of a REAL pre-existing branch, lifting the reference count of the registers it touches without inventing a construct.
- probe: Structural read of candidate.c's body against the contested span [26,44] identified by the QTYDBG table.
- result: No attachment point exists. The contested span is straight-line code; func_80060A68's only conditional is the trailing `if (*(s32 *)D_800A3468 & 0x200000) { D_800A32BC = 0xA; }`, far outside the span, and it references neither p10 nor $a0. The family is unavailable here for want of an arm, not for want of a census.
- verdict: KILLED

## [s13] The endgame-lock AND-gates have changed since the 2026-08-30 record.
- mechanism: Gate (a) needs STRONG scan_hand_coded signals (S1/S2/S6); gate (b) needs an in-hand SOTN-master precedent for the closing construct.
- probe: python3 tools/scan_hand_coded.py --single func_80060A68 on today's HEAD (artifact s13/scan_hand_coded.txt); family review of candidate.c.
- result: Gate (a) FAILS: tier=LOW, score 1/8, S4 only ('6 loads in 8-insn window @ insn 9'). Gate (b) FAILS: candidate.c carries no coercion construct, so no family exists for which a precedent could be cited, and the one family that could have lifted refs honestly is foreclosed structurally.
- verdict: CONFIRMED

## s14 (2026-09-03) — solver modality

Chassis re-measured first: `candidate.c` = **2 / 66 / 66** on today's HEAD (src/text1b.c
has moved on — func_80057CC8 and func_800645B0 are now C bodies — so the s12 HEAD copy at
tmp/grind/func_80060A68/s12/text1b.c.HEAD is stale; s14 re-cut it as s14/text1b.c.HEAD).

**H-s14-1 (CONFIRMED — and it invalidates the mechanism the whole 2026-08-19 escalation
rests on).** "The `birthing_insn_p` LAUNCH_PRIORITY bump is what singles out copy 2's /
the +4 read's address load, so closing the function requires a multiply-set carrier."
FALSE as stated. The instrumented sched1 dump for the 3-load body lists an `adjpri`
observation for **every one of the 44 insns of block 0**, and **every insn with a REG
destination — 34 of 44 — carries `maxpri = 2130706433 = 0x7F000001` (LAUNCH_PRIORITY)**.
`birth: 0` occurs only on the ten insns whose SET_DEST is a MEM (the stores), where
birthing_insn_p returns at the `GET_CODE (SET_DEST (pat)) == REG` test. The three 0xC
loads (25, 32, 42), the other two 0x10 loads (49, 59), every copy load (27, 34, 44) and
the halfword reads (51, 56, 61) are bumped exactly as insn 39 is — and all of them land
in target's own positions. Bumpedness is therefore shared by the insns that MATCH, so it
cannot be the discriminator between our stream and target's. What actually decides where
a bumped insn lands is its READY TIME: a bumped insn is picked the instant it becomes
ready, and it becomes ready when its CONSUMER is picked. The consumer's position is
ordinary spellable C. Artifact: tmp/grind/func_80060A68/s14/sched_block0_pass1.txt.

**H-s14-2 (CONFIRMED, and it re-opens the seat axis s11/s12/s13 declared closed).**
"The +4 pointer can be seated in $a1 — target's register — in a THREE-load body by
ordinary statement order." Measured both directions with QTYDBG/FFR ground truth:
  - d8 / P2A0 (`temp_a1 = *(u16 *)(p10 + 4);` immediately after the 0x18 store):
    reg75 = qty10, birth 34 death 38, refs 2, span 4 -> pri 20000/4 = **5000** ->
    allocation call #19 of 25; the only already-allocated overlapping quantity is qty0
    ($v1), so first-fit gives **$v0**. Score 4 / 66.
  - P2A1 (the same read moved AFTER the 0x1A store): reg75 = qty11, birth 36 death 46,
    refs 2, span 10 -> pri 20000/10 = **2000** -> allocation call #24 of 25; by then
    qty9/qty10 hold $v0 and qty8/qty12 hold $a0 across [36,46], so first-fit is forced
    past {$v0,$v1,$a0} and gives **$a1**. Score 5 / 67.
  s13's escape arithmetic was run in the wrong direction: it asked how to RAISE
  pri(p10) to 5000 to win $a0, and every reference lift self-cancels against the span it
  inflates. The seat this function needs is $a1, which wants pri LOW — and lowering it is
  free, because it is a statement move, not an added reference. Banked at
  rejected/s14-temp_a1-read-late-3loads-plus-a1-seat-nop-at-21-score5-67insns.c.

**H-s14-3 (KILLED, instance).** "The source position of the `p10 = *(s32 *)(outer +
0x10);` statement steers the load's sched1 placement." Full 3x3 sweep (p10 before copy 1
/ copy 2 / copy 3) x (temp_a1 read early / after the 0x1A store / after the idx read):
every column is constant — 4/66, 5/67, 8/68 respectively — i.e. the p10 statement's
position is completely codegen-inert and only the CONSUMER's position moves anything.
Bodies at tmp/grind/func_80060A68/s14/bodies/P{0,1,2}A{0,1,2}.c.

**H-s14-4 (KILLED, instance).** "A single spellable statement move (LUID atom) in sched2
lifts the +4 pointer's load from slot 24 to target's slot 11 on the $a1-seated (P2A1)
chassis." `perturb.py --pass 2 --block 0 --goal-before 27:39 --atoms luid,luid_move
--depth 1` searched all 2970 single atoms: none reaches the goal. The same run for the
+2 pointer load's delay-slot goal (`--goal-before 53:56`) is likewise empty at depth 1.

**H-s14-5 (KILLED, instance).** "The classifier's `PRE-RA` verdict on candidate.c means
the residual is unmodelled." It is modelled — on the RIGHT chassis. `goal_from_tgt.py
classify` returns PRE-RA for candidate.c only because that body has 2 loads and a nop
where target has 3 loads (`ours only: nop x1 / target only: lw #,16(#) x1`), but on the
3-load body (P2A0/d8) the same classifier returns **FIRST DIVERGENCE: RA, `$v0 -> $a1
x2`** — a two-register substitution inside a modelled pass. The 3-load family, not the
2-load floor-2 body, is the chassis the solver stack can actually reason about.

**H-s14-6 (CONFIRMED — the session's headline; it overturns the campaign's central
negative).** "The target's early `lw $a1,0x10($v1)` at slot 11, together with three
independent 0x10 loads, is reachable in ordinary C at 66 instructions." REACHED.
Body W5 (`memory/grind/func_80060A68/rejected/s14-W5-three-loads-early-a1-slot11-
slots0-23-target-identical-score5.c`) measures **5 / build 66 / target 66** and its
**slots 0-23 are byte-identical to target**, including the slot-11 `lw $a1,0x10($v1)`.
It carries no invented local, no second write to any variable, no dead code, no
volatile, no asm — only two statement moves off the d8 body: (1) `temp_a1 =
*(u16 *)(p10 + 4);` after the 0x1A store (buys the $a1 seat, H-s14-2), and (2) `idx =
*(u16 *)outer;` before the D_800A3478 store (buys back the instruction that (1) costs).
This directly contradicts the 2026-08-19 / 2026-08-30 / 2026-09-01 records, all of which
state that target needs "a load that is BOTH unshared (three loads) AND early-in-$a1 —
the one combination the law excludes at 66 instructions" and that closing the function
"REQUIRES a C variable assigned more than once". Both claims are now measured false.
The residual is 5 instructions in slots 24-29 and is a different problem from the one
the campaign has been working: the +2 read must land in $a0 rather than $v0, its 0x1A
store must be deferred past the D_800A3478 gp store, and the idx read must fall to slot
29 instead of being hoisted into the +2 read's load-delay slot at 25.
Neighbours measured this session: X2/X3/X5/X6 = 5/66 (same class), V1/V3 = 6/66,
W1/P2A1 = 5/67, Y3 = 7/67, Y1/Y2/Y7 = 8/67, Y6 = 10/68, Y5 = 11/68.

## [s14] The birthing_insn_p LAUNCH_PRIORITY bump is what singles out the +4 read's address load, so closing the function requires a C variable assigned more than once (the s6 mechanism the 2026-08-19 / 08-30 / 09-01 records rest on).
- mechanism: sched.c:2504-2535 returns reg_n_sets[dest]==1 whenever the dest is live, and adjust_priority then raises the insn to LAUNCH_PRIORITY 0x7F000001 (sched.c:187), so a bumped insn is picked the instant it is ready and is emitted as late as possible.
- probe: tools/sched_solver/extract.py text1b on the 3-load body; read blocks[0].adjpri for func_80060A68 pass 1 (44 insns, 44 observations).
- result: Every one of the 44 observations carries maxpri = 2130706433 = 0x7F000001, and birth=1 on all 34 REG-destination insns (birth=0 only on the 10 MEM-destination stores 22 29 36 46 53 66 69 78 81 123). The three 0xC loads (25/32/42), the +0 and +2 pointer loads (49/59), the three copy loads (27/34/44) and the halfword reads (51/56/61) are bumped exactly as insn 39 is, and every one of them lands in target's own stream position. Bumpedness is shared by the insns that MATCH, so it does not discriminate our stream from target's; what decides a bumped insn's placement is its ready time, i.e. where its CONSUMER sits, which is ordinary spellable C. Confirmed constructively below: moving the consumer produced target's own slot-11 load with no multiply-set carrier anywhere in the body.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis (src/text1b.c at cbb3fbdd with P2A0 / W5 spliced), zero FAKE constructs in any body measured this session

## [s14] The +4 pointer quantity can be seated in $a1 - target's register - in a three-load body by ordinary statement order alone.
- mechanism: local-alloc priority = floor_log2(refs)*refs*size*10000/(death-birth) with first-fit ascending allocation (local-alloc.c:1649-1685 / 1563-1580). Lengthening the live range LOWERS the priority, which pushes the quantity to the end of the allocation order, by which time the quantities holding $v0 and $a0 across its span are already seated and first-fit is forced into $a1.
- probe: QTYDBG/FFR tables via tools/ra_solver/local_extract.py text1b --func func_80060A68 --suggest, taken on two bodies that differ only in the position of the statement temp_a1 = *(u16 *)(p10 + 4);.
- result: d8/P2A0 (read early): pseudo 75 = qty10, birth 34 death 38, refs 2, span 4, pri 5000, allocation call 19 of 25, got $v0. P2A1 (read after the 0x1A store): pseudo 75 = qty11, birth 36 death 46, refs 2, span 10, pri 2000, allocation call 24 of 25, got $a1. s13's escape set was solved for the opposite goal ($a0, pri >= 5000), where every candidate had to ADD a reference and the added instruction inflated the span the priority divides by; the seat this function actually needs wants pri LOW, and lowering it is free.
- verdict: CONFIRMED

## [s14] The target's early slot-11 lw $a1,0x10($v1) cannot coexist with three independent lw ?,0x10($v1) loads at 66 instructions in a body carrying no multiply-assigned carrier (the 'law' recorded in candidate.c's s12 header and in the 2026-08-30 and 2026-09-01 decisions.md entries).
- mechanism: Claimed conservation: three loads implies each load has exactly one consumer implies the +4 load is pinned adjacent to its consumer.
- probe: Built body W5 = d8 with two statement moves (the temp_a1 read placed after the 0x1A store; the idx read placed before the D_800A3478 store), measured it with sandbox --disable all, and diffed its disassembly slot-by-slot against asm/funcs/func_80060A68.s.
- result: W5 measures 5 / build 66 / target 66 with three 0x10 loads, and its slots 0-23 are byte-identical to target INCLUDING lw $a1,0x10($v1) at slot 11 and lw $a0,0x10($v1) at slots 19 and 22. It contains no invented local, no second write to any variable, no dead code, no volatile and no asm. Four sibling spellings (X2/X3/X5/X6) reproduce the same 5/66. The residual is now 5 instructions confined to slots 24-29.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis with W5 spliced into src/text1b.c; zero FAKE constructs present

## [s14] The source position of the p10 = *(s32 *)(outer + 0x10); statement steers that load's sched1 placement.
- mechanism: LUID order feeds rank_for_schedule's final tie-break (LUID descending among equal-priority ready insns), so a statement move should change where the load lands.
- probe: Full 3x3 sweep: p10 before copy 1 / copy 2 / copy 3, crossed with the temp_a1 read early / after the 0x1A store / after the idx read (bodies P0A0..P2A2).
- result: Every column is constant - 4/66, 5/67, 8/68 respectively - so the p10 statement's own position is codegen-inert here and only the CONSUMER's position moves anything.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, nine bodies, zero FAKE constructs

## [s14] A single spellable statement move lifts the +4 pointer's load from slot 24 to target's slot 11 in sched2 on the $a1-seated P2A1 chassis.
- mechanism: sched2 runs post-reload with adjust_priority disabled, so the order is decided by priority, dependence class and LUID alone; a luid / luid_move atom is exactly an ordinary source-statement move.
- probe: tools/sched_solver/perturb.py --func func_80060A68 --pass 2 --block 0 --atoms luid,luid_move --depth 1, goals 27:39 (the +4 pointer load) and 53:56 (the +2 pointer load into the load-delay slot); the model is order-exact on the baseline and 2970 atoms were searched for each goal.
- result: No single atom reaches either goal. (The depth-2 run was started and stopped inconclusive after 15 minutes - it is an operator-lane re-run, not a verdict.) The slot-11 load was nevertheless reached by a TWO-statement move on a different chassis (W5), which is consistent with the depth-1 emptiness rather than contradicted by it.
- verdict: KILLED
- kill_scope: instance
- measured_on: P2A1 chassis (3 loads, $a1 seat, 67 insns), sched_solver model text1b pass 2 block 0, zero FAKE constructs

## s15 (2026-09-03) — forensics modality

**H-s15-1 (CONFIRMED — the session's headline).** "The +2 halfword value pseudo can be
seated in $a0 — target's register — by ordinary statement order alone." Placing
`D_800A3478 = outer + 0x18;` between the +2 read and the 0x1A store makes the addiu
quantity live inside the value's range; the addiu (refs 2, span 2, priority 10000) is
allocated first and takes $v0, and first-fit forces the value past {$v0,$v1} to $a0.
QTYDBG ground truth in tmp/grind/func_80060A68/s15/qtydbg_W5.txt; measured on A8/E2/E3.

**H-s15-2 (CONFIRMED).** "Both register-based stores AND gp symbol stores separate two
`*(s32 *)(outer + 0x10)` reads in cse." E2 and E3 differ only in whether the p10 statement
precedes or follows `D_800A3478 = outer + 0x18;`, and that alone is 2 loads vs 3.

## [s15] The five-instruction residual of s14's W5 body is a scheduling problem in slots 24-29.
- mechanism: s14 recorded the residual as "the +2 read's value must land in $a0, its 0x1A store must be deferred past the D_800A3478 gp store, and the idx read must fall to slot 29", i.e. three coupled facts.
- probe: QTYDBG/FFR extraction on W5 (tools/ra_solver/local_extract.py) plus the -da sched1/sched2 traces; then a direct statement move placing the gp store inside the +2 value's live range, measured on 30 bodies.
- result: It is ONE fact, not three, and it is a local-alloc fact rather than a scheduling one. W5's slots 24-29 hold the same instruction multiset as target; the only wrong register is the +2 value's ($v0 vs $a0), and the two ordering differences follow from it (with the value in $a0 the idx read cannot occupy the load-delay slot at 25). Moving the gp store inside the value's range buys $a0 directly (E2/E3/A8, QTYDBG got=4) and target's 25/26/27 order comes with it for free.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis (identical to s14's text1b.c.HEAD), W5 re-measured 5/66; zero FAKE constructs in any body

## [s15] A three-load body can carry BOTH the early slot-11 $a1 load and the D_800A3478 gp store placed between the +2 read and the 0x1A store, at 66 instructions.
- mechanism: The two requirements are independent in principle — the first is a cse/sched2 fact about where p10's load is emitted, the second a local-alloc fact about the +2 value's seat.
- probe: 30 bodies built and measured with sandbox --disable all: A1-A8 (idx read in all eight gaps), B00-B23 (p10 statement before copy 1 / 2 / 3 crossed with four idx positions), C1-C18 (all tail permutations of {idx, +4 read, D_800A347C, 0x1C store} plus insertions of each into the S2..S3 window). Plus H1-H12, which try `D_800A347C = outer + 0x20;` as an alternative early cse separator.
- result: Every one of the 30 measures 67 instructions (scores 5-9); H1-H12 measure 67-69 (scores 8-14). The sched1/sched2 traces name the cause: with the gp store inside the +2 read's range, sched1 emits p10's load adjacent to the +2 pointer load, and in sched2's reverse walk the ready list at T-30 holds only insn 39, which is forced there and spent as the +2 pointer's delay-slot filler, leaving the +0 read's delay slot a nop. Best 3-load body reachable this way is 5/67; the best 3-load body overall is E3 at 3/66, which drops the early load instead.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 42 bodies, zero FAKE constructs in any of them

## [s15] Moving `D_800A347C = outer + 0x20;` early, to act as the cse separator that gives p10 its own early load, is byte-neutral for the rest of the body.
- mechanism: gp symbol stores separate cse'd loads (H-s15-2), so D_800A347C could supply the separation that a copy store supplies in the W5 family, without disturbing the +2 read's neighbourhood.
- probe: H1-H12 — the statement hoisted above the p10 statement and above it, crossed with all tail permutations.
- result: 67-69 instructions, scores 8-14; the statement's position is load-bearing for slots 30-32 and hoisting it costs one to three instructions immediately.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 12 bodies, zero FAKE constructs

## [s15] The +2 halfword value pseudo can be seated in $a0 - target's register - by ordinary statement order alone, with no multiply-set carrier and no FAKE construct.
- mechanism: local-alloc priority = floor_log2(refs)*refs*size*10000/(death-birth) with first-fit ascending allocation (local-alloc.c:1649-1685 / 1563-1580). In W5 the +2 value has refs 2 and span 2, so its priority 10000 is the maximum a refs=2 quantity can carry; it is allocated 6th of 25, nothing holds $v0 across [36,38], and first-fit gives it $v0. Placing the D_800A3478 addiu/store pair INSIDE its range lengthens the value to span 6 (priority 3333, allocated 23rd-24th) and makes the addiu quantity (refs 2, span 2, priority 10000, allocated 6th-7th) hold $v0 across it, so first-fit is forced past {$v0,$v1} to $a0.
- probe: tools/ra_solver/local_extract.py text1b --func func_80060A68 --suggest on W5 and on the moved-store bodies (A8, E2, E3); sandbox --disable all on each.
- result: W5: qty11 reg1=74 birth 36 death 38 refs 2 ord 5 got=2 ($v0). A8/E3: qty12 reg1=74 birth 38 death 44 refs 2 ord 23 got=4 ($a0), with the addiu quantity (reg 93/94, birth 40 death 42) taking $v0 at ord 6. Target's slot 25/26/27 order `addiu v0,v1,0x18 / sw v0,%gp(D_800A3478) / sh a0,0x1A(v1)` comes with the seat for free, because with the value in $a0 the idx read can no longer be hoisted into the load-delay slot at 25.
- verdict: CONFIRMED

## [s15] Both register-based stores and gp symbol stores separate two `*(s32 *)(outer + 0x10)` reads in cse, so the number of `lw ?,0x10($v1)` loads is set by which stores sit between the reads.
- mechanism: cse invalidates the cached MEM on an intervening store; a store through a register base invalidates conservatively, and the gp symbol stores also invalidate the register-based MEM here.
- probe: Bodies E2 and E3, which differ ONLY in whether the `p10 = *(s32 *)(outer + 0x10);` statement precedes or follows `D_800A3478 = outer + 0x18;`.
- result: E2 (p10 before the gp store) = 2 loads, score 2 / 66. E3 (p10 after it) = 3 loads, score 3 / 66. The same separation comes from every register-based store (the 0x18 store, the 0x1A store, the three copy stores). This is the rule that governs whether the third 0x10 load exists and where it can be emitted.
- verdict: CONFIRMED

## [s15] The five-instruction residual of s14's W5 body is three coupled facts (the +2 read's register, the 0x1A store's deferral past the gp store, and the idx read falling to slot 29).
- mechanism: s14 read the residual off the disassembly as three independent requirements in slots 24-29.
- probe: QTYDBG/FFR extraction on W5 (tools/ra_solver/local_extract.py) plus the -da sched1/sched2 traces, then a single statement move placing the gp store inside the +2 value's live range, measured across 30 bodies.
- result: It is ONE fact, and it is a local-alloc fact rather than a scheduling one. W5's slots 24-29 already hold target's exact instruction multiset; the only wrong register is the +2 value's, and both ordering differences follow from it. Bodies that buy the $a0 seat (E2/E3/A8) reproduce target's 25/26/27 order automatically.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis (byte-identical to s14's tmp/grind/func_80060A68/s14/text1b.c.HEAD; W5 re-measured 5 / 66 / 66); zero FAKE constructs in any body measured

## [s15] A three-load body can carry BOTH the early slot-11 $a1 load and the D_800A3478 gp store placed between the +2 read and the 0x1A store, at 66 instructions.
- mechanism: The early slot-11 load is a cse/sched2 fact about where p10's load is emitted; the $a0 seat is a local-alloc fact about the +2 value's range. They were expected to be independent.
- probe: 30 bodies built and measured with sandbox --disable all: A1-A8 (idx read in all eight gaps), B00-B23 (p10 statement before copy 1 / 2 / 3 crossed with four idx positions), C1-C18 (all tail permutations plus insertions into the S2..S3 window). Plus the -da sched1/sched2 traces of W5 and V4.
- result: Every one of the 30 measures 67 instructions (scores 5-9). Named cause from the sched2 trace: with the gp store inside the +2 read's range, sched1 emits p10's load adjacent to the +2 pointer load; in sched2's reverse walk the ready list at T-30 then holds only insn 39 (p10's load, priority 3), so it is forced into that slot and spent filling the +2 pointer's load-delay slot, leaving the +0 read's own delay slot a nop. In W5 the two are separated by the 0x18 store, insn 39 keeps losing rank_for_schedule against priority-6..10 insns and sinks to slot 11.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 30 bodies, zero FAKE constructs in any of them

## [s15] Moving `D_800A347C = outer + 0x20;` early, to act as the cse separator that gives p10 its own early load, is byte-neutral for the rest of the body.
- mechanism: gp symbol stores separate cse'd loads, so D_800A347C could supply the separation a copy store supplies in the W5 family without disturbing the +2 read's neighbourhood.
- probe: H1-H12: the statement hoisted above and below the p10 statement, crossed with all tail permutations, sandbox --disable all on each.
- result: 67-69 instructions, scores 8-14. The statement's position is load-bearing for slots 30-32 and hoisting it costs one to three instructions immediately.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 12 bodies, zero FAKE constructs

## [s15] The p10 statement's position among the three copy statements steers codegen on the gp-store-moved chassis.
- mechanism: s14's H-s14-3 measured it inert on the W5 chassis; the moved-store chassis has a different sched1 order and might respond.
- probe: B00-B23: p10 before copy 1 / copy 2 / copy 3 crossed with four idx-read positions, on the A-family (gp store between the +2 read and the 0x1A store).
- result: Each idx-position column is constant across all three p10 positions (7/67, 5/67, 7/67, 7/67), so the p10 statement's position among the copies is codegen-inert on this chassis too - a second independent confirmation of H-s14-3.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 12 bodies, zero FAKE constructs

## s16 hypotheses (2026-09-03, forensics)

### H-s16-0 (RE-AUDIT, CONFIRMED) - the s15 instance kills survive the current chassis.
HEAD `src/text1b.c` is byte-identical to `tmp/grind/func_80060A68/s14/text1b.c.HEAD`; the
two closest banked forms re-measure exactly as s15 recorded them (E2 = 2 / 66, E3 = 3 / 66)
and neither carries a FAKE construct, so `tools/fake_ablate.py` has nothing to ablate. The
s15 kills are chassis-current.

### H-s16-1 (CONFIRMED) - target's three-load geometry is reachable in ordinary C at 66 insns.
K7 emits `lw $a1,0x10($v1)` at slot 11 and `lw $a0,0x10($v1)` at slots 19 and 22 - target's
loads in target's slots and target's registers - at 66 instructions, score 5, with no FAKE
construct. The 2026-08-30 / 2026-09-01 disposition entries' claim that an unshared,
early-in-$a1 load is "the one combination the law excludes at 66 instructions" is now
disproved a second time (s14's W5 was the first).

### H-s16-2 (CONFIRMED, corrects s15) - the $a0 seat needs a donor, not the gp store.
Mechanism: local-alloc allocates quantities in descending priority
(pri = floor_log2(refs)*refs*size*10000/(death-birth), local-alloc.c:1649-1685) by first fit
(local-alloc.c:1563-1580). The +2 halfword value (reg74) has refs 2 and span 2 - the maximal
priority a refs=2 quantity can have - so it cannot be out-prioritised; it can only be made to
ENCLOSE a quantity that takes $v0 before it. s15 achieved that with the D_800A3478 addiu;
s16's L1 achieves it with a copy statement's load/store pair, giving reg74 got=4 ($a0) and
reg75 got=5 ($a1) at the same time. The donor's identity is free; what is not free is where
the donor's own instructions land.

### H-s16-3 (KILLED, instance) - P's position on the E2 spine did not produce the slot-11 load.
Probe: 228 ordinary-C bodies on the spine [O, Z, C1, C2, C3, S1, S2, S5, S3] - P at each of
the 9 spine gaps x 12 tail permutations of {S4,S6,S7,S8} (X sweep, 108), and P at the 4
copy-region gaps x S4 at all 10 gaps x 3 tail permutations of {S6,S7,S8} (Y sweep, 120).
Result: minimum score 2 (the E2 class); the third 0x10 load appears at slot 4, at slot 26/27,
or in a 67-instruction body, and in no body at slot 11. Measured on today's HEAD chassis with
zero FAKE constructs in any body. Logs: `tmp/grind/func_80060A68/s16/{xsweep,ysweep}.log`.

### H-s16-4 (KILLED, instance) - a moved copy statement as the $v0 donor is priced at ~9 points.
Probe: L1, L2, L3, L8 (which of the three copies is moved into the +2 read's window) and V1,
V2, V5, V6, V7, V8, V9, VA (P / S4 / S5 / S7 positions around the L1 spine) - 12 ordinary-C
bodies. Result: 10-17, 66-68 instructions; the moved copy's `lw / lw / sw` triple schedules
after the 0x18-0x1A block (slots 21/23/26) instead of target's 16/18/20 in every one.
Measured on today's HEAD chassis, zero FAKE constructs.

### H-s16-5 (KILLED, instance) - p10's load stayed at slot 4 whenever its read precedes the zero store.
Probe: N1, N2, N5, N7, N8 plus the X1_* and X2_* rows (24 bodies) place the p10 read before
or immediately after the D_800F10D0 zero store. Result: whenever the read precedes the zero
store, the load is emitted at slot 4 filling a load-delay slot target leaves as a nop, and
the body is 65 instructions; the best is N2 at score 2. The load depends only on $v1, so it
is ready as soon as its insn exists. Measured on today's HEAD chassis, zero FAKE constructs.

### H-s16-6 (KILLED, instance) - eliminating the temp2 named intermediate is expensive.
Probe: R1-R8 write each halfword read inline into its own store (no temp2, no p10), so the
three 0x10 reads are separated by their own stores and three loads are guaranteed. Result:
66-68 instructions, score 10-12. Measured on today's HEAD chassis, zero FAKE constructs.

## [s16] The two closest banked forms re-measure unchanged on today's HEAD chassis and carry no FAKE construct, so the s15 instance kills are chassis-current.
- mechanism: Mandated kill re-audit. HEAD src/text1b.c is byte-identical to tmp/grind/func_80060A68/s14/text1b.c.HEAD (commits since s15 touched only the ledger), so any chassis-relative conclusion from s15 should reproduce exactly.
- probe: Diffed git show HEAD:src/text1b.c against the s14 saved chassis; regenerated and re-measured E2 (the current candidate.c body) and E3 through `sandbox func_80060A68 --disable all`. Checked every s16 body for FAKE constructs before measuring (tools/fake_ablate.py has nothing to ablate on a FAKE-free body).
- result: Chassis byte-identical. E2 = 2 / build 66 / target 66 with two 0x10 loads; E3 = 3 / build 66 / target 66 with three loads. Both match s15's recorded numbers exactly. No FAKE construct in either, nor in any of the ~270 bodies measured this session.
- verdict: CONFIRMED

## [s16] Target's exact three-load geometry -- lw $a1,0x10($v1) at slot 11 and lw $a0,0x10($v1) at slots 19 and 22 -- is emitted by an ordinary-C body at 66 instructions.
- mechanism: Three separate 0x10 loads require a store between each consecutive pair of *(s32 *)(outer + 0x10) reads (the s15 cse rule). Putting the p10 read AFTER the 0x1A store, with the D_800A3478 gp store left after it rather than inside the +2 read's range, gives the 0x18 store and the 0x1A store as the two separators and leaves p10's pseudo a long enough range (birth 40, death 48) to win $a1 in local-alloc.
- probe: Built K7 = C1,C2,C3,S1,S2,S3,P,S4,S5,S6,S7,S8 and measured it with the sandbox, then disassembled tmp/sandbox/func_80060A68/text1b.o and extracted the QTYDBG quantity table with tools/ra_solver/local_extract.py text1b --func func_80060A68.
- result: K7 = 5 / build 66 / target 66, with the three loads at exactly slots 11 ($a1), 19 ($a0) and 22 ($a0) -- target's load multiset, in target's slots, in target's registers. The whole residual is one seat: qty11 / reg74 (the +2 halfword value) birth 36 death 38 refs 2 order 5 of 25 got=2 ($v0) where target needs $a0, plus the tail order that follows (idx read hoisted into slot 25, addiu / sw %gp pushed to 27/28). L4 (idx moved into the +2 read's window on the same spine) is also 5 / 66 with the same got=2. Banked at memory/grind/func_80060A68/rejected/s16-K7-target-load-geometry-11-19-22-but-plus2-value-seats-v0-score5.c
- verdict: CONFIRMED

## [s16] The +2 halfword value's $a0 seat does not require the D_800A3478 gp store inside its range; any statement in that window that creates a short-lived maximal-priority $v0 pseudo buys it, and one such body carries both target seats at once.
- mechanism: local-alloc allocates quantities in descending priority pri = floor_log2(refs)*refs*size*10000/(death-birth) (local-alloc.c:1649-1685) by first fit (local-alloc.c:1563-1580). The +2 value has refs 2 and span 2, the maximum priority a refs=2 quantity can have, so it cannot be out-prioritised -- it can only be made to ENCLOSE a quantity that takes $v0 before it. The enclosed quantity's identity is free.
- probe: Moved the third COPY statement (not the gp store) into the window between the +2 read and the 0x1A store, leaving the gp store after the 0x1A store: L1 = C1,C2,S1,S2,C3,S3,P,S4,S5,S6,S7,S8. Measured, disassembled, and extracted the QTYDBG table. Repeated with each of the three copies (L2, L3, L8) and with eight surrounding orderings (V1, V2, V5, V6, V7, V8, V9, VA).
- result: L1 gives reg74 birth 34 death 38 got=4 ($a0) AND reg75 (p10) birth 40 death 48 got=5 ($a1) simultaneously, with three 0x10 loads at 66 instructions -- the first body in the campaign to hold both target seats at once. It measures 11 because the moved copy's own lw/lw/sw triple schedules at slots 21/23/26 instead of target's 16/18/20. s15's 'the gp store buys the seat' is therefore an instance of the mechanism, not the mechanism. Banked at memory/grind/func_80060A68/rejected/s16-L1-both-target-seats-but-moved-copy-schedules-late-score11.c
- verdict: CONFIRMED

## [s16] On the E2 spine (D_800A3478 gp store between the +2 read and the 0x1A store), no position of the p10 read among all nine spine gaps, crossed with every tail permutation and every idx position, emitted the third 0x10 load at slot 11.
- mechanism: The E2 spine is what buys the $a0 seat and target's 25/26/27 addiu / sw %gp / sh 0x1A group. If the third load could also be placed at slot 11 on that spine, the function would close. The frontier's next_probe asked for exactly this enumeration (separator geometries s15 did not reach, on the E2 chassis specifically).
- probe: 228 ordinary-C bodies, all measured with sandbox --disable all and disassembled. X sweep (108): spine [O,Z,C1,C2,C3,S1,S2,S5,S3] with P inserted at each of the 9 gaps x 12 tail permutations of {S4,S6,S7,S8} with S6 before S8. Y sweep (120): P at the 4 copy-region gaps x S4 at all 10 gaps x 3 tail permutations of {S6,S7,S8}. Load slots and registers recorded per body in tmp/grind/func_80060A68/s16/xsweep.log and ysweep.log.
- result: Minimum score across the union is 2 -- the existing E2 class, with four new members (X5_3, X7_3, X9_2, Y3_2_2). The third 0x10 load lands at slot 4 (when the p10 read precedes the D_800F10D0 zero store; 65 instructions), at slot 26 or 27 (when it follows the 0x1A store; 66 instructions), or the body costs 67 instructions -- in none of the 228 at slot 11.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis (byte-identical to tmp/grind/func_80060A68/s14/text1b.c.HEAD), 228 bodies, zero FAKE constructs in any of them

## [s16] Using a moved copy statement as the $v0 donor in the +2 read's window costs about nine score points in every ordering measured, because the copy's own three instructions schedule after the 0x18/0x1A block.
- mechanism: The donor statement's RTL is emitted where its source statement sits, and sched1/sched2 will not hoist a load/load/store triple back above the halfword block that now precedes it, so the copy lands at slots 21/23/26 instead of target's 16/18/20.
- probe: 12 ordinary-C bodies: L1, L2, L3, L8 (which of the three copies is moved into the window) and V1, V2, V5, V6, V7, V8, V9, VA (P / S4 / S5 / S7 positions around the L1 spine). Each measured with the sandbox and disassembled for load slots.
- result: Scores 10-17 at 66-68 instructions; the moved copy is at 21/23/26 in every one. Both target register seats are held throughout, so the seat is not what is being lost.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 12 bodies, zero FAKE constructs

## [s16] When the p10 read is placed before the D_800F10D0 zero store, its load is emitted at slot 4 and the body is 65 instructions in every ordering measured.
- mechanism: p10's load depends only on $v1 (outer), so once its insn exists at the top of the block it is unconditionally ready; sched2 spends it filling the load-delay slot after `lhu $v0,0x0($v1)`, which target leaves as a nop, and the body loses that nop.
- probe: 24 ordinary-C bodies: N1, N2, N5, N7, N8 plus every X1_* and X2_* row of the X sweep. Measured and disassembled; load slots recorded.
- result: Every body with the read before the zero store puts the load at slot 4. The best, N2 = O,P,Z,C1,C2,C3,S1,S2,S5,S3,S6,S4,S7,S8, is 2 / build 65 / target 66 -- target's whole stream (including the 25/26/27 addiu / sw %gp / sh 0x1A group and both halfword seats) shifted one slot earlier, one instruction short. Banked at memory/grind/func_80060A68/rejected/s16-N2-p10-read-before-zero-store-load-fills-target-nop-65insns-score2.c
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 24 bodies, zero FAKE constructs

## [s16] Writing each halfword read inline into its own store, eliminating the temp2 and p10 locals, measures 10-12 and does not reach the score-2 class.
- mechanism: Three inline reads are separated by their own stores, so three loads are guaranteed by the cse rule without any statement-position trick; the question was whether the named intermediates are load-bearing for the current class.
- probe: R1-R8: eight ordinary-C bodies with the +2 and +4 reads written inline into the 0x1A and 0x1C stores, sweeping the gp-store and idx positions around them. Measured with the sandbox.
- result: 66-68 instructions, score 10-12. The temp2 named intermediate is load-bearing for the score-2 class; removing it changes the pseudo set enough to lose both seats.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 8 bodies, zero FAKE constructs


## [s17] E2 and K7 re-measure exactly as s15/s16 recorded them on today's HEAD chassis, and no body in the campaign's live set carries a FAKE construct.
- mechanism: Mandated kill re-audit before any new probe. If the chassis had drifted, every banked spelling conclusion would need re-measuring.
- probe: Applied E2 (candidate.c's body) and K7 through tmp/grind/func_80060A68/s14/apply.py and measured both with `sandbox func_80060A68 --disable all`; disassembled both and extracted the QTYDBG quantity table with tools/ra_solver/local_extract.py.
- result: E2 = 2 / build 66 / target 66 with loads at slots 11 ($a1) and 19 ($a0); K7 = 5 / build 66 / target 66 with loads at 11 ($a1), 19 ($a0), 22 ($a0) and reg74 got=2. Identical to the s15/s16 records. No FAKE construct exists in either body, so tools/fake_ablate.py has nothing to ablate.
- verdict: CONFIRMED

### H-s17-1 (KILLED, instance) - redecomposing the three 0x10 reads never reaches the slot-11 load.
Probe: fifteen bodies that change the DECOMPOSITION rather than the statement order - A family (8),
p10 deleted entirely and the +4 read written inline; B family (3), p10 feeding the +0 read; C family
(4), p10 feeding the +2 read. Result: 3-8 at 66-68 instructions, and no `lw ?,0x10($v1)` at slot 11
in any of them; the late load lands at 18/21/24-28. Target's slot-11 load heads a long pointer pseudo
(slot 11 to slot 28); an inline read creates a two-insn pointer pseudo that local-alloc seats in
$v0/$a0 and sched2 does not hoist. Measured on today's HEAD chassis, zero FAKE constructs.

### H-s17-2 (CONFIRMED) - the residual is governed by four conditions and by which quantity can sit in which window.
R1 three loads: a store between each consecutive pair of 0x10 reads. R2 the +2 value seats $a0: its
range [S2,S3] must be LENGTHENED (a refs-2 span-2 quantity has priority 10000, the maximum, and
first-fits into $v0; local-alloc.c:1649-1685 / 1563-1580) so that a span-2 $v0 quantity inside it is
allocated first. R3 p10 seats $a1: a span-2 $v0 quantity inside [P,S6] plus `idx` (argument-suggested
$a0) live across it. R4 the load is emitted at slot 11: p10's read must not follow a gp-based store,
because sched2 disambiguates mem(v1+0x10) from the v1-based stores but not from
`sw $v0,%gp_rel(D_800A3478)($gp)`. The only two span-2 $v0 quantities the function owns are the addiu
halves of the two gp stores, so R2 and R4 compete for the same two statements. QTYDBG ground truth on
K7 (reg74 got=2, reg75 got=5), E3 (reg74 got=4, reg75 got=2), G1 (same as E3), H3 (reg74 got=4,
reg75 got=5).

### H-s17-3 (CONFIRMED) - target's complete instruction stream, both register seats and all three loads are held simultaneously by one ordinary-C body, with a single instruction out of place.
H3 = C1,C2,C3,S1,S2,S5,S3,P,S4,S7,S6,S8 measures 5 / build 67 / target 66 and is target's stream with
`lw $a1,0x10($v1)` relocated from slot 11 to slot 26 (the slot immediately after the
`sw $v0,%gp(D_800A3478)` that blocks it), everything between shifted one slot earlier, and a nop at
slot 32 as the 67th instruction. No FAKE construct. Banked at
rejected/s17-H3-target-stream-with-the-a1-load-displaced-by-the-gp-store-score5-67insns.c.

### H-s17-4 (KILLED, instance) - the D_800A347C addiu/store pair is not a usable window donor.
Probe: G2, G3, G4, G5, GA, HC, HD, HE - seven placements of S7 in and around the window between the
+2 read and the 0x1A store, on the K7 spine and its neighbours (this was s16's named next_probe).
Result: 7-9 at 66-67 instructions in every one. S7 is a gp store, so it buys the $a0 seat by R2 and
destroys the slot-11 load by R4 exactly as S5 does, and it additionally pulls target's slots 30/31
addiu/sw %gp(D_800A347C) pair into the 25/26 window. Measured on today's HEAD chassis, zero FAKE
constructs.

### H-s17-5 (KILLED, instance) - reading p10 among the copy statements does not by itself hoist its load to slot 11.
Probe: J4, J5, J6, J7 place the p10 read between the copies (a copy store is its cse separator, and
it precedes both gp stores) while D_800A3478 stays inside the +2 value's range. Result: both seats and
three loads, but the load is emitted at slot 23-24 and every body is 67 instructions (5-7). Being
textually early is not sufficient for the hoist. Measured on today's HEAD chassis, zero FAKE constructs.

### H-s17-6 (KILLED, instance) - a copy statement moved to separate the +2 read from the p10 read reproduces the s16 nine-point price on a new spine.
Probe: M1, M2, M3, M5, M6, M7 - the only non-gp store available between the +2 read and the p10 read
is a copy, so these bodies move copy 3 (or copy 2) there, letting P precede the gp store. Result:
M2, M6 and M7 DO emit the load at slot 11 in $a1 with both seats and three loads, but each is 67
instructions and scores 10-13; M1, M3, M5 are 11-15. The moved copy's lw/lw/sw triple schedules after
the 0x18/0x1A block in every one, exactly as H-s16-4 recorded on the L1 spine. Measured on today's
HEAD chassis, zero FAKE constructs.

## [s17] E2 and K7 re-measure exactly as s15/s16 recorded them on today's HEAD chassis, and no body in the campaign's live set carries a FAKE construct.
- mechanism: Mandated kill re-audit: chassis-relative conclusions are void if the chassis drifted, and a lever measured with a FAKE carrier on the same pseudo is not a kill.
- probe: Applied E2 (candidate.c's body) and K7 through tmp/grind/func_80060A68/s14/apply.py, measured both with `sandbox func_80060A68 --disable all`, disassembled both, and extracted the QTYDBG quantity table with tools/ra_solver/local_extract.py text1b --func func_80060A68.
- result: E2 = 2 / build 66 / target 66, loads at slots 11 ($a1) and 19 ($a0). K7 = 5 / build 66 / target 66, loads at 11 ($a1), 19 ($a0), 22 ($a0), reg74 got=2. Both identical to the recorded numbers. Neither body contains a FAKE construct, so tools/fake_ablate.py has nothing to ablate; the s15/s16 instance kills are chassis-current.
- verdict: CONFIRMED

## [s17] Fifteen bodies that redecompose the three *(s32 *)(outer + 0x10) reads -- the +4 read written inline with no pointer local (A family), p10 feeding the +0 read instead (B family), p10 feeding the +2 read instead (C family) -- measure 3 to 8 and put no lw ?,0x10($v1) at slot 11.
- mechanism: Target's slot-11 load heads a pointer pseudo that lives from slot 11 to the lhu $a1,0x4($a1) at slot 28. An inline *(s32 *)(outer + 0x10) inside a halfword read creates a two-insn pointer pseudo instead, which local-alloc seats in $v0/$a0 and sched2 does not hoist.
- probe: A1-A8, B1-B3, C1x-C4x: fifteen ordinary-C bodies generated by tmp/grind/func_80060A68/s17/genA.py, each measured with `sandbox func_80060A68 --disable all` and disassembled; load slots and registers recorded per body.
- result: Scores 3-8 at 66-68 instructions. Third load at slots 18/21/24-28 in $v0 or $a0; never at 11. Best is A2 = 3 / 66 (banked at rejected/s17-A2-plus4-read-inline-no-p10-local-third-load-never-hoists-score3.c). The p10 local is load-bearing structure, not a spelling choice.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis (src/text1b.c restored to HEAD after the session), 15 bodies, zero FAKE constructs in any of them

## [s17] Four conditions govern this residual and the two gp stores are the only statements that can satisfy two of them, so those two conditions compete for the same two statements.
- mechanism: R1 three loads = a store between each consecutive pair of 0x10 reads. R2 the +2 value seats $a0 = its range must be lengthened, because with its read and store adjacent it has refs 2 / span 2 = priority 10000 (floor_log2(refs)*refs*size*10000/(death-birth), local-alloc.c:1649-1685) which first-fits to $v0 (local-alloc.c:1563-1580); only a span-2 $v0 quantity allocated earlier and enclosed by its range pushes it to $a0. R3 p10 seats $a1 = a span-2 $v0 quantity inside [p10 read, +4 read] plus idx (argument-suggested $a0) live across it. R4 the load is emitted at slot 11 = the p10 read must not follow a gp-based store, since sched2 disambiguates mem(v1+0x10) from the v1-based 0x18/0x1A/0x20 stores but not from sw $v0,%gp_rel(D_800A3478)($gp). The function's only span-2 $v0 quantities are the addiu halves of the two gp stores.
- probe: QTYDBG quantity tables extracted for K7, E3, G1 and H3 with tools/ra_solver/local_extract.py, cross-read against 56 disassemblies measured this session.
- result: K7 (nothing between the +2 read and the 0x1A store): reg74 birth 36 death 38 got=2, reg75 birth 40 death 48 got=5, load at 11. E3/G1 (gp store in the window): reg74 got=4, reg75 got=2, load at 26. H3 (gp store in the window, D_800A347C inside p10's range): reg74 got=4 AND reg75 got=5, load at 26 because the p10 read follows the gp store. Every s17 body with the p10 read after either gp store emits the load in the slot immediately after that store.
- verdict: CONFIRMED

## [s17] One ordinary-C body, H3, emits target's complete 66-instruction opcode sequence with both target register seats and all three 0x10 loads, differing only by the position of the lw $a1,0x10($v1) and the nop that follows from it.
- mechanism: H3 = C1,C2,C3,S1,S2,S5,S3,P,S4,S7,S6,S8 satisfies R1, R2 (D_800A3478 inside the +2 value's range) and R3 (D_800A347C inside p10's range) but violates R4 (the p10 read follows the D_800A3478 gp store), so sched2 chains the load behind that store.
- probe: Generated by tmp/grind/func_80060A68/s17/genH.py, measured with the sandbox, disassembled to tmp/grind/func_80060A68/s17/H3.dis and diffed slot-by-slot against tmp/grind/func_80060A68/s14/target.ops; QTYDBG table extracted; sched2 dump captured to tmp/grind/func_80060A68/s17/H3.sched2.
- result: 5 / build 67 / target 66. The load sits at slot 26 (immediately after sw $v0,%gp(D_800A3478) at 25) instead of target's slot 11; slots 12-25 shift one earlier; a nop appears at slot 32 as the 67th instruction. reg74 got=4, reg75 got=5. No FAKE construct. Banked at rejected/s17-H3-target-stream-with-the-a1-load-displaced-by-the-gp-store-score5-67insns.c. This is the first body in the campaign to hold both seats, three loads and target's complete opcode sequence at once.
- verdict: CONFIRMED

## [s17] The D_800A347C addiu/store pair used as the donor in the window between the +2 read and the 0x1A store measures 7 to 9 in all eight placements tried.
- mechanism: S7 is a gp store exactly like S5, so it buys the $a0 seat by R2 and blocks the p10 load by R4 identically, and it additionally drags target's slots 30/31 addiu / sw %gp(D_800A347C) pair into the 25/26 window where target puts the D_800A3478 pair.
- probe: G2, G3, G4, G5, GA, HC, HD, HE -- eight ordinary-C bodies sweeping S7 through and around that window on the K7 spine and its neighbours; this was s16's named next_probe. Each measured with the sandbox and disassembled.
- result: 7-9 at 66-67 instructions in every one; no body reaches the score-2 class and none puts a load at slot 11. Banked at rejected/s17-G3-D_800A347C-pair-as-the-window-donor-on-the-K7-spine-score7.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 8 bodies, zero FAKE constructs

## [s17] Reading p10 among the copy statements, with the D_800A3478 gp store inside the +2 value's range, gives both register seats and three loads but emits the load at slot 23 or 24 and costs 67 instructions.
- mechanism: A copy store is a cse separator for the p10 read and the read then precedes both gp stores, so R1 and R4's textual precondition hold; but the copy region's own scheduling changes and the load is not selected for slot 11.
- probe: J4, J5, J6, J7 -- four ordinary-C bodies placing the p10 read at each copy-region gap with the gp store left in the +2 window. Measured with the sandbox and disassembled.
- result: 5-7 at 67 instructions; loads at 18, 23 ($v0) and 24 ($a1) with a nop at slot 21 where target puts lw $a0,0x10($v1). Being textually early is not sufficient for the slot-11 hoist. Banked at rejected/s17-J4-p10-read-among-the-copies-with-the-gp-store-in-the-window-score5-67insns.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 4 bodies, zero FAKE constructs

## [s17] Moving a copy statement to act as the non-gp separator between the +2 read and the p10 read does reach the slot-11 load with both seats, but every such body is 67 instructions and scores 10 to 15.
- mechanism: The copy is the only non-gp store that can sit between those two reads, which lets the p10 read precede both gp stores (R4) while a gp store still occupies the +2 value's window (R2); but the moved copy's own lw/lw/sw triple schedules after the 0x18/0x1A block instead of target's slots 16/18/20.
- probe: M1, M2, M3, M5, M6, M7 -- six ordinary-C bodies from tmp/grind/func_80060A68/s17/genM.py, each measured with the sandbox and disassembled for load slots and registers.
- result: M2, M6, M7 emit lw $a1,0x10($v1) at slot 11 with reg74 in $a0 and reg75 in $a1 and three loads -- but at 67 instructions and 10-13 points. M1, M3, M5 are 11-15. This re-confirms s16's H-s16-4 price on a different spine.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 6 bodies, zero FAKE constructs


## s18 (2026-09-03, rederive modality) - 74 bodies, all ordinary C, zero FAKE constructs

Chassis re-audit first (mandated): E2 = 2 / build 66 / target 66, K7 = 5 / 66, H3 = 5 / 67 on
today's HEAD, all identical to the s15-s17 records.  No FAKE construct exists in any live body, so
`tools/fake_ablate.py` had nothing to ablate; the s15-s17 instance kills remain chassis-current.

### H-s18-1  KILLED (instance)
**Statement.** On the E2/K7 chassis, splitting a copy statement into a value local plus a separate
store (`cv = *(s32 *)(*(s32 *)(outer + 0xC) + 8); ... *(s32 *)(outer + 0x28) = cv;`) and using the
detached store as the cse separator between the +2 read and the p10 read measures 12-14 across the
nine bodies V1-V9.
**Mechanism probed.** s17's frontier named "a span-2 refs-2 quantity that can take $v0 strictly
inside the +2 halfword value's live range without being one of the two gp stores".  A detached copy
store is a store with no addiu, so the hope was a free separator.
**Result.** The control VC (halves left adjacent) measures 5 / 67 - bit-identical to the unsplit H3 -
so the split itself is byte-neutral.  As a separator it is not: `cv` becomes a long-lived pseudo, is
allocated early, and displaces the three quantities that matter.  V1 = 14 / 67 with loads at 11
($a2), 19 ($v0), 22 ($v0); V3 = 14 / 66; V7 (C2 split) = 12 / 66; V8 (C1 split) = 13 / 66.  Splitting
is strictly worse than moving the whole statement (M family, 10-13).
**measured_on.** today's HEAD chassis, 10 bodies (V1-V9 + control VC), zero FAKE constructs.

### H-s18-2  KILLED (instance)  - and it CORRECTS s17's R4
**Statement.** Placing every `*(s32 *)(outer + 0x10)` read ahead of both gp stores by using the 0x1C
store as the second cse separator (spine `S1, P, S6, S8, S2, S5, S3, ...`) leaves the p10 load at
slot 25 in $v0 in all nine bodies Z1-Z9, at 9-11 / 66.
**Mechanism probed.** s17 recorded R4 as a purely positional law ("the p10 read must not follow a
gp-based store").  If that were sufficient, this spine - which satisfies it for all three reads -
would give the slot-11 load.
**Result.** It does not.  With S6 immediately after P, p10's pseudo has span 2 and no $v0 donor lies
inside [P, S6], so R3 fails and local-alloc seats p10 in $v0; a $v0-seated pointer load cannot be
hoisted past the $v0 traffic in slots 12-24.  The slot-11 load is a consequence of the $a1 SEAT, and
R4 is a second necessary condition layered on top of it, not a sufficient one.  This is the single
most important correction s18 makes to the recorded law.
**measured_on.** today's HEAD chassis, 9 bodies, zero FAKE constructs.

### H-s18-3  KILLED (instance)
**Statement.** On the spine where a copy store is the cse separator between the +2 read and the p10
read, no member measured this session is simultaneously 66 instructions and emits its `lw $a1,0x10($v1)`
at slot 11: the 30 bodies T11-T65 (6 copy-block prefixes x 5 tail orders) and the 17 bodies M1-M8 /
W1-WB all land either at 67 instructions with the slot-11 load or at 66 instructions with the load at
slot 24-25.
**Mechanism probed.** The chain R3 (gp donor inside [P, S6]) + corrected R4 (every 0x10 read upstream
of that gp store) + R1 (a store between consecutive reads) forces the second separator to be a copy
store, because the 0x1A store must sit downstream of a gp store for R2 and the 0x1C store is
downstream of S6.  The question was whether the copy store's own three instructions can be made to
land in target's slots 16/18/20.
**Result.** They cannot on any prefix tried.  Only the prefix `C1,C2,S1,S2,C3,P` keeps the slot-11
$a1 load, and in all five tails it is 67 instructions, best score 10 (T61/T62): local-alloc gives the
moved copy $v0 for both its pointer and its value, its two loads end up adjacent, and a load-delay nop
is inserted (M2 slots 21/23 with the nop at 24).  Target's C3 instead uses $a0 for the pointer at slot
16 and $v0 for the value at slot 18, with the 0x24 store filling the delay.  Prefixes that move C1 or
C2 instead reach 66 instructions (T41/T42/T51/T52/T55) but drop the p10 load to slot 24-25.
**measured_on.** today's HEAD chassis, 47 bodies, zero FAKE constructs.

### H-s18-4  CONFIRMED
**Statement.** `Y1 = C1,C2,C3, S1, P, S4, S5, S6, S2, S7, S3, S8` reaches 5 / build 66 / target 66
with slots 0-20 byte-identical to target and loads at 11 ($a1), 19 ($a0), 26 ($v0), moving no copy
statement and inventing no local.
**Mechanism.** The p10 read placed immediately after the 0x18 store uses that store as its cse
separator for free; the first gp store, sitting inside [P, S6], supplies R3's donor and buys the $a1
seat, so the load hoists to slot 11.
**Result.** Confirmed at 5 / 66.  Its residual is exactly one instruction's provenance: the +2 read
is downstream of the gp store, so its load is pinned at slot 26 in $v0 instead of slot 22 in $a0 -
the mirror image of H3's residual.  Y2/Y3/Y4/Y5/Y7/YA/YB permute the spine at 6-9.  Banked at
`rejected/s18-Y1-p10-read-hoisted-above-the-plus2-read-third-load-pinned-behind-gp-store-score5-66insns.c`.

### H-s18-5  CONFIRMED - and it REFINES s16's "temp2 is load-bearing"
**Statement.** `U2 = C1,C2,C3, S1, S3i, P, S4, S5, S6, S7, S8`, with the +2 read written inline into
its own 0x1A store and no `temp2` local declared anywhere, measures 5 / build 66 / target 66 and
reproduces target's exact three-load geometry: `lw $a1,0x10($v1)` at slot 11, `lw $a0,0x10($v1)` at
slots 19 and 22.
**Mechanism.** A fused read+store statement carries its own cse separator, so the 0x1A store
separates the +2 read from the p10 read at zero instruction cost - the thing the copy store was being
charged a nop for.  s16's R1-R8 (which measured 10-12) inlined all three reads AND deleted `p10`;
inlining only the +2 read while keeping `p10` costs nothing.
**Result.** Confirmed at 5 / 66, residual slots 24-29, identical single-seat residual to K7 (the +2
halfword value in $v0 where target needs $a0) but reached with one fewer local.  Note the structural
cost: a fused read+store has no source-level interior, so on the U spine R2's donor site does not
exist at all.  Banked at
`rejected/s18-U2-inline-0x1A-store-gives-targets-exact-load-geometry-without-temp2-score5-66insns.c`.

## [s18] Splitting a copy statement into a value local plus a detached store (cv = *(s32 *)(*(s32 *)(outer + 0xC) + 8); ... *(s32 *)(outer + 0x28) = cv;) and using the detached store as the cse separator between the +2 read and the p10 read measures 12-14 across the nine bodies V1-V9.
- mechanism: s17's frontier asked for a span-2 refs-2 quantity that can take $v0 strictly inside the +2 value's live range without being one of the two gp stores. A detached copy store is a store with no addiu, so it looked like a free separator that leaves the gp stores available as donors.
- probe: Nine bodies V1-V9 (C3, C2 and C1 each split in turn, crossed with the tail orders that keep a gp addiu inside both [S2,S3] and [P,S6]) plus a control VC in which the two halves stay adjacent; each applied to src/text1b.c with tmp/grind/func_80060A68/s14/apply.py and scored with sandbox --disable all, with the 0x10 load slots and registers read out of the objdump.
- result: The control VC measures 5 / build 67 - bit-identical to the unsplit H3 - so the split itself is byte-neutral. As a separator it is not: cv becomes a long-lived pseudo, is allocated early and displaces the three quantities that matter. V1 = 14 / 67 with loads at 11 ($a2), 19 ($v0), 22 ($v0); V3 = 14 / 66; V7 (C2 split) = 12 / 66; V8 (C1 split) = 13 / 66. Splitting a copy is strictly worse than moving the whole statement (M family, 10-13). Banked at rejected/s18-V1-split-copy-store-as-separator-cv-pseudo-destroys-all-three-seats-score14.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis (src/text1b.c restored to HEAD after the session), 10 bodies, zero FAKE constructs in any of them

## [s18] Placing every *(s32 *)(outer + 0x10) read ahead of both gp stores by using the 0x1C store as the second cse separator (spine C1,C2,C3,S1,P,S6,S8,S2,S5,S3,S7) leaves the p10 load at slot 25 in $v0 in the nine bodies Z1-Z9, at 9-11 / build 66.
- mechanism: s17 recorded R4 as a purely positional law - the p10 read must not follow a gp-based store, because sched2 cannot disambiguate sw $v0,%gp_rel(D_800A3478)($gp) from mem(v1+0x10). If that condition were sufficient, this spine, which satisfies it for all three reads, would produce target's slot-11 load.
- probe: Nine bodies Z1-Z9 permuting idx, the 0x1C store and the two gp stores on that spine; each measured with sandbox --disable all and the 0x10 load slots/registers read from the objdump.
- result: It does not produce the slot-11 load: 9-11 / 66 with loads at 19 ($a0), 22 ($a0), 25 ($v0) in every one. With the +4 read immediately after the p10 read, p10's pseudo has span 2 and no $v0 donor lies inside [P, S6], so R3 fails, local-alloc seats p10 in $v0, and a $v0-seated pointer load is not hoisted past the $v0 traffic in slots 12-24. This corrects the recorded law: the slot-11 load follows from the $a1 SEAT (R3), and R4 is a second necessary condition layered on it rather than a sufficient one. Banked at rejected/s18-Z4-all-three-reads-precede-both-gp-stores-but-no-donor-in-p10-range-score9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 9 bodies, zero FAKE constructs in any of them

## [s18] On the spine where a copy store is the cse separator between the +2 read and the p10 read, the 47 bodies measured this session (T11-T65, M1-M8, W1-WB) land either at 67 instructions with the lw $a1,0x10($v1) at slot 11 or at 66 instructions with that load at slot 24-25; none of the 47 is both 66 instructions and slot-11.
- mechanism: R3 (the donor is a gp addiu, so a gp store sits after the p10 read) plus the corrected R4 (every 0x10 read must precede that gp store) plus R1 (a store between consecutive 0x10 reads) leaves the copy store as the only candidate for the second separator, because the 0x1A store must sit downstream of a gp store for R2 and the 0x1C store is downstream of the +4 read. The open question was whether the moved copy's own three instructions can be made to land in target's slots 16/18/20.
- probe: 30 bodies T11-T65 - six copy-block prefixes (which copy is the separator, where the 0x18/0x1A pair sits inside the copy block) crossed with five tail orders - plus the 17 earlier-shape bodies M1-M8 and W1-WB; each measured with sandbox --disable all, recording build_insns and the 0x10 load slots/registers, and M2's objdump read instruction-by-instruction to locate the 67th instruction.
- result: Only the prefix C1,C2,S1,S2,C3,P keeps the slot-11 $a1 load, and in all five tail orders it is 67 instructions, best score 10 (T61/T62, loads 11 $a1 / 17 $a0 / 20 $a0). The 67th instruction is always the same load-delay nop: local-alloc gives the moved copy $v0 for both its pointer and its value, so its lw $v0,0xC($v1) and lw $v0,0x8($v0) end up adjacent (M2 slots 21/23, nop at 24), whereas target's C3 uses $a0 for the pointer at slot 16 and $v0 for the value at slot 18 with the 0x24 store filling the delay. Prefixes that move C1 or C2 instead reach 66 instructions (T41/T42/T51/T52/T55, score 15-16) but drop the p10 load to slot 24-25. Banked at rejected/s18-T61-copy-store-separator-keeps-slot11-a1-load-but-costs-a-load-delay-nop-67insns.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 47 bodies, zero FAKE constructs in any of them

## [s18] Y1 = C1,C2,C3,S1,P,S4,S5,S6,S2,S7,S3,S8 reaches 5 / build 66 / target 66 with slots 0-20 byte-identical to target and 0x10 loads at slots 11 ($a1), 19 ($a0) and 26 ($v0), while moving no copy statement and inventing no local.
- mechanism: The p10 read placed immediately after the 0x18 store uses that store as its cse separator at zero cost; the first gp store, sitting inside [p10 read, +4 read], supplies R3's donor and buys the $a1 seat, so the pointer load hoists to slot 11.
- probe: Eleven bodies Y1-YB sweeping idx, the +4 read, the 0x1C store and the two gp stores around that spine; measured with sandbox --disable all with the 0x10 load slots/registers read from the objdump, and Y1's stream compared slot-by-slot against asm/funcs/func_80060A68.s.
- result: Confirmed at 5 / 66. Y6/Y8/Y9 are the same class; Y2/Y3/Y4/Y5/Y7/YA/YB measure 6-9. Y1's residual is one instruction's provenance: because the +2 read is downstream of the gp store, its address load is pinned at slot 26 in $v0 instead of slot 22 in $a0, and the tail (idx read, 0x1A store, addiu/gp-store pair) follows from that. It is the mirror image of s17's H3 residual and the first body in the campaign to hold the slot-11 $a1 load at 66 instructions without moving a copy statement. Banked at rejected/s18-Y1-p10-read-hoisted-above-the-plus2-read-third-load-pinned-behind-gp-store-score5-66insns.c.
- verdict: CONFIRMED

## [s18] U2 = C1,C2,C3,S1,S3i,P,S4,S5,S6,S7,S8, with the +2 read written inline into its own 0x1A store and no temp2 local declared anywhere, measures 5 / build 66 / target 66 and reproduces target's exact three-load geometry: lw $a1,0x10($v1) at slot 11 and lw $a0,0x10($v1) at slots 19 and 22.
- mechanism: A fused read+store statement carries its own cse separator, so the 0x1A store separates the +2 read from the p10 read at zero instruction cost - which is exactly what the copy store was being charged a load-delay nop for.
- probe: Eight bodies U1-U8 built on the inline-0x1A spine with the tail permuted; measured with sandbox --disable all and U2's objdump compared slot-by-slot against asm/funcs/func_80060A68.s.
- result: Confirmed at 5 / 66, residual slots 24-29, the same single local-alloc seat as K7 (the +2 halfword value lands in $v0 where target needs $a0, so the 0x1A store fires at slot 26 ahead of the addiu/gp-store pair instead of behind it) but reached with one fewer local. This refines s16's finding that the temp2 named intermediate is load-bearing: that was measured on R1-R8, which inlined all three reads AND deleted p10; with p10 kept, inlining only the +2 read costs nothing. The structural cost of the U spine is that a fused read+store has no source-level interior, so R2's donor site does not exist on it. U1/U3-U8 measure 5-9. Banked at rejected/s18-U2-inline-0x1A-store-gives-targets-exact-load-geometry-without-temp2-score5-66insns.c.
- verdict: CONFIRMED


## s19 (2026-09-03, structural modality) - 71 bodies, zero FAKE constructs

Chassis re-audit first: CU2 = 5 / 66, CE2 = 2 / 66, D2 = 2 / 66 (an independent re-spelling of
E2), all identical to the s15-s18 records; nothing to ablate.

**H-s19-0 (CONFIRMED, and the session's headline).** The floor-2 body's entire residual is one
instruction, and it has a free slot.  Slot-for-slot, E2 == target except target's third
`lw $a0,0x10($v1)` (slot 23) is a `nop` in E2, and the `lhu $a0,0x2(...)` at slot 25 consequently
reads `$a1` (p10's register) rather than `$a0`.  Probe: `cmp.sh E2` against `target.txt`.
Because the missing load's slot is already a load-delay nop, a cse separator between the +2 read
and the p10 read that adds no instruction and relocates no load makes the body a byte match.

**H-s19-1 (CONFIRMED).** Splitting the 0x18 halfword store off its read is byte-neutral and
yields a free non-gp separator.  `S1a = temp0 = *(u16 *)(*(s32 *)(outer + 0x10) + 0);` /
`S1b = *(u16 *)(outer + 0x18) = temp0;`; control X1 = 5 / 66 with loads 11/19/22, byte-identical
to the unsplit U2.  X2 (S1b placed in the +2-read-to-p10-read gap) = 4 / build 65 / target 66 and
is target's whole stream minus exactly one instruction.

**H-s19-2 (KILLED, instance).** On the split-0x18 spine the +0/+2 gap can be covered by hoisting
the +0 read above the copy block so the copy stores separate it, at no instruction cost.
Measured N1-NC and R1-RC (24 bodies, today's HEAD, zero FAKE constructs): 12-17 at 65-67.  The
hoisted read's own load is emitted at slot 11-16 in $a0 - the seat target gives p10 - and the
copy block's $v0/$a0 traffic collapses.  R1 has all three loads at 66 instructions and scores 14.

**H-s19-3 (KILLED, instance).** The p10 read hoisted above the copy block lets the copy stores
cover the first gap while the split 0x18 store covers the second, at 66 instructions.  Measured
Q1-QC (12 bodies, today's HEAD, zero FAKE constructs): 4-8 at 66-67; every three-load member is
67 instructions with the p10 load at slot 24-25, and the only 66-instruction member (Q5) has the
reads merged again.

**H-s19-4 (KILLED, instance).** A copy written through a named pointer intermediate
(`pc = *(s32 *)(outer + 0xC); ... *(s32 *)(outer + 0x28) = *(s32 *)(pc + 8);`) moves only the
store into the gap and leaves the load in the copy block, removing s18's load-delay nop.
Measured P1-PC (12 bodies, today's HEAD, zero FAKE constructs): all 67 instructions, 9-14.  P3's
disassembly shows the `pc` load sinking to slot 22 with its deref at 25 and a nop at 26 - the
pointer local does not decouple the load from its use, so the price is identical to moving the
whole statement.  This closes the s18 frontier item that proposed seating the moved copy's
pointer in $a0.

**H-s19-5 (KILLED, instance).** Placing the whole halfword block before the copy block makes the
three copy stores a free separator for the +2-read-to-p10-read gap.  Measured G1-GA (10 bodies,
today's HEAD, zero FAKE constructs): 15-21 at 66-67.  Three loads are restored, but the halfword
loads hoist to slots 10-11 and the copy block loses target's 10/11/17 geometry.

**H-s19-6 (KILLED, instance).** The `D_800F10D0` zero-store - the one store in the function never
priced as a separator - can cover the second gap for free because it already exists.  Measured
D1/D3/D5/D6 (today's HEAD, zero FAKE constructs): 15-19 at 65 instructions.  It does restore the
third load, but its five-instruction address computation (`lhu`/`sll`/`lui`/`addu`/`sw`) moves
with it and target's slots 4-9 are destroyed; the body also loses an instruction to the `*(u16 *)
outer` re-read merging with `idx`.  Control D2 (store left at the top) = 2 / 66 = E2.

**Where this leaves the separator law.** Every store the function owns is now priced in the
second-gap role: 0x18 (free, covers exactly one gap), 0x1A (barred by R2+R4), 0x1C (needs S6
adjacent to P, which removes R3's donor), the three copies (one load-delay nop by every route
tried: whole move, pointer-split, read hoisting, block reorder), and the D_800F10D0 zero-store
(head geometry).  The next attack is therefore NOT another store placement: it is either
(a) breaking the cse equivalence of two `*(s32 *)(outer + 0x10)` reads without adding an
instruction, or (b) finding a second donor so that R2's window no longer needs a gp store, which
would free the 0x1A store to act as a separator.

## [s19] The floor-2 candidate body (E2) differs from target by exactly one instruction, and the slot that instruction belongs in is already an unused load-delay nop.
- mechanism: Nothing separates `temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);` from `p10 = *(s32 *)(outer + 0x10);` in E2's statement order, so cse merges the two reads: target's third `lw $a0,0x10($v1)` at slot 23 never exists (E2 emits a nop there) and the following `lhu $a0,0x2(...)` at slot 25 reads $a1, p10's register, instead of $a0. Both register seats, the addiu/gp-store pair inside the +2 value's window and the whole tail are already target's.
- probe: Slot-for-slot disassembly diff of E2 against asm/funcs/func_80060A68.s lines 2-68 (tmp/grind/func_80060A68/s19/E2.dis + cmp.sh + target.txt), plus a control re-measurement of the same body generated independently as D2 (2 / build 66 / target 66).
- result: CONFIRMED. Exactly two differing slots, adjacent, both explained by the one missing load. Because the missing load's slot is a nop, a cse separator between those two statements that adds no instruction and relocates no load makes this body a byte match. This is the first localization of the residual to a single instruction with a free slot to receive it.
- verdict: CONFIRMED

## [s19] Splitting the 0x18 halfword store off its read (temp0 = *(u16 *)(*(s32 *)(outer + 0x10) + 0); ... *(u16 *)(outer + 0x18) = temp0;) is byte-neutral when the halves stay adjacent and yields a free non-gp cse separator that can be spent on either read gap.
- mechanism: Both the lhu and the sh already exist in every body, so detaching the sh adds no instruction; a halfword store to (v1+0x18) is a store sched2/cse cannot disambiguate from mem(v1+0x10), so it separates two 0x10 reads without the load-delay nop the copy store was being paid in s18.
- probe: Control X1 (C1,C2,C3,S1a,S1b,S3i,P,S4,S5,S6,S7,S8) vs the unsplit U2, and X2 (C1,C2,C3,S1a,S2,S1b,P,S5,S3,S4,S6,S7,S8) plus X3/X4/X6/X8, measured with sandbox --disable all on today's HEAD.
- result: CONFIRMED. X1 = 5 / build 66 / target 66 with loads at 11 ($a1) / 19 ($a0) / 22 ($a0), byte-identical to the unsplit U2, so the split costs nothing. X2 = 4 / build 65 / target 66 and is target's whole instruction stream minus exactly one instruction (the third lw), the smallest instruction-count residual in the campaign. Spending the separator on the +2-to-p10 gap opens the +0/+2 gap: two gaps, one free separator.
- verdict: CONFIRMED

## [s19] On the split-0x18 spine the remaining read gap can be covered at zero instruction cost by hoisting a 0x10 halfword read above the copy block, so the three copy stores separate it without any copy being moved.
- mechanism: The copy stores already sit between the copy block and the halfword block; moving only a READ above them borrows them as cse separators without relocating a copy, which is what cost s18's copy-move family a load-delay nop.
- probe: N1-NC and R1-RC, 24 bodies sweeping which read is hoisted, its position inside the copy block, and five tail orders; sandbox --disable all on today's HEAD.
- result: KILLED. 12-17 at 65-67 instructions. The hoisted read's own load is emitted at slot 11-16 in $a0 - the seat target gives p10 - and the copy block's $v0/$a0 traffic collapses. R1 (S1a,C1,C2,C3,S2,S1b,P,S5,S3,S6,S4,S7,S8) has all three loads at 66 instructions and still scores 14.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis (src/text1b.c restored to HEAD after the session), 24 bodies, zero FAKE constructs in any of them

## [s19] Hoisting the p10 read above the copy block lets the copy stores cover the first read gap while the split 0x18 store covers the second, keeping the body at 66 instructions with all three loads.
- mechanism: With P textually first, the three copy stores lie between the p10 read and the halfword reads for free, and S1b covers the remaining gap; every 0x10 read then precedes both gp stores (R4) while the D_800A3478 addiu still sits inside the +2 value's window (R2).
- probe: Q1-QC, 12 bodies (P at four positions relative to the copy block x split/unsplit 0x18 x tail permutations); sandbox --disable all on today's HEAD.
- result: KILLED. 4-8 at 66-67. Every three-load member is 67 instructions with the p10 load at slot 24-25 (not slot 11-12), and the only 66-instruction member (Q5) has the reads merged again at 4 / 66 with two loads.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 12 bodies, zero FAKE constructs in any of them

## [s19] Writing a copy through a named pointer intermediate (pc = *(s32 *)(outer + 0xC); ... *(s32 *)(outer + 0x28) = *(s32 *)(pc + 8);) moves only the copy's STORE into the read gap while its load stays in the copy block, removing the load-delay nop the whole-statement move costs.
- mechanism: s18's frontier item 1 attributed the 67th instruction to local-alloc giving the moved copy $v0 for both its pointer and its value; a named pointer local was expected to keep the 0xC load in place and let only the sw travel.
- probe: P1-PC, 12 bodies (C3 and C2 split through pc, pc's read at four positions, five tail orders); sandbox --disable all on today's HEAD, plus a slot-for-slot diff of P3.
- result: KILLED. All 12 bodies are 67 instructions, 9-14. P3's disassembly shows the pc load SINKING to slot 22 with its deref at 25 and a nop at 26 - the pointer local does not decouple the load from its use, so the price is identical to moving the whole statement. P3/P6 do hold the slot-12 $a1 load with three loads at 10-11. This closes s18's frontier item 1.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 12 bodies, zero FAKE constructs in any of them

## [s19] Placing the whole halfword block before the copy block makes the three copy stores a free separator for the +2-read-to-p10-read gap without moving any individual statement out of its block.
- mechanism: A wholesale block swap keeps every statement's neighbours intact, so the copy stores land after both halfword reads and separate them from the p10 read at no instruction cost.
- probe: G1-GA, 10 bodies (block swap x split/unsplit 0x18 x interleavings x tail orders); sandbox --disable all on today's HEAD.
- result: KILLED. 15-21 at 66-67. Three loads are restored in every member, but the halfword loads hoist to slots 10-11 and the copy block loses target's 10/11/17 lw 0xC geometry entirely.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 10 bodies, zero FAKE constructs in any of them

## [s19] The D_800F10D0 zero-store at the top of the function - the one store never priced as a cse separator - can cover the second read gap for free because it already exists in the body.
- mechanism: *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0; is a store to a computed global address that cse cannot disambiguate from mem(v1+0x10), so relocating the statement should separate two 0x10 reads without adding an instruction.
- probe: D1/D3/D5/D6 (zero-store moved into or near the gap) against control D2 (zero-store left at the top); sandbox --disable all on today's HEAD.
- result: KILLED. 15-19 at 65 instructions. The store does restore the third load, but its five-instruction address computation (lhu / sll / lui / addu / sw) travels with it and target's slots 4-9 are destroyed; the body also loses an instruction as the *(u16 *)outer re-read merges with idx. Control D2 = 2 / build 66 / target 66, reproducing E2 exactly.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 4 bodies plus control, zero FAKE constructs in any of them

## s20 (2026-09-03, structural modality) - hypotheses measured

**H-s20-1 (KILLED, class).** *No construct that emits neither a memory store, nor a call, nor a
redefinition of the base register `outer` can make the +2 read and the p10 read non-equivalent to
the first cse pass, whatever its C spelling.*
Mechanism: the entry cse hashes for these reads is `(mem:SI (plus (reg 72) (const_int 16)))`,
whose address is varying (`cse_rtx_addr_varies_p` true because the address contains a pseudo), so
`invalidate_memory` (tools/gcc-2.7.2/cse.c:1701) removes it on every invalidation regardless of
the `all`/`nonscalar` refinement that shields fixed-address scalars.  `invalidate_memory` is
reachable only from `invalidate_from_clobbers` (cse.c:7599) when `note_mem_written` set
`writes_memory.var` - which happens only when the written rtx is a `MEM` - and from `cse_insn`
(cse.c:7246) on a non-const `CALL_INSN`.
Probe: dumps (`text1b.rtl` three reads at insns 46/53/58, `text1b.cse` two) plus the cse.c
predicate read.  Predicate cite: `tools/gcc-2.7.2/cse.c:1701`.
Verdict: KILLED, class.  This closes s19's frontier item 1 ("break the cse equivalence without a
store").

**H-s20-2 (KILLED, instance).** *A byte-neutral redefinition of `outer` between the +2 read and
the p10 read invalidates the cse entry and restores the third load.*
Probe: Q1 (`outer = outer;` on the E2 spine).  Result: 2 / build 66 / target 66 with TWO loads -
byte-identical to the control, because `store_expr` returns the same rtx for both sides of a
self-assignment and emits no insn at all, so cse never sees a redefinition.  The spelling that
does emit one (Q5, `outer = D_800A3468;`) pays the gp load: 26 / 68.
Measured on: today's HEAD chassis, 2 bodies plus control, zero FAKE constructs in any of them.

**H-s20-3 (KILLED, instance).** *The declared width of the +2 value changes its local-alloc
priority or seat.*
Probe: D1 (`u16`) / D2 (`s32`) / D3 (`u32`) on the E2 spine.  Result: all three 2 / 66, byte-
identical.  Both refs keep the same `lhu`/`sh` opcodes and the quantity's `size` stays 1.
Measured on: today's HEAD chassis, 3 bodies, zero FAKE constructs in any of them.

**H-s20-4 (KILLED, instance).** *A non-store statement placed between the +2 read and the 0x1A
store lengthens the +2 value's live range enough to push it out of `$v0` (R2), so the 0x1A store
can be moved above `P` and serve as gap 2's free separator.*
Probe: Q2 / R3 / R1 / R2 / R6 / R7 (the 0x1A store above `P`, `idx` used as the window filler in
every position).  Result: Q2 and R3 reach 5 / build 66 / target 66 with all three
`lw ?,0x10($v1)` at target's exact slots and registers (12/$a1, 20/$a0, 23/$a0) and are target
through slot 24 and from slot 33 on - but the +2 value still seats in `$v0`.  QTYDBG on Q2 shows
reg74 birth=36 death=38 refs=2 ord=5 got=2: the span is STILL 2, because local-alloc runs after
sched1 and sched1 scheduled `idx`'s `lhu` out of the window.  R2's filler must be a store, which
sched cannot move out of a store-store chain.
Measured on: today's HEAD chassis, 6 bodies + QTYDBG extraction, zero FAKE constructs in any of them.

**H-s20-5 (KILLED, instance).** *A copy statement moved into the [+2 read, 0x1A store] window
serves as both R2's span-2 `$v0` donor and gap 2's separator, so both gaps close for free.*
Probe: N1-N6 (each of the three copies, several tails).  Result: 9-12 / 66.  The moved copy's
`lw` leaves the copy block, so the two halfword pointer loads are emitted at slots 17/20 instead
of target's 20/23.  N7 (the `D_800F10D0` zero store used as the window filler) = 16 / 65.
Measured on: today's HEAD chassis, 7 bodies, zero FAKE constructs in any of them.

**H-s20-6 (KILLED, instance).** *Hoisting one of the two halfword reads into or above the copy
block lets a copy store act as the missing free separator.*
Probe: A1-A9 (the +2 read hoisted, with and without the split 0x18 store) and F1-F7 (the +0 read
hoisted, with the split 0x18 store spent on gap 2).  Result: 9-16 at 65-67.  Three loads at 66
instructions is reachable (A1/A2/A4/F1 = 12 / 66) but the hoisted read's own `lw` is always
emitted at slot 11-16 in `$a0`, taking the seat target gives p10 - the s19 N/R law, now
re-confirmed for both halfword reads and for the split-store spelling.
Measured on: today's HEAD chassis, 16 bodies, zero FAKE constructs in any of them.

**H-s20-7 (KILLED, instance).** *Reading the +2 halfword before the +0 halfword lets the 0x1A
store cover gap 1 and the 0x18 store cover gap 2, closing both for free.*
Probe: W1, W2, W6, W8.  Result: W8 = 7 / 66 with target's exact three-load geometry, W1 = 7 / 66,
W2/W6 = 7 / 67.  The reads order forces the 0x1A store ahead of the 0x18 store in the emitted
stream, which is seven instructions of reordering.
Measured on: today's HEAD chassis, 4 bodies, zero FAKE constructs in any of them.

**H-s20-8 (KILLED, instance).** *Placing `P` among the copies with the 0x1C store as gap 2's
separator and a copy value as R3's `$v0` donor recovers the slot-12 p10 load (the s18 Z family
failed only for lack of that donor).*
Probe: Z1, Z2, Z4, Z6.  Result: 10-14 / 67-68 with the p10 load at 22-23 in `$v0`.  The donor is
not the missing ingredient; `P` textually inside the copy block is itself disqualifying.
Measured on: today's HEAD chassis, 4 bodies, zero FAKE constructs in any of them.

**H-s20-9 (CONFIRMED).** *Hoisting the p10 read ABOVE the `D_800F10D0` zero store - not merely
above the copy block - breaks the s4/s10 conservation law and yields three loads with every
target register seat.*
Probe: T1 = `P, Z0, C1,C2,C3, S1, S2, S5, S3, S6, S4, S7, S8`.  Result: **2 / build 65 / target
66**, three loads at 5 ($a1) / 19 ($a0) / 22 ($a0).  Slot-for-slot T1 is target's entire stream
with every seat correct (the +2 value is in `$a0`), differing only in that target's slot-5 `nop`
is T1's `lw $a1,0x10($v1)` and everything after is shifted one slot earlier.  The zero store is
an `$at`-based store sched2 cannot disambiguate from `lw ?,0x10($v1)`, so it is a hoist barrier:
above it the p10 load reaches slot 5, below it the load pins to slot 25 (T2/T3/T4 = 5 / 67).
T1 is the second score-2 body in the campaign and the first with three loads and all seats.

**H-s20-10 (KILLED, instance).** *Giving p10 an early consumer while keeping `P` just below the
zero store unpins its load onto slot 12 without losing the third load.*
Probe: V1-V6 (the +4 read and the 0x1C store moved to every position between the copy block and
the 0x1A store, with `P` immediately below the zero store).  Result: 7-12 / 66-67; the p10 load
is at 24-25 in every member and is in `$v0` in four of them.
Measured on: today's HEAD chassis, 6 bodies, zero FAKE constructs in any of them.

## [s20] No construct that emits neither a memory store, nor a call, nor a redefinition of the base register `outer` can make the +2 read and the p10 read non-equivalent to GCC 2.7.2's first cse pass, in any C spelling.
- mechanism: The entry cse hashes for both reads is (mem:SI (plus (reg 72) (const_int 16))); its address is varying (cse_rtx_addr_varies_p is true because the address contains a pseudo), so invalidate_memory removes it on every invalidation regardless of the all/nonscalar refinement that shields fixed-address scalars. invalidate_memory has exactly two callers: invalidate_from_clobbers (cse.c:7599), which fires only when note_mem_written set writes_memory.var, and that flag is set only when the written rtx is a MEM; and cse_insn (cse.c:7246) on a non-const CALL_INSN. The only other loss path is invalidate() on the base register itself.
- probe: pwsh tools/grinder/dump.ps1 func_80060A68 on the E2 body, sliced with tmp/grind/func_80060A68/s20/slice.py: text1b.rtl carries three (mem:SI (plus (reg/v 72) (const_int 16))) loads (insns 46, 53, 58), text1b.jump three, text1b.cse two, and loop/cse2/combine two. The predicate was then read out of tools/gcc-2.7.2/cse.c (note_mem_written, invalidate_memory, invalidate_from_clobbers, cse_insn).
- result: The merge is performed by the FIRST cse pass, on insn 58 (p10's read), exactly where s15 guessed and s19 asked for confirmation. Because the MEM's address varies, the coarse-grained protections in invalidate_memory never apply to it, so every memory store in the function invalidates it and nothing else does. s19's frontier item 1 -- 'find a separator that is not a store' -- is therefore closed at the predicate level, and the separator inventory banked in s19 is the complete inventory.
- verdict: KILLED
- kill_scope: class
- measured_on: today's HEAD chassis, the E2 body's full -da dump set, zero FAKE constructs
- predicate_cite: tools/gcc-2.7.2/cse.c:1701

## [s20] On the E2 spine, writing `outer = outer;` between the +2 read and the p10 read invalidates the cse entry for the base register and restores the third load at zero instruction cost.
- mechanism: invalidate() on a REG dest removes every hashed entry mentioning that register, so a redefinition of `outer` would drop both MEM entries; a self-assignment was the only spelling that could do that without emitting an instruction.
- probe: Q1 = C1,C2,C3,S1,S2,(outer = outer;),P,S5,S3,S6,S4,S7,S8 measured with sandbox --disable all; Q5 = the same slot filled with `outer = D_800A3468;`.
- result: Q1 = 2 / build 66 / target 66 with TWO loads, byte-identical to the control D1: expand_assignment/store_expr returns the same rtx for both sides of a self-assignment, so no insn is emitted at all and cse never sees a redefinition. Q5, the spelling that does emit one, pays the gp reload: 26 / build 68.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 2 bodies plus control, zero FAKE constructs in any of them

## [s20] On the E2 spine, changing the declared width of the +2 value (u16 vs s32 vs u32) changes its local-alloc priority or its register seat.
- mechanism: Structural type-narrowing lever from the codegen-technique catalogue: the quantity's mode feeds local-alloc's size term in the priority formula and its conflict test.
- probe: D1 (u16 temp2, control), D2 (s32 temp2), D3 (u32 temp2), all on the E2 statement order, measured with sandbox --disable all.
- result: All three measure 2 / build 66 / target 66 with loads at 12 ($a1) / 20 ($a0) and are byte-identical. Both refs keep the same lhu/sh opcodes and the quantity's size stays 1 word, so the declared width is inert on this residual.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 3 bodies, zero FAKE constructs in any of them

## [s20] Moving the 0x1A store above the p10 read (so it covers the second read gap) and inserting a non-store statement into the [+2 read, 0x1A store] window lengthens the +2 value's live range enough to push it out of $v0.
- mechanism: R2 as recorded in s17: the +2 value has refs 2, so with its two refs adjacent its span is 2 and its priority is the 10000 maximum a refs-2 quantity can reach (local-alloc.c:1649-1685), and first-fit hands it $v0. Lengthening the range was expected to drop its priority below a span-2 $v0 competitor's.
- probe: Q2 = C1,C2,C3,S1,S2,S4,S3,P,S5,S6,S7,S8 and R3/R1/R2/R6/R7 (idx used as the window filler in every position), plus tools/ra_solver/local_extract.py text1b --func func_80060A68 (QTYDBG) on Q2.
- result: Q2 and R3 reach 5 / build 66 / target 66 with all three lw ?,0x10($v1) at target's exact slots AND registers (12/$a1, 20/$a0, 23/$a0); a slot-for-slot diff shows Q2 is target through slot 24 and again from slot 33 to the end, with the entire residual (slots 25-32) caused by the +2 value seating in $v0 instead of $a0. QTYDBG explains the failure: reg74 is qty 11, birth 36, death 38, refs 2, allocation ord 5, got=2 -- the span is STILL 2, because local-alloc runs after sched1 and sched1 scheduled the filler lhu out of the window. reg73 (idx) is allocated in the suggested pass with range [40,66] and got=4, so it does not even conflict with reg74; reg74 first-fits $v0 purely because nothing occupies $v0 over its two-insn range. The gp store survives in that window in E2 only because sw %gp_rel and sh 0x1A are two stores sched cannot disambiguate, so a store-store edge glues it there -- R2's filler has to be a store.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 6 bodies plus a QTYDBG extraction, zero FAKE constructs in any of them

## [s20] A copy statement moved into the [+2 read, 0x1A store] window serves as both R2's span-2 $v0 donor and the second read gap's separator, closing both gaps on the E2 spine.
- mechanism: A copy's loaded value is a refs-2, span-2 quantity that first-fits $v0, and its sw is a memory store, so one statement could satisfy R2 and R1 at once without a gp store (which R4 forbids ahead of the p10 read).
- probe: N1-N6 (each of the three copies moved, several tail orders) and N7 (the D_800F10D0 zero store used as the window filler instead), measured with sandbox --disable all.
- result: N1/N3/N4 = 9-10 / build 66 with all three loads present but the two halfword pointer loads emitted at slots 17/20 instead of target's 20/23, because the moved copy's lw left the copy block and shortened it; N2/N5/N6 = 11-15 / 66; N7 = 16 / build 65. The copy pays for the window with the copy block's geometry.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 7 bodies, zero FAKE constructs in any of them

## [s20] Hoisting one of the two halfword reads into or above the copy block lets a copy store act as the missing free separator on the E2 and X2 spines.
- mechanism: s19 measured this for hoisted reads in the N/R families; the s20 variants pair the hoist with s19's new split 0x18 store so the freed sh can cover the other gap.
- probe: A1-A9 (the +2 read hoisted, fused and split 0x18 store, several tails) and F1-F7 (the +0 read hoisted, including above the zero store, with the split 0x18 store spent on the second gap).
- result: 9-16 at 65-67 instructions. Three loads at 66 instructions is easy to reach this way (A1/A2/A4/F1 = 12 / 66) but the hoisted read's own lw is emitted at slot 11-16 in $a0 in every member, taking the seat target gives p10; F4 (the read above the zero store) = 9 / 65 and F2/F8 pay an extra instruction as well.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 16 bodies, zero FAKE constructs in any of them

## [s20] Reading the +2 halfword before the +0 halfword lets the 0x1A store cover the first read gap and the 0x18 store cover the second, closing both gaps at zero instruction cost.
- mechanism: Each fused halfword statement emits its store immediately after its own pointer load, so swapping the reads order was expected to make both stores available as separators without moving any statement out of place.
- probe: W1, W2, W6 (split 0x18 store), W8, measured with sandbox --disable all and diffed against target.txt.
- result: W8 = 7 / build 66 with target's exact three-load geometry (12/$a1, 20/$a0, 23/$a0) and W1 = 7 / 66; W2/W6 = 7 / 67. In every member the 0x1A store is emitted ahead of the 0x18 store, which is seven instructions' worth of reordering against target's slots 24 and 28.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 4 bodies, zero FAKE constructs in any of them

## [s20] Placing the p10 read among the copies with the 0x1C store as the second gap's separator and a copy value as R3's $v0 donor recovers target's slot-12 p10 load, which s18's Z family missed only for lack of that donor.
- mechanism: s18 diagnosed the Z family's failure as an empty [p10 read, +4 read] window; a copy statement placed inside that window supplies the span-2 $v0 quantity R3 asks for without using a gp store.
- probe: Z1, Z2, Z4, Z6 (the copy in the window, the p10 read at several positions inside the copy block), measured with sandbox --disable all.
- result: 10-14 at 67-68 instructions, with the p10 load at slot 22-23 in $v0 in every member. Supplying the donor does not recover the slot-12 load once the p10 read sits textually inside the copy block, so the donor was not the missing ingredient there.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 4 bodies, zero FAKE constructs in any of them

## [s20] Hoisting the p10 read ABOVE the D_800F10D0 zero store (rather than above the copy block, which is as far as s19's Q family went) yields three loads with every target register seat.
- mechanism: sw zero,%lo(D_800F10D0)($at) is an $at-based store sched2 cannot disambiguate from lw ?,0x10($v1), so it is a hoist barrier for the p10 load; a read placed above it is free of the sched1 pin that the s4/s10 conservation law describes.
- probe: T1 = P, Z0, C1,C2,C3, S1, S2, S5, S3, S6, S4, S7, S8 (plus T2/T3/T4 with P just below the zero store, T5/T7 tail variants), measured with sandbox --disable all and diffed slot-for-slot against target.txt.
- result: T1 = 2 / build 65 / target 66 with three loads at 5 ($a1) / 19 ($a0) / 22 ($a0). Slot-for-slot it is target's entire instruction stream with every register seat correct -- the +2 value is in $a0 -- and exactly one difference: target's slot-5 nop is T1's lw $a1,0x10($v1), with everything after shifted one slot earlier. T1 is the second score-2 body in the campaign and the first with three loads and all target seats. T2/T3/T4 (P just below the zero store) are 5 / 67 with the load pinned at 25, which locates the discontinuity precisely at the zero store.
- verdict: CONFIRMED

## [s20] Giving the p10 value an early consumer while keeping its read just below the D_800F10D0 zero store unpins its load onto target's slot 12 without losing the third load.
- mechanism: The s4/s10 conservation law says three independent 0x10 loads imply p10 has no early consumer, which lets sched1 chain the bumped insn and sched2 pin the load to slot 25; supplying an early consumer was expected to unpin it while the zero store keeps it from hoisting to slot 5.
- probe: V1-V6 (the +4 read and the 0x1C store moved to every position between the copy block and the 0x1A store, with the p10 read immediately below the zero store), measured with sandbox --disable all.
- result: 7-12 at 66-67 instructions; the p10 load is emitted at slot 24-25 in every member and lands in $v0 in four of them. The early consumer does not move the load to slot 12 from that position.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 6 bodies, zero FAKE constructs in any of them

## s21 (2026-09-03, permuter modality) - hypotheses measured

## [s21] Seeding permuter campaigns on the T1 and Q2 chassis - two bodies that did not exist when s3/s3b ran - reaches a ban-free improvement on the honest floor that random mutation missed on the E2 and w3 chassis.
- mechanism: T1 and Q2 each sit one decision from target with different residuals (a displaced load vs a wrong register seat), so their neighbourhoods in mutation space are disjoint from the floor-2 E2 basin s3 sampled and from the w3 basin s3b sampled; the 2026-09-01 chassis rule asks for exactly this kind of structurally different seed.
- probe: two minimal-TU workspaces built from the validated s3 workspace (base permuter score 315 for T1, 420 for Q2), launched through tools/permuter_campaign.py at -j 4, run to 19,350 and 19,605 iterations (899 s / 905 s) and harvested with --stop in-session; all 87 finds classified by tmp/grind/func_80060A68/s21/semcheck.py against the function's 13 required statements, and the ban-free ordinary-C find re-spelled and measured with sandbox --disable all.
- result: KILLED as an improvement path, CONFIRMED as a classification. Best new permuter scores were 115 (T1) and 175 (Q2), but 37 of the 87 finds re-assign p10 or temp2 a second time to hold a copy's source pointer - the Judge-banned multiply-assigned carrier - 13 introduce new_var alias locals, and most of the rest are semantically unfaithful (temp_a1 read from p10 above p10's assignment, p10 = 4, a dropped copy, if (1) wrapping). Seven finds are faithful; six are plain statement reorderings and one is T1 plus a bare do { } while (0). The one worth spending, permQ2/output-245-1, re-spells as M1 = 4 / 66 and its neighbour M2 = 3 / build 66 / target 66 with three loads - the best three-load score in the campaign, but above the floor of 2.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis (candidate.c re-measured at 2 / 66 / 66 before and src/text1b.c restored to HEAD after), two campaigns totalling 38,955 iterations, zero FAKE constructs in any measured body

## [s21] Any ordinary-C change that makes another instruction ready in target's slot-5 nop turns T1 into a byte match without touching the read structure (s20 frontier item 1).
- mechanism: s20 read the T1 residual as a sched2 ready-list decision at one cycle - the p10 load wins slot 5 only because nothing else is ready there - so occupying that cycle with any other insn should push the load to target's slot 12 and restore the nop.
- probe: G1-G8, eight bodies hoisting a second statement above the D_800F10D0 zero store alongside the p10 read (the idx read, a copy, either gp store, the 0x18 statement, and an idx-fed spelling of the zero store), plus control G0; sandbox --disable all on today's HEAD, with the three 0x10 load slots and registers read out of each disassembly.
- result: KILLED. G0 (control) = 2 / 65 with the load at 5. G1/G2/G8 = 38 at 62 instructions (the hoisted idx read merges with the zero store's own index read and the head collapses); G3/G4 = 11 at 67 with the load at 10; G5 = 9 at 66 with the load at 4; G6 = 9 at 65 with the load at 7; G7 = 7 at 64 with two loads. The premise is wrong: the zero store is a hoist barrier in BOTH directions, so a p10 read above it has its load confined to slots 4-8 and filling slot 5 with another insn only moves the load to slot 6 - the body stays 65 instructions and never reaches target's slot-12 placement.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 9 bodies, zero FAKE constructs in any of them

## [s21] On the Q2 spine the 0x1C store can be placed inside [+2 read, 0x1A store] as R2's span-2 $v0 store by moving the p10 read and the +4 read ahead of the +2 read (s20 frontier item 2).
- mechanism: R2 needs a store glued inside the [+2 read, 0x1A store] window; the 0x1C store is the one store in the function never tried in that role, and unlike E2 the Q2 spine can supply its prerequisites because Q2 already covers both read gaps with the 0x1A store.
- probe: H1-H6, six bodies with the p10 read and the +4 read hoisted ahead of the +2 read at three positions each and the 0x1C store placed between the +2 read and the 0x1A store; sandbox --disable all on today's HEAD.
- result: KILLED. 9-14 at 65-66 instructions, and every member has only TWO 0x10 loads (H2/H4/H6 at 19/22 in $a0; H1/H3/H5 at 19 $a0 and 23 $v0). Putting the +4 read adjacent to the p10 read removes R3's span-2 $v0 donor from p10's range, which is the same mechanism that killed s18's Z family, and the third read merges again. The 0x1C store's prerequisites and R3's donor requirement are mutually exclusive on this spine as well as on E2's.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 6 bodies, zero FAKE constructs in any of them

## [s21] A gp store placed in a read gap works as a free cse separator, so the second gap can be covered without spending the 0x1A store (Q2's defect) or a copy's load-delay nop.
- mechanism: s17 recorded R4 as a positional law - the p10 read must not follow a gp-based store - which is why no session had ever placed a gp store in a read gap; s18 corrected R4 to a register law about p10's $a1 seat and its span-2 $v0 donor, which re-opens the configuration if the OTHER gp store supplies the donor.
- probe: J1-JA, ten bodies with a gp store between the +2 read and the p10 read and the other gp store placed strictly inside [p10 read, +4 read] in six of them; sandbox --disable all on today's HEAD, load slots and registers read out of each disassembly.
- result: CONFIRMED as a separator, KILLED as a route to the match. All ten members are 66 instructions with three 0x10 loads - the separator really is free - but the p10 load is emitted at slot 27-29 and is never in $a1, including in the six members that satisfy s18's corrected donor condition. Scores 5-9 (JA = 5, J5/J8 = 7, J1-J4/J7/J9 = 8-9). The positional flavour of R4 is therefore an independent necessary condition that s18's correction did not subsume: a p10 read following a gp store loses the slot-12 $a1 load whether or not it has a donor.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 10 bodies, zero FAKE constructs in any of them

## [s21] Ordering the reads so the p10 read precedes both gp stores (reads +0/p10/+2 or p10/+0/+2, with one gp store covering the remaining gap and the other serving as R2's window filler) satisfies R1, R2, R3 and both flavours of R4 at once.
- mechanism: with the p10 read early, one gp store can cover the gap that the free 0x18 store cannot reach while simultaneously sitting inside [p10 read, +4 read] as R3's span-2 $v0 donor, and the second gp store still lands inside [+2 read, 0x1A store] for R2 - the first configuration in the campaign in which all four conditions hold simultaneously.
- probe: K1-K3 and K5 (reads +0, p10, +2) and L1-L6 (reads p10, +0, +2, i.e. target's load order), ten bodies with both gp-store assignments and several tail permutations; sandbox --disable all on today's HEAD plus a slot-for-slot diff of K1 against target.
- result: KILLED on instruction count. K1/K2/K3 = 8 at 68 instructions with loads at 12 ($a1) / 20 ($a0) / 27 ($v0) - K1 is identical to target through slot 22, then target's slot-23 lw a0,0x10(v1) is a nop and the +2 load appears at 27 in $v0 with a second nop at 28. L1/L2/L3 = 8-9 at 68 with loads at 12 ($a1) / 24 ($v0) / 25 ($a0); L6 = 9 / 68; L5 (R2 broken control) = 13 / 69; K5 = 5 / 66. A gp store used as a separator ahead of the halfword reads costs exactly two instructions of displacement, which is the price the J family avoids by sitting after them.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 10 bodies, zero FAKE constructs in any of them

## [s21] Seeding permuter campaigns on the T1 and Q2 chassis - two bodies that did not exist when s3/s3b ran - reaches a ban-free improvement on the honest floor that random mutation missed on the E2 and w3 chassis.
- mechanism: T1 and Q2 each sit one decision from target with different residuals (a displaced load vs a wrong register seat), so their neighbourhoods in mutation space are disjoint from the floor-2 E2 basin s3 sampled and the w3 basin s3b sampled; the 2026-09-01 chassis rule asks for exactly this kind of structurally different seed.
- probe: Two minimal-TU workspaces built from the validated s3 workspace (base permuter score 315 for T1, 420 for Q2), launched through tools/permuter_campaign.py at -j 4, run to 19,350 and 19,605 iterations (899 s / 905 s) and harvested with --stop in-session; all 87 finds classified by tmp/grind/func_80060A68/s21/semcheck.py against the function's 13 required statements, and the ban-free ordinary-C find re-spelled and measured with sandbox --disable all.
- result: KILLED as an improvement path, CONFIRMED as a classification. Best new permuter scores were 115 (T1) and 175 (Q2), but 37 of the 87 finds re-assign p10 or temp2 a second time to hold a copy's source pointer - the Judge-banned multiply-assigned carrier - 13 introduce new_var/new_var2 alias locals, and most of the rest are semantically unfaithful (temp_a1 read from p10 above p10's assignment, p10 = 4, a dropped copy statement, if (1) wrapping). Seven finds are faithful: six plain statement reorderings and one T1 plus a bare do { } while (0). The one worth spending, permQ2/output-245-1, re-spells as M1 = 4 / 66 with three loads and its neighbour M2 = 3 / build 66 / target 66 with three loads at 19 ($a0) / 22 ($a0) / 27 ($v0) - the best three-load score in the campaign, but still above the floor of 2.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis (candidate.c re-measured at 2 / build 66 / target 66 before, src/text1b.c restored to HEAD after), two campaigns totalling 38,955 iterations, zero FAKE constructs in any measured body

## [s21] Any ordinary-C change that makes another instruction ready in target's slot-5 nop turns T1 into a byte match without touching the read structure.
- mechanism: s20 read the T1 residual as a sched2 ready-list decision at one cycle - the p10 load wins slot 5 only because nothing else is ready there - so occupying that cycle with any other insn should push the load to target's slot 12 and restore the nop.
- probe: G1-G8, eight bodies hoisting a second statement above the D_800F10D0 zero store alongside the p10 read (the idx read, a copy, either gp store, the 0x18 statement, and an idx-fed spelling of the zero store), plus control G0; sandbox --disable all on today's HEAD, with the three 0x10 load slots and registers read out of each disassembly.
- result: KILLED. G0 (control) = 2 / 65 with the load at slot 5, reproducing s20's T1. G1/G2/G8 = 38 at 62 instructions (the hoisted idx read merges with the zero store's own index read and the head collapses); G3/G4 = 11 at 67 with the load at 10; G5 = 9 at 66 with the load at 4; G6 = 9 at 65 with the load at 7; G7 = 7 at 64 with two loads. The premise is wrong: the zero store is a hoist barrier in BOTH directions, so a p10 read above it has its load confined to slots 4-8 and filling slot 5 with another insn only moves the load to slot 6 - the body stays 65 instructions on this spine and no member reaches target's slot-12 placement.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 9 bodies, zero FAKE constructs in any of them

## [s21] On the Q2 spine the 0x1C store can be placed inside [+2 read, 0x1A store] as R2's span-2 $v0 store by moving the p10 read and the +4 read ahead of the +2 read.
- mechanism: R2 needs a store glued inside the [+2 read, 0x1A store] window; the 0x1C store is the one store in the function never tried in that role, and unlike E2 the Q2 spine can supply its prerequisites because Q2 already covers both read gaps with the 0x1A store.
- probe: H1-H6, six bodies with the p10 read and the +4 read hoisted ahead of the +2 read at three positions each and the 0x1C store placed between the +2 read and the 0x1A store; sandbox --disable all on today's HEAD.
- result: KILLED. 9-14 at 65-66 instructions, and every member has only TWO 0x10 loads (H2/H4/H6 at 19/22 in $a0; H1/H3/H5 at 19 $a0 and 23 $v0). Putting the +4 read adjacent to the p10 read removes R3's span-2 $v0 donor from p10's range - the same mechanism that killed s18's Z family - and the third read merges again. The 0x1C store's prerequisites and R3's donor requirement are mutually exclusive on this spine as well as on E2's.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 6 bodies, zero FAKE constructs in any of them

## [s21] A gp store placed in a read gap works as a free cse separator, so the second gap can be covered without spending the 0x1A store or a copy's load-delay nop.
- mechanism: s17 recorded R4 as a positional law - the p10 read must not follow a gp-based store - which is why no session had ever placed a gp store in a read gap; s18 corrected R4 to a register law about p10's $a1 seat and its span-2 $v0 donor, which re-opens the configuration if the OTHER gp store supplies the donor.
- probe: J1-JA, ten bodies with a gp store between the +2 read and the p10 read and the other gp store placed strictly inside [p10 read, +4 read] in six of them; sandbox --disable all on today's HEAD, load slots and registers read out of each disassembly.
- result: CONFIRMED as a separator, KILLED as a route to the match. All ten members are 66 instructions with three 0x10 loads - the separator is genuinely free - but the p10 load is emitted at slot 27-29 and is never in $a1, including in the six members that satisfy s18's corrected donor condition. Scores 5-9 (JA = 5, J5/J8 = 7, J1-J4/J7/J9 = 8-9). The positional flavour of R4 is therefore an independent necessary condition that s18's correction did not subsume: a p10 read following a gp store loses the slot-12 $a1 load whether or not it has a donor.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 10 bodies, zero FAKE constructs in any of them

## [s21] Ordering the reads so the p10 read precedes both gp stores (reads +0/p10/+2 or p10/+0/+2, with one gp store covering the remaining gap and the other serving as R2's window filler) satisfies R1, R2, R3 and both flavours of R4 at once.
- mechanism: With the p10 read early, one gp store can cover the gap the free 0x18 store cannot reach while simultaneously sitting inside [p10 read, +4 read] as R3's span-2 $v0 donor, and the second gp store still lands inside [+2 read, 0x1A store] for R2 - the first configuration in the campaign in which all four conditions hold simultaneously.
- probe: K1-K3 and K5 (reads +0, p10, +2) and L1-L6 (reads p10, +0, +2, i.e. target's load order), ten bodies covering both gp-store assignments and several tail permutations; sandbox --disable all on today's HEAD plus a slot-for-slot diff of K1 against target.
- result: KILLED on instruction count. K1/K2/K3 = 8 at 68 instructions with loads at 12 ($a1) / 20 ($a0) / 27 ($v0) - K1 is identical to target through slot 22, then target's slot-23 lw a0,0x10(v1) is a nop and the +2 load appears at 27 in $v0 with a second nop at 28. L1/L2/L3 = 8-9 at 68 with loads at 12 ($a1) / 24 ($v0) / 25 ($a0); L6 = 9 / 68; L5 (R2 broken, control) = 13 / 69; K5 = 5 / 66. A gp store used as a separator ahead of the halfword reads costs exactly two instructions of displacement, which is the price the J family avoids by sitting after them.
- verdict: KILLED
- kill_scope: instance
- measured_on: today's HEAD chassis, 10 bodies, zero FAKE constructs in any of them
