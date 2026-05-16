# func_8007EF6C -- naming evidence

- Address: 0x8007EF6C
- Size: 5 instructions
- Asm file: `asm/funcs/func_8007EF6C.s`
- Callers: none observed
- Proposed name: `gte_helper_8007EF6C`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_helper_8007EF6C`
- detail: 3 cop2 transfers (no atomic op)

## First 20 instructions

```
  swc2       $17, 0x0($a0)
  swc2       $18, 0x0($a1)
  swc2       $19, 0x0($a2)
  jr         $ra
  nop        
```
