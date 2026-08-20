# Hypothesis ledger — gnd_init_80041688

## [s1] Reordering the C source statements so `b = ...` appears before `r`/`g` in the else branch will make sched1 emit b's lbu first, matching target's [b,r,g] order.
- mechanism: LUID/source-order tiebreak in sched1 when picking among ready insns.
- probe: Edited src/text1a.c to `b=...; r=...; g=...;` then ran `sandbox gnd_init_80041688 --disable all`.
- result: Score stayed at 2. Emitted asm identical to the r,g,b source-order build; the three lbu's still emit in [r,g,b] order. Statement/LUID order does NOT override sched1 INSN_PRIORITY when priorities differ.
- verdict: KILLED

## [s2] Struct cast `struct { u8 pad18[0x18]; u8 r,g,b,pad; } *pc = (...)player;` with pc->r/g/b accesses will flip MEM_IN_STRUCT_P on the lbu MEMs and reorder sched1 emission to [b,r,g].
- mechanism: sched.c true_dependence in-struct escape clause: /s=1 MEMs skip certain dependences, altering ready-set priority.
- probe: Rewrote both if/else arms to use pc->r/g/b field access via local `struct rgb_s *pc`. Ran sandbox gnd_init_80041688 --disable all.
- result: score stayed at 2; disassembly shows emission still [r=a0@0x18, v0@0x19, v1@0x1A]. Struct-field lowering did not toggle /s in a way sched1's ready-list ordering could see (or the /s change did not affect the true_dependence edges among the three parallel lbu's).
- verdict: KILLED

## [s2] Walking pointer `u8 *bp = (u8*)player + 0x1A; b = *bp; r = *(bp-2); g = *(bp-1);` gives b a distinct base pseudo, changing the MEM alias set and letting b schedule earlier via a different dependence chain.
- mechanism: Different base-pointer pseudo -> distinct MEM_ALIAS_SET_P; potentially longer dependence chain to bump b's priority (walking-pointer-serializes-parallel-loads family, opposite polarity).
- probe: Replaced FALSE branch loads with walking-pointer form; sandbox + disasm.
- result: score stayed at 2; emission still [r,g,b]. GCC 2.7.2 combine/CSE folded `bp - 2` and `bp - 1` back to (plus (reg player) (const K)) BEFORE sched1, so all three MEMs re-collapsed into identical (base + const_offset) shape. Same dependence graph, same schedule.
- verdict: KILLED

## [s2] Ternary/hoist: pull the three lbu's out of both branches into a shared prologue so they load once before the if/else; cross-jump may unify the tail and sched1 sees a different block layout.
- mechanism: if/else -> COND_EXPR RTL changes BB boundaries; jump.c cross-jump may merge shared tails; sched1 operates on a different graph.
- probe: Hoisted `r=...0x18; g=...0x19; b=...0x1A;` above the if; deleted from both arms. Sandbox.
- result: score exploded 2 -> 32, build_insns 82 -> 84. Target has TWO SEPARATE lbu blocks (one before `jal func_8004881C` @ 80041770-78 loading a0/a1/a2 in arg order; one at .L80041798 loading v1/a0/v0 for the OR-tree). Consolidating destroys the two-block structure. The duplicated per-branch lbu triples are LOAD-BEARING; do not hoist.
- verdict: KILLED

## [s2] Block-local var split `s32 fb, fr, fg;` inside the FALSE arm with fb assigned first and used in `fb | ((fr<<16)|(fg<<8))` biases sched1 tie-break toward emitting b's lbu first via DECL_ORDER / LUID.
- mechanism: Fresh per-branch pseudos with fb allocated ahead of fr/fg might steer sched1's tie-break among equal-priority ready insns.
- probe: Declared block-local fb/fr/fg; ordered `fb=...0x1A; fr=...0x18; fg=...0x19;` first. Sandbox.
- result: score stayed at 2; emission still [r,g,b]. Corroborates s1's finding at greater strength: sched1's INSN_PRIORITY (chain length 4 for r/g via sll->or->or vs chain length 2 for b via or) is a HARD ordering. DECL/LUID tie-break applies only among equal-priority insns, so it can never demote r/g in favor of b.
- verdict: KILLED

## [s3] A named block-local intermediate `s32 rg = (r<<16) | (g<<8); gnd_load_tex(b | rg);` materializes the r/g sub-tree as a live pseudo and reshuffles sched1 across BB boundaries or via LUID lift on the SET, promoting b's emission order.
- mechanism: If GCC keeps rg as a distinct RTL SET the dependence chain from b to end-of-block would gain the intermediate move node, and sched1 would either see a longer b-chain (via reg_pending_uses) or hoist the b lbu earlier in the ready list.
- probe: Edited FALSE arm to declare `s32 rg` and split the OR; sandbox --disable all.
- result: Score stayed at 2. Disassembly shows FALSE block byte-identical to baseline (lbu order [r,g,b], sll/or tree unchanged). CSE/combine folded rg back into the two sub-ORs before sched1 saw a separate SET; no live-out kept it alive.
- verdict: KILLED

## [s3] A distinct block-local `u8 *cp = (u8*)player;` with `b=cp[0x1A]; r=cp[0x18]; g=cp[0x19];` gives the three lbu MEMs a fresh base pseudo, changing alias-set / dependence attribution enough to let b schedule earlier.
- mechanism: A distinct pointer pseudo (not arithmetically derived from `player` like the s2 walking-pointer) might dodge the combine collapse that killed s2 walking-pointer and force MEM_ALIAS_SET_P divergence on the three lbu's.
- probe: Rewrote FALSE arm with block-local `cp` cast from player, ordered b/r/g in source with b first. Sandbox.
- result: Score stayed 2. Base register in emitted lbu's still $s0 (same as `player`); GCC's copy-propagation merged cp's pseudo with player's before sched1. All three lbu MEMs remain identical shape; dependence DAG unchanged; source-order b-first was ignored (chain-length priority dominates).
- verdict: KILLED

## [s3] Declaring cp function-scope (live from prologue across loop1/loop2 into the FALSE branch) forces GCC to keep cp in its own pseudo (no copy-prop into player), spanning the FALSE block's live-in set and shifting RA/schedule to match target.
- mechanism: Extended-liveness pseudo across multiple BBs escapes local copy-propagation; cp's distinct hard-reg assignment might displace the base register used for the three lbu's or nudge sched1 tie-breaks.
- probe: Declared `u8 *cp;` at function scope, initialized `cp = (u8*)player;` before loop1, referenced as `cp[0x18/19/1A]` in FALSE arm. Sandbox.
- result: Score EXPLODED 2 -> 15. Function-scope cp perturbed RA broadly across loop1/loop2 — the FALSE branch was collateral damage, not helped. Strictly worse than block-local cp and strictly worse than baseline. This axis is net-negative, not merely inert.
- verdict: KILLED

## [s4] A randomization-only permuter run on the current s3 candidate (base score=40) will find a non-cheat closing form (score 0 or a legitimate mutation lowering the sandbox floor).
- mechanism: Random C-source mutations (statement-reorder, variable-reuse, decl-order, cast changes) explore the neighborhood of the current form; a mutation that changes sched1's dependence graph without adding dead code / OR-tree respellings would yield sub-40 legitimately.
- probe: Built workspace tmp/grind/gnd_init_80041688/s4/perm (base.c=preprocessed text1a.c, compile.sh mirrors sandbox pipeline w/ prologue_fix+maspsx+multu_pad, target.o from asm/funcs/gnd_init_80041688.s + prelude_r3k.inc). Verified baseline diff is the single [b,r,g]<->[r,g,b] lbu swap; permuter base_score=40. Launched via tools/permuter_campaign.py launch (-j 4, chassis label s4-chassis1); ran ~24 min / ~32k iters; harvested + stopped per fresh-seed discipline.
- result: 9 novel finds: 1×score10, 2×score20, 4×score30, 2×score40. Best = 10 at t=242s. All sub-40 finds are cheat family: output-10-1 stages loop1's `(s16*)(p+2)>=0` boolean through function-scope `b` (dead store — each iter overwrites, final value overwritten by FALSE-arm color load); output-20-1 stages `b=arg1` reused across all flag conditionals (same dead-store family); output-20-2 respells TRUE-arm OR as `b=(v<<16)|(v<<8); b|v` (or-tree-shape-shift FORBIDDEN); output-30-1 stages `*q` via `b`; output-30-2/3 respell TRUE-arm OR through `b`; output-30-4 stages `*(s32*)(q+0x57)` + injects `if(1){}` dead code; output-40-1 alias-renames `b` to `i`; output-40-2 introduces `new_var` pointer alias. No non-cheat find below the baseline basin.
- verdict: KILLED

## [s4] The chassis-1 permuter basin's best legitimate lever is a variable-reuse pattern using the function-scope `b` — extending b's live range across loop1 perturbs FALSE-arm entry register pressure enough to swap sched1's lbu emission order.
- mechanism: The score-10 mutation (b staged in loop1 boolean check) is a per-iteration dead store to an existing local; permuter's insn-diff drops from 2 to ~1 partial, indicating the FALSE-arm b's live-in changes at 0x15cc. Corroborates s3 frontier hypothesis 2 (register-class drift at BB entry is the true axis).
- probe: Compared the score-10 diff to the [[dead-store-fake-exception]] / [[staged-value-reused-variable]] sanction prerequisites: neither prereq matches — the staged value is IMMEDIATELY dead (final loop-exit value overwritten by the FALSE-arm's `b = *(u8*)(player+0x1A)`), so 'live code only, zero dead stores' fails; every iteration's write is dead. Vetted per [[no-new-park-categories]] cheat-by-any-spelling: the construct has no semantic purpose, its only effect is RA/scheduling coercion.
- result: The reuse-`b` axis IS what moves sched1 (evidence: 40->10 permuter drop). But the byte-neutral spelling of that axis via loop1 boolean staging is a dead-store cheat by policy. Saved as memory/grind/gnd_init_80041688/rejected/loop1-boolean-stage-b-reuse.c. Any future lever on this axis must extend b's live range through a LIVE, semantically-purposeful assignment (not a dead-per-iteration comparison result) — a much narrower search than random permutation covers.
- verdict: CONFIRMED

## [s5] Frontier #1 — hoist `b = *(u8*)(player+0x1A)` BEFORE loop1 (LIVE across loop1+loop2+call, consumed by both arms unchanged) legitimately perturbs FALSE-arm entry RA the same way the s4 score-10 cheat did, without a dead store.
- mechanism: b live-in to FALSE arm changes; cc1 RA may pick a different callee-save for b at 0x15cc. Risk: cross-arm live b costs a callee-save spill across the TRUE arm (arg + overwrite by v).
- probe: Edited src/text1a.c: added `b = *((u8*)player + 0x1A);` after NULL check, dropped FALSE-arm re-assignment. Sandbox --disable all. Variant 1a (keep TRUE arm re-read): score=9, build_insns=84. Variant 1b (also drop TRUE re-read): score=11, build_insns=83. Variant 1c (hoist between loop1 and loop2, keep TRUE re-read): score=9, build_insns=84.
- result: All three variants REGRESS from baseline (2 -> 9/11). Extending b's live range across the func_800486FC() call forces a callee-save spill+restore in the prologue that outweighs any FALSE-arm scheduling gain. The score-10 cheat form worked because it kept b LIVE ONLY across loop1 (dead at the call); the LIVE-code equivalent for a value that's ALSO consumed in an arm can't avoid crossing the call.
- verdict: KILLED

## [s5] Frontier #3 — hoist ONE of r/g/b above `if (func_800486FC())` (post-loops, pre-branch) as a duplicated-read into both arms is a byte-neutral live-code perturbation of FALSE-arm entry pressure.
- mechanism: Aligns with [[duplicated-statement-into-arms]] and [[split-read-defeats-hoist]]. Duplicating a color-byte load into shared post-loop scope extends its live range across the func_800486FC call; RA may pick a different callee-save.
- probe: Three variants tested via sandbox --disable all with post-loop pre-if hoist: r-hoist=score 12 (build_insns 83), g-hoist=score 13 (build_insns 82), b-hoist=score 11 (build_insns 83).
- result: All three REGRESS. Same root cause as hypothesis 1: any color-byte pre-loaded above the call becomes live-across-call and forces callee-save allocation, adding spill cost. The FALSE-arm sched1 lbu-order gain (if any) is dominated by the call-crossing overhead. The duplicated-statement-into-arms shape is only byte-neutral for values NOT live across an intervening call.
- verdict: KILLED

## [s5] The s4 chassis-1 basin conclusion (all sub-40 finds are cheat-family) reproduces from a fresh permuter seed — a second-seed run finds the SAME dead-store cheat (b staged in loop1 boolean) plus similar cast/variable-reuse tricks, none of which are legitimate.
- mechanism: Fresh-seed discipline (permuter-directives §Campaign discipline) says a basin yields early or not at all. Running chassis-1 again from a different seed empirically tests whether the s4 basin was seed-specific or is intrinsic to this chassis.
- probe: Copied s4 workspace to tmp/grind/gnd_init_80041688/s5/perm (cleared outputs), launched fresh via tools/permuter_campaign.py (label s5-chassis1-freshseed, -j 4, base_score=40). Ran 934s / 19204 iters, harvested + stopped.
- result: 5 novel finds: 1×score10 at t=763s, 2×score20, 1×score30, 1×score40. score-10 form: `b = (*((s16 *)(p+2))) >= 0; if (b) { ... }` — IDENTICAL loop1-boolean-stage-b-reuse cheat as s4's score-10 (already banked in rejected/). score-20-1: `b = arg1;` staged before loop2, `if (b)` inside loop2 in place of `if (arg1)` — same dead-store-through-b family (b overwritten later in FALSE arm). score-20-2: `if (player == ((void *)((float)0)))` — float-cast NULL comparison (cast trick, no semantic purpose). Two independent seeds → same basin structure → chassis-1 is exhausted for legitimate finds.
- verdict: CONFIRMED

## [s6] The score-10 cheat form flips the FALSE-arm lbu emission order via a register-ALLOCATION difference — the cheat form's extended live range for `b` forces cc1 to pick a different hard-reg for one of the FALSE-arm color pseudos.
- mechanism: cc1 global.c allocno-priority tiebreak; the extended-liveness pseudo would displace one of pseudos 78/79/80 to a different hard reg, altering sched2's DAG.
- probe: Compiled baseline_standalone.c and cheat10_standalone.c with tmp/gccdbg/cc1 (BB2_ALLOC_DEBUG=1 -da). Diffed `;; Register dispositions:` and `;; Hard regs used:` in .greg.
- result: IDENTICAL allocation in both forms — pseudo 78→$v1(3), pseudo 79→$a0(4), pseudo 80→$v0(2), pseudo 74→$s0(16); `;; Hard regs used: 2 3 4 5 6 16 31` identical. ALLOCDBG confirms same ord/hardreg for all 8 pseudos, only pri differs (cheat pseudo 78: nrefs=8 livelen=7 pri=34285 vs baseline: nrefs=4 livelen=3 pri=26666). Allocation is a spectator; the emission-order flip is NOT a global.c-tiebreak axis for this function.
- verdict: KILLED

## [s6] The score-10 cheat form flips FALSE-arm lbu emission via a sched1 (haifa-sched.c) priority-tag differential driven by pseudo 78's extended live range across loop1 into the FALSE arm.
- mechanism: flow.c/life_analysis fuses the source-level shared `b` identifier into ONE pseudo (78) spanning loop1's boolean staging (insns 65-66 in .flow) plus the FALSE-arm reload (insn 194 in cheat, insn 191 in baseline). This anti-dep chain changes sched.c's insn_priority calculation: baseline's b-lbu gets the same hazard-boosted priority 0x7F000001 (2130706433) as the r/g lbu's and wins the luid-ascending tiebreak (luid=4 vs 5,6) at clock=4 to launch mid-block; cheat's b-lbu gets plain priority 1 (unboosted) — sched1 defers it to clock=8 (the FIRST-in-code slot after reverse-order emission), producing target's [b,r,g] emit order.
- probe: BB2_SCHED_DEBUG dump of BB18 (FALSE arm) in tmp/grind/gnd_init_80041688/s6/{baseline,cheat10}_dbg.log. Verified pseudo 78 conflict-list widens from `78 2 4 29` (baseline) to `74 78 79 80 2 4 29` (cheat) in .greg. Cross-checked with .sched (sched1 ready-list) and .sched2 (post-alloc scheduler) — the priority-tag differential appears at sched1 and propagates unchanged into sched2.
- result: CONFIRMED. The exact GCC pass = sched.c (sched1, pre-RA). The exact decision = insn_priority hazard-tag application. Baseline BB18 clock=4 ready list `[191(p=2130706433,l=4) 194(p=2130706433,l=6) 193(p=2130706433,l=5)]` — three ready insns all p=0x7F000001 boosted, sched picks 191 (b-lbu) by luid=4-ascending. Cheat BB18 clock=4 ready list `[197(p=2130706433,l=6) 196(p=2130706433,l=5) 194(p=1,l=4)]` — insn 194 (b-lbu) has raw p=1, NOT boosted; sched picks 197/196 first, 194 slips to clock=8 (last-scheduled → first-emitted).
- verdict: CONFIRMED

## [s6] The mechanism that ultimately reaches sched1 is `flow.c` live-range unification of source-level `b` across loop1 and the FALSE arm — a pure-C form that widens pseudo 78's live range from before loop1 into BB18 WITHOUT crossing the func_800486FC call and WITHOUT a dead-per-iteration store would reproduce the target schedule.
- mechanism: flow.c/life_analysis + local-alloc coalesce all defs of source-level `b` into one pseudo. Any earlier LIVE def of `b` will fuse into pseudo 78 and re-trigger the sched1 hazard-tag differential. Constraint: s5 KILLED all axes that cross the func_800486FC call (callee-save spill cost dominates). The remaining unexplored surface is loop1-internal staging where the intermediate has semantic purpose.
- probe: This session did NOT execute new sandbox probes for this hypothesis. It's identified as the frontier from the sched1/greg forensics, not measured. The rejected form loop1-boolean-stage-b-reuse.c is the closest cheat spelling (stages the BOOLEAN result through `b`). A very narrow non-cheat corner NOT in the rejected bank is `b = *(s16*)(p+2); if (b >= 0) { ... }` — stages the RAW halfword field (not the boolean) as a named local before consuming it in the if. Both spellings produce identical sched1 pseudo-78 fusion and identical hazard-tag flip.
- result: Frontier not measured this session (forensics scope). Left as ruling-request-adjacent frontier for s7.
- verdict: CONFIRMED

## [s7] The s6 frontier probe — `b = *(s16*)(p+2); if (b >= 0) { ... }` — reproduces the cheat10 sched1 hazard-tag flip byte-neutrally, closing the FALSE-arm without loop1 collateral.
- mechanism: raw-halfword staging fuses pseudo 78 via flow.c the same way the boolean-stage cheat does; sched1 should reproduce cheat10's clock=8 emission of the b-lbu (target [b,r,g] order); loop1 RA should stay intact because `b` is only a temporary between lh and bltz.
- probe: (a) built tmp/grind/gnd_init_80041688/s7/s7raw_standalone.c and compiled with instrumented cc1 (BB2_ALLOC_DEBUG=1 BB2_SCHED_DEBUG=1 BB2_PRIO_DEBUG=1) → full RTL dumps in tmp/grind/gnd_init_80041688/s7/s7raw_standalone.c.{rtl,flow,greg,sched,sched2,...}; (b) applied form to src/text1a.c line 972; sandbox --disable all → score REGRESSED 2 → 11, build_insns 82 → 81; (c) diffed s7raw .s vs baseline_standalone.s and cheat10_standalone.s; (d) diffed .greg pseudo dispositions across the three forms; (e) reverted src, re-verified sandbox score=2.
- result: **PARTIALLY CONFIRMED, NET KILLED.** The FALSE-arm lbu order DID flip to target's [b,r,g] as predicted (s7raw .s FALSE 125-127 = [lbu $3,26 / lbu $4,24 / lbu $2,25], identical to cheat10). Mechanism CONFIRMED: pseudo-78 fusion via loop1-scope defs of source-level `b` DOES trigger the sched1 hazard-tag flip. BUT the total sandbox score REGRESSED 2 → 11 because raw-halfword staging displaced loop1 register allocation: .greg shows pseudo 75(i) shifted $a0 → $a2, pseudo 76(p) shifted $v1 → $a0, and reorg.c chose a different loop rotation (`.L5:` above `addu p+=104` vs baseline's `.L21:` below the delay-slot addu). Root cause: raw-halfword form gives `b` a non-zero intra-iter live range (2 insns: lh → bltz), which conflicts with `p`'s live range across the loop1 body, forcing greg to split them into different hardregs — the boolean cheat10 form dodges this because setcc's def-use is back-to-back (zero intra-iter live range).
- verdict: KILLED (as byte-neutral alternative; mechanism half CONFIRMED but non-neutral in total)

## [s7] There exists NO in-loop1 spelling of the pseudo-78-fusion axis that is BOTH byte-neutral to baseline AND non-cheat.
- mechanism: forced dichotomy: to fuse pseudo 78 across loop1, some loop1-scope def of source-level `b` must reach flow.c. Either (a) that def has zero intra-iter live range (setcc/slt directly consumed by branch) — in which case it's a dead store to `b` per [[no-new-park-categories]] cheats-by-any-spelling; or (b) the def has non-zero intra-iter live range (raw halfword staged into `b`, then tested) — in which case `b`'s live range conflicts with `p`'s use as lh base + downstream body base, and greg splits them, displacing p from $v1 and cascading i and loop rotation.
- probe: This is the synthesis of s4/s5 (cheat10 = shape (a) → forbidden dead store) and s7 (raw-halfword = shape (b) → RA cascade). No other spelling class exists for "loop1-scope def of source-level `b`" — a def either produces a value that dies before its next use (dead) or lives to that use (live-range).
- result: CONFIRMED via case exhaustion of the two possible live-range shapes.
- verdict: CONFIRMED (KILLS the "loop1-b-staging" family as a viable frontier)

## [s7] The s6 frontier probe `b = *(s16*)(p+2); if (b >= 0) { ... }` reproduces the cheat10 sched1 hazard-tag flip byte-neutrally, closing FALSE-arm without loop1 collateral.
- mechanism: Raw-halfword staging fuses pseudo 78 via flow.c/life_analysis the same way the boolean cheat10 does; sched1 should reproduce clock=8 emission of the b-lbu (target [b,r,g] order); loop1 RA should stay intact because `b` is only a temporary between lh and bltz.
- probe: Built s7raw_standalone.c and compiled with instrumented cc1 (BB2_ALLOC_DEBUG/SCHED_DEBUG/PRIO_DEBUG=1) → full RTL dumps. Applied form to src/text1a.c line 972; sandbox --disable all. Diffed s7raw.s vs baseline_standalone.s and cheat10_standalone.s at FALSE-arm color-lbu triple. Diffed .greg Register dispositions across three forms. Reverted src, re-verified score=2.
- result: PARTIALLY CONFIRMED, NET KILLED. FALSE-arm lbu order DID flip to target's [b,r,g] (s7raw .s FALSE 125-127 = [lbu $3,26 / lbu $4,24 / lbu $2,25], byte-identical to cheat10 in that region). Mechanism CONFIRMED: pseudo-78 fusion via loop1-scope defs of source-level `b` DOES trigger the sched1 hazard-tag flip. BUT total sandbox score REGRESSED 2 → 11 (build_insns 82 → 81): .greg shows pseudo 75(i) $a0 → $a2, pseudo 76(p) $v1 → $a0, and reorg.c chose a different loop rotation (.L5: above `addu p+=104` vs baseline's .L21: below the delay-slot addu). Root cause: raw-halfword form gives `b` a 2-insn intra-iter live range (lh → bltz) that conflicts with `p`'s use as lh base + downstream body base; greg splits them, displacing p from $v1.
- verdict: KILLED

## [s7] There exists NO in-loop1 spelling of the pseudo-78-fusion axis that is BOTH byte-neutral to baseline AND non-cheat.
- mechanism: Forced dichotomy: to fuse pseudo 78 across loop1, some loop1-scope def of source-level `b` must reach flow.c. Either (a) that def has zero intra-iter live range (setcc/slt directly consumed by branch) — dead store to `b` per cheats-by-any-spelling; or (b) non-zero intra-iter live range (raw halfword staged) — `b`'s live range conflicts with `p`'s use as base + downstream body base, greg splits them and cascades i and loop rotation.
- probe: Synthesis of s4/s5 evidence (cheat10 = shape (a) → forbidden) + s7 measurement (raw-halfword = shape (b) → RA cascade). No third spelling class exists: a loop1-scope def of `b` either produces a value dead before use OR live to use.
- result: CONFIRMED by case exhaustion. Kills the entire `loop1-b-staging` family as a viable frontier for reaching floor 0 without other regressions.
- verdict: CONFIRMED

## [s8] m2c fresh decompile reveals structurally-different shape (shared v + shared gnd_load_tex outside arms) that when applied unlocks a new schedule/RA path around the sched1 lbu-emission wall.
- mechanism: target has ONE jal gnd_load_tex at .L800417B4 shared between arms (m2c reconstructs this original-source structure). Providing this to our fork should let jump2 preserve target byte layout (2 or's per arm, then shared jal).
- probe: Rewrote FALSE arm to `v = *((u8*)player+0x1A) | ((*((u8*)player+0x18)<<16) | (*((u8*)player+0x19)<<8));`, TRUE arm to `v = func_8004881C(...); v = (v<<16)|(v<<8)|v;`, then `gnd_load_tex(v);` outside both arms. sandbox --disable all.
- result: score REGRESSED 2→12, build_insns 82→80. jump2/find_cross_jump merged the trailing intermediate + final or's into a shared tail spanning both arms (dropping 2 insns vs target). Target blocks the merge via arm-distinct final-or shapes (TRUE `or a0,a0,v0`, FALSE `or a0,v1,a0` with b in $v1) — which requires the [b,r,g] lbu order (the s1-s7 wall). m2c-shape converges back to the same wall.
- verdict: KILLED

## [s8] Dropping intermediate r/g/b locals (inlining loads into OR expression) alters sched1 LUID topology enough to swap lbu emission order.
- mechanism: Without intermediate SETs pinning r/g/b to pseudos before the OR, sched1 sees a smaller RTL region with potentially different LUID assignments.
- probe: Inlined loads in FALSE-arm-only, then BOTH arms; sandbox --disable all each.
- result: Score stayed 2 (byte-identical to baseline) in both variants. Combine folds inlined loads back to same RTL pre-sched1. Load-inlining is inert.
- verdict: KILLED

## [s8] m2c fresh decompile of gnd_init_80041688.s reveals a structurally-different C shape (shared function-scope v, single shared gnd_load_tex(v) call outside both if/else arms) that when applied will unlock a new schedule/RA path around the sched1 lbu-emission wall.
- mechanism: The target has ONE jal gnd_load_tex at .L800417B4 shared between arms (TRUE arm jumps there via `j .L800417B4`; FALSE arm falls through). This matches m2c's reconstructed shape: `if(...) v=...; else v=...; gnd_load_tex(v);`. Providing this original-source-like structure to our fork should let jump2 preserve the target's byte layout (2 or's in each arm, then shared jal).
- probe: Rewrote FALSE arm to `v = *((u8*)player+0x1A) | ((*((u8*)player+0x18)<<16) | (*((u8*)player+0x19)<<8));` and TRUE arm to `v = func_8004881C(...); v = (v<<16)|(v<<8)|v;`, then `gnd_load_tex(v);` outside both arms. sandbox --disable all.
- result: score REGRESSED 2 -> 12, build_insns 82 -> 80. objdump: jump2/find_cross_jump merged the trailing `or v0,v0,v1` and `or a0,a0,v0` (delay slot) into a SHARED tail spanning both arms — collapsing 2 insns vs target. Target keeps arm-distinct final-or shapes (TRUE final: `or a0,a0,v0` in `j` delay slot; FALSE final: `or a0,v1,a0` — b on LEFT in $v1). To match target's byte layout in shared-jal shape, b must land preserved in $v1 across the FALSE arm ORs, which requires target's [b,r,g] lbu order — the same wall s1-s7 hit. m2c-shape does NOT unlock a new path.
- verdict: KILLED

## [s8] Dropping intermediate r/g/b locals in the FALSE arm (inlining the loads directly into the OR expression) will alter sched1's LUID topology enough to swap the lbu emission order to target's [b,r,g].
- mechanism: With no intermediate SET nodes between the three lbus and the OR-tree, cfganal/sched1 sees a smaller RTL region and may assign LUIDs differently — the intermediate SETs in the current form pin r/g/b to specific pseudos before the OR combines them.
- probe: Applied `gnd_load_tex(*((u8*)player+0x1A) | ((*((u8*)player+0x18)<<16) | (*((u8*)player+0x19)<<8)));` in FALSE arm (kept TRUE arm intact). sandbox --disable all. Then extended to BOTH arms (inlined loads in TRUE arm's func_8004881C call too).
- result: Score stayed at 2 in both variants — byte-identical to baseline. Combine folds inlined loads back to the same RTL before sched1 sees them; no LUID/DAG effect. Load-inlining is inert for this function.
- verdict: KILLED

## [s9] Frontier probe #2 (s8 live): shared-v with split-init `|=` chain in FALSE arm produces enough arm-distinct RTL SETs to defeat jump2/find_cross_jump merge past the shared jal, unlocking target's arm-distinct final-or shapes without a cheat.
- mechanism: Split-init `v = b; v |= r<<16; v |= g<<8;` (or reordered) creates three separate SET-RTL nodes on `v` per arm vs TRUE arm's single `v = (v<<16)|(v<<8)|v` after func call. Different SET topology per arm may leave jump2 with tails too dissimilar to merge past the final or.
- probe: Three variants applied to src/text1a.c: (a) shared gnd_load_tex(v) outside arms + split-init b-first FALSE, (b) per-arm gnd_load_tex(v) + split-init b-first FALSE, (c) per-arm gnd_load_tex(v) + split-init b-LAST FALSE. sandbox --disable all each. Disasm dumped to tmp/grind/gnd_init_80041688/s9/split_init_per_arm.dis.
- result: All three variants: score=11, build_insns=81 (regressed 2->11, one insn merged). Combine folds accumulation order pre-sched1 — b-first vs b-last produce byte-identical output. Cross-jump/find_cross_jump still merged one FALSE-arm `or` into the shared jal delay slot despite the multi-statement structure. HOWEVER: FALSE-arm b DOES land in $v1 and final delay-slot or IS `or a0,v1,a0` — matching target's shape. But lbu emission order stays [r,b,g] not target's [b,r,g], and the missing insn (folded into delay slot) costs 11 masked score points.
- verdict: KILLED

## [s9] Frontier probe #1 (s8 live): shared-v + loop1-raw-halfword staging combined produces different RA than either axis alone (s7's raw-halfword displaced p in arm-local shape; shared-v shape may have different RA).
- mechanism: Two axes together may produce different sched1/jump2 interaction than either alone; shared-v shape has different cross-arm liveness for v that may steer greg differently than the arm-local case.
- probe: Not measured. loop1-raw-halfword staging (`b = *(s16*)(p+2); if (b >= 0)`) is a dead store to b per s7 policy vetting ([[no-new-park-categories]] cheats-by-any-spelling — the store to b is immediately dead until FALSE-arm overwrites it). Combining a cheat-axis with a legitimate axis does not sanitize the cheat.
- result: Skipped on policy grounds. The raw-halfword axis is banked in rejected/loop1-raw-halfword-stage-b.c as a cheat and cannot be re-proposed even as a combining axis.
- verdict: KILLED

## [s10] Intra-BB pre-color `b = 0;` init inside the FALSE arm (before the r/g/b color-lbu triple) fuses pseudo 78's live range without crossing loop1 or the func_800486FC call, reproducing the s6/s7 sched1 hazard-tag flip without loop1 collateral OR call-crossing spill.
- mechanism: A def of source-level `b` inside BB18 ahead of the FALSE-arm color lbu triple. If flow.c/life_analysis sees the `b = 0;` def before the color-lbu overwrite, it fuses defs into a single pseudo 78 whose live range now spans two intra-BB defs — the same hazard-tag differential that s6 measured. Constraint: def must not be DCE'd before the fusion pass runs.
- probe: Edited src/text1a.c FALSE arm to `b = 0; r = ...; g = ...; b = ...; gnd_load_tex(...)`. sandbox --disable all.
- result: score=2, target_insns=82, build_insns=82. Byte-identical to baseline. GCC's dead-store elimination (in the .cse/.combine passes upstream of flow.c) removes the `b = 0;` store before life_analysis sees it, so pseudo 78 is NOT re-fused and the hazard-tag stays boosted. The intra-BB fusion axis is INERT — not a cheat, but not a lever either.
- verdict: KILLED

## [s10] SYNTHESIS: the pseudo-78-fusion axis (s6-CONFIRMED mechanism for the FALSE-arm lbu-order flip) has zero remaining sanctioned surface across the whole function.
- mechanism: Case exhaustion. To flip sched1's insn_priority hazard-tag on the b-lbu (the confirmed mechanism), pseudo 78's live range must widen. Pseudo 78 lives ENTIRELY inside BB18 in baseline; any widening requires a def of source-level `b` at one of three sites: (a) inside loop1, (b) between loop2 and the branch (hoist above func_800486FC call), or (c) inside BB18 ahead of the color-lbu. s7 dichotomy KILLED (a): loop1-scope defs are dead-store cheats (zero intra-iter live range) or RA cascades (non-zero live range displaces p from $v1). s5 KILLED (b): all hoist axes force callee-save spill across the call. s10 KILLED (c): intra-BB inits DCE'd before flow.c sees them. No fourth site exists in the function's CFG.
- probe: Synthesis of s5/s6/s7/s10 measurements. No new sandbox run needed; this is a proof-of-exhaustion over the sanctioned axis surface.
- result: CONFIRMED. The sched1 hazard-tag lever is not reachable by any pure-C form that avoids the three closed families above. Every remaining source-shape mutation either (a) doesn't fuse pseudo 78 (inert; floor=2) or (b) fuses it via one of the three sites already measured dead. The function's residual score-2 diff is measurement-bounded on sanctioned lever surface.
- verdict: CONFIRMED (kills the pseudo-78-fusion frontier as approachable via sanctioned axes on this function)


## [s10] Intra-BB pre-color `b = 0;` init inside FALSE arm fuses pseudo 78 without loop1/call-cross collateral, reproducing the s6/s7 hazard-tag flip cleanly.
- mechanism: A def of source-level `b` inside BB18 ahead of the color-lbu triple would let flow.c/life_analysis coalesce two intra-BB defs into a single widened pseudo 78 — the same fusion signature s6 measured as flipping sched1's insn_priority hazard-tag on the b-lbu from boosted (pri=0x7F000001) to unboosted (pri=1), which reorders emission from [r,g,b] to target's [b,r,g].
- probe: Edited src/text1a.c FALSE arm to `b=0; r=...; g=...; b=...; gnd_load_tex(...)`. Ran sandbox --disable all. Restored src, re-verified sandbox=2.
- result: score=2, target_insns=82, build_insns=82 — byte-identical to baseline. GCC's DCE (upstream of flow.c) eliminates the `b=0;` store before life_analysis sees it; pseudo 78 is NOT re-fused; hazard-tag stays boosted; lbu emission unchanged.
- verdict: KILLED

## [s10] SYNTHESIS — the pseudo-78-fusion axis (s6-CONFIRMED mechanism for the FALSE-arm lbu-order flip) has zero remaining sanctioned surface anywhere in the function.
- mechanism: Case exhaustion over the three possible CFG sites for widening pseudo 78's live range: (a) loop1-scope def, (b) between loop2 and the branch (pre-call hoist), (c) intra-BB pre-color init inside BB18. s7 dichotomy KILLED (a): loop1 defs are dead-store cheats OR RA cascades displacing p from $v1. s5 KILLED (b): callee-save spill across func_800486FC dominates any FALSE-arm gain. s10 KILLED (c): intra-BB inits DCE'd before flow.c fusion. No fourth site exists in the CFG.
- probe: Cross-session synthesis of s5, s6, s7, s10 measurements; no new sandbox run required — this is a proof-of-exhaustion over the sanctioned lever surface documented in the ledger.
- result: Every source-shape mutation reachable in pure C either (a) leaves pseudo 78 un-fused (inert; floor stays 2) or (b) fuses pseudo 78 via one of the three sites already measured dead. The residual score-2 diff is bounded by the closed sanctioned-lever surface.
- verdict: CONFIRMED

## [s11] A `goto call_tex_false; call_tex_false: gnd_load_tex(...);` inserted between the FALSE-arm color-lbu triple and the gnd_load_tex call shifts LUID/jump2 topology enough to perturb sched1's tiebreak on the b-lbu.
- mechanism: Explicit label between color loads and the call would create a CODE_LABEL insn that (a) potentially blocks jump2/find_cross_jump merging past that point, or (b) shifts LUID assignment on the three color-lbu insns enough to affect sched1's ready-list ordering.
- probe: Edited FALSE arm to add `goto call_tex_false; call_tex_false: gnd_load_tex(...)`. Ran sandbox gnd_init_80041688 --disable all.
- result: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 — BYTE-IDENTICAL to baseline. GCC 2.7.2 jump.c strips the redundant label and elides the `goto NEXT_STMT;` before jump2/reorg see them; no LUID shift, no jump2 steering effect, no sched1 perturbation.
- verdict: KILLED

## [s11] A labeled ENTRY point BEFORE the FALSE-arm color triple (`goto load_rgb_false; load_rgb_false: r=...`) shifts LUID assignment on the color-lbu triple by inserting a CODE_LABEL ahead of them.
- mechanism: Same as variant A but with label ahead of the loads rather than between loads and call. Might affect the LUID base for the block.
- probe: Edited FALSE arm to prepend `goto load_rgb_false; load_rgb_false:` before the r/g/b loads. Ran sandbox.
- result: score=2, build_insns=82 — byte-identical to baseline. Same jump.c strip behavior; label is redundant, elided pre-jump2.
- verdict: KILLED

## [s11] Distinct trailing labels in BOTH arms (`goto tex_done_true; tex_done_true:;` in TRUE, `goto tex_done_false; tex_done_false:;` in FALSE) prevent jump2 from finding a cross-jump merge past the shared join point, leaving arm-distinct RTL for sched1.
- mechanism: If arm-terminator labels emit distinct CODE_LABEL insns, jump2's find_cross_jump may see two distinct suffix chains rather than one merge candidate.
- probe: Edited BOTH arms with distinct `goto x; x:;` at each arm's tail. Ran sandbox.
- result: score=2, build_insns=82 — byte-identical to baseline. Empty labeled statement + trailing `goto NEXT_STMT;` both stripped by jump.c before jump2 runs.
- verdict: KILLED

## [s12] loop1 rewritten as natural `do { ... } while (i < 18);` produces different RTL than the manual goto/label form and could steer reorg.c's loop rotation to match target.
- mechanism: GCC 2.7.2's c-parse.y lowers do-while to a distinct RTL sequence (top NOTE_INSN_LOOP_BEG + condition-at-tail branch) which reorg.c's `relax_delay_slots` and loop-note-driven passes handle differently than an explicit `if (cond) goto label;` back-branch. If reorg picks a rotation with different LUID assignments, sched1's ready-list ordering on the FALSE-arm color triple could shift.
- probe: Edited src/text1a.c gnd_init_80041688 loop1 from goto/label to do-while form; ran `sandbox gnd_init_80041688 --disable all`.
- result: score REGRESSED 2 -> 7, build_insns 82 -> 83. Loop-shape lowering adds one insn (compensating uncond branch or duplicated test); natural form is strictly worse than goto/label. FALSE-arm sched1 lbu order unchanged.
- verdict: KILLED

## [s12] loop1 rewritten as top-tested `while (i < 18) { ... }` gives a different loop-note topology than do-while and might avoid the do-while regression while still perturbing reorg.
- mechanism: Top-tested `while` emits NOTE_INSN_LOOP_BEG immediately before the compare, with the fall-through path being the loop-exit; do-while has the compare at tail. Different note placement can steer reorg's delay-slot fill decisions on the back-branch.
- probe: Edited loop1 to `while (i < 18) { p += 0x68; ... i++; }`; ran sandbox.
- result: score REGRESSED 2 -> 7, build_insns 82 -> 83. Identical regression signature to do-while. GCC 2.7.2 lowers both natural-loop shapes to functionally equivalent RTL (both cost +1 insn vs manual goto/label). Exhausts the loop1-shape sub-axis.
- verdict: KILLED

## [s12] loop2 (currently goto/label with exit-in-middle test) rewritten as natural `while (*(s32*)(q+0x57) != 0)` — a head-tested loop that pulls the exit condition to the top — may alter q's live-range shape reaching the FALSE-arm and perturb pseudo/hardreg assignment.
- mechanism: loop2's current shape has a mid-block `if (... == 0) goto after2;` exit followed by an unconditional back-branch; converting to head-tested while makes the exit-test the loop-back branch itself. Different RTL loop structure could alter q's live-out set into the FALSE arm and shift greg conflict lists that affect the color-lbu triple's pseudo assignments (pseudos 78/79/80).
- probe: Edited loop2 to `while (*(s32*)(q+0x57) != 0) { if (arg1) *q |= 1; else *q &= ~1; q += 0x68; }`; ran sandbox.
- result: score REGRESSED 2 -> 11, build_insns 82 -> 84. Loop2's exit-in-middle shape is particularly poorly served by natural-loop lowering — two extra insns vs goto/label. goto/label form is strongly optimal for loop2 as well. FALSE-arm untouched (regression is entirely in loop2 body). Axis KILLED.
- verdict: KILLED

## [s13] Chassis-2 (split-init FALSE-arm form) as a permuter starting point opens a search neighborhood not reachable from chassis-1 and reaches sub-baseline sandbox scores.
- mechanism: Split-init `v=b; v|=r<<16; v|=g<<8;` produces distinct arm-local RTL SET topology vs chassis-1's `v=b|((r<<16)|(g<<8));`. A different starting neighborhood in permuter space might expose mutations invisible to chassis-1's basin.
- probe: Launched permuter campaign in tmp/grind/gnd_init_80041688/s13/perm/ (label=s13-chassis2-splitinit, base_score=625, -j 4). Ran ~100s / 3112 iters / 6 novel finds. Best = score 390 (permuter weights) — one-line mutation staging `(*(u8*)player+0x19)<<8` through the previously-unused `r` local (variable-reuse pattern).
- result: Best chassis-2 permuter score = 390 vs chassis-1's best = 10 (both permuter units). Chassis-2 basin is over an order of magnitude worse than chassis-1. No mutation approaches sandbox floor 0. The split-init structural axis (already KILLED at score=11 sandbox in s9) does not become a productive permuter chassis either.
- verdict: KILLED

## [s13] Chassis-3 (m2c-shape: shared v + shared gnd_load_tex outside arms) as a permuter starting point exposes cross-arm shape mutations that reach or beat chassis-1's basin.
- mechanism: Shared-v/shared-jal shape has different jump2/find_cross_jump interaction (per s8 measurement: base regresses 2->12 because jump2 merges the trailing OR into shared position). A permuter operating from that shape might discover mutations that break the merge without regressing lbu order.
- probe: Launched permuter campaign in tmp/grind/gnd_init_80041688/s13/chassis3/ (label=chassis3-shared-v-shared-call, base_score=745, -j 4). Ran ~1200 iters / ~30 novel finds. Best = score 185 (permuter weights) — output-185-1 splits FALSE-arm OR into two statements (`v=(r<<16)|(g<<8); v=b|v;`) — pure OR-tree partitioning, no byte-improvement.
- result: Best chassis-3 permuter score = 185 vs chassis-1's best = 10 (permuter units). Chassis-3 basin ~18x worse than chassis-1. All ~30 sub-745 findings are OR-tree splits, decl-order shuffles, or variable-reuse patterns — the shared-v shape's structural degrees of freedom are consumed by jump2 merging, leaving the permuter no path back through chassis-1's basin.
- verdict: KILLED

## [s13] SYNTHESIS: no unexplored permuter chassis with a lower or equal base_score than chassis-1 exists for this function; the permuter modality is exhausted.
- mechanism: All alternative score=2-baseline C source forms (s2 struct-cast, s3 named-intermediate-rg, s3 block-local cp, s8 inlined-loads FALSE-only, s8 inlined-loads both-arms) collapse to identical RTL as chassis-1 pre-sched1 via combine/CSE (documented as sandbox-byte-identical to chassis-1 in ledger). Any permuter run on those chassis would search the same neighborhood chassis-1 already dual-seed-exhausted. Higher-baseline chassis (chassis-2 base=625, chassis-3 base=745) proven decisively worse this session. No fourth chassis class remains.
- probe: Cross-session synthesis: chassis-1 dual-seed exhaustion (s4 + s5, ~50k iters combined), chassis-2 measurement this session (~3k iters), chassis-3 measurement this session (~1.2k iters). No sandbox-baseline-2 C form exists that produces different pre-sched1 RTL than chassis-1 per the s2/s3/s8 combine-collapse measurements.
- result: Every distinct permuter chassis in the reachable form space has been either fully searched (chassis-1) or proven basin-wise worse than the fully-searched one (chassis-2, chassis-3). No further chassis expansion is available.
- verdict: CONFIRMED

## [s14] Flipping g_player_ptrs' element type from s32 to u32 at its two extern decls in src/text1a.c will restructure the .rtl on the color reads enough to alter sched1's ready-set ordering on the FALSE-arm color-lbu triple (ledger frontier[1] mechanism claim).
- mechanism: Header-type-correction under [[header-type-correction-from-use-sites]] four-prong test; hypothesized to change LUID/RTL shape of the three color reads via a different global type declaration, orthogonal to pseudo-78 fusion and permuter search-space topology.
- probe: Edited both `extern s32 g_player_ptrs[];` sites (text1a.c:163, 898) to `extern u32 g_player_ptrs[];`. Ran `& tools/wteng.ps1 main sandbox gnd_init_80041688 --disable all`. Restored src, re-verified baseline.
- result: sandbox score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 — BYTE-IDENTICAL to baseline. Mechanistically disconnected from s6 lever (player is loaded once at entry then aliased via u8*; array-element type has no downstream RTL effect on BB18). Also FAILS four-prong test: prong (a) zero signed-specific use sites (every use is NULL check, ptr cast, or zero write), prong (b) no compensating cast is functionally necessary under s32, prong (d) no casts eliminated under u32.
- verdict: KILLED

## [s14] Flipping g_player_ptrs to `void*[]` (scalar->pointer) will restructure array-element reads enough to change sched1 ordering.
- mechanism: Same target-type-correction hypothesis, extended to the actual-semantic type (pointer array) rather than sign flip only. OUT OF SCOPE per rule line 183-185 ('Does NOT sanction changing a type from a struct/union/pointer to a scalar or vice versa'); measured purely for exhaustion.
- probe: Edited both extern sites to `extern void *g_player_ptrs[];`. Ran sandbox --disable all. Restored src.
- result: sandbox score=2, byte-identical to baseline. Combine/CSE folds the array-element load to the same RTL as under s32. Even a hypothetical byte-improvement here could not be committed via [[header-type-correction-from-use-sites]] — the rule excludes scalar<->pointer flips.
- verdict: KILLED

## [s14] A Kengo/scrape sibling function or use-site census provides positive-evidence signed semantics (or width/pointer-shape evidence) that would let a header-type flip clear prong (a) of the four-prong test.
- mechanism: Cross-project sibling ground-truth per [[header-type-correction-from-use-sites]] prong (a) tightening for sparsely-used globals — at least one qualifying signed-specific use-site is required.
- probe: grep g_player_ptrs / D_800A9A10 over Kengo/*.txt (all 0 hits — Kengo dumps are ASM/symbol tables only, addresses do not correspond to BB2's 0x800A9A10). grep 800A9A10 over tmp/decomp_me_corpus/ (5 hits, all false positives from unrelated games: func_8009E550/func_800A8E84/func_800A99A8). grep 'gnd' over corpus (2 hits: GZGND=func_800200A0 POLY_F4 setup unrelated; TIgnD=SOTN EntityNumericDamage). In-repo use-site census (12 sites in src/text1a.c): zero sites have signed-specific semantics (no `< 0`, no signed shift, no clamp, no round-toward-zero); every use is NULL check, pointer cast, or zero write.
- result: Zero sibling evidence available. Zero in-repo positive-evidence signed-semantics sites. Prong (a) unclearable regardless of measurement outcome — the 'absence of contradiction is not evidence of correctness' clause (rule line 45) forbids passing on the basis of consistency alone.
- verdict: KILLED

## [s15] The s6-CONFIRMED sched.c insn_priority hazard-tag differential on pseudo-78 at BB18 clock=4 remains the sole mechanism at the current baseline src state; no drift, no new axis, mechanism proof holds
- mechanism: Fresh instrumented cc1 -da run (tmp/grind/gnd_init_80041688/s15/s15_baseline_dbg.log) reproduces the s6 BB18 SCHEDDBG trace insn-for-insn: ready list at clock=4 contains all three FALSE-arm lbu-family insns [191(p=2130706433,l=4) 194(p=2130706433,l=6) 193(p=2130706433,l=5)] all boosted to 0x7F000001, luid tiebreak places insn 191 (b-lbu) at clock=4 -> emits LAST in reverse-clock code order, driving build lbu order [r,g,b] instead of target [b,r,g]. ALLOCDBG pseudo 78 (source `b`) -> hardreg 3 ($v1), nrefs=4 livelen=3 pri=26666 -- IDENTICAL to s6 numbers.
- probe: wsl bash tmp/grind/gnd_init_80041688/s15/run_cc1.sh with BB2_SCHED_DEBUG=BB2_ALLOC_DEBUG=BB2_PRIO_DEBUG=1; grep 'PICK clock' + ALLOCDBG lines; cross-check against s6 evidence [s6] entries
- result: s15_baseline_dbg.log lines 426-429 show `PICK clock=4 picked=191 (pri=2130706433 luid=4); ready was: [ 191(p=2130706433,l=4) 194(p=2130706433,l=6) 193(p=2130706433,l=5) ]`; lines 441 show `ALLOCDBG ord=0 pseudo=78 hardreg=3 nrefs=4 livelen=3 pri=26666` -- byte-identical to s6 dump.
- verdict: CONFIRMED

## [s15] gnd_init_80041688 is a textbook fit for the 2026-07-20 endgame-lock-disposition species (RA/scheduler-locked, byte-matched only via cheat, few insns short, sanctioned levers exhausted); both AND-gate criteria fail
- mechanism: Prong 1 (canonical-asm ONLY with hand-coded evidence): scan_hand_coded.py --single gnd_init_80041688 = HAND_CODED tier LOW score=0/8; no S1/S2/S3/S4/S5/S6/S7/S8 signals. Prong 2 (coercion family ONLY with SOTN precedent): 3 regfix.txt lines 477-479 are `subst` register/offset paperwork on lbu opcodes -- the archetypal `regfix offset paperwork` no-new-park-categories.md line 66-68 flags as a cheat; no SOTN precedent exists for build-time subst-rule closure of an lbu emission-order divergence.
- probe: `scan_hand_coded.py --single gnd_init_80041688`; cross-check against endgame-lock-disposition.md two AND-gates; verify Judge 2026-07-20 13:32 (a) already REFUSED canonical-asm at the Judge layer
- result: scan_hand_coded output: `HAND_CODED: tier=LOW  score=0/8  (gnd_init_80041688, 82 insns) Reason: no strong hand-coded indicators`. Judge 2026-07-20 13:32 (decisions.md line 1019-1029) refused canonical-asm citing identical grounds. Both AND-gates fail -> species disposition = INCOMPLETE-owner-accepted per rule.
- verdict: CONFIRMED

## [s16] The sched-solver's LEVER verdict (walking-pointer-serializes-parallel-loads) closes BB18's lbu emit order.
- mechanism: docs/grind/inverse-sched-2026-08-06.md maps the block-18 residual to
  `dep_add_true 183 <- 191` and routes it to
  [[walking-pointer-serializes-parallel-loads]], whose recorded symptom text is
  this exact three-parallel-loads shape. The rule's claim is that a
  post-increment pointer walk threads a dependence through the pointer and
  serializes the loads.
- probe: 12 else-branch spellings swept via tools/sweep_variants.py, plus an
  RTL-order trace (rtl -> combine -> sched -> lreg -> greg -> sched2 -> dbr) and
  BB2_PRIO_DEBUG/BB2_RANK_DEBUG on the instrumented cc1. Spellings: the rule's
  literal walk over 0x18..0x1A; a walk visiting +26 first then +24/+25; a
  split-pointer form (separate bp for +26, walked cp for +24/+25); locals in
  b,r,g and b,g,r order; fully inlined loads in the expression; inlined via a
  u8* base with [] indexing; b-in-temp with r/g inlined; three named-intermediate
  forms.
- result: KILLED. ALL 12 emit byte-identical code (`lbu +24, lbu +25, lbu +26`),
  sandbox 2, 82 insns. The walking-pointer mechanism CANNOT apply here: its two
  confirmed cases serialize via intervening MEMORY WRITES between the loads
  (stores to distinct globals), and this block has three loads feeding one
  expression with no store between them. GCC also folds every constant byte
  offset into the lbu displacement, so no pointer register survives to carry a
  dependence. NEW first-hand detail confirming s6/s15: the C statement order DOES
  reach sched1 — `b,r,g` produces target's exact pre-sched RTL order (+26, +24,
  +25) in .combine — and sched1 alone reverts it (.sched onward is stable). All
  three loads carry EQUAL priority, so the LUID tiebreak decides; but LUID is not
  the binding constraint either: putting the +26 load FIRST (vB) and LAST (vJ,
  named-rg intermediate) in RTL both emit +26 last, because bottom-up the +26
  load's only consumer is the final `or`, so it becomes ready strictly earlier
  than the +24/+25 loads (which wait for their slls and the inner or) and is
  therefore always picked early = emitted late. Only a real dependence edge
  delaying its readiness changes this, and three independent byte reads of the
  same object provide none (reads do not anti-depend on reads). The solver's
  remaining vectors are the volatile-spelled anti-dep (FORBIDDEN) and an
  insn_cost change (unspellable) — the report labels both correctly.
- verdict: KILLED

## [s16] SECOND, PREVIOUSLY UNDOCUMENTED CHEAT — the function's byte match also depends on a frame-coercion construct, not only the 3 regfix rules.
- mechanism: The body declares `volatile s32 sp10[8];` and discards it with
  `(void)sp10;`. `engine/volatile_cheats.py` (the `(void) <name>;` discard-cast
  detector, the scalar variant of the `(void)&local;` frame-coercion family)
  flags it, and `engine.inlineasm.write_stripped` strips it — so it is cheat-asm
  by the engine's own classification and `queue done` refuses it independently of
  the rule count.
- probe: Built NOSP variants (declaration + discard removed) over two else-branch
  spellings and scored them.
- result: CONFIRMED and LOAD-BEARING. Removing the construct regresses the honest
  distance 2 -> 8 at an unchanged 82 insns: the target genuinely reserves the
  32 frame bytes and the array is holding them. This is NOT recorded anywhere in
  the s1-s15 ledger, the park reason ("sched1 lbu emit-order (3 regfix)"), or the
  filed owner escalation — all of which describe the debt as 3 regfix rules only.
  Consequence: closing BB18's lbu order would NOT reach COMPLETED-C; the
  frame reservation is a separate residual worth 6 diffs and belongs to the
  [[phantom-slot-frame-lever]] diagnosis surface (find the honest producer of the
  8-word frame slot), not to the scheduler problem.
- verdict: CONFIRMED
- follow-up 2026-08-06: the 2 -> 8 measurement above needed a hand-built NOSP
  variant because `write_stripped` removed the `(void)sp10;` discard but left the
  declaration standing (GCC still reserved the 32 bytes, so the "honest" sandbox
  reported 2). `engine/volatile_cheats.py:find_orphaned_local_decls` now strips a
  declaration whose every remaining reference is itself stripped, and
  `sandbox gnd_init_80041688 --disable all` reports **8** directly — the recorded
  claim verified, no correction needed. The queue's recorded distance for this
  function (2) is stale until the next `queue regen`.

## s16 (2026-08-20, escalation modality) — chassis re-baselined; residual decomposed; two owner gates

- CHASSIS: src/text1a_post.c now; honest baseline = 8 (NOT the ledger's 2). Decomposition: 6 = stripped frame pad (sp10 has no _SANCTIONED_UNWRITTEN_PADS row) + 2 = the lbu order.
- KILLED: the 2026-08-20 or-tree carve-out's own axis — `(b|(r<<16))|(g<<8)` = 16/80insns; `((r<<16)|(g<<8))|b` = 9 (final-or operand flip only; lbu's are statement insns, unreachable from the expression tree). rejected/or-tree-carveout-groupings.c.
- KILLED: v-borrow blue-byte carrier = 12/80insns ($v0 home → identical tails → cross-jump merge). rejected/v-borrow-blue-byte.c.
- CONFIRMED (measurement): staged loop1 guard `b = *(s16 *)(p+2) >= 0; if (b)` → sandbox 6, FALSE arm byte-exact; residual = exactly the six pad-dependent frame immediates. Mechanism dump-proven on current chassis (s16 scheddbg.log block=18): birthing_insn_p reg_n_sets==1 launch-boost gate.
- FRONTIER (both owner-surface, ruling-request filed s16):
  1. Classification of the staged-guard spelling: rejected-bank s4/s5 "variable-reuse-with-dead-store" vs [[staged-value-reused-variable]] (whose Origin cites this exact reg_n_sets mechanism; value consumed by the branch). If in-family → honest floor 6.
  2. Pad allowlist row `"func_80041688": frozenset({("pre_pad", 8)})` (owner-only, engine surface; func_800481E8-identical state) + sp10 → first-declared `volatile u32 pre_pad[8];` respell. With both granted candidate.c should measure 0; integration retires regfix.txt:477-479 in the same step (they subst the lbu operands and would corrupt the already-correct stream).
- NO further in-session sanctioned lever exists: every non-owner-gated axis is measurement-dead across s1-s16.
