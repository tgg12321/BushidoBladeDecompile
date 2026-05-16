# func_8003DBE4 -- naming evidence

- Address: 0x8003DBE4
- Size: 133 instructions
- Asm file: `asm/funcs/func_8003DBE4.s`
- Callers (1): ['tslLineG5Init']
- Callees (3): ['func_8003DDF8', 'func_80052C28', 'game_GetPlayerCount']
- Proposed name: `tslLineG5Init_helper_8003DBE4`
- Confidence: **medium**

## Evidence

### sole_caller_path  (rank=medium)

- proposed: `tslLineG5Init_helper_8003DBE4`
- detail: sole caller is tslLineG5Init

## First 20 instructions

```
  addiu      $sp, $sp, -0x40
  sw         $s7, 0x34($sp)
  lw         $s7, 0x50($sp)
  sw         $s4, 0x28($sp)
  addu       $s4, $a0, $zero
  sw         $s1, 0x1C($sp)
  addu       $s1, $a1, $zero
  sw         $s0, 0x18($sp)
  addu       $s0, $a2, $zero
  sw         $s3, 0x24($sp)
  addu       $s3, $a3, $zero
  sw         $ra, 0x38($sp)
  sw         $s6, 0x30($sp)
  sw         $s5, 0x2C($sp)
  beqz       $s7, .L8003DC2C
  sw         $s2, 0x20($sp)
  j          .L8003DC30
  addu       $s5, $s1, $zero
  addiu      $s5, $s1, -0x1
  lui        $v0, %hi(D_800F6656)
```
