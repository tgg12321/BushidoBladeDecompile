glabel func_8004A808
    /* 3B008 8004A808 FDFF8424 */  addiu      $a0, $a0, -0x3
    /* 3B00C 8004A80C 00000E96 */  lhu        $t6, 0x0($s0)
    /* 3B010 8004A810 02000F96 */  lhu        $t7, 0x2($s0)
    /* 3B014 8004A814 04001896 */  lhu        $t8, 0x4($s0)
    /* 3B018 8004A818 007C0F00 */  sll        $t7, $t7, 16
    /* 3B01C 8004A81C 2570CF01 */  or         $t6, $t6, $t7
    /* 3B020 8004A820 00008E48 */  mtc2       $t6, $0 /* handwritten instruction */
    /* 3B024 8004A824 00089848 */  mtc2       $t8, $1 /* handwritten instruction */
    /* 3B028 8004A828 06000E96 */  lhu        $t6, 0x6($s0)
    /* 3B02C 8004A82C 08000F96 */  lhu        $t7, 0x8($s0)
    /* 3B030 8004A830 0A001896 */  lhu        $t8, 0xA($s0)
    /* 3B034 8004A834 007C0F00 */  sll        $t7, $t7, 16
    /* 3B038 8004A838 2570CF01 */  or         $t6, $t6, $t7
    /* 3B03C 8004A83C 00108E48 */  mtc2       $t6, $2 /* handwritten instruction */
    /* 3B040 8004A840 00189848 */  mtc2       $t8, $3 /* handwritten instruction */
    /* 3B044 8004A844 0C000E96 */  lhu        $t6, 0xC($s0)
    /* 3B048 8004A848 0E000F96 */  lhu        $t7, 0xE($s0)
    /* 3B04C 8004A84C 10001896 */  lhu        $t8, 0x10($s0)
    /* 3B050 8004A850 007C0F00 */  sll        $t7, $t7, 16
    /* 3B054 8004A854 2570CF01 */  or         $t6, $t6, $t7
    /* 3B058 8004A858 00208E48 */  mtc2       $t6, $4 /* handwritten instruction */
    /* 3B05C 8004A85C 00289848 */  mtc2       $t8, $5 /* handwritten instruction */
    /* 3B060 8004A860 00000000 */  nop
    /* 3B064 8004A864 00000000 */  nop
    /* 3B068 8004A868 3000284A */  rtpt
    /* 3B06C 8004A86C 222A0108 */  j          .L8004A888
    /* 3B070 8004A870 12001026 */   addiu     $s0, $s0, 0x12
  .L8004A874:
    /* 3B074 8004A874 3000284A */  rtpt
    /* 3B078 8004A878 0600C624 */  addiu      $a2, $a2, 0x6
    /* 3B07C 8004A87C FAFFC8A4 */  sh         $t0, -0x6($a2)
    /* 3B080 8004A880 FCFFC9A4 */  sh         $t1, -0x4($a2)
    /* 3B084 8004A884 FEFFCAA4 */  sh         $t2, -0x2($a2)
  .L8004A888:
    /* 3B088 8004A888 0000ACE8 */  swc2       $12, 0x0($a1)
    /* 3B08C 8004A88C 0400ADE8 */  swc2       $13, 0x4($a1)
    /* 3B090 8004A890 0800AEE8 */  swc2       $14, 0x8($a1)
    /* 3B094 8004A894 00880848 */  mfc2       $t0, $17 /* handwritten instruction */
    /* 3B098 8004A898 00900948 */  mfc2       $t1, $18 /* handwritten instruction */
    /* 3B09C 8004A89C 00980A48 */  mfc2       $t2, $19 /* handwritten instruction */
    /* 3B0A0 8004A8A0 0C00A524 */  addiu      $a1, $a1, 0xC
    /* 3B0A4 8004A8A4 19008018 */  blez       $a0, .L8004A90C
    /* 3B0A8 8004A8A8 00000000 */   nop
    /* 3B0AC 8004A8AC FDFF8424 */  addiu      $a0, $a0, -0x3
    /* 3B0B0 8004A8B0 00000E96 */  lhu        $t6, 0x0($s0)
    /* 3B0B4 8004A8B4 02000F96 */  lhu        $t7, 0x2($s0)
    /* 3B0B8 8004A8B8 04001896 */  lhu        $t8, 0x4($s0)
    /* 3B0BC 8004A8BC 007C0F00 */  sll        $t7, $t7, 16
    /* 3B0C0 8004A8C0 2570CF01 */  or         $t6, $t6, $t7
    /* 3B0C4 8004A8C4 00008E48 */  mtc2       $t6, $0 /* handwritten instruction */
    /* 3B0C8 8004A8C8 00089848 */  mtc2       $t8, $1 /* handwritten instruction */
    /* 3B0CC 8004A8CC 06000E96 */  lhu        $t6, 0x6($s0)
    /* 3B0D0 8004A8D0 08000F96 */  lhu        $t7, 0x8($s0)
    /* 3B0D4 8004A8D4 0A001896 */  lhu        $t8, 0xA($s0)
    /* 3B0D8 8004A8D8 007C0F00 */  sll        $t7, $t7, 16
    /* 3B0DC 8004A8DC 2570CF01 */  or         $t6, $t6, $t7
    /* 3B0E0 8004A8E0 00108E48 */  mtc2       $t6, $2 /* handwritten instruction */
    /* 3B0E4 8004A8E4 00189848 */  mtc2       $t8, $3 /* handwritten instruction */
    /* 3B0E8 8004A8E8 0C000E96 */  lhu        $t6, 0xC($s0)
    /* 3B0EC 8004A8EC 0E000F96 */  lhu        $t7, 0xE($s0)
    /* 3B0F0 8004A8F0 10001896 */  lhu        $t8, 0x10($s0)
    /* 3B0F4 8004A8F4 007C0F00 */  sll        $t7, $t7, 16
    /* 3B0F8 8004A8F8 2570CF01 */  or         $t6, $t6, $t7
    /* 3B0FC 8004A8FC 00208E48 */  mtc2       $t6, $4 /* handwritten instruction */
    /* 3B100 8004A900 00289848 */  mtc2       $t8, $5 /* handwritten instruction */
    /* 3B104 8004A904 1D2A0108 */  j          .L8004A874
    /* 3B108 8004A908 12001026 */   addiu     $s0, $s0, 0x12
  .L8004A90C:
    /* 3B10C 8004A90C 0000C8A4 */  sh         $t0, 0x0($a2)
    /* 3B110 8004A910 0200C9A4 */  sh         $t1, 0x2($a2)
    /* 3B114 8004A914 0400CAA4 */  sh         $t2, 0x4($a2)
    /* 3B118 8004A918 20208400 */  add        $a0, $a0, $a0 /* handwritten instruction */
    /* 3B11C 8004A91C 20408400 */  add        $t0, $a0, $a0 /* handwritten instruction */
    /* 3B120 8004A920 20200401 */  add        $a0, $t0, $a0 /* handwritten instruction */
    /* 3B124 8004A924 20800402 */  add        $s0, $s0, $a0 /* handwritten instruction */
    /* 3B128 8004A928 03000832 */  andi       $t0, $s0, 0x3
    /* 3B12C 8004A92C 20800802 */  add        $s0, $s0, $t0 /* handwritten instruction */
    /* 3B130 8004A930 0800E003 */  jr         $ra
    /* 3B134 8004A934 00000000 */   nop
endlabel func_8004A808
