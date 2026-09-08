glabel SsEnd
    /* 74154 80083954 0A80023C */  lui        $v0, %hi(_snd_seq_tick_env_plus_0x4)
    /* 74158 80083958 D026428C */  lw         $v0, %lo(_snd_seq_tick_env_plus_0x4)($v0)
    /* 7415C 8008395C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 74160 80083960 29004014 */  bnez       $v0, .L80083A08
    /* 74164 80083964 1000BFAF */   sw        $ra, 0x10($sp)
    /* 74168 80083968 0A80033C */  lui        $v1, %hi(_snd_seq_tick_env_plus_0x12)
    /* 7416C 8008396C DE266390 */  lbu        $v1, %lo(_snd_seq_tick_env_plus_0x12)($v1)
    /* 74170 80083970 7F000224 */  addiu      $v0, $zero, 0x7F
    /* 74174 80083974 0A80013C */  lui        $at, %hi(_snd_seq_tick_env_plus_0x11)
    /* 74178 80083978 DD2620A0 */  sb         $zero, %lo(_snd_seq_tick_env_plus_0x11)($at)
    /* 7417C 8008397C 22006210 */  beq        $v1, $v0, .L80083A08
    /* 74180 80083980 00000000 */   nop
    /* 74184 80083984 6EE2010C */  jal        EnterCriticalSection
    /* 74188 80083988 00000000 */   nop
    /* 7418C 8008398C 0A80023C */  lui        $v0, %hi(_snd_seq_tick_env_plus_0x10)
    /* 74190 80083990 DC264290 */  lbu        $v0, %lo(_snd_seq_tick_env_plus_0x10)($v0)
    /* 74194 80083994 00000000 */  nop
    /* 74198 80083998 07004010 */  beqz       $v0, .L800839B8
    /* 7419C 8008399C 00000000 */   nop
    /* 741A0 800839A0 D40A020C */  jal        VSyncCallback
    /* 741A4 800839A4 21200000 */   addu      $a0, $zero, $zero
    /* 741A8 800839A8 0A80013C */  lui        $at, %hi(_snd_seq_tick_env_plus_0x10)
    /* 741AC 800839AC DC2620A0 */  sb         $zero, %lo(_snd_seq_tick_env_plus_0x10)($at)
    /* 741B0 800839B0 7D0E0208 */  j          .L800839F4
    /* 741B4 800839B4 00000000 */   nop
  .L800839B8:
    /* 741B8 800839B8 0A80023C */  lui        $v0, %hi(_snd_seq_tick_env_plus_0x12)
    /* 741BC 800839BC DE264290 */  lbu        $v0, %lo(_snd_seq_tick_env_plus_0x12)($v0)
    /* 741C0 800839C0 00000000 */  nop
    /* 741C4 800839C4 09004014 */  bnez       $v0, .L800839EC
    /* 741C8 800839C8 06000424 */   addiu     $a0, $zero, 0x6
    /* 741CC 800839CC 0A80053C */  lui        $a1, %hi(_snd_seq_tick_env_plus_0xC)
    /* 741D0 800839D0 D826A58C */  lw         $a1, %lo(_snd_seq_tick_env_plus_0xC)($a1)
    /* 741D4 800839D4 BC0A020C */  jal        InterruptCallback
    /* 741D8 800839D8 21200000 */   addu      $a0, $zero, $zero
    /* 741DC 800839DC 0A80013C */  lui        $at, %hi(_snd_seq_tick_env_plus_0xC)
    /* 741E0 800839E0 D82620AC */  sw         $zero, %lo(_snd_seq_tick_env_plus_0xC)($at)
    /* 741E4 800839E4 7D0E0208 */  j          .L800839F4
    /* 741E8 800839E8 00000000 */   nop
  .L800839EC:
    /* 741EC 800839EC BC0A020C */  jal        InterruptCallback
    /* 741F0 800839F0 21280000 */   addu      $a1, $zero, $zero
  .L800839F4:
    /* 741F4 800839F4 72E2010C */  jal        ExitCriticalSection
    /* 741F8 800839F8 00000000 */   nop
    /* 741FC 800839FC 7F000224 */  addiu      $v0, $zero, 0x7F
    /* 74200 80083A00 0A80013C */  lui        $at, %hi(_snd_seq_tick_env_plus_0x12)
    /* 74204 80083A04 DE2622A0 */  sb         $v0, %lo(_snd_seq_tick_env_plus_0x12)($at)
  .L80083A08:
    /* 74208 80083A08 1000BF8F */  lw         $ra, 0x10($sp)
    /* 7420C 80083A0C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 74210 80083A10 0800E003 */  jr         $ra
    /* 74214 80083A14 00000000 */   nop
endlabel SsEnd
