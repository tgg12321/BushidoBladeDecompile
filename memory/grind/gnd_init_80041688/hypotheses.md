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
