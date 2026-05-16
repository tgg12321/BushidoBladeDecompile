# func_8007DC9C -- naming evidence

- Address: 0x8007DC9C
- Size: 91 instructions
- Asm file: `asm/funcs/func_8007DC9C.s`
- Callers (5): ['func_8007CAC8', 'func_8007CE0C', 'func_8007D048', 'func_8007D3F8', 'func_8007DB20']
- Callees (3): ['debug_printf', 'motion_make_table', 'sys_VSync']
- Proposed name: `debug_func_8007DC9C`
- Confidence: **low**

## Evidence

### string_adjacent  (rank=low)

- proposed: `debug_func_8007DC9C`
- detail: loads strings: ["0x80016044='func=(%08x)(%08x,%08x)\\n'", "0x80016010='GPU timeout:que=%d,stat=%08x,chcr=%08x,m'", 'g_str_gpu_timeout // GPU timeout debug']

## First 20 instructions

```
  addiu      $sp, $sp, -0x20
  sw         $ra, 0x18($sp)
  jal        sys_VSync
  addiu      $a0, $zero, -0x1
  lui        $v1, %hi(D_8009BF8C)
  lw         $v1, %lo(D_8009BF8C)($v1)
  nop        
  slt        $v1, $v1, $v0
  bnez       $v1, .L8007DCEC
  nop        
  lui        $v1, %hi(D_8009BF90)
  lw         $v1, %lo(D_8009BF90)($v1)
  nop        
  addiu      $v0, $v1, 0x1
  lui        $at, %hi(D_8009BF90)
  sw         $v0, %lo(D_8009BF90)($at)
  lui        $v0, (0xF0000 >> 16)
  slt        $v0, $v0, $v1
  beqz       $v0, .L8007DDF4
  nop        
```
