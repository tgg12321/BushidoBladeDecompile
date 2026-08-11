glabel _spu_2pitch
    /* 7C294 8008BA94 3B100324 */  addiu      $v1, $zero, 0x103B
    /* 7C298 8008BA98 18008300 */  mult       $a0, $v1
    /* 7C29C 8008BA9C F0FFBD27 */  addiu      $sp, $sp, -0x10
    /* 7C2A0 8008BAA0 00430400 */  sll        $t0, $a0, 12
    /* 7C2A4 8008BAA4 21300000 */  addu       $a2, $zero, $zero
    /* 7C2A8 8008BAA8 42390500 */  srl        $a3, $a1, 5
    /* 7C2AC 8008BAAC 1F00A530 */  andi       $a1, $a1, 0x1F
    /* 7C2B0 8008BAB0 12480000 */  mflo       $t1
    /* 7C2B4 8008BAB4 1000E010 */  beqz       $a3, .L8008BAF8
    /* 7C2B8 8008BAB8 0800A9AF */   sw        $t1, 0x8($sp)
    /* 7C2BC 8008BABC 18008300 */  mult       $a0, $v1
  .L8008BAC0:
    /* 7C2C0 8008BAC0 80110300 */  sll        $v0, $v1, 6
    /* 7C2C4 8008BAC4 21104300 */  addu       $v0, $v0, $v1
    /* 7C2C8 8008BAC8 00110200 */  sll        $v0, $v0, 4
    /* 7C2CC 8008BACC 23104300 */  subu       $v0, $v0, $v1
    /* 7C2D0 8008BAD0 80100200 */  sll        $v0, $v0, 2
    /* 7C2D4 8008BAD4 12400000 */  mflo       $t0
    /* 7C2D8 8008BAD8 23184300 */  subu       $v1, $v0, $v1
    /* 7C2DC 8008BADC 021B0300 */  srl        $v1, $v1, 12
    /* 7C2E0 8008BAE0 18008300 */  mult       $a0, $v1
    /* 7C2E4 8008BAE4 0100C624 */  addiu      $a2, $a2, 0x1
    /* 7C2E8 8008BAE8 2A10C700 */  slt        $v0, $a2, $a3
    /* 7C2EC 8008BAEC 12480000 */  mflo       $t1
    /* 7C2F0 8008BAF0 F3FF4014 */  bnez       $v0, .L8008BAC0
    /* 7C2F4 8008BAF4 0800A9AF */   sw        $t1, 0x8($sp)
  .L8008BAF8:
    /* 7C2F8 8008BAF8 0800A98F */  lw         $t1, 0x8($sp)
    /* 7C2FC 8008BAFC 00000000 */  nop
    /* 7C300 8008BB00 23102801 */  subu       $v0, $t1, $t0
    /* 7C304 8008BB04 42110200 */  srl        $v0, $v0, 5
    /* 7C308 8008BB08 18004500 */  mult       $v0, $a1
    /* 7C30C 8008BB0C 12480000 */  mflo       $t1
    /* 7C310 8008BB10 21100901 */  addu       $v0, $t0, $t1
    /* 7C314 8008BB14 02130200 */  srl        $v0, $v0, 12
    /* 7C318 8008BB18 1000BD27 */  addiu      $sp, $sp, 0x10
    /* 7C31C 8008BB1C 0800E003 */  jr         $ra
    /* 7C320 8008BB20 00000000 */   nop
endlabel _spu_2pitch
