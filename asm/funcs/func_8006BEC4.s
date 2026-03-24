glabel func_8006BEC4
    /* 5C6C4 8006BEC4 A0FFBD27 */  addiu      $sp, $sp, -0x60
    /* 5C6C8 8006BEC8 4000B0AF */  sw         $s0, 0x40($sp)
    /* 5C6CC 8006BECC 21808000 */  addu       $s0, $a0, $zero
    /* 5C6D0 8006BED0 4C00B3AF */  sw         $s3, 0x4C($sp)
    /* 5C6D4 8006BED4 2198A000 */  addu       $s3, $a1, $zero
    /* 5C6D8 8006BED8 21280000 */  addu       $a1, $zero, $zero
    /* 5C6DC 8006BEDC 1000A627 */  addiu      $a2, $sp, 0x10
    /* 5C6E0 8006BEE0 0A80083C */  lui        $t0, %hi(D_800A36AC)
    /* 5C6E4 8006BEE4 AC36088D */  lw         $t0, %lo(D_800A36AC)($t0)
    /* 5C6E8 8006BEE8 5800BFAF */  sw         $ra, 0x58($sp)
    /* 5C6EC 8006BEEC 5400B5AF */  sw         $s5, 0x54($sp)
    /* 5C6F0 8006BEF0 5000B4AF */  sw         $s4, 0x50($sp)
    /* 5C6F4 8006BEF4 4800B2AF */  sw         $s2, 0x48($sp)
    /* 5C6F8 8006BEF8 4400B1AF */  sw         $s1, 0x44($sp)
    /* 5C6FC 8006BEFC 340880AF */  sw         $zero, %gp_rel(D_800A3900)($gp)
    /* 5C700 8006BF00 01000831 */  andi       $t0, $t0, 0x1
    /* 5C704 8006BF04 80180800 */  sll        $v1, $t0, 2
    /* 5C708 8006BF08 21186800 */  addu       $v1, $v1, $t0
    /* 5C70C 8006BF0C 00110300 */  sll        $v0, $v1, 4
    /* 5C710 8006BF10 23104300 */  subu       $v0, $v0, $v1
    /* 5C714 8006BF14 80100200 */  sll        $v0, $v0, 2
    /* 5C718 8006BF18 0F80033C */  lui        $v1, %hi(D_800F11E0)
    /* 5C71C 8006BF1C E0116324 */  addiu      $v1, $v1, %lo(D_800F11E0)
    /* 5C720 8006BF20 21104300 */  addu       $v0, $v0, $v1
    /* 5C724 8006BF24 180682AF */  sw         $v0, %gp_rel(D_800A36E4)($gp)
    /* 5C728 8006BF28 40100800 */  sll        $v0, $t0, 1
    /* 5C72C 8006BF2C 21104800 */  addu       $v0, $v0, $t0
    /* 5C730 8006BF30 00110200 */  sll        $v0, $v0, 4
    /* 5C734 8006BF34 0F80033C */  lui        $v1, %hi(D_800F1438)
    /* 5C738 8006BF38 38146324 */  addiu      $v1, $v1, %lo(D_800F1438)
    /* 5C73C 8006BF3C 21104300 */  addu       $v0, $v0, $v1
    /* 5C740 8006BF40 80410800 */  sll        $t0, $t0, 6
    /* 5C744 8006BF44 140682AF */  sw         $v0, %gp_rel(D_800A36E0)($gp)
    /* 5C748 8006BF48 0F80023C */  lui        $v0, %hi(D_800F1498)
    /* 5C74C 8006BF4C 98144224 */  addiu      $v0, $v0, %lo(D_800F1498)
    /* 5C750 8006BF50 21400201 */  addu       $t0, $t0, $v0
    /* 5C754 8006BF54 100688AF */  sw         $t0, %gp_rel(D_800A36DC)($gp)
    /* 5C758 8006BF58 4AAF010C */  jal        func_8006BD28
    /* 5C75C 8006BF5C 21380000 */   addu      $a3, $zero, $zero
    /* 5C760 8006BF60 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5C764 8006BF64 21A80000 */  addu       $s5, $zero, $zero
    /* 5C768 8006BF68 2400428C */  lw         $v0, 0x24($v0)
    /* 5C76C 8006BF6C 80801000 */  sll        $s0, $s0, 2
    /* 5C770 8006BF70 4800528C */  lw         $s2, 0x48($v0)
    /* 5C774 8006BF74 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 5C778 8006BF78 47006212 */  beq        $s3, $v0, .L8006C098
    /* 5C77C 8006BF7C 21905002 */   addu      $s2, $s2, $s0
    /* 5C780 8006BF80 02006012 */  beqz       $s3, .L8006BF8C
    /* 5C784 8006BF84 28001424 */   addiu     $s4, $zero, 0x28
    /* 5C788 8006BF88 1F001424 */  addiu      $s4, $zero, 0x1F
  .L8006BF8C:
    /* 5C78C 8006BF8C 10001524 */  addiu      $s5, $zero, 0x10
    /* 5C790 8006BF90 12000424 */  addiu      $a0, $zero, 0x12
    /* 5C794 8006BF94 1000A627 */  addiu      $a2, $sp, 0x10
    /* 5C798 8006BF98 21386002 */  addu       $a3, $s3, $zero
    /* 5C79C 8006BF9C 02004586 */  lh         $a1, 0x2($s2)
    /* 5C7A0 8006BFA0 00000000 */  nop
    /* 5C7A4 8006BFA4 1000A524 */  addiu      $a1, $a1, 0x10
    /* 5C7A8 8006BFA8 43280500 */  sra        $a1, $a1, 1
    /* 5C7AC 8006BFAC 340885AF */  sw         $a1, %gp_rel(D_800A3900)($gp)
    /* 5C7B0 8006BFB0 4AAF010C */  jal        func_8006BD28
    /* 5C7B4 8006BFB4 21880000 */   addu      $s1, $zero, $zero
    /* 5C7B8 8006BFB8 01006332 */  andi       $v1, $s3, 0x1
    /* 5C7BC 8006BFBC C0100300 */  sll        $v0, $v1, 3
    /* 5C7C0 8006BFC0 23104300 */  subu       $v0, $v0, $v1
    /* 5C7C4 8006BFC4 80100200 */  sll        $v0, $v0, 2
    /* 5C7C8 8006BFC8 21104300 */  addu       $v0, $v0, $v1
    /* 5C7CC 8006BFCC 40100200 */  sll        $v0, $v0, 1
    /* 5C7D0 8006BFD0 13015324 */  addiu      $s3, $v0, 0x113
  .L8006BFD4:
    /* 5C7D4 8006BFD4 1006848F */  lw         $a0, %gp_rel(D_800A36DC)($gp)
    /* 5C7D8 8006BFD8 B4EA010C */  jal        func_8007AAD0
    /* 5C7DC 8006BFDC 00000000 */   nop
    /* 5C7E0 8006BFE0 00141100 */  sll        $v0, $s1, 16
    /* 5C7E4 8006BFE4 03140200 */  sra        $v0, $v0, 16
    /* 5C7E8 8006BFE8 05004014 */  bnez       $v0, .L8006C000
    /* 5C7EC 8006BFEC FFFF4224 */   addiu     $v0, $v0, -0x1
    /* 5C7F0 8006BFF0 FF001024 */  addiu      $s0, $zero, 0xFF
    /* 5C7F4 8006BFF4 1006848F */  lw         $a0, %gp_rel(D_800A36DC)($gp)
    /* 5C7F8 8006BFF8 05B00108 */  j          .L8006C014
    /* 5C7FC 8006BFFC 21280000 */   addu      $a1, $zero, $zero
  .L8006C000:
    /* 5C800 8006C000 C0110200 */  sll        $v0, $v0, 7
    /* 5C804 8006C004 FF000324 */  addiu      $v1, $zero, 0xFF
    /* 5C808 8006C008 23806200 */  subu       $s0, $v1, $v0
    /* 5C80C 8006C00C 1006848F */  lw         $a0, %gp_rel(D_800A36DC)($gp)
    /* 5C810 8006C010 01000524 */  addiu      $a1, $zero, 0x1
  .L8006C014:
    /* 5C814 8006C014 5AEA010C */  jal        func_8007A968
    /* 5C818 8006C018 00000000 */   nop
    /* 5C81C 8006C01C 1006828F */  lw         $v0, %gp_rel(D_800A36DC)($gp)
    /* 5C820 8006C020 00000000 */  nop
    /* 5C824 8006C024 040050A0 */  sb         $s0, 0x4($v0)
    /* 5C828 8006C028 1006828F */  lw         $v0, %gp_rel(D_800A36DC)($gp)
    /* 5C82C 8006C02C 00000000 */  nop
    /* 5C830 8006C030 050040A0 */  sb         $zero, 0x5($v0)
    /* 5C834 8006C034 1006828F */  lw         $v0, %gp_rel(D_800A36DC)($gp)
    /* 5C838 8006C038 84FF2326 */  addiu      $v1, $s1, -0x7C
    /* 5C83C 8006C03C 060040A0 */  sb         $zero, 0x6($v0)
    /* 5C840 8006C040 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5C844 8006C044 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5C848 8006C048 1006858F */  lw         $a1, %gp_rel(D_800A36DC)($gp)
    /* 5C84C 8006C04C 3408828F */  lw         $v0, %gp_rel(D_800A3900)($gp)
    /* 5C850 8006C050 20008424 */  addiu      $a0, $a0, 0x20
    /* 5C854 8006C054 23104300 */  subu       $v0, $v0, $v1
    /* 5C858 8006C058 0A00A2A4 */  sh         $v0, 0xA($a1)
    /* 5C85C 8006C05C 01000224 */  addiu      $v0, $zero, 0x1
    /* 5C860 8006C060 0800B3A4 */  sh         $s3, 0x8($a1)
    /* 5C864 8006C064 0C00B4A4 */  sh         $s4, 0xC($a1)
    /* 5C868 8006C068 2DEA010C */  jal        func_8007A8B4
    /* 5C86C 8006C06C 0E00A2A4 */   sh        $v0, 0xE($a1)
    /* 5C870 8006C070 01002226 */  addiu      $v0, $s1, 0x1
    /* 5C874 8006C074 21884000 */  addu       $s1, $v0, $zero
    /* 5C878 8006C078 00140200 */  sll        $v0, $v0, 16
    /* 5C87C 8006C07C 03140200 */  sra        $v0, $v0, 16
    /* 5C880 8006C080 1006838F */  lw         $v1, %gp_rel(D_800A36DC)($gp)
    /* 5C884 8006C084 03004228 */  slti       $v0, $v0, 0x3
    /* 5C888 8006C088 10006324 */  addiu      $v1, $v1, 0x10
    /* 5C88C 8006C08C 100683AF */  sw         $v1, %gp_rel(D_800A36DC)($gp)
    /* 5C890 8006C090 D0FF4014 */  bnez       $v0, .L8006BFD4
    /* 5C894 8006C094 00000000 */   nop
  .L8006C098:
    /* 5C898 8006C098 1006848F */  lw         $a0, %gp_rel(D_800A36DC)($gp)
    /* 5C89C 8006C09C B4EA010C */  jal        func_8007AAD0
    /* 5C8A0 8006C0A0 00000000 */   nop
    /* 5C8A4 8006C0A4 1006828F */  lw         $v0, %gp_rel(D_800A36DC)($gp)
    /* 5C8A8 8006C0A8 00000000 */  nop
    /* 5C8AC 8006C0AC 040040A0 */  sb         $zero, 0x4($v0)
    /* 5C8B0 8006C0B0 1006828F */  lw         $v0, %gp_rel(D_800A36DC)($gp)
    /* 5C8B4 8006C0B4 00000000 */  nop
    /* 5C8B8 8006C0B8 050040A0 */  sb         $zero, 0x5($v0)
    /* 5C8BC 8006C0BC 1006828F */  lw         $v0, %gp_rel(D_800A36DC)($gp)
    /* 5C8C0 8006C0C0 00000000 */  nop
    /* 5C8C4 8006C0C4 060040A0 */  sb         $zero, 0x6($v0)
    /* 5C8C8 8006C0C8 40010224 */  addiu      $v0, $zero, 0x140
    /* 5C8CC 8006C0CC 00004396 */  lhu        $v1, 0x0($s2)
    /* 5C8D0 8006C0D0 1006848F */  lw         $a0, %gp_rel(D_800A36DC)($gp)
    /* 5C8D4 8006C0D4 001C0300 */  sll        $v1, $v1, 16
    /* 5C8D8 8006C0D8 431C0300 */  sra        $v1, $v1, 17
    /* 5C8DC 8006C0DC 23104300 */  subu       $v0, $v0, $v1
    /* 5C8E0 8006C0E0 080082A4 */  sh         $v0, 0x8($a0)
    /* 5C8E4 8006C0E4 02004396 */  lhu        $v1, 0x2($s2)
    /* 5C8E8 8006C0E8 78000224 */  addiu      $v0, $zero, 0x78
    /* 5C8EC 8006C0EC 001C0300 */  sll        $v1, $v1, 16
    /* 5C8F0 8006C0F0 431C0300 */  sra        $v1, $v1, 17
    /* 5C8F4 8006C0F4 23104300 */  subu       $v0, $v0, $v1
    /* 5C8F8 8006C0F8 0A0082A4 */  sh         $v0, 0xA($a0)
    /* 5C8FC 8006C0FC 00004296 */  lhu        $v0, 0x0($s2)
    /* 5C900 8006C100 00000000 */  nop
    /* 5C904 8006C104 0C0082A4 */  sh         $v0, 0xC($a0)
    /* 5C908 8006C108 02004296 */  lhu        $v0, 0x2($s2)
    /* 5C90C 8006C10C 01000524 */  addiu      $a1, $zero, 0x1
    /* 5C910 8006C110 21105500 */  addu       $v0, $v0, $s5
    /* 5C914 8006C114 5AEA010C */  jal        func_8007A968
    /* 5C918 8006C118 0E0082A4 */   sh        $v0, 0xE($a0)
    /* 5C91C 8006C11C 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5C920 8006C120 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5C924 8006C124 1006858F */  lw         $a1, %gp_rel(D_800A36DC)($gp)
    /* 5C928 8006C128 2DEA010C */  jal        func_8007A8B4
    /* 5C92C 8006C12C 20008424 */   addiu     $a0, $a0, 0x20
    /* 5C930 8006C130 1006828F */  lw         $v0, %gp_rel(D_800A36DC)($gp)
    /* 5C934 8006C134 00000000 */  nop
    /* 5C938 8006C138 10004224 */  addiu      $v0, $v0, 0x10
    /* 5C93C 8006C13C 100682AF */  sw         $v0, %gp_rel(D_800A36DC)($gp)
    /* 5C940 8006C140 5800BF8F */  lw         $ra, 0x58($sp)
    /* 5C944 8006C144 5400B58F */  lw         $s5, 0x54($sp)
    /* 5C948 8006C148 5000B48F */  lw         $s4, 0x50($sp)
    /* 5C94C 8006C14C 4C00B38F */  lw         $s3, 0x4C($sp)
    /* 5C950 8006C150 4800B28F */  lw         $s2, 0x48($sp)
    /* 5C954 8006C154 4400B18F */  lw         $s1, 0x44($sp)
    /* 5C958 8006C158 4000B08F */  lw         $s0, 0x40($sp)
    /* 5C95C 8006C15C 6000BD27 */  addiu      $sp, $sp, 0x60
    /* 5C960 8006C160 0800E003 */  jr         $ra
    /* 5C964 8006C164 00000000 */   nop
endlabel func_8006BEC4
