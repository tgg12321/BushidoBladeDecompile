# func_8004DDB4 -- naming evidence

- Address: 0x8004DDB4
- Size: 491 instructions
- Asm file: `asm/funcs/func_8004DDB4.s`
- Callers (1): ['func_800430E4']
- Callees (2): ['func_80052930', 'func_80054440']
- Proposed name: `gte_mvmva_wrapper_8004DDB4`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_mvmva_wrapper_8004DDB4`
- detail: GTE: {'mvmva': 4, 'rtps': 2, 'rtpt': 2} atomic ops, 108 cop2 transfers

## First 20 instructions

```
  addiu      $sp, $sp, -0x8C
  sw         $ra, 0x88($sp)
  sw         $fp, 0x84($sp)
  sw         $s7, 0x80($sp)
  sw         $s6, 0x7C($sp)
  sw         $s5, 0x78($sp)
  sw         $s4, 0x74($sp)
  sw         $s3, 0x70($sp)
  sw         $s2, 0x6C($sp)
  sw         $s1, 0x68($sp)
  sw         $s0, 0x64($sp)
  addu       $fp, $a0, $zero
  addu       $s0, $a1, $zero
  addu       $s7, $a2, $zero
  addu       $s1, $a3, $zero
  addiu      $s5, $zero, 0xC
  lui        $s6, (0x1F800020 >> 16)
  ori        $s6, $s6, (0x1F800020 & 0xFFFF)
  addu       $s3, $s6, $zero
  addi       $s4, $s6, 0x224 /* handwritten instruction */
```
