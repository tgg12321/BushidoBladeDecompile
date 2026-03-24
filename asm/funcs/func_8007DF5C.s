glabel func_8007DF5C
    /* 6E75C 8007DF5C 01088228 */  slti       $v0, $a0, 0x801
    /* 6E760 8007DF60 10004010 */  beqz       $v0, .L8007DFA4
    /* 6E764 8007DF64 01048228 */   slti      $v0, $a0, 0x401
    /* 6E768 8007DF68 06004010 */  beqz       $v0, .L8007DF84
    /* 6E76C 8007DF6C 40100400 */   sll       $v0, $a0, 1
    /* 6E770 8007DF70 0A80013C */  lui        $at, %hi(D_8009BF94)
    /* 6E774 8007DF74 21082200 */  addu       $at, $at, $v0
    /* 6E778 8007DF78 94BF2284 */  lh         $v0, %lo(D_8009BF94)($at)
    /* 6E77C 8007DF7C F9F70108 */  j          .L8007DFE4
    /* 6E780 8007DF80 00000000 */   nop
  .L8007DF84:
    /* 6E784 8007DF84 00080224 */  addiu      $v0, $zero, 0x800
    /* 6E788 8007DF88 23104400 */  subu       $v0, $v0, $a0
    /* 6E78C 8007DF8C 40100200 */  sll        $v0, $v0, 1
    /* 6E790 8007DF90 0A80013C */  lui        $at, %hi(D_8009BF94)
    /* 6E794 8007DF94 21082200 */  addu       $at, $at, $v0
    /* 6E798 8007DF98 94BF2284 */  lh         $v0, %lo(D_8009BF94)($at)
    /* 6E79C 8007DF9C F9F70108 */  j          .L8007DFE4
    /* 6E7A0 8007DFA0 00000000 */   nop
  .L8007DFA4:
    /* 6E7A4 8007DFA4 010C8228 */  slti       $v0, $a0, 0xC01
    /* 6E7A8 8007DFA8 09004014 */  bnez       $v0, .L8007DFD0
    /* 6E7AC 8007DFAC 40100400 */   sll       $v0, $a0, 1
    /* 6E7B0 8007DFB0 00100224 */  addiu      $v0, $zero, 0x1000
    /* 6E7B4 8007DFB4 23104400 */  subu       $v0, $v0, $a0
    /* 6E7B8 8007DFB8 40100200 */  sll        $v0, $v0, 1
    /* 6E7BC 8007DFBC 0A80013C */  lui        $at, %hi(D_8009BF94)
    /* 6E7C0 8007DFC0 21082200 */  addu       $at, $at, $v0
    /* 6E7C4 8007DFC4 94BF2284 */  lh         $v0, %lo(D_8009BF94)($at)
    /* 6E7C8 8007DFC8 F9F70108 */  j          .L8007DFE4
    /* 6E7CC 8007DFCC 23100200 */   negu      $v0, $v0
  .L8007DFD0:
    /* 6E7D0 8007DFD0 0A80013C */  lui        $at, %hi(D_8009AF94)
    /* 6E7D4 8007DFD4 21082200 */  addu       $at, $at, $v0
    /* 6E7D8 8007DFD8 94AF2284 */  lh         $v0, %lo(D_8009AF94)($at)
    /* 6E7DC 8007DFDC 00000000 */  nop
    /* 6E7E0 8007DFE0 23100200 */  negu       $v0, $v0
  .L8007DFE4:
    /* 6E7E4 8007DFE4 0800E003 */  jr         $ra
    /* 6E7E8 8007DFE8 00000000 */   nop
endlabel func_8007DF5C
