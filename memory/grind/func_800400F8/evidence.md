# Evidence bank — func_800400F8

- Audit diagnosis (regressions.md): Pre-comparison `s0 = 0;` + `if (s2[0] > s0)` is a blez-delay-slot scheduling coercion; clean fix: reorder `s0 = 0; s1 = s2;` (not `s1 = s2; s0 = 0;`) inside the if block and remove the pre-comparison assignment, which naturally puts `move s0, zero` in the delay slot without dead code.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Construct located: src/config.c:553 pre-comparison `s0 = 0;`, :554 compare-vs-variable `if (s2[0] > s0)`, :556 redundant second `s0 = 0;` inside the block.

- [s1] [fable-blitz 2026-07-07] Target-byte shape: `blez $v0, .L80040150` with `addu $s0, $zero, $zero` in the DELAY slot (asm/funcs/func_800400F8.s:13-14), then `addu $s1, $s2, $zero` (s:15). Exactly ONE s0-init exists in the bytes -- the committed duplicate at :556 is CSE-deleted (value already 0); the :553 copy is what seats the init at the branch.

- [s1] [fable-blitz 2026-07-07] The compare-vs-variable spelling is const-folded anyway: the target compares against literal zero (`blez $v0`), i.e. GCC const-propagated s0=0 into the compare -- so the judge's literal `if (s2[0] > 0)` is byte-equivalent on the compare side by construction.

- [s1] [fable-blitz 2026-07-07] Judge's fix mechanism check: with `if (s2[0] > 0) { s0 = 0; s1 = s2; do ... }`, reorg.c fill_simple_delay_slots may steal the FIRST fall-through insn (the s0-init) into the blez slot; the safety condition holds -- s0 is dead on the taken path (.L80040150 is the epilogue, s0 only restored from stack, s:29). Ordering matters: s0 = 0 must precede s1 = s2 or the slot candidate becomes the s1 copy.

- [s1] [fable-blitz 2026-07-07] Preserve during the redo: the loop-bottom re-read `while (s0 < s2[0])` compiles to a fresh `lh $v0, 0x0($s2)` per iteration (s:20) -- keep the memory re-read spelling; and the jal delay slot already gets the s1 advance (s:19) from the :558-559 statement order (arg read, then advance). Both are natural and already match.

- [s1] [fable-blitz 2026-07-07] Fallback note: the committed form is semantically-TRUE C (the :553 value genuinely feeds the compare), so if the natural-geometry sweep unexpectedly fails, a FAKE-annotated retention is available under the 2026-07-06 construct-honesty line -- but the judge's fix is expected to close it cleanly, so no ruling question is filed.
