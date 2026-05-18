# func_8005B6FC -- naming evidence

- Address: 0x8005B6FC
- Size: 12 instructions
- Asm file: `asm/funcs/func_8005B6FC.s`
- Callers (2): ['func_800693CC', 'func_800779C8']
- Callees (1): ['func_80087F64']
- Proposed name: `obj_local_8005B6FC`
- Confidence: **low**

## Evidence

### address_neighborhood  (rank=low)

- proposed: `obj_local_8005B6FC`
- detail: neighbors within +-0x200: ['get_point_value', 'obj_InitChars', 'GetAllocPacketSize', 'obj_InitAll', 'obj_InitPair'] prefix-dominant=obj (3/5)

### string_adjacent_info  (rank=info)

- detail: refs strings: ['g_effect_struct_b_plus_4']

## First 20 instructions

```
  addiu      $sp, $sp, -0x18
  sw         $ra, 0x10($sp)
  jal        func_80087F64
  addiu      $a0, $zero, 0x1
  lui        $at, %hi(D_800EFC3C)
  sw         $zero, %lo(D_800EFC3C)($at)
  lui        $at, %hi(D_800EFB3C)
  sw         $zero, %lo(D_800EFB3C)($at)
  lw         $ra, 0x10($sp)
  addiu      $sp, $sp, 0x18
  jr         $ra
  nop        
```
