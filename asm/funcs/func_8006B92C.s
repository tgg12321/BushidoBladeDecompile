glabel func_8006B92C
    /* 5C12C 8006B92C D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 5C130 8006B930 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 5C134 8006B934 2188A000 */  addu       $s1, $a1, $zero
    /* 5C138 8006B938 1800B0AF */  sw         $s0, 0x18($sp)
    /* 5C13C 8006B93C 21800000 */  addu       $s0, $zero, $zero
    /* 5C140 8006B940 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5C144 8006B944 21280000 */  addu       $a1, $zero, $zero
    /* 5C148 8006B948 3004868F */  lw         $a2, %gp_rel(D_800A34FC)($gp)
    /* 5C14C 8006B94C 0A80073C */  lui        $a3, %hi(D_800A350C)
    /* 5C150 8006B950 0C35E724 */  addiu      $a3, $a3, %lo(D_800A350C)
    /* 5C154 8006B954 2000BFAF */  sw         $ra, 0x20($sp)
    /* 5C158 8006B958 0000238E */  lw         $v1, 0x0($s1)
    /* 5C15C 8006B95C 0C00C624 */  addiu      $a2, $a2, 0xC
    /* 5C160 8006B960 FFFF6230 */  andi       $v0, $v1, 0xFFFF
    /* 5C164 8006B964 021C0300 */  srl        $v1, $v1, 16
    /* 5C168 8006B968 25104300 */  or         $v0, $v0, $v1
    /* 5C16C 8006B96C B0A4010C */  jal        func_800692C0
    /* 5C170 8006B970 1000A2AF */   sw        $v0, 0x10($sp)
    /* 5C174 8006B974 031C0200 */  sra        $v1, $v0, 16
    /* 5C178 8006B978 01000224 */  addiu      $v0, $zero, 0x1
    /* 5C17C 8006B97C 05006210 */  beq        $v1, $v0, .L8006B994
    /* 5C180 8006B980 02000224 */   addiu     $v0, $zero, 0x2
    /* 5C184 8006B984 14006210 */  beq        $v1, $v0, .L8006B9D8
    /* 5C188 8006B988 00000000 */   nop
    /* 5C18C 8006B98C 8FAE0108 */  j          .L8006BA3C
    /* 5C190 8006B990 00000000 */   nop
  .L8006B994:
    /* 5C194 8006B994 2C04848F */  lw         $a0, %gp_rel(D_800A34F8)($gp)
    /* 5C198 8006B998 00400224 */  addiu      $v0, $zero, 0x4000
    /* 5C19C 8006B99C 00E08330 */  andi       $v1, $a0, 0xE000
    /* 5C1A0 8006B9A0 07006214 */  bne        $v1, $v0, .L8006B9C0
    /* 5C1A4 8006B9A4 FFFF033C */   lui       $v1, (0xFFFF1FFF >> 16)
    /* 5C1A8 8006B9A8 FFFF023C */  lui        $v0, (0xFFFF1FFF >> 16)
    /* 5C1AC 8006B9AC FF1F4234 */  ori        $v0, $v0, (0xFFFF1FFF & 0xFFFF)
    /* 5C1B0 8006B9B0 24108200 */  and        $v0, $a0, $v0
    /* 5C1B4 8006B9B4 2C0482AF */  sw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5C1B8 8006B9B8 8CAE0108 */  j          .L8006BA30
    /* 5C1BC 8006B9BC 21200000 */   addu      $a0, $zero, $zero
  .L8006B9C0:
    /* 5C1C0 8006B9C0 FF1F6334 */  ori        $v1, $v1, (0xFFFF1FFF & 0xFFFF)
    /* 5C1C4 8006B9C4 24188300 */  and        $v1, $a0, $v1
    /* 5C1C8 8006B9C8 42130400 */  srl        $v0, $a0, 13
    /* 5C1CC 8006B9CC 07004230 */  andi       $v0, $v0, 0x7
    /* 5C1D0 8006B9D0 87AE0108 */  j          .L8006BA1C
    /* 5C1D4 8006B9D4 01004224 */   addiu     $v0, $v0, 0x1
  .L8006B9D8:
    /* 5C1D8 8006B9D8 2C04848F */  lw         $a0, %gp_rel(D_800A34F8)($gp)
    /* 5C1DC 8006B9DC 00000000 */  nop
    /* 5C1E0 8006B9E0 00E08230 */  andi       $v0, $a0, 0xE000
    /* 5C1E4 8006B9E4 08004014 */  bnez       $v0, .L8006BA08
    /* 5C1E8 8006B9E8 FFFF033C */   lui       $v1, (0xFFFF1FFF >> 16)
    /* 5C1EC 8006B9EC FFFF023C */  lui        $v0, (0xFFFF1FFF >> 16)
    /* 5C1F0 8006B9F0 FF1F4234 */  ori        $v0, $v0, (0xFFFF1FFF & 0xFFFF)
    /* 5C1F4 8006B9F4 24108200 */  and        $v0, $a0, $v0
    /* 5C1F8 8006B9F8 00404234 */  ori        $v0, $v0, 0x4000
    /* 5C1FC 8006B9FC 2C0482AF */  sw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5C200 8006BA00 8CAE0108 */  j          .L8006BA30
    /* 5C204 8006BA04 21200000 */   addu      $a0, $zero, $zero
  .L8006BA08:
    /* 5C208 8006BA08 FF1F6334 */  ori        $v1, $v1, (0xFFFF1FFF & 0xFFFF)
    /* 5C20C 8006BA0C 24188300 */  and        $v1, $a0, $v1
    /* 5C210 8006BA10 42130400 */  srl        $v0, $a0, 13
    /* 5C214 8006BA14 07004230 */  andi       $v0, $v0, 0x7
    /* 5C218 8006BA18 FFFF4224 */  addiu      $v0, $v0, -0x1
  .L8006BA1C:
    /* 5C21C 8006BA1C 07004230 */  andi       $v0, $v0, 0x7
    /* 5C220 8006BA20 40130200 */  sll        $v0, $v0, 13
    /* 5C224 8006BA24 25186200 */  or         $v1, $v1, $v0
    /* 5C228 8006BA28 2C0483AF */  sw         $v1, %gp_rel(D_800A34F8)($gp)
    /* 5C22C 8006BA2C 21200000 */  addu       $a0, $zero, $zero
  .L8006BA30:
    /* 5C230 8006BA30 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5C234 8006BA34 9471010C */  jal        func_8005C650
    /* 5C238 8006BA38 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8006BA3C:
    /* 5C23C 8006BA3C 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5C240 8006BA40 00000000 */  nop
    /* 5C244 8006BA44 42130200 */  srl        $v0, $v0, 13
    /* 5C248 8006BA48 07004330 */  andi       $v1, $v0, 0x7
    /* 5C24C 8006BA4C 01000224 */  addiu      $v0, $zero, 0x1
    /* 5C250 8006BA50 16006210 */  beq        $v1, $v0, .L8006BAAC
    /* 5C254 8006BA54 02006228 */   slti      $v0, $v1, 0x2
    /* 5C258 8006BA58 05004010 */  beqz       $v0, .L8006BA70
    /* 5C25C 8006BA5C 00000000 */   nop
    /* 5C260 8006BA60 08006010 */  beqz       $v1, .L8006BA84
    /* 5C264 8006BA64 4000033C */   lui       $v1, (0x400040 >> 16)
    /* 5C268 8006BA68 CAAE0108 */  j          .L8006BB28
    /* 5C26C 8006BA6C 1000033C */   lui       $v1, (0x100010 >> 16)
  .L8006BA70:
    /* 5C270 8006BA70 02000224 */  addiu      $v0, $zero, 0x2
    /* 5C274 8006BA74 18006210 */  beq        $v1, $v0, .L8006BAD8
    /* 5C278 8006BA78 4000033C */   lui       $v1, (0x400040 >> 16)
    /* 5C27C 8006BA7C CAAE0108 */  j          .L8006BB28
    /* 5C280 8006BA80 1000033C */   lui       $v1, (0x100010 >> 16)
  .L8006BA84:
    /* 5C284 8006BA84 0000228E */  lw         $v0, 0x0($s1)
    /* 5C288 8006BA88 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 5C28C 8006BA8C 24104300 */  and        $v0, $v0, $v1
    /* 5C290 8006BA90 24004010 */  beqz       $v0, .L8006BB24
    /* 5C294 8006BA94 01000424 */   addiu     $a0, $zero, 0x1
    /* 5C298 8006BA98 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5C29C 8006BA9C 9471010C */  jal        func_8005C650
    /* 5C2A0 8006BAA0 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5C2A4 8006BAA4 C9AE0108 */  j          .L8006BB24
    /* 5C2A8 8006BAA8 02001024 */   addiu     $s0, $zero, 0x2
  .L8006BAAC:
    /* 5C2AC 8006BAAC 4000033C */  lui        $v1, (0x400040 >> 16)
    /* 5C2B0 8006BAB0 0000228E */  lw         $v0, 0x0($s1)
    /* 5C2B4 8006BAB4 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 5C2B8 8006BAB8 24104300 */  and        $v0, $v0, $v1
    /* 5C2BC 8006BABC 19004010 */  beqz       $v0, .L8006BB24
    /* 5C2C0 8006BAC0 01000424 */   addiu     $a0, $zero, 0x1
    /* 5C2C4 8006BAC4 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5C2C8 8006BAC8 9471010C */  jal        func_8005C650
    /* 5C2CC 8006BACC 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5C2D0 8006BAD0 C9AE0108 */  j          .L8006BB24
    /* 5C2D4 8006BAD4 03001024 */   addiu     $s0, $zero, 0x3
  .L8006BAD8:
    /* 5C2D8 8006BAD8 0000228E */  lw         $v0, 0x0($s1)
    /* 5C2DC 8006BADC 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 5C2E0 8006BAE0 24104300 */  and        $v0, $v0, $v1
    /* 5C2E4 8006BAE4 0F004010 */  beqz       $v0, .L8006BB24
    /* 5C2E8 8006BAE8 01000424 */   addiu     $a0, $zero, 0x1
    /* 5C2EC 8006BAEC 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5C2F0 8006BAF0 9471010C */  jal        func_8005C650
    /* 5C2F4 8006BAF4 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5C2F8 8006BAF8 01001024 */  addiu      $s0, $zero, 0x1
    /* 5C2FC 8006BAFC 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5C300 8006BB00 FFE30324 */  addiu      $v1, $zero, -0x1C01
    /* 5C304 8006BB04 24184300 */  and        $v1, $v0, $v1
    /* 5C308 8006BB08 82120200 */  srl        $v0, $v0, 10
    /* 5C30C 8006BB0C 07004230 */  andi       $v0, $v0, 0x7
    /* 5C310 8006BB10 01004224 */  addiu      $v0, $v0, 0x1
    /* 5C314 8006BB14 07004230 */  andi       $v0, $v0, 0x7
    /* 5C318 8006BB18 80120200 */  sll        $v0, $v0, 10
    /* 5C31C 8006BB1C 25186200 */  or         $v1, $v1, $v0
    /* 5C320 8006BB20 2C0483AF */  sw         $v1, %gp_rel(D_800A34F8)($gp)
  .L8006BB24:
    /* 5C324 8006BB24 1000033C */  lui        $v1, (0x100010 >> 16)
  .L8006BB28:
    /* 5C328 8006BB28 0000228E */  lw         $v0, 0x0($s1)
    /* 5C32C 8006BB2C 10006334 */  ori        $v1, $v1, (0x100010 & 0xFFFF)
    /* 5C330 8006BB30 24104300 */  and        $v0, $v0, $v1
    /* 5C334 8006BB34 05004010 */  beqz       $v0, .L8006BB4C
    /* 5C338 8006BB38 02000424 */   addiu     $a0, $zero, 0x2
    /* 5C33C 8006BB3C 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5C340 8006BB40 9471010C */  jal        func_8005C650
    /* 5C344 8006BB44 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5C348 8006BB48 01001024 */  addiu      $s0, $zero, 0x1
  .L8006BB4C:
    /* 5C34C 8006BB4C 21100002 */  addu       $v0, $s0, $zero
    /* 5C350 8006BB50 2000BF8F */  lw         $ra, 0x20($sp)
    /* 5C354 8006BB54 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 5C358 8006BB58 1800B08F */  lw         $s0, 0x18($sp)
    /* 5C35C 8006BB5C 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 5C360 8006BB60 0800E003 */  jr         $ra
    /* 5C364 8006BB64 00000000 */   nop
endlabel func_8006B92C
