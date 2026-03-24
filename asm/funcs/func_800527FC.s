glabel func_800527FC
    /* 42FFC 800527FC 0000888C */  lw         $t0, 0x0($a0)
    /* 43000 80052800 04008A84 */  lh         $t2, 0x4($a0)
    /* 43004 80052804 024C0800 */  srl        $t1, $t0, 16
    /* 43008 80052808 00100234 */  ori        $v0, $zero, 0x1000
    /* 4300C 8005280C 22104600 */  sub        $v0, $v0, $a2 /* handwritten instruction */
    /* 43010 80052810 00408248 */  mtc2       $v0, $8 /* handwritten instruction */
    /* 43014 80052814 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 43018 80052818 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 4301C 8005281C 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 43020 80052820 0000A88C */  lw         $t0, 0x0($a1)
    /* 43024 80052824 00000000 */  nop
    /* 43028 80052828 3D00984B */  gpf        1
    /* 4302C 8005282C 0400AA84 */  lh         $t2, 0x4($a1)
    /* 43030 80052830 034C0800 */  sra        $t1, $t0, 16
    /* 43034 80052834 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 43038 80052838 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 4303C 8005283C 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 43040 80052840 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 43044 80052844 0800898C */  lw         $t1, 0x8($a0)
    /* 43048 80052848 00000000 */  nop
    /* 4304C 8005284C 3E00A84B */  gpl        1
    /* 43050 80052850 06008884 */  lh         $t0, 0x6($a0)
    /* 43054 80052854 02540900 */  srl        $t2, $t1, 16
    /* 43058 80052858 00480B48 */  mfc2       $t3, $9 /* handwritten instruction */
    /* 4305C 8005285C 00500C48 */  mfc2       $t4, $10 /* handwritten instruction */
    /* 43060 80052860 00580D48 */  mfc2       $t5, $11 /* handwritten instruction */
    /* 43064 80052864 0000EBA4 */  sh         $t3, 0x0($a3)
    /* 43068 80052868 0200ECA4 */  sh         $t4, 0x2($a3)
    /* 4306C 8005286C 0400EDA4 */  sh         $t5, 0x4($a3)
    /* 43070 80052870 00408248 */  mtc2       $v0, $8 /* handwritten instruction */
    /* 43074 80052874 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 43078 80052878 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 4307C 8005287C 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 43080 80052880 0800A98C */  lw         $t1, 0x8($a1)
    /* 43084 80052884 00000000 */  nop
    /* 43088 80052888 3D00984B */  gpf        1
    /* 4308C 8005288C 0600A884 */  lh         $t0, 0x6($a1)
    /* 43090 80052890 02540900 */  srl        $t2, $t1, 16
    /* 43094 80052894 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 43098 80052898 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 4309C 8005289C 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 430A0 800528A0 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 430A4 800528A4 0C00888C */  lw         $t0, 0xC($a0)
    /* 430A8 800528A8 00000000 */  nop
    /* 430AC 800528AC 3E00A84B */  gpl        1
    /* 430B0 800528B0 10008A84 */  lh         $t2, 0x10($a0)
    /* 430B4 800528B4 024C0800 */  srl        $t1, $t0, 16
    /* 430B8 800528B8 00480B48 */  mfc2       $t3, $9 /* handwritten instruction */
    /* 430BC 800528BC 00500C48 */  mfc2       $t4, $10 /* handwritten instruction */
    /* 430C0 800528C0 00580D48 */  mfc2       $t5, $11 /* handwritten instruction */
    /* 430C4 800528C4 0600EBA4 */  sh         $t3, 0x6($a3)
    /* 430C8 800528C8 0800ECA4 */  sh         $t4, 0x8($a3)
    /* 430CC 800528CC 0A00EDA4 */  sh         $t5, 0xA($a3)
    /* 430D0 800528D0 00408248 */  mtc2       $v0, $8 /* handwritten instruction */
    /* 430D4 800528D4 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 430D8 800528D8 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 430DC 800528DC 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 430E0 800528E0 0C00A88C */  lw         $t0, 0xC($a1)
    /* 430E4 800528E4 00000000 */  nop
    /* 430E8 800528E8 3D00984B */  gpf        1
    /* 430EC 800528EC 1000AA84 */  lh         $t2, 0x10($a1)
    /* 430F0 800528F0 024C0800 */  srl        $t1, $t0, 16
    /* 430F4 800528F4 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 430F8 800528F8 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 430FC 800528FC 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 43100 80052900 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 43104 80052904 00000000 */  nop
    /* 43108 80052908 00000000 */  nop
    /* 4310C 8005290C 3E00A84B */  gpl        1
    /* 43110 80052910 00480B48 */  mfc2       $t3, $9 /* handwritten instruction */
    /* 43114 80052914 00480B48 */  mfc2       $t3, $9 /* handwritten instruction */
    /* 43118 80052918 00500C48 */  mfc2       $t4, $10 /* handwritten instruction */
    /* 4311C 8005291C 00580D48 */  mfc2       $t5, $11 /* handwritten instruction */
    /* 43120 80052920 0C00EBA4 */  sh         $t3, 0xC($a3)
    /* 43124 80052924 0E00ECA4 */  sh         $t4, 0xE($a3)
    /* 43128 80052928 0800E003 */  jr         $ra
    /* 4312C 8005292C 1000EDA4 */   sh        $t5, 0x10($a3)
endlabel func_800527FC
