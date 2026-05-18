# func_8008ACD0 -- naming evidence

- Address: 0x8008ACD0
- Size: 37 instructions
- Asm file: `asm/funcs/func_8008ACD0.s`
- Callers: none observed
- Proposed name: `spu_local_8008ACD0`
- Confidence: **low**

## Evidence

### address_neighborhood  (rank=low)

- proposed: `spu_local_8008ACD0`
- detail: neighbors within +-0x200: ['spu_TransferDirect_capped', 'spu_TransferData_capped'] prefix-dominant=spu (2/2)

## First 20 instructions

```
  addiu      $a1, $zero, -0x1
  addu       $v1, $zero, $zero
  addiu      $a2, $zero, 0x1
  sllv       $v0, $a2, $v1
  and        $v0, $a0, $v0
  bnez       $v0, .L8008AD10
  nop        
  addiu      $v1, $v1, 0x1
  slti       $v0, $v1, 0x18
  bnez       $v0, .L8008ACE0
  sllv       $v0, $a2, $v1
  addiu      $v0, $zero, -0x1
  bne        $a1, $v0, .L8008AD18
  sll        $a0, $a1, 4
  j          .L8008AD5C
  nop        
  j          .L8008ACFC
  addu       $a1, $v1, $zero
  lui        $v0, %hi(D_800A2CDC)
  lw         $v0, %lo(D_800A2CDC)($v0)
```
