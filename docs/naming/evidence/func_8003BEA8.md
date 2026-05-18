# func_8003BEA8 -- naming evidence

- Address: 0x8003BEA8
- Size: 71 instructions
- Asm file: `asm/funcs/func_8003BEA8.s`
- Callers: none observed
- Callees (2): ['func_800372C0', 'func_800788B0']
- Proposed name: `mode_local_8003BEA8`
- Confidence: **low**

## Evidence

### address_neighborhood  (rank=low)

- proposed: `mode_local_8003BEA8`
- detail: neighbors within +-0x200: ['md_game_check_change_sub_mode', 'mode_handler_10_GameTeardown', 'mode_handler_15_TeardownVariant', 'cpu_side_move_dir_2'] prefix-dominant=mode (2/4)

## First 20 instructions

```
  addiu      $sp, $sp, -0x18
  sw         $s0, 0x10($sp)
  sw         $ra, 0x14($sp)
  jal        func_800788B0
  addu       $s0, $zero, $zero
  lui        $v1, %hi(D_80102794)
  lw         $v1, %lo(D_80102794)($v1)
  nop        
  andi       $v1, $v1, 0x40
  beqz       $v1, .L8003BED8
  sltu       $v0, $zero, $v0
  addiu      $v0, $zero, 0x1
  beqz       $v0, .L8003BFB0
  nop        
  jal        func_800372C0
  nop        
  lui        $a0, %hi(D_80106A50)
  addiu      $a0, $a0, %lo(D_80106A50)
  lw         $v1, 0x0($a0)
  lui        $v0, %hi(D_800A37A4)
```
