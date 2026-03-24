glabel func_8006DF68
    /* 5E768 8006DF68 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 5E76C 8006DF6C 1800B0AF */  sw         $s0, 0x18($sp)
    /* 5E770 8006DF70 2180A000 */  addu       $s0, $a1, $zero
    /* 5E774 8006DF74 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 5E778 8006DF78 21880000 */  addu       $s1, $zero, $zero
    /* 5E77C 8006DF7C 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5E780 8006DF80 21280000 */  addu       $a1, $zero, $zero
    /* 5E784 8006DF84 FFFF0232 */  andi       $v0, $s0, 0xFFFF
    /* 5E788 8006DF88 021C1000 */  srl        $v1, $s0, 16
    /* 5E78C 8006DF8C 25104300 */  or         $v0, $v0, $v1
    /* 5E790 8006DF90 3004868F */  lw         $a2, %gp_rel(D_800A34FC)($gp)
    /* 5E794 8006DF94 0A80073C */  lui        $a3, %hi(D_800A350C)
    /* 5E798 8006DF98 0C35E724 */  addiu      $a3, $a3, %lo(D_800A350C)
    /* 5E79C 8006DF9C 2000BFAF */  sw         $ra, 0x20($sp)
    /* 5E7A0 8006DFA0 1000A2AF */  sw         $v0, 0x10($sp)
    /* 5E7A4 8006DFA4 B0A4010C */  jal        func_800692C0
    /* 5E7A8 8006DFA8 0C00C624 */   addiu     $a2, $a2, 0xC
    /* 5E7AC 8006DFAC 03140200 */  sra        $v0, $v0, 16
    /* 5E7B0 8006DFB0 FF004230 */  andi       $v0, $v0, 0xFF
    /* 5E7B4 8006DFB4 08004010 */  beqz       $v0, .L8006DFD8
    /* 5E7B8 8006DFB8 21200000 */   addu      $a0, $zero, $zero
    /* 5E7BC 8006DFBC 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5E7C0 8006DFC0 6004828F */  lw         $v0, %gp_rel(D_800A352C)($gp)
    /* 5E7C4 8006DFC4 00000000 */  nop
    /* 5E7C8 8006DFC8 01004224 */  addiu      $v0, $v0, 0x1
    /* 5E7CC 8006DFCC 600482AF */  sw         $v0, %gp_rel(D_800A352C)($gp)
    /* 5E7D0 8006DFD0 9471010C */  jal        func_8005C650
    /* 5E7D4 8006DFD4 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8006DFD8:
    /* 5E7D8 8006DFD8 1000033C */  lui        $v1, (0x100010 >> 16)
    /* 5E7DC 8006DFDC 10006334 */  ori        $v1, $v1, (0x100010 & 0xFFFF)
    /* 5E7E0 8006DFE0 6004828F */  lw         $v0, %gp_rel(D_800A352C)($gp)
    /* 5E7E4 8006DFE4 24180302 */  and        $v1, $s0, $v1
    /* 5E7E8 8006DFE8 01004230 */  andi       $v0, $v0, 0x1
    /* 5E7EC 8006DFEC 600482AF */  sw         $v0, %gp_rel(D_800A352C)($gp)
    /* 5E7F0 8006DFF0 07006010 */  beqz       $v1, .L8006E010
    /* 5E7F4 8006DFF4 02000424 */   addiu     $a0, $zero, 0x2
    /* 5E7F8 8006DFF8 FFFF1124 */  addiu      $s1, $zero, -0x1
    /* 5E7FC 8006DFFC 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5E800 8006E000 9471010C */  jal        func_8005C650
    /* 5E804 8006E004 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5E808 8006E008 11B80108 */  j          .L8006E044
    /* 5E80C 8006E00C 00000000 */   nop
  .L8006E010:
    /* 5E810 8006E010 4000023C */  lui        $v0, (0x400040 >> 16)
    /* 5E814 8006E014 40004234 */  ori        $v0, $v0, (0x400040 & 0xFFFF)
    /* 5E818 8006E018 24100202 */  and        $v0, $s0, $v0
    /* 5E81C 8006E01C 09004010 */  beqz       $v0, .L8006E044
    /* 5E820 8006E020 01000424 */   addiu     $a0, $zero, 0x1
    /* 5E824 8006E024 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5E828 8006E028 9471010C */  jal        func_8005C650
    /* 5E82C 8006E02C 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5E830 8006E030 6004828F */  lw         $v0, %gp_rel(D_800A352C)($gp)
    /* 5E834 8006E034 00000000 */  nop
    /* 5E838 8006E038 02004010 */  beqz       $v0, .L8006E044
    /* 5E83C 8006E03C 01001124 */   addiu     $s1, $zero, 0x1
    /* 5E840 8006E040 FFFF1124 */  addiu      $s1, $zero, -0x1
  .L8006E044:
    /* 5E844 8006E044 B471010C */  jal        func_8005C6D0
    /* 5E848 8006E048 00000000 */   nop
    /* 5E84C 8006E04C 21102002 */  addu       $v0, $s1, $zero
    /* 5E850 8006E050 2000BF8F */  lw         $ra, 0x20($sp)
    /* 5E854 8006E054 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 5E858 8006E058 1800B08F */  lw         $s0, 0x18($sp)
    /* 5E85C 8006E05C 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 5E860 8006E060 0800E003 */  jr         $ra
    /* 5E864 8006E064 00000000 */   nop
endlabel func_8006DF68
