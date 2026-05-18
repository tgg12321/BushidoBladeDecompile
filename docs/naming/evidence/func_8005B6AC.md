# func_8005B6AC -- naming evidence

- Address: 0x8005B6AC
- Size: 20 instructions
- Asm file: `asm/funcs/func_8005B6AC.s`
- Callers (1): ['saTanMainDispGnd_80046020']
- Callees (2): ['func_80087F64', 'title_mv_exec2']
- Proposed name: `obj_local_8005B6AC`
- Confidence: **low**

## Evidence

### address_neighborhood  (rank=low)

- proposed: `obj_local_8005B6AC`
- detail: neighbors within +-0x200: ['get_point_value', 'obj_InitChars', 'GetAllocPacketSize', 'obj_InitAll', 'obj_InitPair'] prefix-dominant=obj (3/5)

### string_adjacent_info  (rank=info)

- detail: refs strings: ['g_effect_struct_b_plus_8', 'g_effect_struct_b_off_C_plus_8']

## First 20 instructions

```
  addiu      $sp, $sp, -0x18
  sw         $ra, 0x10($sp)
  jal        title_mv_exec2
  addu       $a0, $zero, $zero
  jal        func_80087F64
  addiu      $a0, $zero, 0x2
  lui        $at, %hi(D_800EFC40)
  sw         $zero, %lo(D_800EFC40)($at)
  lui        $at, %hi(D_800EFB40)
  sw         $zero, %lo(D_800EFB40)($at)
  jal        func_80087F64
  addiu      $a0, $zero, 0x5
  lui        $at, %hi(D_800EFC4C)
  sw         $zero, %lo(D_800EFC4C)($at)
  lui        $at, %hi(D_800EFB4C)
  sw         $zero, %lo(D_800EFB4C)($at)
  lw         $ra, 0x10($sp)
  addiu      $sp, $sp, 0x18
  jr         $ra
  nop        
```
