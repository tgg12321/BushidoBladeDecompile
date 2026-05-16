# func_8007EF4C -- naming evidence

- Address: 0x8007EF4C
- Size: 8 instructions
- Asm file: `asm/funcs/func_8007EF4C.s`
- Callers: none observed
- Proposed name: `gte_helper_8007EF4C`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_helper_8007EF4C`
- detail: 3 cop2 transfers (no atomic op)

## First 20 instructions

```
  lw         $t0, 0x14($a0)
  lw         $t1, 0x18($a0)
  lw         $t2, 0x1C($a0)
  ctc2       $t0, $5 /* handwritten instruction */
  ctc2       $t1, $6 /* handwritten instruction */
  ctc2       $t2, $7 /* handwritten instruction */
  jr         $ra
  nop        
```
