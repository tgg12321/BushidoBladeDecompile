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
