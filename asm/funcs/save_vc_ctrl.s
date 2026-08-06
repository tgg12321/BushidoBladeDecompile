.include "macro.inc"

.section .text

.set	noat
.set	noreorder
.set noat
.set noreorder
glabel save_vc_ctrl
    addiu   $sp,$sp,-0x8
    beqz    $a2,.L80041530_svc
    addiu   $v1,$a2,-0x1
    addiu   $a2,$zero,-0x1
    addiu   $a1,$a1,0xC
.L80041510_svc:
    lw      $v0,0($a1)
    nop
    beqz    $v0,.L80041524_svc
    addu    $v0,$v0,$a0
    sw      $v0,0($a1)
.L80041524_svc:
    addiu   $v1,$v1,-0x1
    bne     $v1,$a2,.L80041510_svc
    addiu   $a1,$a1,0x68
.L80041530_svc:
    addiu   $sp,$sp,0x8
    jr      $ra
    nop
endlabel save_vc_ctrl
.set	reorder
.set	at
.set reorder
.set at
