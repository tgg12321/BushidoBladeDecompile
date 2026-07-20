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
