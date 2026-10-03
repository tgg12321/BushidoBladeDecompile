glabel pad_ResetState
    /* 9CF4 800194F4 04000224 */  addiu      $v0, $zero, 0x4
    /* 9CF8 800194F8 1080013C */  lui        $at, %hi(g_pad_state)
    /* 9CFC 800194FC 882722A4 */  sh         $v0, %lo(g_pad_state)($at)
    /* 9D00 80019500 1080013C */  lui        $at, %hi(g_pad_state_plus_0x2)
    /* 9D04 80019504 8A2722A4 */  sh         $v0, %lo(g_pad_state_plus_0x2)($at)
    /* 9D08 80019508 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 9D0C 8001950C 1080013C */  lui        $at, %hi(g_pad_state_plus_0x8)
    /* 9D10 80019510 902720AC */  sw         $zero, %lo(g_pad_state_plus_0x8)($at)
    /* 9D14 80019514 1080013C */  lui        $at, %hi(g_pad_state_plus_0xC)
    /* 9D18 80019518 942720AC */  sw         $zero, %lo(g_pad_state_plus_0xC)($at)
    /* 9D1C 8001951C 1080013C */  lui        $at, %hi(g_pad_state_plus_0x10)
    /* 9D20 80019520 982720AC */  sw         $zero, %lo(g_pad_state_plus_0x10)($at)
    /* 9D24 80019524 1080013C */  lui        $at, %hi(g_pad_state_plus_0x14)
    /* 9D28 80019528 9C2722AC */  sw         $v0, %lo(g_pad_state_plus_0x14)($at)
    /* 9D2C 8001952C 0800E003 */  jr         $ra
    /* 9D30 80019530 00000000 */   nop
endlabel pad_ResetState
