# func_8008AAD4 -- naming evidence

- Address: 0x8008AAD4
- Size: 127 instructions
- Asm file: `asm/funcs/func_8008AAD4.s`
- Callers (1): ['action_CheckHitZangeki']
- Proposed name: `action_CheckHitZangeki_helper_8008AAD4`
- Confidence: **medium**

## Evidence

### sole_caller_path  (rank=medium)

- proposed: `action_CheckHitZangeki_helper_8008AAD4`
- detail: sole caller is action_CheckHitZangeki

## First 20 instructions

```
  lui        $v0, (0xFFFFFF >> 16)
  ori        $v0, $v0, (0xFFFFFF & 0xFFFF)
  and        $a1, $a1, $v0
  addu       $a2, $a1, $zero
  srl        $t0, $a1, 16
  beqz       $a0, .L8008ABE4
  addu       $a3, $t0, $zero
  addiu      $v0, $zero, 0x1
  bne        $a0, $v0, .L8008ACC8
  nop        
  lui        $v0, %hi(D_800A2CD4)
  lw         $v0, %lo(D_800A2CD4)($v0)
  nop        
  andi       $v0, $v0, 0x1
  beqz       $v0, .L8008ABC0
  nop        
  lui        $v0, %hi(D_800F7420)
  addiu      $v0, $v0, %lo(D_800F7420)
  sh         $a2, 0x0($v0)
  lui        $at, %hi(D_800F7420 + 0x2)
```
