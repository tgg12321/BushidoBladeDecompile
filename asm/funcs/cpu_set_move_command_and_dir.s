glabel cpu_set_move_command_and_dir
    /* 2122C 80030A2C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 21230 80030A30 1800B2AF */  sw         $s2, 0x18($sp)
    /* 21234 80030A34 21908000 */  addu       $s2, $a0, $zero
    /* 21238 80030A38 1000B0AF */  sw         $s0, 0x10($sp)
    /* 2123C 80030A3C 2180C000 */  addu       $s0, $a2, $zero
    /* 21240 80030A40 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 21244 80030A44 60C1000C */  jal        coli_hit_body_weapon
    /* 21248 80030A48 1400B1AF */   sw        $s1, 0x14($sp)
    /* 2124C 80030A4C 21884000 */  addu       $s1, $v0, $zero
    /* 21250 80030A50 040020A2 */  sb         $zero, 0x4($s1)
    /* 21254 80030A54 0000028E */  lw         $v0, 0x0($s0)
    /* 21258 80030A58 0400038E */  lw         $v1, 0x4($s0)
    /* 2125C 80030A5C 0800048E */  lw         $a0, 0x8($s0)
    /* 21260 80030A60 2C0022AE */  sw         $v0, 0x2C($s1)
    /* 21264 80030A64 300023AE */  sw         $v1, 0x30($s1)
    /* 21268 80030A68 340024AE */  sw         $a0, 0x34($s1)
    /* 2126C 80030A6C 6E5C000C */  jal        rng_Next
    /* 21270 80030A70 00000000 */   nop
    /* 21274 80030A74 FF004230 */  andi       $v0, $v0, 0xFF
    /* 21278 80030A78 80FF4224 */  addiu      $v0, $v0, -0x80
    /* 2127C 80030A7C 6E5C000C */  jal        rng_Next
    /* 21280 80030A80 440022AE */   sw        $v0, 0x44($s1)
    /* 21284 80030A84 3F004230 */  andi       $v0, $v0, 0x3F
    /* 21288 80030A88 23100200 */  negu       $v0, $v0
    /* 2128C 80030A8C 80FF4224 */  addiu      $v0, $v0, -0x80
    /* 21290 80030A90 6E5C000C */  jal        rng_Next
    /* 21294 80030A94 480022AE */   sw        $v0, 0x48($s1)
    /* 21298 80030A98 FF004230 */  andi       $v0, $v0, 0xFF
    /* 2129C 80030A9C 80FF4224 */  addiu      $v0, $v0, -0x80
    /* 212A0 80030AA0 6E5C000C */  jal        rng_Next
    /* 212A4 80030AA4 4C0022AE */   sw        $v0, 0x4C($s1)
    /* 212A8 80030AA8 21184000 */  addu       $v1, $v0, $zero
    /* 212AC 80030AAC 00106230 */  andi       $v0, $v1, 0x1000
    /* 212B0 80030AB0 03004010 */  beqz       $v0, .L80030AC0
    /* 212B4 80030AB4 FF036230 */   andi      $v0, $v1, 0x3FF
    /* 212B8 80030AB8 B2C20008 */  j          .L80030AC8
    /* 212BC 80030ABC 00024224 */   addiu     $v0, $v0, 0x200
  .L80030AC0:
    /* 212C0 80030AC0 23100200 */  negu       $v0, $v0
    /* 212C4 80030AC4 00FE4224 */  addiu      $v0, $v0, -0x200
  .L80030AC8:
    /* 212C8 80030AC8 6E5C000C */  jal        rng_Next
    /* 212CC 80030ACC 5C0022A6 */   sh        $v0, 0x5C($s1)
    /* 212D0 80030AD0 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 212D4 80030AD4 00FC4224 */  addiu      $v0, $v0, -0x400
    /* 212D8 80030AD8 5E0022A6 */  sh         $v0, 0x5E($s1)
    /* 212DC 80030ADC 01000224 */  addiu      $v0, $zero, 0x1
    /* 212E0 80030AE0 600020A6 */  sh         $zero, 0x60($s1)
    /* 212E4 80030AE4 070022A2 */  sb         $v0, 0x7($s1)
    /* 212E8 80030AE8 12004296 */  lhu        $v0, 0x12($s2)
    /* 212EC 80030AEC 00000000 */  nop
    /* 212F0 80030AF0 0B0022A2 */  sb         $v0, 0xB($s1)
    /* 212F4 80030AF4 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 212F8 80030AF8 1800B28F */  lw         $s2, 0x18($sp)
    /* 212FC 80030AFC 1400B18F */  lw         $s1, 0x14($sp)
    /* 21300 80030B00 1000B08F */  lw         $s0, 0x10($sp)
    /* 21304 80030B04 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 21308 80030B08 0800E003 */  jr         $ra
    /* 2130C 80030B0C 00000000 */   nop
endlabel cpu_set_move_command_and_dir
