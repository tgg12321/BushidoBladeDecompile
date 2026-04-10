glabel func_8001EEB4
    /* F6B4 8001EEB4 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* F6B8 8001EEB8 0A80043C */  lui        $a0, %hi(D_800A3748)
    /* F6BC 8001EEBC 48378480 */  lb         $a0, %lo(D_800A3748)($a0)
    /* F6C0 8001EEC0 1080033C */  lui        $v1, %hi(D_80101EC8)
    /* F6C4 8001EEC4 C81E6324 */  addiu      $v1, $v1, %lo(D_80101EC8)
    /* F6C8 8001EEC8 1800BFAF */  sw         $ra, 0x18($sp)
    /* F6CC 8001EECC 1400B1AF */  sw         $s1, 0x14($sp)
    /* F6D0 8001EED0 1000B0AF */  sw         $s0, 0x10($sp)
    /* F6D4 8001EED4 00110400 */  sll        $v0, $a0, 4
    /* F6D8 8001EED8 21104400 */  addu       $v0, $v0, $a0
    /* F6DC 8001EEDC 80100200 */  sll        $v0, $v0, 2
    /* F6E0 8001EEE0 21104400 */  addu       $v0, $v0, $a0
    /* F6E4 8001EEE4 80100200 */  sll        $v0, $v0, 2
    /* F6E8 8001EEE8 23104400 */  subu       $v0, $v0, $a0
    /* F6EC 8001EEEC 80100200 */  sll        $v0, $v0, 2
    /* F6F0 8001EEF0 21884300 */  addu       $s1, $v0, $v1
    /* F6F4 8001EEF4 6A002596 */  lhu        $a1, 0x6A($s1)
    /* F6F8 8001EEF8 0A000224 */  addiu      $v0, $zero, 0xA
    /* F6FC 8001EEFC FFFFA330 */  andi       $v1, $a1, 0xFFFF
    /* F700 8001EF00 1A006210 */  beq        $v1, $v0, .L8001EF6C
    /* F704 8001EF04 00000000 */   nop
    /* F708 8001EF08 72002286 */  lh         $v0, 0x72($s1)
    /* F70C 8001EF0C 00000000 */  nop
    /* F710 8001EF10 16004014 */  bnez       $v0, .L8001EF6C
    /* F714 8001EF14 E9FFA224 */   addiu     $v0, $a1, -0x17
    /* F718 8001EF18 0200422C */  sltiu      $v0, $v0, 0x2
    /* F71C 8001EF1C 13004014 */  bnez       $v0, .L8001EF6C
    /* F720 8001EF20 00000000 */   nop
    /* F724 8001EF24 96002286 */  lh         $v0, 0x96($s1)
    /* F728 8001EF28 00000000 */  nop
    /* F72C 8001EF2C 0F004014 */  bnez       $v0, .L8001EF6C
    /* F730 8001EF30 00000000 */   nop
    /* F734 8001EF34 3286000C */  jal        func_800218C8
    /* F738 8001EF38 01001024 */   addiu     $s0, $zero, 0x1
    /* F73C 8001EF3C 0A80043C */  lui        $a0, %hi(D_800A3748)
    /* F740 8001EF40 48378480 */  lb         $a0, %lo(D_800A3748)($a0)
    /* F744 8001EF44 0A002586 */  lh         $a1, 0xA($s1)
    /* F748 8001EF48 8F86000C */  jal        func_80021A3C
    /* F74C 8001EF4C 00000000 */   nop
    /* F750 8001EF50 21284000 */  addu       $a1, $v0, $zero
    /* F754 8001EF54 0A80043C */  lui        $a0, %hi(D_800A3748)
    /* F758 8001EF58 48378480 */  lb         $a0, %lo(D_800A3748)($a0)
    /* F75C 8001EF5C 01000624 */  addiu      $a2, $zero, 0x1
    /* F760 8001EF60 A686000C */  jal        func_80021A98
    /* F764 8001EF64 5E0030A6 */   sh        $s0, 0x5E($s1)
    /* F768 8001EF68 6C0230A6 */  sh         $s0, 0x26C($s1)
  .L8001EF6C:
    /* F76C 8001EF6C 5E84010C */  jal        game_Cleanup
    /* F770 8001EF70 00000000 */   nop
    /* F774 8001EF74 11000224 */  addiu      $v0, $zero, 0x11
    /* F778 8001EF78 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* F77C 8001EF7C B83720AC */  sw         $zero, %lo(D_800A37B8)($at)
    /* F780 8001EF80 0A80013C */  lui        $at, %hi(D_800A3834)
    /* F784 8001EF84 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* F788 8001EF88 1800BF8F */  lw         $ra, 0x18($sp)
    /* F78C 8001EF8C 1400B18F */  lw         $s1, 0x14($sp)
    /* F790 8001EF90 1000B08F */  lw         $s0, 0x10($sp)
    /* F794 8001EF94 2000BD27 */  addiu      $sp, $sp, 0x20
    /* F798 8001EF98 0800E003 */  jr         $ra
    /* F79C 8001EF9C 00000000 */   nop
endlabel func_8001EEB4
