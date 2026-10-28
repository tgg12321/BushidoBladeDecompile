glabel DrawGradientAndStepMemcardSavePromptsThenSelectRowMenuInit
    /* 26638 80035E38 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2663C 80035E3C 01000224 */  addiu      $v0, $zero, 0x1
    /* 26640 80035E40 1000BFAF */  sw         $ra, 0x10($sp)
    /* 26644 80035E44 0A80013C */  lui        $at, %hi(g_main_frame_period)
    /* 26648 80035E48 F13622A0 */  sb         $v0, %lo(g_main_frame_period)($at)
    /* 2664C 80035E4C 4FD5000C */  jal        gpu_AddScreenGradient
    /* 26650 80035E50 00000000 */   nop
    /* 26654 80035E54 1CE3000C */  jal        memcard_StepSaveOverwriteAndFormatPrompts
    /* 26658 80035E58 00000000 */   nop
    /* 2665C 80035E5C 06004010 */  beqz       $v0, .L80035E78
    /* 26660 80035E60 08000224 */   addiu     $v0, $zero, 0x8
    /* 26664 80035E64 0A80013C */  lui        $at, %hi(g_main_loop_handler_index)
    /* 26668 80035E68 343822A4 */  sh         $v0, %lo(g_main_loop_handler_index)($at)
    /* 2666C 80035E6C 02000224 */  addiu      $v0, $zero, 0x2
    /* 26670 80035E70 0A80013C */  lui        $at, %hi(g_main_frame_period)
    /* 26674 80035E74 F13622A0 */  sb         $v0, %lo(g_main_frame_period)($at)
  .L80035E78:
    /* 26678 80035E78 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2667C 80035E7C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 26680 80035E80 0800E003 */  jr         $ra
    /* 26684 80035E84 00000000 */   nop
endlabel DrawGradientAndStepMemcardSavePromptsThenSelectRowMenuInit
