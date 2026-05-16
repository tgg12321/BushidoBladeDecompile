# func_8007E3BC -- naming evidence

- Address: 0x8007E3BC
- Size: 31 instructions
- Asm file: `asm/funcs/func_8007E3BC.s`
- Callers: none observed
- Proposed name: `gte_helper_8007E3BC`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_helper_8007E3BC`
- detail: 12 cop2 transfers (no atomic op)

## First 20 instructions

```
  lbu        $t0, 0x0($a0)
  lbu        $t1, 0x1($a0)
  lbu        $t2, 0x2($a0)
  mtc2       $a2, $8 /* handwritten instruction */
  mtc2       $t0, $9 /* handwritten instruction */
  mtc2       $t1, $10 /* handwritten instruction */
  mtc2       $t2, $11 /* handwritten instruction */
  nop        
  gpf        0
  lbu        $t0, 0x0($a1)
  lbu        $t1, 0x1($a1)
  lbu        $t2, 0x2($a1)
  mfc2       $v0, $31 /* handwritten instruction */
  mtc2       $a3, $8 /* handwritten instruction */
  mtc2       $t0, $9 /* handwritten instruction */
  mtc2       $t1, $10 /* handwritten instruction */
  mtc2       $t2, $11 /* handwritten instruction */
  addiu      $t3, $zero, 0xC
  gpl        0
  lw         $t5, 0x10($sp)
```
