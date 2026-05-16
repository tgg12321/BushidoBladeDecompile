# func_8007EEEC -- naming evidence

- Address: 0x8007EEEC
- Size: 12 instructions
- Asm file: `asm/funcs/func_8007EEEC.s`
- Callers: none observed
- Proposed name: `gte_helper_8007EEEC`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_helper_8007EEEC`
- detail: 5 cop2 transfers (no atomic op)

## First 20 instructions

```
  lw         $t0, 0x0($a0)
  lw         $t1, 0x4($a0)
  lw         $t2, 0x8($a0)
  lw         $t3, 0xC($a0)
  lw         $t4, 0x10($a0)
  ctc2       $t0, $0 /* handwritten instruction */
  ctc2       $t1, $1 /* handwritten instruction */
  ctc2       $t2, $2 /* handwritten instruction */
  ctc2       $t3, $3 /* handwritten instruction */
  ctc2       $t4, $4 /* handwritten instruction */
  jr         $ra
  nop        
```
