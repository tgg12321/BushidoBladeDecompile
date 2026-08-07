glabel func_800880B8
    /* 788B8 800880B8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 788BC 800880BC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 788C0 800880C0 2138C000 */  addu       $a3, $a2, $zero
    /* 788C4 800880C4 002C0500 */  sll        $a1, $a1, 16
    /* 788C8 800880C8 032C0500 */  sra        $a1, $a1, 16
    /* 788CC 800880CC 3A20020C */  jal        func_800880E8
    /* 788D0 800880D0 01000624 */   addiu     $a2, $zero, 0x1
    /* 788D4 800880D4 00140200 */  sll        $v0, $v0, 16
    /* 788D8 800880D8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 788DC 800880DC 03140200 */  sra        $v0, $v0, 16
    /* 788E0 800880E0 0800E003 */  jr         $ra
    /* 788E4 800880E4 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_800880B8
