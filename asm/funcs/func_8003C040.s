glabel func_8003C040
    /* 2C840 8003C040 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 2C844 8003C044 2400BFAF */  sw         $ra, 0x24($sp)
    /* 2C848 8003C048 225A000C */  jal        gpu_InitDisplay
    /* 2C84C 8003C04C 2000B0AF */   sw        $s0, 0x20($sp)
    /* 2C850 8003C050 1A5A000C */  jal        gpu_EnableDisplay
    /* 2C854 8003C054 00000000 */   nop
    /* 2C858 8003C058 3783000C */  jal        func_80020CDC
    /* 2C85C 8003C05C 00000000 */   nop
    /* 2C860 8003C060 0A80023C */  lui        $v0, %hi(D_800A38A4)
    /* 2C864 8003C064 A4384290 */  lbu        $v0, %lo(D_800A38A4)($v0)
    /* 2C868 8003C068 00000000 */  nop
    /* 2C86C 8003C06C FCFF4224 */  addiu      $v0, $v0, -0x4
    /* 2C870 8003C070 0200422C */  sltiu      $v0, $v0, 0x2
    /* 2C874 8003C074 03004010 */  beqz       $v0, .L8003C084
    /* 2C878 8003C078 00000000 */   nop
    /* 2C87C 8003C07C 1D5B000C */  jal        file_ResetDmaFlag
    /* 2C880 8003C080 00000000 */   nop
  .L8003C084:
    /* 2C884 8003C084 0A80033C */  lui        $v1, %hi(D_800A38A4)
    /* 2C888 8003C088 A4386390 */  lbu        $v1, %lo(D_800A38A4)($v1)
    /* 2C88C 8003C08C 06000224 */  addiu      $v0, $zero, 0x6
    /* 2C890 8003C090 09006214 */  bne        $v1, $v0, .L8003C0B8
    /* 2C894 8003C094 07000224 */   addiu     $v0, $zero, 0x7
    /* 2C898 8003C098 08000224 */  addiu      $v0, $zero, 0x8
    /* 2C89C 8003C09C 1080013C */  lui        $at, %hi(D_8010277C)
    /* 2C8A0 8003C0A0 7C2722A0 */  sb         $v0, %lo(D_8010277C)($at)
    /* 2C8A4 8003C0A4 06000224 */  addiu      $v0, $zero, 0x6
    /* 2C8A8 8003C0A8 1080013C */  lui        $at, %hi(D_8010277E)
    /* 2C8AC 8003C0AC 7E2722A0 */  sb         $v0, %lo(D_8010277E)($at)
    /* 2C8B0 8003C0B0 44F00008 */  j          .L8003C110
    /* 2C8B4 8003C0B4 21200000 */   addu      $a0, $zero, $zero
  .L8003C0B8:
    /* 2C8B8 8003C0B8 09006214 */  bne        $v1, $v0, .L8003C0E0
    /* 2C8BC 8003C0BC 08000224 */   addiu     $v0, $zero, 0x8
    /* 2C8C0 8003C0C0 16000224 */  addiu      $v0, $zero, 0x16
    /* 2C8C4 8003C0C4 1080013C */  lui        $at, %hi(D_8010277C)
    /* 2C8C8 8003C0C8 7C2722A0 */  sb         $v0, %lo(D_8010277C)($at)
    /* 2C8CC 8003C0CC 07000224 */  addiu      $v0, $zero, 0x7
    /* 2C8D0 8003C0D0 1080013C */  lui        $at, %hi(D_8010277E)
    /* 2C8D4 8003C0D4 7E2722A0 */  sb         $v0, %lo(D_8010277E)($at)
    /* 2C8D8 8003C0D8 44F00008 */  j          .L8003C110
    /* 2C8DC 8003C0DC 21200000 */   addu      $a0, $zero, $zero
  .L8003C0E0:
    /* 2C8E0 8003C0E0 04006214 */  bne        $v1, $v0, .L8003C0F4
    /* 2C8E4 8003C0E4 09000224 */   addiu     $v0, $zero, 0x9
    /* 2C8E8 8003C0E8 21200000 */  addu       $a0, $zero, $zero
    /* 2C8EC 8003C0EC 40F00008 */  j          .L8003C100
    /* 2C8F0 8003C0F0 1E000224 */   addiu     $v0, $zero, 0x1E
  .L8003C0F4:
    /* 2C8F4 8003C0F4 0F006214 */  bne        $v1, $v0, .L8003C134
    /* 2C8F8 8003C0F8 21200000 */   addu      $a0, $zero, $zero
    /* 2C8FC 8003C0FC 20000224 */  addiu      $v0, $zero, 0x20
  .L8003C100:
    /* 2C900 8003C100 1080013C */  lui        $at, %hi(D_8010277C)
    /* 2C904 8003C104 7C2722A0 */  sb         $v0, %lo(D_8010277C)($at)
    /* 2C908 8003C108 1080013C */  lui        $at, %hi(D_8010277E)
    /* 2C90C 8003C10C 7E2720A0 */  sb         $zero, %lo(D_8010277E)($at)
  .L8003C110:
    /* 2C910 8003C110 D0EB000C */  jal        func_8003AF40
    /* 2C914 8003C114 00000000 */   nop
    /* 2C918 8003C118 43EC000C */  jal        func_8003B10C
    /* 2C91C 8003C11C 21200000 */   addu      $a0, $zero, $zero
    /* 2C920 8003C120 0A80033C */  lui        $v1, %hi(D_800A38A4)
    /* 2C924 8003C124 A4386390 */  lbu        $v1, %lo(D_800A38A4)($v1)
    /* 2C928 8003C128 09000224 */  addiu      $v0, $zero, 0x9
    /* 2C92C 8003C12C 05006210 */  beq        $v1, $v0, .L8003C144
    /* 2C930 8003C130 08000424 */   addiu     $a0, $zero, 0x8
  .L8003C134:
    /* 2C934 8003C134 0A80043C */  lui        $a0, %hi(D_800A38A4)
    /* 2C938 8003C138 A4388490 */  lbu        $a0, %lo(D_800A38A4)($a0)
    /* 2C93C 8003C13C 52F00008 */  j          .L8003C148
    /* 2C940 8003C140 1180053C */   lui       $a1, (0x80118800 >> 16)
  .L8003C144:
    /* 2C944 8003C144 1180053C */  lui        $a1, (0x80118800 >> 16)
  .L8003C148:
    /* 2C948 8003C148 F27E010C */  jal        func_8005FBC8
    /* 2C94C 8003C14C 0088A534 */   ori       $a1, $a1, (0x80118800 & 0xFFFF)
    /* 2C950 8003C150 0A80033C */  lui        $v1, %hi(D_800A38A4)
    /* 2C954 8003C154 A4386390 */  lbu        $v1, %lo(D_800A38A4)($v1)
    /* 2C958 8003C158 04000224 */  addiu      $v0, $zero, 0x4
    /* 2C95C 8003C15C 09006214 */  bne        $v1, $v0, .L8003C184
    /* 2C960 8003C160 05000224 */   addiu     $v0, $zero, 0x5
    /* 2C964 8003C164 1080023C */  lui        $v0, %hi(D_80101ED2)
    /* 2C968 8003C168 D21E4284 */  lh         $v0, %lo(D_80101ED2)($v0)
    /* 2C96C 8003C16C 0980013C */  lui        $at, %hi(D_8008D9EC)
    /* 2C970 8003C170 21082200 */  addu       $at, $at, $v0
    /* 2C974 8003C174 ECD92290 */  lbu        $v0, %lo(D_8008D9EC)($at)
    /* 2C978 8003C178 00000000 */  nop
    /* 2C97C 8003C17C 0B004014 */  bnez       $v0, .L8003C1AC
    /* 2C980 8003C180 05000224 */   addiu     $v0, $zero, 0x5
  .L8003C184:
    /* 2C984 8003C184 15006214 */  bne        $v1, $v0, .L8003C1DC
    /* 2C988 8003C188 00000000 */   nop
    /* 2C98C 8003C18C 1080023C */  lui        $v0, %hi(D_80101ED2)
    /* 2C990 8003C190 D21E4284 */  lh         $v0, %lo(D_80101ED2)($v0)
    /* 2C994 8003C194 0980013C */  lui        $at, %hi(D_8008D9EC)
    /* 2C998 8003C198 21082200 */  addu       $at, $at, $v0
    /* 2C99C 8003C19C ECD92290 */  lbu        $v0, %lo(D_8008D9EC)($at)
    /* 2C9A0 8003C1A0 00000000 */  nop
    /* 2C9A4 8003C1A4 0E004014 */  bnez       $v0, .L8003C1E0
    /* 2C9A8 8003C1A8 1180033C */   lui       $v1, (0x80118800 >> 16)
  .L8003C1AC:
    /* 2C9AC 8003C1AC 1080023C */  lui        $v0, %hi(D_8010277D)
    /* 2C9B0 8003C1B0 7D274290 */  lbu        $v0, %lo(D_8010277D)($v0)
    /* 2C9B4 8003C1B4 1080033C */  lui        $v1, %hi(D_8010277F)
    /* 2C9B8 8003C1B8 7F276390 */  lbu        $v1, %lo(D_8010277F)($v1)
    /* 2C9BC 8003C1BC 1080013C */  lui        $at, %hi(D_8010277C)
    /* 2C9C0 8003C1C0 7C2722A0 */  sb         $v0, %lo(D_8010277C)($at)
    /* 2C9C4 8003C1C4 1080013C */  lui        $at, %hi(D_8010277E)
    /* 2C9C8 8003C1C8 7E2723A0 */  sb         $v1, %lo(D_8010277E)($at)
    /* 2C9CC 8003C1CC D0EB000C */  jal        func_8003AF40
    /* 2C9D0 8003C1D0 21200000 */   addu      $a0, $zero, $zero
    /* 2C9D4 8003C1D4 43EC000C */  jal        func_8003B10C
    /* 2C9D8 8003C1D8 21200000 */   addu      $a0, $zero, $zero
  .L8003C1DC:
    /* 2C9DC 8003C1DC 1180033C */  lui        $v1, (0x80118800 >> 16)
  .L8003C1E0:
    /* 2C9E0 8003C1E0 00886334 */  ori        $v1, $v1, (0x80118800 & 0xFFFF)
    /* 2C9E4 8003C1E4 16000424 */  addiu      $a0, $zero, 0x16
    /* 2C9E8 8003C1E8 21300000 */  addu       $a2, $zero, $zero
    /* 2C9EC 8003C1EC 0A80023C */  lui        $v0, %hi(D_800A38A4)
    /* 2C9F0 8003C1F0 A4384290 */  lbu        $v0, %lo(D_800A38A4)($v0)
    /* 2C9F4 8003C1F4 1080073C */  lui        $a3, %hi(D_80101EDA)
    /* 2C9F8 8003C1F8 DA1EE784 */  lh         $a3, %lo(D_80101EDA)($a3)
    /* 2C9FC 8003C1FC 0980013C */  lui        $at, %hi(D_8009016C)
    /* 2CA00 8003C200 21082200 */  addu       $at, $at, $v0
    /* 2CA04 8003C204 6C012590 */  lbu        $a1, %lo(D_8009016C)($at)
    /* 2CA08 8003C208 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 2CA0C 8003C20C 1000A2AF */  sw         $v0, 0x10($sp)
    /* 2CA10 8003C210 1400A2AF */  sw         $v0, 0x14($sp)
    /* 2CA14 8003C214 1800A2AF */  sw         $v0, 0x18($sp)
    /* 2CA18 8003C218 2152010C */  jal        func_80054884
    /* 2CA1C 8003C21C 1C00A3AF */   sw        $v1, 0x1C($sp)
    /* 2CA20 8003C220 21200000 */  addu       $a0, $zero, $zero
    /* 2CA24 8003C224 A205010C */  jal        func_80041688
    /* 2CA28 8003C228 21280000 */   addu      $a1, $zero, $zero
    /* 2CA2C 8003C22C 01000424 */  addiu      $a0, $zero, 0x1
    /* 2CA30 8003C230 A205010C */  jal        func_80041688
    /* 2CA34 8003C234 21280000 */   addu      $a1, $zero, $zero
    /* 2CA38 8003C238 5E84010C */  jal        func_80061178
    /* 2CA3C 8003C23C 00000000 */   nop
    /* 2CA40 8003C240 0A80023C */  lui        $v0, %hi(D_800A38A4)
    /* 2CA44 8003C244 A4384290 */  lbu        $v0, %lo(D_800A38A4)($v0)
    /* 2CA48 8003C248 0980033C */  lui        $v1, %hi(D_8008EA70)
    /* 2CA4C 8003C24C 70EA6324 */  addiu      $v1, $v1, %lo(D_8008EA70)
    /* 2CA50 8003C250 40100200 */  sll        $v0, $v0, 1
    /* 2CA54 8003C254 21804300 */  addu       $s0, $v0, $v1
    /* 2CA58 8003C258 00000282 */  lb         $v0, 0x0($s0)
    /* 2CA5C 8003C25C 00000000 */  nop
    /* 2CA60 8003C260 0C004004 */  bltz       $v0, .L8003C294
    /* 2CA64 8003C264 13000224 */   addiu     $v0, $zero, 0x13
    /* 2CA68 8003C268 EAD7000C */  jal        func_80035FA8
    /* 2CA6C 8003C26C 00000000 */   nop
    /* 2CA70 8003C270 00000582 */  lb         $a1, 0x0($s0)
    /* 2CA74 8003C274 AADB000C */  jal        func_80036EA8
    /* 2CA78 8003C278 05000424 */   addiu     $a0, $zero, 0x5
    /* 2CA7C 8003C27C 01000592 */  lbu        $a1, 0x1($s0)
    /* 2CA80 8003C280 F5DB000C */  jal        func_80036FD4
    /* 2CA84 8003C284 21204000 */   addu      $a0, $v0, $zero
    /* 2CA88 8003C288 98DC000C */  jal        func_80037260
    /* 2CA8C 8003C28C 00000000 */   nop
    /* 2CA90 8003C290 13000224 */  addiu      $v0, $zero, 0x13
  .L8003C294:
    /* 2CA94 8003C294 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2CA98 8003C298 B83720AC */  sw         $zero, %lo(D_800A37B8)($at)
    /* 2CA9C 8003C29C 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2CAA0 8003C2A0 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2CAA4 8003C2A4 345A000C */  jal        gpu_DisableDisplay
    /* 2CAA8 8003C2A8 00000000 */   nop
    /* 2CAAC 8003C2AC 2400BF8F */  lw         $ra, 0x24($sp)
    /* 2CAB0 8003C2B0 2000B08F */  lw         $s0, 0x20($sp)
    /* 2CAB4 8003C2B4 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 2CAB8 8003C2B8 0800E003 */  jr         $ra
    /* 2CABC 8003C2BC 00000000 */   nop
endlabel func_8003C040
