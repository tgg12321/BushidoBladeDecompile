glabel func_80052CD4
    mfc2   $t0, $9
    mfc2   $t1, $10
    sra    $t0, $t0, 2
    sra    $t1, $t1, 2
    sw     $t0, 0($a0)
    sw     $t1, 0($a1)
    jr     $ra
    nop
