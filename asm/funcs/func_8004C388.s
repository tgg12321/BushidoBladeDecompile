glabel func_8004C388
    /* 3CB88 8004C388 00008884 */  lh         $t0, 0x0($a0)
    /* 3CB8C 8004C38C 02008984 */  lh         $t1, 0x2($a0)
    /* 3CB90 8004C390 04008A84 */  lh         $t2, 0x4($a0)
    /* 3CB94 8004C394 0000AB84 */  lh         $t3, 0x0($a1)
    /* 3CB98 8004C398 0200AC84 */  lh         $t4, 0x2($a1)
    /* 3CB9C 8004C39C 0400AD84 */  lh         $t5, 0x4($a1)
    /* 3CBA0 8004C3A0 20400B01 */  add        $t0, $t0, $t3 /* handwritten instruction */
    /* 3CBA4 8004C3A4 20482C01 */  add        $t1, $t1, $t4 /* handwritten instruction */
    /* 3CBA8 8004C3A8 20504D01 */  add        $t2, $t2, $t5 /* handwritten instruction */
    /* 3CBAC 8004C3AC 43400800 */  sra        $t0, $t0, 1
    /* 3CBB0 8004C3B0 43480900 */  sra        $t1, $t1, 1
    /* 3CBB4 8004C3B4 43500A00 */  sra        $t2, $t2, 1
    /* 3CBB8 8004C3B8 0000C8A4 */  sh         $t0, 0x0($a2)
    /* 3CBBC 8004C3BC 0200C9A4 */  sh         $t1, 0x2($a2)
    /* 3CBC0 8004C3C0 0400CAA4 */  sh         $t2, 0x4($a2)
    /* 3CBC4 8004C3C4 06008B94 */  lhu        $t3, 0x6($a0)
    /* 3CBC8 8004C3C8 0600AC94 */  lhu        $t4, 0x6($a1)
    /* 3CBCC 8004C3CC 00FF6831 */  andi       $t0, $t3, 0xFF00
    /* 3CBD0 8004C3D0 00FF8931 */  andi       $t1, $t4, 0xFF00
    /* 3CBD4 8004C3D4 FF006B31 */  andi       $t3, $t3, 0xFF
    /* 3CBD8 8004C3D8 FF008C31 */  andi       $t4, $t4, 0xFF
    /* 3CBDC 8004C3DC 20586C01 */  add        $t3, $t3, $t4 /* handwritten instruction */
    /* 3CBE0 8004C3E0 20400901 */  add        $t0, $t0, $t1 /* handwritten instruction */
    /* 3CBE4 8004C3E4 42580B00 */  srl        $t3, $t3, 1
    /* 3CBE8 8004C3E8 42400800 */  srl        $t0, $t0, 1
    /* 3CBEC 8004C3EC FF006B31 */  andi       $t3, $t3, 0xFF
    /* 3CBF0 8004C3F0 00FF0831 */  andi       $t0, $t0, 0xFF00
    /* 3CBF4 8004C3F4 25586801 */  or         $t3, $t3, $t0
    /* 3CBF8 8004C3F8 0800E003 */  jr         $ra
    /* 3CBFC 8004C3FC 0600CBA4 */   sh        $t3, 0x6($a2)
endlabel func_8004C388
