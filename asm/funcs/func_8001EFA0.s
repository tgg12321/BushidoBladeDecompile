glabel func_8001EFA0
    /* F7A0 8001EFA0 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* F7A4 8001EFA4 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* F7A8 8001EFA8 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* F7AC 8001EFAC 2800BFAF */  sw         $ra, 0x28($sp)
    /* F7B0 8001EFB0 01004224 */  addiu      $v0, $v0, 0x1
    /* F7B4 8001EFB4 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* F7B8 8001EFB8 B83722AC */  sw         $v0, %lo(D_800A37B8)($at)
    /* F7BC 8001EFBC AC1C010C */  jal        func_800472B0
    /* F7C0 8001EFC0 00000000 */   nop
    /* F7C4 8001EFC4 099C033C */  lui        $v1, (0x9C09C09D >> 16)
    /* F7C8 8001EFC8 0A80063C */  lui        $a2, %hi(D_800A37B8)
    /* F7CC 8001EFCC B837C68C */  lw         $a2, %lo(D_800A37B8)($a2)
    /* F7D0 8001EFD0 9DC06334 */  ori        $v1, $v1, (0x9C09C09D & 0xFFFF)
    /* F7D4 8001EFD4 00330600 */  sll        $a2, $a2, 12
    /* F7D8 8001EFD8 1800C300 */  mult       $a2, $v1
    /* F7DC 8001EFDC 0A80013C */  lui        $at, %hi(D_800A3778)
    /* F7E0 8001EFE0 783722AC */  sw         $v0, %lo(D_800A3778)($at)
    /* F7E4 8001EFE4 0A80033C */  lui        $v1, %hi(D_800A3748)
    /* F7E8 8001EFE8 48376380 */  lb         $v1, %lo(D_800A3748)($v1)
    /* F7EC 8001EFEC 1080023C */  lui        $v0, %hi(D_80101EC8)
    /* F7F0 8001EFF0 C81E4224 */  addiu      $v0, $v0, %lo(D_80101EC8)
    /* F7F4 8001EFF4 00210300 */  sll        $a0, $v1, 4
    /* F7F8 8001EFF8 21208300 */  addu       $a0, $a0, $v1
    /* F7FC 8001EFFC 80200400 */  sll        $a0, $a0, 2
    /* F800 8001F000 21208300 */  addu       $a0, $a0, $v1
    /* F804 8001F004 80200400 */  sll        $a0, $a0, 2
    /* F808 8001F008 23208300 */  subu       $a0, $a0, $v1
    /* F80C 8001F00C 80200400 */  sll        $a0, $a0, 2
    /* F810 8001F010 21208200 */  addu       $a0, $a0, $v0
    /* F814 8001F014 10380000 */  mfhi       $a3
    /* F818 8001F018 2128E600 */  addu       $a1, $a3, $a2
    /* F81C 8001F01C 83290500 */  sra        $a1, $a1, 6
    /* F820 8001F020 C3370600 */  sra        $a2, $a2, 31
    /* F824 8001F024 3C6F000C */  jal        func_8001BCF0
    /* F828 8001F028 2328A600 */   subu      $a1, $a1, $a2
    /* F82C 8001F02C 0179000C */  jal        func_8001E404
    /* F830 8001F030 00000000 */   nop
    /* F834 8001F034 C8E4000C */  jal        func_80039320
    /* F838 8001F038 00000000 */   nop
    /* F83C 8001F03C 1B80000C */  jal        func_8002006C
    /* F840 8001F040 00000000 */   nop
    /* F844 8001F044 826F000C */  jal        func_8001BE08
    /* F848 8001F048 1000A427 */   addiu     $a0, $sp, 0x10
    /* F84C 8001F04C 21200000 */  addu       $a0, $zero, $zero
    /* F850 8001F050 C28F000C */  jal        func_80023F08
    /* F854 8001F054 1000A527 */   addiu     $a1, $sp, 0x10
    /* F858 8001F058 01000424 */  addiu      $a0, $zero, 0x1
    /* F85C 8001F05C C28F000C */  jal        func_80023F08
    /* F860 8001F060 1000A527 */   addiu     $a1, $sp, 0x10
    /* F864 8001F064 87B1000C */  jal        func_8002C61C
    /* F868 8001F068 00000000 */   nop
    /* F86C 8001F06C 5FC3000C */  jal        func_80030D7C
    /* F870 8001F070 00000000 */   nop
    /* F874 8001F074 7AC8000C */  jal        func_800321E8
    /* F878 8001F078 00000000 */   nop
    /* F87C 8001F07C E8E5000C */  jal        func_800397A0
    /* F880 8001F080 00000000 */   nop
    /* F884 8001F084 6A1B010C */  jal        func_80046DA8
    /* F888 8001F088 01000424 */   addiu     $a0, $zero, 0x1
    /* F88C 8001F08C 76CD000C */  jal        func_800335D8
    /* F890 8001F090 00000000 */   nop
    /* F894 8001F094 0A80033C */  lui        $v1, %hi(D_800A3748)
    /* F898 8001F098 48376380 */  lb         $v1, %lo(D_800A3748)($v1)
    /* F89C 8001F09C 00000000 */  nop
    /* F8A0 8001F0A0 00110300 */  sll        $v0, $v1, 4
    /* F8A4 8001F0A4 21104300 */  addu       $v0, $v0, $v1
    /* F8A8 8001F0A8 80100200 */  sll        $v0, $v0, 2
    /* F8AC 8001F0AC 21104300 */  addu       $v0, $v0, $v1
    /* F8B0 8001F0B0 80100200 */  sll        $v0, $v0, 2
    /* F8B4 8001F0B4 23104300 */  subu       $v0, $v0, $v1
    /* F8B8 8001F0B8 80100200 */  sll        $v0, $v0, 2
    /* F8BC 8001F0BC 1080013C */  lui        $at, %hi(D_80101F5E)
    /* F8C0 8001F0C0 21082200 */  addu       $at, $at, $v0
    /* F8C4 8001F0C4 5E1F2284 */  lh         $v0, %lo(D_80101F5E)($at)
    /* F8C8 8001F0C8 00000000 */  nop
    /* F8CC 8001F0CC 08004010 */  beqz       $v0, .L8001F0F0
    /* F8D0 8001F0D0 01000224 */   addiu     $v0, $zero, 0x1
    /* F8D4 8001F0D4 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* F8D8 8001F0D8 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* F8DC 8001F0DC 00000000 */  nop
    /* F8E0 8001F0E0 03006214 */  bne        $v1, $v0, .L8001F0F0
    /* F8E4 8001F0E4 69000224 */   addiu     $v0, $zero, 0x69
    /* F8E8 8001F0E8 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* F8EC 8001F0EC B83722AC */  sw         $v0, %lo(D_800A37B8)($at)
  .L8001F0F0:
    /* F8F0 8001F0F0 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* F8F4 8001F0F4 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* F8F8 8001F0F8 00000000 */  nop
    /* F8FC 8001F0FC 69004228 */  slti       $v0, $v0, 0x69
    /* F900 8001F100 07004010 */  beqz       $v0, .L8001F120
    /* F904 8001F104 4000033C */   lui       $v1, (0x400040 >> 16)
    /* F908 8001F108 1080023C */  lui        $v0, %hi(D_80102794)
    /* F90C 8001F10C 9427428C */  lw         $v0, %lo(D_80102794)($v0)
    /* F910 8001F110 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* F914 8001F114 24104300 */  and        $v0, $v0, $v1
    /* F918 8001F118 26004010 */  beqz       $v0, .L8001F1B4
    /* F91C 8001F11C 00000000 */   nop
  .L8001F120:
    /* F920 8001F120 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* F924 8001F124 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* F928 8001F128 04000224 */  addiu      $v0, $zero, 0x4
    /* F92C 8001F12C 1A006210 */  beq        $v1, $v0, .L8001F198
    /* F930 8001F130 05006228 */   slti      $v0, $v1, 0x5
    /* F934 8001F134 05004010 */  beqz       $v0, .L8001F14C
    /* F938 8001F138 01000224 */   addiu     $v0, $zero, 0x1
    /* F93C 8001F13C 08006210 */  beq        $v1, $v0, .L8001F160
    /* F940 8001F140 00000000 */   nop
    /* F944 8001F144 687C0008 */  j          .L8001F1A0
    /* F948 8001F148 00000000 */   nop
  .L8001F14C:
    /* F94C 8001F14C 06000224 */  addiu      $v0, $zero, 0x6
    /* F950 8001F150 16006210 */  beq        $v1, $v0, .L8001F1AC
    /* F954 8001F154 0C000224 */   addiu     $v0, $zero, 0xC
    /* F958 8001F158 687C0008 */  j          .L8001F1A0
    /* F95C 8001F15C 00000000 */   nop
  .L8001F160:
    /* F960 8001F160 0A80023C */  lui        $v0, %hi(D_800A3748)
    /* F964 8001F164 48374280 */  lb         $v0, %lo(D_800A3748)($v0)
    /* F968 8001F168 00000000 */  nop
    /* F96C 8001F16C 0F004014 */  bnez       $v0, .L8001F1AC
    /* F970 8001F170 0C000224 */   addiu     $v0, $zero, 0xC
    /* F974 8001F174 8B76000C */  jal        func_8001DA2C
    /* F978 8001F178 00000000 */   nop
    /* F97C 8001F17C 02000224 */  addiu      $v0, $zero, 0x2
    /* F980 8001F180 0A80013C */  lui        $at, %hi(D_800A3768)
    /* F984 8001F184 683722A0 */  sb         $v0, %lo(D_800A3768)($at)
    /* F988 8001F188 F0CE000C */  jal        func_80033BC0
    /* F98C 8001F18C 00000000 */   nop
    /* F990 8001F190 6D7C0008 */  j          .L8001F1B4
    /* F994 8001F194 00000000 */   nop
  .L8001F198:
    /* F998 8001F198 6B7C0008 */  j          .L8001F1AC
    /* F99C 8001F19C 0C000224 */   addiu     $v0, $zero, 0xC
  .L8001F1A0:
    /* F9A0 8001F1A0 8B76000C */  jal        func_8001DA2C
    /* F9A4 8001F1A4 00000000 */   nop
    /* F9A8 8001F1A8 02000224 */  addiu      $v0, $zero, 0x2
  .L8001F1AC:
    /* F9AC 8001F1AC 0A80013C */  lui        $at, %hi(D_800A3834)
    /* F9B0 8001F1B0 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
  .L8001F1B4:
    /* F9B4 8001F1B4 2800BF8F */  lw         $ra, 0x28($sp)
    /* F9B8 8001F1B8 3000BD27 */  addiu      $sp, $sp, 0x30
    /* F9BC 8001F1BC 0800E003 */  jr         $ra
    /* F9C0 8001F1C0 00000000 */   nop
endlabel func_8001EFA0
