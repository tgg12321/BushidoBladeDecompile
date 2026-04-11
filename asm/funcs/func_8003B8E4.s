glabel func_8003B8E4
    /* 2C0E4 8003B8E4 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2C0E8 8003B8E8 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2C0EC 8003B8EC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2C0F0 8003B8F0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2C0F4 8003B8F4 01004224 */  addiu      $v0, $v0, 0x1
    /* 2C0F8 8003B8F8 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2C0FC 8003B8FC B83722AC */  sw         $v0, %lo(D_800A37B8)($at)
    /* 2C100 8003B900 03004228 */  slti       $v0, $v0, 0x3
    /* 2C104 8003B904 0F004010 */  beqz       $v0, .L8003B944
    /* 2C108 8003B908 00000000 */   nop
    /* 2C10C 8003B90C 0A80043C */  lui        $a0, %hi(D_800A38B4)
    /* 2C110 8003B910 B438848C */  lw         $a0, %lo(D_800A38B4)($a0)
    /* 2C114 8003B914 5181010C */  jal        func_80060544
    /* 2C118 8003B918 01000524 */   addiu     $a1, $zero, 0x1
    /* 2C11C 8003B91C 02004104 */  bgez       $v0, .L8003B928
    /* 2C120 8003B920 00000000 */   nop
    /* 2C124 8003B924 03004224 */  addiu      $v0, $v0, 0x3
  .L8003B928:
    /* 2C128 8003B928 83100200 */  sra        $v0, $v0, 2
    /* 2C12C 8003B92C 0A80033C */  lui        $v1, %hi(D_800A38B4)
    /* 2C130 8003B930 B438638C */  lw         $v1, %lo(D_800A38B4)($v1)
    /* 2C134 8003B934 80100200 */  sll        $v0, $v0, 2
    /* 2C138 8003B938 21186200 */  addu       $v1, $v1, $v0
    /* 2C13C 8003B93C 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 2C140 8003B940 B43823AC */  sw         $v1, %lo(D_800A38B4)($at)
  .L8003B944:
    /* 2C144 8003B944 0A80033C */  lui        $v1, %hi(D_800A37B8)
    /* 2C148 8003B948 B837638C */  lw         $v1, %lo(D_800A37B8)($v1)
    /* 2C14C 8003B94C 03000224 */  addiu      $v0, $zero, 0x3
    /* 2C150 8003B950 1B006214 */  bne        $v1, $v0, .L8003B9C0
    /* 2C154 8003B954 00000000 */   nop
    /* 2C158 8003B958 CFEC010C */  jal        gpu_DrawSync
    /* 2C15C 8003B95C 21200000 */   addu      $a0, $zero, $zero
    /* 2C160 8003B960 0A80043C */  lui        $a0, %hi(D_800A3844)
    /* 2C164 8003B964 4438848C */  lw         $a0, %lo(D_800A3844)($a0)
    /* 2C168 8003B968 97EB000C */  jal        func_8003AE5C
    /* 2C16C 8003B96C 00000000 */   nop
    /* 2C170 8003B970 F4010224 */  addiu      $v0, $zero, 0x1F4
    /* 2C174 8003B974 0A80013C */  lui        $at, %hi(D_800A37C0)
    /* 2C178 8003B978 C03722AC */  sw         $v0, %lo(D_800A37C0)($at)
    /* 2C17C 8003B97C 14000224 */  addiu      $v0, $zero, 0x14
    /* 2C180 8003B980 0A80013C */  lui        $at, %hi(D_800A38F8)
    /* 2C184 8003B984 F83820A0 */  sb         $zero, %lo(D_800A38F8)($at)
    /* 2C188 8003B988 0A80013C */  lui        $at, %hi(D_800A3768)
    /* 2C18C 8003B98C 683722A0 */  sb         $v0, %lo(D_800A3768)($at)
    /* 2C190 8003B990 E475000C */  jal        se_data_set
    /* 2C194 8003B994 00000000 */   nop
    /* 2C198 8003B998 69ED000C */  jal        suDispMentalBar
    /* 2C19C 8003B99C 00000000 */   nop
    /* 2C1A0 8003B9A0 01000424 */  addiu      $a0, $zero, 0x1
    /* 2C1A4 8003B9A4 21280000 */  addu       $a1, $zero, $zero
    /* 2C1A8 8003B9A8 21300000 */  addu       $a2, $zero, $zero
    /* 2C1AC 8003B9AC DA59000C */  jal        disp_SetFramebufferMode
    /* 2C1B0 8003B9B0 21380000 */   addu      $a3, $zero, $zero
    /* 2C1B4 8003B9B4 01000224 */  addiu      $v0, $zero, 0x1
    /* 2C1B8 8003B9B8 0A80013C */  lui        $at, %hi(D_800A390D)
    /* 2C1BC 8003B9BC 0D3922A0 */  sb         $v0, %lo(D_800A390D)($at)
  .L8003B9C0:
    /* 2C1C0 8003B9C0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2C1C4 8003B9C4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2C1C8 8003B9C8 0800E003 */  jr         $ra
    /* 2C1CC 8003B9CC 00000000 */   nop
endlabel func_8003B8E4
