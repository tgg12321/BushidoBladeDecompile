# Evidence bank — func_80045294

## s1 (recon, 2026-07-18)

- **Floor: 2** (sandbox --disable all; 83/83 insns; 0 rules, 78 cheat-asm-stripped lines are file-wide, not this func).
- **Exact residual diff** (tmp/grind/func_80045294/s1/diff_baseline.txt, target extracted from
  disc/SLUS_006.63 @ file offset 0x35A94, 83 insns): the ONLY divergence is one prologue cluster —
  - target: `sw $16,16($sp) ; move $16,$18 ; sll $3,$18,0x4`
  - build:  `sll $3,$18,0x4 ; sw $16,16($sp) ; move $16,$18`
  Every register allocation, every branch, both loops, the epilogue: byte-identical. The residual is
  purely the placement of `sll $3,$18,4` (v1 = a0<<4) relative to the `$16` save/copy (i = a0) —
  a sched2 ordering of two independent insns that both depend only on `move $18,$4`.
- **RA fingerprint of the matching form**: a0→$18(s2), a1→$19(s3), sum→$17(s1), i→$16(s0),
  s4(table val)→$20, s5(s4+a1)→$21, v1(shift)→$3, count→$4. Init order in C must be
  sum, v1, s4, i, count, s5 (v1 BEFORE i — see H1 kill).
- **H1 kill detail** (i = a0 moved before v1 = a0<<4): RA rotates a0→$21, s4→$18, s5→$20 AND cse
  copy-propagates the shift operand to `sll $3,$16` (target reads $18). So the original source
  provably computes `v1 = a0 << 4` textually before `i = a0`; the move-before-sll order in target
  is a SCHEDULER outcome, not source order.
- **Tie is robust to downstream init reordering**: swapping count/i/s5 init order among themselves
  (keeping v1-before-i) = still 2, same shape (H2, H3 neutral).
- **CSE-reuse spelling adds an insn**: folding the shift into s4's initializer and re-deriving
  `v1 = a0 << 4` after `i = a0` builds 84 insns (extra copy for the reused sll temp) — H4 killed.
- Function already incorporates the documented loop-counter-fills-load-delay recipe (see
  .claude/rules/ index entry for func_80045294, 2026-06-14) — the loop body is fully matched;
  do not disturb `val = *(p+off); off += 0x10; i += 1; sum += val;`.
- No duplicate/sibling leads (tmp/duplicates_leads.txt has no entry for 80045294).
- Scheduling mechanics to beat: in block0 the chain sll→addu $1→lw $20→addu $21(beqz delay) is
  longer than move16→slt→beqz, so GCC 2.7.2 sched gives sll higher priority and emits it first.
  Target emits move16 first despite this. Lever must either shorten sll's downstream chain,
  lengthen move16's, or change the tie some other way — WITHOUT changing pseudo creation order
  (RA fingerprint above) and without moving addu $21 out of block0 (it must stay for the
  beqz delay-slot fill).

- [s1] Honest floor = 2 with 0 regfix/asmfix rules and no non-canonical cheat-asm in func_80045294 (sandbox --disable all this session).

- [s1] 83/83 insn count parity; residual = 2-insn sched2 ordering of `sll $3,$18,0x4` vs `sw $16,16($sp) ; move $16,$18` in the prologue cluster.

- [s1] RA fingerprint of the matching form: a0->$18(s2), a1->$19(s3), sum->$17(s1), i->$16(s0), s4(table val)->$20, s5(s4+a1)->$21, v1(shift)->$3, count->$4. Confirmed identical in build.

- [s1] Original source provably has textual order `v1 = a0<<4` BEFORE `i = a0` (H1 kill). The target's move-before-sll emission is a scheduler outcome, not source order.

- [s1] Neutral reorderings of count/i/s5 init (keeping v1-before-i) do not perturb the score or shape (H2, H3 neutral).

- [s1] Loop body already implements the documented [[loop-counter-fills-load-delay]] recipe (val = *(p+off); off+=0x10; i+=1; sum+=val;) — do not disturb.

- [s1] No sibling/duplicate leads: tmp/duplicates_leads.txt has no entry for 80045294.

- [s1] Scheduling mechanics to beat in block0: chain sll->addu $1->lw $20->addu $21(beqz delay) is longer than move16->slt->beqz, so sched2 elevates sll. A closing lever must either shorten sll's downstream chain, lengthen move16's, or change the tie some other way — WITHOUT altering pseudo creation order and WITHOUT moving addu $21 out of block0 (delay-slot fill).
