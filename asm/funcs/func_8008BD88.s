glabel func_8008BD88
    /* 7C588 8008BD88 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7C58C 8008BD8C DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7C590 8008BD90 00210400 */  sll        $a0, $a0, 4
    /* 7C594 8008BD94 21208200 */  addu       $a0, $a0, $v0
    /* 7C598 8008BD98 00008394 */  lhu        $v1, 0x0($a0)
    /* 7C59C 8008BD9C 02008494 */  lhu        $a0, 0x2($a0)
    /* 7C5A0 8008BDA0 FFFF6730 */  andi       $a3, $v1, 0xFFFF
    /* 7C5A4 8008BDA4 0040E22C */  sltiu      $v0, $a3, 0x4000
    /* 7C5A8 8008BDA8 04004014 */  bnez       $v0, .L8008BDBC
    /* 7C5AC 8008BDAC 00800234 */   ori       $v0, $zero, 0x8000
    /* 7C5B0 8008BDB0 2310E200 */  subu       $v0, $a3, $v0
    /* 7C5B4 8008BDB4 702F0208 */  j          .L8008BDC0
    /* 7C5B8 8008BDB8 0000A2A4 */   sh        $v0, 0x0($a1)
  .L8008BDBC:
    /* 7C5BC 8008BDBC 0000A3A4 */  sh         $v1, 0x0($a1)
  .L8008BDC0:
    /* 7C5C0 8008BDC0 FFFF8330 */  andi       $v1, $a0, 0xFFFF
    /* 7C5C4 8008BDC4 0040622C */  sltiu      $v0, $v1, 0x4000
    /* 7C5C8 8008BDC8 04004014 */  bnez       $v0, .L8008BDDC
    /* 7C5CC 8008BDCC 00800234 */   ori       $v0, $zero, 0x8000
    /* 7C5D0 8008BDD0 23106200 */  subu       $v0, $v1, $v0
    /* 7C5D4 8008BDD4 782F0208 */  j          .L8008BDE0
    /* 7C5D8 8008BDD8 0000C2A4 */   sh        $v0, 0x0($a2)
  .L8008BDDC:
    /* 7C5DC 8008BDDC 0000C4A4 */  sh         $a0, 0x0($a2)
  .L8008BDE0:
    /* 7C5E0 8008BDE0 0800E003 */  jr         $ra
    /* 7C5E4 8008BDE4 00000000 */   nop
endlabel func_8008BD88
