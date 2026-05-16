# func_80020DDC -- naming evidence

- Address: 0x80020DDC
- Size: 38 instructions
- Asm file: `asm/funcs/func_80020DDC.s`
- Callers (1): ['DispPracticeMenuTex_B']
- Callees (3): ['func_80036EA8', 'game_FrameLoop', 'replay_camera_Init']
- Proposed name: `DispPracticeMenuTex_B_helper_80020DDC`
- Confidence: **medium**

## Evidence

### sole_caller_path  (rank=medium)

- proposed: `DispPracticeMenuTex_B_helper_80020DDC`
- detail: sole caller is DispPracticeMenuTex_B

## First 20 instructions

```
  addiu      $sp, $sp, -0x18
  addiu      $a0, $zero, 0x1
  sw         $ra, 0x10($sp)
  jal        func_80036EA8
  addiu      $a1, $zero, 0x1
  lui        $a1, %hi(D_800A3830)
  lw         $a1, %lo(D_800A3830)($a1)
  jal        replay_camera_Init
  addu       $a0, $v0, $zero
  jal        game_FrameLoop
  nop        
  lui        $v1, %hi(D_800A3830)
  lw         $v1, %lo(D_800A3830)($v1)
  nop        
  addiu      $v0, $v1, 0x14
  lui        $at, %hi(D_80102760)
  sw         $v0, %lo(D_80102760)($at)
  lw         $v0, 0x4($v1)
  nop        
  addu       $v0, $v1, $v0
```
