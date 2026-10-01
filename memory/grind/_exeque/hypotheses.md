# Hypothesis ledger — _exeque

> COMPACTED 2026-10-01 by `grindlib.py compact-ledger` (89,127 bytes, cap 64K). Older entries are one-line index items;
> full text: `git show 11cb352eb:memory/grind/_exeque/hypotheses.md` (history before 2026-10-01: tag pre-slim-2026-10-01).
> Look one up only when its index line bears on your probe. Append below as usual.

## Current state (at compaction)
floor 2 at s12; 12 sessions; live frontier (state.json is authoritative):
  - Axis 3 sub-probe: inserting a genuinely separate, real (non-dead) statement between the D_8009BE7C=0 store and the D_8009BE80() call may change what reorg.c's fill_simple_delay_slots backward scan sees as the delay-slot…
  - An owner class grant extending legitimate-volatile-interrupt-touched's three catalogued use-site shapes to cover 'guard-clear-and-invoke' would directly close gate (b) and let the already-known volatile-D_8009BE7C-guard…

## Class-level kills (binding until the cited predicate changes)

## [s10] FORENSICS — formal proof (reorg.c source, not measurement) that frontier axis 2 (s8: force D_8009BE7C's address into a call-clobbered/call-target register to create a reorg.c resource conflict) is dead for EVERY register choice.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/reorg.c:690
- result: see evidence.md [s10] for the full derivation. `set`={$ra only, since v0's SET_DEST-of-a-CALL is excluded by reorg.c:625-627 when include_delayed_effects=0} and `needed`={v0 only, referenced as call target}; the store's `insn_sets` is always {memory}, never a register bit, for ANY register assignment of its address pointer. No C-level restructuring of D_8009BE7C's address expression can exclude t…

## [s10] Frontier axis 2 (s8): restructuring D_8009BE7C's address computation to force its store's address pseudo into a call_used_reg the D_8009BE80 jalr needs live can create a reorg.c resource conflict that excludes the store from the jalr's delay-slot fill, for some register assignment.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/reorg.c:690
- result: For a plain memory-store insn (D_8009BE7C's clear), mark_set_resources's MEM case (reorg.c:663-671) always visits the address subexpression with in_dest=0, UNCONDITIONALLY, regardless of the store's own in_dest -- so the REG case (reorg.c:690-693, which only sets a register bit `if (in_dest)`) never contributes ANY register to insn_sets for the store's address operand, no matter which hard regist…

## Confirmed levers

## [s1] H1 — CONFIRMED: struct-based `_que[idx].field` (already-merged GpuQueueItem object model, include/gpu.h:81-88) is the right object model for the whole function, matching the DATA MODEL brief's SPLIT-AGGREGATE signal on `_que_plus_0x4`/`_que_plus_0x8`. Mechanism: no GCC pass — this is a declaration-correctness question…
- verdict: CONFIRMED
- result: canonical distance 187 -> 24 on first compile; instruction diff

## [s1] H2 — CONFIRMED: `D_8009BF84` was undeclared in the TU; adding `extern s32 D_8009BF84;` is required (bug fix, not a technique). Mechanism: none (missing declaration, not a codegen lever) — GCC 2.7.2 silently implicit-declared the undeclared identifier and produced wrong codegen for the `SetIntrMask(D_8009BF84)` call (l…
- verdict: CONFIRMED
- result: score 24 -> 15, build_insns 183 -> 186. CONFIRMED.

## [s1] The struct-based object model (_que[idx].func/.arg/.count via include/gpu.h's already-declared GpuQueueItem, per the DATA MODEL SPLIT-AGGREGATE signal on _que_plus_0x4/_que_plus_0x8) is the correct object model for _exeque.
- verdict: CONFIRMED
- result: canonical distance 187 -> 24 immediately; instruction diff (tmp/grind/_exeque/s1/diagdiff.py) shows the call-site struct-member address computation matches target's per-field recompute pattern exactly in that region.

## [s1] D_8009BF84 (the saved interrupt mask _exeque writes at entry and restores at exit) had no extern declaration anywhere in src/display.c before this session, and adding one is required for correct codegen.
- verdict: CONFIRMED
- result: score 24 -> 15, build_insns 183 -> 186 (target is 187). Confirmed via objdump -dr showing the SetIntrMask call's delay slot changed from `move a0,zero` (wrong) to loading the actual global.

## [s2] H5a — CONFIRMED (partial, non-closing): reusing the existing dead-after-use `mask` local (SOTN-sanctioned "variable reuse for codegen control" family) to hold both `.arg` and `.count` field values sequentially, instead of relying on GCC's own temp allocation, changes the triple-store block's register allocation from a…
- verdict: CONFIRMED
- result: score unchanged (15->15 in isolation), but objdump shows both field loads now land in `a0` (matching target's practice of reusing one register across both fields, though target's register is `v0` not `a0`). Kept in candidate.c since it's a genuine SOTN-sanctioned, non-regressing structural improvement.

## [s2] H5b — CONFIRMED: pre-computing a pointer to `D_8009BE7C` (`s32 *p = &D_8009BE7C;`) and dereferencing it for BOTH the guard read (`*p != 0`) and the clear store (`*p = 0;`) in the function's final "clear the pending flag and invoke the queued callback" block closes that ENTIRE block to a byte-exact match (only masked b…
- verdict: CONFIRMED
- result: score 15 -> 12 (build_insns 186 -> 185, target 187); diagdiff shows the ENTIRE final-callback block (`bne`/`bnez`/`beqz` chain through the `sw zero,0(v1)` clear) matching target with only masked branch-offset differences remaining. This is genuine, ordinary, submittable pure C (no FAKE, no volatile) — banked in candidate.c.

## [s2] Reusing the existing dead-after-use `mask` local (SOTN-sanctioned variable-reuse family) to hold both .arg and .count field values sequentially converges the triple-store block's register allocation toward target's single-register-reuse pattern.
- verdict: CONFIRMED
- result: Score unchanged in isolation (still 15 at that point), but objdump shows both field loads now land in a single register (a0) matching target's practice of register reuse across the two fields (target itself reuses v0, not a0). Non-regressing, kept in candidate.c.

## [s2] Pre-computing a pointer to D_8009BE7C and dereferencing it for both the guard read and the clear store in the final callback block closes that block to a byte-exact match.
- verdict: CONFIRMED
- result: Score dropped 15 -> 12 (build_insns 186 -> 185, target 187). The entire final-callback instruction block now matches target with only masked branch-offset differences remaining.

## [s3] H8 — CONFIRMED (diagnostic, not a fix): the instrumented cc1's `BB2_RANK_DEBUG` hook (tools/gcc-2.7.2/cc1, NOT tools/gcc-2.7.2/build/cc1 — see [[instrumented-cc1-location]]) proves the D_8009BF6C store (insn 189) ties with insn 198 (part of the .count field's address recompute chain) in BOTH `INSN_PRIORITY` (8) and de…
- verdict: CONFIRMED
- result: `RANKDBG last=204 y=198 cls=3 x=189 cls2=3 val=0` — confirms the

## [s3] The instrumented cc1's BB2_RANK_DEBUG hook (tools/gcc-2.7.2/cc1) shows the D_8009BF6C store (insn 189) ties with insn 198 (part of the .count field's address recompute) in both INSN_PRIORITY (8) and dependency class (3 = independent of last-scheduled insn 204) in rank_for_schedule, forcing the decision to the final IN…
- verdict: CONFIRMED
- result: RANKDBG last=204 y=198 cls=3 x=189 cls2=3 val=0 -- confirms the exact class+priority tie. This is diagnostic evidence explaining why five independently measured spellings (this session's 3 + s2's H4a/H4b) all produced byte-identical output; it does not by itself close the residual.

## [s4] H9 — CONFIRMED: wrapping the post-call triple-store block (D_8009BF68[0]/D_8009BF6C/D_8009BF70) in a single `do { ... } while (0);` breaks the rank_for_schedule LUID tie that H7/H8 (s3) proved was invariant across five plain statement/variable respellings, dropping the sandbox floor from 12 to 7.
- verdict: CONFIRMED
- result: sandbox score 12 -> 7 (build_insns 185 -> 186, target 187). Confirmed via direct objdump -dr diff of tmp/sandbox/_exeque/display.o against a freshly rebuilt target.o that the ENTIRE triple-store region now matches target byte-for-byte; the sole remaining residual is a different, previously-known site (H6's final-callback jalr delay slot).

## [s4] H10 — CONFIRMED: nesting a SECOND do-while(0) inside the first, wrapped around only the first two stores (D_8009BF68[0]/D_8009BF6C, leaving D_8009BF70's store and the D_8009BF7C increment outside both wraps), drops the floor further from 7 to 2. Single-level wrap (H9) is measurably insufficient on its own -- satisfies…
- verdict: CONFIRMED
- result: sandbox score 7 -> 2 (build_insns 186, unchanged -- pure reschedule, no insn count change). Direct A/B on the identical surrounding chassis: single-level wrap = floor 7, nested wrap = floor 2 -- this IS the single-level-insufficient justification the rule requires for a nested wrap, not an assertion.

## [s4] Wrapping the post-call triple-store block (D_8009BF68[0]/D_8009BF6C/D_8009BF70) in a single do { ... } while (0); breaks the rank_for_schedule INSN_LUID tie that s3's H7/H8 proved was invariant across five plain statement/variable respellings.
- verdict: CONFIRMED
- result: sandbox score 12 -> 7 (build_insns 185 -> 186, target 187)

## [s4] Nesting a second do-while(0) inside the first, wrapped around only the first two stores (D_8009BF68[0]/D_8009BF6C), drops the floor further from 7 to 2; the single-level wrap alone is measurably insufficient, satisfying do-while-zero-exception's nested-wrap prerequisite.
- verdict: CONFIRMED
- result: sandbox score 7 -> 2 (build_insns 186, unchanged insn count -- pure reschedule); direct A/B on the identical chassis: single-level = floor 7, nested = floor 2

## [s7] Re-measuring the s4/s5/s6 do-while(0)-wrapped chassis (memory/grind/_exeque/candidate.c, unchanged) this session reproduces sandbox score 2/187 exactly, confirming the chassis is still current and the ledger's banked floor has not drifted.
- verdict: CONFIRMED
- result: score 2, target_insns 187, build_insns 186 — matches every prior session's banked floor exactly. Reverted src/display.c to INCLUDE_ASM afterward (git checkout), leaving no cheats/draft C on main.

## [s8] SYNTHESIS re-audit: applying memory/grind/_exeque/candidate.c verbatim to src/display.c this session (chassis check at dispatch reported "measurement unavailable" because src was in its committed INCLUDE_ASM state) re-confirms sandbox score 2/187 exactly, matching every prior session s4-s7. No drift.
- verdict: CONFIRMED
- result: score 2, target_insns 187, build_insns 186. Identical to s4-s7.

## [s8] KILL RE-AUDIT (mandatory per the driver's flat-floor trigger): ran `tools/fake_ablate.py --func _exeque --file display --candidate memory/grind/_exeque/candidate.c` on the current chassis to verify neither `/* FAKE */` do-while(0) wrap is an inert carrier sitting on a pseudo a real lever needs (the func_8002EA24 s8 fa…
- verdict: CONFIRMED
- result: keep-all (both wraps present) = 2/187 (186 build insns); drop nested only = 7/187; drop outer only = 7/187 (185 build insns); drop both = 12/187 (185 build insns). Both wraps are independently load-bearing (each alone recovers only floor 7, not 2) and their combination is required to reach floor 2 — this is the SAME shape as s4's original single-vs-nested A/B measurement, now re-verified via the…

## [s8] SYNTHESIS: merged frontier assessment. Across s4 (permuter, 2 campaigns), s5 (permuter, re-confirm + 2nd campaign), s6 (structural, 4 fresh respellings), s7 (enumerate, 19-way systematic sweep), and s8 (synthesis, fake-ablation re-audit), the floor-2/187 chassis has been re-confirmed FIVE times with zero improvement f…
- verdict: CONFIRMED
- result: Frontier narrowed to exactly these two axes; both remain open (not killed) and are carried to the next session per the ladder.

## [s8] Applying memory/grind/_exeque/candidate.c verbatim to src/display.c reproduces sandbox _exeque --disable all == 2/187 (186 build insns vs 187 target), identical to every prior session s4-s7.
- verdict: CONFIRMED
- result: score 2, target_insns 187, build_insns 186. No drift from s4-s7.

## [s9] SOLVER — classify (tools/ra_solver/inverse_compose.py) confirms the floor-2/187 residual is a pure SCHED/nop-only multiset difference, no RA component.
- verdict: CONFIRMED
- result: "_exeque (display): honest 186 insns, target 187 insns ... FIRST DIVERGENCE: SCHED ... the ONLY multiset difference is 1 nop(s) (target has more)." Register-blanked instruction multisets are otherwise identical, so there is no RA-level (ra_solver/inverse.py) question for this function at all — everything not already CONFIRMED/KILLED at s1-s8 lives downstream of allocation.

## [s9] SYNTHESIS: the s9 solver findings upgrade s6's dump-based diagnosis from "read one BB2_R... dump line" to "searched exhaustively, formally, at the sched1+sched2 model layer, with zero candidate blocks". The floor-2/187 residual is now established at three independent levels of rigor (s2 first noticed it, s6 traced the…
- verdict: CONFIRMED
- result: frontier narrowed from 2 items to the single structural axis 2 (axis 1, the volatile ruling-request, is unchanged and still open as a policy question).

## [s9] tools/ra_solver/inverse_compose.py classify (object-level path: --target-object build/src/display.o --ours-object tmp/sandbox/_exeque/display.o) confirms the floor-2/187 residual is a pure SCHED/nop-only instruction-multiset difference (186 vs 187 insns, one extra nop in target) with no RA-level component anywhere in…
- verdict: CONFIRMED
- result: "_exeque (display): honest 186 insns, target 187 insns ... FIRST DIVERGENCE: SCHED ... the ONLY multiset difference is 1 nop(s) (target has more)." Register-blanked multisets otherwise identical.

## [s10] src/display.c is at its committed baseline (INCLUDE_ASM("asm/funcs", _exeque);, no C body) at both the start and end of this session, and memory/grind/_exeque/candidate.c (the s4-s9 do-while(0)-wrapped body, banked at floor 2/187 by s9's chassis re-confirmation) is unchanged from the prior discarded s10 attempt.
- verdict: CONFIRMED
- result: sandbox on the committed baseline scores 187/187 (as expected for INCLUDE_ASM with no C body); candidate.c's chassis is unchanged from the prior s10 attempt, so the banked floor of 2/187 (last measured fresh at s9 and re-derived by source-proof this session) stands without needing a redundant sandbox re-run of an unmodified candidate body.

## Instance kills and other entries (index; chassis-relative, re-testable)

## [s1] H3 — KILLED (instance): marking `D_8009BF6C`/`D_8009BF70` volatile does NOT fix the remaining floor-15 residual, and would contradict banked project evidence. Mechanism: n/a — not measured this session (see rationale).…
- verdict: KILLED (instance)

## [s1] Marking D_8009BF6C/D_8009BF70 volatile (to force the target's strict load-then-immediate-store ordering in the post-call triple-store block) is NOT the fix for the remaining floor-15 residual on this chassis.
- verdict: KILLED (instance) — Not independently re-measured with volatile added on this candidate; the existing cross-function grant evidence for the same two symbols already rules the fix…

## [s2] H4a — KILLED (instance): swapping the source order of the `D_8009BF6C = arg;` / `D_8009BF70 = count;` statements does not change the scheduler's register/timing choice for the triple-store residual.
- verdict: KILLED (instance) — score unchanged at 15; objdump diff identical in shape (still `lw a1,8(at)` / `lw a0,4(at)` two-register split, both stores deferred to just before the loop-co…

## [s2] H4b — KILLED (instance): moving the `D_8009BF7C = (D_8009BF7C+1)&0x3F;` increment statement to BETWEEN the `mask=count` load and the `D_8009BF70=mask` store (instead of after both stores) does not change the scheduler's…
- verdict: KILLED (instance) — score unchanged at 15; objdump diff byte-identical to the pre-move `mask`-reuse form (same register `a0` reused for both fields, same store-deferral shape).

## [s2] H6 — KILLED (instance, NOT submittable): making the `D_8009BE7C` pointer `volatile` (`volatile s32 *p = &D_8009BE7C;`) closes ONE more instruction (score 12 -> 10) by preventing cc1's delay-slot filler from moving the `…
- verdict: KILLED (instance) — score 12 -> 10 (build_insns 186, target 187); objdump shows the store now sits BEFORE `jalr v0` with the delay slot as an explicit `nop`, byte-identical to tar…

## [s2] Swapping the source order of the D_8009BF6C=arg / D_8009BF70=count statements changes the scheduler's register/timing choice for the triple-store residual.
- verdict: KILLED (instance) — Score unchanged at 15; objdump diff identical in shape, only which register (a0 vs a1) loaded which offset changed.

## [s2] Moving the D_8009BF7C increment statement to between the count load and its store (instead of after both field stores) changes the scheduler's output for the triple-store residual.
- verdict: KILLED (instance) — Score unchanged at 15; objdump diff byte-identical to the pre-move mask-reuse form.

## [s2] Marking the D_8009BE7C pointer volatile closes one more instruction by preventing the jalr delay-slot filler from moving the clear-store into the call's delay slot, but this construct does not qualify under the current…
- verdict: KILLED (instance) — Score 12 -> 10 (build_insns 186, target 187); the store now sits before jalr with an explicit nop in the delay slot, byte-identical to target in that region. B…

## [s3] H7 — KILLED (instance): three independently-spelled variants of the post-call triple-store block (s2's `mask`-reuse; two freshly-named locals `arg_val`/`count_val` both loaded before either store; a direct mask-free ass…
- verdict: KILLED (instance) — all three: score 12, build_insns 185, byte-identical

## [s3] Reusing three independently-spelled forms of the post-call triple-store block (s2's mask-reuse; two fresh locals arg_val/count_val both loaded before either store; a direct mask-free assignment) changes the scheduler's…
- verdict: KILLED (instance) — All three variants: sandbox score 12, build_insns 185, byte-identical objdump in the triple-store region (both stores land at the same two instructions immedia…

## [s4] Directed permuter campaign 1 also surfaced `extern volatile int/short/char D_8009BF6C`/`D_8009BF70` coercion forms (output-715-1, output-765-1, output-915-1) scoring between the do-while form and the unwrapped baseline.
- verdict: KILLED (instance) — REJECTED without measurement beyond reading the diff -- these globals have no identifiable IRQ writer independent of _exeque's own queue-draining loop and no c…

## [s4] Directed permuter campaign 1 also surfaced `output-905-1`: moving `SetIntrMask(D_8009BF84)` from after the loop to inside the loop body (between the two triple-store fields).
- verdict: KILLED (instance) — REJECTED as not a candidate at all (behavior-changing, not a respelling) -- not measured further, not applied to src.

## [s4] Marking D_8009BF6C and/or D_8009BF70 volatile (permuter campaign-1 finds output-715-1/765-1/915-1) is not a legitimate lever for this residual.
- verdict: KILLED (instance) — REJECTED without src application -- no identifiable IRQ writer independent of _exeque's own loop, no catalogued use-site shape, and direct contradiction of alr…

## [s4] Moving SetIntrMask(D_8009BF84) from after the loop to inside the loop body (permuter campaign-1 find output-905-1) is a valid respelling.
- verdict: KILLED (instance) — REJECTED as not a valid candidate at all (behavior-changing, not a respelling); never applied to src/display.c

## [s4] A third directed permuter campaign (base permuter score 200, sandbox floor 2/187) targeting the sole remaining residual -- the final IRQ-callback block's jalr delay-slot fill, where target keeps sw $zero,0($v1) (D_8009B…
- verdict: KILLED (instance) — 0 novel finds below 200 after 9096 iterations -- the campaign never found any spelling closer to 0 than the do-while chassis itself. This corroborates (does no…

## [s5] A do-while(0) wrap around the final "clear D_8009BE7C, invoke D_8009BE80 callback" two-statement block does NOT perturb the jalr delay-slot fill (unlike H9/H10's triple-store wraps, which each dropped the floor).
- verdict: KILLED (instance) — score unchanged at 2/187 (build_insns unchanged 186) -- confirms this residual is not an RTL-compound-statement-boundary issue the way the triple-store block w…

## [s5] Hoisting the D_8009BE80 callback pointer into a named local `cb` before the guard test, calling `cb()` instead of a cast-call-through-global, makes the score WORSE (2 -> 11), not better.
- verdict: KILLED (instance) — score 2 -> 11 (worse); reverted immediately, not applied to src (memory/grind/_exeque/rejected/cb-local-hoist-worse.c)

## [s5] A second fresh-seed permuter campaign (15356 iterations, ~9.3 min) on the exact floor-2/187 jalr-delay-slot residual, re-launched from a copy of s4's campaign-3 workspace (same base.c/target.o, base permuter score 200),…
- verdict: KILLED (instance) — 0 novel finds after 15356 iterations this session. Combined with s4 campaign 3's 9096 iterations on the identical residual/chassis, cumulative iteration count…

## [s5] A do-while(0) wrap around the final "clear D_8009BE7C, invoke D_8009BE80 callback" two-statement block does NOT perturb the jalr delay-slot fill (unlike H9/H10's triple-store wraps, which each dropped the floor).
- verdict: KILLED (instance) — score unchanged at 2/187, build_insns unchanged (186) -- confirms this residual is not an RTL-compound-statement-boundary issue the way the triple-store block…

## [s5] Hoisting the D_8009BE80 callback pointer into a named local `cb` before the guard test, calling `cb()` instead of a cast-call-through-global, improves the residual.
- verdict: KILLED (instance) — score got WORSE, 2 -> 11 (build_insns dropped 186 -> 184, i.e. this changed codegen structurally rather than just the delay-slot reschedule); reverted immediat…

## [s5] A second fresh-seed directed permuter campaign on the exact floor-2/187 jalr-delay-slot residual (re-launched from a copy of s4's campaign-3 workspace, identical base.c/target.o, base permuter score 200) can find a non-…
- verdict: KILLED (instance) — 0 novel finds after 15356 iterations this session. Combined with s4 campaign 3's 9096 iterations on the identical residual/chassis, cumulative iteration count…

## [s6] Ran the instrumented-cc1 .dbr dump (pass-attribution discipline) on the s4/s5 floor-2/187 chassis to re-confirm the jalr-delay-slot mechanism before probing further, per role-prompt requirement.
- verdict: N/A (instance) — confirmed the exact mechanism the ledger already identified: RTL insn 311 (`(set (mem:SI (reg/v:SI 3 v1)) (const_int 0))` — the `*p = 0;` store, address pre-ma…

## [s6] Three fresh structural respellings of the final-callback block/outer-guard/SetIntrMask-placement, none of which were previously measured, all regress the sandbox floor from 2/187 — the jalr-delay-slot residual is not re…
- verdict: KILLED (instance)

## [s6] Respelling the outer if+do-while loop as a plain `while` loop (previously-untried loop-geometry spelling) has NO effect on the jalr-delay-slot residual.
- verdict: KILLED (instance) — score unchanged at 2/187. GCC 2.7.2's loop.c performs the same loop-rotation on a plain `while` as the source already spelled explicitly, so this respelling is…

## [s6] Swapping the final callback block's inner guard `&&` operand order (`if (*p != 0 && D_8009BE80 != 0)` -> `if (D_8009BE80 != 0 && *p != 0)`) on the s4/s5 do-while(0)-wrapped floor-2/187 chassis does not close or improve…
- verdict: KILLED (instance) — Score regressed 2 -> 8. Reverted. Banked memory/grind/_exeque/rejected/final-block-cond-swap-worse.c

## [s6] Swapping the outer post-loop guard's `&&` operand order (`if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000))` -> `if (!(*D_8009BF54 & 0x01000000) && D_8009BF78 == D_8009BF7C)`) on the identical chassis does no…
- verdict: KILLED (instance) — Score regressed 2 -> 14 (worse than the inner-guard swap). Reverted. Banked memory/grind/_exeque/rejected/outer-guard-cond-swap-worse.c

## [s6] Relocating the unconditional `SetIntrMask(D_8009BF84);` call (moving it from before the outer post-loop guard to duplicated inside both if/else arms of that guard, adjacent to the final callback block) does not close or…
- verdict: KILLED (instance) — Score regressed 2 -> 18 (worst of the three probes). Reverted. Banked memory/grind/_exeque/rejected/setintrmask-duplicated-into-arms-worse.c

## [s6] Respelling the outer `if (cond) { do { body } while (cond); }` loop as a plain `while (cond) { body }` loop (removing the C-level duplicated top-of-loop guard) has no effect on the jalr-delay-slot residual.
- verdict: KILLED (instance) — Score unchanged at 2/187. Reverted to the already-banked if+do-while spelling (equal score, no reason to prefer the diff).

## [s7] SYSTEMATIC SPELLING ENUMERATION (owner ruling 2026-09-08 protocol) of the final-callback block's named-intermediate space fails to find anything better than floor 2/187, and reproduces floor 2 ONLY from spellings struct…
- verdict: KILLED (instance) — Best score across all 19 spellings is 2/187 (v12.c and v16.c -- both are the "flag and cb fully inlined back to their original `*p`/`D_8009BE80` spelling" form…

## [s7] On the s4/s5/s6 do-while(0)-wrapped floor-2/187 chassis (both FAKE-annotated wraps present, no volatile/cheat constructs), the three-local {p,flag,cb} named-intermediate enumeration space for the final-callback block (1…
- verdict: KILLED (instance) — ?

## [s8] On the current floor-2/187 chassis, neither of the two /* FAKE */ do-while(0) wraps is an inert carrier occupying a pseudo that a live lever needs (the func_8002EA24 s8 false-kill pattern) — both are independently load-…
- verdict: KILLED (instance) — keep-all (both wraps)=2/187 (186 insns); drop-nested-only=7/187; drop-outer-only=7/187 (185 insns); drop-both=12/187 (185 insns). Confirms s4's original single…

## [s9] SOLVER — sched_solver's own model proves this SCHED classification is a false positive for the sched1/sched2 layer: perturb.py finds ZERO blocks in `_exeque` (either pass) where target's required pre-reorg pick order di…
- verdict: KILLED (instance) — both passes print only the header alignment stats (`align honobj->tgtobj: |A|=186 |B|=187 {'equal': 185, 'replace': 1, 'delete': 0, 'insert': 1, 'moved': 0}`,…

## [s9] On the current floor-2/187 do-while(0)-wrapped chassis (memory/grind/_exeque/candidate.c, both FAKE wraps present), sched_solver's validated sched1+sched2 model of _exeque finds ZERO blocks, in either pass, where target…
- verdict: KILLED (instance) — Both passes print only the header alignment stats (align honobj->tgtobj: |A|=186 |B|=187 {'equal': 185, 'replace': 1, 'delete': 0, 'insert': 1, 'moved': 0}, ma…

## [s10] SYNTHESIS: with axis 2 formally class-killed, the floor-2/187 frontier is down to exactly ONE item: axis 1, the H6 legitimate-volatile-interrupt-touched ruling-request (unchanged since s2, now the ONLY remaining un-fals…
- verdict: KILLED (instance) — frontier narrowed to axis 1 (ruling-request) plus a newly-identified, broader, unexplored axis 3 (RTL-shape restructuring beyond register assignment) for a fut…

## [s11] A fresh m2c --target=mipsel-ido-c decompile of asm/funcs/_exeque.s reconstructs the top-level guard as a single-exit accumulator (`ret = 1; if (!cond) { ...; ret = result; } return ret;`) instead of the candidate's earl…
- verdict: KILLED (instance) — Score 7/187 (build_insns 188) vs the banked 2/187 (build_insns 186) — worse. Reverted; banked as memory/grind/_exeque/rejected/m2c-single-exit-toplevel.c.

## [s11] The same m2c decompile reconstructs the final guard/clear/call block as ONE merged `&&` condition reading/writing D_8009BE7C directly, with no pointer-local indirection (`if (a && b && D_8009BE7C != 0 && D_8009BE80 != 0…
- verdict: KILLED (instance) — Score 5/187 (build_insns 187) vs the banked 2/187 — worse. Reverted; banked as memory/grind/_exeque/rejected/m2c-final-block-merged-condition.c.

## [s11] Isolating the pointer-local question alone (keeping the candidate's nested if/if two-level control-flow structure for the final block, but dropping ONLY `s32 *p = &D_8009BE7C;` in favor of direct `D_8009BE7C` reads/writ…
- verdict: KILLED (instance) — Score 5/187 (build_insns 187) vs the banked 2/187 -- worse, identical to the merged-condition variant's score. This DISPROVES the inertness hypothesis: the poi…

## Latest session s12 (verbatim)

## [s12] On the current chassis (memory/grind/_exeque/candidate.c applied verbatim to src/display.c, s4-s11 do-while(0) wraps unchanged, no volatile/pin/cheat-asm present), the honest sandbox floor re-measures at exactly 2/187 this session.
- mechanism: measurement, not inference
- probe: tools/wteng.ps1 main sandbox _exeque --disable all after applying candidate.c to src/display.c
- result: score=2, target_insns=187, build_insns=186, rules_dropped=0, cheat_asm_stripped=143
- verdict: CONFIRMED

## [s12] scan_hand_coded reports tier=LOW score=0/8 for _exeque this session, with all eight hand-coded signals (S1-S8) unset.
- mechanism: measurement, not inference
- probe: python3 tools/scan_hand_coded.py --single _exeque
- result: HAND_CODED: tier=LOW score=0/8 (_exeque, 187 insns); no strong hand-coded indicators; all S1-S8 boxes unchecked
- verdict: CONFIRMED

## [s12] No PSX-provenance SOTN-master precedent exists in docs/reference/sotn-construct-index.md for a 'guard-clear-and-invoke' extern-volatile use-site shape (the only known closing spelling for the jalr-delay-slot residual, rejected as memory/grind/_exeque/rejected/volatile-D_8009BE7C-guard-clear.c).
- mechanism: measurement, not inference (grep census)
- probe: grep -n -i 'extern volatile|IRQ|interrupt|guard.*clear.*invoke|clear-and-invoke|delay-slot' docs/reference/sotn-construct-index.md
- result: zero matches for extern volatile / IRQ / interrupt anywhere in the 2746-line index; the only volatile/pad hits (lines 29-919) are unrelated pad/dummy-local exhibits, not IRQ-touched-global or guard-clear-and-invoke shapes
- verdict: KILLED
- kill_scope: class
- measured_on: docs/reference/sotn-construct-index.md at HEAD (2026-09-16), full-file grep, PSX-tagged entries only
- predicate_cite: .claude/rules/legitimate-volatile-interrupt-touched.md:1
