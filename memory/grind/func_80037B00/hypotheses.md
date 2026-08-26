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
