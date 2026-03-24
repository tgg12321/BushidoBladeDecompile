glabel func_8007EDBC
    /* 6F5BC 8007EDBC 0000888C */  lw         $t0, 0x0($a0)
    /* 6F5C0 8007EDC0 0000AB8C */  lw         $t3, 0x0($a1)
    /* 6F5C4 8007EDC4 FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F5C8 8007EDC8 004C0900 */  sll        $t1, $t1, 16
    /* 6F5CC 8007EDCC 034C0900 */  sra        $t1, $t1, 16
    /* 6F5D0 8007EDD0 19002B01 */  multu      $t1, $t3
    /* 6F5D4 8007EDD4 0400AC8C */  lw         $t4, 0x4($a1)
    /* 6F5D8 8007EDD8 03540800 */  sra        $t2, $t0, 16
    /* 6F5DC 8007EDDC 0800AD8C */  lw         $t5, 0x8($a1)
    /* 6F5E0 8007EDE0 0400888C */  lw         $t0, 0x4($a0)
    /* 6F5E4 8007EDE4 21108000 */  addu       $v0, $a0, $zero
    /* 6F5E8 8007EDE8 12480000 */  mflo       $t1
    /* 6F5EC 8007EDEC 034B0900 */  sra        $t1, $t1, 12
    /* 6F5F0 8007EDF0 FFFF2931 */  andi       $t1, $t1, 0xFFFF
    /* 6F5F4 8007EDF4 19004C01 */  multu      $t2, $t4
    /* 6F5F8 8007EDF8 12500000 */  mflo       $t2
    /* 6F5FC 8007EDFC 03530A00 */  sra        $t2, $t2, 12
    /* 6F600 8007EE00 00540A00 */  sll        $t2, $t2, 16
    /* 6F604 8007EE04 25482A01 */  or         $t1, $t1, $t2
    /* 6F608 8007EE08 000089AC */  sw         $t1, 0x0($a0)
    /* 6F60C 8007EE0C FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F610 8007EE10 004C0900 */  sll        $t1, $t1, 16
    /* 6F614 8007EE14 034C0900 */  sra        $t1, $t1, 16
    /* 6F618 8007EE18 19002D01 */  multu      $t1, $t5
    /* 6F61C 8007EE1C 03540800 */  sra        $t2, $t0, 16
    /* 6F620 8007EE20 0800888C */  lw         $t0, 0x8($a0)
    /* 6F624 8007EE24 12480000 */  mflo       $t1
    /* 6F628 8007EE28 034B0900 */  sra        $t1, $t1, 12
    /* 6F62C 8007EE2C FFFF2931 */  andi       $t1, $t1, 0xFFFF
    /* 6F630 8007EE30 19004B01 */  multu      $t2, $t3
    /* 6F634 8007EE34 12500000 */  mflo       $t2
    /* 6F638 8007EE38 03530A00 */  sra        $t2, $t2, 12
    /* 6F63C 8007EE3C 00540A00 */  sll        $t2, $t2, 16
    /* 6F640 8007EE40 25482A01 */  or         $t1, $t1, $t2
    /* 6F644 8007EE44 040089AC */  sw         $t1, 0x4($a0)
    /* 6F648 8007EE48 FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F64C 8007EE4C 004C0900 */  sll        $t1, $t1, 16
    /* 6F650 8007EE50 034C0900 */  sra        $t1, $t1, 16
    /* 6F654 8007EE54 19002C01 */  multu      $t1, $t4
    /* 6F658 8007EE58 03540800 */  sra        $t2, $t0, 16
    /* 6F65C 8007EE5C 0C00888C */  lw         $t0, 0xC($a0)
    /* 6F660 8007EE60 12480000 */  mflo       $t1
    /* 6F664 8007EE64 034B0900 */  sra        $t1, $t1, 12
    /* 6F668 8007EE68 FFFF2931 */  andi       $t1, $t1, 0xFFFF
    /* 6F66C 8007EE6C 19004D01 */  multu      $t2, $t5
    /* 6F670 8007EE70 12500000 */  mflo       $t2
    /* 6F674 8007EE74 03530A00 */  sra        $t2, $t2, 12
    /* 6F678 8007EE78 00540A00 */  sll        $t2, $t2, 16
    /* 6F67C 8007EE7C 25482A01 */  or         $t1, $t1, $t2
    /* 6F680 8007EE80 080089AC */  sw         $t1, 0x8($a0)
    /* 6F684 8007EE84 FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F688 8007EE88 004C0900 */  sll        $t1, $t1, 16
    /* 6F68C 8007EE8C 034C0900 */  sra        $t1, $t1, 16
    /* 6F690 8007EE90 19002B01 */  multu      $t1, $t3
    /* 6F694 8007EE94 03540800 */  sra        $t2, $t0, 16
    /* 6F698 8007EE98 1000888C */  lw         $t0, 0x10($a0)
    /* 6F69C 8007EE9C 12480000 */  mflo       $t1
    /* 6F6A0 8007EEA0 034B0900 */  sra        $t1, $t1, 12
    /* 6F6A4 8007EEA4 FFFF2931 */  andi       $t1, $t1, 0xFFFF
    /* 6F6A8 8007EEA8 19004C01 */  multu      $t2, $t4
    /* 6F6AC 8007EEAC 12500000 */  mflo       $t2
    /* 6F6B0 8007EEB0 03530A00 */  sra        $t2, $t2, 12
    /* 6F6B4 8007EEB4 00540A00 */  sll        $t2, $t2, 16
    /* 6F6B8 8007EEB8 25482A01 */  or         $t1, $t1, $t2
    /* 6F6BC 8007EEBC 0C0089AC */  sw         $t1, 0xC($a0)
    /* 6F6C0 8007EEC0 FFFF0931 */  andi       $t1, $t0, 0xFFFF
    /* 6F6C4 8007EEC4 004C0900 */  sll        $t1, $t1, 16
    /* 6F6C8 8007EEC8 034C0900 */  sra        $t1, $t1, 16
    /* 6F6CC 8007EECC 19002D01 */  multu      $t1, $t5
    /* 6F6D0 8007EED0 12480000 */  mflo       $t1
    /* 6F6D4 8007EED4 034B0900 */  sra        $t1, $t1, 12
    /* 6F6D8 8007EED8 0800E003 */  jr         $ra
    /* 6F6DC 8007EEDC 100089AC */   sw        $t1, 0x10($a0)
endlabel func_8007EDBC
