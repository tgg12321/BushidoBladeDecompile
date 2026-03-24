glabel func_8006E480
    /* 5EC80 8006E480 00008290 */  lbu        $v0, 0x0($a0)
    /* 5EC84 8006E484 01008390 */  lbu        $v1, 0x1($a0)
    /* 5EC88 8006E488 1FFE4230 */  andi       $v0, $v0, 0xFE1F
    /* 5EC8C 8006E48C C0190300 */  sll        $v1, $v1, 7
    /* 5EC90 8006E490 21104300 */  addu       $v0, $v0, $v1
    /* 5EC94 8006E494 0800E003 */  jr         $ra
    /* 5EC98 8006E498 21104500 */   addu      $v0, $v0, $a1
endlabel func_8006E480
