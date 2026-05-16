# func_8007E5EC -- naming evidence

- Address: 0x8007E5EC
- Size: 88 instructions
- Asm file: `asm/funcs/func_8007E5EC.s`
- Callers (1): ['func_8006295C']
- Proposed name: `gte_mvmva_wrapper_8007E5EC`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_mvmva_wrapper_8007E5EC`
- detail: GTE: {'mvmva': 4} atomic ops, 25 cop2 transfers

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
  lhu        $t0, 0x0($a1)
  lw         $t1, 0x4($a1)
  lw         $t2, 0xC($a1)
  lui        $at, (0xFFFF0000 >> 16)
  and        $t1, $t1, $at
  or         $t0, $t0, $t1
  mtc2       $t0, $0 /* handwritten instruction */
  mtc2       $t2, $1 /* handwritten instruction */
  nop        
  mvmva      1, 0, 0, 3, 0
```
