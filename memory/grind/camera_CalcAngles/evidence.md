# Evidence bank — camera_CalcAngles

- Audit diagnosis (regressions.md): Dead store `sp18[2] = a1` (line 543, src/sound.c): sp18[2] is written but never read afterward — the next call uses `a1` directly from register, not sp18[2] from the stack. The store has no semantic purpose (Test 1 fails) and exists only to emit the sw instruction in the target asm. Next action: sandbox-verify Form A — replace `single_game_getEnemyCharId(sp18[1], a1)` with `single_game_getEnemyCharId(sp18[1], sp18[2])` (dropping the explicit dead store). If Form A still matches, it is the cleaner original-intent decomp and the dead store was a codegen coercion; commit Form A as the replacement. If Form A breaks the match, escalate for investigation of the original call-site form.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct: `sp18[2] = a1;` at src/sound.c:543 with the next call (line 544) taking `a1` directly — the store is dead in the current SOURCE spelling (no later read of sp18[2])

- [s1] [fable-blitz 2026-07-07] The store is REAL in target: asm/funcs/func_80047384.s:47-49 emit `sra $a1,$a1,12; jal single_game_getEnemyCharId; sw $a1,0x20($sp)` — the sw sits in the jal delay slot, and lines 50-59 never read 0x20($sp) again, so the ORIGINAL binary also carries a dead store to sp18[2]; the original source plausibly wrote the value INTO sp18[2] and passed sp18[2]

- [s1] [fable-blitz 2026-07-07] Why the store survives compilation: sp18's address escapes (asm line 14: `addiu $a2,$sp,0x18` passed to func_8007ED6C), so GCC 2.7.2 treats the array as addressable memory and cannot DCE stores to it — a live-in-source `sp18[2] = expr;` is guaranteed to emit the sw

- [s1] [fable-blitz 2026-07-07] Form A mechanism (judge's proposed fix): `single_game_getEnemyCharId(sp18[1], sp18[2])` after the store — cse.c read-after-write equivalence (the store makes mem(sp+0x20) equivalent to the $a1 pseudo within the basic block) should pass the register copy as arg2 with NO reload, leaving the bytes identical; the arg1 load `lw $a0,0x1C($sp)` (asm line 44) already schedules early exactly as target

- [s1] [fable-blitz 2026-07-07] The `a1` local itself may be unnecessary in Form A: the pseudo lands in $a1 naturally because its only uses are the store + the call's arg2 (mflo $a1 at asm line 40, addu/sra at 46-47); folding the expression straight into `sp18[2] = (...) >> 12;` is a candidate simplification to sweep alongside Form A

- [s1] [fable-blitz 2026-07-07] Fallback policy note: even if Form A's CSE reloads instead of forwarding, the target-bytes-contain-the-dead-store situation is exactly the [[dead-store-fake-exception]] / written-array carve-out shape (2026-07-01) — a /* FAKE */-annotated store would be sanctionable after lever exhaustion; but Form A is the clean original-intent form and should be tried first
