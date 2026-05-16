# func_8005FBC8 -- naming evidence

- Address: 0x8005FBC8
- Size: 53 instructions
- Asm file: `asm/funcs/func_8005FBC8.s`
- Callers (1): ['cpu_side_move_dir_2']
- Callees (6): ['func_80036EA8', 'func_80036F28', 'game_FrameLoop', 'gpu_DrawSync', 'gpu_LoadImage', 'replay_camera_Init']
- Proposed name: `cpu_side_move_dir_2_helper_8005FBC8`
- Confidence: **medium**

## Evidence

### sole_caller_path  (rank=medium)

- proposed: `cpu_side_move_dir_2_helper_8005FBC8`
- detail: sole caller is cpu_side_move_dir_2

## First 20 instructions

```
  addiu      $sp, $sp, -0x30
  sw         $s1, 0x24($sp)
  addu       $s1, $a1, $zero
  addiu      $a1, $a0, 0x33
  addiu      $a0, $zero, 0x2
  sw         $ra, 0x28($sp)
  jal        func_80036EA8
  sw         $s0, 0x20($sp)
  addu       $s0, $v0, $zero
  addu       $a0, $s0, $zero
  jal        replay_camera_Init
  addu       $a1, $s1, $zero
  jal        game_FrameLoop
  nop        
  jal        func_80036F28
  addu       $a0, $s0, $zero
  lui        $a1, %hi(D_800A327C)
  addiu      $a1, $a1, %lo(D_800A327C)
  lwl        $v0, 0x3($a1)
  lwr        $v0, 0x0($a1)
```
