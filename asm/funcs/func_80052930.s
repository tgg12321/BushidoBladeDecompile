glabel func_80052930
    /* 43130 80052930 0000888C */  lw         $t0, 0x0($a0)
    /* 43134 80052934 0400898C */  lw         $t1, 0x4($a0)
    /* 43138 80052938 08008A8C */  lw         $t2, 0x8($a0)
    /* 4313C 8005293C 0C008B8C */  lw         $t3, 0xC($a0)
    /* 43140 80052940 10008C8C */  lw         $t4, 0x10($a0)
    /* 43144 80052944 FFFF193C */  lui        $t9, (0xFFFF0000 >> 16)
    /* 43148 80052948 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 4314C 8005294C 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 43150 80052950 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 43154 80052954 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 43158 80052958 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 4315C 8005295C 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 43160 80052960 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 43164 80052964 0038C048 */  ctc2       $zero, $7 /* handwritten instruction */
    /* 43168 80052968 0000A88C */  lw         $t0, 0x0($a1)
    /* 4316C 8005296C 0400A98C */  lw         $t1, 0x4($a1)
    /* 43170 80052970 0800AA8C */  lw         $t2, 0x8($a1)
    /* 43174 80052974 0C00AB8C */  lw         $t3, 0xC($a1)
    /* 43178 80052978 1000AC8C */  lw         $t4, 0x10($a1)
    /* 4317C 8005297C 24683901 */  and        $t5, $t1, $t9
    /* 43180 80052980 FFFF0E31 */  andi       $t6, $t0, 0xFFFF
    /* 43184 80052984 2568AE01 */  or         $t5, $t5, $t6
    /* 43188 80052988 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 4318C 8005298C FFFF6E31 */  andi       $t6, $t3, 0xFFFF
    /* 43190 80052990 00088E48 */  mtc2       $t6, $1 /* handwritten instruction */
    /* 43194 80052994 00140A00 */  sll        $v0, $t2, 16
    /* 43198 80052998 00000000 */  nop
    /* 4319C 8005299C 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 431A0 800529A0 021C0800 */  srl        $v1, $t0, 16
    /* 431A4 800529A4 25104300 */  or         $v0, $v0, $v1
    /* 431A8 800529A8 021C0B00 */  srl        $v1, $t3, 16
    /* 431AC 800529AC 00480D48 */  mfc2       $t5, $9 /* handwritten instruction */
    /* 431B0 800529B0 00500E48 */  mfc2       $t6, $10 /* handwritten instruction */
    /* 431B4 800529B4 00580F48 */  mfc2       $t7, $11 /* handwritten instruction */
    /* 431B8 800529B8 00008248 */  mtc2       $v0, $0 /* handwritten instruction */
    /* 431BC 800529BC 00088348 */  mtc2       $v1, $1 /* handwritten instruction */
    /* 431C0 800529C0 0000CDA4 */  sh         $t5, 0x0($a2)
    /* 431C4 800529C4 0600CEA4 */  sh         $t6, 0x6($a2)
    /* 431C8 800529C8 0C00CFA4 */  sh         $t7, 0xC($a2)
    /* 431CC 800529CC 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 431D0 800529D0 24105901 */  and        $v0, $t2, $t9
    /* 431D4 800529D4 FFFF2331 */  andi       $v1, $t1, 0xFFFF
    /* 431D8 800529D8 25104300 */  or         $v0, $v0, $v1
    /* 431DC 800529DC FFFF8331 */  andi       $v1, $t4, 0xFFFF
    /* 431E0 800529E0 00480D48 */  mfc2       $t5, $9 /* handwritten instruction */
    /* 431E4 800529E4 00500E48 */  mfc2       $t6, $10 /* handwritten instruction */
    /* 431E8 800529E8 00580F48 */  mfc2       $t7, $11 /* handwritten instruction */
    /* 431EC 800529EC 00008248 */  mtc2       $v0, $0 /* handwritten instruction */
    /* 431F0 800529F0 00088348 */  mtc2       $v1, $1 /* handwritten instruction */
    /* 431F4 800529F4 0200CDA4 */  sh         $t5, 0x2($a2)
    /* 431F8 800529F8 0800CEA4 */  sh         $t6, 0x8($a2)
    /* 431FC 800529FC 0E00CFA4 */  sh         $t7, 0xE($a2)
    /* 43200 80052A00 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 43204 80052A04 00480D48 */  mfc2       $t5, $9 /* handwritten instruction */
    /* 43208 80052A08 00500E48 */  mfc2       $t6, $10 /* handwritten instruction */
    /* 4320C 80052A0C 00580F48 */  mfc2       $t7, $11 /* handwritten instruction */
    /* 43210 80052A10 0400CDA4 */  sh         $t5, 0x4($a2)
    /* 43214 80052A14 0A00CEA4 */  sh         $t6, 0xA($a2)
    /* 43218 80052A18 0800E003 */  jr         $ra
    /* 4321C 80052A1C 1000CFA4 */   sh        $t7, 0x10($a2)
endlabel func_80052930
