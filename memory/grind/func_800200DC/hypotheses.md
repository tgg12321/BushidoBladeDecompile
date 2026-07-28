# Hypothesis ledger — func_800200DC

## s1 (recon, 2026-07-27) — baseline + precise diff map

- H-s1-a: "The 14-insn residual is one monolithic rotation" — KILLED (refined).
  Normalized objdump diff (tmp/grind/func_800200DC/s1/diff_named.txt) shows TWO
  separable sub-rotations:
  - **Rotation A (6 insns):** dy dest at the subu — target `subu $v0,$v1,$v0`
    (dy reuses the subtrahend/arg0[1] reg), ours `subu $v1,$v1,$v0` (dy reuses
    the minuend/arg1[1] reg). Propagates: target mflo $v1 (arg2^2) + sll $s0,$v0,1;
    ours mflo $v0 + sll $s0,$v1,1; disc addu operands follow.
  - **Rotation B (8 insns):** clean $v1<->$a1 two-register swap — target: sq
    (call-result copy) `move $v1,$v0`, quotient `mflo $a1` + bgez/sra/addiu/sra
    rounding on $a1, sq re-use `subu $v0,$a2,$v1`; ours has sq in $a1 and the
    quotient in $v1.
- H-s1-b (OPEN, mechanism lead): in Rotation A's disc compute, target burns
  **$t1** for the arg3*dy2 mflo (`mflo $t1; addu $v1,$v1,$t1`) while ours reuses
  the freed v-reg (`mflo $v1; addu $v1,$v0,$v1`). If MIPS REG_ALLOC_ORDER is
  low-first, target choosing $t1 over free $v0/$a0/$a1/$t0 implies those were
  occupied in target's RTL at that point — i.e. dy (in $v0) may still be LIVE
  past the sll in the original's RTL, or the allocno order differs. This is the
  concrete question for an ALLOCDBG/-da greg dump probe.
- H-s1-c: m2c shape (tmp/grind/func_800200DC/s1/m2c.c) confirms structure is
  fully correct: `var_a0 = 0x12C` hoisted before the disc>=0 test (GCC puts it
  in the bltz delay slot — our else-arm `a0 = 300;` already produces this), and
  the arg4[2] store is a cross-jumped shared tail (ours matches at 168=168).
  No structural lever remains; the gap is 100% RA.
- H-s1-d: no sibling/duplicate analog exists — tools/find_duplicates.py full
  scan (206 pairs) has zero entries involving func_800200DC.

## [s1] The 14-insn residual is one monolithic coupled rotation
- mechanism: register allocation across the mid-function sqrt call
- probe: normalized objdump diff of sandbox .o vs asm/funcs/func_800200DC.s (syntax + maspsx nops filtered)
- result: Residual decomposes into Rotation A (6 insns: dy dest $v0 target vs $v1 ours, propagating through the disc compute) + Rotation B (8 insns: clean $v1<->$a1 swap of sq and the division quotient after the second call). No insert/delete/reorder diffs.
- verdict: KILLED

## [s1] Structure may still have a gap (delay-slot a0=300 preset, shared tail store)
- mechanism: m2c reconstruction vs our C
- probe: m2c --valid-syntax on target asm; compare against src form
- result: m2c shape matches our C exactly: a0=300 preset lands in the bltz delay slot from our else-arm assignment; arg4[2] tail store is cross-jump-shared; both sides 168 insns. Gap is 100% RA.
- verdict: CONFIRMED

## [s1] A sibling/duplicate function could donate a matched allocation shape
- mechanism: byte/structure similarity scan
- probe: tools/find_duplicates.py full scan (206 pairs) + tmp/duplicates_leads.txt grep
- result: Zero entries involving func_800200DC in either output.
- verdict: KILLED
