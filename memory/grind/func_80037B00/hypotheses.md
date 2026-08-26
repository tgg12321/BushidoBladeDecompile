# Hypothesis ledger — func_80037B00

## [s1] The +8 phantom frame is not independently addressable; it materializes only when register pressure keeps a local alive across the loop, so it's downstream of the register-rotation axis.
- mechanism: Target has addiu sp,-8/addiu sp,+8 with ZERO memory accesses at sp+0..7 and zero jals (outgoing-args=0), so the 8 bytes are a vars slot. GCC 2.7.2 provably never DCEs local-array stores (file_LoadSectors DCE tests), so any pure-C written-local emits stores that target lacks; unused-local is dead-vars-local-array (forbidden except the 2026-07-01 carve-out which requires target dead stores — target has none). Only surviving mechanism: pin-induced reload pressure keeps a coalesced-away scalar alive, producing the frame as a side-effect of the register axis.
- probe: Confirmed by ledger evidence (WIP note #2) + this session's asm read; no fresh measurement needed.
- result: Axis is contingent, not independent. Any register-axis fix that keeps a natural live-but-memory-dead local produces the frame implicitly.
- verdict: CONFIRMED

## [s1] do-while loop restructure lowers the score.
- mechanism: Move outer/inner test to bottom to match target's bnez-with-delay-slot-advance shape.
- probe: Ledger: prior session measured do-while pin-free → sandbox=17 weighted, build_insns=33.
- result: Score WORSE than goto form (17 > 15 weighted, 33 < 34 build_insns — lost one inner-loop insn). Do-while collapses one slt/bnez pair GCC produces in the goto form.
- verdict: KILLED

## [s2] Narrow-int retyping (var_t1:s32->s16, var_t2:s32->u8) shifts RA scavenger enough to lock at least one of the rotated regs.
- mechanism: register-alloc-pure-c Lever B: GCC 2.7.2 global.c breaks conflict-graph ties by n_refs and mode; narrower modes could flip the tiebreaker.
- probe: Retyped var_t1 to s16 and var_t2 to u8 in src/code6cac_c.c and ran sandbox --disable all.
- result: Score ROSE from 15 to 21; build_insns rose 34->38 due to GCC inserting sign/zero-extension pack instructions at load/compare sites (net loss, target has no extend ops).
- verdict: KILLED

## [s2] Shared-end-label restructure (return ret; joined) with s32 ret local rebinds v0 assignment order to match target's late-zero pattern.
- mechanism: shared-end-label rule: single trailing return via ret prevents per-case constant-fold of return values that drops target stores; also could bias RA on the returned scalar.
- probe: Introduced s32 ret; both exit paths set ret and goto end; end: return ret; measured sandbox.
- result: Score ROSE from 15 to 19 (+4 weighted). build_insns unchanged at 34. The added live scalar competes with existing rotation locals and made the scavenger tiebreak WORSE for the 5-way rotation, not better.
- verdict: KILLED

## [s2] Splitting var_v0 into distinct outer var_count + inner var_v0 narrows live-range and unblocks the rotation.
- mechanism: register-alloc-pure-c Lever A: block-local var split — two disjoint live ranges instead of one variable-reuse coalesce.
- probe: Renamed the outer D_800A38C8 reader to var_count while keeping inner byte-loads on var_v0; measured sandbox.
- result: Score UNCHANGED at 15, build_insns=34. GCC coalesced var_count and var_v0 back into the same pseudo (single-def-then-dead form) — the split had no effect on RA.
- verdict: KILLED

## [s2] Declaration order in target-register-hint sequence (t1,a1,a2,t0,v1 first) biases GCC's initial pseudo numbering / allocation preference toward the target rotation.
- mechanism: register-alloc-pure-c: decl order can influence pseudo-register numbering and thus tiebreak in allocation.
- probe: Reordered locals to var_t1,var_a1,var_a2,var_t0,var_v1 first; measured sandbox.
- result: Score UNCHANGED at 15, build_insns=34. Decl order not a live lever for this function's rotation — GCC's global.c re-sorts pseudos by n_refs+mode before allocation.
- verdict: KILLED

## [s2] Reading both bytes (var_v1=*var_a2; var_v0=*var_a1;) before either compare produces parallel loads that shift the scheduling and free the register rotation.
- mechanism: Load-order restructuring: moving var_v0 load ahead of the zero-terminator branch removes a branch-hop between the two loads.
- probe: Placed both u8 loads at top of loop_inner before both if-tests; measured sandbox.
- result: Score ROSE from 15 to 17; build_insns FELL from 34 to 32 (target has 36). GCC folded/scheduled the paired loads and DROPPED two instructions target keeps. Moves further from target shape.
- verdict: KILLED

## [s3] Statement re-association: defer var_t1 = 0 past the guard + inline the guard's early-exit as return 0 shifts the t1-init LUID position and lets it schedule differently into the blez delay slot.
- mechanism: register-alloc-pure-c interaction with sched1 — moving the zero-init AFTER the blez check places it in the taken arm's LUID sequence, which could bias GCC's scheduler on where the `addu tX,zero,zero` lands (delay-slot vs pre-branch) and thus which register the counter pseudo gets.
- probe: Edited src to `var_v0 = D_800A38C8; if (var_v0 <= 0) return 0; var_t1 = 0; var_t3 = var_v0; ...` and dropped the block_end label. Ran sandbox --disable all.
- result: Score UNCHANGED at 15 weighted, build_insns=34 (cheat_asm_stripped=8 vs 27 confirms pin-free measure). GCC hoisted the t1-init back into the blez delay slot identically to the baseline form; no LUID differential survives the scheduler. Saved rejected/stmt-reassoc-direct-return.c.
- verdict: KILLED

## [s3] Statement re-association (defer var_t1=0 past the guard + inline the guard as return 0 + drop block_end label) shifts the t1-init LUID and lets the scheduler place `addu tX,zero,zero` into blez's delay slot from a different register, biasing RA on the counter.
- mechanism: register-alloc-pure-c interaction with sched1 — source position of a zero-init influences LUID ordering, which can steer where the init lands (delay-slot vs pre-branch) and which pseudo the counter maps to.
- probe: Rewrote src/code6cac_c.c func_80037B00 to `var_v0 = D_800A38C8; if (<=0) return 0; var_t1 = 0; var_t3 = var_v0; ...` (also removed trailing block_end label). Ran `sandbox func_80037B00 --disable all`.
- result: sandbox=15 weighted, target_insns=36, build_insns=34, cheat_asm_stripped=8 (pin-free measure confirmed). Identical to baseline candidate.c form — GCC hoisted the init back into the blez delay slot; the inlined `return 0` folded to the same j/addu tail. Saved memory/grind/func_80037B00/rejected/stmt-reassoc-direct-return.c.
- verdict: KILLED

## [s4] decomp-permuter import.py produces a workable base.c/compile.sh/target.o triplet out-of-the-box for func_80037B00.
- mechanism: Standard import path per permuter-directives; used for prior BB2 workspaces (mar, csmd4, etc.).
- probe: Ran `python3 tools/decomp-permuter/import.py src/code6cac_c.c asm/funcs/func_80037B00.s` from repo root via WSL venv.
- result: Import completed, but the generated target.o was elf32-tradbigmips (wrong endian: emitted compile.sh's assembler line was `mips-linux-gnu-as -march=vr4300 -mabi=32`); base.c had two duplicate-declaration lines (EnterCriticalSection, D_80102810) and one undefined `_permuter_ignore_line` marker at line 751 inside func_80037A20's __asm__ block. All three fixed manually. target.o rebuilt using r3k prelude (elf32-tradlittlemips confirmed, 43-line objdump).
- verdict: KILLED

## [s4] The engine-standard compile.sh pipeline (cpp | cc1 | prologue_fix | maspsx | sed | multu_pad | as) invoked via `bash compile.sh base.c -o base.o` produces a valid base.o.
- mechanism: Full pipeline mirrors the engine's build stages; drops regfix/asmfix since func_80037B00 has zero rules in HEAD (pin-free).
- probe: Ran `bash nonmatchings/func_80037B00/compile.sh base.c -o base.o` under WSL. Rewrote compile.sh three times: (1) piped form matching import.py output, (2) simplified without regfix/asmfix, (3) temp-file staging with `cat file | tool > next` per stage. Tried `--force-stdin` on maspsx.
- result: All three forms FAIL at the maspsx stage with `MASPSX: An exception occurred: too many values to unpack (expected 3)`. s3.s writes 0 bytes; downstream `as` errors out. When the identical pipeline stages are invoked STANDALONE (tmp/grind/func_80037B00/s4/trace.sh: cpp | cc1 -w > s1.s; prologue_fix < s1.s > pre.s; maspsx < pre.s > post.s), maspsx SUCCEEDS — post.s is 733 lines and ends cleanly with `.end func_80037C34`. Difference between the two invocations not localized in-session.
- verdict: KILLED

## [s5] A working permuter workspace for func_80037B00 can be built via the mar_perm_workspace.sh full-TU compile + region-extract pattern, bypassing s4's broken decomp-permuter import.py path.
- mechanism: cpp preprocess src/code6cac_c.c -> base.c (all-includes-resolved); compile.sh runs the engine pipeline (cc1 | prologue_fix | maspsx | fix_lwl | multu_pad) into a scratch .s, awk-extracts the func_80037B00 region (`.globl func_80037B00` -> `.size func_80037B00`), assembles under r3k prelude with mipsel-linux-gnu-as. target.o built from asm/funcs/func_80037B00.s + prelude_r3k.inc (drop `.set gp=64`).
- probe: Wrote tmp/grind/func_80037B00/s5/build_ws.sh; ran under WSL; sanity-checked with mipsel-linux-gnu-objdump diff base.o vs target.o (both elf32-tradlittlemips, func at offset 0).
- result: Workspace built cleanly. Base 30 insns vs target 36 insns; diff shows 5-way register rotation (v1/a2/a1/t0/t1 vs a1/v1/a2/t1/t0) + missing 8-byte prologue/epilogue frame — exact shape the ledger predicted. Permuter launch reported base_score=290 (permuter-weighted metric).
- verdict: CONFIRMED

## [s5] A fresh-seed random-mode permuter campaign from the pin-free candidate.c will find a score-0 pure-C form for func_80037B00 within a ~20-30 min fresh-seed window.
- mechanism: decomp-permuter mutates base.c structurally (declaration reorder, variable rename/split/merge, associative rewrites, statement move, etc.) and rescores. If a matching allocation exists in the accessible mutation space, score 0 emerges.
- probe: Launched: `permuter_campaign.py launch --func func_80037B00 --dir tmp/grind/func_80037B00/s5/perm_ws --label s5_pinfree_v1 -j 4 --stop-on-zero`. Waited in-turn via 27 successive `wait --timeout-min 9` calls. Campaign wall time ~20 min, ~29,000 iterations. Novel-find rate stayed positive throughout; harvest --stop on completion.
- result: 66 novel finds, ZERO score-0. Best legitimate non-cheat score: 195 (still ~20 permuter-weighted diffs above target). Every score <=175 (six candidates: 70/75/90/95x2/175) belongs to the SAME banned family: named-holder-local (`short new_var;` / `int new_var;` / dead-reassign an existing local as a zero-holder) + occasionally a no-semantic wrapper (`if (1) { ... }`). Directly the family the Judge s4-1549 rejected form (judge-fail-0728-1549.c) documents, and the Judge constraint in this brief explicitly bans (`may NOT be closed via nested do-while(0) + named-holder locals`).
- verdict: KILLED

## [s5] Standing 2026-07-27 auto-ruling: with the permuter axis measured and only banned closing forms surviving, BOTH endgame-lock gates fail with no lever remaining, so the terminal disposition is REFUSED / OWNER-ACCEPTED INCOMPLETE.
- mechanism: Gate 1 (canonical-asm STRONG scan_hand_coded S1/S2/S6): func_80037B00 is a plain strncmp-style dispatch loop (matches arg0 against each 0x15-byte D_80102810 entry, 0x28 stride); no GTE/BIOS/hardware, no unusual constructs. FAILS. Gate 2 (SOTN in-hand precedent covering 5-way coupled register rotation + 8-byte phantom frame): no dead-store / pointer-alias / named-local / duplicated-statement / exit-path carve-out binds; the s2/s3 structural axis exhaustively confirms this. FAILS. Structural axis exhausted s2/s3 (7 killed + 2 inert), permuter axis now measured — no grindable lever remains.
- probe: Filed docs/grind/decisions.md entry titled OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE, stating both gates' evidence + permuter fresh-seed negative result + banned-family score table. Reverted src/code6cac_c.c to HEAD.
- result: Entry filed at docs/grind/decisions.md (section '2026-07-28 — func_80037B00 — OWNER-ESCALATION — RESOLVED BY STANDING RULING'). No owner action pending; the driver parks func_80037B00 out of active grind.
- verdict: CONFIRMED

## [s6] The residual is not "5-way register rotation + phantom frame + scheduling shift"; typing it mechanically will show a much smaller, differently-shaped question.
- mechanism: inverse_compose's funnel taxonomy — register-blanked multisets differing means PRE-RA (a different instruction set, unreachable by RA or scheduler perturbation); multisets matching with different registers means RA; matching texts in a different order means SCHED. No prior session ran this classification, so the "5-way rotation" framing was never validated against the actual streams.
- probe: tmp/grind/func_80037B00/s6/multiset.py — objdump the cheat-stripped sandbox object, parse asm/funcs/func_80037B00.s, blank register names / immediates / symbols, normalise objdump's `move`/`li` aliases back to `addu`/`addiu`, then compare as a multiset AND position-by-position.
- result: The multiset delta is EXACTLY `addiu $sp,$sp,-0x8` + `addiu $sp,$sp,0x8` — two instructions, nothing more, all four nops matching. With those two removed the ORDERED streams align position-for-position across all 34 slots. So there is ONE pre-RA question (the frame) and ONE RA question (registers), and ZERO scheduler question.
- verdict: CONFIRMED

## [s6] s1's third axis — "subtle shift in slt/addiu ordering near .L80037B30, a side-effect of the different RA" — does not exist.
- mechanism: s1 read the two streams side by side while they had different lengths (34 vs 36), so every instruction after the missing prologue appeared displaced by one slot.
- probe: the ordered diff above, after accounting for the two sp instructions.
- result: No ordering difference anywhere in the function. reorg/sched are not implicated. One axis removed from the search space for free.
- verdict: KILLED

## [s6] GCC 2.7.2's register assignment for this function is a deterministic function of allocno priority rank, so the "rotation" is a sorting problem with exactly two C-visible dials (n_refs, live_length) plus birth order as tiebreak.
- mechanism: global.c allocno_compare sorts by pri = floor_log2(n_refs)*n_refs*size/live_length descending, ties to the lower pseudo number (= birth order); find_reg then walks the free hard registers ascending. All ten allocnos here mutually conflict (greg conflict lines), so no preference or conflict subtlety can intervene — rank N gets the Nth free register.
- probe: computed pri from the .flow `used N times across M insns` lines for pseudos 72-81 and compared the predicted ordering against the greg's own `;; 10 regs to allocate:` line.
- result: Predicted [81 77 78 80 76 73 79 75 74 72]; greg prints [81 77 78 80 76 73 79 75 74 72]. Exact. The model is usable as a forward oracle for candidate spellings without compiling.
- verdict: CONFIRMED

## [s6] The 8-byte phantom frame can be produced by ordinary live C via the folded-loop-guard-compare orphan, dissolving the s1..s5 conclusion that it required the forbidden dead-vars-local-array family.
- mechanism: .claude/rules/phantom-slot-frame-lever.md producer #1 + [[phantom-frame-slots-gcc272]]. A top-tested loop makes GCC duplicate the exit test as an entry guard; combine folds the duplicated comparison into a bare branch (`blez`) but the compare pseudo keeps a reference, reaches reload unallocated, and alter_reg assigns it a stack slot. get_frame_size() counts it; no instruction ever touches it; MIPS_STACK_ALIGN rounds 4 up to 8.
- probe: rewrote the outer loop as `while (var_t1 < var_t3)` with the explicit pre-guard deleted; read cc1's own `.frame` line via tmp/grind/func_80037B00/s6/frame_probe.sh; confirmed the orphan in the .greg dump (pseudo 86 listed in `;; 10 regs to allocate:` but absent from `;; Register dispositions:`).
- result: `.frame $sp,8,$31 # vars= 8` — target's frame exactly, at zero instruction cost, from live named locals only. Combined with the s0..s5 goto inner loop this measured score=11, build_insns=35 (target 36), lowering the floor from 15 for the first time since s2.
- verdict: CONFIRMED

## [s6] Re-adding an explicit `if (count <= 0) return 0;` guard in front of the top-tested while keeps the frame while restoring target's post-guard `la` placement.
- mechanism: the explicit guard would dominate the loop, so the preheader (and any invariant placed in it) sits after the branch, matching target's `lui/addiu $a3` position — while GCC's own rotation guard would still supply the orphan.
- probe: v_hybrid3.c (= the merged form plus `var_v0 = D_800A38C8; if (var_v0 <= 0) return 0; var_t3 = var_v0;`); read the .frame line.
- result: `vars= 0`. Jump threading recognises GCC's duplicated guard as redundant against the programmer's and deletes it, taking the block-0 compare pseudo with it. The orphan is only available when the guard duplication is GCC's own. Banked rejected/explicit-guard-kills-rotation-orphan.c.
- verdict: KILLED

## [s6] Writing the inner loop as natural `do { ... break; ... } while (p < end)` is byte-neutral relative to the goto spelling.
- mechanism: both should lower to the same bottom-tested loop with two early exits.
- probe: v_for.c (whole function in natural C: for-loop outer, do-while inner with breaks); sandbox + frame probe.
- result: `vars= 8` (frame fine) but score 22 / build_insns 38 — GCC rotates and peels the inner loop too, duplicating the first `lbu` above the loop and adding a `beqz`/`bnez` pair. The inner loop must keep the s0..s5 goto spelling; only the OUTER loop may be top-tested. Banked rejected/for-loop-rotates-inner-loop-too.c.
- verdict: KILLED

## [s6] Splitting the count read into `var_v0 = D_800A38C8; var_t3 = var_v0;` recovers target's `addu $t3,$v0,$zero` preheader copy.
- mechanism: target's stream loads D_800A38C8 into $v0, branches on it, then copies it into the loop-bound register $t3 — two pseudos where our form has one, so spelling two C locals should reproduce the pair.
- probe: v_h4.c; sandbox + frame probe.
- result: Identical to the merged form (score 11, build_insns 35, vars= 8) — copy propagation folds the second local away because the first has no surviving use. The copy must come from a mechanism that keeps both pseudos live, not from a C-level duplicate. Banked rejected/split-count-copy-folded-by-copyprop.c.
- verdict: KILLED

## [s6] The residual is not '5-way register rotation + phantom frame + scheduling shift'; typing it mechanically shows a much smaller, differently-shaped question.
- mechanism: inverse_compose's funnel taxonomy: register-blanked multisets differing => PRE-RA (a different instruction set, unreachable by RA or scheduler perturbation); multisets matching with different registers => RA; matching texts in a different order => SCHED. No prior session ran this classification, so the 'five-way rotation' framing was never validated against the actual streams.
- probe: tmp/grind/func_80037B00/s6/multiset.py — objdump the cheat-stripped sandbox object, parse asm/funcs/func_80037B00.s, blank register names/immediates/symbols, normalise objdump's move/li aliases back to addu/addiu, compare as a multiset AND position-by-position.
- result: The multiset delta is EXACTLY `addiu $sp,$sp,-0x8` + `addiu $sp,$sp,0x8` — two instructions, nothing else, all four nops matching with identical multiplicity. Removing those two makes the ORDERED streams align position-for-position across all 34 slots. So there is one PRE-RA question (the frame) and one RA question (registers), and zero scheduler question.
- verdict: CONFIRMED

## [s6] s1's third axis — 'subtle shift in slt/addiu ordering near .L80037B30, a side-effect of the different RA' — does not exist.
- mechanism: s1 read the two streams side by side while they had different lengths (34 vs 36), so every instruction after the missing prologue appeared displaced by one slot.
- probe: The ordered diff above, after accounting for the two sp instructions.
- result: No ordering difference anywhere in the function. sched1/sched2/reorg are not implicated at all. One axis removed from the search space for free.
- verdict: KILLED

## [s6] GCC 2.7.2's register assignment for this function is a deterministic function of allocno priority rank, so the 'rotation' is a sorting problem with exactly two C-visible dials (n_refs, live_length) plus birth order as tiebreak.
- mechanism: global.c allocno_compare sorts by pri = floor_log2(n_refs)*n_refs*size/live_length descending, ties to the lower pseudo number (= birth order); find_reg then walks the free hard registers ascending. All ten allocnos here mutually conflict (greg conflict lines), so no preference or conflict subtlety can intervene — rank N gets the Nth free register.
- probe: Computed pri from the .flow 'used N times across M insns' lines for pseudos 72-81 and compared the predicted ordering against the greg's own ';; 10 regs to allocate:' line.
- result: Predicted [81 77 78 80 76 73 79 75 74 72]; greg prints [81 77 78 80 76 73 79 75 74 72]. Exact match — the model is usable as a forward oracle for candidate spellings without compiling.
- verdict: CONFIRMED

## [s6] The 8-byte phantom frame can be produced by ordinary live C via the folded-loop-guard-compare orphan, dissolving the s1..s5 conclusion that it required the forbidden dead-vars-local-array family.
- mechanism: .claude/rules/phantom-slot-frame-lever.md producer #1 + the phantom-frame-slots-gcc272 memory. A top-tested loop makes GCC duplicate the exit test as an entry guard; combine folds the duplicated comparison into a bare branch (blez) but the compare pseudo keeps a reference, reaches reload unallocated, and alter_reg assigns it a stack slot. get_frame_size() counts it; no instruction ever touches it; MIPS_STACK_ALIGN rounds 4 up to 8.
- probe: Rewrote the outer loop as `while (var_t1 < var_t3)` with the explicit pre-guard deleted, keeping the s0..s5 goto inner body verbatim; read cc1's own .frame line via tmp/grind/func_80037B00/s6/frame_probe.sh; confirmed the orphan in the .greg dump; ran sandbox --disable all.
- result: `.frame $sp,8,$31 # vars= 8` — target's frame exactly, at zero instruction cost, from live named locals only. The orphan is visible directly in the dump (pseudo 86 present in ';; 10 regs to allocate:' but absent from ';; Register dispositions:'). sandbox --disable all: score=11, target_insns=36, build_insns=35 — floor lowered from 15 for the first time since s2.
- verdict: CONFIRMED

## [s6] Re-adding an explicit `if (count <= 0) return 0;` guard in front of the top-tested while keeps the frame while restoring target's post-guard `la` placement.
- mechanism: The explicit guard would dominate the loop, so the preheader (and any invariant placed in it) sits after the branch, matching target's lui/addiu $a3 position — while GCC's own rotation guard would still supply the orphan.
- probe: tmp/grind/func_80037B00/s6/v_hybrid3.c (the merged form plus `var_v0 = D_800A38C8; if (var_v0 <= 0) return 0; var_t3 = var_v0;`); read the .frame line.
- result: `vars= 0`. Jump threading recognises GCC's duplicated guard as redundant against the programmer's and deletes it, taking the block-0 compare pseudo with it. The orphan is only available when the guard duplication is GCC's own. Banked rejected/explicit-guard-kills-rotation-orphan.c.
- verdict: KILLED

## [s6] Writing the inner loop as natural `do { ... break; ... } while (p < end)` is byte-neutral relative to the goto spelling.
- mechanism: Both should lower to the same bottom-tested loop with two early exits.
- probe: tmp/grind/func_80037B00/s6/v_for.c (whole function in natural C: for-loop outer, do-while inner with breaks); sandbox + frame probe.
- result: `vars= 8` (frame fine) but score 22 / build_insns 38 — GCC rotates and peels the inner loop too, duplicating the first lbu above the loop and adding a beqz/bnez pair. Only the OUTER loop may be top-tested; the inner loop must keep the s0..s5 goto spelling. Banked rejected/for-loop-rotates-inner-loop-too.c.
- verdict: KILLED

## [s6] Splitting the count read into `var_v0 = D_800A38C8; var_t3 = var_v0;` recovers target's `addu $t3,$v0,$zero` preheader copy.
- mechanism: Target's stream loads D_800A38C8 into $v0, branches on it, then copies it into the loop-bound register $t3 — two pseudos where our form has one, so spelling two C locals should reproduce the pair.
- probe: tmp/grind/func_80037B00/s6/v_h4.c; sandbox + frame probe.
- result: Identical to the merged form (score 11, build_insns 35, vars= 8) — copy propagation folds the second local away because the first has no surviving use. The copy must come from a mechanism that keeps both pseudos live, not from a C-level duplicate. Banked rejected/split-count-copy-folded-by-copyprop.c.
- verdict: KILLED

## [s7] The missing `addu $t3,$v0,$zero` preheader copy appears when the loop bound is READ BY THE LOOP TEST ITSELF, with no count local at all.
- mechanism: In the s6 form the load's destination IS the loop-bound pseudo, so GCC emits `lw $t3` and the rotation guard branches on $t3 directly — one pseudo, no copy. Writing `while (var_t1 < D_800A38C8)` puts the load inside the loop's exit test: the rotation guard in block 0 gets its own copy of the load, loop.c hoists the in-loop load into the preheader, and cse unifies the hoisted load with the guard's, leaving a register-to-register copy in the preheader. The second pseudo is created by loop.c + cse, which is why the s6 C-level duplicate (`var_v0 = D_800A38C8; var_t3 = var_v0;`) could not produce it — copy propagation folds a source-level duplicate, but not a pass-created one.
- probe: tmp/grind/func_80037B00/s7/vA_bound_in_test.c (s6 body, `var_t3` deleted, `while (var_t1 < D_800A38C8)`); frame probe + `sandbox --disable all` + objdump of the honest object (tmp/grind/func_80037B00/s7/dis.sh).
- result: CONFIRMED and it lowered the floor. `vars= 8` retained; score 11 -> 9; build_insns 35 -> 36 == target_insns 36; `move t3,v0` present in the preheader at exactly target's slot. `goal_from_tgt.py classify` now reports FIRST DIVERGENCE: RA (was PRE-RA) with substitutions `$t0 -> $t1 x4`, `$t1 -> $t0 x2` — the instruction multiset matches target exactly.
- verdict: CONFIRMED

## [s7] Target's post-guard `lui/addiu` placement for D_80102810 can be reached by making `&D_80102810` a loop-invariant / induction-variable base so loop.c puts it in the preheader.
- mechanism: Only loop.c writes insns into a loop preheader. An assignment written before the `while` is ordinary block-0 straight-line code and is never moved there (the rotation guard is emitted at the loop's position, so everything textually before the loop precedes it). Writing the row address as `(s8 *)&D_80102810 + i * 0x28` makes the symbol loop-invariant, so LICM hoists the `la` into the preheader and strength reduction can supply the walking pointer.
- probe: tmp/grind/func_80037B00/s7/vB_giv_index.c (`var_a1 = (s8 *)&D_80102810 + var_t1 * 0x28;`, no `var_a3`) and vC_giv_pure.c (a dedicated never-modified `var_a3 = base + i*0x28;` then `var_a1 = var_a3;`). sandbox + objdump.
- result: The PLACEMENT hypothesis is CONFIRMED — in both variants the `lui/addiu` moves into the preheader, immediately after the guard, exactly as in target. The COST is fatal: both variants measure score 19 / build_insns 38 (+2 over target) and produce byte-identical streams to each other. GCC 2.7.2 reduces only the MULT giv (`$t2 = i*40`, init `move t2,zero` in the preheader) and then re-adds the invariant base every iteration (`addu $v1,$t2,$t4`); it never folds the base into the giv's initial value. vC's dedicated `var_a3` does not survive to loop.c — cse1 substitutes the whole expression into `var_a1`'s initialiser and into `var_t0 = var_a3 + 0x15`, so the giv target becomes a register that the inner loop modifies, which disqualifies it as a giv. The remaining +1 is loop.c peeling the `var_t2 = 0` flag init into the preheader and duplicating it into the back-branch delay slot. Banked rejected/giv-index-costs-offset-biv-add.c and rejected/giv-pure-coalesced-into-walking-ptr.c.
- verdict: KILLED (as a floor-lowering form; the placement mechanism itself is confirmed and remains the only known route to residual (a))

## [s7] Inlining the inner-loop end pointer into the test (`var_a1 < var_a3 + 0x15`, no `var_t0` local) lets loop.c create the pseudo and may change its allocation class favourably.
- mechanism: If loop.c hoists the invariant `var_a3 + 0x15` into the inner loop's preheader, the pseudo is created late and its refs/live-length are recomputed, which is the axis the register 2-swap needs.
- probe: tmp/grind/func_80037B00/s7/vD_inline_end.c; sandbox + `tools/grinder/dump.ps1` + tmp/grind/func_80037B00/s7/rainfo.sh.
- result: build_insns stays 36 but score REGRESSES 9 -> 14. The end pointer stops being a global allocno entirely: it is created inside block 5 and local-alloc assigns it `$v0` (`;; 10 regs to allocate:` drops from 11 to 10 entries; new pseudo 84 "used 4 times across 2 insns in block 5" -> reg 2). The end pointer must remain an outer-loop-scope named local. Banked rejected/inline-end-ptr-demotes-to-local-alloc.c.
- verdict: KILLED

## [s7] The remaining $t0/$t1 2-swap is reachable by re-weighting the COUNTER allocno (lengthening its live range) rather than the end pointer.
- mechanism: global.c allocno_compare, pri = floor_log2(n_refs)*n_refs*size/live_length. Lowering the counter below the end pointer requires pri(73) < 0.888 while staying above pri(74)=0.75, i.e. 24/live_length in (0.75, 0.888), i.e. live_length in (27, 32).
- probe: arithmetic against the measured model (validated on this exact form: predicted rank order [79 76 77 75 73 87 78 74 86 81 72] equals the greg's `;; 11 regs to allocate:` line verbatim).
- result: FORECLOSED. The counter is set in block 0 and last used at the outer-loop bottom test, so its live range already spans 22 of the function's ~26 RTL insns; there is no room to reach 27+ without adding instructions, and nothing uses the counter after the loop. Reducing its refs from 8 to 7 instead drops pri to 0.636, below pri(74)=0.75, which mis-assigns the flag register.
- verdict: KILLED

## [s7] The missing `addu $t3,$v0,$zero` preheader copy appears when the loop bound is read by the loop TEST itself, with no count local at all.
- mechanism: In the s6 form the load's destination IS the loop-bound pseudo, so GCC emits `lw $t3` and the rotation guard branches on $t3 directly - one pseudo, no copy. Writing `while (var_t1 < D_800A38C8)` puts the load inside the loop's exit test: the rotation guard duplicated into block 0 keeps its own copy of the load, loop.c hoists the in-loop load into the preheader, and cse unifies the two into a register-to-register copy. The second pseudo is created by a PASS, which is exactly why the s6 source-level duplicate (`var_v0 = D_800A38C8; var_t3 = var_v0;`) was folded by copy propagation while this spelling is not.
- probe: tmp/grind/func_80037B00/s7/vA_bound_in_test.c spliced into src/code6cac_c.c; cc1 .frame gradient (s7/probe.sh), `sandbox --disable all`, objdump of the honest sandbox object (s7/dis.sh), and `goal_from_tgt.py classify`.
- result: score 11 -> 9; build_insns 35 -> 36 == target_insns 36; `.frame $sp,8,$31 # vars= 8` retained; `move t3,v0` present in the preheader at target's exact slot. classify flips from PRE-RA/rtl_shape (every earlier form) to FIRST DIVERGENCE: RA with substitutions `$t0 -> $t1 x4`, `$t1 -> $t0 x2`.
- verdict: CONFIRMED

## [s7] Target's post-guard `lui/addiu` placement for D_80102810 is reachable by making `&D_80102810` a loop-invariant / induction-variable base so loop.c places it in the preheader.
- mechanism: Only loop.c writes insns into a loop preheader; an assignment written before the `while` is ordinary block-0 straight-line code and is never moved there, because the rotation guard is emitted at the loop's position and everything textually earlier precedes it. Writing the row address as `(s8 *)&D_80102810 + i * 0x28` makes the symbol loop-invariant so LICM hoists the `la` into the preheader and strength reduction can supply the walking pointer.
- probe: tmp/grind/func_80037B00/s7/vB_giv_index.c (row address computed inline, no var_a3) and s7/vC_giv_pure.c (dedicated never-modified var_a3 giv, then var_a1 = var_a3); sandbox + objdump.
- result: The PLACEMENT is confirmed - in both variants the lui/addiu moves into the preheader exactly where target has it. The COST is fatal: both measure score 19 / build_insns 38 (+2) and emit byte-identical streams. GCC 2.7.2 reduces only the mult giv (`$t2 = i*40`, preheader `move t2,zero`) and re-adds the invariant base every iteration (`addu $v1,$t2,$t4`); it never folds the base into the giv's initial value. vC's dedicated var_a3 does not survive to loop.c - cse1 substitutes the expression into var_a1's initialiser and into `var_t0 = var_a3 + 0x15`, so the giv target becomes a register the inner loop modifies. The last +1 is loop.c peeling the `var_t2 = 0` flag init into the preheader and duplicating it into the back-branch delay slot.
- verdict: KILLED

## [s7] Inlining the inner-loop end pointer into the test (`var_a1 < var_a3 + 0x15`, no var_t0 local) lets loop.c create the pseudo and may re-weight it favourably for the $t0/$t1 swap.
- mechanism: If loop.c hoists the invariant `var_a3 + 0x15` into the inner loop's preheader the pseudo is created late, and its refs / live-length - the only two dials in the allocno priority formula - are recomputed.
- probe: tmp/grind/func_80037B00/s7/vD_inline_end.c; sandbox + `pwsh tools/grinder/dump.ps1 func_80037B00` + s7/rainfo.sh.
- result: build_insns stays 36 but the score REGRESSES 9 -> 14. The end pointer stops being a global allocno: it is born in block 5 and local-alloc seats it in $v0 (the greg's allocno list drops from 11 to 10 entries; new pseudo 84 `used 4 times across 2 insns in block 5` -> reg 2). The end pointer must remain an outer-loop-scope named local.
- verdict: KILLED

## [s7] The remaining $t0/$t1 2-swap can be reached by re-weighting the COUNTER allocno (lengthening its live range) instead of the end pointer.
- mechanism: global.c allocno_compare, pri = floor_log2(n_refs)*n_refs*size/live_length. Pushing the counter below the end pointer while keeping it above the flag allocno needs 24/live_length in (0.75, 0.888), i.e. live_length in (27, 32) against the measured 22.
- probe: Arithmetic against the priority model re-validated on this exact form: predicted rank order [79 76 77 75 73 87 78 74 86 81 72] equals the greg's `;; 11 regs to allocate:` line verbatim, and the eight mutually-conflicting allocnos take [$3,$5,$6,$7,$8,$9,$10,$11] in that order.
- result: FORECLOSED. The counter is set in block 0 and last used at the outer-loop bottom test, so it already spans 22 of the function's ~26 RTL insns and nothing uses it after the loop - the range cannot reach 27 without adding instructions. Dropping its refs from 8 to 7 instead lands it at 0.636, below the flag allocno's 0.750, which mis-seats $t2.
- verdict: KILLED


## s8 (2026-08-26, forensics) — frontier replacement

**Status of the s7 frontier.** #1 (add one weight-1 reference to the end
pointer on the candidate.c chassis) is FORECLOSED: the weighting question it
was gated on resolved in its favour (out-of-loop refs really do weigh 1 —
flow.c:434 `depth = 1`, flow.c:2081/2329/2515/2725 `reg_n_refs += loop_depth`),
but the weight-1 region of this function contains no insn in which var_t0 is
live, so the (refs 5, live 9) cell has no spelling site. #2 (a giv spelling
that puts the `la` in the preheader) is SUPERSEDED, not by a better giv but by
the discovery that the `la` never needed hoisting: writing the entry guard as
a source-level `if` puts it in the preheader for free (alt_base_vF2.c). #3
(engineer an exact priority tie) is unchanged and still parked.

### F1 — Remove the redundant entry branch from the "explicit if + top-tested while" form.
That form (rejected/if-while-init-before-if-double-guard.c and its
init-inside-the-if sibling, tmp/grind/func_80037B00/s8/vG_if_while.c) is the
only measured form that has BOTH the 8-byte phantom frame (`vars= 8`) AND the
`la` in the guarded preheader. Its whole cost is that cc1 emits two
`blez $2,.L40` branches: the source `if`, and loop.c's
`duplicate_loop_exit_test` copy of the while's exit test placed in front of
NOTE_INSN_LOOP_BEG. Both the frame's orphan compare pseudo and the redundant
branch come from that duplication, so the question is precisely whether the
*earlier*, source-level branch can be made to disappear while the duplicate
stays.
*Mechanism to read first:* `tools/gcc-2.7.2/loop.c`, `duplicate_loop_exit_test`
— its bail-out conditions and exactly which insns it copies; and
`tools/gcc-2.7.2/jump.c` `jump_optimize` / `thread_jumps`, to see why the two
identical conditional branches on the same pseudo are not merged (jump1 runs
before loop.c creates the second one; jump2 runs after reload).
*Next probe:* dump `.loop`, `.jump2` and `.combine` for vG_if_while.c and
locate the two branch insns; then test spellings of the source guard that
jump2 CAN fold away — e.g. a guard whose condition is syntactically the loop
test (`if (var_t1 < D_800A38C8)` with `var_t1` already 0), or a guard placed so
that the la is the only insn between the two branches. Screen with
build_insns: 36 with `vars= 8` is the target; anything at 37 is the same
double guard.

### F2 — Find a frame producer for the alt_base_vF2 chassis.
alt_base_vF2.c is 34 insns and needs exactly the two `addiu $sp` adjusts; it
has no unallocated pseudo (`vars= 0`) because its single source-level guard
compare folds cleanly into `blez` and dies with zero refs. Producer #1 of
`.claude/rules/phantom-slot-frame-lever.md` needs a guard comparison pseudo
that combine folds into a bare branch while the pseudo still carries refs.
*Next probe:* extract the block-0 RTL of alt_base_vF2 from `.flow` and
`.combine` and compare it against the floor-9 form's insns 132/133/134 (see
tmp/grind/func_80037B00/s8/flow.rtl:110-125), where pseudo 87 survives combine
with 2 refs in ST_REGS and reaches reload homeless. Identify what gives 87 its
surviving reference, then look for an ordinary-C guard spelling on the vF2
chassis that reproduces it. NOTE the boundary in that rule file: the locals
must be real and live and the guard must be the function's own logic — a dead
conditional store that happens to produce the same orphan is the forbidden
family and must not be reached for.

### F3 — Re-open the RA swap only after F1 or F2 changes the stream.
Both chassis are exactly one adjacent allocno swap from target and both swaps
are currently cell-free:
  * candidate.c (floor 9): need pri(78, end ptr) in (pri(73) = 1.0909,
    pri(75) = 1.125); only (refs 5, live 9) fits and it has no spelling site.
  * alt_base_vF2.c (score 10): need pri(73, counter) in (pri(74) = 0.750,
    pri(78) = 0.888); refs 7 wants live_length 16-18 against a measured 23,
    refs 8 wants 27-32 in a ~26-insn function.
Any structural change from F1/F2 moves block boundaries and therefore every
live_length, so re-run tmp/grind/func_80037B00/s7/rainfo.sh and recompute the
window BEFORE spending probes on the swap. Note the measured coupling: moving
the `la` into the preheader shortens allocno 75 from live 24 to live 21 and
lifts its priority 1.125 -> 1.2857, which is what widens candidate.c's window.


## s8 addendum — after the floor moved 9 -> 5

F1 and F2 as written above are both CLOSED by the floor-5 form
(memory/grind/func_80037B00/candidate.c): spelling the entry guard as the
loop's own test (`if (var_t1 < D_800A38C8)` with var_t1 already 0) around a
top-tested `while` gives ONE `blez` and still lets duplicate_loop_exit_test
create the frame orphan, and reading the bound into `var_t3` as a source
statement before the `var_a3` assignment puts target's `addu $t3,$v0,$zero`
ahead of the `lui/addiu` pair. The instruction stream is exact, 36 for 36,
position for position.

### The single remaining frontier — the counter/end-pointer 2-swap
Our outer counter takes $t0 and our inner end pointer takes $t1; target has
counter=$t1, end=$t0. Allocnos 73 (counter, refs 8, live 23, pri 1.0435) and
78 (end pointer, refs 4, live 9, pri 0.888) are adjacent in rank and must be
transposed, i.e. pri(78) must land strictly inside
(pri(73) = 1.0435, pri(75) = 1.350).

Reachable cells (pri = floor_log2(refs)*refs/live_length; out-of-loop refs
weigh 1, in-loop refs weigh 2 per flow.c:434/2081; a
`(set (reg X) (plus (reg X) c))` counts TWO occurrences):
  * refs 6, live_length 9  -> 1.333   (one extra IN-LOOP raw reference, live range unchanged)
  * refs 6, live_length 10 -> 1.200
  * refs 6, live_length 11 -> 1.0909
  * refs 5, live_length 9  -> 1.111   (one extra OUT-OF-LOOP reference, live range unchanged)
  * refs 5, live_length 8  -> 1.250
  * refs 4, live_length 6 or 7        (shorten the live range instead)
Or, symmetrically, lower pri(73) below 0.888 while keeping it above
pri(74) = 0.750: refs 8 needs live_length in (27, 32) against a measured 23;
refs 7 needs live_length in (15.8, 18.7).

The refs-6 row is the one s7 never had access to, and it is much weaker than
the s7 cell: it needs one additional in-loop reference to the end pointer and
tolerates the live range growing by up to two insns. The constraint that
remains hard is that the extra reference must not change any of the 36
instructions. Candidate directions to measure, in order:
  1. Spellings of the inner-loop exit test that mention the end pointer twice
     at RTL level while still folding to the single `slt $v0,$a1,$t0` cc1
     already emits — remember reg_n_refs is computed in flow.c BEFORE combine,
     so an occurrence that combine later folds away still counts. Screen every
     candidate with a disassembly diff first: anything that is not 36
     instructions in target's order is dead on arrival.
  2. Spellings that lengthen the end pointer's live range by 1-2 insns while
     adding an in-loop reference (refs 6 / live 10 or 11 both land inside the
     window), e.g. moving the `var_t0 = var_a3 + 0x15;` definition earlier
     among the four loop-top statements — cc1 currently emits it last, and
     target emits it last too, so any reordering must be checked against the
     stream.
  3. If the extra reference cannot be had without changing an instruction,
     attack pri(73) instead: it is the only other allocno in the pair and its
     window (0.750, 0.888) is reachable at refs 7 with live_length 16-18.
Re-run tmp/grind/func_80037B00/s7/rainfo.sh after EVERY structural change —
every live_length in the table is a function of block boundaries.

## [s8] s7's open caveat — that an added 'outside-the-loop' reference might land at flow.c weight 2 rather than 1, which would make frontier #1's (refs 5, live_length 9) cell unreachable.
- mechanism: GCC 2.7.2 flow.c computes basic_block_loop_depth[] by scanning for NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END with depth initialised to 1 (flow.c:434, 439-443); propagate_block seeds loop_depth from that array (flow.c:1385) and every ref site does reg_n_refs[regno] += loop_depth (flow.c:2081, 2329, 2515, 2725). A reference outside every loop note weighs exactly 1.
- probe: Read the compiler source directly, then verified occurrence-by-occurrence against tmp/grind/func_80037B00/s8/flow.rtl (the .flow RTL slice for the floor-9 form; NOTE_INSN_LOOP_BEG at line 132, NOTE_INSN_LOOP_END at line 286). Allocno 73's refs = init at :100 (weight 1) + duplicated-guard compare at :115 (weight 1) + the plus insn's set AND use at :223/:224 (weight 2 each) + loop test at :274 (weight 2) = 8, matching the .lreg banner exactly. Allocno 78 = set at :151 (2) + slt at :208 (2) = 4.
- result: The caveat resolves in favour of weight 1. s7's hand-count was off because reg_n_refs counts OCCURRENCES, not insns: `(set (reg 73) (plus (reg 73) 1))` contributes two refs. The counter's block-0 init is NOT double-weighted. This is a permanent correction to the RA model used by every future session on this function.
- verdict: KILLED

## [s8] Frontier #2 — target's lui/addiu for D_80102810 inside the guarded preheader requires a giv spelling whose invariant base folds into the induction variable's initial value.
- mechanism: FALSE PREMISE. The `la` does not have to be hoisted by loop.c at all; it only has to be WRITTEN AFTER THE ENTRY GUARD in RTL order. A source-level `if` around the loop puts the guard branch ahead of the `var_a3 = (s8*)&D_80102810;` statement, so cc1 emits lui/addiu inside the guarded preheader for free.
- probe: Built and measured the explicit-if family and disassembled each out of the honest sandbox object: vE (bound cached in a local before the if, do-while) score 11 / 34 insns; vF (bound re-read in the do-while test, init inside the if) score 10 / 35; vF2 (init before the if) score 10 / 34 with lui/addiu in target's position.
- result: CONFIRMED that the guard placement, not strength reduction, controls the la position. s7's two giv spellings were solving a problem that does not exist; both cost +2 for nothing.
- verdict: CONFIRMED

## [s8] An explicit `if (D_800A38C8 > 0)` guard around a top-tested while gives both the 8-byte phantom frame and the correct la position.
- mechanism: The frame comes from loop.c's duplicate_loop_exit_test: the copied exit test leaves an orphaned compare pseudo that reload's alter_reg pays off with a stack slot. An explicit source guard does not suppress that duplication.
- probe: Measured both spellings: init inside the if -> score 10, build_insns 37, cc1 `.frame $sp,8,$31 # vars= 8`; init before the if -> score 14, build_insns 37. Read the cc1 assembly directly (tmp/grind/func_80037B00/s7/raw.s).
- result: Both properties present, but cc1 emits TWO `blez $2,.L40` branches with the la between them (+3 insns) because the source guard's condition and the duplicated exit test are not the same jump for jump2 to merge. Banked rejected/if-while-init-before-if-double-guard.c. This kill is what pointed at the fix.
- verdict: KILLED

## [s8] Writing the entry guard AS the loop's own test — `if (var_t1 < D_800A38C8)` with var_t1 already 0 — collapses the two branches to one while keeping the duplicated exit test that produces the frame orphan.
- mechanism: The source guard and loop.c's duplicated exit test are then the same comparison on the same pseudos, so only one `blez` survives, while duplicate_loop_exit_test still runs and still strands the compare pseudo that alter_reg pays off as the 8-byte frame.
- probe: tmp/grind/func_80037B00/s8/vH_guard_is_loop_test.c: sandbox --disable all -> score=7, build_insns=36, target_insns=36; objdump shows one blez, the frame adjusts present, the la in the guarded preheader, and the blez delay slot filled with the counter init.
- result: CONFIRMED — floor 9 -> 7. The only defect left on that form was that `move t3,v0` landed AFTER `lui/addiu a3`, because on that spelling the copy is duplicate_loop_exit_test's TRANSFER copy, inserted in front of NOTE_INSN_LOOP_BEG and therefore after the la.
- verdict: CONFIRMED

## [s8] Reading the loop bound into a local as a source statement placed BEFORE the `var_a3` assignment puts target's `addu $t3,$v0,$zero` ahead of the lui/addiu pair.
- mechanism: cse sees D_800A38C8 already loaded into the guard's pseudo and turns `var_t3 = D_800A38C8;` into a register copy rather than a second lw; because it is a source statement, it is emitted in source order — before the la — which is target's order. This is a different copy-producing mechanism from s7's (loop.c hoist + cse of an in-test re-read), and it is the one that gets the ORDER right.
- probe: tmp/grind/func_80037B00/s8/vI_count_local_in_if.c (== s8/FLOOR5.c == the new memory/grind/func_80037B00/candidate.c): sandbox --disable all -> score=5, target_insns=36, build_insns=36, cheat_asm_stripped=3, rules_dropped=0; cc1 `.frame $sp,8,$31 # vars= 8`; objdump diffed against asm/funcs/func_80037B00.s.
- result: CONFIRMED — floor 7 -> 5, and the instruction stream is now EXACT position-for-position: every one of the 36 instructions matches target's opcode, operands and order. Note this does NOT contradict the s6 kill of `var_v0 = D_800A38C8; var_t3 = var_v0;` (rejected/split-count-copy-folded-by-copyprop.c): there the copy source was a source-level load that copy propagation folded; here the copy source is the guard's own pseudo.
- verdict: CONFIRMED

## [s8] Frontier #1 as s7 stated it — one extra reference to the end pointer OUTSIDE the loop, at zero instruction cost with live_length frozen at 9.
- mechanism: The arithmetic was sound, but the SITE does not exist. The weight-1 region of this function is block 0, the insns duplicate_loop_exit_test copies in front of NOTE_INSN_LOOP_BEG, and the two return blocks. The end pointer is defined inside the outer loop body and is dead in all of them.
- probe: Enumerated every insn outside the loop notes in tmp/grind/func_80037B00/s8/flow.rtl and checked allocno 78's liveness at each; cross-checked against the 36-instruction stream, where every out-of-loop instruction is one target also has.
- result: KILLED as stated. It is however no longer the only route: on the floor-5 form the window widened to (1.0435, 1.350) and the refs-6 row (one extra IN-LOOP reference, live_length 9, 10 or 11) is newly available.
- verdict: KILLED

## s9 (forensics) — the 73/78 transposition, measured against the dumps

Chassis re-measured at session start: `sandbox --disable all` on candidate.c =
score 5, target_insns 36, build_insns 36, rules_dropped 0. The s8 RA table
reproduces EXACTLY (`tmp/grind/func_80037B00/s9/d_candidate/x.lreg`):
73 = 8 refs / 23 live, 78 = 4 refs / 9 live, greg order
`79 76 77 75 73 83 78 74 82 81 72`, dispositions 73→$8, 78→$9.

### The window, recomputed with global.c's ACTUAL arithmetic
`tools/gcc-2.7.2/global.c:635-656` — `pri = (int)((double)(floor_log2(refs)*refs)
/ live_length * 10000 * size)`, ties broken by `*v1 - *v2` (LOWER allocno wins).
Truncation to int matters: the baseline priorities are the integers
79=30000, 76=27272, 77=24000, 75=13500, **73=10434, 78=8888**, 74=7500,
81=1428, 72=1250. `tmp/grind/func_80037B00/s9/prisolve.py` enumerates every
(refs, live_length) cell that transposes the pair while leaving all other seats
intact; the reachable low-cost rows are
  * 78 at (4,6) / (4,7)      — same refs, live range 2-3 insns SHORTER
  * 78 at (5,8) / (5,9)      — one extra weight-1 (out-of-loop) reference
  * 78 at (6,9) / (6,10) / (6,11) — one extra weight-2 (in-loop) reference,
    live range allowed to grow by up to two insns
  * 73 at (7,16..18) or (8,28..32) — the counter side
Also proved by the same enumeration: adding pre-combine insns that do NOT
reference 78 can never help — 8/(9+N) > 24/(23+N) has no solution for N>0, so
any extra flow-visible insn must MENTION the end pointer.

## [s9] The refs-6 row is reachable by spelling the end pointer's definition or its exit test as a two-insn chain (the "reg_n_refs is computed before combine" route).
- mechanism: reg_n_refs is set by flow.c's life_analysis, which runs BEFORE combine (toplev.c:3004 combine, :3049 local_alloc — life_analysis precedes both), so an occurrence in an insn that combine later folds away was expected to still count.
- probe: vB (`var_t0 = var_a3; var_t0 += 0x15;`) and vA (`if ((s32)var_t0 - (s32)var_a1 > 0)`), both built with tools/grinder/dump.ps1's exact front half into tmp/grind/func_80037B00/s9/d_vB_split_def/ and d_vA_ptrdiff_test/. Traced insn 46/49 through .rtl → .jump → .cse → .loop → .cse2 → .flow.
- result: KILLED, and the pass is named. In vB the initial RTL does carry two insns; **cse1 rewrites `(set 78 (plus 78 21))` into `(set 78 (plus 75 21))`** (visible in x.cse), which makes the copy `(set 78 (reg 75))` dead. The copy still exists as an insn in .loop and .cse2 and is turned into `NOTE_INSN_DELETED` **by the flow pass itself**, i.e. propagate_block's dead-store elimination runs in the same scan that counts refs, so the occurrence is never credited. vA's RA table is byte-identical to the baseline's (73 = 8/23, 78 = 4/9, same greg order and dispositions) — the ptrdiff test folds before flow too. PERMANENT MODEL CORRECTION: "counted before combine" is true only for insns that are LIVE at flow time; anything cse1 makes dead is deleted by flow before counting. The only insn-deleting pass AFTER flow is combine, so an extra reference can only survive if combine (not cse) is what absorbs it — i.e. it must be an insn combine merges INTO its consumer (on MIPS: essentially only an address `(plus reg const)` folded into a `mem`, and target's stream has no memory access based on the end pointer).
- verdict: KILLED

## [s9] A named intermediate copy of the end pointer used in the exit test (`var_e = var_t0; if (var_a1 < var_e)`) adds a flow-visible reference.
- mechanism: same family as above but with the copy CONSUMED rather than overwritten, so it is not a dead store.
- probe: tmp/grind/func_80037B00/s9/vE_copy_into_cmp.c; dumps in d_vE_copy_into_cmp/.
- result: KILLED. cse1 propagates the copy into the compare and the copy dies; the emitted assembly is IDENTICAL to the baseline (same 26 cc1 insns, counter $8 / end $9), the RA banner shows 73 = 8/23 and the same seats. The extra C variable only renumbers pseudos.
- verdict: KILLED

## [s9] cse1's equivalence table reaches every use of the end pointer, so no C-level second reference can survive it.
- mechanism: if true, the refs axis is closed outright.
- probe: vG (`var_a3 = var_t0 + 0x13;` at block_74 instead of `var_a3 += 0x28;` — the end pointer reused to step to the next 0x28-byte entry). tmp/grind/func_80037B00/s9/vG_reuse_end_for_next.c, dumps in d_vG_reuse_end_for_next/.
- result: KILLED — and this is the session's positive find. The reference SURVIVES: allocno 78 measures **6 refs / 14 live** (up from 4/9), exactly the +2 weight-2 credit the flow.c model predicts, so cse1 does NOT fold `var_t0 + 0x13` back into `var_a3 + 0x28` across the intervening blocks. The form is nevertheless dead as a candidate: pri(78) = 2*6/14 = 8571 is still below pri(73) = 10434 because the live range now stretches to block_74, and the reuse also collapses var_a3 (allocno 75 → 5 refs / 6 live) and drops the stream to 25 cc1 insns. What it proves is the SITE constraint, not the mechanism: an extra in-loop reference is available in ordinary C, but it must sit close enough to the last use (the `slt`) that live_length stays ≤ 11.
- verdict: KILLED (as a candidate) / CONFIRMED (as the mechanism)

## [s9] Declaration order controls pseudo numbering on the floor-5 chassis, so the tie-break can be pointed at the end pointer for free.
- mechanism: global.c:655 breaks an exact priority tie with `*v1 - *v2` (lower allocno first); allocnos are numbered in pseudo order and pseudos in C declaration order.
- probe: vC (declare `s8 *var_t0;` before `s32 var_t1;`), tmp/grind/func_80037B00/s9/vC_declorder.c, banked as memory/grind/func_80037B00/decl_order_swap_vC.c.
- result: CONFIRMED and INERT-at-score. The end pointer becomes allocno 73 (4 refs / 9 live) and the counter 74 (8 refs / 23 live); the priorities and therefore the seats are unchanged ($8 counter, $9 end). The lever is precondition-only: it costs nothing and converts the required window from strict `pri(end) > pri(counter)` to `pri(end) >= pri(counter)`. Apply it only together with whatever moves a priority — on its own it changes nothing. NOTE the tie enumeration found exactly one tie cell (end pointer at 8 refs / 23 live, pri 10434), which is not plausibly reachable.
- verdict: CONFIRMED (mechanism), INERT (score)

## [s9] The refs-6 row for allocno 78 (end pointer) is reachable by spelling its definition or its exit test as a two-insn chain, because reg_n_refs is computed in flow.c before combine.
- mechanism: toplev.c runs life_analysis (flow) before combine_instructions (toplev.c:3004) and before local_alloc (toplev.c:3049), so an occurrence in an insn combine later folds away was expected to still be credited by flow.c's reg_n_refs accumulation.
- probe: Built vB (var_t0 = var_a3; var_t0 += 0x15;) and vA (exit test as (s32)var_t0 - (s32)var_a1 > 0) through the project's exact cpp|cc1 -da front half; traced the insns through .rtl -> .jump -> .cse -> .loop -> .cse2 -> .flow in tmp/grind/func_80037B00/s9/d_vB_split_def/ and d_vA_ptrdiff_test/.
- result: The initial RTL does carry two insns, but cse1 rewrites (set 78 (plus 78 21)) to (set 78 (plus 75 21)); the resulting dead copy survives loop and cse2 unchanged and is turned into NOTE_INSN_DELETED by the FLOW pass itself. propagate_block's dead-insn deletion and its ref counting are the same scan, so the occurrence is never credited: the RA banner is byte-identical to the baseline (73 = 8 refs/23 live, 78 = 4/9, same greg order, same seats). vA's RA table is identical to the baseline as well.
- verdict: KILLED

## [s9] A named intermediate copy of the end pointer that is CONSUMED by the exit test (var_e = var_t0; if (var_a1 < var_e)) survives to flow and adds the missing reference.
- mechanism: A consumed copy is not a dead store, so flow's dead-insn deletion should not remove it; combine would then fold the copy into the slt at zero instruction cost.
- probe: tmp/grind/func_80037B00/s9/vE_copy_into_cmp.c, dumps in tmp/grind/func_80037B00/s9/d_vE_copy_into_cmp/; compared the cc1 assembly and the .lreg banner against the baseline.
- result: cse1 propagates the copy into the compare before flow runs, so the copy is dead again and is deleted. Emitted assembly identical to the baseline (counter $8, end pointer $9); the extra C variable only renumbers pseudos. No refs change.
- verdict: KILLED

## [s9] cse1's equivalence table reaches every use of the end pointer, so no C-level second reference to it can survive into flow's count.
- mechanism: If cse1 always folds a re-mention back onto var_a3, the whole refs axis is closed and only the live-length axis remains.
- probe: vG: replaced 'var_a3 += 0x28;' at block_74 with 'var_a3 = var_t0 + 0x13;' (the end pointer reused to step to the next 0x28-byte entry). tmp/grind/func_80037B00/s9/vG_reuse_end_for_next.c, dumps in tmp/grind/func_80037B00/s9/d_vG_reuse_end_for_next/.
- result: FALSE as stated. The reference survives: allocno 78 measures 6 refs / 14 live, exactly the +2 weight-2 credit flow.c's model predicts, so cse1 does not fold var_t0 + 0x13 back into var_a3 + 0x28 across the intervening blocks. The form is still not a candidate (pri = 2*6/14 = 8571 < pri(73) = 10434 because the live range now reaches block_74; var_a3 collapses to 5 refs/6 live and the stream drops to 25 cc1 insns), but the mechanism is confirmed and the constraint is now distance, not cse.
- verdict: CONFIRMED

## [s9] Declaration order controls pseudo numbering on the floor-5 chassis, so global.c's tie-break can be pointed at the end pointer for free.
- mechanism: global.c:655 breaks an exact priority tie with *v1 - *v2 (lower allocno wins); allocnos are numbered in pseudo order and pseudos follow C declaration order.
- probe: vC: declared 's8 *var_t0;' before 's32 var_t1;'. tmp/grind/func_80037B00/s9/vC_declorder.c; banked as memory/grind/func_80037B00/decl_order_swap_vC.c.
- result: Confirmed: the end pointer becomes allocno 73 (4 refs/9 live) and the counter 74 (8 refs/23 live) with every priority and every seat unchanged. The lever is precondition-only and costs nothing; it converts the required window from pri(end) > pri(counter) to pri(end) >= pri(counter). The tie enumeration finds exactly one tie cell (end pointer at 8 refs/23 live) which is not plausibly reachable, so this must be paired with a priority-moving change.
- verdict: CONFIRMED

## [s9] An extra flow-visible insn inside the inner loop that does not itself mention the end pointer can transpose the pair by lengthening the counter's live range.
- mechanism: Both live ranges grow by the same N, and pri(78) = 8/(9+N) must exceed pri(73) = 24/(23+N).
- probe: Solved the inequality over the measured baseline inputs in tmp/grind/func_80037B00/s9/prisolve.py together with the full (refs, live_length) cell enumeration.
- result: 8(23+N) > 24(9+N) reduces to -32 > 16N, which has no positive solution. Any perturbation that reaches the window MUST add a reference to allocno 78 itself. This closes an entire class of 'add a harmless insn' spellings without measuring them.
- verdict: KILLED

## s10 addendum (rederive, 2026-08-26) — FRONTIER REPLACED

The s9 frontier (#1 "raise allocno 78 to 6 refs from block_5c/block_6c", #2 "shorten 78's
live_length to 6 or 7", #3 "find a new 36-insn chassis") is superseded. #1 and #2 are KILLED by
measurement + a stream-level invariance argument; #3 is narrowed to a single arithmetic target.

### KILLED — s9 frontier #1 (raise refs(78) to 6)
`$t0` occurs in exactly two of target's 36 instructions (its `addiu $t0,$a3,0x15` definition and
the `slt $v0,$a1,$t0` loop test). A third occurrence must either materialise a 37th instruction,
respell an existing instruction to name $t0 while keeping its encoding (impossible — no other
target insn mentions $t0), or ride an insn that dies before flow counts it (s9 proved flow.c
deletes cse-dead insns in the same scan that accumulates reg_n_refs). refs(78) = 4 is therefore a
property of the target stream, not of our spelling.

### KILLED — s9 frontier #2 (shorten live_length(78) to 6 or 7)
78's pre-combine span (s9/flow.rtl insns 46..82) is exactly the inner loop: def, QI load,
zero_extend, branch_zero, QI load, zero_extend, branch_equality, `a1 += 1`, `a2 += 1`, `slt`.
Each of those ten insns is mandated by the 36-instruction stream. Three probes confirmed the span
will not move: `u8 *` pointer retyping (inert), `u8` scalar retyping (inert), and relocating
`var_a2 += 1` into the loop-back arm (stream unchanged, but BOTH ranges grow +1). live(78) = 9 is
therefore also a property of the target stream.

### THE WHOLE RESIDUAL, IN ONE LINE
pri(78) = 8888 is fixed. refs(73) = 8 is fixed by the stream. Closing the function requires
`live_length(73) >= 27` (27 ties at 8888 and is won by the decl-order swap in
`memory/grind/func_80037B00/decl_order_swap_vC.c`; 28..31 wins outright; 32 loses the flag
allocno 74 at 7500). live(73) is 23 today, so the ask is **+4 flow-visible insns that emit no
bytes**, in a region where 73 is live and 78 is not.

### THE REGION MAP (measured this session)
| region | +1 to live(73)? | byte cost | probe |
|---|---|---|---|
| preheader (between `var_t1 = 0` and the loop head) | YES | 0, combine folds the pair | rejected/preheader-foldable-chain-plus1-only.c |
| block_5c | YES | +1 insn — combine cannot fold across the block edge | s10/vG1_stride_at_5c.c |
| block_74 | NO (flow.c:1685 only counts regs in `regs_sometimes_live`; 73 never changes state there) | n/a | rejected/stride-split-block74-no-counter-live-change.c |
| inner-loop arm | YES but also +1 to live(78) — net worse | 0 | rejected/a2-inc-in-taken-arm-lengthens-both-ranges.c |

### AND WHY THE +1 DOES NOT STACK
A four-link constant chain on a fresh preheader temp buys +1, not +4: cse1 constant-folds the
chain before flow runs and flow deletes the dead links, leaving a 5-insn residue pseudo
(rejected/preheader-const-chain4-cse1-collapses.c, counter still 8/24 = pri 10000). The surviving
`+0x14 / -0x14` pair works only because cse1 does not fold its two links into one another.

### s10 FRONTIER (for the next session)
1. **A 36-instruction chassis whose PREHEADER is naturally four pre-combine insns longer.**
   The preheader today is two insns (`addu $t3,$v0,$zero`, the `la`). Any source shape that keeps
   the emitted stream at exactly 36 in target's order but expands the preheader's pre-combine RTL
   by four LIVE, combine-absorbed insns closes the function. Mechanism: flow.c:1685 counts
   preheader insns for the counter (measured +1) and never for the end pointer (78 is not live
   there). Next probe: enumerate expression spellings of the preheader's two statements whose
   expansion is multi-insn and whose extra insns are absorbed by combine rather than deleted by
   flow — e.g. address computations GCC expands through an intermediate (`&D_80102810` reached via
   a subscript/member expression, a `(s8 *)((s32)X + k)` round trip, a bound read through a
   narrower type that widens). Screen with tmp/grind/func_80037B00/s10/probe.sh: accept only rows
   where the counter shows `used 8 times across >= 27 insns`, the end pointer still shows
   `used 4 times across 9 insns`, and the cc1 insn count is still 26. POLICY BOUNDARY: the extra
   insns must be the natural expansion of a statement the function genuinely needs. A chain
   written purely to lengthen a live range is semantically purposeless (T1/T2/T3 FAIL) and at best
   the F1 combine-foldable chain-extender family, which this session measured caps at +1 anyway.
2. **A 36-instruction chassis with a THIRD outer-loop block in which the counter changes state.**
   flow.c:1685's `regs_sometimes_live` gate is why block_74 is free: the counter neither starts nor
   stops being live there. A chassis in which the counter's liveness CHANGES inside an
   after-the-inner-loop block would make that block's insns count. Next probe: re-read the s8
   chassis table in this file, then look for a spelling where the counter is dead on the `return 1`
   arm and re-live at block_74 (or vice versa) — flow would then count the return-arm insns
   (s9/flow.rtl insns 105, 107) and the epilogue (133, 147) for it, which is +4 at zero byte cost
   and is exactly the required delta. Treat any form that is not 36 instructions in target's order
   as dead on arrival.
3. **If neither lands, this function is a decision packet, not a grind.** Both remaining knobs are
   properties of GCC's flow pass rather than of the program, and the only measured lever that moves
   them at zero byte cost is a purposeless one. The decidable question for the owner would be a
   FIDELITY/ROUTING question (is a 36-of-36 exact stream with a single adjacent register
   transposition a COMPLETED-C candidate under some named disposition?), NOT a family grant or a
   standard-lowering ask — the latter is the pre-decided AUTO-REJECT class. Note for whoever writes
   it: the same-file sibling func_80037AA4 closed an equivalent RA residual with a Judge-sanctioned
   `/* FAKE */` constant-holder on 2026-07-28, but that precedent does NOT transfer, because no
   constant-holder changes either of this function's two pinned quantities.

## [s10] A sibling/Kengo rederive yields a structurally different C shape for func_80037B00.
- mechanism: func_80037A20 and func_80037AA4 (both MATCHED, both in src/code6cac_c.c) walk the same table D_80102810 with the same bound global D_800A38C8, so their accepted C shows the original author's idiom for this loop; kengo_matches.csv proposes ki_every_little_thing as a transplant source.
- probe: Read src/code6cac_c.c:268-316 (both matched siblings) and grep kengo_matches.csv for all three functions.
- result: The siblings' accepted shape is `counter = 0; bound = D_800A38C8; if (counter < bound) { p = (s8*)&D_80102810; do {...; counter += 1; p += 0x28; } while (counter < bound); }` - i.e. candidate.c's floor-5 chassis IS the house idiom, not an artefact of our search. No structurally different shape is available from the siblings. Kengo is size-only-ambiguous (36 vs 36 insns, similarity 0.00/0.00/0.00/0.42) for all three functions - no transplant.
- verdict: KILLED

## [s10] s9 frontier #1: an ordinary-C reference to the end pointer in block_5c/block_6c raises allocno 78 to 6 refs at zero byte cost, landing pri(78) inside (10434, 13500).
- mechanism: flow.c:434/2081 weight in-loop occurrences 2; a cross-block re-mention is not folded by cse1 (s9's vG measured 78 at 6 refs).
- probe: Enumerate the target stream's uses of $t0 and cross-check against s9's proof that flow deletes cse-dead insns in the same scan that accumulates reg_n_refs.
- result: $t0 appears in exactly TWO of target's 36 instructions - its definition `addiu $t0,$a3,0x15` and the loop test `slt $v0,$a1,$t0` - one occurrence each, both weight 2, total 4. A third occurrence must (i) materialise a 37th instruction, (ii) respell an existing instruction to name $t0 with an unchanged encoding (no such instruction exists), or (iii) ride an insn deleted before flow counts it (s9 proved that is never credited). refs(78)=4 is a property of the TARGET STREAM, not of our spelling.
- verdict: KILLED

## [s10] s9 frontier #2: the end pointer's live_length can be shortened from 9 to 6 or 7 by emitting its definition later in the pre-combine order while sched1 restores target's stream.
- mechanism: reg_live_length is computed by flow on the pre-combine, pre-sched stream, so it measures cc1's RTL emission order rather than the final scheduled order.
- probe: Counted 78's span in tmp/grind/func_80037B00/s9/flow.rtl (insn 46 def .. insn 82 slt), then measured three spellings that could plausibly shorten it with tmp/grind/func_80037B00/s10/probe.sh.
- result: The span is exactly the inner loop's mandated pre-combine chain: def, QI load, zero_extend, branch_zero, QI load, zero_extend, branch_equality, `a1 += 1`, `a2 += 1`, `slt` - ten insns, every one required by the 36-instruction stream. Retyping the four pointers to `u8 *` and dropping the casts is RA-INERT (78 still 4/9, table byte-identical). Retyping var_v1/var_v0 to `u8` is RA-INERT (78 still 4/9) - so the QI-load+zero_extend split is not a product of the (u8) cast. Relocating `var_a2 += 1` into the loop-back arm leaves the emitted stream unchanged but costs BOTH ranges +1 (73 -> 8/24, 78 -> 4/10 = pri 8000), strictly worse. live(78)=9 is also a property of the target stream.
- verdict: KILLED

## [s10] There exists a region where an added flow-visible insn lengthens the counter's live range (allocno 73) WITHOUT lengthening the end pointer's, at zero emitted-byte cost.
- mechanism: flow.c:1685 increments reg_live_length only for registers in regs_sometimes_live; 78 is not live outside the inner loop, and combine folds an adjacent `+k / -k` pair back into one instruction after flow has already counted both.
- probe: Three splices measured with tmp/grind/func_80037B00/s10/probe.sh: a foldable `+0x14 / -0x14` pair in the preheader; the 0x28 stride split half at block_5c and half at block_74; the same stride split entirely inside block_74.
- result: CONFIRMED for the preheader: `var_a3 = (s8 *)&D_80102810 + 0x14; var_a3 -= 0x14;` leaves the cc1 stream unchanged at 26 insns (combine folds the pair) while the counter goes 23 -> 24 live and the end pointer stays 4/9. This is the first measured zero-byte-cost +1 on the counter axis in this function's history. CONFIRMED for block_5c but at +1 emitted insn (combine cannot fold across the block edge). REFUTED for block_74: allocno 75 goes 9 -> 13 refs (so both insns ARE live and ARE counted for 75) yet the counter's live_length does not move - 73 never changes liveness state inside block_74, so flow.c:1685's regs_sometimes_live gate skips it. This is a permanent correction to the naive 'live_length = insn span' model used in s7-s9.
- verdict: CONFIRMED

## [s10] The measured +1 preheader mechanism can be stacked to the +4 the arithmetic requires (live(73) >= 27).
- mechanism: If combine folds a chain of constant adds after flow has counted every link, N links should buy N-1 extra live insns for the counter at zero byte cost.
- probe: Built the maximal form - a fresh pointer temp carrying a four-link constant chain in the preheader plus the s9 decl-order swap - and read its .lreg table (rejected/preheader-const-chain4-cse1-collapses.c).
- result: Buys only +1, not +4: the counter measures 8 refs / 24 live (pri 10000, still above the end pointer's 8888) and a 5-insn residue pseudo (3 refs / 5 live) appears. cse1 constant-folds the chain BEFORE flow runs and flow deletes the dead links, exactly as s9's fact 4 predicts. The surviving `+0x14 / -0x14` pair works only because cse1 does not fold its two links into one another's operand; a longer chain is folded. The +1 is not a repeatable unit.
- verdict: KILLED

## [s11] The outer loop's `while` form is load-bearing for the phantom 8-byte frame (s8 attribution).
- mechanism: s8 attributed the frame to loop.c's `duplicate_loop_exit_test` copying the while's exit
  test in front of NOTE_INSN_LOOP_BEG, combine folding the duplicate's compare into a bare `blez` and
  leaving the compare pseudo referenced-but-homeless for reload's `alter_reg`.
- probe: `tmp/grind/func_80037B00/s11/v6_dowhile_t1guard.c` — candidate.c with the outer `while
  (var_t1 < var_t3) { ... }` rewritten as `do { ... } while (var_t1 < var_t3);`, everything else
  byte-identical. Banked as `memory/grind/func_80037B00/alt_base_v6_dowhile.c`.
- result: REFUTED. v6 is CODEGEN-IDENTICAL to candidate.c — 26 cc1 insns, `subu $sp,$sp,8` /
  `addu $sp,$sp,8` both present, the same `;; 11 regs to allocate: 79 76 77 75 73 83 78 74 82 81 72`,
  the same `73 in 8  78 in 9`. The frame comes from the GUARD, not from the loop form. The orphan is
  visible in `tmp/grind/func_80037B00/s11/d_v6_dowhile_t1guard/x.flow` as
  `(insn 16 ... (set (reg:SI 83) (lt:SI (reg/v:SI 73) ...)))` feeding `(if_then_else (eq (reg 83) 0))`;
  reg 83 is gone from `x.combine` entirely, so it reaches local-alloc with flow's stale refs (2 refs /
  2 insns, class ST_REGS), takes no hard reg, and `alter_reg` pays it a 4-byte slot that
  MIPS_STACK_ALIGN rounds to 8. THE OUTER LOOP FORM IS FREE. This retires the s8 claim and means the
  house-idiom `do { } while` of the matched siblings func_80037A20 / func_80037AA4 is usable.
- verdict: KILLED (the attribution), CONFIRMED (the do-while chassis is equivalent)

## [s11] refs(allocno 73) = 8 is a property of the target instruction stream.
- mechanism: s10 argued every quantity in the priority formula is pinned by the 36-insn stream.
- probe: measured refs(73) across guard spellings on an otherwise identical do-while chassis.
- result: REFUTED, and this is the session's central finding. EXACTLY ONE of the 8 references comes
  from the source guard naming the counter. `if (var_t1 < D_800A38C8)` -> refs(73) = 8
  (v6, v7, candidate). `if (D_800A38C8 > 0)` / `if (D_800A38C8 >= 1)` / `if (D_800A38C8 < 1) return 0;`
  / `if (var_t3 > 0)` -> refs(73) = 7 (v3, v5, v10, v11, v13, v14). The other seven are the stream's:
  init (weight 1) + `var_t1 += 1` (set+use, weight 2 each = 4) + the bottom `slt` (weight 2).
  A `while` outer loop re-adds the eighth even with a counter-free source guard, because loop.c's
  duplicated exit test names the counter (v1: `while` + `D_800A38C8 > 0` guard = refs 8, 27 insns,
  two `blez`).
- verdict: KILLED (the pinning claim)

## [s11] refs(73) = 7 crosses a floor_log2 boundary and SOLVES the $t0/$t1 transposition that blocked s7-s10.
- mechanism: global.c priority = floor_log2(refs) * refs / live_length. refs 8 -> floor_log2 3 ->
  24/live; refs 7 -> floor_log2 2 -> 14/live. At live(73) = 20-23 that is a drop from ~1.04 to ~0.65,
  which is below pri(78) = 8/9 = 0.888 with room to spare.
- probe: `tmp/grind/func_80037B00/s11/v3_dowhile_t3.c`, `v5_init_inside_last.c`, `v8_a3_before_flag.c`,
  `v10..v18` — all measured with `tmp/grind/func_80037B00/s11/probe.sh`.
- result: CONFIRMED. Every refs-7 form allocates the END POINTER FIRST:
  `;; 10 regs to allocate: 79 76 77 75 78 74 73 81 72` with `78 in 8` — i.e. the inner-loop end
  pointer takes $t0 exactly as target does. The three-session wall (s7 "typed RA residual", s9/s10
  "refs(78)=4 and live(78)=9 are pinned, so live(73) must reach 28..31") is dissolved from the other
  side of the formula: the fix was never to move 78, it was to stop the guard from naming 73.
- verdict: CONFIRMED

## [s11] A guard that does not name the counter can still produce the orphan compare pseudo (frame).
- mechanism: the orphan is a pre-combine `(set (reg N) (lt (reg A) (reg B)))` whose `(ne (reg N) 0)`
  branch combine rewrites into a bare `blez`. Any two-register signed `lt` whose left operand cse can
  later prove is 0 should do it.
- probe: four counter-free guard spellings measured for `subu $sp,$sp,8` and the cc1 insn count —
  `if (D_800A38C8 > 0)` (v3/v5), `if (var_t3 > 0)` with the bound read first (v10),
  `if (D_800A38C8 >= 1)` (v13), `if (D_800A38C8 < 1) { return 0; }` (v14).
- result: KILLED. All four measure 23-24 cc1 insns and NO frame adjust. GCC canonicalises every
  bound-vs-constant guard at expand time into MIPS `branch_zero` / `bgtz` and never materialises a
  compare pseudo; `< 1` is folded to `<= 0` before expand, so the hoped-for `slti + bne -> blez`
  combine never happens. The only natural zero-valued REGISTER in this function is the counter, so
  every counter-free guard costs the 2 frame instructions. THIS IS THE NEW RESIDUAL: the guard must
  name the counter to buy the frame, and must not name it to buy the register order.
- verdict: KILLED

## [s11] In the refs-7 family, the remaining order defect (counter 73 vs match-flag 74) is reachable.
- mechanism: target needs 73 before 74 ($t1 then $t2). refs(73)=7, refs(74)=6, both floor_log2 2, so
  the condition is 14/L73 >= 12/L74, i.e. 7*L74 >= 6*L73 (an exact tie is won by 73, the lower allocno,
  via global.c:655).
- probe: measured (L73, L74) over eight refs-7 spellings.
- result: MEASURED BUT NOT YET REACHED. v5 (20,16) -> 112 < 120. v8 `var_a3 += 0x28` moved ahead of
  the flag test (20,17) -> 119 < 120, ONE UNIT SHORT, and it costs +1 emitted insn because the bottom
  `bnez`'s delay slot loses its filler. v15 counter-increment after the flag test (20,15) -> worse.
  v11 early-return guard, v13/v14 guard respellings, v17/v18 shared-end-label: all (20,16) or (20,17).
  L73 = 20 is rigid: 73 is live over the whole outer-loop body, and the three insns where 73 is live
  and 74 is not (the counter's def, the bottom `slt`, the bottom `bnez`) are all mandated by the
  stream. Reachable targets are (20,18), (19,17) or (21,18).
- verdict: KILLED for the eight spellings measured; the axis itself is OPEN

## [s11] s10's block_74 result re-confirmed on the 26-insn chassis.
- probe: v19 — candidate/v6 with `var_a3 += 0x28` split into `var_a3 += 0x14; var_a3 += 0x14;` at
  block_74 (`memory/grind/func_80037B00/rejected/block74-foldable-pair-no-live-change.c`).
- result: L73 stays 23, L74 stays 16, refs(73) stays 8, stream stays 26 insns with the frame.
  Insns added at block_74 buy nothing for the counter, exactly as s10 measured. Independent of the
  regs_sometimes_live explanation s10 offered, the empirical rule holds on this chassis too.
- verdict: CONFIRMED (s10's measurement)

## [s11] The outer loop's `while` form is load-bearing for the phantom 8-byte frame (s8's attribution: loop.c's duplicate_loop_exit_test + combine folding the duplicate's compare).
- mechanism: loop.c copies a while's exit test in front of NOTE_INSN_LOOP_BEG; combine folds the duplicate into a bare blez, leaving the compare pseudo referenced-but-homeless for reload's alter_reg, which pays it a 4-byte slot that MIPS_STACK_ALIGN rounds to 8.
- probe: tmp/grind/func_80037B00/s11/v6_dowhile_t1guard.c - candidate.c with the outer `while (var_t1 < var_t3) {...}` rewritten as `do {...} while (var_t1 < var_t3);` and nothing else changed; banked as memory/grind/func_80037B00/alt_base_v6_dowhile.c.
- result: REFUTED. v6 is codegen-identical to candidate.c: 26 cc1 insns, `subu $sp,$sp,8` and `addu $sp,$sp,8` both present, identical `;; 11 regs to allocate: 79 76 77 75 73 83 78 74 82 81 72`, identical seats `73 in 8  78 in 9`. The orphan is visible in tmp/grind/func_80037B00/s11/d_v6_dowhile_t1guard/x.flow as `(insn 16 (set (reg:SI 83) (lt:SI (reg/v:SI 73) ...)))` feeding `(if_then_else (eq (reg 83) 0))`; reg 83 is absent from x.combine, reaches local-alloc with flow's stale 2 refs / 2 insns in class ST_REGS, takes no hard register, and alter_reg gives it the slot. The frame comes from the SOURCE GUARD, not from the loop form. The do-while house idiom of the matched siblings func_80037A20 / func_80037AA4 is therefore free to use.
- verdict: KILLED

## [s11] refs(allocno 73) = 8 is a property of the target instruction stream and cannot be changed by spelling (s10's central claim).
- mechanism: s10 argued every input to the priority formula is pinned by the 36-instruction stream, so live(73) was the sole degree of freedom.
- probe: Measured refs(73) across guard spellings on an otherwise byte-identical do-while chassis with tmp/grind/func_80037B00/s11/probe.sh: v6/v7/candidate (`if (var_t1 < D_800A38C8)` / `if (var_t1 < var_t3)`) vs v3/v5/v10/v11/v13/v14 (`if (D_800A38C8 > 0)`, `if (var_t3 > 0)`, `if (D_800A38C8 >= 1)`, `if (D_800A38C8 < 1) return 0;`).
- result: REFUTED. Exactly one of the eight references is the source guard naming the counter. Counter-naming guard -> refs(73)=8; every counter-free guard -> refs(73)=7. The other seven are stream-mandated: init (weight 1) + `var_t1 += 1` (set+use, weight 2 each = 4) + the bottom `slt` (weight 2). A `while` outer loop re-adds the eighth even with a counter-free source guard, because loop.c's duplicated exit test names the counter (v1: while + `D_800A38C8 > 0` = refs 8, 27 insns, two blez).
- verdict: KILLED

## [s11] Dropping the guard's reference to the counter (refs 7) crosses a floor_log2 boundary and seats the inner-loop end pointer in $t0, solving the $t0/$t1 transposition that s7-s10 reported as the entire residual.
- mechanism: global.c priority = floor_log2(refs) * refs / live_length. refs 8 -> floor_log2 3 -> 24/live; refs 7 -> floor_log2 2 -> 14/live. At live(73)=20..23 that is a drop from ~1.04 to ~0.65, comfortably below pri(78) = 8/9 = 0.888, so allocno 78 is now allocated first.
- probe: tmp/grind/func_80037B00/s11/v3_dowhile_t3.c, v5_init_inside_last.c, v8_a3_before_flag.c, v10..v18 - .lreg/.greg read for every one.
- result: CONFIRMED. Every refs-7 form prints `;; 10 regs to allocate: 79 76 77 75 78 74 ... 73 ...` with `78 in 8` - i.e. the end pointer takes $t0 exactly as target does (`addiu $t0,$a3,0x15` / `slt $v0,$a1,$t0`). The three-session wall is dissolved from the other side of the formula: the fix was never to move allocno 78, it was to stop the guard from naming allocno 73. v5's sandbox score is 8 (build_insns 35 vs target 36) because the frame pair is gone, but it is the first form in this function's history whose end pointer lands in $t0.
- verdict: CONFIRMED

## [s11] A guard that does not name the counter can still materialise the orphan compare pseudo, so the refs-7 chassis can keep the phantom 8-byte frame.
- mechanism: The orphan is a pre-combine `(set (reg N) (lt (reg A) (reg B)))` whose `(ne (reg N) 0)` branch combine rewrites into a bare blez. Any two-register signed `lt` whose left operand cse later proves is 0 should orphan the same way; `x < 1` in particular should expand to `slti` into a pseudo plus `bne`, which combine can fold to blez.
- probe: Four counter-free guard spellings measured for `subu $sp,$sp,8` and cc1 insn count: `if (D_800A38C8 > 0)` (v3/v5), `if (var_t3 > 0)` with the bound read into a local first (v10), `if (D_800A38C8 >= 1)` (v13), `if (D_800A38C8 < 1) { return 0; }` (v14).
- result: KILLED. All four measure 23-24 cc1 insns with NO frame adjust and no ST_REGS orphan in .lreg. GCC canonicalises every bound-vs-constant guard at expand time into MIPS branch_zero/bgtz and never materialises a compare pseudo; `< 1` is folded to `<= 0` before expand, so the hoped-for slti+bne -> blez combine never runs. The only natural zero-valued REGISTER in this function is the counter itself, so a counter-free guard always costs the 2 frame instructions.
- verdict: KILLED

## [s11] In the refs-7 family the remaining order defect (counter 73 must precede match-flag 74, i.e. $t1 then $t2) is reachable by ordinary statement placement.
- mechanism: refs(73)=7 and refs(74)=6 both have floor_log2 2, so the condition is exactly 7*L74 >= 6*L73, with an exact tie won by 73 as the lower allocno (global.c:655).
- probe: Measured (L73, L74) over eight refs-7 spellings: v5 baseline, v8 (`var_a3 += 0x28` moved ahead of the flag test), v11 (early-return guard), v13/v14 (guard respellings), v15/v16 (counter increment moved after the flag test), v17/v18 (shared end label for the `return 1` arm).
- result: NOT REACHED, and bounded. v5 (20,16) = 112 vs 120. v8 (20,17) = 119 vs 120 - ONE UNIT SHORT - and it costs +1 emitted instruction because the bottom `bnez` loses its delay-slot filler. v15 (20,15) is worse. Everything else measures (20,16). L73 = 20 is rigid: the counter is live over the whole outer-loop body and the only three insns where 73 is live and 74 is not are the counter's definition and the bottom `slt`/`bnez`, all mandated by the stream. Reachable cells are (20,18), (19,17) and (21,18).
- verdict: KILLED

## [s11] s10's block_74 result (insns added after the inner loop buy nothing for the counter's live length) holds on the 26-instruction chassis too.
- mechanism: s10 attributed it to flow.c:1685's regs_sometimes_live gate; whatever the explanation, the empirical rule is what matters for search pruning.
- probe: v19 - candidate/v6 with `var_a3 += 0x28` split into `var_a3 += 0x14; var_a3 += 0x14;` at block_74 (memory/grind/func_80037B00/rejected/block74-foldable-pair-no-live-change.c).
- result: CONFIRMED. L73 stays 23, L74 stays 16, refs(73) stays 8, the stream stays 26 insns with both frame adjusts. Insns placed at block_74 remain free of charge for the counter.
- verdict: CONFIRMED

## [s12] CHASSIS RE-MEASURED + the two chassis unified into ONE arithmetic target.
Session start: `sandbox --disable all` on candidate.c = score 5 / 36 vs 36 (unchanged, chassis
intact). v5 (the s11 refs-7 form) re-measured at score 8 / build_insns 35: its ONLY defects are
the two missing `addiu $sp,$sp,+-8` and the counter/flag seats. Its cc1 asm (banked at
tmp/grind/func_80037B00/s12/d_v5_init_inside_last/x.s) is target's stream position-for-position
otherwise, with end=$8($t0) as target wants, flag=$9, counter=$10 (target: counter=$9, flag=$10).

**The unified rule (new - supersedes s11's cell list).** Write gap = L73 - L74. global.c's
`pri = floor_log2(refs)*refs/live_length`, ties to the LOWER allocno (global.c:655).
  * refs-7 chassis (counter-free guard, NO frame): need pri(73) >= pri(74), i.e. 14/L73 >= 12/L74,
    i.e. **L73 >= 7*gap**; and pri(73) < pri(78)=8888, i.e. L73 >= 16. Measured gaps: v5 gap 4
    (20,16) -> needs L73 >= 28; v8 gap 3 (20,17) -> needs L73 >= 21; wD gap 7 (23,16) -> needs 49.
  * refs-8 chassis (counter-naming guard, frame present): the ONLY wrong inequality is
    pri(73)=24/L73 < pri(78)=8/9, i.e. **L73 in [28,32]** (74 is already last at 7500).
  Both chassis therefore reduce to the SAME quantity: **L73 must reach 28** (it is 23 in the
  refs-8 chassis, 20 in the refs-7 chassis) - refs-8 needs the added insns OUTSIDE the flag's live
  range (preheader, or loop-top before `var_t2 = 0`), refs-7 needs them INSIDE it. This is the one
  number the next session should attack; everything else in the priority formula is measured pinned.

## [s12] A guard comparison naming a zero-valued local OTHER than the loop counter also orphans a compare pseudo and pays the phantom 8-byte frame (s11 frontier #1, the "any GR-class combine-deleted pseudo" claim).
- mechanism: s11 proposed that the frame is paid by any pseudo flow counts and combine then deletes,
  not specifically by the counter's compare, and nominated address intermediates and a second-pointer
  end-pointer computation as the shapes to try.
- probe: two minimal spellings on the refs-7 do-while chassis, dumped with
  `tmp/grind/func_80037B00/s12/probe.sh` (cpp | cc1 -da, .lreg/.greg/.combine/.flow read):
  wC (`memory/grind/func_80037B00/rejected/invented-zero-local-guard-no-orphan-no-frame.c`) - a fresh
  local `var_z = 0; if (var_z < D_800A38C8)`; and wA
  (`rejected/flag-reuse-as-guard-zero-refs74-8-no-frame.c`) - the EXISTING match flag borrowed as the
  guard's zero (`var_t2 = 0; if (var_t2 < D_800A38C8)`).
- result: KILLED, and the pass-level reason is now in hand. At FLOW time all three spellings are
  structurally identical: `(insn 11 (set (reg X) (const_int 0)))`, `(insn 14 (set (reg B) (mem
  D_800A38C8)))`, `(insn 16 (set (reg C) (lt (reg X) (reg B))))`, `(jump_insn 17 (if_then_else (eq
  (reg C) 0) ...))` - the compare pseudo C exists in every case. What differs is what COMBINE does
  with insn 16. When X is the loop counter (still live after the guard), combine rewrites the branch
  to `(le (reg B) 0)` and RE-SITES the counter's init INTO insn 16's slot
  (`(insn 16 (set (reg/v 73) (const_int 0)))`, insn 11 -> NOTE), so pseudo C is left referenced-only
  in flow's stale tables, reaches local-alloc as `ST_REGS or none`, gets no hard register and
  `alter_reg` pays it the 4-byte slot MIPS_STACK_ALIGN rounds to 8 (`vars= 8`). When X is a local
  with no later use (wC) or a local whose value is dead because the loop top re-initialises it (wA),
  combine deletes insns 11 AND 16 outright, pseudo C never reaches the lreg table at all, and
  `vars= 0`. wA additionally costs refs(74) 6 -> 8 (both the dead preheader store and the guard
  occurrence are counted, weight 1 each), which lifts pri(74) to 15000 and moves the flag AHEAD of
  var_a3 (`79 76 77 74 75 82 78 73`) - strictly worse. CONCLUSION: the frame requires a guard whose
  compared register is LIVE PAST THE GUARD, and the only such zero-valued register this function's
  logic contains is the loop counter. s11's residual ("the guard must name the counter to buy the
  frame and must not name it to buy the register order") is re-confirmed against two more spellings.
- verdict: KILLED

## [s12] The counter's live range can be extended to 28 by keeping it live across the `return 1` arm and the epilogue (s9's "+4 at zero byte cost" idea).
- mechanism: flow counts every insn where a reg is live; the counter is dead on the return-1 arm
  (2 insns) and in the epilogue (3 insns), so making it live there was expected to buy +5 exactly.
- probe: wE (`rejected/counter-live-to-end-costs-refs9-10.c`) - candidate.c with `return 1;`
  replaced by `return (var_t1 != 0);`, a use that forces the counter live to the function end.
- result: KILLED, and the reason is a closed-form trap. Liveness can only be extended by adding a
  USE, and every use is also a REFERENCE. wE measures refs(73) = 10 / L73 = 24, i.e. floor_log2 3 ->
  pri 12500, further ABOVE pri(78)=8888 than the baseline's 10434; the greg order is unchanged
  (`79 76 77 75 73 83 78 74 82 81 72`). Doing the arithmetic for the cheapest possible version (one
  extra out-of-loop use, refs(73)=9): the window becomes 27/L73 in (7500, 8888) -> L73 in [31,36],
  while the return-arm + epilogue extension can only supply 23 + 5 = 28. The floor_log2(8)->
  floor_log2(9) step costs more than the liveness it buys. Any use-based extension of 73 is dead.
- verdict: KILLED

## [s12] The measured +1 zero-byte-cost live-range extension (s10's foldable `+k / -k` pair) stacks if the pairs are spread over DIFFERENT variables, or if placed at the loop top instead of the preheader.
- mechanism: s10 proved a 4-link chain on ONE variable is folded by cse1 before flow; distinct
  variables should not share a cse equivalence class, and the loop top is a region where 73 is live
  and 74 is not (so refs-8 needs its added insns there).
- probe: wG (`rejected/two-preheader-pairs-distinct-vars-still-plus1.c`) - a `+1 / -1` pair on
  var_t3 AND a `+0x14 / -0x14` pair on var_a3, both in the preheader; wF
  (`rejected/looptop-foldable-pair-plus1-refs75-blowup.c`) - the var_a3 pair moved to the first
  statement of the loop body, ahead of `var_t2 = 0`.
- result: BOTH KILLED as a route to L73 = 28, but with two useful sub-facts.
  (a) wG measures L73 = 24 - +1, NOT +2: cse1 folds `D_800A38C8 + 1` / `- 1` into one another before
      flow (exactly as it folds a long chain on one variable), so only the pointer pair survives.
      The +1 is confirmed non-stackable through a second, independent mechanism.
  (b) wF measures L73 = 24 and L74 = 16 - so the LOOP-TOP REGION DOES RESPOND (+1) and, placed ahead
      of `var_t2 = 0`, it does NOT lengthen the flag, which is the placement refs-8 needs. But the
      pair's two occurrences are in-loop (weight 2 each), so refs(75) goes 9 -> 17, pri(75) jumps to
      3*17/21 = 24285 and var_a3 is allocated FIRST (`75 79 76 77 73 83 78 74 ...`, 75 -> $3),
      destroying every other seat. An in-loop free insn must therefore name a variable whose refs
      can absorb +4 without crossing a floor_log2 boundary - and every such variable in this
      function is already seated.
  Net: the cost of L73 = 28 is 5 independent foldable pairs (10 pre-combine insns), which s10's and
  this session's measurements say cse1 will not permit, and which would in any case be the F1
  chain-extender FAKE family five times over - not an honest form.
- verdict: KILLED

## [s12] Sibling-precedent re-read (targeted at the frame, which s10's rederive did not check).
- probe: `asm/funcs/func_80037AA4.s` + its MATCHED C at src/code6cac_c.c:285-316.
- result: CONFIRMED and it is load-bearing for how this function should be read. func_80037AA4 -
  same file, same table D_80102810, same bound D_800A38C8, MATCHED - carries the SAME phantom
  `addiu $sp,$sp,-0x8` / `addiu $sp,$sp,0x8` with no `($sp)` reference, and its accepted C uses
  exactly the counter-naming guard (`var_a1 = 0; var_a2 = D_800A38C8; if (var_a1 < var_a2)`).
  So the frame is the house idiom's fingerprint and candidate.c's refs-8 chassis is the ORIGINAL
  shape, not a search artefact. It is also precedent for the shape of the endgame: AA4's own RA
  residual was closed with a Judge-sanctioned `/* FAKE */` constant-holder (`s32 sh = 0xD;`, ruling
  2026-07-28), because reload's update_equiv_regs substitutes the constant and deletes the `li` at
  zero byte cost. That lever cannot be transplanted here - a constant holder changes neither L73 nor
  refs(78) nor L78 - but it establishes that a same-author, same-table sibling needed a sanctioned
  exception for the same class of residual.
- verdict: CONFIRMED


## [s12] s11 frontier #1: a construct other than the counter-naming guard can orphan a pseudo and pay the phantom 8-byte frame, so the refs-7 chassis (which already seats the inner-loop end pointer in $t0) can keep the frame.
- mechanism: s11 proposed the slot is paid by any pseudo flow counts and combine then deletes, not specifically by an ST_REGS compare pseudo; reload's alter_reg pays a slot to any allocno with refs and no hard register.
- probe: Two minimal spellings on the refs-7 do-while chassis, each dumped with cpp | cc1 -da and read at .flow / .combine / .lreg / .greg level (tmp/grind/func_80037B00/s12/probe.sh): wC = a fresh local `var_z = 0; if (var_z < D_800A38C8)`; wA = the EXISTING match flag borrowed as the guard's zero, `var_t2 = 0; if (var_t2 < D_800A38C8)`.
- result: Both measure `vars= 0` (no frame) and no ST_REGS orphan. At FLOW time all three spellings are structurally identical - insn 11 `(set (reg X) (const_int 0))`, insn 14 the bound load, insn 16 `(set (reg C) (lt (reg X) (reg B)))`, jump_insn 17 the branch on C. The difference is what COMBINE does: when X is the loop counter (live past the guard) combine rewrites the branch to `(le (reg B) 0)` and RE-SITES the counter init into insn 16's slot, stranding pseudo C with flow's stale 2 refs, class `ST_REGS or none`, no hard reg, alter_reg slot, `vars= 8`. When X dies at the guard (wC) or is dead because the loop top re-initialises it (wA), combine deletes insns 11 AND 16 outright and pseudo C never reaches the lreg table. wA additionally drives refs(74) 6 -> 8, pri(74) to 15000, and moves the match flag ahead of var_a3 - strictly worse.
- verdict: KILLED

## [s12] s9/s11 frontier: the counter's live range can be lifted the required ~5 insns at zero byte cost by keeping it live across the `return 1` arm and the epilogue.
- mechanism: flow counts every insn where a reg is live; the counter is dead on the return-1 arm (2 insns) and in the epilogue (3 insns), which is exactly the +5 the priority window needs.
- probe: wE = candidate.c with `return 1;` replaced by `return (var_t1 != 0);` (a use that forces the counter live to the function end), RA table read from .lreg/.greg.
- result: refs(73) = 10 / L73 = 24 -> floor_log2 3, pri 12500, which is FURTHER above pri(78) = 8888 than the baseline's 10434; greg order unchanged. Closed form for the cheapest possible version (one extra out-of-loop use, refs 9): the window moves to L73 in [31,36] while the return-arm + epilogue extension can supply at most 23 + 5 = 28. Liveness can only be extended by a USE and every use is also a REFERENCE, so the floor_log2(8) -> floor_log2(9) step always outruns the liveness it buys.
- verdict: KILLED

## [s12] s10's measured +1 zero-byte-cost live-range extension (a combine-foldable `+k / -k` pair) stacks to the needed +5 if the pairs are spread over DIFFERENT variables, or if placed at the loop top (where the flag is not yet live, which is the placement the frame-bearing chassis needs).
- mechanism: s10 proved a 4-link chain on ONE variable is folded by cse1 before flow; distinct variables should not share a cse equivalence class, and the loop-top region lies inside the counter's live range but outside the flag's.
- probe: wG = a `+1 / -1` pair on var_t3 plus a `+0x14 / -0x14` pair on var_a3, both in the preheader; wF = the var_a3 pair moved to the first statement of the loop body, ahead of `var_t2 = 0`.
- result: wG measures L73 = 24, i.e. +1 not +2 - cse1 folds the scalar `+1 / -1` pair before flow, so only the pointer pair survives; the +1 is non-stackable through a second independent mechanism. wF measures L73 = 24, L74 = 16, confirming the LOOP-TOP REGION DOES RESPOND and does not lengthen the flag - but the pair's two in-loop occurrences take refs(75) from 9 to 17, pri(75) to 24285, and var_a3 is then allocated FIRST (75 -> $3), destroying every other seat. Any in-loop free insn must name a variable whose refs can absorb +4 without crossing a floor_log2 boundary, and every variable in this function is already seated.
- verdict: KILLED

## [s12] The refs-7 (correct-register-order) and refs-8 (frame-bearing) chassis pose two independent problems.
- mechanism: s11 described them as two separate knobs - a frame producer for one, a counter-vs-flag priority tie for the other.
- probe: Re-derived global.c's inequalities from the measured tables and re-measured both chassis (candidate.c = score 5 / 36 insns; v5 = score 8 / 35 insns; wD = refs-7 with the counter init before the guard).
- result: They are the SAME problem. With gap = L73 - L74: refs-7 needs 14/L73 >= 12/L74, i.e. L73 >= 7*gap, which at the best measured gap (4, form v5, L73 = 20) is L73 >= 28; refs-8 needs 24/L73 < 8/9, i.e. L73 in [28,32] (L73 = 23). Both reduce to live_length(73) >= 28 - refs-8 with the added insns outside the flag's live range, refs-7 with them inside it. Everything else in the formula is measured pinned: refs(78)=4 and L78=9 (s10), refs(73) in {7,8} by guard spelling (s11), refs(74)=6 and L74=16 across every spelling measured in s11 and s12.
- verdict: CONFIRMED

## [s12] The phantom 8-byte frame is an artefact of our search rather than of the original author's idiom.
- mechanism: s10's rederive compared the matched siblings' loop SHAPE but not their frames.
- probe: Read asm/funcs/func_80037AA4.s against its MATCHED C at src/code6cac_c.c:285-316.
- result: func_80037AA4 - same file, same table D_80102810, same bound D_800A38C8, MATCHED - carries the SAME phantom `addiu $sp,$sp,-0x8` / `addiu $sp,$sp,0x8` with no `($sp)` reference, and its accepted C uses exactly the counter-naming guard `var_a1 = 0; var_a2 = D_800A38C8; if (var_a1 < var_a2)`. candidate.c's refs-8 chassis IS the author's idiom. The same sibling's own RA residual was closed with a Judge-sanctioned /* FAKE */ constant-holder (ruling 2026-07-28) whose lever (reload's update_equiv_regs deleting the li) moves none of L73 / refs(78) / L78 here, so it still does not transfer.
- verdict: CONFIRMED
