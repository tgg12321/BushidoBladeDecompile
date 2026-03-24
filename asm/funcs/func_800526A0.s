glabel func_800526A0
    /* 42EA0 800526A0 00F08448 */  mtc2       $a0, $30 /* handwritten instruction */
    /* 42EA4 800526A4 1A008010 */  beqz       $a0, .L80052710
    /* 42EA8 800526A8 1F000824 */   addiu     $t0, $zero, 0x1F
    /* 42EAC 800526AC 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 42EB0 800526B0 0180093C */  lui        $t1, %hi(D_80015620)
    /* 42EB4 800526B4 20562925 */  addiu      $t1, $t1, %lo(D_80015620)
    /* 42EB8 800526B8 1E004230 */  andi       $v0, $v0, 0x1E
    /* 42EBC 800526BC E8FF4820 */  addi       $t0, $v0, -0x18 /* handwritten instruction */
    /* 42EC0 800526C0 07000005 */  bltz       $t0, .L800526E0
    /* 42EC4 800526C4 00000000 */   nop
    /* 42EC8 800526C8 40100400 */  sll        $v0, $a0, 1
    /* 42ECC 800526CC 20104900 */  add        $v0, $v0, $t1 /* handwritten instruction */
    /* 42ED0 800526D0 00004284 */  lh         $v0, 0x0($v0)
    /* 42ED4 800526D4 00000000 */  nop
    /* 42ED8 800526D8 0800E003 */  jr         $ra
    /* 42EDC 800526DC 42120200 */   srl       $v0, $v0, 9
  .L800526E0:
    /* 42EE0 800526E0 18000824 */  addiu      $t0, $zero, 0x18
    /* 42EE4 800526E4 22400201 */  sub        $t0, $t0, $v0 /* handwritten instruction */
    /* 42EE8 800526E8 07400401 */  srav       $t0, $a0, $t0
    /* 42EEC 800526EC 40400800 */  sll        $t0, $t0, 1
    /* 42EF0 800526F0 20400901 */  add        $t0, $t0, $t1 /* handwritten instruction */
    /* 42EF4 800526F4 00000885 */  lh         $t0, 0x0($t0)
    /* 42EF8 800526F8 1F000924 */  addiu      $t1, $zero, 0x1F
    /* 42EFC 800526FC 22482201 */  sub        $t1, $t1, $v0 /* handwritten instruction */
    /* 42F00 80052700 43480900 */  sra        $t1, $t1, 1
    /* 42F04 80052704 04102801 */  sllv       $v0, $t0, $t1
    /* 42F08 80052708 0800E003 */  jr         $ra
    /* 42F0C 8005270C 02130200 */   srl       $v0, $v0, 12
  .L80052710:
    /* 42F10 80052710 0800E003 */  jr         $ra
    /* 42F14 80052714 21100000 */   addu      $v0, $zero, $zero
endlabel func_800526A0
