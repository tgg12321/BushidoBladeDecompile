glabel func_8006CBD4
    /* 5D3D4 8006CBD4 00310400 */  sll        $a2, $a0, 4
    /* 5D3D8 8006CBD8 10000224 */  addiu      $v0, $zero, 0x10
    /* 5D3DC 8006CBDC 0410C200 */  sllv       $v0, $v0, $a2
    /* 5D3E0 8006CBE0 2410A200 */  and        $v0, $a1, $v0
    /* 5D3E4 8006CBE4 03004010 */  beqz       $v0, .L8006CBF4
    /* 5D3E8 8006CBE8 40000224 */   addiu     $v0, $zero, 0x40
    /* 5D3EC 8006CBEC 0EB30108 */  j          .L8006CC38
    /* 5D3F0 8006CBF0 01000724 */   addiu     $a3, $zero, 0x1
  .L8006CBF4:
    /* 5D3F4 8006CBF4 0410C200 */  sllv       $v0, $v0, $a2
    /* 5D3F8 8006CBF8 2410A200 */  and        $v0, $a1, $v0
    /* 5D3FC 8006CBFC 03004010 */  beqz       $v0, .L8006CC0C
    /* 5D400 8006CC00 80000224 */   addiu     $v0, $zero, 0x80
    /* 5D404 8006CC04 0EB30108 */  j          .L8006CC38
    /* 5D408 8006CC08 02000724 */   addiu     $a3, $zero, 0x2
  .L8006CC0C:
    /* 5D40C 8006CC0C 0410C200 */  sllv       $v0, $v0, $a2
    /* 5D410 8006CC10 2410A200 */  and        $v0, $a1, $v0
    /* 5D414 8006CC14 03004010 */  beqz       $v0, .L8006CC24
    /* 5D418 8006CC18 20000224 */   addiu     $v0, $zero, 0x20
    /* 5D41C 8006CC1C 0EB30108 */  j          .L8006CC38
    /* 5D420 8006CC20 03000724 */   addiu     $a3, $zero, 0x3
  .L8006CC24:
    /* 5D424 8006CC24 0410C200 */  sllv       $v0, $v0, $a2
    /* 5D428 8006CC28 2410A200 */  and        $v0, $a1, $v0
    /* 5D42C 8006CC2C 03004010 */  beqz       $v0, .L8006CC3C
    /* 5D430 8006CC30 21280000 */   addu      $a1, $zero, $zero
    /* 5D434 8006CC34 21380000 */  addu       $a3, $zero, $zero
  .L8006CC38:
    /* 5D438 8006CC38 21280000 */  addu       $a1, $zero, $zero
  .L8006CC3C:
    /* 5D43C 8006CC3C 40400400 */  sll        $t0, $a0, 1
    /* 5D440 8006CC40 01000224 */  addiu      $v0, $zero, 0x1
    /* 5D444 8006CC44 0410E200 */  sllv       $v0, $v0, $a3
    /* 5D448 8006CC48 80180400 */  sll        $v1, $a0, 2
    /* 5D44C 8006CC4C 04306200 */  sllv       $a2, $v0, $v1
    /* 5D450 8006CC50 27380600 */  nor        $a3, $zero, $a2
  .L8006CC54:
    /* 5D454 8006CC54 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5D458 8006CC58 001C0500 */  sll        $v1, $a1, 16
    /* 5D45C 8006CC5C 21100201 */  addu       $v0, $t0, $v0
    /* 5D460 8006CC60 28004284 */  lh         $v0, 0x28($v0)
    /* 5D464 8006CC64 03240300 */  sra        $a0, $v1, 16
    /* 5D468 8006CC68 07008214 */  bne        $a0, $v0, .L8006CC88
    /* 5D46C 8006CC6C 00000000 */   nop
    /* 5D470 8006CC70 5804838F */  lw         $v1, %gp_rel(D_800A3524)($gp)
    /* 5D474 8006CC74 00000000 */  nop
    /* 5D478 8006CC78 21186400 */  addu       $v1, $v1, $a0
    /* 5D47C 8006CC7C 17006290 */  lbu        $v0, 0x17($v1)
    /* 5D480 8006CC80 28B30108 */  j          .L8006CCA0
    /* 5D484 8006CC84 25104600 */   or        $v0, $v0, $a2
  .L8006CC88:
    /* 5D488 8006CC88 5804838F */  lw         $v1, %gp_rel(D_800A3524)($gp)
    /* 5D48C 8006CC8C 00000000 */  nop
    /* 5D490 8006CC90 21186400 */  addu       $v1, $v1, $a0
    /* 5D494 8006CC94 17006290 */  lbu        $v0, 0x17($v1)
    /* 5D498 8006CC98 00000000 */  nop
    /* 5D49C 8006CC9C 24104700 */  and        $v0, $v0, $a3
  .L8006CCA0:
    /* 5D4A0 8006CCA0 170062A0 */  sb         $v0, 0x17($v1)
    /* 5D4A4 8006CCA4 0100A224 */  addiu      $v0, $a1, 0x1
    /* 5D4A8 8006CCA8 21284000 */  addu       $a1, $v0, $zero
    /* 5D4AC 8006CCAC 00140200 */  sll        $v0, $v0, 16
    /* 5D4B0 8006CCB0 03140200 */  sra        $v0, $v0, 16
    /* 5D4B4 8006CCB4 03004228 */  slti       $v0, $v0, 0x3
    /* 5D4B8 8006CCB8 E6FF4014 */  bnez       $v0, .L8006CC54
    /* 5D4BC 8006CCBC 00000000 */   nop
    /* 5D4C0 8006CCC0 0800E003 */  jr         $ra
    /* 5D4C4 8006CCC4 00000000 */   nop
endlabel func_8006CBD4
