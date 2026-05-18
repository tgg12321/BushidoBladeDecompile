# func_8002BEA0 -- naming evidence

- Address: 0x8002BEA0
- Size: 143 instructions
- Asm file: `asm/funcs/func_8002BEA0.s`
- Callers (1): ['func_80026DA4']
- Proposed name: ``
- Confidence: **none**

## Evidence

### string_adjacent_info  (rank=info)

- detail: refs strings: ['g_char_struct_p1_field_FC', 'g_char_struct_p2_field_FC']

## First 20 instructions

```
  lui        $v1, %hi(D_80101FBC)
  lw         $v1, %lo(D_80101FBC)($v1)
  lui        $v0, %hi(D_80102408)
  lw         $v0, %lo(D_80102408)($v0)
  nop        
  subu       $a3, $v1, $v0
  mult       $a3, $a3
  lui        $v1, %hi(D_80101FC4)
  lw         $v1, %lo(D_80101FC4)($v1)
  lui        $v0, %hi(D_80102410)
  lw         $v0, %lo(D_80102410)($v0)
  mflo       $a0
  subu       $t1, $v1, $v0
  nop        
  mult       $t1, $t1
  addiu      $sp, $sp, -0x8
  lui        $t2, %hi(D_80101EC8)
  addiu      $t2, $t2, %lo(D_80101EC8)
  mflo       $v1
  addu       $a0, $a0, $v1
```
