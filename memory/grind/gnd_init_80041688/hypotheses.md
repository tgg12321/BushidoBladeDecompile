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
