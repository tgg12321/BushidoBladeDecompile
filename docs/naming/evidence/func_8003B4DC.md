# func_8003B4DC -- naming evidence

- Address: 0x8003B4DC
- Size: 22 instructions
- Asm file: `asm/funcs/func_8003B4DC.s`
- Callers (1): ['suDispMentalBar']
- Callees (2): ['func_8003AF40', 'md_menu_logo_exec']
- Proposed name: `suDispMentalBar_helper_8003B4DC`
- Confidence: **medium**

## Evidence

### kengo_pattern  (rank=low)

- proposed: `coli_cancel_hit_pause_katana_8003B4DC`
- detail: Kengo CSV: confidence=seq-similarity, bb2_insns=22 kengo_insns=22 diff=0, callee_overlap=0.00, caller_overlap=0.00, opseq_ratio=0.32, n_claimants=3

### sole_caller_path  (rank=medium)

- proposed: `suDispMentalBar_helper_8003B4DC`
- detail: sole caller is suDispMentalBar

## First 20 instructions

```
  addiu      $sp, $sp, -0x18
  addiu      $v0, $zero, 0x1
  lui        $at, %hi(D_800A3712)
  sb         $v0, %lo(D_800A3712)($at)
  addiu      $v0, $zero, 0x1F
  sw         $ra, 0x10($sp)
  lui        $at, %hi(D_8010277E)
  sb         $zero, %lo(D_8010277E)($at)
  lui        $at, %hi(D_8010277D)
  sb         $v0, %lo(D_8010277D)($at)
  lui        $at, %hi(D_8010277F)
  sb         $zero, %lo(D_8010277F)($at)
  jal        func_8003AF40
  addu       $a0, $zero, $zero
  jal        func_8003AF40
  addiu      $a0, $zero, 0x1
  jal        md_menu_logo_exec
  nop        
  lw         $ra, 0x10($sp)
  addiu      $sp, $sp, 0x18
```
