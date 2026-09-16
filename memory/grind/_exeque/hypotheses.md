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
