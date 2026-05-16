# func_8003FFE0 -- naming evidence

- Address: 0x8003FFE0
- Size: 15 instructions
- Asm file: `asm/funcs/func_8003FFE0.s`
- Callers (1): ['mario_test_Exec']
- Callees (1): ['func_8004153C']
- Proposed name: `mario_test_Exec_helper_8003FFE0`
- Confidence: **medium**

## Evidence

### sole_caller_path  (rank=medium)

- proposed: `mario_test_Exec_helper_8003FFE0`
- detail: sole caller is mario_test_Exec

## First 20 instructions

```
  addiu      $sp, $sp, -0x18
  sw         $ra, 0x10($sp)
  jal        func_8004153C
  nop        
  beqz       $v0, .L8004000C
  nop        
  lw         $v1, 0x24($v0)
  nop        
  beqz       $v1, .L8004000C
  addiu      $v0, $zero, 0x1
  sh         $v0, 0x2($v1)
  lw         $ra, 0x10($sp)
  addiu      $sp, $sp, 0x18
  jr         $ra
  nop        
```
