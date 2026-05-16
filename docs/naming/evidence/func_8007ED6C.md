# func_8007ED6C -- naming evidence

- Address: 0x8007ED6C
- Size: 20 instructions
- Asm file: `asm/funcs/func_8007ED6C.s`
- Callers (5): ['func_80042FA0', 'func_80047384', 'func_800475A4', 'func_80048BA4', 'func_80049718']
- Proposed name: `gte_mvmva_wrapper_8007ED6C`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_mvmva_wrapper_8007ED6C`
- detail: GTE: {'mvmva': 1} atomic ops, 10 cop2 transfers

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
  lwc2       $0, 0x0($a1)
  lwc2       $1, 0x4($a1)
  nop        
  mvmva      1, 0, 0, 3, 0
  swc2       $25, 0x0($a2)
  swc2       $26, 0x4($a2) /* handwritten instruction */
  swc2       $27, 0x8($a2) /* handwritten instruction */
  addu       $v0, $a2, $zero
  jr         $ra
  nop        
```
