glabel func_8001B690
    /* BE90 8001B690 0A80023C */  lui        $v0, %hi(D_800A38BA)
    /* BE94 8001B694 BA384284 */  lh         $v0, %lo(D_800A38BA)($v0)
    /* BE98 8001B698 00000000 */  nop
    /* BE9C 8001B69C 13004010 */  beqz       $v0, .L8001B6EC
    /* BEA0 8001B6A0 00000000 */   nop
    /* BEA4 8001B6A4 0A80023C */  lui        $v0, %hi(D_800A36F6)
    /* BEA8 8001B6A8 F6364284 */  lh         $v0, %lo(D_800A36F6)($v0)
    /* BEAC 8001B6AC 00000000 */  nop
    /* BEB0 8001B6B0 0E004414 */  bne        $v0, $a0, .L8001B6EC
    /* BEB4 8001B6B4 FF0FA530 */   andi      $a1, $a1, 0xFFF
    /* BEB8 8001B6B8 0008A228 */  slti       $v0, $a1, 0x800
    /* BEBC 8001B6BC 04004014 */  bnez       $v0, .L8001B6D0
    /* BEC0 8001B6C0 0104A228 */   slti      $v0, $a1, 0x401
    /* BEC4 8001B6C4 00100224 */  addiu      $v0, $zero, 0x1000
    /* BEC8 8001B6C8 23284500 */  subu       $a1, $v0, $a1
    /* BECC 8001B6CC 0104A228 */  slti       $v0, $a1, 0x401
  .L8001B6D0:
    /* BED0 8001B6D0 06004014 */  bnez       $v0, .L8001B6EC
    /* BED4 8001B6D4 19000224 */   addiu     $v0, $zero, 0x19
    /* BED8 8001B6D8 0A80013C */  lui        $at, %hi(D_800A36FC)
    /* BEDC 8001B6DC FC3622A4 */  sh         $v0, %lo(D_800A36FC)($at)
    /* BEE0 8001B6E0 00080224 */  addiu      $v0, $zero, 0x800
    /* BEE4 8001B6E4 0F80013C */  lui        $at, %hi(D_800F5344)
    /* BEE8 8001B6E8 445322A4 */  sh         $v0, %lo(D_800F5344)($at)
  .L8001B6EC:
    /* BEEC 8001B6EC 0800E003 */  jr         $ra
    /* BEF0 8001B6F0 00000000 */   nop
endlabel func_8001B690
