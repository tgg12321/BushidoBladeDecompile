# func_8003AFFC -- naming evidence

- Address: 0x8003AFFC
- Size: 68 instructions
- Asm file: `asm/funcs/func_8003AFFC.s`
- Callers: none observed
- Callees (6): ['EndADRSound', 'func_800493E4', 'func_800494D4', 'func_80049584', 'gnd_close_8004939C', 'gpu_EnableDisplay']
- Proposed name: ``
- Confidence: **none**

## Evidence

### string_adjacent_info  (rank=info)

- detail: refs strings: ['g_char_struct_p1_field_E', 'g_char_struct_p1_field12', 'g_char_struct_p1_field12_plus_2']

## First 20 instructions

```
  addiu      $sp, $sp, -0x28
  sw         $s3, 0x1C($sp)
  lui        $s3, (0x80190800 >> 16)
  ori        $s3, $s3, (0x80190800 & 0xFFFF)
  sw         $ra, 0x24($sp)
  sw         $s4, 0x20($sp)
  sw         $s2, 0x18($sp)
  sw         $s1, 0x14($sp)
  jal        gpu_EnableDisplay
  sw         $s0, 0x10($sp)
  jal        EndADRSound
  addu       $s2, $zero, $zero
  jal        gnd_close_8004939C
  addu       $s0, $zero, $zero
  lui        $s4, %hi(D_8008E5CC)
  addiu      $s4, $s4, %lo(D_8008E5CC)
  lui        $s1, %hi(D_80101EDC)
  addiu      $s1, $s1, %lo(D_80101EDC)
  lui        $at, %hi(D_80101EDA)
  addu       $at, $at, $s0
```
