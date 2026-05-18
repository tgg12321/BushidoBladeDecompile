# func_80068ECC -- naming evidence

- Address: 0x80068ECC
- Size: 41 instructions
- Asm file: `asm/funcs/func_80068ECC.s`
- Callers (2): ['func_80035438', 'func_80035828']
- Proposed name: ``
- Confidence: **none**

## Evidence

### string_adjacent_info  (rank=info)

- detail: refs strings: ['g_str_r_8009BBFC_plus_8']

## First 20 instructions

```
  lui        $a1, %hi(D_8009BC04)
  addiu      $a1, $a1, %lo(D_8009BC04)
  lw         $v0, 0x0($a1)
  addiu      $v1, $zero, -0x2
  and        $v0, $v0, $v1
  andi       $v1, $a0, 0x1
  or         $v0, $v0, $v1
  addiu      $v1, $zero, -0x3
  and        $v0, $v0, $v1
  andi       $v1, $a0, 0x2
  or         $v0, $v0, $v1
  addiu      $v1, $zero, -0x5
  and        $v0, $v0, $v1
  andi       $v1, $a0, 0x4
  or         $v0, $v0, $v1
  addiu      $v1, $zero, -0x9
  and        $v0, $v0, $v1
  srl        $v1, $a0, 1
  andi       $v1, $v1, 0x8
  or         $v0, $v0, $v1
```
