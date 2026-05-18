# func_8003CCCC -- naming evidence

- Address: 0x8003CCCC
- Size: 17 instructions
- Asm file: `asm/funcs/func_8003CCCC.s`
- Callers: none observed
- Callees (3): ['game_Cleanup', 'gpu_DisableDisplay', 'gpu_InitDisplay']
- Proposed name: `mode_local_8003CCCC`
- Confidence: **low**

## Evidence

### address_neighborhood  (rank=low)

- proposed: `mode_local_8003CCCC`
- detail: neighbors within +-0x200: ['mode_handler_33_RebootBegin', 'mode_handler_28_PostBattleMisc'] prefix-dominant=mode (2/2)

## First 20 instructions

```
  addiu      $sp, $sp, -0x18
  sw         $ra, 0x10($sp)
  jal        gpu_InitDisplay
  nop        
  jal        game_Cleanup
  nop        
  addiu      $v0, $zero, 0x21
  lui        $at, %hi(D_800A37B8)
  sw         $zero, %lo(D_800A37B8)($at)
  lui        $at, %hi(D_800A3834)
  sh         $v0, %lo(D_800A3834)($at)
  jal        gpu_DisableDisplay
  nop        
  lw         $ra, 0x10($sp)
  addiu      $sp, $sp, 0x18
  jr         $ra
  nop        
```
