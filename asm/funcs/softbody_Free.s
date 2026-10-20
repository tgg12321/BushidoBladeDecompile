glabel softbody_Free
    /* 868C 80017E8C 40100400 */  sll        $v0, $a0, 1
    /* 8690 80017E90 21104400 */  addu       $v0, $v0, $a0
    /* 8694 80017E94 80100200 */  sll        $v0, $v0, 2
    /* 8698 80017E98 21104400 */  addu       $v0, $v0, $a0
    /* 869C 80017E9C 80100200 */  sll        $v0, $v0, 2
    /* 86A0 80017EA0 0F80013C */  lui        $at, %hi(g_softbody_slots)
    /* 86A4 80017EA4 21082200 */  addu       $at, $at, $v0
    /* 86A8 80017EA8 406720AC */  sw         $zero, %lo(g_softbody_slots)($at)
    /* 86AC 80017EAC 0800E003 */  jr         $ra
    /* 86B0 80017EB0 00000000 */   nop
endlabel softbody_Free
