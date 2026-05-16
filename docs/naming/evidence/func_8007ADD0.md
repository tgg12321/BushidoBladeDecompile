# func_8007ADD0 -- naming evidence

- Address: 0x8007ADD0
- Size: 43 instructions
- Asm file: `asm/funcs/func_8007ADD0.s`
- Callers: none observed
- Proposed name: ``
- Confidence: **none**

## Evidence

### string_adjacent_info  (rank=info)

- detail: refs strings: ["0x80015DD8='disp   (%3d,%3d)-(%d,%d)\\n'", "0x80015DF4='screen (%3d,%3d)-(%d,%d)\\n'", "0x80015E10='isinter %d\\n'", "0x80015E1C='isrgb24 %d\\n'"]

## First 20 instructions

```
  addiu      $sp, $sp, -0x20
  sw         $s0, 0x18($sp)
  addu       $s0, $a0, $zero
  sw         $ra, 0x1C($sp)
  lh         $a1, 0x0($s0)
  lh         $a2, 0x2($s0)
  lh         $a3, 0x4($s0)
  lh         $v0, 0x6($s0)
  lui        $v1, %hi(D_8009BE70)
  lw         $v1, %lo(D_8009BE70)($v1)
  lui        $a0, %hi(D_80015DD8)
  addiu      $a0, $a0, %lo(D_80015DD8)
  jalr       $v1
  sw         $v0, 0x10($sp)
  lh         $a1, 0x8($s0)
  lh         $a2, 0xA($s0)
  lh         $a3, 0xC($s0)
  lh         $v0, 0xE($s0)
  lui        $v1, %hi(D_8009BE70)
  lw         $v1, %lo(D_8009BE70)($v1)
```
