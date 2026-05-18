# func_8007BAB4 -- naming evidence

- Address: 0x8007BAB4
- Size: 71 instructions
- Asm file: `asm/funcs/func_8007BAB4.s`
- Callers: none observed
- Callees (1): ['func_8007C4B8']
- Proposed name: `gpu_local_8007BAB4`
- Confidence: **low**

## Evidence

### address_neighborhood  (rank=low)

- proposed: `gpu_local_8007BAB4`
- detail: neighbors within +-0x200: ['gpu_SendPacket', 'gpu_DrawOTag', 'gpu_GetDrawEnv'] prefix-dominant=gpu (3/3)

### string_adjacent_info  (rank=info)

- detail: refs strings: ['g_str_drawotagenv_80015FDC_fmt', "0x80015FDC='DrawOTagEnv(%08x,&08x)...\\n'"]

## First 20 instructions

```
  addiu      $sp, $sp, -0x28
  sw         $s2, 0x18($sp)
  addu       $s2, $a0, $zero
  sw         $s3, 0x1C($sp)
  lui        $s3, %hi(D_8009BE76)
  addiu      $s3, $s3, %lo(D_8009BE76)
  sw         $ra, 0x20($sp)
  sw         $s1, 0x14($sp)
  sw         $s0, 0x10($sp)
  lbu        $v0, 0x0($s3)
  nop        
  sltiu      $v0, $v0, 0x2
  bnez       $v0, .L8007BB0C
  addu       $s1, $a1, $zero
  lui        $a0, %hi(D_80015FDC)
  addiu      $a0, $a0, %lo(D_80015FDC)
  addu       $a1, $s2, $zero
  lui        $v0, %hi(D_8009BE70)
  lw         $v0, %lo(D_8009BE70)($v0)
  nop        
```
