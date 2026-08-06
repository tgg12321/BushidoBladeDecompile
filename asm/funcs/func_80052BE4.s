glabel func_80052BE4
    cfc2   $t0, $21
    cfc2   $t1, $22
    cfc2   $t2, $23
    srl    $t0, $t0, 4
    srl    $t1, $t1, 4
    srl    $t2, $t2, 4
    sb     $t0, 0($a0)
    sb     $t1, 1($a0)
    sb     $t2, 2($a0)
    jr     $ra
    nop
