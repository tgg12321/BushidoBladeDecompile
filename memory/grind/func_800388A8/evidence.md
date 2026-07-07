# Evidence bank — func_800388A8

- Audit diagnosis (regressions.md): Empty-body 'if (D_800A3318 != 0) { }' is a scheduling-direction cheat: semantically identical to 'if (D_800A3318 == 0) { result = 1; }' but the backwards condition + empty true body exists solely to force a bnez branch (not beqz) so GCC's delay-slot filler can hoist the pre-call 'result = -1' assignment into the bnez delay slot. Worker must rewrite with the condition un-inverted and result=-1 placed inside the if-body (or equivalent non-cheat form that produces the same scheduling naturally), then re-close.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct at src/code6cac_c_mid.c:730-735: `result = -1;` before the call, then `if (D_800A3318 != 0) { } else { result = 1; }` - empty true arm, inverted condition. Diagnosis confirmed as cheat-shaped: the empty arm has no semantic purpose and exists to steer branch sense; NOT covered by do-while-zero or any 2026-07-01 sanction (those cover dead stores/dup statements, not empty control arms).

- [s1] [fable-blitz 2026-07-07] Target fine structure (asm/funcs/func_800388A8.s:21-26): lbu v0,D_800A3318; NOP (load-delay, line 22); bnez v0,.L80038958 with slot `addiu $s0,-1` (lines 23-24); j .L80038958 with slot `addiu $s0,1` (lines 25-26). The -1 set is NOT before the jal and NOT in the load-delay position - it is in the bnez slot specifically.

- [s1] [fable-blitz 2026-07-07] That layout is the canonical reorg.c own-thread-steal signature for `if (x == 0) { result = 1; } else { result = -1; }`: jump.c emits bnez v0,L_else (branch-if-false of ==0); L_else's single insn `s0=-1` is own-thread (reachable only via the branch) so reorg steals it into the slot and redirects the branch to the join; the then-arm's `s0=1` hoists into the j delay slot. Both target slots explained with zero fake constructs.

- [s1] [fable-blitz 2026-07-07] The committed pre-call `result = -1;` (line 730) is dead-in-place in target - no `addiu $s0,-1` exists before the jal (asm lines 17-20: a0/a1/a2 arg setup only), so dropping it in favor of the else-arm assignment loses nothing.

- [s1] [fable-blitz 2026-07-07] Context guards the shape: this is the first arm of a 3-way else-if chain (buttons & 0x400040 / 0x80008000 / 0x20002000, src lines 729-742); the arm must end with j to the chain join .L80038958 (asm line 25) - satisfied naturally since the else-if chain follows.

- [s1] [fable-blitz 2026-07-07] `result = 0` initializer maps to prologue `addu $s0,$zero,$zero` (asm line 5) and the s0-based return/D_800A3204 tail (asm lines 53-57) - untouched by the respelling.
