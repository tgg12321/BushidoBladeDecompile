glabel func_80052C4C
    /* 4344C 80052C4C 06008894 */  lhu        $t0, 0x6($a0)
    /* 43450 80052C50 08008984 */  lh         $t1, 0x8($a0)
    /* 43454 80052C54 0A008A94 */  lhu        $t2, 0xA($a0)
    /* 43458 80052C58 0C008B84 */  lh         $t3, 0xC($a0)
    /* 4345C 80052C5C 0E008C94 */  lhu        $t4, 0xE($a0)
    /* 43460 80052C60 10008D84 */  lh         $t5, 0x10($a0)
    /* 43464 80052C64 004C0900 */  sll        $t1, $t1, 16
    /* 43468 80052C68 005C0B00 */  sll        $t3, $t3, 16
    /* 4346C 80052C6C 006C0D00 */  sll        $t5, $t5, 16
    /* 43470 80052C70 25400901 */  or         $t0, $t0, $t1
    /* 43474 80052C74 25504B01 */  or         $t2, $t2, $t3
    /* 43478 80052C78 25608D01 */  or         $t4, $t4, $t5
    /* 4347C 80052C7C 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 43480 80052C80 0008CA48 */  ctc2       $t2, $1 /* handwritten instruction */
    /* 43484 80052C84 0010CC48 */  ctc2       $t4, $2 /* handwritten instruction */
    /* 43488 80052C88 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 4348C 80052C8C 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 43490 80052C90 0038C048 */  ctc2       $zero, $7 /* handwritten instruction */
    /* 43494 80052C94 00008884 */  lh         $t0, 0x0($a0)
    /* 43498 80052C98 02008984 */  lh         $t1, 0x2($a0)
    /* 4349C 80052C9C 04008A84 */  lh         $t2, 0x4($a0)
    /* 434A0 80052CA0 2228A800 */  sub        $a1, $a1, $t0 /* handwritten instruction */
    /* 434A4 80052CA4 2230C900 */  sub        $a2, $a2, $t1 /* handwritten instruction */
    /* 434A8 80052CA8 2238EA00 */  sub        $a3, $a3, $t2 /* handwritten instruction */
    /* 434AC 80052CAC FFFFA530 */  andi       $a1, $a1, 0xFFFF
    /* 434B0 80052CB0 00340600 */  sll        $a2, $a2, 16
    /* 434B4 80052CB4 2528A600 */  or         $a1, $a1, $a2
    /* 434B8 80052CB8 00008548 */  mtc2       $a1, $0 /* handwritten instruction */
    /* 434BC 80052CBC 00088748 */  mtc2       $a3, $1 /* handwritten instruction */
    /* 434C0 80052CC0 00000000 */  nop
    /* 434C4 80052CC4 00000000 */  nop
    /* 434C8 80052CC8 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 434CC 80052CCC 0800E003 */  jr         $ra
    /* 434D0 80052CD0 00000000 */   nop
endlabel func_80052C4C
