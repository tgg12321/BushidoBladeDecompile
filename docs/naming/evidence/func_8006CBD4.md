# func_8006CBD4 -- naming evidence

- Address: 0x8006CBD4
- Size: 61 instructions
- Asm file: `asm/funcs/func_8006CBD4.s`
- Callers (1): ['func_8006CCC8']
- Proposed name: ``
- Confidence: **none**

## Evidence

### string_adjacent_info  (rank=info)

- detail: refs strings: ['g_text1b_arg1_struct_ptr']

## First 20 instructions

```
  sll        $a2, $a0, 4
  addiu      $v0, $zero, 0x10
  sllv       $v0, $v0, $a2
  and        $v0, $a1, $v0
  beqz       $v0, .L8006CBF4
  addiu      $v0, $zero, 0x40
  j          .L8006CC38
  addiu      $a3, $zero, 0x1
  sllv       $v0, $v0, $a2
  and        $v0, $a1, $v0
  beqz       $v0, .L8006CC0C
  addiu      $v0, $zero, 0x80
  j          .L8006CC38
  addiu      $a3, $zero, 0x2
  sllv       $v0, $v0, $a2
  and        $v0, $a1, $v0
  beqz       $v0, .L8006CC24
  addiu      $v0, $zero, 0x20
  j          .L8006CC38
  addiu      $a3, $zero, 0x3
```
