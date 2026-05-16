# func_8007A7DC -- naming evidence

- Address: 0x8007A7DC
- Size: 24 instructions
- Asm file: `asm/funcs/func_8007A7DC.s`
- Callers: none observed
- Proposed name: ``
- Confidence: **none**

## Evidence

### string_adjacent_info  (rank=info)

- detail: refs strings: ["0x80015D58='tpage: (%d,%d,%d,%d)\\n'"]

## First 20 instructions

```
  addiu      $sp, $sp, -0x20
  andi       $a3, $a0, 0xFFFF
  sll        $v1, $a3, 4
  andi       $v1, $v1, 0x100
  srl        $v0, $a3, 2
  andi       $v0, $v0, 0x200
  addu       $v1, $v1, $v0
  srl        $a1, $a3, 7
  srl        $a2, $a3, 5
  sll        $a3, $a3, 6
  lui        $a0, %hi(D_80015D58)
  addiu      $a0, $a0, %lo(D_80015D58)
  andi       $a1, $a1, 0x3
  andi       $a2, $a2, 0x3
  lui        $v0, %hi(D_8009BE70)
  lw         $v0, %lo(D_8009BE70)($v0)
  andi       $a3, $a3, 0x7C0
  sw         $ra, 0x18($sp)
  jalr       $v0
  sw         $v1, 0x10($sp)
```
