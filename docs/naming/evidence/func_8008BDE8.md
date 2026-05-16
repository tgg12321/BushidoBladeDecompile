# func_8008BDE8 -- naming evidence

- Address: 0x8008BDE8
- Size: 7 instructions
- Asm file: `asm/funcs/func_8008BDE8.s`
- Callers (1): ['action_CheckHitZangeki']
- Proposed name: `action_CheckHitZangeki_helper_8008BDE8`
- Confidence: **medium**

## Evidence

### sole_caller_path  (rank=medium)

- proposed: `action_CheckHitZangeki_helper_8008BDE8`
- detail: sole caller is action_CheckHitZangeki

## First 20 instructions

```
  lui        $v0, %hi(D_800A2CDC)
  lw         $v0, %lo(D_800A2CDC)($v0)
  sll        $a0, $a0, 4
  addu       $a0, $a0, $v0
  lhu        $v0, 0xC($a0)
  jr         $ra
  sh         $v0, 0x0($a1)
```
