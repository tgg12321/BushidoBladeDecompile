# func_8006CFBC -- naming evidence

- Address: 0x8006CFBC
- Size: 218 instructions
- Asm file: `asm/funcs/func_8006CFBC.s`
- Callers (1): ['func_8006D338']
- Callees (5): ['func_80069898', 'func_8007352C', 'initTexPage', 'ot_Link', 'saMotionSet']
- Proposed name: ``
- Confidence: **none**

## Evidence

### string_adjacent_info  (rank=info)

- detail: refs strings: ['g_text1b_arg1_struct_ptr']

## First 20 instructions

```
  addiu      $sp, $sp, -0x88
  sw         $s2, 0x68($sp)
  addu       $s2, $a0, $zero
  sw         $fp, 0x80($sp)
  addu       $fp, $zero, $zero
  sw         $ra, 0x84($sp)
  sw         $s7, 0x7C($sp)
  sw         $s6, 0x78($sp)
  sw         $s5, 0x74($sp)
  sw         $s4, 0x70($sp)
  sw         $s3, 0x6C($sp)
  sw         $s1, 0x64($sp)
  sw         $s0, 0x60($sp)
  sh         $zero, 0x58($sp)
  lw         $v0, 0x4($s2)
  addiu      $s4, $sp, 0x18
  lw         $s7, 0x30($v0)
  addiu      $v0, $zero, 0x8
  sw         $v0, 0x2C($sp)
  sb         $zero, 0x40($sp)
```
