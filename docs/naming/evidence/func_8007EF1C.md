# func_8007EF1C -- naming evidence

- Address: 0x8007EF1C
- Size: 12 instructions
- Asm file: `asm/funcs/func_8007EF1C.s`
- Callers: none observed
- Proposed name: `gte_helper_8007EF1C`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_helper_8007EF1C`
- detail: 5 cop2 transfers (no atomic op)

## First 20 instructions

```
  lw         $t0, 0x0($a0)
  lw         $t1, 0x4($a0)
  lw         $t2, 0x8($a0)
  lw         $t3, 0xC($a0)
  lw         $t4, 0x10($a0)
  ctc2       $t0, $16 /* handwritten instruction */
  ctc2       $t1, $17 /* handwritten instruction */
  ctc2       $t2, $18 /* handwritten instruction */
  ctc2       $t3, $19 /* handwritten instruction */
  ctc2       $t4, $20 /* handwritten instruction */
  jr         $ra
  nop        
```
