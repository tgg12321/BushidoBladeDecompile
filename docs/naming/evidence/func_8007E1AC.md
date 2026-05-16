# func_8007E1AC -- naming evidence

- Address: 0x8007E1AC
- Size: 20 instructions
- Asm file: `asm/funcs/func_8007E1AC.s`
- Callers (2): ['func_80044CCC', 'func_80044DE4']
- Proposed name: `gte_helper_8007E1AC`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_helper_8007E1AC`
- detail: 12 cop2 transfers (no atomic op)

## First 20 instructions

```
  mtc2       $a2, $8 /* handwritten instruction */
  lwc2       $9, 0x0($a0)
  lwc2       $10, 0x4($a0)
  lwc2       $11, 0x8($a0)
  nop        
  gpf        1
  mfc2       $v0, $31 /* handwritten instruction */
  mtc2       $a3, $8 /* handwritten instruction */
  lwc2       $9, 0x0($a1)
  lwc2       $10, 0x4($a1)
  lwc2       $11, 0x8($a1)
  nop        
  gpl        1
  lw         $t0, 0x10($sp)
  nop        
  swc2       $9, 0x0($t0)
  swc2       $10, 0x4($t0)
  swc2       $11, 0x8($t0)
  jr         $ra
  nop        
```
