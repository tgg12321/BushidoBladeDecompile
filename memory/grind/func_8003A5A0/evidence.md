# func_8003A5A0 — evidence ledger

## Session 1 (recon) — 2026-08-11 — floor 21 → 0

### Baseline
- `canonical func_8003A5A0` → verdict **C**, 0 asm insns, 87 target insns, distance 21.
  Pure-C target; no canonical-asm question exists for this function.
- `sandbox func_8003A5A0 --disable all` → **21**, build 88 insns vs target 87,
  21 regfix rules dropped, 35 cheat-asm instructions stripped (file-wide count).
- All 21 rules (`regfix.txt:140-160`) are register substitutions swapping `$2`↔`$3`
  over maspsx indices 42-76, plus `subst "addu.*" "nop" @ 73` and `delete @ 79`.
  Read literally, the rule cluster says: our build's register assignment is the exact
  mirror of target's ($v0 where target has $v1 and vice versa) across the poll and
  checksum regions, and our build emits one instruction too many.

### The instruction-level diagnosis (tmp/grind/func_8003A5A0/s1/cmp2.py)
The scratch comparator disassembles the sandbox object and matches it word-for-word
against the hex in `asm/funcs/func_8003A5A0.s`, printing only mismatches. Relocation
sites (`jal`, `%gp_rel`, `%hi/%lo`, `j .L`) are always unequal in an unlinked object
and are noise; there were 22 such lines throughout, and at the end exactly ZERO
non-relocation mismatches remained.

Baseline residual (before any edit), read off that comparator + objdump:
1. Poll region — target `sra $v0,$v0,7; andi $v0,$v0,3; li $v1,1; beq $v0,$v1`;
   ours `sra $v0,$v0,7; andi $v1,$v0,3; li $v0,1; beq $v1,$v0`. Mirror image.
2. Timeout region — target `subu $v0,$v0,$s0; slti $v0,$v0,0x3C01`; ours
   `subu $v1,$v0,$s0; slti $v0,$v1,0x3C01`. Mirror image.
3. Checksum region — same mirror on all seven instructions.
4. Two extra `move $v0,$v1` copies (one at the shared join `.L`, one in a `j`
   delay slot) that target does not have; hence 88 vs 87.

### Lever 1 — CONFIRMED: shared-local overload cost $v0 (21 → 16)
The pre-session body reused ONE local `v0` for three unrelated roles: the poll
temporary, the timeout temporary, and the return value. That makes a single
multi-block allocno whose union of conflicts loses $v0 to the short block-local
temps — `local_alloc` runs before `global_alloc` and hands out the first free hard
reg to block-local pseudos, so $v0 goes to the temps and the multi-block pseudo
takes $v1, mirroring the entire function.

Probe: write both poll conditions as direct expressions with no variable —
`if (((_comb_control(0,0,0) >> 7) & 3) == 1)` and
`if (GetRCnt(0xF2000001) - s0 < 0x3C01)`.
Result: **21 → 16**, and the whole poll + timeout region became byte-exact
(`sra $v0,$v0,7; andi $v0,$v0,3; li $v1,1; beq $v0,$v1` and
`subu $v0,$v0,$s0; slti $v0,$v0,0x3C01`) — the compare temps coalesced with the
call return already in $v0, and the constant 1 fell to $v1 exactly as in target.

### Lever 2 — CONFIRMED: the shared `epilogue: return v0;` join (16 → 6)
Target's `.L8003A6E4` is a bare epilogue (`lw $ra; lw $s1; lw $s0; addiu $sp; jr`).
Ours had a `move $v0,$v1` immediately before it plus a second copy in a delay slot —
the classic shared-join copy. Replacing the join with a `return` at each exit
(`return 0;` at the retry-exhausted paths and the checksum-mismatch path,
`return 1;` at match) removed the copies and put the value in $v0 directly.
Result: **16 → 6**. The checksum block then matched target's registers exactly.

### Lever 3 — CONFIRMED: ONE shared `ret0_tramp` label for both retry exits (6 → 0*)
The two `if (s1 >= 5)` exits are the residual's whole story. Three spellings measured:
- BOTH inline `return 0;` → **6**, build 85 insns (2 SHORT). jump2's
  `find_cross_jump` merged the two identical `[set $v0,0; jump epilogue]` blocks
  into one fall-through block at the poll site, and inverted the poll branch to
  `bnez → overflow` (target has `beqz → epilogue`).
- loop_check inline `return 0;` + poll `goto ret0_tramp;` → **6**, 87 insns. The poll
  region became byte-exact (reorg stole `$v0=0` from the trampoline into the branch's
  empty delay slot and redirected it to the epilogue) but the loop_check exit's block
  was emitted INLINE with an inverted `bnez` skip instead of target's out-of-line
  `beqz → .L8003A6CC` trampoline.
- TWO DISTINCT labels (`ret0_tramp` after the mismatch block, `ret0_poll` after
  `match`) → **7**, 87 insns. They re-merged anyway — confirming the
  [[cross-jump-store-tail-merge]] note that distinct labels forwarding to the same
  `return G` get coalesced — and the survivor landed after `match`, which destroyed
  match's fall-through into the epilogue (`li $v0,1` moved into a `j` delay slot).
- ONE shared label `ret0_tramp` used by BOTH exits → **1**. Exactly target's shape:
  the loop_check branch keeps the out-of-line 2-insn trampoline (its delay slot is
  already occupied by the fall-through's `addiu $a0,$zero,2` argument setup, so reorg
  cannot consume the trampoline for it), while the poll branch — whose fall-through
  delay slot is free — gets `$v0 = 0` copied into it and is redirected straight to
  `.L8003A6E4`. The trampoline survives because it still has the other predecessor.

### Lever 4 — CONFIRMED: expand_binop's commutative swap on the first xor (1 → 0)
Last residual, one instruction: target `xor $v0,$a1,$v0`, ours `xor $v0,$v0,$a1`.
- KILLED: changing SOURCE operand order (`v0 = a1 ^ v0;` instead of `v0 = v0 ^ a1;`)
  does nothing — still **1**. The swap is not a source-order effect.
- CONFIRMED: it is `expand_binop`'s commutative-operand swap, which fires when the
  destination is the same rtx as operand 1 (it swaps to avoid an extra copy). In
  `v0 = a1 >> 16; v0 = v0 ^ a1;` the accumulator is both dest and op1. Writing the
  fold as ONE expression, `v0 = a1 ^ (a1 >> 16);`, gives a destination distinct from
  both operands at expand time (the shift result is an anonymous temp that RA later
  coalesces into $v0), no swap fires, and the emitted order matches target.
Result: **1 → 0**.

### Final state
`sandbox func_8003A5A0 --disable all` → **score 0**, 87/87 instructions, with all 21
regfix rules dropped and cheat-asm stripped. The comparator's only remaining
differences are the 22 relocation sites, which are structurally unequal in any
unlinked object. The body is ordinary C: no `volatile`, no `register asm()` pin, no
`__asm__` of any kind, no dead store, no unused declaration, no cast added, no
`/* FAKE */` construct. Net, the session REMOVED source constructs (one overloaded
local's extra roles, the shared-join return variable, one statement) rather than
adding any.

### Notes for whoever integrates this
- The 21 regfix rules at `regfix.txt:140-160` are now all redundant and are the
  function's remaining queue debt; `retire func_8003A5A0` + `queue done` is the
  operator's step (this session may not touch rule files, run retire, or commit).
- `sdata_exclude.txt:20` (`func_8003A5A0: D_800A368C, D_800A36C4`) is untouched
  pre-existing pipeline config and is still load-bearing — target reads/writes those
  two globals through `%hi/%lo` rather than `$gp`, which the current build reproduces.
