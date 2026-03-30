glabel spu_IrqHandler
    /* 79B84 80089384 21108000 */  addu       $v0, $a0, $zero
    /* 79B88 80089388 0300401C */  bgtz       $v0, .L80089398
    /* 79B8C 8008938C 0040033C */   lui       $v1, (0x40001010 >> 16)
    /* 79B90 80089390 F4240208 */  j          .L800893D0
    /* 79B94 80089394 21100000 */   addu      $v0, $zero, $zero
  .L80089398:
    /* 79B98 80089398 0A80043C */  lui        $a0, %hi(D_800A2D04)
    /* 79B9C 8008939C 042D848C */  lw         $a0, %lo(D_800A2D04)($a0)
    /* 79BA0 800893A0 10106334 */  ori        $v1, $v1, (0x40001010 & 0xFFFF)
    /* 79BA4 800893A4 0000A3AC */  sw         $v1, 0x0($a1)
    /* 79BA8 800893A8 0100033C */  lui        $v1, (0x10000 >> 16)
    /* 79BAC 800893AC 0A80013C */  lui        $at, %hi(D_800A2D40)
    /* 79BB0 800893B0 402D25AC */  sw         $a1, %lo(D_800A2D40)($at)
    /* 79BB4 800893B4 0A80013C */  lui        $at, %hi(D_800A2D3C)
    /* 79BB8 800893B8 3C2D20AC */  sw         $zero, %lo(D_800A2D3C)($at)
    /* 79BBC 800893BC 0A80013C */  lui        $at, %hi(D_800A2D38)
    /* 79BC0 800893C0 382D22AC */  sw         $v0, %lo(D_800A2D38)($at)
    /* 79BC4 800893C4 04188300 */  sllv       $v1, $v1, $a0
    /* 79BC8 800893C8 F0EF6324 */  addiu      $v1, $v1, -0x1010
    /* 79BCC 800893CC 0400A3AC */  sw         $v1, 0x4($a1)
  .L800893D0:
    /* 79BD0 800893D0 0800E003 */  jr         $ra
    /* 79BD4 800893D4 00000000 */   nop
endlabel spu_IrqHandler
