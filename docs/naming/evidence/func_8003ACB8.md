# func_8003ACB8 -- naming evidence

- Address: 0x8003ACB8
- Size: 105 instructions
- Asm file: `asm/funcs/func_8003ACB8.s`
- Callers (1): ['replay_camera_rob_back_loose2']
- Callees (18): ['func_80019488', 'func_800194C0', 'func_800194F4', 'func_80035E88', 'func_80035EDC', 'func_8003A174', 'func_8003AA48', 'func_8003AB44', 'func_8005C6D0', 'func_80077AE0'] ...
- Proposed name: `replay_camera_rob_back_loose2_helper_8003ACB8`
- Confidence: **medium**

## Evidence

### sole_caller_path  (rank=medium)

- proposed: `replay_camera_rob_back_loose2_helper_8003ACB8`
- detail: sole caller is replay_camera_rob_back_loose2

## First 20 instructions

```
  addiu      $sp, $sp, -0x18
  sw         $ra, 0x14($sp)
  jal        func_80077AE0
  sw         $s0, 0x10($sp)
  jal        gpu_SetDispMask
  addu       $a0, $zero, $zero
  lui        $at, %hi(D_800A37B8)
  sw         $zero, %lo(D_800A37B8)($at)
  sb         $zero, %gp_rel(D_800A38AC)($gp)
  sw         $zero, %gp_rel(D_800A38D0)($gp)
  sw         $zero, %gp_rel(D_800A3908)($gp)
  sw         $zero, %gp_rel(D_800A38FC)($gp)
  jal        func_8003A174
  nop        
  jal        single_game_VoiceContorol
  addiu      $a0, $zero, 0x1
  jal        func_8005C6D0
  nop        
  jal        func_8003AB44
  nop        
```
