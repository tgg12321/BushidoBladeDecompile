# func_8008AEB0 -- naming evidence

- Address: 0x8008AEB0
- Size: 42 instructions
- Asm file: `asm/funcs/func_8008AEB0.s`
- Callers (2): ['func_8005BF78', 'func_80088584']
- Callees (1): ['bios_TestEvent']
- Proposed name: `spu_local_8008AEB0`
- Confidence: **low**

## Evidence

### address_neighborhood  (rank=low)

- proposed: `spu_local_8008AEB0`
- detail: neighbors within +-0x200: ['coli_calc_motion2', 'spu_TransferDirect_capped', 'spu_TransferData_capped', 'ReturnVSMode'] prefix-dominant=spu (2/4)

## First 20 instructions

```
  lui        $v0, %hi(D_800A2878)
  lw         $v0, %lo(D_800A2878)($v0)
  addiu      $sp, $sp, -0x20
  sw         $s1, 0x14($sp)
  addu       $s1, $a0, $zero
  sw         $s0, 0x10($sp)
  addiu      $s0, $zero, 0x1
  beq        $v0, $s0, .L8008AEE8
  sw         $ra, 0x18($sp)
  lui        $v0, %hi(D_800A2D10)
  lw         $v0, %lo(D_800A2D10)($v0)
  nop        
  bne        $v0, $s0, .L8008AEF0
  nop        
  j          .L8008AF40
  addiu      $v0, $zero, 0x1
  lui        $a0, %hi(D_800A2870)
  lw         $a0, %lo(D_800A2870)($a0)
  jal        bios_TestEvent
  nop        
```
