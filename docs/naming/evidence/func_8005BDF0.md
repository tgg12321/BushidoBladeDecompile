# func_8005BDF0 -- naming evidence

- Address: 0x8005BDF0
- Size: 37 instructions
- Asm file: `asm/funcs/func_8005BDF0.s`
- Callers (1): ['se_data_set']
- Callees (1): ['func_80087F64']
- Proposed name: `se_data_set_helper_8005BDF0`
- Confidence: **medium**

## Evidence

### sole_caller_path  (rank=medium)

- proposed: `se_data_set_helper_8005BDF0`
- detail: sole caller is se_data_set

## First 20 instructions

```
  addiu      $sp, $sp, -0x28
  sw         $s3, 0x1C($sp)
  lui        $s3, %hi(D_800EFC38)
  addiu      $s3, $s3, %lo(D_800EFC38)
  sw         $s2, 0x18($sp)
  lui        $s2, %hi(D_800EFB38)
  addiu      $s2, $s2, %lo(D_800EFB38)
  sw         $s0, 0x10($sp)
  lui        $s0, %hi(D_8009AD18)
  addiu      $s0, $s0, %lo(D_8009AD18)
  sw         $s1, 0x14($sp)
  addiu      $s1, $s0, 0x3
  sw         $ra, 0x20($sp)
  lbu        $a0, 0x0($s0)
  jal        func_80087F64
  nop        
  lbu        $v0, 0x0($s0)
  nop        
  sll        $v0, $v0, 2
  addu       $v0, $v0, $s3
```
