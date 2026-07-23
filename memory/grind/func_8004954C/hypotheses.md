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

### F2 — Natural post-loop use of i that faithfully equals arg1  (KILLED s3)
- mechanism: equalizing i.live_length up to sum's (both to the return) would
  make priority a near-tie; the allocno-number tiebreak (sum=75 < i=76) then
  allocates sum first → sum gets $v1.
- result (s3): KILLED. Extending i to the return RAISES the score (6→7), not
  lowers it — i still outranks sum on the freq axis (3 refs vs 2), and forcing i
  to hold a hard reg through the epilogue breaks target's `subu v0,a2,a1`.
  Also unfaithful: i==arg1 only for arg1>0. No faithful post-loop use of i
  exists. See rejected/f2_extend_i_liverange.c.

### F3 — Raise sum's loop-ref count via a semantically-motivated recompute (KILLED s3)
- mechanism: sum loses the priority race partly on freq (2 loop refs vs i's 3).
  Raising sum's per-iteration refs would flip freq.
- result (s3): mechanism CONFIRMED but only via a cheat. split-add
  (`sum += arg0/2; sum += arg0 - arg0/2;`, == `sum += arg0`) drops score 6→3
  (build 14) by lifting reg_n_refs(sum) — but it is a no-semantic-purpose
  redundant-arithmetic split, cheat-reviewer FAIL. A faithful extra read
  (`s32 t = sum+arg0; sum=t;`) stays at 6 (DCE'd before n_refs counting).
  No faithful ref-raise exists → F3-as-legitimate-lever KILLED.
  See rejected/f3_split_add_nref_lift.c.

### F1 — Directed permuter campaign from candidate.c  (HIGHEST EV, UNTRIED — DIFFERENT MODALITY)
- Now the ONLY remaining live avenue after structural is comprehensively dead.
  This is the permuter modality, not structural — for a future permuter session.
  mechanism/probe unchanged from the s1 F1 entry below (build target.o from
  asm/funcs/func_8004954C.s at offset 0, fresh-seed campaign, wait+harvest
  in-turn, cheat-vet any sandbox-0 form). If the permuter also fails, every
  sanctioned pure-C axis is dead → endgame-lock / owner-gated.

## Rejected forms bank (do NOT re-propose — all measured dead)
- `s32 off = arg2 - arg1;` HOISTED before loop, `return sum + off` (score 4, s2) —
  REJECTED cheat-reviewer FAIL. Shortens sum's live range across the loop (single final
  `addu` vs `subu;addu`) to flip sum/i allocno priority; same species as paren-reassoc,
  spelled as a named local. `off_temp_late` (same code, offset computed at END) reverts
  to 6 — proving the placement is codegen-motivated. See rejected/off_early_livrange_steer.c.
- [s2] ALL subu-at-end structural forms measured at 6: cmp-swap (`arg1 > i`), do-while,
  postdec (`sum+=arg0--`), sum-explicit, pre-increment, every decl/stmt order of
  {off,sum,i}, off-split, off-late-assign, bound-var, off-commute. No structural form
  flips RA with subu-at-end.
- [s2] Type narrowing dead: i=s16 -> 5 (build 17, worse); i=u32 -> 7.
- [s2] Count-down dead: `n=arg1;while(n>0)` -> 9; `for(i=arg1;i!=0;i--)` -> 10.
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

## [s2] A named-intermediate hoist `s32 off = arg2 - arg1;` before the loop is a legitimate structural lever that flips the sum/i register allocation.
- mechanism: Hoisting off makes the final return a single `addu v0,sum,off` instead of baseline `subu;addu`, shortening sum's live range by one insn; that raises sum's allocno priority (global.c:604 = flog2(nrefs)*nrefs/live_length) above counter i's, so sum wins $v1 and i wins $a3 (matches target). Sandbox drops 6->4.
- probe: Applied off-early, sandbox --disable all -> score 4 (RA correct per isolated greg dump: sum(76)->$v1, i(77)->$a3, off(75)->$a2); residual 4 is pure delay-slot scheduling (subu carried in $a2 fills the blez delay slot). Submitted to cheat-reviewer.
- result: score 4, RA correct, but cheat-reviewer FAIL: same species as the already-rejected paren-reassoc (allocno-priority steering by shortening sum's live range), re-spelled as a named local. Decisive: my own off_temp_late counter-experiment (offset computed near natural use at END) reverts to 6, proving placement is codegen-motivated, not SOTN named-intermediate.
- verdict: KILLED

## [s2] Some legitimate subu-at-end structural form (compare-swap, do-while, postdec, explicit accumulate, pre-increment, declaration/statement reorder, split, bound-var) flips the RA so sum wins $v1 without shortening sum's live range.
- mechanism: If a structural variant raises sum's priority or lowers i's without moving the subtraction pre-loop, it would match target (subu-at-end AND sum->$v1) at score 0.
- probe: Swept 12 subu-at-end variants via tmp/grind/func_8004954C/s2/sweep3.py + sweep2.py measuring sandbox --disable all.
- result: Every subu-at-end form stays at score 6. i strictly outranks sum in all of them (3 loop refs + shorter live length vs sum's 2 refs + longer live). No structural lever flips RA with subu-at-end.
- verdict: KILLED

## [s2] Type narrowing the counter/accumulator (s16/u32) or a count-down loop reaches below 6.
- mechanism: Different integer width or loop direction could change ref counts / codegen shape favorably.
- probe: i=s16, i=u32, separate down-counter n, for(i=arg1;i!=0;i--) — sandbox each.
- result: i=s16 -> 5 but build_insns 17 (extra sign-extends, worse structure, not a real improvement); i=u32 -> 7; down-counter n -> 9; for-down -> 10 (break the 14-insn/slt-vs-a1 structure). All worse.
- verdict: KILLED

## [s3] Extending counter i's live-length to the return (F2) equalizes live_length with sum and lets the allocno-number tiebreak allocate sum first, flipping RA so sum wins $v1 (match).
- mechanism: global.c:604 allocno_compare priority = flog2(nrefs)*nrefs/live_length; if i.live_length rises to equal sum's, priority becomes a near-tie broken by lower allocno number (sum=75 < i=76).
- probe: Measured `return sum + (arg2 - i)`, a bound-var decoupled variant, and a forced-correct `if(arg1<=0) i=arg1;` form via cheat-invisible sandbox (tmp/grind/func_8004954C/s3/sweep_f2f3.py).
- result: Scores rose to 7, 7, and 10/build17 respectively (baseline 6). Extending i to the return makes it WORSE: i still outranks sum on the freq axis (3 loop refs vs 2), and forcing i to hold a hard reg through the epilogue breaks target's subu v0,a2,a1 (emits against i's reg, +1 diff). Also unfaithful: i==arg1 only for arg1>0.
- verdict: KILLED

## [s3] Raising sum's per-iteration reference count (F3) flips the freq axis so sum outranks i and wins $v1, via a semantically-faithful restructure.
- mechanism: sum loses partly on freq (2 loop refs vs i's 3). Raising reg_n_refs(sum) lifts sum's allocno_compare priority (flog2(nrefs)*nrefs term).
- probe: Measured split-add `sum += arg0/2; sum += arg0 - arg0/2;` (==sum+=arg0), a real-temp extra read `s32 t=sum+arg0; sum=t;`, and the split+F2 combo; sandbox each; ran cheat-reviewer on the split-add.
- result: Split-add drops score 6->3 (build_insns unchanged at 14) — the mechanism WORKS (lifts reg_n_refs(sum), flips RA toward sum) — but it is a no-semantic-purpose redundant-arithmetic split; independent cheat-reviewer verdict FAIL (fails semantic-purpose, human-programmer, GCC-internals-justification, family tests). The faithful real-temp form stays at 6 (DCE deletes the extra read before n_refs counting). No faithful ref-raise exists, so F3 as a legitimate lever is dead; it also never reaches 0 (residual 3).
- verdict: KILLED

## [s4] F1 permuter — CONFIRMED: do-while(0) loop-body wrap closes the sum/i RA gap to score 0.
- mechanism: a single-level `do { sum += arg0; arg0 -= 1; } while (0);` around the
  loop body emits a GCC loop-note that shifts allocno ref weighting so accumulator
  sum outranks counter i, seating sum->$v1 and i->$a3 (target). This is the
  RA-weighting do-while(0) effect sanctioned by owner ruling 2026-07-06
  (do-while-zero-exception.md), same class as the named marionation_Exec precedent.
- probe: isolated-leaf decomp-permuter workspace (tmp/grind/func_8004954C/s4/ws),
  target.o at offset 0, base_score 40, -j8 --stop-on-zero. Found score-0 at iter 278
  (23.1s). Applied to src with FAKE annotation; sandbox --disable all = 0, build 14,
  0 rules, 0 cheat-asm.
- result: MATCH. Single-level, semantically-true, FAKE-annotated; structural axis
  already exhausted (s2/s3) so "prefer natural geometry" satisfied. Candidate saved.
- verdict: CONFIRMED (byte-match; pending layer-1/2 cheat-reviewer acceptance).
