# func_8007E4DC -- naming evidence

- Address: 0x8007E4DC
- Size: 67 instructions
- Asm file: `asm/funcs/func_8007E4DC.s`
- Callers (5): ['func_8002FF20', 'func_800300B4', 'func_80044504', 'func_800475A4', 'func_80049718']
- Proposed name: `gte_mvmva_wrapper_8007E4DC`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_mvmva_wrapper_8007E4DC`
- detail: GTE: {'mvmva': 3} atomic ops, 20 cop2 transfers

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
