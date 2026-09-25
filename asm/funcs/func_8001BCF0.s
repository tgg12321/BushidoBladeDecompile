glabel func_8001BCF0
    /* C4F0 8001BCF0 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* C4F4 8001BCF4 1800B2AF */  sw         $s2, 0x18($sp)
    /* C4F8 8001BCF8 21908000 */  addu       $s2, $a0, $zero
    /* C4FC 8001BCFC 1000B0AF */  sw         $s0, 0x10($sp)
    /* C500 8001BD00 2180A000 */  addu       $s0, $a1, $zero
    /* C504 8001BD04 00100224 */  addiu      $v0, $zero, 0x1000
    /* C508 8001BD08 1400B1AF */  sw         $s1, 0x14($sp)
    /* C50C 8001BD0C 23885000 */  subu       $s1, $v0, $s0
    /* C510 8001BD10 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* C514 8001BD14 79FC000C */  jal        func_8003F1E4
    /* C518 8001BD18 21200000 */   addu      $a0, $zero, $zero
    /* C51C 8001BD1C 0F80053C */  lui        $a1, %hi(D_800F6608)
    /* C520 8001BD20 0866A524 */  addiu      $a1, $a1, %lo(D_800F6608)
    /* C524 8001BD24 B800428E */  lw         $v0, 0xB8($s2)
    /* C528 8001BD28 BC00438E */  lw         $v1, 0xBC($s2)
    /* C52C 8001BD2C C000448E */  lw         $a0, 0xC0($s2)
    /* C530 8001BD30 0000A2AC */  sw         $v0, 0x0($a1)
    /* C534 8001BD34 0400A3AC */  sw         $v1, 0x4($a1)
    /* C538 8001BD38 0800A4AC */  sw         $a0, 0x8($a1)
    /* C53C 8001BD3C C0101000 */  sll        $v0, $s0, 3
    /* C540 8001BD40 21105000 */  addu       $v0, $v0, $s0
    /* C544 8001BD44 0F80033C */  lui        $v1, %hi(D_800F660C)
    /* C548 8001BD48 0C66638C */  lw         $v1, %lo(D_800F660C)($v1)
    /* C54C 8001BD4C 40110200 */  sll        $v0, $v0, 5
    /* C550 8001BD50 B4FB6324 */  addiu      $v1, $v1, -0x44C
    /* C554 8001BD54 0F80013C */  lui        $at, %hi(D_800F660C)
    /* C558 8001BD58 0C6623AC */  sw         $v1, %lo(D_800F660C)($at)
    /* C55C 8001BD5C 03004104 */  bgez       $v0, .L8001BD6C
    /* C560 8001BD60 031B0200 */   sra       $v1, $v0, 12
    /* C564 8001BD64 FF0F4224 */  addiu      $v0, $v0, 0xFFF
    /* C568 8001BD68 031B0200 */  sra        $v1, $v0, 12
  .L8001BD6C:
    /* C56C 8001BD6C 00010224 */  addiu      $v0, $zero, 0x100
    /* C570 8001BD70 23104300 */  subu       $v0, $v0, $v1
    /* C574 8001BD74 0F80013C */  lui        $at, %hi(D_800F6618)
    /* C578 8001BD78 186622A4 */  sh         $v0, %lo(D_800F6618)($at)
    /* C57C 8001BD7C 02000106 */  bgez       $s0, .L8001BD88
    /* C580 8001BD80 21280002 */   addu      $a1, $s0, $zero
    /* C584 8001BD84 03000526 */  addiu      $a1, $s0, 0x3
  .L8001BD88:
    /* C588 8001BD88 40181000 */  sll        $v1, $s0, 1
    /* C58C 8001BD8C 21187000 */  addu       $v1, $v1, $s0
    /* C590 8001BD90 00190300 */  sll        $v1, $v1, 4
    /* C594 8001BD94 23187000 */  subu       $v1, $v1, $s0
    /* C598 8001BD98 C0180300 */  sll        $v1, $v1, 3
    /* C59C 8001BD9C 23187000 */  subu       $v1, $v1, $s0
    /* C5A0 8001BDA0 C0180300 */  sll        $v1, $v1, 3
    /* C5A4 8001BDA4 40111100 */  sll        $v0, $s1, 5
    /* C5A8 8001BDA8 23105100 */  subu       $v0, $v0, $s1
    /* C5AC 8001BDAC 80100200 */  sll        $v0, $v0, 2
    /* C5B0 8001BDB0 21105100 */  addu       $v0, $v0, $s1
    /* C5B4 8001BDB4 80110200 */  sll        $v0, $v0, 6
    /* C5B8 8001BDB8 21186200 */  addu       $v1, $v1, $v0
    /* C5BC 8001BDBC CA014496 */  lhu        $a0, 0x1CA($s2)
    /* C5C0 8001BDC0 031B0300 */  sra        $v1, $v1, 12
    /* C5C4 8001BDC4 0F80013C */  lui        $at, %hi(D_800F6620)
    /* C5C8 8001BDC8 206623AC */  sw         $v1, %lo(D_800F6620)($at)
    /* C5CC 8001BDCC 83180500 */  sra        $v1, $a1, 2
    /* C5D0 8001BDD0 000B0224 */  addiu      $v0, $zero, 0xB00
    /* C5D4 8001BDD4 23104300 */  subu       $v0, $v0, $v1
    /* C5D8 8001BDD8 0F80013C */  lui        $at, %hi(D_800F661C)
    /* C5DC 8001BDDC 1C6620A4 */  sh         $zero, %lo(D_800F661C)($at)
    /* C5E0 8001BDE0 23104400 */  subu       $v0, $v0, $a0
    /* C5E4 8001BDE4 0F80013C */  lui        $at, %hi(D_800F661A)
    /* C5E8 8001BDE8 1A6622A4 */  sh         $v0, %lo(D_800F661A)($at)
    /* C5EC 8001BDEC 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* C5F0 8001BDF0 1800B28F */  lw         $s2, 0x18($sp)
    /* C5F4 8001BDF4 1400B18F */  lw         $s1, 0x14($sp)
    /* C5F8 8001BDF8 1000B08F */  lw         $s0, 0x10($sp)
    /* C5FC 8001BDFC 2000BD27 */  addiu      $sp, $sp, 0x20
    /* C600 8001BE00 0800E003 */  jr         $ra
    /* C604 8001BE04 00000000 */   nop
endlabel func_8001BCF0
