# func_8007EF9C -- naming evidence

- Address: 0x8007EF9C
- Size: 8 instructions
- Asm file: `asm/funcs/func_8007EF9C.s`
- Callers: none observed
- Proposed name: `gte_helper_8007EF9C`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_helper_8007EF9C`
- detail: 3 cop2 transfers (no atomic op)

## First 20 instructions

```
  sll        $a0, $a0, 4
  sll        $a1, $a1, 4
  sll        $a2, $a2, 4
  ctc2       $a0, $13 /* handwritten instruction */
  ctc2       $a1, $14 /* handwritten instruction */
  ctc2       $a2, $15 /* handwritten instruction */
  jr         $ra
  nop        
```
