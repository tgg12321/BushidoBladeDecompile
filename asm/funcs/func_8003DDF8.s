glabel func_8003DDF8
    /* 2E5F8 8003DDF8 FF00033C */  lui        $v1, (0xFFFFFF >> 16)
    /* 2E5FC 8003DDFC FFFF6334 */  ori        $v1, $v1, (0xFFFFFF & 0xFFFF)
    /* 2E600 8003DE00 0A80023C */  lui        $v0, %hi(D_800A378C)
    /* 2E604 8003DE04 8C37428C */  lw         $v0, %lo(D_800A378C)($v0)
    /* 2E608 8003DE08 24208300 */  and        $a0, $a0, $v1
    /* 2E60C 8003DE0C 0800E003 */  jr         $ra
    /* 2E610 8003DE10 FC3F44AC */   sw        $a0, 0x3FFC($v0)
endlabel func_8003DDF8
