glabel func_8006E10C
    /* 5E90C 8006E10C 5804828F */  lw         $v0, %gp_rel(D_800A3524)($gp)
    /* 5E910 8006E110 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 5E914 8006E114 3000BFAF */  sw         $ra, 0x30($sp)
    /* 5E918 8006E118 2C00B3AF */  sw         $s3, 0x2C($sp)
    /* 5E91C 8006E11C 2800B2AF */  sw         $s2, 0x28($sp)
    /* 5E920 8006E120 2400B1AF */  sw         $s1, 0x24($sp)
    /* 5E924 8006E124 2000B0AF */  sw         $s0, 0x20($sp)
    /* 5E928 8006E128 0A80063C */  lui        $a2, %hi(D_800A32D8)
    /* 5E92C 8006E12C D832C624 */  addiu      $a2, $a2, %lo(D_800A32D8)
    /* 5E930 8006E130 0300C388 */  lwl        $v1, 0x3($a2)
    /* 5E934 8006E134 0000C398 */  lwr        $v1, 0x0($a2)
    /* 5E938 8006E138 0700C488 */  lwl        $a0, 0x7($a2)
    /* 5E93C 8006E13C 0400C498 */  lwr        $a0, 0x4($a2)
    /* 5E940 8006E140 1B00A3AB */  swl        $v1, 0x1B($sp)
    /* 5E944 8006E144 1800A3BB */  swr        $v1, 0x18($sp)
    /* 5E948 8006E148 1F00A4AB */  swl        $a0, 0x1F($sp)
    /* 5E94C 8006E14C 1C00A4BB */  swr        $a0, 0x1C($sp)
    /* 5E950 8006E150 2000428C */  lw         $v0, 0x20($v0)
    /* 5E954 8006E154 3404938F */  lw         $s3, %gp_rel(D_800A3500)($gp)
    /* 5E958 8006E158 01004230 */  andi       $v0, $v0, 0x1
    /* 5E95C 8006E15C 04004010 */  beqz       $v0, .L8006E170
    /* 5E960 8006E160 00000000 */   nop
    /* 5E964 8006E164 02000424 */  addiu      $a0, $zero, 0x2
    /* 5E968 8006E168 5EB80108 */  j          .L8006E178
    /* 5E96C 8006E16C 60000524 */   addiu     $a1, $zero, 0x60
  .L8006E170:
    /* 5E970 8006E170 02000424 */  addiu      $a0, $zero, 0x2
    /* 5E974 8006E174 07000524 */  addiu      $a1, $zero, 0x7
  .L8006E178:
    /* 5E978 8006E178 AADB000C */  jal        func_80036EA8
    /* 5E97C 8006E17C F0001124 */   addiu     $s1, $zero, 0xF0
    /* 5E980 8006E180 21804000 */  addu       $s0, $v0, $zero
    /* 5E984 8006E184 3404858F */  lw         $a1, %gp_rel(D_800A3500)($gp)
    /* 5E988 8006E188 66DB000C */  jal        replay_camera_Init
    /* 5E98C 8006E18C 21200002 */   addu      $a0, $s0, $zero
    /* 5E990 8006E190 D0DB000C */  jal        func_80036F40
    /* 5E994 8006E194 00000000 */   nop
    /* 5E998 8006E198 CADB000C */  jal        func_80036F28
    /* 5E99C 8006E19C 21200002 */   addu      $a0, $s0, $zero
    /* 5E9A0 8006E1A0 A8EC010C */  jal        func_8007B2A0
    /* 5E9A4 8006E1A4 21200000 */   addu      $a0, $zero, $zero
    /* 5E9A8 8006E1A8 0F80103C */  lui        $s0, %hi(D_800F7438)
    /* 5E9AC 8006E1AC 38741026 */  addiu      $s0, $s0, %lo(D_800F7438)
    /* 5E9B0 8006E1B0 21200002 */  addu       $a0, $s0, $zero
    /* 5E9B4 8006E1B4 21280000 */  addu       $a1, $zero, $zero
    /* 5E9B8 8006E1B8 21300000 */  addu       $a2, $zero, $zero
    /* 5E9BC 8006E1BC 80020724 */  addiu      $a3, $zero, 0x280
    /* 5E9C0 8006E1C0 A5E9010C */  jal        func_8007A694
    /* 5E9C4 8006E1C4 1000B1AF */   sw        $s1, 0x10($sp)
    /* 5E9C8 8006E1C8 90400426 */  addiu      $a0, $s0, 0x4090
    /* 5E9CC 8006E1CC 21280000 */  addu       $a1, $zero, $zero
    /* 5E9D0 8006E1D0 F0000624 */  addiu      $a2, $zero, 0xF0
    /* 5E9D4 8006E1D4 80020724 */  addiu      $a3, $zero, 0x280
    /* 5E9D8 8006E1D8 A5E9010C */  jal        func_8007A694
    /* 5E9DC 8006E1DC 1000B1AF */   sw        $s1, 0x10($sp)
    /* 5E9E0 8006E1E0 5C000426 */  addiu      $a0, $s0, 0x5C
    /* 5E9E4 8006E1E4 21280000 */  addu       $a1, $zero, $zero
    /* 5E9E8 8006E1E8 F0000624 */  addiu      $a2, $zero, 0xF0
    /* 5E9EC 8006E1EC 80020724 */  addiu      $a3, $zero, 0x280
    /* 5E9F0 8006E1F0 D3E9010C */  jal        func_8007A74C
    /* 5E9F4 8006E1F4 1000B1AF */   sw        $s1, 0x10($sp)
    /* 5E9F8 8006E1F8 EC401226 */  addiu      $s2, $s0, 0x40EC
    /* 5E9FC 8006E1FC 21204002 */  addu       $a0, $s2, $zero
    /* 5EA00 8006E200 21280000 */  addu       $a1, $zero, $zero
    /* 5EA04 8006E204 21300000 */  addu       $a2, $zero, $zero
    /* 5EA08 8006E208 80020724 */  addiu      $a3, $zero, 0x280
    /* 5EA0C 8006E20C D3E9010C */  jal        func_8007A74C
    /* 5EA10 8006E210 1000B1AF */   sw        $s1, 0x10($sp)
    /* 5EA14 8006E214 0F80013C */  lui        $at, %hi(D_800F74A4)
    /* 5EA18 8006E218 A47420A0 */  sb         $zero, %lo(D_800F74A4)($at)
    /* 5EA1C 8006E21C 1080013C */  lui        $at, %hi(D_800FB534)
    /* 5EA20 8006E220 34B520A0 */  sb         $zero, %lo(D_800FB534)($at)
    /* 5EA24 8006E224 0F80013C */  lui        $at, %hi(D_800F74A5)
    /* 5EA28 8006E228 A57420A0 */  sb         $zero, %lo(D_800F74A5)($at)
    /* 5EA2C 8006E22C 1080013C */  lui        $at, %hi(D_800FB535)
    /* 5EA30 8006E230 35B520A0 */  sb         $zero, %lo(D_800FB535)($at)
    /* 5EA34 8006E234 CFEC010C */  jal        func_8007B33C
    /* 5EA38 8006E238 21200000 */   addu      $a0, $zero, $zero
    /* 5EA3C 8006E23C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5EA40 8006E240 21280000 */  addu       $a1, $zero, $zero
    /* 5EA44 8006E244 21300000 */  addu       $a2, $zero, $zero
    /* 5EA48 8006E248 34ED010C */  jal        func_8007B4D0
    /* 5EA4C 8006E24C 21380000 */   addu      $a3, $zero, $zero
    /* 5EA50 8006E250 CFEC010C */  jal        func_8007B33C
    /* 5EA54 8006E254 21200000 */   addu      $a0, $zero, $zero
    /* 5EA58 8006E258 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5EA5C 8006E25C 80ED010C */  jal        func_8007B600
    /* 5EA60 8006E260 14006526 */   addiu     $a1, $s3, 0x14
    /* 5EA64 8006E264 CFEC010C */  jal        func_8007B33C
    /* 5EA68 8006E268 21200000 */   addu      $a0, $zero, $zero
    /* 5EA6C 8006E26C 6CEE010C */  jal        func_8007B9B0
    /* 5EA70 8006E270 21200002 */   addu      $a0, $s0, $zero
    /* 5EA74 8006E274 02EF010C */  jal        func_8007BC08
    /* 5EA78 8006E278 21204002 */   addu      $a0, $s2, $zero
    /* 5EA7C 8006E27C A8EC010C */  jal        func_8007B2A0
    /* 5EA80 8006E280 01000424 */   addiu     $a0, $zero, 0x1
    /* 5EA84 8006E284 01000224 */  addiu      $v0, $zero, 0x1
    /* 5EA88 8006E288 3000BF8F */  lw         $ra, 0x30($sp)
    /* 5EA8C 8006E28C 2C00B38F */  lw         $s3, 0x2C($sp)
    /* 5EA90 8006E290 2800B28F */  lw         $s2, 0x28($sp)
    /* 5EA94 8006E294 2400B18F */  lw         $s1, 0x24($sp)
    /* 5EA98 8006E298 2000B08F */  lw         $s0, 0x20($sp)
    /* 5EA9C 8006E29C 3800BD27 */  addiu      $sp, $sp, 0x38
    /* 5EAA0 8006E2A0 0800E003 */  jr         $ra
    /* 5EAA4 8006E2A4 00000000 */   nop
endlabel func_8006E10C
