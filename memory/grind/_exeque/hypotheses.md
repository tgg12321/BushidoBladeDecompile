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

## Frontier for s4 (updated 2026-09-16, session 3)

1. **`ruling-request` opportunity on H6 — highest-value remaining lever.**
   Ask whether "an IRQ-callback-installed function's own single-read
   test-and-clear of a flag it owns" qualifies as a fourth catalogued
   use-site shape in `.claude/rules/legitimate-volatile-interrupt-touched.md`.
   If granted, this closes 2 more instructions (floor 12 -> 10) with zero
   further search. Evidence pointers: `memory/grind/_exeque/rejected/volatile-D_8009BE7C-guard-clear.c`,
   this hypothesis file's H6 entry, `src/display.c:915` (the
   `DMACallback(2, _exeque);` IRQ-install site). Unchanged since s2 —
   still not spent.
2. **Triple-store block (floor 12, ~2 of the residual instructions) is now
   evidence-exhausted for pure C-level statement/variable respelling
   WITHIN the block** — H4a, H4b (s2) and H7's three variants (s3) are five
   independently-measured spellings, all byte-identical, and H8 (s3) traces
   the exact `rank_for_schedule` tie via the instrumented cc1's
   `BB2_RANK_DEBUG` hook to a class+priority tie resolved by `INSN_LUID`
   (RTL-generation order), which none of the five spellings altered
   relative to the competing field's recompute chain. The one UNTRIED axis:
   changing the LOOP-LEVEL structure so the store's LUID lands on the far
   side of the .count recompute chain — e.g. restructure the two field
   accesses so the SECOND field (`.count`) is computed+stored FIRST inside
   the loop but the FIRST field (`.func`, already stored earlier at
   `D_8009BF68[0]`) is what feeds the LAST store — this reshuffles which
   insn is "the one being deferred" rather than whether deferral happens,
   so it is unlikely to help without also changing target's own field
   emission order (which the s1 H1-confirmed object model must preserve).
   A directed permuter scoped to just this ~16-instruction region
   (`tmp/grind/_exeque/dumps/display.sched:16329-17507` for the function's
   full RTL) is the more promising next step — cheap now that the base is
   12/187, not 15/187 or 187/187.
3. Re-run `pwsh tools/grinder/dump.ps1 _exeque` fresh again next session if
   the chassis changes (this session's dumps, taken fresh at floor-12, are
   in `tmp/grind/_exeque/dumps/`; `tmp/grind/_exeque/s3/rankdbg_run.py` is
   the reusable BB2_RANK_DEBUG harness — rerun it after any edit to the
   triple-store block to get a fresh RANKDBG trace).

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

## [s3] H7 — KILLED (instance): three independently-spelled variants of the
post-call triple-store block (s2's `mask`-reuse; two freshly-named locals
`arg_val`/`count_val` both loaded before either store; a direct
mask-free assignment with no intermediate local) all produce
BYTE-IDENTICAL object code — sandbox score 12 and objdump of the
triple-store region identical down to register numbers, in every case.
- mechanism: n/a for the comparison itself (three ordinary-C spellings
  measured against each other); the underlying compiler mechanism this
  disproves any C-level fix via is GCC 2.7.2's `rank_for_schedule`
  (tools/gcc-2.7.2/sched.c:2417-2464) — see H8 for the dump-verified trace.
- probe: rewrote the block three ways on the identical surrounding chassis
  (s2's H5b final-callback pointer unchanged); ran `sandbox _exeque
  --disable all` after each; diffed `mipsel-linux-gnu-objdump -dr
  tmp/sandbox/_exeque/display.o` for the triple-store region (offsets
  0x20e0-0x2218 in the disabled object) across all three.
- result: all three: score 12, build_insns 185, byte-identical
  objdump in the triple-store region (both stores land at the same two
  instructions, `sw a1,%lo(D_8009BF6C)(at)` / `sw a0,%lo(D_8009BF70)(at)`,
  immediately before the loop-continuation `beq`, in every variant).
  Kept the direct-assignment form (fewest constructs) in candidate.c per
  [[ordinary-c-judge-decidable]] Ruling 1(4).
- verdict: KILLED
- kill_scope: instance
- measured_on: struct-based `_que[idx].field` chassis with the s2 H5b
  final-callback pointer in place; no FAKE/cheat constructs present in any
  of the three variants.

## [s3] H8 — CONFIRMED (diagnostic, not a fix): the instrumented cc1's
`BB2_RANK_DEBUG` hook (tools/gcc-2.7.2/cc1, NOT tools/gcc-2.7.2/build/cc1
— see [[instrumented-cc1-location]]) proves the D_8009BF6C store (insn 189)
ties with insn 198 (part of the .count field's address recompute chain) in
BOTH `INSN_PRIORITY` (8) and dependency class (3 = independent of the
last-scheduled insn, insn 204) when `rank_for_schedule` compares them, so
the decision falls through to the final `INSN_LUID` tiebreak — which is
fixed by RTL-generation (= C statement) order and is IDENTICAL across
every spelling this session and s2 tried (H7's three variants, s2's H4a
statement-swap, s2's H4b increment-move), because the store is always
generated in the same relative position versus the OTHER field's recompute
chain regardless of which local (if any) carries the value.
- mechanism: GCC 2.7.2 `rank_for_schedule` (tools/gcc-2.7.2/sched.c:2417-2464):
  priority tie (line 2418) -> dependency-class tie relative to
  `last_scheduled_insn` (lines 2421-2458) -> `INSN_LUID` tiebreak (lines
  2461-2464, `return INSN_LUID(tmp) - INSN_LUID(tmp2);`).
- probe: built `tmp/grind/_exeque/s3/rankdbg_run.py` (preprocesses
  src/display.c, invokes the INSTRUMENTED `tools/gcc-2.7.2/cc1` — not the
  plain build/cc1 — with `BB2_RANK_DEBUG=1` in its env, `-da` dump flags);
  captured stderr to `/tmp/rankdbg3.log` (1050 RANKDBG lines for the whole
  TU); grepped for insn UIDs 189/207 (the two deferred stores identified in
  the fresh `.sched` dump from `pwsh tools/grinder/dump.ps1 _exeque`, read
  at `tmp/grind/_exeque/dumps/display.sched:16542-16583` this session).
- result: `RANKDBG last=204 y=198 cls=3 x=189 cls2=3 val=0` — confirms the
  exact tie (val=0 means the class comparison itself was also a tie, so
  the caller falls through to the LUID compare, which this session's five
  measured spellings (H7 x3 + s2 H4a/H4b) show is invariant to every
  tested C-level respelling of this specific block). This is DIAGNOSTIC
  evidence explaining WHY H4a/H4b/H7 all measured null — it does not
  itself close the residual, and no C form that changes ONLY this block's
  internal statement/variable spelling (holding the surrounding loop and
  field order fixed) has been found to break the tie.
- verdict: CONFIRMED
- kill_scope: n/a (positive/diagnostic confirmation, not a kill of a
  proposed fix)

## [s3] Reusing three independently-spelled forms of the post-call triple-store block (s2's mask-reuse; two fresh locals arg_val/count_val both loaded before either store; a direct mask-free assignment) changes the scheduler's placement of the D_8009BF6C/D_8009BF70 stores.
- mechanism: n/a for the comparison; underlying mechanism traced separately (see next hypothesis)
- probe: Rewrote the block three ways on the identical s2 H5b chassis; ran sandbox _exeque --disable all after each; diffed mipsel-linux-gnu-objdump -dr tmp/sandbox/_exeque/display.o for the triple-store region across all three.
- result: All three variants: sandbox score 12, build_insns 185, byte-identical objdump in the triple-store region (both stores land at the same two instructions immediately before the loop-continuation beq in every variant). Kept the direct-assignment form (fewest constructs) in candidate.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: struct-based _que[idx].field chassis with the s2 H5b final-callback pointer in place; no FAKE/cheat constructs present in any of the three variants

## [s3] The instrumented cc1's BB2_RANK_DEBUG hook (tools/gcc-2.7.2/cc1) shows the D_8009BF6C store (insn 189) ties with insn 198 (part of the .count field's address recompute) in both INSN_PRIORITY (8) and dependency class (3 = independent of last-scheduled insn 204) in rank_for_schedule, forcing the decision to the final INSN_LUID tiebreak, which is fixed by RTL-generation (C statement) order and is invariant across every C-level respelling of this block tested this session and in s2.
- mechanism: GCC 2.7.2 rank_for_schedule (tools/gcc-2.7.2/sched.c:2417-2464): priority tie at line 2418, dependency-class tie at lines 2421-2458, INSN_LUID tiebreak at lines 2461-2464
- probe: Built tmp/grind/_exeque/s3/rankdbg_run.py (preprocesses src/display.c, invokes tools/gcc-2.7.2/cc1 -- the instrumented binary, not build/cc1 -- with BB2_RANK_DEBUG=1, -da dump flags); captured stderr; grepped for insn UIDs 189/207 identified from a fresh dump.ps1 .sched dump read at tmp/grind/_exeque/dumps/display.sched:16542-16583.
- result: RANKDBG last=204 y=198 cls=3 x=189 cls2=3 val=0 -- confirms the exact class+priority tie. This is diagnostic evidence explaining why five independently measured spellings (this session's 3 + s2's H4a/H4b) all produced byte-identical output; it does not by itself close the residual.
- verdict: CONFIRMED

## [s4] H9 — CONFIRMED: wrapping the post-call triple-store block (D_8009BF68[0]/D_8009BF6C/D_8009BF70) in a single `do { ... } while (0);` breaks the rank_for_schedule LUID tie that H7/H8 (s3) proved was invariant across five plain statement/variable respellings, dropping the sandbox floor from 12 to 7.
- mechanism: do-while(0) wrap (SOTN-sanctioned family, [[do-while-zero-exception]], owner ruling 2026-07-06 -- sanctioned for ANY codegen effect including register/scheduling); the wrap changes the compound-statement's RTL boundary structure, which is exactly the axis H8's diagnostic traced as invariant to same-boundary statement reordering. A directed decomp-permuter campaign found this construct; it was NOT hand-derived first.
- probe: Directed decomp-permuter campaign 1 on a clean single-function workspace (tmp/grind/_exeque/s4/perm_ws/, base permuter score 980, target = asm/funcs/_exeque.s assembled standalone). `tools/permuter_campaign.py launch --func _exeque -j4 --stop-on-zero`, waited via `permuter_campaign.py wait` (two ~15-70s windows, 4696 iterations total). Applied the found form (output-615-1/source.c) to src/display.c; measured via `sandbox _exeque --disable all`.
- result: sandbox score 12 -> 7 (build_insns 185 -> 186, target 187). Confirmed via direct objdump -dr diff of tmp/sandbox/_exeque/display.o against a freshly rebuilt target.o that the ENTIRE triple-store region now matches target byte-for-byte; the sole remaining residual is a different, previously-known site (H6's final-callback jalr delay slot).
- verdict: CONFIRMED

## [s4] H10 — CONFIRMED: nesting a SECOND do-while(0) inside the first, wrapped around only the first two stores (D_8009BF68[0]/D_8009BF6C, leaving D_8009BF70's store and the D_8009BF7C increment outside both wraps), drops the floor further from 7 to 2. Single-level wrap (H9) is measurably insufficient on its own -- satisfies [[do-while-zero-exception]]'s prerequisite for nested wraps.
- mechanism: same reorg.c/rank_for_schedule ordering axis as H9, applied to a narrower RTL compound-statement boundary nested one level deeper.
- probe: Directed decomp-permuter campaign 2, re-seeded from the H9 chassis (base.c updated to the do-while-wrapped src/display.c, base permuter score 615). ~2888 iterations. Applied the found form (output-200-1/source.c) to src/display.c; measured via sandbox.
- result: sandbox score 7 -> 2 (build_insns 186, unchanged -- pure reschedule, no insn count change). Direct A/B on the identical surrounding chassis: single-level wrap = floor 7, nested wrap = floor 2 -- this IS the single-level-insufficient justification the rule requires for a nested wrap, not an assertion.
- verdict: CONFIRMED

## [s4] Directed permuter campaign 1 also surfaced `extern volatile int/short/char D_8009BF6C`/`D_8009BF70` coercion forms (output-715-1, output-765-1, output-915-1) scoring between the do-while form and the unwrapped baseline.
- mechanism: n/a -- these are volatile-coercion CHEATS, not a legitimate lever; recorded as a KILL of the "just mark it volatile" temptation, not a proposal.
- probe: Read each output-*/diff.txt from campaign 1's workspace; checked D_8009BF6C/D_8009BF70 against [[legitimate-volatile-interrupt-touched]]'s two-prong gate and the volatile_extern_allowlist.txt precedent already banked in this ledger's s1 evidence (D_8009BF68's own grant explicitly names D_8009BF6C/D_8009BF70 as the NON-volatile siblings).
- result: REJECTED without measurement beyond reading the diff -- these globals have no identifiable IRQ writer independent of _exeque's own queue-draining loop and no catalogued use-site shape; volatile coercion on them contradicts the get_alarm-derived allowlist evidence already in this ledger. Not applied to src.
- verdict: KILLED
- kill_scope: instance
- measured_on: campaign-1 permuter search space (do-while-unwrapped baseline chassis); construct itself never applied to src/display.c

## [s4] Directed permuter campaign 1 also surfaced `output-905-1`: moving `SetIntrMask(D_8009BF84)` from after the loop to inside the loop body (between the two triple-store fields).
- mechanism: n/a -- this changes RUNTIME BEHAVIOR (re-arms the interrupt mask every loop iteration instead of once after the loop exits), not just codegen. The permuter's mutation search does not verify semantic equivalence, only byte score.
- probe: Read output-905-1/diff.txt; traced the moved statement's control-flow position against the surrounding do/while loop structure.
- result: REJECTED as not a candidate at all (behavior-changing, not a respelling) -- not measured further, not applied to src.
- verdict: KILLED
- kill_scope: instance
- measured_on: campaign-1 permuter search space; construct never applied to src/display.c

## [s4] Wrapping the post-call triple-store block (D_8009BF68[0]/D_8009BF6C/D_8009BF70) in a single do { ... } while (0); breaks the rank_for_schedule INSN_LUID tie that s3's H7/H8 proved was invariant across five plain statement/variable respellings.
- mechanism: do-while(0) wrap changing the compiled RTL compound-statement boundary, per GCC 2.7.2 rank_for_schedule (tools/gcc-2.7.2/sched.c:2417-2464) as diagnosed in s3 H8
- probe: Directed decomp-permuter campaign 1 on a clean single-function workspace (tmp/grind/_exeque/s4/perm_ws/, base permuter score 980); applied the found form (output-615-1) to src/display.c; measured via sandbox _exeque --disable all
- result: sandbox score 12 -> 7 (build_insns 185 -> 186, target 187)
- verdict: CONFIRMED

## [s4] Nesting a second do-while(0) inside the first, wrapped around only the first two stores (D_8009BF68[0]/D_8009BF6C), drops the floor further from 7 to 2; the single-level wrap alone is measurably insufficient, satisfying do-while-zero-exception's nested-wrap prerequisite.
- mechanism: same reorg.c/rank_for_schedule RTL-boundary ordering axis as the outer wrap, applied one level deeper on a narrower compound statement
- probe: Directed decomp-permuter campaign 2, re-seeded from the campaign-1 chassis (base permuter score 615); applied the found form (output-200-1) to src/display.c; measured via sandbox
- result: sandbox score 7 -> 2 (build_insns 186, unchanged insn count -- pure reschedule); direct A/B on the identical chassis: single-level = floor 7, nested = floor 2
- verdict: CONFIRMED

## [s4] Marking D_8009BF6C and/or D_8009BF70 volatile (permuter campaign-1 finds output-715-1/765-1/915-1) is not a legitimate lever for this residual.
- mechanism: n/a -- volatile-coercion cheat, not a program-logic construct; rejected on catalog + banked-evidence grounds without a sandbox measurement
- probe: Read each output-*/diff.txt from campaign 1; checked against legitimate-volatile-interrupt-touched's two-prong gate and the s1-banked volatile_extern_allowlist.txt:75 evidence naming D_8009BF6C/D_8009BF70 as the non-volatile control case for the D_8009BF68 grant
- result: REJECTED without src application -- no identifiable IRQ writer independent of _exeque's own loop, no catalogued use-site shape, and direct contradiction of already-banked project evidence
- verdict: KILLED
- kill_scope: instance
- measured_on: campaign-1 permuter search space (do-while-unwrapped baseline chassis); construct never applied to src/display.c

## [s4] Moving SetIntrMask(D_8009BF84) from after the loop to inside the loop body (permuter campaign-1 find output-905-1) is a valid respelling.
- mechanism: n/a -- the move changes runtime behavior (re-arms the interrupt mask every loop iteration instead of once after the loop exits), which the permuter's byte-score search does not itself verify
- probe: Read output-905-1/diff.txt; traced the moved statement's control-flow position
- result: REJECTED as not a valid candidate at all (behavior-changing, not a respelling); never applied to src/display.c
- verdict: KILLED
- kill_scope: instance
- measured_on: campaign-1 permuter search space; construct never applied to src/display.c

## [s4] A third directed permuter campaign (base permuter score 200, sandbox floor 2/187) targeting the sole remaining residual -- the final IRQ-callback block's jalr delay-slot fill, where target keeps sw $zero,0($v1) (D_8009BE7C = 0;) BEFORE jalr with an explicit unfilled nop while our build's reorg.c filler moves the store into the jalr delay slot -- can find a non-volatile C-level lever that s2's H6 did not.
- mechanism: exhaustive PERM_* random search over the now much smaller (2-instruction) residual
- probe: tools/permuter_campaign.py launch --func _exeque -j4 --stop-on-zero on the do-while(0)-wrapped chassis; waited via permuter_campaign.py wait/harvest across ~9100 iterations with zero novel finds below score 200 (the chassis's own base score)
- result: 0 novel finds below 200 after 9096 iterations -- the campaign never found any spelling closer to 0 than the do-while chassis itself. This corroborates (does not newly prove, since s2 already identified the mechanism) that the residual is the same jalr-delay-slot site s2's H6 found closable only via volatile, which random C-statement mutation cannot reach because reorg.c's fill_simple_delay_slots exclusion of volatile-marked memory accesses is not expressible as a plain-C statement reordering.
- verdict: KILLED
- kill_scope: instance
- measured_on: do-while(0)-wrapped chassis (this session's candidate.c, both FAKE-annotated wraps present), s2's H5b final-callback pointer in place, no volatile/cheat constructs present

## [s5] A do-while(0) wrap around the final "clear D_8009BE7C, invoke D_8009BE80 callback" two-statement block does NOT perturb the jalr delay-slot fill (unlike H9/H10's triple-store wraps, which each dropped the floor).
- mechanism: reorg.c fill_simple_delay_slots (tools/gcc-2.7.2/reorg.c:2861-3027) backward-scans from the jalr for a resource-conflict-free trial insn; this is a two-instruction body with no intervening compound-statement boundary for the wrap to change, unlike the triple-store block H9/H10 targeted
- probe: Applied do-while(0) wrap to memory/grind/_exeque/rejected/dowhile-final-block-no-effect.c's shape on the s4 floor-2 chassis; measured via sandbox _exeque --disable all
- result: score unchanged at 2/187 (build_insns unchanged 186) -- confirms this residual is not an RTL-compound-statement-boundary issue the way the triple-store block was; reverted, not applied to src
- verdict: KILLED
- kill_scope: instance
- measured_on: s4 do-while(0)-wrapped floor-2 chassis, single added do-while(0) wrap around the final block only, no other constructs changed

## [s5] Hoisting the D_8009BE80 callback pointer into a named local `cb` before the guard test, calling `cb()` instead of a cast-call-through-global, makes the score WORSE (2 -> 11), not better.
- mechanism: n/a -- this changed codegen structurally (build_insns dropped 186 -> 184, i.e. fewer instructions emitted than target, not just a delay-slot reschedule), so it is not a viable direction for this residual
- probe: Applied the cb-local-hoist form to the s4 floor-2 chassis; measured via sandbox
- result: score 2 -> 11 (worse); reverted immediately, not applied to src (memory/grind/_exeque/rejected/cb-local-hoist-worse.c)
- verdict: KILLED
- kill_scope: instance
- measured_on: s4 do-while(0)-wrapped floor-2 chassis, cb-local hoist applied to final block only

## [s5] A second fresh-seed permuter campaign (15356 iterations, ~9.3 min) on the exact floor-2/187 jalr-delay-slot residual, re-launched from a copy of s4's campaign-3 workspace (same base.c/target.o, base permuter score 200), finds zero novel forms below score 200.
- mechanism: exhaustive PERM_* random search over the 2-instruction residual (same search space as s4 campaign 3)
- probe: tools/permuter_campaign.py launch --func _exeque --dir tmp/grind/_exeque/s5/perm_ws --label jalr-delay-slot-s5 -j4 --stop-on-zero; waited via permuter_campaign.py wait (one ~547s blocking call); harvested + stopped
- result: 0 novel finds after 15356 iterations this session. Combined with s4 campaign 3's 9096 iterations on the identical residual/chassis, cumulative iteration count on this exact search space is now ~24452, crossing the CHASSIS RULE's >=20k threshold -- further re-seeding of this SAME chassis+residual is not a valid probe per the ledger's own discipline. The residual requires either (a) a structurally different chassis for the surrounding block (none of this session's 2 hand-derived structural variants helped -- see the two KILLED hypotheses above), or (b) the ruling-request already on file in the frontier (volatile-based closure, blocked on use-site-shape classification, not on search).
- verdict: KILLED
- kill_scope: instance
- measured_on: s4/s5 do-while(0)-wrapped floor-2 chassis (both FAKE-annotated wraps present, no other cheat/volatile constructs), jalr-delay-slot-targeted permuter search space only

## [s5] A do-while(0) wrap around the final "clear D_8009BE7C, invoke D_8009BE80 callback" two-statement block does NOT perturb the jalr delay-slot fill (unlike H9/H10's triple-store wraps, which each dropped the floor).
- mechanism: reorg.c fill_simple_delay_slots (tools/gcc-2.7.2/reorg.c:2861-3027) backward-scans from the jalr for a resource-conflict-free trial insn; this two-instruction body has no intervening compound-statement boundary for a do-while(0) wrap to change, unlike the triple-store block H9/H10 targeted
- probe: Applied do-while(0) wrap around the final block on the s4 floor-2/187 chassis; measured via sandbox _exeque --disable all
- result: score unchanged at 2/187, build_insns unchanged (186) -- confirms this residual is not an RTL-compound-statement-boundary issue the way the triple-store block was; reverted, not applied to src
- verdict: KILLED
- kill_scope: instance
- measured_on: s4 do-while(0)-wrapped floor-2 chassis, single added do-while(0) wrap around the final block only, no other constructs changed

## [s5] Hoisting the D_8009BE80 callback pointer into a named local `cb` before the guard test, calling `cb()` instead of a cast-call-through-global, improves the residual.
- mechanism: n/a -- speculative register-pressure change, not a named GCC pass mechanism
- probe: Applied the cb-local-hoist form to the s4 floor-2/187 chassis; measured via sandbox
- result: score got WORSE, 2 -> 11 (build_insns dropped 186 -> 184, i.e. this changed codegen structurally rather than just the delay-slot reschedule); reverted immediately, not applied to src
- verdict: KILLED
- kill_scope: instance
- measured_on: s4 do-while(0)-wrapped floor-2 chassis, cb-local hoist applied to final block only

## [s5] A second fresh-seed directed permuter campaign on the exact floor-2/187 jalr-delay-slot residual (re-launched from a copy of s4's campaign-3 workspace, identical base.c/target.o, base permuter score 200) can find a non-volatile C-level lever that s2's H6 and s4's campaign 3 (9096 iterations) did not.
- mechanism: exhaustive PERM_* random search over the 2-instruction residual (same search space as s4 campaign 3)
- probe: tools/permuter_campaign.py launch --func _exeque --dir tmp/grind/_exeque/s5/perm_ws --label jalr-delay-slot-s5 -j4 --stop-on-zero; waited via permuter_campaign.py wait (one ~547s blocking call, 15158-15356 iterations counted at different checkpoints); harvest --stop
- result: 0 novel finds after 15356 iterations this session. Combined with s4 campaign 3's 9096 iterations on the identical residual/chassis, cumulative iteration count on this exact search space is now ~24452, crossing the ledger's own CHASSIS RULE >=20k threshold -- further re-seeding of this SAME chassis+residual is not a valid probe for a future session without a structural change first
- verdict: KILLED
- kill_scope: instance
- measured_on: s4/s5 do-while(0)-wrapped floor-2/187 chassis (both FAKE-annotated wraps present, no other cheat/volatile constructs), jalr-delay-slot-targeted permuter search space only

## [s6] Ran the instrumented-cc1 .dbr dump (pass-attribution discipline) on the s4/s5 floor-2/187 chassis to re-confirm the jalr-delay-slot mechanism before probing further, per role-prompt requirement.

- probe: `pwsh tools/grinder/dump.ps1 _exeque`, then grepped/read `tmp/grind/_exeque/dumps/display.dbr` (reorg.c delay-branch-reorg pass output — the correct dump for delay-slot fill, distinct from `.sched`/`.sched2` which are the two list-scheduler passes before reorg) around `D_8009BE7C`/`D_8009BE80`.
- result: confirmed the exact mechanism the ledger already identified: RTL insn 311 (`(set (mem:SI (reg/v:SI 3 v1)) (const_int 0))` — the `*p = 0;` store, address pre-materialized in hard reg v1 by insn 298) is folded by reorg.c into a `(sequence [call_insn 316 ... insn 311])` — the call_insn (jalr through v0, calling D_8009BE80) followed by the store as its delay-slot fill. No new information beyond what H6/s4/s5 already established; this was pass-attribution due diligence before spending probes, not a new finding.
- verdict: n/a (confirmatory read, not a hypothesis)

## [s6] Three fresh structural respellings of the final-callback block/outer-guard/SetIntrMask-placement, none of which were previously measured, all regress the sandbox floor from 2/187 — the jalr-delay-slot residual is not reachable by statement/operand reordering in the surrounding region either.

- probe 1: swapped the final block's inner guard `&&` operand order (`if (*p != 0 && D_8009BE80 != 0)` -> `if (D_8009BE80 != 0 && *p != 0)`) on the s4/s5 floor-2 chassis; sandbox _exeque --disable all.
- result 1: score 2 -> 8. Reverted. Banked: memory/grind/_exeque/rejected/final-block-cond-swap-worse.c
- probe 2: swapped the OUTER post-loop guard's `&&` operand order (`if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000))` -> `if (!(*D_8009BF54 & 0x01000000) && D_8009BF78 == D_8009BF7C)`) on the identical chassis; sandbox.
- result 2: score 2 -> 14 (worse than probe 1). Reverted. Banked: memory/grind/_exeque/rejected/outer-guard-cond-swap-worse.c
- probe 3: moved the unconditional `SetIntrMask(D_8009BF84);` call (currently BEFORE the outer post-loop guard `if`) to be duplicated into both arms of that guard instead (once at the top of the `if` block adjacent to the final callback code, once alone in a new `else` arm) — testing whether relocating the call closer to the jalr changes the reorg.c resource-conflict computation for the DIFFERENT (later) call_insn; sandbox.
- result 3: score 2 -> 18 (worst of the three). Reverted. Banked: memory/grind/_exeque/rejected/setintrmask-duplicated-into-arms-worse.c
- (control) probe 4: converted the final block's fall-through return into a mixed-exit form (`return ...;` inlined at the end of the `if` block, duplicated `return ...;` after it) — the SOTN-sanctioned mixed-exit-forms family, own known-neutral construct — as a sanity check that the surrounding region isn't otherwise perturbable; sandbox.
- result 4: score unchanged at 2 (neither better nor worse). Not adopted (adds a construct for zero benefit; simplest-known-form stays candidate.c's plain single return).
- verdict: KILLED (all three, individually)
- kill_scope: instance (each) — the exact respelling named, on the s4/s5 do-while(0)-wrapped floor-2/187 chassis, both FAKE-annotated wraps present, no other construct changed
- measured_on: s4/s5 do-while(0)-wrapped floor-2/187 chassis, one respelling applied at a time, reverted to the unmodified chassis (re-confirmed score 2) between each probe

## Frontier update [s6]

The jalr-delay-slot residual (H6, s2) remains the sole remaining gap. This
session's three fresh structural probes (final-block condition order,
outer-guard condition order, SetIntrMask relocation/duplication) all
regressed the score, adding to s4/s5's already-exhausted permuter search
(~24k iterations) and hand-derived variants (do-while wrap: no effect;
cb-local hoist: worse). Every lever tried in the *local region surrounding*
the residual — statement order, operand order, call placement — either has
no effect or actively worsens the score; none reaches the delay-slot fill
itself. Per the pass-attribution read this session confirmed (`.dbr` dump),
the mechanism is exactly reorg.c's `fill_simple_delay_slots` choosing RTL
insn 311 (the `*p=0` store, address cached in hard reg v1) as the jalr's
delay-slot fill because no resource conflict is detected between the store
and the call — and no C-level restructuring found so far changes that
resource-conflict computation. The two live frontier items from s4/s5
(ruling-request on H6's volatile form; and "try restructuring the
surrounding do-while loop's exit path, or moving SetIntrMask relative to
the guard") both remain the honest next steps — this session executed the
SetIntrMask-relocation half of the second item (worse) and the loop-exit
restructuring half is still genuinely untried (this session's probes were
all downstream of/adjacent to the loop, not the loop's own exit-condition
structure).

## [s6] Respelling the outer if+do-while loop as a plain `while` loop (previously-untried loop-geometry spelling) has NO effect on the jalr-delay-slot residual.

- probe: rewrote `if (cond) { do { body } while (cond); }` as `while (cond) { body }` (removing the duplicated top-of-loop guard, keeping the identical body) on the s4/s5 floor-2/187 chassis; sandbox _exeque --disable all.
- result: score unchanged at 2/187. GCC 2.7.2's loop.c performs the same loop-rotation on a plain `while` as the source already spelled explicitly, so this respelling is codegen-neutral here — confirms the if+do-while spelling already in candidate.c is not itself contributing to (or masking) the jalr-delay-slot residual; the residual is fully local to the final callback block regardless of the enclosing loop's C-level spelling. Reverted to the simpler/already-banked if+do-while form (no reason to prefer either at equal score; kept candidate.c unchanged to avoid an unreviewed diff from the s4/s5-verified chassis).
- verdict: KILLED (instance)
- kill_scope: instance
- measured_on: s4/s5 do-while(0)-wrapped floor-2/187 chassis, outer loop spelling changed only, reverted after measurement

## [s6] Swapping the final callback block's inner guard `&&` operand order (`if (*p != 0 && D_8009BE80 != 0)` -> `if (D_8009BE80 != 0 && *p != 0)`) on the s4/s5 do-while(0)-wrapped floor-2/187 chassis does not close or improve the jalr-delay-slot residual.
- mechanism: reorg.c fill_simple_delay_slots resource-conflict computation for the D_8009BE80 jalr call_insn
- probe: Applied the operand-order swap to src/display.c's final guard only; ran sandbox _exeque --disable all.
- result: Score regressed 2 -> 8. Reverted. Banked memory/grind/_exeque/rejected/final-block-cond-swap-worse.c
- verdict: KILLED
- kill_scope: instance
- measured_on: s4/s5 do-while(0)-wrapped floor-2/187 chassis, both FAKE-annotated wraps present, only the named guard operand order changed

## [s6] Swapping the outer post-loop guard's `&&` operand order (`if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000))` -> `if (!(*D_8009BF54 & 0x01000000) && D_8009BF78 == D_8009BF7C)`) on the identical chassis does not close or improve the residual.
- mechanism: reorg.c fill_simple_delay_slots resource-conflict computation for the D_8009BE80 jalr call_insn
- probe: Applied the operand-order swap to src/display.c's outer post-loop guard only; ran sandbox _exeque --disable all.
- result: Score regressed 2 -> 14 (worse than the inner-guard swap). Reverted. Banked memory/grind/_exeque/rejected/outer-guard-cond-swap-worse.c
- verdict: KILLED
- kill_scope: instance
- measured_on: s4/s5 do-while(0)-wrapped floor-2/187 chassis, both FAKE-annotated wraps present, only the named guard operand order changed

## [s6] Relocating the unconditional `SetIntrMask(D_8009BF84);` call (moving it from before the outer post-loop guard to duplicated inside both if/else arms of that guard, adjacent to the final callback block) does not close or improve the residual.
- mechanism: reorg.c fill_simple_delay_slots resource-conflict computation for the D_8009BE80 jalr call_insn
- probe: Restructured src/display.c to duplicate the SetIntrMask call into both arms of the final guard if/else; ran sandbox _exeque --disable all.
- result: Score regressed 2 -> 18 (worst of the three probes). Reverted. Banked memory/grind/_exeque/rejected/setintrmask-duplicated-into-arms-worse.c
- verdict: KILLED
- kill_scope: instance
- measured_on: s4/s5 do-while(0)-wrapped floor-2/187 chassis, both FAKE-annotated wraps present, only the named SetIntrMask relocation/duplication applied

## [s6] Respelling the outer `if (cond) { do { body } while (cond); }` loop as a plain `while (cond) { body }` loop (removing the C-level duplicated top-of-loop guard) has no effect on the jalr-delay-slot residual.
- mechanism: GCC 2.7.2 loop.c's automatic while-to-do-while rotation makes the two C spellings RTL-equivalent
- probe: Rewrote the outer loop as a plain while loop with the identical body on the s4/s5 chassis; ran sandbox _exeque --disable all.
- result: Score unchanged at 2/187. Reverted to the already-banked if+do-while spelling (equal score, no reason to prefer the diff).
- verdict: KILLED
- kill_scope: instance
- measured_on: s4/s5 do-while(0)-wrapped floor-2/187 chassis, only the outer loop's C-level spelling changed, reverted after measurement

## [s7] SYSTEMATIC SPELLING ENUMERATION (owner ruling 2026-09-08 protocol) of the final-callback block's named-intermediate space fails to find anything better than floor 2/187, and reproduces floor 2 ONLY from spellings structurally identical to the current do-while(0) chassis.
- mechanism: n/a (exhaustive spelling search over the block, not a single-lever hypothesis)
- probe: Marked the final block (`s32 *p = &D_8009BE7C; if (*p != 0 && D_8009BE80 != 0) { *p = 0; ((s32(*)(void))D_8009BE80)(); }`) in fully-named form -- introducing two additional named intermediates (`flag = *p;`, `cb = D_8009BE80;`) alongside the existing pointer local `p`, so the region carries THREE named locals -- between ENUM-BEGIN/END markers (tmp/grind/_exeque/s7/enum_input.c). Ran `tools/spelling_enum.py --no-swaps` (12 spellings: every subset of {p,flag,cb} inlined-vs-kept x every valid def-before-use decl ordering) and again with the commutative-swap axis enabled (19 spellings). Swept all 19 through `tools/sweep_variants.py --func _exeque --file display` against the REAL engine sandbox (see note below on a sweep-harness pitfall this session found and fixed first).
- IMPORTANT PROCESS NOTE for future sessions: the first sweep attempt (before this session applied candidate.c's forward-declaration fixups to src/display.c) gave WRONG, misleadingly-high scores (best found was 11, matching baseline itself scored 11 instead of 2) because `sweep_variants.py` splices only the FUNCTION SPAN and leaves the surrounding file's forward declarations/externs untouched -- and src/display.c on disk (INCLUDE_ASM state, per asm-until-matched) still carried the STALE `void _exeque();` forward decl and was missing `extern s32 D_8009BF84;` entirely (both fixups only exist inside memory/grind/_exeque/candidate.c's header comment, never committed, since candidates never land on main until COMPLETED-C). Applying candidate.c's full body (with the two decl fixups) to src/display.c first made `sweep_variants.py --variants memory/grind/_exeque/candidate.c` correctly reproduce the ledger's floor 2/187, and ONLY THEN did the enum sweep produce trustworthy numbers. **Any future sweep_variants.py run on this function MUST first apply the candidate's forward-declaration fixups to src/display.c** (or the sweep's "baseline"/scores are meaningless -- verify baseline reproduces the ledger floor before trusting any variant's score).
- result: Best score across all 19 spellings is 2/187 (v12.c and v16.c -- both are the "flag and cb fully inlined back to their original `*p`/`D_8009BE80` spelling" forms, i.e. structurally IDENTICAL to the already-banked candidate.c chassis, just with cosmetic extra parens from the inliner). Every spelling that introduces a genuinely NEW named local for `flag` (the guard read) or `cb` (the callback pointer) -- all 17 remaining spellings -- scores 5-27, strictly worse than 2. This confirms s5's H "cb-local-hoist is worse" finding generalizes: naming EITHER intermediate in this block (not just cb) perturbs the reorg.c delay-slot fill / RA in a way that only ever regresses, never improves, across every declaration order and commutative-swap variant tried.
- verdict: KILLED
- kill_scope: instance (this exact three-local {p,flag,cb} enumeration space, all 19 orderings/inlinings/swaps, on the s4/s5/s6 do-while(0)-wrapped floor-2/187 chassis with both FAKE-annotated wraps present, no volatile/cheat constructs in any variant)
- measured_on: s4/s5/s6 chassis (memory/grind/_exeque/candidate.c, unchanged), final-callback block only, all 19 enumerated spellings swept via tools/sweep_variants.py against the real `sandbox _exeque --disable all` engine after the forward-decl fixups were applied to src/display.c for the duration of the sweep (reverted to INCLUDE_ASM before session end, no cheats/draft C left on main)
- artifacts: tmp/grind/_exeque/s7/enum_input.c, tmp/grind/_exeque/s7/enum2/ (12 no-swap variants), tmp/grind/_exeque/s7/enum2swaps/ (19 variants incl. commutative swaps), tmp/sw2.json (full sweep JSON)

This narrows (does not eliminate) the frontier: the jalr-delay-slot residual is NOT closable by any RESPELLING of the final block's own local-variable declarations/ordering -- the remaining pure-C search space (per s6's frontier item 2: forcing the store's address into a specific hard register via pressure from an unrelated intervening read/call, or restructuring how D_8009BE7C's address is computed, e.g. a struct/field-access spelling instead of a raw pointer local) is a STRUCTURAL lever, not a spelling one, and remains untried. The ruling-request on H6's volatile form (frontier item 1, unchanged) is still the highest-value single remaining lever if a fourth legitimate-volatile-interrupt-touched use-site shape is ever ruled in.

## [s7] Re-measuring the s4/s5/s6 do-while(0)-wrapped chassis (memory/grind/_exeque/candidate.c, unchanged) this session reproduces sandbox score 2/187 exactly, confirming the chassis is still current and the ledger's banked floor has not drifted.
- mechanism: n/a (chassis re-confirmation, not a new lever)
- probe: Applied candidate.c verbatim to src/display.c (function body + the two forward-declaration fixups: `extern s32 _exeque(void);` replacing the stale `void _exeque();`, and the added `extern s32 D_8009BF84;`), then ran `sandbox _exeque --disable all`.
- result: score 2, target_insns 187, build_insns 186 — matches every prior session's banked floor exactly. Reverted src/display.c to INCLUDE_ASM afterward (git checkout), leaving no cheats/draft C on main.
- verdict: CONFIRMED

## [s7] On the s4/s5/s6 do-while(0)-wrapped floor-2/187 chassis (both FAKE-annotated wraps present, no volatile/cheat constructs), the three-local {p,flag,cb} named-intermediate enumeration space for the final-callback block (19 spellings: every inline/keep subset of {flag,cb} beyond the existing `p`, crossed with every valid def-before-use declaration order and the commutative-swap axis) scores 2/187 ONLY for the two spellings that collapse back to the exact banked chassis (flag and cb both fully inlined); the other 17 spellings that introduce flag or cb as a real, kept local score 5-27, strictly worse.
- mechanism: n/a (exhaustive spelling search via tools/spelling_enum.py + tools/sweep_variants.py over one named block, not a single-lever hypothesis about a GCC pass)
- probe: This is s7's already-banked finding (memory/grind/_exeque/hypotheses.md [s7], artifacts tmp/grind/_exeque/s7/enum_input.c + enum2/ + enum2swaps/ + tmp/sw2.json), carried forward unchanged this session. Restating it here ONLY to correct the wording the driver rejected: the prior outcome JSON's STATEMENT field used class wording ('every spelling') while kill_scope was 'instance' — this restatement names the exact enumeration space (19 spellings, three named locals, one block) and the exact chassis instead.
- result: ?
- verdict: KILLED
- kill_scope: instance
- measured_on: s4/s5/s6 do-while(0)-wrapped floor-2/187 chassis (memory/grind/_exeque/candidate.c), final-callback block only, all 19 enumerated {p,flag,cb} spellings/orderings/swaps, swept via tools/sweep_variants.py against the real sandbox _exeque --disable all engine, forward-decl fixups applied to src/display.c for the duration of the sweep and reverted before session end

## [s8] SYNTHESIS re-audit: applying memory/grind/_exeque/candidate.c verbatim to src/display.c this session (chassis check at dispatch reported "measurement unavailable" because src was in its committed INCLUDE_ASM state) re-confirms sandbox score 2/187 exactly, matching every prior session s4-s7. No drift.
- mechanism: n/a (chassis re-confirmation)
- probe: Applied candidate.c's function body + both forward-declaration fixups (`extern s32 _exeque(void);` at both call-site forward decls, `extern s32 D_8009BF84;` added near the D_8009BE7C/D_8009BE80 externs) to src/display.c; ran `& tools/wteng.ps1 main sandbox _exeque --disable all`.
- result: score 2, target_insns 187, build_insns 186. Identical to s4-s7.
- verdict: CONFIRMED

## [s8] KILL RE-AUDIT (mandatory per the driver's flat-floor trigger): ran `tools/fake_ablate.py --func _exeque --file display --candidate memory/grind/_exeque/candidate.c` on the current chassis to verify neither `/* FAKE */` do-while(0) wrap is an inert carrier sitting on a pseudo a real lever needs (the func_8002EA24 s8 failure mode named in the driver's KILL RE-AUDIT REQUIRED instructions).
- mechanism: reorg.c list-scheduler / rank_for_schedule ordering effect of each wrap, per their existing FAKE annotations
- probe: `bash tools/wsl.sh 'source .venv/bin/activate && python3 tools/fake_ablate.py --func _exeque --file display --candidate memory/grind/_exeque/candidate.c --json'` — sweeps all 2^2 subsets of the two FAKE wraps (keep-all, drop outer, drop nested, drop both) against the real sandbox.
- result: keep-all (both wraps present) = 2/187 (186 build insns); drop nested only = 7/187; drop outer only = 7/187 (185 build insns); drop both = 12/187 (185 build insns). Both wraps are independently load-bearing (each alone recovers only floor 7, not 2) and their combination is required to reach floor 2 — this is the SAME shape as s4's original single-vs-nested A/B measurement, now re-verified via the dedicated ablation tool rather than by hand. No inert-carrier pattern found: this is NOT a func_8002EA24-style false kill.
- verdict: CONFIRMED
- artifacts: tmp/grind/_exeque/s8/ablate/ (fake_ablate.py's variant .c files + tmp/sw2.json-equivalent scoring, written under tmp/grind/_exeque/ablate/ by the tool itself)

## [s8] SYNTHESIS: merged frontier assessment. Across s4 (permuter, 2 campaigns), s5 (permuter, re-confirm + 2nd campaign), s6 (structural, 4 fresh respellings), s7 (enumerate, 19-way systematic sweep), and s8 (synthesis, fake-ablation re-audit), the floor-2/187 chassis has been re-confirmed FIVE times with zero improvement from any non-volatile pure-C lever. The residual is a single, well-understood mechanism (s2/s6 dump-proven: reorg.c's fill_simple_delay_slots fills the `D_8009BE80` jalr's delay slot with the `*p = 0;` (D_8009BE7C-clear) store, where target keeps that store BEFORE the jalr with an explicit unfilled nop). Two genuinely distinct axes remain untried at the STRUCTURAL level (not spelling-level, which s6+s7 together closed): (1) the legitimate-volatile-interrupt-touched ruling-request (H6, frontier item 1, unchanged since s2) and (2) forcing the store address's hard-register choice into one that reorg.c's mark_set_resources(CALL_INSN, include_delayed_effects=1) (reorg.c:573-604) marks as SET by the call — i.e. a call_used_reg ($v0/$v1/$a0-$a3/$t0-$t9) that the call itself also needs live across the jalr (the callee-address register), which would exclude the store from the delay slot on a genuine resource conflict rather than a scheduling preference. s7's cb-naming enumeration already falsified the most obvious spelling of axis (2) (naming the callback pointer as a fresh local, in all 19 combinations, only ever regressed) — the remaining un-falsified form of axis (2) is restructuring D_8009BE7C's address computation itself (e.g. a field-access/struct spelling instead of `&D_8009BE7C`) rather than the callback pointer, which s7 did NOT enumerate (s7 enumerated {p, flag, cb} where p already IS the D_8009BE7C address local — it varied whether p/flag/cb are named, not how p's address is computed).
- mechanism: policy question (axis 1) / reorg.c mark_set_resources CALL_INSN resource-conflict computation (axis 2)
- probe: n/a (synthesis — no new code measured beyond the s8 re-confirmation and fake-ablation entries above)
- result: Frontier narrowed to exactly these two axes; both remain open (not killed) and are carried to the next session per the ladder.
- verdict: n/a (synthesis entry, not a hypothesis test)

## [s8] Applying memory/grind/_exeque/candidate.c verbatim to src/display.c reproduces sandbox _exeque --disable all == 2/187 (186 build insns vs 187 target), identical to every prior session s4-s7.
- mechanism: n/a (chassis re-confirmation)
- probe: Applied candidate.c's body + both forward-decl fixups (extern s32 _exeque(void); at both call sites, extern s32 D_8009BF84;) to src/display.c; ran tools/wteng.ps1 main sandbox _exeque --disable all.
- result: score 2, target_insns 187, build_insns 186. No drift from s4-s7.
- verdict: CONFIRMED

## [s8] On the current floor-2/187 chassis, neither of the two /* FAKE */ do-while(0) wraps is an inert carrier occupying a pseudo that a live lever needs (the func_8002EA24 s8 false-kill pattern) — both are independently load-bearing.
- mechanism: reorg.c list-scheduler / rank_for_schedule ordering effect of each wrap, per their existing FAKE annotations
- probe: tools/fake_ablate.py --func _exeque --file display --candidate memory/grind/_exeque/candidate.c --json, sweeping all 2^2 subsets of the two FAKE wraps against the real sandbox.
- result: keep-all (both wraps)=2/187 (186 insns); drop-nested-only=7/187; drop-outer-only=7/187 (185 insns); drop-both=12/187 (185 insns). Confirms s4's original single-vs-nested A/B measurement via the dedicated ablation tool.
- verdict: KILLED
- kill_scope: instance
- measured_on: s4/s5/s6/s7/s8 do-while(0)-wrapped floor-2/187 chassis (memory/grind/_exeque/candidate.c), both FAKE-annotated wraps present at baseline, ablated singly and jointly

## [s9] SOLVER — classify (tools/ra_solver/inverse_compose.py) confirms the floor-2/187 residual is a pure SCHED/nop-only multiset difference, no RA component.
- mechanism: n/a (triage tool, no GCC-pass claim)
- probe: `python3 tools/ra_solver/inverse_compose.py classify display _exeque --target-object build/src/display.o --ours-object tmp/sandbox/_exeque/display.o` (object-level path, since display.c is zero-rule and the text-stream classifier is unusable for it per the tool's own guard).
- result: "_exeque (display): honest 186 insns, target 187 insns ... FIRST DIVERGENCE: SCHED ... the ONLY multiset difference is 1 nop(s) (target has more)." Register-blanked instruction multisets are otherwise identical, so there is no RA-level (ra_solver/inverse.py) question for this function at all — everything not already CONFIRMED/KILLED at s1-s8 lives downstream of allocation.
- verdict: CONFIRMED

## [s9] SOLVER — sched_solver's own model proves this SCHED classification is a false positive for the sched1/sched2 layer: perturb.py finds ZERO blocks in `_exeque` (either pass) where target's required pre-reorg pick order differs from our own.
- mechanism: sched.c `schedule_block`'s backward pick order, replicated exactly by tools/sched_solver (validated 6978/6978 blocks project-wide); `goal_for_block` composes target's post-reorg order back through our own known reorg permutation into pre-reorg space per block (README "reorg.c cancels rather than being modelled").
- probe: built the display.c sched model (`tools/sched_solver/extract.py display`, parity=True) and ran `tools/sched_solver/perturb.py tmp/sched_solver_work/display.sched.json --func _exeque --pass {1,2} --goal-from-target display --target-object build/src/display.o --ours-object tmp/sandbox/_exeque/display.o --atoms luid,luid_move --depth 2`. (Note: `--target asm/funcs/_exeque.s` — the text-file escape — is NOT usable here: `goalmap.asm_body`'s parser looks for a bare `func:` label and skips every `/* offset addr bytes */`-prefixed disassembly line, so it raises `KeyError: _exeque not found`; the object-level `--target-object`/`--ours-object` escape is the only working path for this INCLUDE_ASM-originated, not-yet-matched function, same class of limitation `docs/grind/inverse-compose-2026-08-06.md` documents and resolves for `classify`/`goal_from_tgt.py`.) Full logs: tmp/grind/_exeque/s9/perturb_sched2_pass2.log, tmp/grind/_exeque/s9/perturb_sched1_pass1.log.
- result: both passes print only the header alignment stats (`align honobj->tgtobj: |A|=186 |B|=187 {'equal': 185, 'replace': 1, 'delete': 0, 'insert': 1, 'moved': 0}`, matching the sandbox score exactly) and the `filled delay slots` line (6 reorg fills in this function, including cc1-UID pair `316/311` — the jalr/store pair this ledger has tracked since s2/s6) — then NOTHING further. Per perturb.py's own control flow (`if goal == ours: continue`), silence means goal_for_block found target's required order IDENTICAL to our own pick order in every single block of both sched1 and sched2. There is no block where an atom search (luid/luid_move, depth 1 or 2) even has a non-trivial goal to search for — the search space is empty, not merely exhausted.
- verdict: KILLED
- kill_scope: instance
- measured_on: s4-s8 do-while(0)-wrapped floor-2/187 chassis (memory/grind/_exeque/candidate.c unchanged), sched_solver's full sched1+sched2 model of display.c (parity=True vs build/cc1), goal derived from the real oracle object (build/src/display.o) via the object-level escape

## [s9] SYNTHESIS: the s9 solver findings upgrade s6's dump-based diagnosis from "read one BB2_R... dump line" to "searched exhaustively, formally, at the sched1+sched2 model layer, with zero candidate blocks". The floor-2/187 residual is now established at three independent levels of rigor (s2 first noticed it, s6 traced the exact RTL insn + pass via the instrumented cc1 dump, s9 proved via the validated sched_solver model that no sched1/sched2 input perturbation is even a candidate) to be ENTIRELY a reorg.c `fill_simple_delay_slots` decision — a pass sched_solver's README explicitly documents as out of its model ("Anything after sched2 ... reorg.c's delay-slot filling ... are downstream. The mapper cancels reorg rather than modelling it"). Both `ra_solver` (no RA component — classify confirmed) and `sched_solver` (no SCHED1/2 component — perturb confirmed empty) are exhausted for this residual; neither tool's atom vocabulary can reach it, by their own documented scope. This does NOT mean the residual is C-unreachable (reorg.c's fill decision is itself driven by C-controlled register liveness/resource-conflict facts, per s8's frontier axis 2) — only that the SOLVER family of tools has nothing further to offer here. The single remaining un-falsified lever is s8's frontier axis 2 (restructure D_8009BE7C's ADDRESS computation, not the callback pointer, to try to force its pseudo into a call_used_reg the D_8009BE80 jalr needs live across the call) — this requires reading the exact hard register the jalr's target materializes into from the .sched/.combine dump and is a `structural`-modality probe, not a solver one.
- mechanism: n/a (synthesis)
- probe: n/a
- result: frontier narrowed from 2 items to the single structural axis 2 (axis 1, the volatile ruling-request, is unchanged and still open as a policy question).
- verdict: n/a (synthesis entry)

## [s9] tools/ra_solver/inverse_compose.py classify (object-level path: --target-object build/src/display.o --ours-object tmp/sandbox/_exeque/display.o) confirms the floor-2/187 residual is a pure SCHED/nop-only instruction-multiset difference (186 vs 187 insns, one extra nop in target) with no RA-level component anywhere in the function.
- mechanism: n/a (triage classifier, no GCC-pass claim of its own)
- probe: python3 tools/ra_solver/inverse_compose.py classify display _exeque --target-object build/src/display.o --ours-object tmp/sandbox/_exeque/display.o
- result: "_exeque (display): honest 186 insns, target 187 insns ... FIRST DIVERGENCE: SCHED ... the ONLY multiset difference is 1 nop(s) (target has more)." Register-blanked multisets otherwise identical.
- verdict: CONFIRMED

## [s9] On the current floor-2/187 do-while(0)-wrapped chassis (memory/grind/_exeque/candidate.c, both FAKE wraps present), sched_solver's validated sched1+sched2 model of _exeque finds ZERO blocks, in either pass, where target's required pre-reorg pick order differs from our own — the sched-order atom search space (luid, luid_move, depth 1-2) is empty, not merely exhausted, for this function.
- mechanism: sched.c schedule_block's backward pick order, replicated exactly by tools/sched_solver (validated 6978/6978 blocks project-wide, display.c itself 690/690 order- and clock-exact); goalmap.goal_for_block composes target's post-reorg order back through our own known reorg permutation into pre-reorg space per block.
- probe: tools/sched_solver/extract.py display (parity=True); tools/sched_solver/perturb.py tmp/sched_solver_work/display.sched.json --func _exeque --pass {1,2} --goal-from-target display --target-object build/src/display.o --ours-object tmp/sandbox/_exeque/display.o --atoms luid,luid_move --depth 2 --root . (both passes). Logs: tmp/grind/_exeque/s9/perturb_sched2_pass2.log, tmp/grind/_exeque/s9/perturb_sched1_pass1.log.
- result: Both passes print only the header alignment stats (align honobj->tgtobj: |A|=186 |B|=187 {'equal': 185, 'replace': 1, 'delete': 0, 'insert': 1, 'moved': 0}, matching the sandbox score exactly) plus a 'filled delay slots' line naming 6 reorg fills including the tracked jalr/store pair (cc1-UID 316/311) — then nothing further. Per perturb.py's control flow (`if goal == ours: continue`), silence over every block in both passes means goal_for_block found target's order identical to ours everywhere; no block ever reaches the atom-search code path at all.
- verdict: KILLED
- kill_scope: instance
- measured_on: s4-s8 do-while(0)-wrapped floor-2/187 chassis (memory/grind/_exeque/candidate.c unchanged this session), sched_solver's full sched1+sched2 model of display.c (parity=True vs build/cc1), goal derived from the real oracle object build/src/display.o via the object-level --target-object/--ours-object escape (the asm/funcs/_exeque.s text-file escape is unusable for this function: goalmap.asm_body's parser looks for a bare `func:` label line and skips every `/* offset addr bytes */`-prefixed disassembly line, raising KeyError)
