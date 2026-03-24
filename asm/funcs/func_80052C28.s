glabel func_80052C28
    /* 43428 80052C28 03000824 */  addiu      $t0, $zero, 0x3
    /* 4342C 80052C2C 22280501 */  sub        $a1, $t0, $a1 /* handwritten instruction */
    /* 43430 80052C30 0620A400 */  srlv       $a0, $a0, $a1
    /* 43434 80052C34 C2420400 */  srl        $t0, $a0, 11
    /* 43438 80052C38 FF078430 */  andi       $a0, $a0, 0x7FF
    /* 4343C 80052C3C 00F08420 */  addi       $a0, $a0, -0x1000 /* handwritten instruction */
    /* 43440 80052C40 07200401 */  srav       $a0, $a0, $t0
    /* 43444 80052C44 0800E003 */  jr         $ra
    /* 43448 80052C48 FF0F8230 */   andi      $v0, $a0, 0xFFF
endlabel func_80052C28
