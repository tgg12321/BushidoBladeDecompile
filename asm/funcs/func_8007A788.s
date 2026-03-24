glabel func_8007A788
    /* 6AF88 8007A788 03008230 */  andi       $v0, $a0, 0x3
    /* 6AF8C 8007A78C C0110200 */  sll        $v0, $v0, 7
    /* 6AF90 8007A790 0300A530 */  andi       $a1, $a1, 0x3
    /* 6AF94 8007A794 40290500 */  sll        $a1, $a1, 5
    /* 6AF98 8007A798 25104500 */  or         $v0, $v0, $a1
    /* 6AF9C 8007A79C 0001E330 */  andi       $v1, $a3, 0x100
    /* 6AFA0 8007A7A0 03190300 */  sra        $v1, $v1, 4
    /* 6AFA4 8007A7A4 25104300 */  or         $v0, $v0, $v1
    /* 6AFA8 8007A7A8 FF03C630 */  andi       $a2, $a2, 0x3FF
    /* 6AFAC 8007A7AC 83310600 */  sra        $a2, $a2, 6
    /* 6AFB0 8007A7B0 25104600 */  or         $v0, $v0, $a2
    /* 6AFB4 8007A7B4 0002E730 */  andi       $a3, $a3, 0x200
    /* 6AFB8 8007A7B8 80380700 */  sll        $a3, $a3, 2
    /* 6AFBC 8007A7BC 0800E003 */  jr         $ra
    /* 6AFC0 8007A7C0 25104700 */   or        $v0, $v0, $a3
endlabel func_8007A788
