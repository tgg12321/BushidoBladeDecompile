glabel RotTransPers4
    lwc2   $0, 0($a0)
    lwc2   $1, 4($a0)
    lwc2   $2, 0($a1)
    lwc2   $3, 4($a1)
    lwc2   $4, 0($a2)
    lwc2   $5, 4($a2)
    nop
    rtpt
    lw     $t0, 16($sp)
    lw     $t1, 20($sp)
    lw     $t2, 24($sp)
    swc2   $12, 0($t0)
    swc2   $13, 0($t1)
    swc2   $14, 0($t2)
    cfc2   $v1, $31
    lwc2   $0, 0($a3)
    lwc2   $1, 4($a3)
    nop
    rtps
    lw     $t0, 28($sp)
    lw     $t1, 32($sp)
    lw     $t2, 36($sp)
    swc2   $14, 0($t0)
    swc2   $8, 0($t1)
    cfc2   $t0, $31
    mfc2   $v0, $19
    or     $t0, $t0, $v1
    sw     $t0, 0($t2)
    jr     $ra
    sra    $v0, $v0, 2
    nop
    nop
endlabel RotTransPers4
