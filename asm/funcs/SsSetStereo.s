glabel SsSetStereo
    /* 78724 80087F24 1080013C */  lui        $at, %hi(_svm_stereo_mono)
    /* 78728 80087F28 78F520A4 */  sh         $zero, %lo(_svm_stereo_mono)($at)
    /* 7872C 80087F2C 0800E003 */  jr         $ra
    /* 78730 80087F30 00000000 */   nop
endlabel SsSetStereo
