# Hypothesis ledger — _exeque

## [s1] H1 — CONFIRMED: struct-based `_que[idx].field` (already-merged GpuQueueItem
object model, include/gpu.h:81-88) is the right object model for the whole
function, matching the DATA MODEL brief's SPLIT-AGGREGATE signal on
`_que_plus_0x4`/`_que_plus_0x8`.
Mechanism: no GCC pass — this is a declaration-correctness question, not
codegen. Probe: write the function against `_que[idx].func/.arg/.count`
(never against bare `_que_plus_0x4`/`_que_plus_0x8` externs) and measure.
Result: canonical distance 187 -> 24 on first compile; instruction diff
shows the call-site struct-member address computation matches target's
per-field recompute pattern exactly. CONFIRMED, not re-testable-needed —
this is now the committed object model for the function.

## [s1] H2 — CONFIRMED: `D_8009BF84` was undeclared in the TU; adding
`extern s32 D_8009BF84;` is required (bug fix, not a technique).
Mechanism: none (missing declaration, not a codegen lever) — GCC 2.7.2
silently implicit-declared the undeclared identifier and produced wrong
codegen for the `SetIntrMask(D_8009BF84)` call (loaded a0=0 instead of the
global's value; confirmed via objdump -dr showing the R_MIPS_26 SetIntrMask
relocation's delay slot was `move a0,zero` before the fix).
Probe: add the declaration, re-run `sandbox _exeque --disable all`.
Result: score 24 -> 15, build_insns 183 -> 186. CONFIRMED.
kill_scope: n/a (this is a positive confirmation, not a kill).

## [s1] H3 — KILLED (instance): marking `D_8009BF6C`/`D_8009BF70` volatile
does NOT fix the remaining floor-15 residual, and would contradict banked
project evidence.
Mechanism: n/a — not measured this session (see rationale). The claim being
killed is "the residual triple-store block needs a volatile declaration
fix", based on inspecting `volatile_extern_allowlist.txt:75` (the
D_8009BF68 grant's OWN justification text): "the SAME printf folds the
non-volatile adjacent siblings D_8009BF6C and D_8009BF70" — i.e. the
project's existing get_alarm-derived evidence already establishes these two
globals are NOT volatile in the original source (they get folded in a
DIFFERENT function, get_alarm, that reads all three siblings). Marking them
volatile in _exeque would silently contradict that grant's evidence chain
for the SAME symbols.
kill_scope: instance (this specific declaration-level fix, on the current
struct-based chassis, no FAKE constructs present).
measured_on: chassis = struct-based `_que[idx].field` body (this session's
candidate.c), no FAKE/cheat constructs present; NOT independently
re-measured with volatile added (killed by existing cross-function evidence
in volatile_extern_allowlist.txt:75, not a fresh sandbox run).

## Frontier for next session

1. **Scheduling fix for the post-call triple-store block** (floor 15, all
   in one place). Target strictly interleaves compute+store per field
   (`load .arg -> store D_8009BF6C -> reload idx -> load .count -> store
   D_8009BF70`); our build's list scheduler groups both stores at the end,
   after the idx increment and loop-continuation reload. Next probes:
   (a) read the `.sched` dump (`pwsh tools/grinder/dump.ps1 _exeque` then
   `tmp/grind/_exeque/dumps/*.sched`) to see WHY the scheduler defers the
   two stores — likely `INSN_PRIORITY`/dependency-height reasons per
   [[sched-rank-class-tie-wall]] or a `cse.c` block-extension effect per
   [[cse-block-extension-controls-fold-span]]; (b) try reordering the C
   statements (e.g. interleave the `D_8009BF7C` increment differently, or
   duplicate the idx-read the way [[split-read-defeats-hoist]] does) —
   ordinary-C reorderings only, no volatile/FAKE constructs, since H3 rules
   out the volatile route for these two globals specifically.
2. If (1) doesn't close it, run the permuter directed at just this
   16-instruction region (`PERM_*` macros on the triple-store block) per
   [[permuter-directives]] — this is a small, well-isolated residual
   (floor 15/187, single contiguous region) so a directed permuter run is
   cheap and likely to find the exact statement form.
3. Re-run `diagnose _exeque` once splat/scan tooling recognizes the
   now-non-INCLUDE_ASM body (it currently reports "not found in
   tmp/scan/display/display.purec.o" — stale scan cache from when the
   function was still INCLUDE_ASM; a fresh `canonical`/`sandbox` pair is
   already sufficient and was used this session instead).

## [s1] The struct-based object model (_que[idx].func/.arg/.count via include/gpu.h's already-declared GpuQueueItem, per the DATA MODEL SPLIT-AGGREGATE signal on _que_plus_0x4/_que_plus_0x8) is the correct object model for _exeque.
- mechanism: none (declaration correctness, not a codegen pass)
- probe: Write the full function against _que[idx].field exclusively (never bare _que_plus_0x4/_que_plus_0x8 externs); measure canonical + sandbox distance.
- result: canonical distance 187 -> 24 immediately; instruction diff (tmp/grind/_exeque/s1/diagdiff.py) shows the call-site struct-member address computation matches target's per-field recompute pattern exactly in that region.
- verdict: CONFIRMED

## [s1] D_8009BF84 (the saved interrupt mask _exeque writes at entry and restores at exit) had no extern declaration anywhere in src/display.c before this session, and adding one is required for correct codegen.
- mechanism: none (missing declaration; GCC 2.7.2 silently implicit-declared the undeclared identifier, producing a wrong argument load for the SetIntrMask(D_8009BF84) call)
- probe: Add `extern s32 D_8009BF84;` alongside the other D_8009BE7C/D_8009BE80 externs; re-run sandbox _exeque --disable all.
- result: score 24 -> 15, build_insns 183 -> 186 (target is 187). Confirmed via objdump -dr showing the SetIntrMask call's delay slot changed from `move a0,zero` (wrong) to loading the actual global.
- verdict: CONFIRMED

## [s1] Marking D_8009BF6C/D_8009BF70 volatile (to force the target's strict load-then-immediate-store ordering in the post-call triple-store block) is NOT the fix for the remaining floor-15 residual on this chassis.
- mechanism: n/a - not measured this session; killed by existing project evidence, not a fresh probe
- probe: Inspected volatile_extern_allowlist.txt:75 (the D_8009BF68 grant's own justification), which states the SAME printf in get_alarm folds the non-volatile adjacent siblings D_8009BF6C and D_8009BF70 - i.e. project evidence already establishes these two are non-volatile in the original source.
- result: Not independently re-measured with volatile added on this candidate; the existing cross-function grant evidence for the same two symbols already rules the fix out, so no sandbox run was spent confirming it would fail.
- verdict: KILLED
- kill_scope: instance
- measured_on: struct-based _que[idx].field chassis (this session's candidate.c), no FAKE/cheat constructs present

## [s2] H4a — KILLED (instance): swapping the source order of the `D_8009BF6C = arg;` / `D_8009BF70 = count;` statements does not change the scheduler's register/timing choice for the triple-store residual.
- mechanism: GCC 2.7.2 sched.c list scheduler — statement source order within a straight-line block does not change insn priority/dependency-class ranking for these two independent stores.
- probe: swapped statement order (count store before arg store); re-ran `sandbox _exeque --disable all`; re-ran `diagdiff.py` to compare exact objdump.
- result: score unchanged at 15; objdump diff identical in shape (still `lw a1,8(at)` / `lw a0,4(at)` two-register split, both stores deferred to just before the loop-continue branch) — only WHICH register loaded which offset changed (a1<->a0 swapped), confirming pure statement-order has no effect here.
- verdict: KILLED
- kill_scope: instance
- measured_on: struct-based `_que[idx].field` chassis, s1 form (no `mask` reuse yet), no FAKE/cheat constructs present

## [s2] H4b — KILLED (instance): moving the `D_8009BF7C = (D_8009BF7C+1)&0x3F;` increment statement to BETWEEN the `mask=count` load and the `D_8009BF70=mask` store (instead of after both stores) does not change the scheduler's output.
- mechanism: same as H4a — GCC 2.7.2's list scheduler orders these insns by dependency height/class, not textual position, for straight-line independent statements.
- probe: with the `mask`-reuse form (H5a) in place, moved the increment statement up between the two `mask=`/`D_8009BF70=mask` lines; re-ran sandbox + diagdiff.
- result: score unchanged at 15; objdump diff byte-identical to the pre-move `mask`-reuse form (same register `a0` reused for both fields, same store-deferral shape).
- verdict: KILLED
- kill_scope: instance
- measured_on: struct-based `_que[idx].field` chassis with the H5a `mask`-reuse triple-store, no FAKE/cheat constructs present

## [s2] H5a — CONFIRMED (partial, non-closing): reusing the existing dead-after-use `mask` local (SOTN-sanctioned "variable reuse for codegen control" family) to hold both `.arg` and `.count` field values sequentially, instead of relying on GCC's own temp allocation, changes the triple-store block's register allocation from a TWO-register split (a1+a0) to a SINGLE reused register (a0) for both field loads — structurally closer to target's single-register (`v0`) reuse pattern, but does not by itself move the sandbox score (still 15 at the time it was isolated; see H5b for the score-moving change).
- mechanism: WAR (write-after-read) anti-dependency on the `mask` pseudo forces GCC's allocator to serialize the two field loads onto the same hard register instead of allocating two independent ones.
- probe: `mask = (s32)_que[D_8009BF7C].arg; D_8009BF6C = mask; mask = _que[D_8009BF7C].count; D_8009BF70 = mask;` in place of two independent-local field stores; sandbox + diagdiff.
- result: score unchanged (15->15 in isolation), but objdump shows both field loads now land in `a0` (matching target's practice of reusing one register across both fields, though target's register is `v0` not `a0`). Kept in candidate.c since it's a genuine SOTN-sanctioned, non-regressing structural improvement.
- verdict: CONFIRMED (structural convergence; not a closing lever by itself)

## [s2] H5b — CONFIRMED: pre-computing a pointer to `D_8009BE7C` (`s32 *p = &D_8009BE7C;`) and dereferencing it for BOTH the guard read (`*p != 0`) and the clear store (`*p = 0;`) in the function's final "clear the pending flag and invoke the queued callback" block closes that ENTIRE block to a byte-exact match (only masked branch-target diffs remain) and drops the sandbox floor from 15 to 12.
- mechanism: `defeat-combine-symbol-fold` (codegen-technique-index) — reading the global by name twice re-derives `%hi(D_8009BE7C)`/`%lo(D_8009BE7C)` addressing at each access; pre-computing the address into a pointer local forces GCC to materialize the base ONCE and reuse the same register for the later store, matching target's `lui v1; addiu v1,v1,0; lw v0,0(v1); ...; sw zero,0(v1)` shape exactly (register `v1` reused for both the load and the later store).
- probe: added `s32 *p = &D_8009BE7C;` before the guard `if`, replaced `D_8009BE7C != 0`/`D_8009BE7C = 0` with `*p != 0`/`*p = 0`; sandbox + diagdiff (full objdump comparison via `tmp/grind/_exeque/s2/fulldump.py`).
- result: score 15 -> 12 (build_insns 186 -> 185, target 187); diagdiff shows the ENTIRE final-callback block (`bne`/`bnez`/`beqz` chain through the `sw zero,0(v1)` clear) matching target with only masked branch-offset differences remaining. This is genuine, ordinary, submittable pure C (no FAKE, no volatile) — banked in candidate.c.
- verdict: CONFIRMED
- kill_scope: n/a (positive confirmation)

## [s2] H6 — KILLED (instance, NOT submittable): making the `D_8009BE7C` pointer `volatile` (`volatile s32 *p = &D_8009BE7C;`) closes ONE more instruction (score 12 -> 10) by preventing cc1's delay-slot filler from moving the `*p = 0;` store into the trailing `jalr v0` delay slot (target keeps an unfilled nop there instead) — but this construct does NOT qualify under the current `legitimate-volatile-interrupt-touched` two-prong carve-out: the use-site shape here (single-read guard-and-clear of an interrupt-owned flag) is not one of the three catalogued shapes (spin-wait / double-read-across-sequence-point / IRQ-mutated-loop-bound), even though `_exeque` itself is genuinely IRQ-installed (`DMACallback(2, _exeque);`, display.c:915/927) and would likely satisfy prong 1. Per [[no-new-park-categories]] / [[ordinary-c-judge-decidable]], a construct outside a sanctioned family's documented scope is a FAIL for the worker to self-submit, not a judgment call — so this form is banked as rejected, NOT put in candidate.c.
- mechanism: GCC 2.7.2 `reorg.c` `fill_simple_delay_slots` — a non-volatile store one insn after a call becomes a scheduling-motion candidate for the call's delay slot; volatile-qualified memory accesses are excluded from that motion.
- probe: changed `s32 *p` to `volatile s32 *p` in the H5b form; sandbox + diagdiff.
- result: score 12 -> 10 (build_insns 186, target 187); objdump shows the store now sits BEFORE `jalr v0` with the delay slot as an explicit `nop`, byte-identical to target in that region. Full form banked at `memory/grind/_exeque/rejected/volatile-D_8009BE7C-guard-clear.c` with the scope analysis. NOT adopted into candidate.c.
- verdict: KILLED
- kill_scope: instance (this exact volatile spelling, on the H5b chassis, un-annotated — no FAKE claimed since this isn't a FAKE-eligible family, it's a scope-mismatch against the ONE narrow volatile carve-out)
- measured_on: H5b chassis (candidate.c with `s32 *p` promoted to `volatile s32 *p`), no other FAKE/cheat constructs present

## Frontier for s3 (updated 2026-09-16, session 2)

1. **Triple-store block (floor 12, ~2 of the residual instructions)** — H4a/H4b confirm pure statement/declaration reordering doesn't move it; this is a genuine `sched-rank-class-tie-wall`-shaped residual (dependency-class tie between the field-store chain and the idx-increment/loop-condition reloads). Next probe: read `tmp/grind/_exeque/dumps/display.sched` around the basic-block-11 region (insns 103-222, already dumped this session) for the exact `INSN_PRIORITY`/rank_for_schedule tie-break, or run a directed permuter scoped to just this ~16-instruction region (ledger's original frontier item 2, still valid and now MUCH cheaper since the base is 12 not 15).
2. **`ruling-request` opportunity on H6** — ask whether "an IRQ-callback-installed function's own single-read test-and-clear of a flag it owns" qualifies as a fourth catalogued use-site shape in `.claude/rules/legitimate-volatile-interrupt-touched.md`. If granted, this closes 2 more instructions (floor 12 -> 10) with zero further search. Evidence pointers: `memory/grind/_exeque/rejected/volatile-D_8009BE7C-guard-clear.c`, this hypothesis file's H6 entry, `src/display.c:915` (the `DMACallback(2, _exeque);` IRQ-install site).
3. Re-run `pwsh tools/grinder/dump.ps1 _exeque` fresh next session (dumps are chassis-relative; the H5b pointer-alias change may shift insn numbering in the .sched/.greg dumps for the still-open triple-store region).

## [s2] Swapping the source order of the D_8009BF6C=arg / D_8009BF70=count statements changes the scheduler's register/timing choice for the triple-store residual.
- mechanism: GCC 2.7.2 sched.c list scheduler dependency-class ranking (sched-rank-class-tie-wall shape)
- probe: Swapped statement order on the s1 chassis (no mask reuse yet); re-ran sandbox _exeque --disable all and objdump diff via tmp/grind/_exeque/s1/diagdiff.py.
- result: Score unchanged at 15; objdump diff identical in shape, only which register (a0 vs a1) loaded which offset changed.
- verdict: KILLED
- kill_scope: instance
- measured_on: struct-based _que[idx].field chassis, s1 form (no mask reuse), no FAKE/cheat constructs present

## [s2] Moving the D_8009BF7C increment statement to between the count load and its store (instead of after both field stores) changes the scheduler's output for the triple-store residual.
- mechanism: GCC 2.7.2 sched.c list scheduler dependency-class ranking (same as above)
- probe: Moved the increment statement earlier on the H5a mask-reuse chassis; re-ran sandbox + diagdiff.
- result: Score unchanged at 15; objdump diff byte-identical to the pre-move mask-reuse form.
- verdict: KILLED
- kill_scope: instance
- measured_on: struct-based _que[idx].field chassis with mask-reuse triple-store (H5a), no FAKE/cheat constructs present

## [s2] Reusing the existing dead-after-use `mask` local (SOTN-sanctioned variable-reuse family) to hold both .arg and .count field values sequentially converges the triple-store block's register allocation toward target's single-register-reuse pattern.
- mechanism: WAR anti-dependency on the reused pseudo forces the allocator to serialize both field loads onto one hard register instead of two independent ones
- probe: mask = (s32)_que[D_8009BF7C].arg; D_8009BF6C = mask; mask = _que[D_8009BF7C].count; D_8009BF70 = mask; -- sandbox + diagdiff.
- result: Score unchanged in isolation (still 15 at that point), but objdump shows both field loads now land in a single register (a0) matching target's practice of register reuse across the two fields (target itself reuses v0, not a0). Non-regressing, kept in candidate.c.
- verdict: CONFIRMED

## [s2] Pre-computing a pointer to D_8009BE7C and dereferencing it for both the guard read and the clear store in the final callback block closes that block to a byte-exact match.
- mechanism: defeat-combine-symbol-fold (codegen-technique-index): a pointer local materializes the global's address once and lets GCC reuse the same register for a later store, instead of re-deriving %hi/%lo addressing at each named access
- probe: s32 *p = &D_8009BE7C; if (*p != 0 && D_8009BE80 != 0) { *p = 0; ((s32(*)(void))D_8009BE80)(); } in place of two separate D_8009BE7C accesses; sandbox + full objdump diff via tmp/grind/_exeque/s2/fulldump.py.
- result: Score dropped 15 -> 12 (build_insns 186 -> 185, target 187). The entire final-callback instruction block now matches target with only masked branch-offset differences remaining.
- verdict: CONFIRMED

## [s2] Marking the D_8009BE7C pointer volatile closes one more instruction by preventing the jalr delay-slot filler from moving the clear-store into the call's delay slot, but this construct does not qualify under the current legitimate-volatile-interrupt-touched two-prong carve-out (the use-site shape -- single-read guard-and-clear -- is not one of the three catalogued shapes) even though _exeque itself is genuinely IRQ-installed via DMACallback(2, _exeque).
- mechanism: GCC 2.7.2 reorg.c fill_simple_delay_slots excludes volatile memory accesses from delay-slot motion candidacy
- probe: Changed `s32 *p` to `volatile s32 *p` on the H5b chassis; sandbox + diagdiff.
- result: Score 12 -> 10 (build_insns 186, target 187); the store now sits before jalr with an explicit nop in the delay slot, byte-identical to target in that region. Banked as rejected (memory/grind/_exeque/rejected/volatile-D_8009BE7C-guard-clear.c) rather than submitted, since the use-site shape is not in the frozen carve-out's three-shape catalog and self-authorizing a new shape is not this worker's call.
- verdict: KILLED
- kill_scope: instance
- measured_on: H5b chassis (candidate.c with s32 *p promoted to volatile s32 *p), no other FAKE/cheat constructs present
