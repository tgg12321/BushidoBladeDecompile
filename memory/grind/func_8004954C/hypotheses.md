# Hypothesis ledger — func_8004954C

Function: triangular-sum leaf, text1b.c:837. Floor 6 (pure RA swap: sum↔i
between $v1/$a3). Structure is byte-exact vs target/m2c; only register
assignment differs. See evidence.md s1 for the measured wall mechanism.

## Live frontier (built s1, recon) — mechanism-grounded, ordered by EV

### F1 — Directed permuter campaign from candidate.c  (HIGHEST EV, UNTRIED)
- mechanism: the gap is a single-axis, 6-token RA rename with ALL structure
  (frame, 14 insns, delay slots, opcode order) already correct. That is the
  small, dense search space decomp-permuter is built to close. Random/PERM_*
  structural mutations may surface a byte-neutral C form that lifts sum's
  allocno priority above i's without a dead use.
- next_probe: build target.o from asm/funcs/func_8004954C.s + permuter prelude
  at offset 0 (per difficult-is-not-impossible §3 — avoids the whole-file
  offset-noise trap); base score should be the real weighted reg diff. Run
  tools/permuter_campaign.py with fresh-seed discipline; wait in-turn, harvest,
  --stop before ending the turn. Vet any sandbox-0 form against the cheat
  catalog BEFORE proposing (no dead stores / pins / reassoc).

### F2 — Natural post-loop use of i that faithfully equals arg1  (LOW EV)
- mechanism: equalizing i.live_length up to sum's (both to the return) would
  make priority a near-tie; the allocno-number tiebreak (sum=75 < i=76) then
  allocates sum first → sum gets $v1. BUT: any post-loop use of i emits with
  i's register (not $a1), breaking the target `subu v0,a2,a1`; and a use that
  doesn't affect output is a dead-use cheat. Recorded as the theory of the
  wall; probe only to CONFIRM it's a dead-end (measure a faithful form if one
  exists). Expect KILLED.

### F3 — Raise sum's loop-ref count via a semantically-motivated recompute (LOW EV)
- mechanism: sum loses the priority race partly on freq (2 loop refs vs i's 3).
  A form where the accumulator is genuinely referenced more per iteration could
  flip freq — but the target loop body is exactly `addu v1,v1,a0` (2 refs); any
  extra ref changes the emitted loop bytes. Likely no faithful form exists;
  measure to KILL or to feed the permuter a better seed.

## Rejected forms bank (do NOT re-propose — all measured dead)
- `(sum + arg2) - arg1` paren-reassoc (score 2) — FORBIDDEN or-tree-shape-shift.
  See rejected/paren_reassoc.c.
- `sum = sum + arg2; return sum - arg1;` two-statement split (score 2) — same family.
- `volatile unsigned int pad;` + `register asm($7/$3)` pins + `(void)var_a3;`
  (HEAD floor 9) — dead-vars / pins / dead-use cheats; also WORSE than clean floor.
- declaration-order swaps, `register` keyword, block-local i, arg0 local-alias,
  `(arg2-arg1)+sum` commutative reorder — all no effect (stay at 6).
- `(i - i)` return term — folded, no IR extension.
- `for` vs `while` vs `if+do-while` — only `for` emits the 14-insn/8-byte-frame
  structure; the others collapse to 11 insns.

## [s1] The floor-6 gap is a pure register-allocation swap driven by allocno priority, not a tie fixable by declaration order.
- mechanism: global.c allocno_compare priority = floor_log2(n_refs)*freq*size/live_length. From target asm: counter i has 3 loop refs (addiu a3,1 use+set; slt a3,a1 use) vs accumulator sum's 2 (addu v1,v1,a0 use+set), AND i dies at the loop while sum lives to the return addu v0,v1,v0. i wins on BOTH freq and live_length, so it sorts strictly ahead of sum (not a tie) and grabs the lower reg $v1.
- probe: Fresh greg dump of the applied candidate + manual ref/live-range count against asm/funcs/func_8004954C.s and m2c output.
- result: greg reproduces prior v02 exactly: '6 regs to allocate: 72 76 75 73 80 74', dispositions 75 in 7 (sum->$a3), 76 in 3 (i->$v1). i(76) allocated before sum(75). m2c shape is byte-identical to candidate.c; only RA differs.
- verdict: CONFIRMED

## [s1] No trivial faithful restructure can remove i's 3rd loop reference to equalize priority.
- mechanism: The byte target slt $v0,$a3,$a1 forces an up-counter compared to arg1 (arg1 must stay live in $a1 for the final subu v0,a2,a1). Count-down, i-->0, or arg2-i forms either destroy arg1's liveness or emit the compare/subtract with the wrong register, breaking the slt/subu bytes.
- probe: Enumerated the byte constraints from target asm against candidate structures (count-down, i-- > 0, arg2-i offset).
- result: Every alternative loop form breaks target's 'slt vs a1' and/or 'subu v0,a2,a1' bytes, so i's separate test-ref is structurally mandatory. The counting-up i inherently has 3 loop refs.
- verdict: CONFIRMED
