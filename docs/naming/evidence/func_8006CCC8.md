# func_8006CCC8 -- naming evidence

- Address: 0x8006CCC8
- Size: 189 instructions
- Asm file: `asm/funcs/func_8006CCC8.s`
- Callers (1): ['func_8006D338']
- Callees (2): ['func_8005C650', 'func_8006CBD4']
- Proposed name: ``
- Confidence: **none**

## Evidence

### string_adjacent_info  (rank=info)

- detail: refs strings: ['g_text1b_arg1_struct_ptr']

## First 20 instructions

```
  addiu      $sp, $sp, -0x48
  sw         $s4, 0x30($sp)
  addu       $s4, $a1, $zero
  sw         $fp, 0x40($sp)
  addu       $fp, $zero, $zero
  lui        $v0, (0x50005 >> 16)
  lw         $v1, %gp_rel(D_800A34FC)($gp)
  ori        $v0, $v0, (0x50005 & 0xFFFF)
  sw         $ra, 0x44($sp)
  sw         $s7, 0x3C($sp)
  sw         $s6, 0x38($sp)
  sw         $s5, 0x34($sp)
  sw         $s3, 0x2C($sp)
  sw         $s2, 0x28($sp)
  sw         $s1, 0x24($sp)
  sw         $s0, 0x20($sp)
  lw         $v1, 0x28($v1)
  nop        
  bne        $v1, $v0, .L8006CD40
  addu       $s7, $a2, $zero
```
