# func_8007EA0C -- naming evidence

- Address: 0x8007EA0C
- Size: 78 instructions
- Asm file: `asm/funcs/func_8007EA0C.s`
- Callers (3): ['func_800620B8', 'func_800646E8', 'motion_SetExMotion']
- Proposed name: `gte_mvmva_wrapper_8007EA0C`
- Confidence: **low**

## Evidence

### gte_op  (rank=low)

- proposed: `gte_mvmva_wrapper_8007EA0C`
- detail: GTE: {'mvmva': 2} atomic ops, 12 cop2 transfers

## First 20 instructions

```
  lw         $t0, 0x0($a0)
  lw         $t1, 0x4($a0)
  lw         $t2, 0x8($a0)
  bgez       $t0, .L8007EA3C
  sra        $t3, $t0, 15
  negu       $t0, $t0
  sra        $t3, $t0, 15
  andi       $t0, $t0, 0x7FFF
  negu       $t3, $t3
  b          .L8007EA40
  negu       $t0, $t0
  sra        $t3, $t0, 15
  andi       $t0, $t0, 0x7FFF
  bgez       $t1, .L8007EA64
  sra        $t4, $t1, 15
  negu       $t1, $t1
  sra        $t4, $t1, 15
  andi       $t1, $t1, 0x7FFF
  negu       $t4, $t4
  b          .L8007EA68
```
