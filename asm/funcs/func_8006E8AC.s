glabel func_8006E8AC
    /* 5F0AC 8006E8AC 40100400 */  sll        $v0, $a0, 1
    /* 5F0B0 8006E8B0 21104400 */  addu       $v0, $v0, $a0
    /* 5F0B4 8006E8B4 80100200 */  sll        $v0, $v0, 2
    /* 5F0B8 8006E8B8 23104400 */  subu       $v0, $v0, $a0
    /* 5F0BC 8006E8BC E004838F */  lw         $v1, %gp_rel(D_800A35AC)($gp)
    /* 5F0C0 8006E8C0 80100200 */  sll        $v0, $v0, 2
    /* 5F0C4 8006E8C4 0800E003 */  jr         $ra
    /* 5F0C8 8006E8C8 21106200 */   addu      $v0, $v1, $v0
endlabel func_8006E8AC
