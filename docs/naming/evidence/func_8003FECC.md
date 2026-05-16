# func_8003FECC -- naming evidence

- Address: 0x8003FECC
- Size: 55 instructions
- Asm file: `asm/funcs/func_8003FECC.s`
- Callers (1): ['tslPrintScreen']
- Proposed name: `tslPrintScreen_helper_8003FECC`
- Confidence: **medium**

## Evidence

### sole_caller_path  (rank=medium)

- proposed: `tslPrintScreen_helper_8003FECC`
- detail: sole caller is tslPrintScreen

## First 20 instructions

```
  addiu      $sp, $sp, -0x10
  lw         $t2, 0x1C($a1)
  addiu      $v0, $zero, -0x2
  lh         $v1, 0x0($a2)
  lhu        $t0, 0x0($a2)
  beq        $v1, $v0, .L8003FF94
  addiu      $a1, $a1, 0x1C
  addiu      $t3, $zero, -0x2
  sll        $v0, $t2, 4
  addu       $a3, $v0, $a1
  sll        $v0, $t2, 2
  addu       $t1, $v0, $a1
  addiu      $a2, $a2, 0x2
  sll        $v1, $t0, 16
  sra        $v1, $v1, 16
  sll        $v0, $v1, 1
  addu       $v0, $v0, $v1
  sll        $v0, $v0, 2
  addu       $v0, $v0, $v1
  sll        $v0, $v0, 3
```
