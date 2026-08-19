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
