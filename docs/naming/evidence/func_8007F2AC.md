# func_8007F2AC -- naming evidence

- Address: 0x8007F2AC
- Size: 10 instructions
- Asm file: `asm/funcs/func_8007F2AC.s`
- Callers (3): ['func_80017A44', 'func_80061C00', 'func_80061D74']
- Proposed name: `gte_mvmva_wrapper_8007F2AC`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_mvmva_wrapper_8007F2AC`
- detail: GTE: {'mvmva': 1} atomic ops, 6 cop2 transfers

## First 20 instructions

```
  lwc2       $0, 0x0($a0)
  lwc2       $1, 0x4($a0)
  nop        
  mvmva      1, 0, 0, 0, 0
  swc2       $25, 0x0($a1)
  swc2       $26, 0x4($a1) /* handwritten instruction */
  swc2       $27, 0x8($a1) /* handwritten instruction */
  cfc2       $v0, $31 /* handwritten instruction */
  jr         $ra
  sw         $v0, 0x0($a2)
```
