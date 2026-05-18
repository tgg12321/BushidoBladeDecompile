# func_8003BFC4 -- naming evidence

- Address: 0x8003BFC4
- Size: 31 instructions
- Asm file: `asm/funcs/func_8003BFC4.s`
- Callers: none observed
- Callees (7): ['file_ResetDmaFlag', 'func_80020CDC', 'func_80037540', 'func_80045814', 'game_Init', 'gpu_EnableDisplay', 'player_Destroy']
- Proposed name: `mode_local_8003BFC4`
- Confidence: **low**

## Evidence

### address_neighborhood  (rank=low)

- proposed: `mode_local_8003BFC4`
- detail: neighbors within +-0x200: ['mode_handler_10_GameTeardown', 'mode_handler_11_PadInputCheck', 'cpu_side_move_dir_2'] prefix-dominant=mode (2/3)

## First 20 instructions

```
  addiu      $sp, $sp, -0x20
  sw         $ra, 0x18($sp)
  jal        gpu_EnableDisplay
  nop        
  jal        func_80020CDC
  nop        
  jal        player_Destroy
  addu       $a0, $zero, $zero
  jal        player_Destroy
  addiu      $a0, $zero, 0x1
  jal        file_ResetDmaFlag
  nop        
  jal        func_80045814
  nop        
  addu       $a0, $v0, $zero
  lui        $a1, (0x80118000 >> 16)
  ori        $a1, $a1, (0x80118000 & 0xFFFF)
  addiu      $v0, $zero, 0xB01
  addiu      $a2, $zero, 0x1
  addiu      $a3, $zero, 0xCF8
```
