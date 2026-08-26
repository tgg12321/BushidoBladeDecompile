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
