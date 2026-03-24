glabel func_8007C21C
    /* 6CA1C 8007C21C 02000224 */  addiu      $v0, $zero, 0x2
    /* 6CA20 8007C220 030082A0 */  sb         $v0, 0x3($a0)
    /* 6CA24 8007C224 0200A010 */  beqz       $a1, .L8007C230
    /* 6CA28 8007C228 00E6023C */   lui       $v0, (0xE6000002 >> 16)
    /* 6CA2C 8007C22C 02004234 */  ori        $v0, $v0, (0xE6000002 & 0xFFFF)
  .L8007C230:
    /* 6CA30 8007C230 0200C010 */  beqz       $a2, .L8007C23C
    /* 6CA34 8007C234 00000000 */   nop
    /* 6CA38 8007C238 01004234 */  ori        $v0, $v0, (0xE6000001 & 0xFFFF)
  .L8007C23C:
    /* 6CA3C 8007C23C 040082AC */  sw         $v0, 0x4($a0)
    /* 6CA40 8007C240 0800E003 */  jr         $ra
    /* 6CA44 8007C244 080080AC */   sw        $zero, 0x8($a0)
endlabel func_8007C21C
