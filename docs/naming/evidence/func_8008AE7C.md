# func_8008AE7C -- naming evidence

- Address: 0x8008AE7C
- Size: 13 instructions
- Asm file: `asm/funcs/func_8008AE7C.s`
- Callers (1): ['func_800884C4']
- Proposed name: `spu_local_8008AE7C`
- Confidence: **low**

## Evidence

### address_neighborhood  (rank=low)

- proposed: `spu_local_8008AE7C`
- detail: neighbors within +-0x200: ['coli_calc_motion2', 'spu_TransferDirect_capped', 'spu_TransferData_capped', 'ReturnVSMode'] prefix-dominant=spu (2/4)

## First 20 instructions

```
  beqz       $a0, .L8008AE94
  addiu      $v0, $zero, 0x1
  bne        $a0, $v0, .L8008AE98
  addu       $v0, $zero, $zero
  j          .L8008AE98
  addiu      $v0, $zero, 0x1
  addu       $v0, $zero, $zero
  lui        $at, %hi(D_800A2878)
  sw         $a0, %lo(D_800A2878)($at)
  lui        $at, %hi(D_800A2CF8)
  sw         $v0, %lo(D_800A2CF8)($at)
  jr         $ra
  nop        
```
