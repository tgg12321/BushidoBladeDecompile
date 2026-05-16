# func_8007FF7C -- naming evidence

- Address: 0x8007FF7C
- Size: 38 instructions
- Asm file: `asm/funcs/func_8007FF7C.s`
- Callers (3): ['func_80035FE0', 'func_80036034', 'marionation_camera_Init_80037468']
- Callees (6): ['cdrom_CheckReady', 'cdrom_SetCallbackA', 'cdrom_SetCallbackB', 'debug_printf', 'sys_SetTimer', 'sys_SetVsyncMode']
- Proposed name: `fail_func_8007FF7C`
- Confidence: **low**

## Evidence

### string_adjacent  (rank=low)

- proposed: `fail_func_8007FF7C`
- detail: loads strings: ["0x8001605C='CdInit: Init failed\\n'", 'g_str_cdinit_fail // CD init failure message']

## First 20 instructions

```
  addiu      $sp, $sp, -0x18
  sw         $s0, 0x10($sp)
  addiu      $s0, $zero, 0x4
  sw         $ra, 0x14($sp)
  jal        cdrom_CheckReady
  addiu      $a0, $zero, 0x1
  addiu      $v1, $zero, 0x1
  bne        $v0, $v1, .L8007FFE0
  addiu      $s0, $s0, -0x1
  lui        $a0, %hi(D_80080014)
  addiu      $a0, $a0, %lo(D_80080014)
  jal        cdrom_SetCallbackA
  nop        
  lui        $a0, %hi(D_8008003C)
  addiu      $a0, $a0, %lo(D_8008003C)
  jal        cdrom_SetCallbackB
  nop        
  lui        $a0, %hi(D_80080064)
  addiu      $a0, $a0, %lo(D_80080064)
  jal        sys_SetVsyncMode
```
