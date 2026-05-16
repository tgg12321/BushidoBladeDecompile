# func_8008BE4C -- naming evidence

- Address: 0x8008BE4C
- Size: 22 instructions
- Asm file: `asm/funcs/func_8008BE4C.s`
- Callers (1): ['func_8003A264']
- Callees (4): ['EnterCriticalSection', 'ExitCriticalSection', 'func_80078FF0', 'func_8008D060']
- Proposed name: ``
- Confidence: **none**

## Evidence

### string_adjacent_info  (rank=info)

- detail: refs strings: ['g_str_sio // sio string', "0x800164A8='sio'"]

## First 20 instructions

```
  addiu      $sp, $sp, -0x18
  sw         $ra, 0x14($sp)
  jal        EnterCriticalSection
  sw         $s0, 0x10($sp)
  lui        $a0, %hi(D_800164A8)
  addiu      $a0, $a0, %lo(D_800164A8)
  jal        func_8008D060
  addu       $s0, $v0, $zero
  jal        func_80078FF0
  nop        
  addiu      $v0, $zero, 0x1
  bne        $s0, $v0, .L8008BE88
  nop        
  jal        ExitCriticalSection
  nop        
  lw         $ra, 0x14($sp)
  lw         $s0, 0x10($sp)
  addiu      $sp, $sp, 0x18
  jr         $ra
  nop        
```
