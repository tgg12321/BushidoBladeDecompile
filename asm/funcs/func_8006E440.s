glabel func_8006E440
    /* 5EC40 8006E440 0000838C */  lw         $v1, 0x0($a0)
    /* 5EC44 8006E444 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 5EC48 8006E448 0B006210 */  beq        $v1, $v0, .L8006E478
    /* 5EC4C 8006E44C 21288000 */   addu      $a1, $a0, $zero
    /* 5EC50 8006E450 FFFF0324 */  addiu      $v1, $zero, -0x1
    /* 5EC54 8006E454 0000828C */  lw         $v0, 0x0($a0)
    /* 5EC58 8006E458 00000000 */  nop
    /* 5EC5C 8006E45C 21104500 */  addu       $v0, $v0, $a1
  .L8006E460:
    /* 5EC60 8006E460 000082AC */  sw         $v0, 0x0($a0)
    /* 5EC64 8006E464 04008424 */  addiu      $a0, $a0, 0x4
    /* 5EC68 8006E468 0000828C */  lw         $v0, 0x0($a0)
    /* 5EC6C 8006E46C 00000000 */  nop
    /* 5EC70 8006E470 FBFF4314 */  bne        $v0, $v1, .L8006E460
    /* 5EC74 8006E474 21104500 */   addu      $v0, $v0, $a1
  .L8006E478:
    /* 5EC78 8006E478 0800E003 */  jr         $ra
    /* 5EC7C 8006E47C 00000000 */   nop
endlabel func_8006E440
