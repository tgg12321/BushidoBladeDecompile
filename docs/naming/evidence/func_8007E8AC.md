# func_8007E8AC -- naming evidence

- Address: 0x8007E8AC
- Size: 12 instructions
- Asm file: `asm/funcs/func_8007E8AC.s`
- Callers (3): ['func_80063084', 'func_80073C78', 'motion_SetExMotion']
- Proposed name: `gte_mvmva_wrapper_8007E8AC`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_mvmva_wrapper_8007E8AC`
- detail: GTE: {'mvmva': 1} atomic ops, 5 cop2 transfers

## First 20 instructions

```
  lw         $t0, 0x0($a0)
  lw         $t1, 0x4($a0)
  mtc2       $t0, $0 /* handwritten instruction */
  mtc2       $t1, $1 /* handwritten instruction */
  nop        
  mvmva      1, 0, 0, 3, 0
  swc2       $9, 0x0($a1)
  swc2       $10, 0x4($a1)
  swc2       $11, 0x8($a1)
  addu       $v0, $a2, $zero
  jr         $ra
  nop        
```
