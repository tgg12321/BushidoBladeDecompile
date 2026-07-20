# Hypothesis ledger — AddTbpOfst

## Session 1 (recon)

- H1 "fresh-subexpr shift fixes sll/addu regs" — CONFIRMED (A1: sll/addu both
  land in $2 matching target; placement remains).
- H2 "s32 params fix the andi operand" — KILLED (6; single pseudo, ALL uses
  read the copy, slti regression).
- H3 "s16 param (or (s16)&0xFFFF guard) folds mask to and($4)" — KILLED for
  placement (7): produces the right andi but cse shares the sign-extend
  across the branch, hoisting it above the guard; target extends after beqz.
- H4 "second local for the sb makes param var single-use so combine folds
  zero_extend" — KILLED in u16 spelling (cse unifies, no-op) and u8 spelling
  (extra move, 50 insns). Mechanism validated by neighbor func_80087F64
  (fold fires when var dies) — the right SPELLING is still open.
- H5 "fence position lets scheduler hoist the fresh sll above sb1" — KILLED
  for all tried positions (before sb's: 5; none: 19; after sb2: no change).

## Live frontier (for session 2+)

1. **andi-operand spelling hunt**: find a form where the sb's value reaches
   RTL derived from pseudo 73 with no extra insn. Next probes: read
   main.i.combine log for the attempted/refused combinations on insn 16;
   grep other COMPLETED u16-param functions that KEEP an $a3 copy AND read
   $a0 raw in an andi (none found among `andi $v0,$a0,0xFFFF` carriers yet —
   widen the pattern to other dest regs / masks); try guard spellings whose
   zero-extend is a multi-insn chain combine can fold while keeping the copy
   (e.g. `(u32)a0 << 16 >> 16`-family, `(a0 ^ 0)`), each measured.
2. **sll scheduling**: understand region-2 priorities in main.i.sched for the
   A1 form (why the fresh sll won't lead the region); probe permuter
   (PERM_* on statement order of the sb/entry block) with a clean
   single-function target.o per difficult-is-not-impossible §3.
3. **Interaction check**: any candidate that fixes cluster 1 must be re-scored
   against cluster 2's form (A1) — they touch disjoint regions but share RA.

## [s1] Writing sa1<<4 as a fresh subexpression (not in-place sa1=sa1<<4) makes the shift land in $v0 like target
- mechanism: target's slt keeps sa1 in $3, so the shifted value is a fresh pseudo; a C temporary expression allocates to $2 (dies at the addu/lw)
- probe: entry = *((s32*)(((sa1<<4)+v1)+8)); sandbox --disable all
- result: sll $2,$3,4 and addu $2,$2,$4 now byte-match target registers; score stays 3 (sll emitted at slot 36 vs target 32, blocked by the do-while fence region)
- verdict: CONFIRMED

## [s1] s32 params fix the andi reading the $a3 copy
- mechanism: single SI pseudo would let cse propagate the hard reg into the mask
- probe: s32 AddTbpOfst(s32,s32) + explicit casts
- result: score 6: ALL first uses read the copies ($7/$8), sltiu degrades to slti; the u16/s16 two-pseudo split is REQUIRED
- verdict: KILLED

## [s1] s16 param (or (s16)a0 & 0xFFFF guard) folds the mask to and(SI-copy) = andi $v0,$a0
- mechanism: combine folds sext-shifts + and into and(reg73,0xffff), as proven by matched neighbor func_80087F64
- probe: both spellings, sandbox each
- result: score 7 both: andi $2,$4 achieved BUT cse shares the sign-extend across the branch and hoists sll/sra above the guard; target extends after beqz
- verdict: KILLED

## [s1] A second local carrying the sb value makes the param var single-use so combine folds the zero-extend onto the SI copy
- mechanism: combine refuses insn6->insn16 fold while pseudo 72 is multi-use (added_sets_2 PARALLEL fails recog); killing the sb use should unlock it
- probe: u16 saved = a0 (score 3, cse unifies, no-op) and u8 saved = a0 (score 5, 50 insns, extra move)
- result: both spellings fail; mechanism confirmed correct via func_80087F64 but the copy-spelling that survives cse without an extra insn is still open
- verdict: KILLED

## [s1] Moving/removing the do-while(0) fence lets the scheduler hoist the fresh sll above the first sb
- mechanism: fence loop-notes split sched1 regions; sll cannot cross upward
- probe: no fence (19), fence before both sbs (5), entry stmt before fence (20)
- result: all worse; fence between the two sbs is load-bearing for the whole tail; within a shared region the scheduler still orders sb1 before the fresh sll
- verdict: KILLED
