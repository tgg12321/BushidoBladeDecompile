glabel func_8002C61C
    /* 1CE1C 8002C61C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 1CE20 8002C620 1400B1AF */  sw         $s1, 0x14($sp)
    /* 1CE24 8002C624 1080113C */  lui        $s1, %hi(D_80101EC8)
    /* 1CE28 8002C628 C81E3126 */  addiu      $s1, $s1, %lo(D_80101EC8)
    /* 1CE2C 8002C62C 1000B0AF */  sw         $s0, 0x10($sp)
    /* 1CE30 8002C630 4C043026 */  addiu      $s0, $s1, 0x44C
    /* 1CE34 8002C634 1080033C */  lui        $v1, %hi(D_80101F32)
    /* 1CE38 8002C638 321F6394 */  lhu        $v1, %lo(D_80101F32)($v1)
    /* 1CE3C 8002C63C 0F000224 */  addiu      $v0, $zero, 0xF
    /* 1CE40 8002C640 FFFF6430 */  andi       $a0, $v1, 0xFFFF
    /* 1CE44 8002C644 0B008210 */  beq        $a0, $v0, .L8002C674
    /* 1CE48 8002C648 1800BFAF */   sw        $ra, 0x18($sp)
    /* 1CE4C 8002C64C E4FF6224 */  addiu      $v0, $v1, -0x1C
    /* 1CE50 8002C650 0200422C */  sltiu      $v0, $v0, 0x2
    /* 1CE54 8002C654 07004014 */  bnez       $v0, .L8002C674
    /* 1CE58 8002C658 E2FF6224 */   addiu     $v0, $v1, -0x1E
    /* 1CE5C 8002C65C 0200422C */  sltiu      $v0, $v0, 0x2
    /* 1CE60 8002C660 04004014 */  bnez       $v0, .L8002C674
    /* 1CE64 8002C664 E0FF6224 */   addiu     $v0, $v1, -0x20
    /* 1CE68 8002C668 0200422C */  sltiu      $v0, $v0, 0x2
    /* 1CE6C 8002C66C 05004010 */  beqz       $v0, .L8002C684
    /* 1CE70 8002C670 11000224 */   addiu     $v0, $zero, 0x11
  .L8002C674:
    /* 1CE74 8002C674 699B000C */  jal        func_80026DA4
    /* 1CE78 8002C678 00000000 */   nop
    /* 1CE7C 8002C67C 03B20008 */  j          .L8002C80C
    /* 1CE80 8002C680 00000000 */   nop
  .L8002C684:
    /* 1CE84 8002C684 05008214 */  bne        $a0, $v0, .L8002C69C
    /* 1CE88 8002C688 00000000 */   nop
    /* 1CE8C 8002C68C 37B0000C */  jal        func_8002C0DC
    /* 1CE90 8002C690 00000000 */   nop
    /* 1CE94 8002C694 03B20008 */  j          .L8002C80C
    /* 1CE98 8002C698 00000000 */   nop
  .L8002C69C:
    /* 1CE9C 8002C69C CBA1000C */  jal        func_8002872C
    /* 1CEA0 8002C6A0 00000000 */   nop
    /* 1CEA4 8002C6A4 32A2000C */  jal        saTan3MainJump
    /* 1CEA8 8002C6A8 00000000 */   nop
    /* 1CEAC 8002C6AC 15A5000C */  jal        func_80029454
    /* 1CEB0 8002C6B0 00000000 */   nop
    /* 1CEB4 8002C6B4 0A80013C */  lui        $at, %hi(D_800A3824)
    /* 1CEB8 8002C6B8 243822A4 */  sh         $v0, %lo(D_800A3824)($at)
    /* 1CEBC 8002C6BC 00140200 */  sll        $v0, $v0, 16
    /* 1CEC0 8002C6C0 1F004004 */  bltz       $v0, .L8002C740
    /* 1CEC4 8002C6C4 00000000 */   nop
    /* 1CEC8 8002C6C8 8BB0000C */  jal        PutRobShadow
    /* 1CECC 8002C6CC 00000000 */   nop
    /* 1CED0 8002C6D0 0A80023C */  lui        $v0, %hi(D_800A3824)
    /* 1CED4 8002C6D4 24384284 */  lh         $v0, %lo(D_800A3824)($v0)
    /* 1CED8 8002C6D8 00000000 */  nop
    /* 1CEDC 8002C6DC 18004004 */  bltz       $v0, .L8002C740
    /* 1CEE0 8002C6E0 00000000 */   nop
    /* 1CEE4 8002C6E4 1080023C */  lui        $v0, %hi(D_80101F75)
    /* 1CEE8 8002C6E8 751F4290 */  lbu        $v0, %lo(D_80101F75)($v0)
    /* 1CEEC 8002C6EC 00000000 */  nop
    /* 1CEF0 8002C6F0 06004014 */  bnez       $v0, .L8002C70C
    /* 1CEF4 8002C6F4 21202002 */   addu      $a0, $s1, $zero
    /* 1CEF8 8002C6F8 1080023C */  lui        $v0, %hi(D_801023C1)
    /* 1CEFC 8002C6FC C1234290 */  lbu        $v0, %lo(D_801023C1)($v0)
    /* 1CF00 8002C700 00000000 */  nop
    /* 1CF04 8002C704 0E004010 */  beqz       $v0, .L8002C740
    /* 1CF08 8002C708 00000000 */   nop
  .L8002C70C:
    /* 1CF0C 8002C70C 801F053C */  lui        $a1, (0x1F8003F4 >> 16)
    /* 1CF10 8002C710 F4A0000C */  jal        saTan2KabutoWareMove
    /* 1CF14 8002C714 F403A534 */   ori       $a1, $a1, (0x1F8003F4 & 0xFFFF)
    /* 1CF18 8002C718 21200002 */  addu       $a0, $s0, $zero
    /* 1CF1C 8002C71C 801F053C */  lui        $a1, (0x1F8003F4 >> 16)
    /* 1CF20 8002C720 F4A0000C */  jal        saTan2KabutoWareMove
    /* 1CF24 8002C724 F403A534 */   ori       $a1, $a1, (0x1F8003F4 & 0xFFFF)
    /* 1CF28 8002C728 1080013C */  lui        $at, %hi(D_801023C1)
    /* 1CF2C 8002C72C C12320A0 */  sb         $zero, %lo(D_801023C1)($at)
    /* 1CF30 8002C730 1080013C */  lui        $at, %hi(D_80101F75)
    /* 1CF34 8002C734 751F20A0 */  sb         $zero, %lo(D_80101F75)($at)
    /* 1CF38 8002C738 D2B10008 */  j          .L8002C748
    /* 1CF3C 8002C73C 00000000 */   nop
  .L8002C740:
    /* 1CF40 8002C740 C2AA000C */  jal        calc_loc_mat_fw
    /* 1CF44 8002C744 21200000 */   addu      $a0, $zero, $zero
  .L8002C748:
    /* 1CF48 8002C748 3C00228E */  lw         $v0, 0x3C($s1)
    /* 1CF4C 8002C74C 00000000 */  nop
    /* 1CF50 8002C750 03004228 */  slti       $v0, $v0, 0x3
    /* 1CF54 8002C754 2D004014 */  bnez       $v0, .L8002C80C
    /* 1CF58 8002C758 00000000 */   nop
    /* 1CF5C 8002C75C 3C00028E */  lw         $v0, 0x3C($s0)
    /* 1CF60 8002C760 00000000 */  nop
    /* 1CF64 8002C764 03004228 */  slti       $v0, $v0, 0x3
    /* 1CF68 8002C768 28004014 */  bnez       $v0, .L8002C80C
    /* 1CF6C 8002C76C 00000000 */   nop
    /* 1CF70 8002C770 0A80023C */  lui        $v0, %hi(D_800A38A8)
    /* 1CF74 8002C774 A8384284 */  lh         $v0, %lo(D_800A38A8)($v0)
    /* 1CF78 8002C778 00000000 */  nop
    /* 1CF7C 8002C77C 23004010 */  beqz       $v0, .L8002C80C
    /* 1CF80 8002C780 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 1CF84 8002C784 86022386 */  lh         $v1, 0x286($s1)
    /* 1CF88 8002C788 00000000 */  nop
    /* 1CF8C 8002C78C 1F006214 */  bne        $v1, $v0, .L8002C80C
    /* 1CF90 8002C790 00000000 */   nop
    /* 1CF94 8002C794 86020286 */  lh         $v0, 0x286($s0)
    /* 1CF98 8002C798 00000000 */  nop
    /* 1CF9C 8002C79C 1B004314 */  bne        $v0, $v1, .L8002C80C
    /* 1CFA0 8002C7A0 1F000324 */   addiu     $v1, $zero, 0x1F
    /* 1CFA4 8002C7A4 0C002286 */  lh         $v0, 0xC($s1)
    /* 1CFA8 8002C7A8 00000000 */  nop
    /* 1CFAC 8002C7AC 17004310 */  beq        $v0, $v1, .L8002C80C
    /* 1CFB0 8002C7B0 00000000 */   nop
    /* 1CFB4 8002C7B4 0C000286 */  lh         $v0, 0xC($s0)
    /* 1CFB8 8002C7B8 00000000 */  nop
    /* 1CFBC 8002C7BC 13004310 */  beq        $v0, $v1, .L8002C80C
    /* 1CFC0 8002C7C0 00000000 */   nop
    /* 1CFC4 8002C7C4 F800238E */  lw         $v1, 0xF8($s1)
    /* 1CFC8 8002C7C8 F800028E */  lw         $v0, 0xF8($s0)
    /* 1CFCC 8002C7CC 00000000 */  nop
    /* 1CFD0 8002C7D0 23106200 */  subu       $v0, $v1, $v0
    /* 1CFD4 8002C7D4 02004104 */  bgez       $v0, .L8002C7E0
    /* 1CFD8 8002C7D8 00000000 */   nop
    /* 1CFDC 8002C7DC 23100200 */  negu       $v0, $v0
  .L8002C7E0:
    /* 1CFE0 8002C7E0 E8034228 */  slti       $v0, $v0, 0x3E8
    /* 1CFE4 8002C7E4 09004010 */  beqz       $v0, .L8002C80C
    /* 1CFE8 8002C7E8 0A000224 */   addiu     $v0, $zero, 0xA
    /* 1CFEC 8002C7EC 860222A6 */  sh         $v0, 0x286($s1)
    /* 1CFF0 8002C7F0 860202A6 */  sh         $v0, 0x286($s0)
    /* 1CFF4 8002C7F4 8C0200AE */  sw         $zero, 0x28C($s0)
    /* 1CFF8 8002C7F8 8C0220AE */  sw         $zero, 0x28C($s1)
    /* 1CFFC 8002C7FC 0A80013C */  lui        $at, %hi(D_800A3910)
    /* 1D000 8002C800 103920A4 */  sh         $zero, %lo(D_800A3910)($at)
    /* 1D004 8002C804 0A80013C */  lui        $at, %hi(D_800A389C)
    /* 1D008 8002C808 9C3820A4 */  sh         $zero, %lo(D_800A389C)($at)
  .L8002C80C:
    /* 1D00C 8002C80C 1080023C */  lui        $v0, %hi(D_80101F32)
    /* 1D010 8002C810 321F4294 */  lhu        $v0, %lo(D_80101F32)($v0)
    /* 1D014 8002C814 05000324 */  addiu      $v1, $zero, 0x5
    /* 1D018 8002C818 05004314 */  bne        $v0, $v1, .L8002C830
    /* 1D01C 8002C81C 01000224 */   addiu     $v0, $zero, 0x1
    /* 1D020 8002C820 0A80013C */  lui        $at, %hi(D_800A3748)
    /* 1D024 8002C824 483722A0 */  sb         $v0, %lo(D_800A3748)($at)
    /* 1D028 8002C828 14B20008 */  j          .L8002C850
    /* 1D02C 8002C82C 1C000224 */   addiu     $v0, $zero, 0x1C
  .L8002C830:
    /* 1D030 8002C830 1080023C */  lui        $v0, %hi(D_8010237E)
    /* 1D034 8002C834 7E234294 */  lhu        $v0, %lo(D_8010237E)($v0)
    /* 1D038 8002C838 00000000 */  nop
    /* 1D03C 8002C83C 07004314 */  bne        $v0, $v1, .L8002C85C
    /* 1D040 8002C840 21480000 */   addu      $t1, $zero, $zero
    /* 1D044 8002C844 1C000224 */  addiu      $v0, $zero, 0x1C
    /* 1D048 8002C848 0A80013C */  lui        $at, %hi(D_800A3748)
    /* 1D04C 8002C84C 483720A0 */  sb         $zero, %lo(D_800A3748)($at)
  .L8002C850:
    /* 1D050 8002C850 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 1D054 8002C854 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 1D058 8002C858 21480000 */  addu       $t1, $zero, $zero
  .L8002C85C:
    /* 1D05C 8002C85C 1080023C */  lui        $v0, %hi(D_801020D8)
    /* 1D060 8002C860 D8204224 */  addiu      $v0, $v0, %lo(D_801020D8)
    /* 1D064 8002C864 4C044824 */  addiu      $t0, $v0, 0x44C
    /* 1D068 8002C868 21384000 */  addu       $a3, $v0, $zero
    /* 1D06C 8002C86C 801F063C */  lui        $a2, (0x1F80002C >> 16)
  .L8002C870:
    /* 1D070 8002C870 0000C28C */  lw         $v0, (0x1F800000 & 0xFFFF)($a2)
    /* 1D074 8002C874 0400C38C */  lw         $v1, (0x1F800004 & 0xFFFF)($a2)
    /* 1D078 8002C878 0800C48C */  lw         $a0, (0x1F800008 & 0xFFFF)($a2)
    /* 1D07C 8002C87C 0000E2AC */  sw         $v0, 0x0($a3)
    /* 1D080 8002C880 0400E3AC */  sw         $v1, 0x4($a3)
    /* 1D084 8002C884 0800E4AC */  sw         $a0, 0x8($a3)
    /* 1D088 8002C888 2400C28C */  lw         $v0, (0x1F800024 & 0xFFFF)($a2)
    /* 1D08C 8002C88C 2800C38C */  lw         $v1, (0x1F800028 & 0xFFFF)($a2)
    /* 1D090 8002C890 2C00C48C */  lw         $a0, (0x1F80002C & 0xFFFF)($a2)
    /* 1D094 8002C894 000002AD */  sw         $v0, 0x0($t0)
    /* 1D098 8002C898 040003AD */  sw         $v1, 0x4($t0)
    /* 1D09C 8002C89C 080004AD */  sw         $a0, 0x8($t0)
    /* 1D0A0 8002C8A0 0C000825 */  addiu      $t0, $t0, 0xC
    /* 1D0A4 8002C8A4 0C00E724 */  addiu      $a3, $a3, 0xC
    /* 1D0A8 8002C8A8 01002925 */  addiu      $t1, $t1, 0x1
    /* 1D0AC 8002C8AC 03002229 */  slti       $v0, $t1, 0x3
    /* 1D0B0 8002C8B0 EFFF4014 */  bnez       $v0, .L8002C870
    /* 1D0B4 8002C8B4 0C00C624 */   addiu     $a2, $a2, %lo(D_1F80000C)
    /* 1D0B8 8002C8B8 21480000 */  addu       $t1, $zero, $zero
    /* 1D0BC 8002C8BC 1080023C */  lui        $v0, %hi(D_801020FC)
    /* 1D0C0 8002C8C0 FC204224 */  addiu      $v0, $v0, %lo(D_801020FC)
    /* 1D0C4 8002C8C4 4C044824 */  addiu      $t0, $v0, 0x44C
    /* 1D0C8 8002C8C8 21384000 */  addu       $a3, $v0, $zero
    /* 1D0CC 8002C8CC 801F063C */  lui        $a2, (0x1F800068 >> 16)
  .L8002C8D0:
    /* 1D0D0 8002C8D0 4800C28C */  lw         $v0, (0x1F800048 & 0xFFFF)($a2)
    /* 1D0D4 8002C8D4 4C00C38C */  lw         $v1, (0x1F80004C & 0xFFFF)($a2)
    /* 1D0D8 8002C8D8 5000C48C */  lw         $a0, (0x1F800050 & 0xFFFF)($a2)
    /* 1D0DC 8002C8DC 0000E2AC */  sw         $v0, 0x0($a3)
    /* 1D0E0 8002C8E0 0400E3AC */  sw         $v1, 0x4($a3)
    /* 1D0E4 8002C8E4 0800E4AC */  sw         $a0, 0x8($a3)
    /* 1D0E8 8002C8E8 6000C28C */  lw         $v0, (0x1F800060 & 0xFFFF)($a2)
    /* 1D0EC 8002C8EC 6400C38C */  lw         $v1, (0x1F800064 & 0xFFFF)($a2)
    /* 1D0F0 8002C8F0 6800C48C */  lw         $a0, (0x1F800068 & 0xFFFF)($a2)
    /* 1D0F4 8002C8F4 000002AD */  sw         $v0, 0x0($t0)
    /* 1D0F8 8002C8F8 040003AD */  sw         $v1, 0x4($t0)
    /* 1D0FC 8002C8FC 080004AD */  sw         $a0, 0x8($t0)
    /* 1D100 8002C900 0C000825 */  addiu      $t0, $t0, 0xC
    /* 1D104 8002C904 0C00E724 */  addiu      $a3, $a3, 0xC
    /* 1D108 8002C908 01002925 */  addiu      $t1, $t1, 0x1
    /* 1D10C 8002C90C 02002229 */  slti       $v0, $t1, 0x2
    /* 1D110 8002C910 EFFF4014 */  bnez       $v0, .L8002C8D0
    /* 1D114 8002C914 0C00C624 */   addiu     $a2, $a2, %lo(D_1F80000C)
    /* 1D118 8002C918 21480000 */  addu       $t1, $zero, $zero
    /* 1D11C 8002C91C 5555073C */  lui        $a3, (0x55555556 >> 16)
    /* 1D120 8002C920 5655E734 */  ori        $a3, $a3, (0x55555556 & 0xFFFF)
    /* 1D124 8002C924 801F053C */  lui        $a1, (0x1F8000EC >> 16)
    /* 1D128 8002C928 EC00A534 */  ori        $a1, $a1, (0x1F8000EC & 0xFFFF)
    /* 1D12C 8002C92C 21300000 */  addu       $a2, $zero, $zero
  .L8002C930:
    /* 1D130 8002C930 C8FFA28C */  lw         $v0, -0x38($a1)
    /* 1D134 8002C934 D4FFA38C */  lw         $v1, -0x2C($a1)
    /* 1D138 8002C938 E0FFA48C */  lw         $a0, -0x20($a1)
    /* 1D13C 8002C93C 21104300 */  addu       $v0, $v0, $v1
    /* 1D140 8002C940 21104400 */  addu       $v0, $v0, $a0
    /* 1D144 8002C944 18004700 */  mult       $v0, $a3
    /* 1D148 8002C948 C3170200 */  sra        $v0, $v0, 31
    /* 1D14C 8002C94C 10500000 */  mfhi       $t2
    /* 1D150 8002C950 23104201 */  subu       $v0, $t2, $v0
    /* 1D154 8002C954 1080013C */  lui        $at, %hi(D_80102054)
    /* 1D158 8002C958 21082600 */  addu       $at, $at, $a2
    /* 1D15C 8002C95C 542022AC */  sw         $v0, %lo(D_80102054)($at)
    /* 1D160 8002C960 CCFFA28C */  lw         $v0, -0x34($a1)
    /* 1D164 8002C964 D8FFA38C */  lw         $v1, -0x28($a1)
    /* 1D168 8002C968 E4FFA48C */  lw         $a0, -0x1C($a1)
    /* 1D16C 8002C96C 21104300 */  addu       $v0, $v0, $v1
    /* 1D170 8002C970 21104400 */  addu       $v0, $v0, $a0
    /* 1D174 8002C974 18004700 */  mult       $v0, $a3
    /* 1D178 8002C978 C3170200 */  sra        $v0, $v0, 31
    /* 1D17C 8002C97C 10500000 */  mfhi       $t2
    /* 1D180 8002C980 23104201 */  subu       $v0, $t2, $v0
    /* 1D184 8002C984 1080013C */  lui        $at, %hi(D_80102058)
    /* 1D188 8002C988 21082600 */  addu       $at, $at, $a2
    /* 1D18C 8002C98C 582022AC */  sw         $v0, %lo(D_80102058)($at)
    /* 1D190 8002C990 D0FFA28C */  lw         $v0, -0x30($a1)
    /* 1D194 8002C994 DCFFA38C */  lw         $v1, -0x24($a1)
    /* 1D198 8002C998 E8FFA48C */  lw         $a0, -0x18($a1)
    /* 1D19C 8002C99C 21104300 */  addu       $v0, $v0, $v1
    /* 1D1A0 8002C9A0 21104400 */  addu       $v0, $v0, $a0
    /* 1D1A4 8002C9A4 18004700 */  mult       $v0, $a3
    /* 1D1A8 8002C9A8 C3170200 */  sra        $v0, $v0, 31
    /* 1D1AC 8002C9AC 10500000 */  mfhi       $t2
    /* 1D1B0 8002C9B0 23104201 */  subu       $v0, $t2, $v0
    /* 1D1B4 8002C9B4 1080013C */  lui        $at, %hi(D_8010205C)
    /* 1D1B8 8002C9B8 21082600 */  addu       $at, $at, $a2
    /* 1D1BC 8002C9BC 5C2022AC */  sw         $v0, %lo(D_8010205C)($at)
    /* 1D1C0 8002C9C0 ECFFA28C */  lw         $v0, -0x14($a1)
    /* 1D1C4 8002C9C4 F8FFA38C */  lw         $v1, -0x8($a1)
    /* 1D1C8 8002C9C8 00000000 */  nop
    /* 1D1CC 8002C9CC 21104300 */  addu       $v0, $v0, $v1
    /* 1D1D0 8002C9D0 C21F0200 */  srl        $v1, $v0, 31
    /* 1D1D4 8002C9D4 21104300 */  addu       $v0, $v0, $v1
    /* 1D1D8 8002C9D8 43100200 */  sra        $v0, $v0, 1
    /* 1D1DC 8002C9DC 1080013C */  lui        $at, %hi(D_8010203C)
    /* 1D1E0 8002C9E0 21082600 */  addu       $at, $at, $a2
    /* 1D1E4 8002C9E4 3C2022AC */  sw         $v0, %lo(D_8010203C)($at)
    /* 1D1E8 8002C9E8 F0FFA28C */  lw         $v0, -0x10($a1)
    /* 1D1EC 8002C9EC FCFFA38C */  lw         $v1, -0x4($a1)
    /* 1D1F0 8002C9F0 01002925 */  addiu      $t1, $t1, 0x1
    /* 1D1F4 8002C9F4 21104300 */  addu       $v0, $v0, $v1
    /* 1D1F8 8002C9F8 C21F0200 */  srl        $v1, $v0, 31
    /* 1D1FC 8002C9FC 21104300 */  addu       $v0, $v0, $v1
    /* 1D200 8002CA00 43100200 */  sra        $v0, $v0, 1
    /* 1D204 8002CA04 1080013C */  lui        $at, %hi(D_80102040)
    /* 1D208 8002CA08 21082600 */  addu       $at, $at, $a2
    /* 1D20C 8002CA0C 402022AC */  sw         $v0, %lo(D_80102040)($at)
    /* 1D210 8002CA10 F4FFA28C */  lw         $v0, -0xC($a1)
    /* 1D214 8002CA14 0000A38C */  lw         $v1, 0x0($a1)
    /* 1D218 8002CA18 0801A524 */  addiu      $a1, $a1, 0x108
    /* 1D21C 8002CA1C 21104300 */  addu       $v0, $v0, $v1
    /* 1D220 8002CA20 C21F0200 */  srl        $v1, $v0, 31
    /* 1D224 8002CA24 21104300 */  addu       $v0, $v0, $v1
    /* 1D228 8002CA28 43100200 */  sra        $v0, $v0, 1
    /* 1D22C 8002CA2C 1080013C */  lui        $at, %hi(D_80102044)
    /* 1D230 8002CA30 21082600 */  addu       $at, $at, $a2
    /* 1D234 8002CA34 442022AC */  sw         $v0, %lo(D_80102044)($at)
    /* 1D238 8002CA38 02002229 */  slti       $v0, $t1, 0x2
    /* 1D23C 8002CA3C BCFF4014 */  bnez       $v0, .L8002C930
    /* 1D240 8002CA40 4C04C624 */   addiu     $a2, $a2, 0x44C
    /* 1D244 8002CA44 86023086 */  lh         $s0, 0x286($s1)
    /* 1D248 8002CA48 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 1D24C 8002CA4C 09000216 */  bne        $s0, $v0, .L8002CA74
    /* 1D250 8002CA50 00000000 */   nop
    /* 1D254 8002CA54 C9C6000C */  jal        func_80031B24
    /* 1D258 8002CA58 00000000 */   nop
    /* 1D25C 8002CA5C 86022286 */  lh         $v0, 0x286($s1)
    /* 1D260 8002CA60 00000000 */  nop
    /* 1D264 8002CA64 03005014 */  bne        $v0, $s0, .L8002CA74
    /* 1D268 8002CA68 00000000 */   nop
    /* 1D26C 8002CA6C C5C8000C */  jal        Pad_Prs
    /* 1D270 8002CA70 00000000 */   nop
  .L8002CA74:
    /* 1D274 8002CA74 1800BF8F */  lw         $ra, 0x18($sp)
    /* 1D278 8002CA78 1400B18F */  lw         $s1, 0x14($sp)
    /* 1D27C 8002CA7C 1000B08F */  lw         $s0, 0x10($sp)
    /* 1D280 8002CA80 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 1D284 8002CA84 0800E003 */  jr         $ra
    /* 1D288 8002CA88 00000000 */   nop
endlabel func_8002C61C
