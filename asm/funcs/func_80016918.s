glabel func_80016918
    /* 7118 80016918 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 711C 8001691C 21200000 */  addu       $a0, $zero, $zero
    /* 7120 80016920 2000BFAF */  sw         $ra, 0x20($sp)
    /* 7124 80016924 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 7128 80016928 9FEB010C */  jal        func_8007AE7C
    /* 712C 8001692C 1800B0AF */   sw        $s0, 0x18($sp)
    /* 7130 80016930 45EC010C */  jal        func_8007B114
    /* 7134 80016934 21200000 */   addu      $a0, $zero, $zero
    /* 7138 80016938 A8EC010C */  jal        func_8007B2A0
    /* 713C 8001693C 21200000 */   addu      $a0, $zero, $zero
    /* 7140 80016940 25F8010C */  jal        func_8007E094
    /* 7144 80016944 F0001024 */   addiu     $s0, $zero, 0xF0
    /* 7148 80016948 40010424 */  addiu      $a0, $zero, 0x140
    /* 714C 8001694C F7FB010C */  jal        func_8007EFDC
    /* 7150 80016950 78000524 */   addiu     $a1, $zero, 0x78
    /* 7154 80016954 B159000C */  jal        disp_CalcFov
    /* 7158 80016958 2D000424 */   addiu     $a0, $zero, 0x2D
    /* 715C 8001695C FFFB010C */  jal        func_8007EFFC
    /* 7160 80016960 21204000 */   addu      $a0, $v0, $zero
    /* 7164 80016964 0F80113C */  lui        $s1, %hi(D_800F7438)
    /* 7168 80016968 38743126 */  addiu      $s1, $s1, %lo(D_800F7438)
    /* 716C 8001696C 21202002 */  addu       $a0, $s1, $zero
    /* 7170 80016970 21280000 */  addu       $a1, $zero, $zero
    /* 7174 80016974 21300000 */  addu       $a2, $zero, $zero
    /* 7178 80016978 80020724 */  addiu      $a3, $zero, 0x280
    /* 717C 8001697C A5E9010C */  jal        func_8007A694
    /* 7180 80016980 1000B0AF */   sw        $s0, 0x10($sp)
    /* 7184 80016984 90402426 */  addiu      $a0, $s1, 0x4090
    /* 7188 80016988 21280000 */  addu       $a1, $zero, $zero
    /* 718C 8001698C F0000624 */  addiu      $a2, $zero, 0xF0
    /* 7190 80016990 80020724 */  addiu      $a3, $zero, 0x280
    /* 7194 80016994 A5E9010C */  jal        func_8007A694
    /* 7198 80016998 1000B0AF */   sw        $s0, 0x10($sp)
    /* 719C 8001699C 5C002426 */  addiu      $a0, $s1, 0x5C
    /* 71A0 800169A0 21280000 */  addu       $a1, $zero, $zero
    /* 71A4 800169A4 F0000624 */  addiu      $a2, $zero, 0xF0
    /* 71A8 800169A8 80020724 */  addiu      $a3, $zero, 0x280
    /* 71AC 800169AC D3E9010C */  jal        func_8007A74C
    /* 71B0 800169B0 1000B0AF */   sw        $s0, 0x10($sp)
    /* 71B4 800169B4 EC402426 */  addiu      $a0, $s1, 0x40EC
    /* 71B8 800169B8 21280000 */  addu       $a1, $zero, $zero
    /* 71BC 800169BC 21300000 */  addu       $a2, $zero, $zero
    /* 71C0 800169C0 80020724 */  addiu      $a3, $zero, 0x280
    /* 71C4 800169C4 D3E9010C */  jal        func_8007A74C
    /* 71C8 800169C8 1000B0AF */   sw        $s0, 0x10($sp)
    /* 71CC 800169CC 01000424 */  addiu      $a0, $zero, 0x1
    /* 71D0 800169D0 21280000 */  addu       $a1, $zero, $zero
    /* 71D4 800169D4 21300000 */  addu       $a2, $zero, $zero
    /* 71D8 800169D8 DA59000C */  jal        disp_SetFramebufferMode
    /* 71DC 800169DC 21380000 */   addu      $a3, $zero, $zero
    /* 71E0 800169E0 0A80043C */  lui        $a0, %hi(D_800A30CC)
    /* 71E4 800169E4 CC308424 */  addiu      $a0, $a0, %lo(D_800A30CC)
    /* 71E8 800169E8 21280000 */  addu       $a1, $zero, $zero
    /* 71EC 800169EC 21300000 */  addu       $a2, $zero, $zero
    /* 71F0 800169F0 34ED010C */  jal        func_8007B4D0
    /* 71F4 800169F4 21380000 */   addu      $a3, $zero, $zero
    /* 71F8 800169F8 CFEC010C */  jal        func_8007B33C
    /* 71FC 800169FC 21200000 */   addu      $a0, $zero, $zero
    /* 7200 80016A00 2000BF8F */  lw         $ra, 0x20($sp)
    /* 7204 80016A04 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 7208 80016A08 1800B08F */  lw         $s0, 0x18($sp)
    /* 720C 80016A0C 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 7210 80016A10 0800E003 */  jr         $ra
    /* 7214 80016A14 00000000 */   nop
endlabel func_80016918
