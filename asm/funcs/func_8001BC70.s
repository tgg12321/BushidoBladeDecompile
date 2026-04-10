glabel func_8001BC70
    /* C470 8001BC70 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* C474 8001BC74 1000B0AF */  sw         $s0, 0x10($sp)
    /* C478 8001BC78 21808000 */  addu       $s0, $a0, $zero
    /* C47C 8001BC7C 1400B1AF */  sw         $s1, 0x14($sp)
    /* C480 8001BC80 2188A000 */  addu       $s1, $a1, $zero
    /* C484 8001BC84 1800BFAF */  sw         $ra, 0x18($sp)
    /* C488 8001BC88 79FC000C */  jal        game_SetControllerPorts
    /* C48C 8001BC8C 21200000 */   addu      $a0, $zero, $zero
    /* C490 8001BC90 0F80053C */  lui        $a1, %hi(D_800F6608)
    /* C494 8001BC94 0866A524 */  addiu      $a1, $a1, %lo(D_800F6608)
    /* C498 8001BC98 7401028E */  lw         $v0, 0x174($s0)
    /* C49C 8001BC9C 7801038E */  lw         $v1, 0x178($s0)
    /* C4A0 8001BCA0 7C01048E */  lw         $a0, 0x17C($s0)
    /* C4A4 8001BCA4 0000A2AC */  sw         $v0, 0x0($a1)
    /* C4A8 8001BCA8 0400A3AC */  sw         $v1, 0x4($a1)
    /* C4AC 8001BCAC 0800A4AC */  sw         $a0, 0x8($a1)
    /* C4B0 8001BCB0 20010224 */  addiu      $v0, $zero, 0x120
    /* C4B4 8001BCB4 0F80013C */  lui        $at, %hi(D_800F6618)
    /* C4B8 8001BCB8 186622A4 */  sh         $v0, %lo(D_800F6618)($at)
    /* C4BC 8001BCBC 62110224 */  addiu      $v0, $zero, 0x1162
    /* C4C0 8001BCC0 0F80013C */  lui        $at, %hi(D_800F661A)
    /* C4C4 8001BCC4 1A6631A4 */  sh         $s1, %lo(D_800F661A)($at)
    /* C4C8 8001BCC8 0F80013C */  lui        $at, %hi(D_800F661C)
    /* C4CC 8001BCCC 1C6620A4 */  sh         $zero, %lo(D_800F661C)($at)
    /* C4D0 8001BCD0 0F80013C */  lui        $at, %hi(D_800F6620)
    /* C4D4 8001BCD4 206622AC */  sw         $v0, %lo(D_800F6620)($at)
    /* C4D8 8001BCD8 1800BF8F */  lw         $ra, 0x18($sp)
    /* C4DC 8001BCDC 1400B18F */  lw         $s1, 0x14($sp)
    /* C4E0 8001BCE0 1000B08F */  lw         $s0, 0x10($sp)
    /* C4E4 8001BCE4 2000BD27 */  addiu      $sp, $sp, 0x20
    /* C4E8 8001BCE8 0800E003 */  jr         $ra
    /* C4EC 8001BCEC 00000000 */   nop
endlabel func_8001BC70
