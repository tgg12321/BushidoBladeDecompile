# Evidence bank — func_800400F8

- Audit diagnosis (regressions.md): Pre-comparison `s0 = 0;` + `if (s2[0] > s0)` is a blez-delay-slot scheduling coercion; clean fix: reorder `s0 = 0; s1 = s2;` (not `s1 = s2; s0 = 0;`) inside the if block and remove the pre-comparison assignment, which naturally puts `move s0, zero` in the delay slot without dead code.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Construct located: src/config.c:553 pre-comparison `s0 = 0;`, :554 compare-vs-variable `if (s2[0] > s0)`, :556 redundant second `s0 = 0;` inside the block.

- [s1] [fable-blitz 2026-07-07] Target-byte shape: `blez $v0, .L80040150` with `addu $s0, $zero, $zero` in the DELAY slot (asm/funcs/func_800400F8.s:13-14), then `addu $s1, $s2, $zero` (s:15). Exactly ONE s0-init exists in the bytes -- the committed duplicate at :556 is CSE-deleted (value already 0); the :553 copy is what seats the init at the branch.

- [s1] [fable-blitz 2026-07-07] The compare-vs-variable spelling is const-folded anyway: the target compares against literal zero (`blez $v0`), i.e. GCC const-propagated s0=0 into the compare -- so the judge's literal `if (s2[0] > 0)` is byte-equivalent on the compare side by construction.

- [s1] [fable-blitz 2026-07-07] Judge's fix mechanism check: with `if (s2[0] > 0) { s0 = 0; s1 = s2; do ... }`, reorg.c fill_simple_delay_slots may steal the FIRST fall-through insn (the s0-init) into the blez slot; the safety condition holds -- s0 is dead on the taken path (.L80040150 is the epilogue, s0 only restored from stack, s:29). Ordering matters: s0 = 0 must precede s1 = s2 or the slot candidate becomes the s1 copy.

- [s1] [fable-blitz 2026-07-07] Preserve during the redo: the loop-bottom re-read `while (s0 < s2[0])` compiles to a fresh `lh $v0, 0x0($s2)` per iteration (s:20) -- keep the memory re-read spelling; and the jal delay slot already gets the s1 advance (s:19) from the :558-559 statement order (arg read, then advance). Both are natural and already match.

- [s1] [fable-blitz 2026-07-07] Fallback note: the committed form is semantically-TRUE C (the :553 value genuinely feeds the compare), so if the natural-geometry sweep unexpectedly fails, a FAKE-annotated retention is available under the 2026-07-06 construct-honesty line -- but the judge's fix is expected to close it cleanly, so no ruling question is filed.

- [s2] [structural 2026-07-14] JUDGE FIX FALSIFIED: the predicted clean respell (literal `> 0` compare + `s0 = 0; s1 = s2;` first inside the block) scores 16, not 0. Two independent failures: frame 0x20 vs target 0x28, and RA swap (counter->$s1, pointer->$s0). Full ordering/decl sweep of the literal-compare if+do-while family: 12 (s1-first), 16 (block-local decls; decl-order swap; hoisted init), 17 (both inits hoisted). Best-in-family 12.

- [s2] [structural 2026-07-14] FRAME MECHANISM FOUND (cc1 -da, tmp/grind/func_800400F8/s2/cc1/): the target's 0x28 frame comes from a combine-leftover `(use (reg 79))` — the slt temp of the folded variable-compare guard. cse folds `s2[0] > s0` (s0=0 known) to blez; combine leaves a USE of the dead slt pseudo; the pseudo gets no hard reg and reload assigns it a stack slot at sp+20 -> +8 frame bytes, visible as `(use (mem sp+20))` in .greg/.sched2. The literal-compare spelling leaves no such pseudo -> frame 0x20. TARGET'S FRAME IS ITSELF EVIDENCE the original source compiled a folded variable-compare guard.

- [s2] [structural 2026-07-14] BEQZ-SLOT MECHANISM FOUND: target's beqz delay slot is nop because the first fall-through insn is the guard `lh v0,0(s2)` — may_trap_p (only safe under beqz s2!=0), so reorg's fall-through steal scan stops at it. Any trap-free init move sitting between beqz and the lh IS stolen into the slot. sched1 hoists the lh above a pre-compare `s0 = 0` within one block (v6 dump: "launching 23 before 25"), but cannot hoist it above pre-loop inits in while/for spellings because jump1's duplicate_loop_exit_test inserts the guard AFTER all pre-loop statements (separate blocks).

- [s2] [structural 2026-07-14] WHILE/FOR FAMILY: `while (s0 < s2[0])` (and for-loop, all 4 init orderings incl. comma-for) scores exactly 2 / 28 insns — reproduces frame 0x28 AND correct RA naturally (the loop condition IS a variable compare), but reorg steals `move s1,s2` into the beqz slot (target: nop + `addu s1,s2` after blez). Structurally unreachable within this family per the two mechanisms above.

- [s2] [structural 2026-07-14] SCORE-0 FORMS: exactly two found — v0 (committed, with dead inner `s0 = 0;` dup) and v6 (`s0 = 0; if (s2[0] > s0) { s1 = s2; do ... }`, dup REMOVED, every statement live). Both are the judge-banned family's letter. v6 verified sandbox --disable all = 0 this session. The inner dup is inert (v9: literal compare + dup = 16), so the compare-vs-variable is the sole load-bearing element.

- [s2] [structural 2026-07-14] Constant-holder respell KILLED: `s32 zero = 0; if (s2[0] > zero)` with real inits inside the block scores 13/18 at 30 insns — the holder's init survives the fold and emits an extra insn. Not byte-neutral.
