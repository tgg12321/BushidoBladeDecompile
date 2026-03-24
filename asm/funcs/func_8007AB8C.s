glabel func_8007AB8C
    /* 6B38C 8007AB8C 01000224 */  addiu      $v0, $zero, 0x1
    /* 6B390 8007AB90 030082A0 */  sb         $v0, 0x3($a0)
    /* 6B394 8007AB94 0200C010 */  beqz       $a2, .L8007ABA0
    /* 6B398 8007AB98 00E1033C */   lui       $v1, (0xE1000200 >> 16)
    /* 6B39C 8007AB9C 00026334 */  ori        $v1, $v1, (0xE1000200 & 0xFFFF)
  .L8007ABA0:
    /* 6B3A0 8007ABA0 0200A010 */  beqz       $a1, .L8007ABAC
    /* 6B3A4 8007ABA4 FF09E230 */   andi      $v0, $a3, 0x9FF
    /* 6B3A8 8007ABA8 00044234 */  ori        $v0, $v0, 0x400
  .L8007ABAC:
    /* 6B3AC 8007ABAC 25106200 */  or         $v0, $v1, $v0
    /* 6B3B0 8007ABB0 0800E003 */  jr         $ra
    /* 6B3B4 8007ABB4 040082AC */   sw        $v0, 0x4($a0)
endlabel func_8007AB8C
