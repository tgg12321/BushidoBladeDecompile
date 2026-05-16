# func_8007EFBC -- naming evidence

- Address: 0x8007EFBC
- Size: 8 instructions
- Asm file: `asm/funcs/func_8007EFBC.s`
- Callers: none observed
- Proposed name: `gte_helper_8007EFBC`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_helper_8007EFBC`
- detail: 3 cop2 transfers (no atomic op)

## First 20 instructions

```
  sll        $a0, $a0, 4
  sll        $a1, $a1, 4
  sll        $a2, $a2, 4
  ctc2       $a0, $21 /* handwritten instruction */
  ctc2       $a1, $22 /* handwritten instruction */
  ctc2       $a2, $23 /* handwritten instruction */
  jr         $ra
  nop        
```
