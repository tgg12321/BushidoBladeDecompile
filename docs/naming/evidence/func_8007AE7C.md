# func_8007AE7C -- naming evidence

- Address: 0x8007AE7C
- Size: 97 instructions
- Asm file: `asm/funcs/func_8007AE7C.s`
- Callers: none observed
- Callees (5): ['bb2_memset', 'bios_GPU_cw', 'debug_printf', 'func_8007D9C4', 'irq_DisableInterrupts']
- Proposed name: ``
- Confidence: **none**

## Evidence

### string_adjacent_info  (rank=info)

- detail: refs strings: ['g_str_resetgraph_jtb_env_80015E5C_fmt', 'g_str_resetgraph_80015E7C_fmt', "0x80015E5C='ResetGraph:jtb=%08x,env=%08x\\n'", "0x80015E7C='ResetGraph(%d)...\\n'"]

## First 20 instructions

```
  addiu      $sp, $sp, -0x20
  sw         $s1, 0x14($sp)
  addu       $s1, $a0, $zero
  andi       $v1, $s1, 0x7
  addiu      $v0, $zero, 0x3
  sw         $ra, 0x18($sp)
  beq        $v1, $v0, .L8007AECC
  sw         $s0, 0x10($sp)
  slti       $v0, $v1, 0x4
  beqz       $v0, .L8007AEB8
  nop        
  beqz       $v1, .L8007AECC
  nop        
  j          .L8007AF98
  nop        
  addiu      $v0, $zero, 0x5
  beq        $v1, $v0, .L8007AEEC
  nop        
  j          .L8007AF98
  nop        
```
