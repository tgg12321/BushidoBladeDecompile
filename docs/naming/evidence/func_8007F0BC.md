# func_8007F0BC -- naming evidence

- Address: 0x8007F0BC
- Size: 10 instructions
- Asm file: `asm/funcs/func_8007F0BC.s`
- Callers (2): ['func_80017748', 'func_800177C8']
- Proposed name: `gte_sqr_wrapper_8007F0BC`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_sqr_wrapper_8007F0BC`
- detail: GTE: {'sqr': 1} atomic ops, 6 cop2 transfers

## First 20 instructions

```
  lwc2       $9, 0x0($a0)
  lwc2       $10, 0x4($a0)
  lwc2       $11, 0x8($a0)
  nop        
  sqr        1
  swc2       $25, 0x0($a1)
  swc2       $26, 0x4($a1) /* handwritten instruction */
  swc2       $27, 0x8($a1) /* handwritten instruction */
  jr         $ra
  addu       $v0, $a1, $zero
```
