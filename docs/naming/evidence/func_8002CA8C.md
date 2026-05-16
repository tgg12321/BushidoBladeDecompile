# func_8002CA8C -- naming evidence

- Address: 0x8002CA8C
- Size: 179 instructions
- Asm file: `asm/funcs/func_8002CA8C.s`
- Callers (1): ['calc_loc_mat_fw']
- Callees (2): ['func_8002D320', 'saTan0KiWareMoveA']
- Proposed name: `calc_loc_mat_fw_helper_8002CA8C`
- Confidence: **medium**

## Evidence

### sole_caller_path  (rank=medium)

- proposed: `calc_loc_mat_fw_helper_8002CA8C`
- detail: sole caller is calc_loc_mat_fw

## First 20 instructions

```
  addiu      $sp, $sp, -0x60
  sw         $s1, 0x3C($sp)
  lui        $s1, (0x1F8002B8 >> 16)
  ori        $s1, $s1, (0x1F8002B8 & 0xFFFF)
  sw         $s6, 0x50($sp)
  addu       $s6, $zero, $zero
  sw         $s3, 0x44($sp)
  addu       $s3, $zero, $zero
  sw         $s7, 0x54($sp)
  addu       $s7, $zero, $zero
  sw         $ra, 0x5C($sp)
  sw         $fp, 0x58($sp)
  sw         $s5, 0x4C($sp)
  sw         $s4, 0x48($sp)
  sw         $s2, 0x40($sp)
  sw         $s0, 0x38($sp)
  sw         $a0, 0x18($sp)
  sw         $a1, 0x20($sp)
  sw         $a2, 0x28($sp)
  sw         $zero, 0x30($sp)
```
