# func_8005BA8C -- naming evidence

- Address: 0x8005BA8C
- Size: 169 instructions
- Asm file: `asm/funcs/func_8005BA8C.s`
- Callers (1): ['se_data_set']
- Callees (9): ['func_80036EA8', 'func_80036F28', 'func_80062020', 'func_80087F64', 'game_FrameLoop', 'replay_camera_Init', 'saFidLoad', 'title_mv_exec2', 'tslGlobalMemFree_8005C2A8']
- Proposed name: `se_data_set_helper_8005BA8C`
- Confidence: **medium**

## Evidence

### sole_caller_path  (rank=medium)

- proposed: `se_data_set_helper_8005BA8C`
- detail: sole caller is se_data_set

### address_neighborhood  (rank=low)

- proposed: `obj_local_8005BA8C`
- detail: neighbors within +-0x200: ['obj_InitTask', 'obj_InitTaskCamera', 'obj_ExecTask'] prefix-dominant=obj (3/3)

### string_adjacent_info  (rank=info)

- detail: refs strings: ['g_effect_struct_b', 'g_effect_struct_b_off_C', 'g_effect_struct_b_off_18']

## First 20 instructions

```
  addiu      $sp, $sp, -0x80
  sw         $s5, 0x6C($sp)
  addu       $s5, $a1, $zero
  sw         $s7, 0x74($sp)
  addu       $s7, $a2, $zero
  sw         $fp, 0x78($sp)
  addu       $fp, $a3, $zero
  sw         $s3, 0x64($sp)
  addu       $s3, $a0, $zero
  addu       $a0, $zero, $zero
  sw         $ra, 0x7C($sp)
  sw         $s6, 0x70($sp)
  sw         $s4, 0x68($sp)
  sw         $s2, 0x60($sp)
  sw         $s1, 0x5C($sp)
  jal        title_mv_exec2
  sw         $s0, 0x58($sp)
  lui        $s4, %hi(D_800EFC38)
  addiu      $s4, $s4, %lo(D_800EFC38)
  lui        $s2, %hi(D_800EFB38)
```
