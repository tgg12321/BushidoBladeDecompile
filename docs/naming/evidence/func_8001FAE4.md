# func_8001FAE4 -- naming evidence

- Address: 0x8001FAE4
- Size: 20 instructions
- Asm file: `asm/funcs/func_8001FAE4.s`
- Callers (1): ['single_game_CheckStatusUpDataTotalOver']
- Proposed name: `single_game_CheckStatusUpDataTotalOver_helper_8001FAE4`
- Confidence: **medium**

## Evidence

### sole_caller_path  (rank=medium)

- proposed: `single_game_CheckStatusUpDataTotalOver_helper_8001FAE4`
- detail: sole caller is single_game_CheckStatusUpDataTotalOver

## First 20 instructions

```
  lhu        $v1, 0xA($a0)
  nop        
  beqz       $v1, .L8001FB28
  addiu      $a0, $a0, 0xA
  andi       $v0, $v1, 0x4000
  bnez       $v0, .L8001FB2C
  addu       $v0, $a0, $zero
  andi       $v0, $v1, 0xC000
  beqz       $v0, .L8001FB14
  nop        
  j          .L8001FB18
  addiu      $a0, $a0, 0x8
  addiu      $a0, $a0, 0x4
  lhu        $v1, 0x0($a0)
  nop        
  bnez       $v1, .L8001FAF8
  andi       $v0, $v1, 0x4000
  addu       $v0, $zero, $zero
  jr         $ra
  nop        
```
