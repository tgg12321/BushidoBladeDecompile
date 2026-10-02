# Evidence bank — func_8005D554

> COMPACTED 2026-10-01 by `grindlib.py compact-ledger` (211,006 bytes, cap 64K). Older entries are one-line index items;
> full text: `git show 11cb352eb:memory/grind/func_8005D554/evidence.md` (history before 2026-10-01: tag pre-slim-2026-10-01).
> Look one up only when its index line bears on your probe. Append below as usual.

## Index of older entries (first line each; full text in git, above)

(238 older entries omitted)
### 1. The control window reproduces s9 exactly, and the SELBEST line names the tie-break
sched1, half 1, clocks 61-67 (`tmp/grind/func_8005D554/s10/sched.log`):
Backward scheduler ⇒ emission is the reverse of the pick order: `211,240,205,242`
The `SELBEST ... pos=1` line is new information: at clock 64 the ready array (before the
### 2. potential_hazard can never separate these three insns (frontier item 2 — CLASS KILL)
`potential_hazard` (sched.c:1327-1366) returns its incoming cost unchanged unless
`mips.md` defines function units only for the types `load`, `store`, `xfer` (unit "memory"),
This is directly visible in the trace: the *stores* (unit=0 "memory", `maxb=3` in the
Consequence: a hazard-winning spelling of the a2 base would have to be emitted as a memory or
### 3. The last-scheduled CLASS criterion is structurally 3-vs-3 (upgrades s7 from instance to class)
`rank_for_schedule` computes `tmp_class = 3` whenever
### 4. The LUID axis is bounded by expand_call (frontier item 3 — measured, and the bound named)
`expand_call`'s final loop emits the hard-register argument moves —
Measured, control: `LUID(a0 move) = 42`, `LUID(a1 move) = 43`, and the largest LUID over every
Measured, new form **v1** (`tmp/grind/func_8005D554/s10/v1.c`, banked at
So the maximum LUID any source-expressed pre-call value can reach is bounded strictly below
### 5. RELOAD inserts nothing in either window (frontier item 1 — measured dead)
`tmp/grind/func_8005D554/s10/greg_seg.txt` (the `func_8005D554` segment of `text1b.greg`):
### Where that leaves the residual
All three criteria of `rank_for_schedule` plus `schedule_select`'s hazard tie-break are now
- [s10] Control floor re-measured this session: score 6, target_insns 176, build_insns 176, on HEAD…
- [s10] The clock-64 SELBEST trace line (`SELBEST clock=64 insn=242 pos=1`) shows the potential-haz…
- [s10] The three struct stores win clocks 61-63 over the higher-LUID argument moves purely because…
- [s10] Control LUIDs in the half-1 window: a0 argument move 42, a1 argument move 43, s.zero1C stor…
- [s10] greg for func_8005D554: 18 pseudos to allocate, 18 dispositions, no reload insertions; only…
- [s10] func_80073728 reads only $a0 and $a1; $a2 is never read before being written, so the callee…
## s11 (rederive, 2026-09-09, HEAD main @ e4c60089)
- Control (memory/grind/func_8005D554/candidate.c applied to src/text1b.c) re-measures
- Kill re-audit: `tools/fake_ablate.py` on the closest banked form
- **src/text1b.c holds a THIRD S46C sibling that no prior session named: func_8005FA98
- Frontier item 1 (s10) measured in its narrow form: pointer local used ONLY as the first call
- Frontier item 3 (s10) measured for the first time: a real if/else for half 2's p0 selection
- Reading of the target window that this session re-derived from asm/funcs/func_8005D554.s
- [s11] Control re-measured 6/176 (build_insns == target_insns == 176) on HEAD main @ e4c60089; cha…
- [s11] Kill re-audit passed for a third session: tools/fake_ablate.py on the closest banked form (…
- [s11] src/text1b.c holds a THIRD S46C sibling no prior session named: func_8005FA98 at text1b.c:2…
- [s11] A real two-armed branch inside the loop body costs only ONE instruction on this chassis (17…
- [s11] Re-derived reading of the target window (asm/funcs/func_8005D554.s lines 92-109), to be inh…
- [s11] Frontier item 2 (sched2 inversion) is analytically de-prioritised, not measured: rank_for_s…
## s12 (rederive, 2026-09-09, HEAD main @ 0c19707c)
- Control (memory/grind/func_8005D554/candidate.c applied to src/text1b.c) re-measures
- Kill re-audit (fourth consecutive pass): `tools/fake_ablate.py --func func_8005D554 --file
### 1. CORRECTION to s8: the "three-argument call is byte-inert" finding had the wrong cause
s8 concluded that `func_80073728(&s, 0, a2_offset)` is byte-inert "because expand_call evaluates
### 2. A GENUINE third argument does materialise, and it is emitted AFTER both argument moves
Form p3 (`tmp/grind/func_8005D554/s12/p3_fnptr_third_arg.c`, banked at
### 3. A base whose only use is the call argument is LICM-hoisted out of the loop
Form p1 (`rejected/base-only-as-third-arg-licm-hoisted-scores-66.c`) removes the accumulate and
### 4. The whole INSN_PRIORITY family is now closed by a predicate (frontier item 3, class kill)
Reading `schedule_select` (sched.c:2660-2726) settles what s8/s9 could only measure. The ready
- [s12] Control re-measured 6/176 on HEAD main @ 0c19707c; kill re-audit passed for a fourth sessio…
- [s12] src/text1b.c:2642 declares `extern s32 func_80073728(s32, s32);` at file scope, so a functi…
- [s12] A genuine third argument (function-pointer cast, p3) scores 17/178 and materialises as `add…
- [s12] `(s32)r4 - K` with no accumulate and a single use is a loop.c movable and gets hoisted to t…
- [s12] sched.c:2674 processes the ready list in priority groups and exhausts a higher-priority gro…
- [s12] Control re-measured 6/176 (build_insns equals target_insns equals 176) on HEAD main @ 0c197…
- [s12] Kill re-audit passed for a fourth session: fake_ablate finds no FAKE construct in rejected/…
- [s12] src/text1b.c:2642 declares extern s32 func_80073728(s32, s32) at file scope. A function-loc…
- [s12] A genuine third argument via a local function-pointer cast measures 17/178 and materialises…
- [s12] (s32)r4 - K with the accumulate removed is a loop.c movable and is hoisted to the preheader…
- [s12] sched.c:2674 processes the ready list in maximal equal-priority groups and exhausts a highe…
- [s12] sched.c:1359: for insn_unit -1 the loop that iterates the unit bitmask never executes, so p…
- [s12] The only scheduler configuration that reproduces the target window is priority(stores) >= p…
## s13 (structural, 2026-09-09, HEAD main @ 5daf178d) -- the deciding term is the birthing boost
Control re-measured 6/176 (176 build insns, 176 target insns) on memory/grind/func_8005D554/candida…
### New instrumentation
### What the trace says
Control, block 6 nodes around the half-1 window:
Score-0 body (rejected/judge-failed-fresh-multiwrite-nv-nw-carrier-scores-0.c), same block:
### Why the boost is out of reach from a single-set source spelling
### The five spellings measured
Two structural facts fall out. (1) The `carrier = ret; s.ret = carrier;` second write is NOT the
- [s13] Control floor re-measured this session at 6/176 on memory/grind/func_8005D554/candidate.c (…
- [s13] tools/gcc-2.7.2/cc1 carries a previously unused BB2_PRIO_DEBUG hook inside priority() (sche…
- [s13] GCC 2.7.2's priority() walks LOG_LINKS (predecessors), so INSN_PRIORITY is depth from the B…
- [s13] Both loop halves are in ONE basic block (SCHEDDBG block=6 n_insns=92), which is why half 1'…
- [s13] In the control the a2 base is insn 211 (luid 31, pri 3, birth=0); in the banked score-0 bod…
- [s13] adjust_priority's boost (sched.c:2584) fires only when birthing_insn_p (sched.c:2505) holds…
- [s13] The score-0 body's second write ('nv = ret; s.ret = nv;') is deleted by combine, so loop.c…
- [s13] Every existing-loop-local carrier whose live range spans the third rand() call costs exactl…
- [s13] The ret restage alone is byte-inert (6/176), so it is not the lever; the early base is.
## s14 (structural)
- [s14] Control floor re-measured 6/176 on memory/grind/func_8005D554/candidate.c at HEAD main @ 46…
- [s14] Kill re-audit (fifth consecutive pass): tools/fake_ablate.py reports no FAKE-annotated cons…
- [s14] loop.c's movable guard is a THREE-WAY OR (loop.c:695-700). A candidate is rejected only whe…
- [s14] reg_in_basic_block_p (loop.c:1062) returns 0 in two ways: regno_first_uid[regno] != INSN_UI…
- [s14] maybe_never is set only when scan_loop's forward scan passes a CODE_LABEL or JUMP_INSN insi…
- [s14] MEASURED: the plain non-guard-duplicated while chassis gives no maybe_never, because GCC ro…
- [s14] MEASURED: falsifying C alone (base1 read again in half 2 across a real if/else join) leaves…
- [s14] MEASURED: no-op sets of r4 (r4 = r4, and r4 = (u32)(base1 + 0xC) which folds to it) are del…
- [s14] CORRECTION to s12: the target window asm/funcs/func_8005D554.s 4DEB4-4DECC is a0 / a1 / lw…
- [s14] Control floor re-measured 6/176 on memory/grind/func_8005D554/candidate.c at HEAD main @ 46…
- [s14] Kill re-audit passes for a fifth consecutive session: tools/fake_ablate.py finds no FAKE-an…
- [s14] loop.c's movable guard is a THREE-way OR (loop.c:695-700), not a single test: a candidate i…
- [s14] B is false for free for every C-level local, since every user variable carries REG_USERVAR_…
- [s14] reg_in_basic_block_p (loop.c:1062) returns 0 either when regno_first_uid[regno] != INSN_UID…
- [s14] maybe_never is sticky once scan_loop's forward scan passes a CODE_LABEL or JUMP_INSN inside…
- [s14] MEASURED: the plain while chassis is rotated by GCC into a guarded do-while (guard blez at…
- [s14] MEASURED: falsifying C alone still hoists - 57/179 on the do-while chassis, 70/177 on the w…
- [s14] MEASURED: no-op sets of r4 are removed by cse before loop_optimize (54/178 twice, byte-iden…
- [s14] CORRECTION to s12: the target emits a0 / a1 / lw / a2 base / sw / sw / sw at 4DEB4-4DECC, s…
## s15 (enumerate, 2026-09-09, HEAD main @ 36291a08)
Kill re-audit (sixth consecutive session): tools/fake_ablate.py --func func_8005D554 --file text1b
ENUMERATION: 61 spellings, best 6, 2 at the floor.
Forensic reading that framed the sweep. The block-6 PRIODBG capture banked by s13
Positional bound on the INSN_LUID term (corroborates s10's calls.c:1880 class kill from the other
Result 1 -- the naming space is closed by LICM. 44 of the 48 enumA variants build 153 instructions,
Result 2 -- the first form in fifteen sessions that moves the a2 base out of the window. The
Result 3 -- statement position confirmed byte-inert once more. Staging the multiply-shift through
Result 4 -- a second spelling at the floor. a2_offset = (s32)r4; a2_offset -= K; a2_offset +=
Result 5 -- moving the s.zero10/s.one14/s.ret stores above the a2 chain (so the a2 chain is the
- [s15] Kill re-audit passes for a sixth consecutive session: fake_ablate reports no FAKE-annotated…
- [s15] ENUMERATION: 61 spellings, best 6, 2 at the floor. Score histogram: 6 x 2, 8 x 2, 15 x 7, 1…
- [s15] 44 of 48 variants in the a2-expression naming cross product build 153 instructions - 23 bel…
- [s15] PRIODBG (s13 capture, re-read this session): insns 205, 211, 240 and 242 all take priority…
- [s15] Positional bound on INSN_LUID: the a2 base insn is necessarily followed before the call by…
- [s15] The base-last association emits [addiu a0,sp,16][lw v1,0(gp)][move a1,zero][sw][sw][sw] at…
- [s15] Staging the multiply-shift through the dead existing local a0_offset adds no instruction (1…
- [s15] A second distinct spelling sits exactly at the floor: a2_offset = (s32)r4; a2_offset -= K;…
## s16 (enumerate, 2026-09-09, HEAD main @ 07f3c383)
Floor RE-MEASURED at 6/176 on candidate.c. 44 new spellings measured in two sweeps.
### Kill re-audit (mandated, floor flat)
### Round A - the base-carrier axis (36 forms, sweepA.json)
Histogram: 6 (1 form), 8 (2), 12 (1), 15 (3), 16 (8), 19 (12), 21 (2), 22 (2), 26 (2),
Disassembly (tmp/sandbox/func_8005D554/text1b.o, half-1 window):
The decisive comparison is S1 vs S2. s15 recorded "no spelling produced both the late base
### Round B - the LICM escape for a single-set carrier (8 forms, sweepB.json)
Half 2's p0 selection is the loop body's only real conditional. Splitting it into two ifs
But the escape is self-defeating. schedule_insns runs per basic block (sched.c:4937), and
Carrier identity is again invisible: the identical shape with the multi-set a0_offset as the
Controls in the same sweep:
The other C-falsifier (loop.c:1068, an earlier reference to the carrier) would require a
### What s16 leaves standing
- [s16] Floor re-measured 6/176 on candidate.c at HEAD main @ 07f3c383; candidate.c compiles cleanl…
- [s16] 44 new spellings measured this session: 36 in tmp/grind/func_8005D554/s16/enumA (sweepA.jso…
- [s16] Target window for reference (asm/funcs/func_8005D554.s 4DEB4-4DECC): addiu a0,sp,0x10 / add…
- [s16] S2_both (base staged through the dead multi-set a0_offset, base statement after the multipl…
- [s16] S6_both (v0 borrow) emits the half-1 window byte-identically to the control at 0x3564-0x35b…
- [s16] reg_in_basic_block_p (loop.c:1062-1098) has two falsifiers s14 did not use: regno_first_uid…
- [s16] The conditional chassis for half 2's p0 selection costs exactly one instruction whether spe…
## s17 (enumerate, 2026-09-09, HEAD main @ fa84454f) — the existing-local borrow quadrant, swept
Floor RE-MEASURED at 6/176 on candidate.c. Kill re-audit passes for an EIGHTH session:
### What was swept and why
The Judge's 2026-09-08 constraint bans a FRESH invented local written more than once as a
Round A (36 forms, tmp/grind/func_8005D554/s17/enumA, histogram s17/enumA.json) — the score-0
Round B (30 forms, s17/enumB, histogram s17/enumB.json) — base set pinned at the CONTROL slot
### The three results
1. THE EARLY-BIRTH SLOT COSTS AT LEAST +2 INSTRUCTIONS FOR EVERY EXISTING LOCAL, 18/18 FORMS.
2. THE COMBINE-DELETABLE SECOND WRITE IS BYTE-INERT IN BOTH VALUE AND POSITION.
3. AT THE CONTROL SLOT THE CONSUMPTION SHAPE IS INERT TOO, AND THE BASE IS MERELY RESEATED.
### Reading of the quadrant after s17
The existing-local borrow quadrant has exactly two cells and both are now measured across every
- [s17] Kill re-audit (eighth consecutive session): fake_ablate finds no FAKE-annotated construct i…
- [s17] 66 complete spellings measured this session; combined score histogram 19 (2), 21 (19), 23 (…
- [s17] The existing-local borrow quadrant has exactly two cells and both are now measured across e…
- [s17] Read end to end this session: rank_for_schedule (sched.c:2408-2464) and schedule_select (sc…
- [s17] loop.c:695-760 read end to end: a candidate with n_times_set == 2 is STILL a movable when c…
- [s17] src/ and include/ are unmodified at end of session; candidate.c re-measures 6/176 with the…
## s18 (synthesis, 2026-09-09, HEAD main @ 9a80dc30) — the LICM hoist of a single-set carrier is NO…
Floor RE-MEASURED at 6/176 on `memory/grind/func_8005D554/candidate.c`. Kill re-audit passes
### 1. The loop.c hoist gate re-read: `savings` is a SET count, not a use count
`loop.c:597` is `bcopy ((char *) n_times_set, (char *) n_times_used, nregs * sizeof (short));`
with `m->lifetime = uid_luid[regno_last_uid[regno]] - uid_luid[regno_first_uid[regno]]`
### 2. The boundary MEASURED, and it is exactly where the arithmetic puts it
Round B (`tmp/grind/func_8005D554/s18/genB.py`, `enumB/`, sweep `enumB.json`) keeps the control's
| form | gap | score / insns (both halves) | (half 1 only) |
The step from gap 3 to gap 4 is the whole 176 -> 178 / 8 -> 54 discontinuity, exactly as
### 3. `birthing_insn_p` FIRES for that carrier — no multi-write trick required
Instrumented-cc1 capture for G1 (`tmp/grind/func_8005D554/s18/dumps/g1/sched.log`, block region
Insn 223 is the a2 base (`addiu v0,s4,-12` in the G1 disassembly at 0x35c4 — the TARGET's operand
### 4. Why the boost still does not match: the boosted base emits next to its consumer
Because the boost is `max_priority`, the base is selected the moment it becomes ready — clock 55,
versus the target (4DEB4-4DEEC):
Two residual facts fall out. (a) G1 gets `addiu a0,sp,0x10` into window slot 1 — the target's
### 5. Round C: an early base whose single use is an adjacent COPY is hoisted anyway (cse)
Round C (`s18/genC.py`, `enumC/`) tried to keep the base at the CONTROL luid slot while holding
- [s18] Kill re-audit passes for a NINTH consecutive session; control 6/176, rejected/a2-statements…
- [s18] loop.c:597 bcopies n_times_set into n_times_used, so m->savings (loop.c:793) is the carrier…
- [s18] MEASURED BOUNDARY (14 forms, s18/enumB): a fresh WRITTEN-ONCE base carrier whose def-to-las…
- [s18] CORRECTION to s13: "the invariant (s32)r4 - K is always a movable and is hoisted in every m…
- [s18] MEASURED IN THE COMPILER (s18/dumps/g1/sched.log:55885): for the gap-1 form the a2 base ins…
- [s18] The boosted base is selected one clock after its own consumer and is therefore emitted imme…
- [s18] G1 is the first measured form that puts `addiu a0,sp,0x10` in window slot 1, the target's s…
- [s18] MEASURED: an early base whose single use is an adjacent copy (`b1 = base; a2_offset = b1; a…
- [s18] Round A of this session (a named local for the multiply result) was INVALID: the generator'…
- [s18] loop.c:597 bcopies n_times_set into n_times_used, so m->savings (loop.c:793) is the carrier…
- [s18] MEASURED BOUNDARY: a fresh written-once a2 base carrier with def-to-last-use luid gap 1, 2…
- [s18] CORRECTION to s13: 'the invariant (s32)r4 - K is always a movable and is hoisted in every m…
- [s18] MEASURED IN THE COMPILER (tmp/grind/func_8005D554/s18/dumps/g1/sched.log:55885): the gap-1…
- [s18] The boosted base is selected one clock after its own consumer and is therefore emitted imme…
- [s18] G1 (gap 1) is the first form in this ledger that puts addiu a0,sp,0x10 into window slot 1,…
- [s18] An early base whose single use is an adjacent copy is hoisted anyway (54/178 in all four po…
- [s18] Round A of this session (a named local for the multiply result) was INVALID and must not be…
- [s18] Kill re-audit passes for a ninth consecutive session: control 6/176, rejected/a2-statements…
- [s18] src/ and include/ are unmodified at end of session; every measurement went through tools/sw…
## s19 (solver, 2026-09-09, HEAD main @ afafbb45) — the two chassis have DISJOINT, mutually exclusi…
Floor RE-MEASURED at **6/176** with candidate.c applied to src/text1b.c. Kill re-audit passed
- [s19] Tool routing note for every future solver session on this function: `inverse_compose.py
- [s19] Window uids, CONTROL chassis: 240 `addu a0,sp,16`, 242 `move a1,0`, 205
- [s19] The two chassis' depth-1 vector spaces for the SAME window goal are disjoint and each is
- [s19] The `s.zero10 = 0` slot cannot take a DATA dependence at all: combine runs before sched1,
- [s19] Compiler agreement with H49's vector, MEASURED: spelling the dependence as
- [s19] The same dependence spelled with the second use AFTER the accumulate is self-defeating:
- [s19] Both sched_solver models built this session are EXACT on this function (16/16 blocks
- [s19] src/text1b.c was left byte-clean (func_8005D554 still INCLUDE_ASM); every score came from
- [s19] Floor re-measured 6/176 on HEAD main @ afafbb45 with candidate.c applied to src/text1b.c; s…
- [s19] Tool routing for every future solver session here: inverse_compose.py classify REFUSES for…
- [s19] Window uids are CHASSIS-SPECIFIC and must be re-derived after every edit (tmp/grind/func_80…
- [s19] The two chassis have DISJOINT depth-1 vector spaces for the identical window goal: the cont…
- [s19] The s.zero10 = 0 slot cannot take a DATA dependence at all: combine runs before sched1, so…
- [s19] Both sched_solver models built this session are exact on this function (16/16 blocks order-…
## s20 (forensics, 2026-09-09, HEAD main @ 7a5fbffb) — the sched1 pick at clock 64 is now enumerate…
Floor RE-MEASURED at **6/176** with `memory/grind/func_8005D554/candidate.c` applied to
### 1. The complete sched1 trace of the half-1 window (control chassis)
Captured with the instrumented cc1 (`tools/gcc-2.7.2/cc1`, NOT `build/cc1`) under
Emission is the reverse of the pick order (sched.c:4036-4038), giving the control's
### 2. CORRECTION to s9/s17: the three struct stores do NOT win by LUID, they win by potential_hazard
s9 and s17 both read the window as a pure `rank_for_schedule` LUID contest. The trace refutes
This is why the residual is the four-insn rotation and never the store block: the stores'
### 3. Every input to the clock-64 pick, enumerated with its source-side reachability
`schedule_select` then `rank_for_schedule` consume exactly five inputs. All five are now
(a) **`actual_hazard` (the SELBLOCK gate, sched.c:2684).** Non-zero only for unit >= 0.
(b) **`potential_hazard` (the SELBEST term, sched.c:2717 and 1334).** 0 for all three of
(c) **`INSN_PRIORITY` (sched.c:2417).** Measured from PRIODBG at sched.log:56745-56805:
(d) **The last-scheduled class (sched.c:2429-2441).** MIPS `ADJUST_COST`
(e) **`INSN_LUID` (sched.c:2462).** 31 vs 42/43. `expand_call` emits both hard-register argument
So the control chassis is closed on all five inputs, each with a predicate, and the boost
### 4. The zero-cost releasing dependent (s19 frontier items 1 and 2) is measured out
Ten complete spellings, generated by `tmp/grind/func_8005D554/s20/gen.py` onto s19's
| form | spelling of the base to `s.zero10 = 0` edge | score / insns |
The split is total and it is a fold/no-fold split. Every spelling whose value or address GCC can
- [s20] Floor re-measured 6/176 on HEAD main @ 7a5fbffb with candidate.c applied; src/ and include/…
- [s20] Kill re-audit passes for a TENTH consecutive session: fake_ablate finds no FAKE construct i…
- [s20] CORRECTION to s9 and s17: the three struct stores are picked ahead of the two call-argument…
- [s20] MEASURED IN THE COMPILER (s20/dumps/ctrl/sched.log:56745-56805): every window insn's priori…
- [s20] MEASURED IN THE COMPILER: in the CONTROL chassis the multiply chain and the gp load are ALR…
- [s20] MIPS ADJUST_COST (tools/gcc-2.7.2/config/mips/mips.h:2947) zeroes the cost of every anti/ou…
- [s20] 10 spellings measured for the base-to-s.zero10-store dependence edge (s20/enumA, s20/enumA.…
- [s20] V4 (s.zero10 = b1 followed by s.zero10 = 0) builds 176 instructions, so GCC 2.7.2 deletes t…
- [s20] Floor re-measured 6/176 on HEAD main @ 7a5fbffb with candidate.c applied to src/text1b.c; s…
- [s20] Kill re-audit passes for a TENTH consecutive session: tools/fake_ablate.py finds no FAKE-an…
- [s20] The complete sched1 pick trace of the half-1 window (control chassis): clock 60 pick 206 (m…
- [s20] CORRECTION to s9 and s17: the three struct stores lead the two argument-setup insns by sche…
- [s20] MEASURED IN THE COMPILER (s20/dumps/ctrl/sched.log:56745-56805): every window insn's priori…
- [s20] MEASURED IN THE COMPILER: in the CONTROL chassis the multiply chain and the gp load are ALR…
- [s20] MIPS ADJUST_COST (tools/gcc-2.7.2/config/mips/mips.h:2947) zeroes the cost of every anti/ou…
- [s20] All five inputs to the decisive clock-64 pick are now individually measured and closed in t…
- [s20] 10 spellings measured for the base-to-s.zero10-store dependence edge (tmp/grind/func_8005D5…
- [s20] V4 (s.zero10 = b1 followed by s.zero10 = 0) builds 176 instructions, so GCC 2.7.2 deletes t…
## s21 (forensics, 2026-09-09, HEAD main @ 764b2eb1)
Floor RE-MEASURED at 6/176 on candidate.c (`tmp/grind/func_8005D554/s21/reaudit.json`).
### 1. RELOAD IS LIVE ON THIS FUNCTION — the frontier's "no reload insn" branch is refuted
`-da` dumps with `BB2_RELOAD_DEBUG=1` + `BB2_ALLOC_DEBUG=1`
| new uid | insn | where |
`RELOADDBG` shows why: global alloc leaves pseudos 119 (`base_offset`, nrefs=2, livelen=1,
### 2. CLASS KILL — sched2 cannot produce the four-insn rotation; it is an identity here
Pass-2 trace of block 6 (`s21/ctrl_sched2_block6.txt`). The whole half-1 window is picked in
(the only `SELBEST` lines anywhere in sched2's block 6 are clocks 45/46/47 for the half-2 stores
### 3. CLASS KILL — the ready-clock route (s20 frontier item 1) is closed by a chain inequality
`schedule_insn` releases a predecessor into the ready list only when its `INSN_REF_COUNT` hits 0,
With `clock(244) = 51` this gives, measured: ready(240) = ready(242) = 52, clock(237) = 52,
### 4. The third-argument form creates NO insn — RTL insn-for-insn identical to the control
s12's `func_80073728((s32)&s, 0, a2_offset)` form (rejected/three-argument-call-reading-inert-
### 5. What the target's own emission order forces (the sharpened residual)
Both halves emit, in order: [the four contested insns] [the three struct stores] [206 217 219 220
Giving the base the birth boost (s18's G1 chassis, re-dumped this session as
- [s21] Floor re-measured 6/176 on HEAD main @ 764b2eb1 with candidate.c applied; the body re-measu…
- [s21] Kill re-audit passes for an ELEVENTH consecutive session: tools/fake_ablate.py finds no FAK…
- [s21] RELOAD IS LIVE on func_8005D554: reload creates uids 398 (spill of base_offset to sp+64 in…
- [s21] ALLOCDBG: global alloc leaves three pseudos unallocated on this function - 119 (base_offset…
- [s21] sched2 block 6 picks the half-1 window in strict descending-LUID order (63:234 64:231 65:22…
- [s21] Measured release clocks in sched1 block 6: clock(call 244)=51, ready(240)=ready(242)=52, cl…
- [s21] The a2 base is READY from clock 55 and must lose nine consecutive clocks before the decisiv…
- [s21] s18's G1 boost chassis re-dumped (s21/g1_sched1_block6.txt): the boosted base carrier is pi…
- [s21] s12's third-argument form is RTL-identical to the control (same UIDs, same LUIDs 28..49, sa…

## Recent entries (verbatim)

## s22 (rederive, 2026-09-09) - measurements

Chassis: control candidate.c re-measured 6/176 on HEAD main @ 8d423e0f (sweep baseline).
Kill re-audit: fake_ablate on rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c
reports "no FAKE-annotated constructs found" (eleventh consecutive session).

sweepA (tmp/grind/func_8005D554/s22/vars, sweepA.json) - store-group + chassis, 11 forms:
  V00_control 6/176 | V03_zero1C_first 6/176 | V06_byte28_in_group 10/176 |
  V08_ret_first_in_group 10/176 | V09_one14_before_zero10 10/176 | V04_c20c24_in_group 12/177 |
  V05_p1_in_group 13/176 | V10_p0_in_group 23/176 | V07_zero18_in_group 28/178 |
  V01_wide_pointer_store 42/179 | V02_inline_helper 59/173

sweepB (tmp/grind/func_8005D554/s22/varsB) - the LICM-source-invariance axis, 4 forms:
  W2_r4_init_in_loop 26/179  (fresh single-set b1/b2 + r4 set in loop)  <-- target rotation
  W3_r4_init_in_loop_multiset 32/179 (same chassis, multi-set a2_offset) <-- control rotation
  W0_fresh_single_set_base 54/178 (fresh single-set base, r4 invariant -> both bases hoisted)
  W1_r4r5_init_in_loop 61/176 (both r5 and r4 moved into the loop)

sweepC (tmp/grind/func_8005D554/s22/varsC) - cheaper spellings of r4 non-invariance, 5 forms:
  C6_while_chassis 26/179 | C5_copy_from_preheader_temp 28/178 |
  C3_cond_init_at_body_end 32/178 | C2_cond_init_before_a2 32/181 | C4_a0_fresh_too 68/182

Disassembly evidence: tmp/grind/func_8005D554/s21/dumps/s22W2/text1b.s (cc1 -da output for the
W2 body) - both call windows emit
  addu $4,$sp,16 / move $5,$0 / lw $3,D_800A3418 / addu $6,$20,-12 (-25) / sw / sw / sw
which is the target's 4DEB4-4DECC and 4DF6C-4DF84 verbatim in order and operand form.

Target fact established this session: $s4 and $s5 are each written exactly once in the whole
function, in the pre-loop straight line (4DDE0 srl $s5 / 4DE1C srl $s4), and never inside the
loop body 4DE50-4DFCC. The original's a2 base therefore escaped LICM with a LOOP-INVARIANT
source register, so the r4-non-invariance route measured here reproduces the target's schedule
by a mechanism the original did not use.

- [s22] Chassis check: memory/grind/func_8005D554/candidate.c re-measures 6/176 on HEAD main @ 8d423e0f, and re-measures 6/176 again after this session's header edit (the s9 comment-delimiter trap was checked for).

- [s22] Kill re-audit passes for the eleventh consecutive session: tools/fake_ablate.py reports 'no FAKE-annotated constructs found' in rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c.

- [s22] loop.c's movable acceptance has FIVE conjuncts, not the two the ledger has been attacking. Sessions s13-s16 falsified n_times_set of the DEST (loop.c:705) and enumerated the three-way OR (loop.c:695-700). The FIRST conjunct is invariant_p(SET_SRC) (loop.c:702), whose REG case is 'return n_times_set[REGNO (x)] == 0' -- i.e. the base insn stops being hoistable the moment its SOURCE register is set inside the loop, with no second write and no label.

- [s22] Measured consequence: W2_r4_init_in_loop.c (fresh once-written b1/b2 + r4 set in the loop) emits BOTH call windows as the target's rotation -- [addu $4,$sp,16][move $5,$0][lw $3,D_800A3418][addu $6,$20,-12 / -25][sw][sw][sw], matching 4DEB4-4DECC and 4DF6C-4DF84 -- at 26/179. The identical chassis with the base carried by the multi-set a2_offset scores 32/179, exactly six worse.

- [s22] The original did NOT use this route: $s4 and $s5 are each written exactly once in the whole target function, at 4DDE0 and 4DE1C in the pre-loop straight line, and never inside the loop body 4DE50-4DFCC. So the original's a2 base escaped LICM with a LOOP-INVARIANT source, which leaves may_not_optimize (an explicit standalone (clobber (reg)), set by count_loop_regs_set at loop.c:2989, gating the whole analysis at loop.c:649) and reg_in_basic_block_p's regno_first_uid test (loop.c:1068) as the only two remaining escapes for a single-set invariant base.

- [s22] rtlanal.c's may_trap_p FALLS THROUGH from the DIV/MOD/UDIV/UMOD cases into 'case EXPR_LIST: return 1;' -- there is no break after the constant-divisor tests -- so any div/mod rtx makes may_trap_p true. Since call_passed is already 1 at the a2 statement (three rand calls precede it), loop.c:715's '! ((maybe_never || call_passed) && may_trap_p (src))' is a real, never-enumerated falsifier of movability; it needs a div or a trapping MEM in the base insn's own SET_SRC.

- [s22] In GCC 2.7.2 the only emitter of a standalone (clobber (reg)) for a pseudo that is not a multi-word/DImode path is store_constructor for a UNION built into a register (expr.c:2996); expr.c:1991 and expr.c:913 are the multi-word paths and belong to the banned DImode family.

- [s22] The a0 side must be left alone: converting the two a0 bases to fresh single-set locals on the same chassis costs six points (68/182 vs 26/179). The a0 windows already match the target with the multi-set a0_offset.

- [s22] CC1PSX SELF-DISPROOF (driver, ruling 2026-09-08): candidate 56267f85ffea scores 6 under our cc1 and 18 under the original cc1psx — SOURCE-SIDE: the original compiler is no closer from this source, so the residual is a spelling not yet found (a pure-C preimage exists by construction).

## s23 (escalation / disposition) — 2026-09-09, HEAD main @ 465f9fd0

Modality: `escalation` (driver-assigned; floor flat at 6 across s1–s22 and eight
distinct modalities). Outcome: `owner-gated`, disposition entry filed at
`pre-slim-2026-10-01:docs/grind/decisions.md:26067` ("LADDER EXHAUSTED (non-endgame residual,
floor 6): ROTATED"). The item is rotated to the back of the active worklist and
returns automatically on queue drain / toolchain change / sibling movement.

### Chassis re-measurement (mandatory, brief said "measurement unavailable")
`memory/grind/func_8005D554/candidate.c` applied to `src/text1b.c`, then
`sandbox func_8005D554 --disable all`: **score 6, target_insns 176,
build_insns 176, rules_dropped 0, cheat_asm_stripped 155** (the stripped 155 is
the INCLUDE_ASM body on main, not a coercion). `tools/fake_ablate.py --func
func_8005D554 --file text1b --candidate memory/grind/func_8005D554/candidate.c`
→ "no FAKE-annotated constructs found; nothing to ablate". **The floor is
FAKE-free**, so the mandated "kill measured under a FAKE carrier" contamination
check comes back clean for the whole ledger: no banked kill on this chassis had a
FAKE carrier occupying the contested pseudo.

### Kill re-audit (mandatory — floor flat, instance kills exist)
The three banked forms that sit closest to the target were re-measured on the
current chassis:

| form | banked | re-measured s23 |
|---|---|---|
| `rejected/fresh-single-set-base-gap1-birth-boost-fires-scores-8.c` | 8 | **8 / 176** |
| `rejected/zero10-dep-folded-by-combine-byte-identical-to-boost-ctl-scores-8.c` | 8 | **8 / 176** |
| `rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c` | 6 | **6 / 176** |

All three reproduce exactly. No banked instance kill on this function is void
through chassis drift or FAKE contamination.

### Frontier item 1 CLOSED — the explicit-CLOBBER / `may_not_optimize` escape
The s18–s22 frontier proposed making the a2 base a fresh single-set local that
escapes LICM via `may_not_move` rather than via source non-invariance, on the
theory that `may_not_move` is set by an explicit `(clobber (reg))` while
`reg_n_sets` stays 1 (keeping `birthing_insn_p`'s precondition).

Read out of the compiler source this session:

- `tools/gcc-2.7.2/regclass.c`, `reg_scan_mark_refs` (function head at
  `regclass.c:1736`): `reg_n_sets[REGNO (dest)]++` happens ONLY under
  `case SET:`. A CLOBBER's REG operand is reached through the generic `fmt`
  recursion and lands in `case REG:`, which updates `regno_first_uid` /
  `regno_last_uid` but never `reg_n_sets`. **So the premise is true for
  `reg_n_sets`.**

- BUT `count_loop_regs_set` (`tools/gcc-2.7.2/loop.c:3018`) does two separate
  things with a CLOBBER: it sets `may_not_move[regno] = 1` (loop.c:3018–3022,
  "Don't move a reg that has an explicit clobber"), AND — in the very next block,
  `if (GET_CODE (PATTERN (insn)) == SET || GET_CODE (PATTERN (insn)) == CLOBBER)`
  at `loop.c:3024`–`3049` — it increments `n_times_set[regno]` exactly as for a
  SET.

Consequence: the clobber route suppresses the hoist but leaves
`reg_n_sets == 1`, so `sched.c:2505 birthing_insn_p` still fires the LAUNCH
boost — which is precisely what the s18 G1 chassis measured at **8**. The route
lands on the score-8 chassis by construction, not on the score-0 one. It was
measured anyway, using the only emitter of a standalone `(clobber (reg))` for an
SImode pseudo that is reachable from C source (`expr.c:2996`,
`store_constructor` building an aggregate into a register — the others are
`expr.c:1991` multi-word moves i.e. the forbidden DImode-chain family,
`stmt.c:2794` BLKmode return copies, and `optabs.c` libcall expansion):

| probe | spelling | score / insns |
|---|---|---|
| `tmp/grind/func_8005D554/s23/P1_union_clobber_both.c` | one-member `union { s32 i; }` with a non-constant initialiser carrying `(s32)r4 - K` in BOTH halves | **54 / 178** |
| `tmp/grind/func_8005D554/s23/P2_union_clobber_half1.c` | same, half 1 only | **35 / 178** |

The union carrier does not fold away — it materialises +2 instructions (178 vs
the target's 176) and scrambles the allocation. Banked as
`rejected/union-constructor-clobber-carrier-costs-2-insns-scores-54.c` and
`rejected/union-constructor-clobber-half1-only-scores-35.c`. Independently of the
measurement, the construct is outside the frozen sanctioned-family list and would
be an AUTO-REJECT under the owner's 2026-08-24 ruling, so it is not a candidate
route under any score.

### Frontier item 2 CLOSED — the `reg_in_basic_block_p` first-uid escape
`loop.c:1062` opens with `if (regno_first_uid[regno] != INSN_UID (insn)) return
0;`. Disjunct 3 of `loop.c:695`–`700` therefore goes false only when the base
register is MENTIONED at a lower uid than its set. `reg_scan_mark_refs` fills
`regno_first_uid` from ANY REG occurrence, and every REG occurrence in a compiled
function comes from either a read or a set of the C object. So the escape needs
either (a) a second set of the base — the Judge-banned fresh multi-write staging
carrier, or the existing-local borrow that s17 swept flat at best 19 — or (b) a
read of the base before its only set, which reads an uninitialised value on the
first iteration and is a semantic change rather than a spelling. Both routes
reduce to constructs already disposed of; the disjunct-3 escape carries no new
search space.

### Disposition gates

- **Gate (a) canonical-asm scan: FAIL.** `python3 tools/scan_hand_coded.py
  --single func_8005D554` → `tier=LOW score=0/8`, "no strong hand-coded
  indicators"; S1–S8 all unchecked. Compiler-shaped code; no grant path.

- **Gate (b) SOTN-master precedent: FAIL.** The closing construct (a fresh local
  written more than once purely as a staging carrier for call-argument emission
  order) has no entry in `docs/reference/sotn-construct-index.md`. The only
  reassignment hit is `src/st/rcat/e_frozen_half.c:451`, a comment marking a
  program BUG. The nearest class `new_var_temp` (20 PSX hits, index lines
  1423–1442) is declaration-shape only — the index cannot show write
  multiplicity and no sotn-decomp checkout is available here to exhibit one.

- **cc1psx self-disproof** (driver-banked `state.json.cc1psx_check`,
  2026-09-10T00:24Z): ours 6, cc1psx 18, `closer: false`.

### What a future session should NOT redo
The LICM-escape axis is finished: both source-level escapes for an invariant
single-set base (`may_not_move` via CLOBBER, `regno_first_uid` via earlier
mention) are now closed with source citations and, for the first, a measurement.
Do not re-derive them. The only construct ever measured at distance 0 remains the
Judge-FAILed fresh multi-write carrier; re-activation requires an owner class
grant or an exhibited sotn-decomp master body for that shape (see the
re-activation triggers in `pre-slim-2026-10-01:docs/grind/decisions.md:26067`).

- [s23] Chassis re-measured this session: candidate.c applied to src/text1b.c scores 6 with target_insns 176, build_insns 176, rules_dropped 0, cheat_asm_stripped 155 (the stripped 155 is the INCLUDE_ASM body on main, not a coercion). The brief's chassis check said 'measurement unavailable'; the ledger's floor of 6 is confirmed.

- [s23] The floor body carries no FAKE construct at all (fake_ablate: nothing to ablate), so no banked kill for this function is contaminated by a carrier occupying the contested pseudo.

- [s23] Gate (a) FAILS: tools/scan_hand_coded.py --single func_8005D554 -> HAND_CODED tier=LOW score=0/8, 'no strong hand-coded indicators', S1-S8 all unchecked (0 multu/mflo pairs, no empty-body branches, 29 spills over 14 distinct regs, max load burst 2, no high-similarity siblings, no BIOS jumptable, no unsaved $sN, no redundant mask-before-shift).

- [s23] Gate (b) FAILS: docs/reference/sotn-construct-index.md (sotn-decomp master aa535002) has no entry for a fresh local written more than once as a staging carrier for call-argument emission order. The only reassignment hit is src/st/rcat/e_frozen_half.c:451, a comment marking a program BUG. The nearest class new_var_temp (20 PSX hits, index lines 1423-1442) records declaration lines only, so write multiplicity cannot be shown, and no sotn-decomp checkout is available locally to exhibit one.

- [s23] cc1psx self-disproof banked by the driver in state.json.cc1psx_check (2026-09-10T00:24Z): ours 6, cc1psx 18, closer=false. The residual is not a compiler-provenance artifact.

- [s23] Exhaustion accounting: 23 sessions, floor flat at 6 since s1, eight distinct modalities (recon, structural, permuter, enumerate, synthesis, solver, forensics, rederive) plus this escalation session; ~2,500 spellings measured; 104 banked rejected forms (two added this session); 45 instance kills and 15 predicate-cited class kills.

- [s23] The residual is two identical 3-instruction rotations, one per loop half: we emit [addiu a2,s4,-K][addiu a0,sp,16][lw v1,gp][move a1,zero], the target emits [addiu a0,sp,16][addu a1,zero,zero][lw v1,gp][addiu a2,s4,-K] (asm/funcs/func_8005D554.s 0x4DEB4-0x4DEC0). Registers and frame are byte-identical to the target at 176/176.

- [s23] The only body ever measured at distance 0 is rejected/judge-failed-fresh-multiwrite-nv-nw-carrier-scores-0.c, FAILed by the Judge at FINAL CALL on 2026-09-08 (pre-slim-2026-10-01:docs/grind/decisions.md:26063) with a binding ban on fresh multi-write staging carriers; the driver rejects that body on resubmission without review.

- [s23] Disposition entry filed this session at pre-slim-2026-10-01:docs/grind/decisions.md:26067, titled '2026-09-09 - func_8005D554 (src/text1b.c) - OWNER-ESCALATION - LADDER EXHAUSTED (non-endgame residual, floor 6): ROTATED' (floor 6 > ENDGAME_LOCK_MAX_FLOOR 5, so the 2026-07-27 standing ruling is not this function's subject, per owner ruling 2026-09-02).

- [s23] src/text1b.c was restored to its pristine INCLUDE_ASM state at the end of the session; the only modified surfaces are docs/grind/decisions.md and memory/grind/func_8005D554/.

## s24 (manual lane oct2-b5, 2026-10-02, HEAD main @ 926ed40c9) — Q91 re-judge, distance 0

Q91 (`.claude/rules/completion-bar.md`) re-opens the 2026-09-08 refusal of the nv/nw carrier: it
rested on "outside the frozen list", and under item 3 a reused local with no semantic reading is
simply an annotated FAKE. Item 3 still refuses cross-symbol address derivation, which the old
score-0 body also had (`p_b390 = p_b388 + 2` reaching D_8009B390 from D_8009B388's address).

Re-baseline: `rejected/judge-failed-fresh-multiwrite-nv-nw-carrier-scores-0.c` = 0/176 on this HEAD.

Cross-symbol removal (old-body chassis): `p_b390 = &D_8009B390` 37/174; `s.p1 = &D_8009B390`
direct 39/174; both tables direct 40/174; p_b390 first / p_b390 set in the loop 37/174, 39/174.
The two lost insns are the hdr1_row spill (sp+0x40): the D_8009B388 base loses $s7.
FIX: one object `extern Unk8009B400Record D_8009B388[2]` (two adjacent 8-byte cells) and
`s.table = &D_8009B388[0]` / `&D_8009B388[1]` directly — 0/176, no pointer locals needed.
Same body with two separate symbols (`&D_8009B388`, `&D_8009B390`): 40/174.

Simplifications that keep 0/176: `D_800A326C %= 4;` for the v0/v3 bias sequence; `i += 1` at the
end of the loop body; `y = tmpN + rnd` as one expression (the y side needs no split); block
`extern rand` / `extern D_800A3418` dropped (file scope already declares them); Env5E54C
(honest field names, same 0x2C layout as S46C) instead of S46C (zero18/zero1C would be false names).

Ablation table on the landed body (memory/grind/func_8005D554/candidate.c; each row
removes one construct, score/insns):
| construct | removed spelling | score |
|---|---|---|
| scale = 0x100 holder | literal at both scale stores | 15/176 |
| ot = 1 holder | literal | 2/176 |
| scale + ot together | literals | 17/176 |
| x split init (both halves) | one expression | 8/178 |
| hdr0 (D_8009B2E0 alias) | D_8009B2E0 direct | 41/174 |
| hdr1 (hdr0 + 0xC) | folded into hdr1_row | 2/176 |
| hdr1_row (hoisted row address) | hdr1 + row_off inline | 24/176 |
| hdr0+hdr1+hdr1_row together | D_8009B2E0 direct everywhere | 34/175 |
| integer sum row_off + (s32)hdr0 | pointer sum hdr0 + row_off | 1/176 |
| tmp second write (tmpN = ft4) | s.ft4_out = ft4 | 54/178 |
| tmp early y base | y = base_y - K + rnd | 15/176 |
| both tmp writes together | no carriers | 15/176 |
| one carrier for both halves | tmp1 everywhere | 30/178 |
| table pointer locals (cell0/cell1 = cell0 + 1) | — (not needed with [0]/[1]) | 0/176 |

All rows nonzero except the last, so every remaining FAKE is load-bearing (item 5).
