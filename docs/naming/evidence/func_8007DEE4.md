# func_8007DEE4 -- naming evidence

- Address: 0x8007DEE4
- Size: 11 instructions
- Asm file: `asm/funcs/func_8007DEE4.s`
- Callers: none observed
- Proposed name: `psyq_memset_8007DEE4`
- Confidence: **high**

## Evidence

### psyq_idiom  (rank=high)

- proposed: `psyq_memset_8007DEE4`
- detail: 11 insns, store loop no load; agrees with known_psyq_stdlib.txt=psyq_memset

## First 20 instructions

```
  addiu      $sp, $sp, -0x8
  beqz       $a2, .L8007DF04
  addiu      $v0, $a2, -0x1
  addiu      $v1, $zero, -0x1
  sb         $a1, 0x0($a0)
  addiu      $v0, $v0, -0x1
  bne        $v0, $v1, .L8007DEF4
  addiu      $a0, $a0, 0x1
  addiu      $sp, $sp, 0x8
  jr         $ra
  nop        
```
