#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* --- Functions from text1b segment (0x80047ED0 - 0x80079A30) --- */

extern s32 D_800A33D0;
void func_80047ED0(s32 a0) {
    D_800A33D0 += a0;
}
INCLUDE_ASM("asm/funcs", func_80047EE8);
INCLUDE_ASM("asm/funcs", func_80047FBC);
INCLUDE_ASM("asm/funcs", func_800480C0);
INCLUDE_ASM("asm/funcs", func_800481E8);
INCLUDE_ASM("asm/funcs", func_800482C8);
INCLUDE_ASM("asm/funcs", func_800483DC);
INCLUDE_ASM("asm/funcs", func_800484A0);
INCLUDE_ASM("asm/funcs", func_80048530);
INCLUDE_ASM("asm/funcs", func_800485EC);
extern s16 D_800F6652;
s32 func_800167AC(void);
s16 func_800486FC(void) {
    if (func_800167AC()) {
        D_800F6652 = 1;
    } else {
        D_800F6652 = 0;
    }
    return D_800F6652;
}
extern s16 D_800F6652;
void func_80048744(s32 a0) {
    if (a0) {
        D_800F6652 = 1;
    } else {
        D_800F6652 = 0;
    }
}
INCLUDE_ASM("asm/funcs", func_8004876C);
INCLUDE_ASM("asm/funcs", func_8004881C);
INCLUDE_ASM("asm/funcs", func_80048864);
INCLUDE_ASM("asm/funcs", func_80048A7C);
INCLUDE_ASM("asm/funcs", func_80048AD0);
extern s32 D_800A33E4;
void func_80048B8C(s32 a0) {
    D_800A33E4 += a0;
}
INCLUDE_ASM("asm/funcs", func_80048BA4);
extern u8 D_800EF848[];
extern u16 D_80099C34[];
extern void func_80052C10(void);
void func_80048F58(s32 a0, s32 a1) {
    s32 s1 = a0;
    s32 s0 = a1;
    s32 i;
    u16 *src;
    u16 *dst;
    u8 *base;
    if (s0 > 0) {
        func_80052C10();
    }
    base = D_800EF848 + s0 * 308;
    *(u32 *)base = 0;
    src = (u16 *)(D_80099C34 + s1 * 7);
    dst = (u16 *)(base + 0x124);
    i = 0;
    do {
        *dst = *src;
        src++;
        i++;
        dst++;
    } while (i < 7);
}
INCLUDE_ASM("asm/funcs", func_80048FFC);
extern s16 D_800EF9F2;
extern s16 D_800EF9F4;
extern s16 D_800A33EA;
extern s16 D_800A33E8;
extern s32 D_800A33EC;
void func_8004939C(void) {
    s16 val = -1;
    s32 i = 0x39;
    s16 *p = &D_800EF9F2;
    do {
        *p = val;
        i--;
        p--;
    } while (i >= 0);
    D_800EF9F4 = -2;
    D_800A33EA = -1;
    D_800A33E8 = -1;
    D_800A33EC = -1;
}
INCLUDE_ASM("asm/funcs", func_800493E4);
INCLUDE_ASM("asm/funcs", func_800494D4);
INCLUDE_ASM("asm/funcs", func_8004954C);
INCLUDE_ASM("asm/funcs", func_80049584);
void func_80049710(void) {
}
INCLUDE_ASM("asm/funcs", func_80049718);
INCLUDE_ASM("asm/funcs", func_80049A2C);
INCLUDE_ASM("asm/funcs", func_80049C24);
extern s16 D_80099CC2;
extern s32 D_800A324C;
void func_80049E1C(void) {
    s16 val = -1;
    s32 i = 0x39;
    s16 *p = &D_80099CC2;
    do {
        *p = val;
        i--;
        p--;
    } while (i >= 0);
    D_800A324C = -1;
}
INCLUDE_ASM("asm/funcs", func_80049E4C);
INCLUDE_ASM("asm/funcs", func_80049F4C);
INCLUDE_ASM("asm/funcs", func_8004A09C);
INCLUDE_ASM("asm/funcs", func_8004A1FC);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004A348\n"
    "    addiu  $sp, $sp, -0x38\n"
    "    sw     $ra, 52($sp)\n"
    "    sw     $fp, 48($sp)\n"
    "    sw     $s7, 44($sp)\n"
    "    sw     $s6, 40($sp)\n"
    "    sw     $s5, 36($sp)\n"
    "    sw     $s4, 32($sp)\n"
    "    sw     $s3, 28($sp)\n"
    "    sw     $s2, 24($sp)\n"
    "    sw     $s1, 20($sp)\n"
    "    sw     $s0, 16($sp)\n"
    "    lui    $v0, %hi(D_800973FC)\n"
    "    addiu  $v0, $v0, %lo(D_800973FC)\n"
    "    lhu    $t0, 0($a0)\n"
    "    lhu    $t1, 2($a0)\n"
    "    lhu    $t2, 4($a0)\n"
    "    andi   $t0, $t0, 0xFFF\n"
    "    andi   $t1, $t1, 0xFFF\n"
    "    andi   $t2, $t2, 0xFFF\n"
    "    add    $t0, $t0, $t0\n"
    "    add    $t1, $t1, $t1\n"
    "    add    $t2, $t2, $t2\n"
    "    add    $t3, $v0, $t0\n"
    "    add    $t4, $v0, $t1\n"
    "    add    $t5, $v0, $t2\n"
    "    lh     $s0, 0($t3)\n"
    "    lh     $s1, 0($t4)\n"
    "    lh     $s2, 0($t5)\n"
    "    addi   $t0, $t0, 0x800\n"
    "    addi   $t1, $t1, 0x800\n"
    "    addi   $t2, $t2, 0x800\n"
    "    andi   $t0, $t0, 0x1FFE\n"
    "    andi   $t1, $t1, 0x1FFE\n"
    "    andi   $t2, $t2, 0x1FFE\n"
    "    add    $t0, $v0, $t0\n"
    "    add    $t1, $v0, $t1\n"
    "    add    $t2, $v0, $t2\n"
    "    lh     $t5, 0($t2)\n"
    "    lh     $s4, 0($t1)\n"
    "    lh     $s3, 0($t0)\n"
    "    mtc2   $t5, $8\n"
    "    mtc2   $s0, $9\n"
    "    mtc2   $s3, $10\n"
    "    mtc2   $s4, $11\n"
    "    mfc2   $s0, $9\n"
    "    nop\n"
    "    gpf    1\n"
    "    mult   $s0, $s4\n"
    "    neg    $t7, $s1\n"
    "    sh     $t7, 12($a1)\n"
    "    mfc2   $t6, $9\n"
    "    mfc2   $t7, $10\n"
    "    mfc2   $t8, $11\n"
    "    mtc2   $s2, $8\n"
    "    mtc2   $s0, $9\n"
    "    mtc2   $s3, $10\n"
    "    mtc2   $s4, $11\n"
    "    sh     $t8, 0($a1)\n"
    "    mflo   $t9\n"
    "    gpf    1\n"
    "    sra    $t9, $t9, 12\n"
    "    mult   $s3, $s4\n"
    "    mtc2   $s1, $8\n"
    "    sh     $t9, 14($a1)\n"
    "    mfc2   $t8, $9\n"
    "    mfc2   $t9, $10\n"
    "    mfc2   $a2, $11\n"
    "    mfc2   $s1, $8\n"
    "    mtc2   $t7, $25\n"
    "    neg    $a3, $t6\n"
    "    mtc2   $a3, $26\n"
    "    mtc2   $t8, $27\n"
    "    mtc2   $t7, $11\n"
    "    mflo   $a3\n"
    "    nop\n"
    "    gpl    1\n"
    "    sh     $a2, 6($a1)\n"
    "    mult   $s1, $t6\n"
    "    sra    $a3, $a3, 12\n"
    "    sh     $a3, 16($a1)\n"
    "    mfc2   $t6, $9\n"
    "    mfc2   $t7, $10\n"
    "    mfc2   $t8, $11\n"
    "    sh     $t6, 8($a1)\n"
    "    sh     $t7, 10($a1)\n"
    "    mflo   $a2\n"
    "    sh     $t8, 4($a1)\n"
    "    sra    $a2, $a2, 12\n"
    "    sub    $a2, $a2, $t9\n"
    "    sh     $a2, 2($a1)\n"
    "    lw     $ra, 52($sp)\n"
    "    lw     $fp, 48($sp)\n"
    "    lw     $s7, 44($sp)\n"
    "    lw     $s6, 40($sp)\n"
    "    lw     $s5, 36($sp)\n"
    "    lw     $s4, 32($sp)\n"
    "    lw     $s3, 28($sp)\n"
    "    lw     $s2, 24($sp)\n"
    "    lw     $s1, 20($sp)\n"
    "    lw     $s0, 16($sp)\n"
    "    jr     $ra\n"
    "    addiu  $sp, $sp, 0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004A4E0\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,52($sp)\n"
    "    sw   $fp,48($sp)\n"
    "    sw   $s7,44($sp)\n"
    "    sw   $s6,40($sp)\n"
    "    sw   $s5,36($sp)\n"
    "    sw   $s4,32($sp)\n"
    "    sw   $s3,28($sp)\n"
    "    sw   $s2,24($sp)\n"
    "    sw   $s1,20($sp)\n"
    "    sw   $s0,16($sp)\n"
    "    lui   $v0,%hi(D_800A3820)\n"
    "    lw   $v0,%lo(D_800A3820)($v0)\n"
    "    lui   $s7,%hi(D_80102C00)\n"
    "    addiu   $s7,$s7,%lo(D_80102C00)\n"
    "    beq   $v0,$s7,.L8004A5D0\n"
    "    lui   $s1,0x1F80\n"
    "    addu   $s5,$zero,$zero\n"
    "    lui   $fp,%hi(D_800FF610)\n"
    "    addiu   $fp,$fp,%lo(D_800FF610)\n"
    "    lw   $s4,28($s1)\n"
    ".L8004A534:\n"
    "    lw   $s2,0($s7)\n"
    "    addu   $s6,$fp,$zero\n"
    "    lbu   $t0,0($s2)\n"
    "    nop\n"
    "    sll   $t0,$t0,2\n"
    "    lui   $at,%hi(jtbl_8001541C)\n"
    "    addu   $at,$at,$t0\n"
    "    lw   $t0,%lo(jtbl_8001541C)($at)\n"
    "    nop\n"
    "    jr   $t0\n"
    "    addiu   $s7,$s7,0x4\n"
    ".L8004A560:\n"
    "    lh   $t0,4($s2)\n"
    "    lh   $t1,2($s2)\n"
    "    sll   $t0,$t0,2\n"
    "    lui   $t2,%hi(D_80103608)\n"
    "    addiu   $t2,$t2,%lo(D_80103608)\n"
    "    addu   $t0,$t0,$t2\n"
    "    lw   $t0,0($t0)\n"
    "    sll   $t1,$t1,2\n"
    "    addu   $t0,$t0,$t1\n"
    "    lw   $s0,0($t0)\n"
    "    nop\n"
    ".L8004A58C:\n"
    "    lw   $a1,0($s0)\n"
    "    nop\n"
    "    andi   $a0,$a1,0xFFFF\n"
    "    srl   $a1,$a1,16\n"
    "    beqz   $a0,.L8004A5D0\n"
    "    addi   $s0,$s0,0x4\n"
    "    sll   $a1,$a1,2\n"
    "    lui   $at,%hi(jtbl_8001545C)\n"
    "    addu   $at,$at,$a1\n"
    "    lw   $a1,%lo(jtbl_8001545C)($at)\n"
    "    nop\n"
    "    jr   $a1\n"
    "    nop\n"
    ".L8004A5C0:\n"
    "    lhu   $t0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    "    bnez   $t0,.L8004A58C\n"
    "    nop\n"
    "    .global .L8004A5D0\n"
    ".L8004A5D0:\n"
    "    lui   $v0,%hi(D_800A3820)\n"
    "    lw   $v0,%lo(D_800A3820)($v0)\n"
    "    nop\n"
    "    bne   $s7,$v0,.L8004A534\n"
    "    nop\n"
    "    lw   $ra,52($sp)\n"
    "    lw   $fp,48($sp)\n"
    "    lw   $s7,44($sp)\n"
    "    lw   $s6,40($sp)\n"
    "    lw   $s5,36($sp)\n"
    "    lw   $s4,32($sp)\n"
    "    lw   $s3,28($sp)\n"
    "    lw   $s2,24($sp)\n"
    "    lw   $s1,20($sp)\n"
    "    lw   $s0,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    "    .global .L8004A614\n"
    ".L8004A614:\n"
    "    addiu   $t0,$zero,0x3\n"
    "    subu   $t0,$t0,$s5\n"
    "    lui   $at,0x1F80\n"
    "    sw   $t0,8($at)\n"
    "    addiu   $a1,$s1,0x20\n"
    "    addiu   $a2,$s1,0x2B4\n"
    "    jal   func_8004A76C\n"
    "    nop\n"
    "    jal   func_8004A940\n"
    "    nop\n"
    "    j   .L8004A5C0\n"
    "    nop\n"
    "    .global .L8004A644\n"
    ".L8004A644:\n"
    "    addiu   $t0,$zero,0x3\n"
    "    subu   $t0,$t0,$s5\n"
    "    lui   $at,0x1F80\n"
    "    sw   $t0,8($at)\n"
    "    addiu   $a1,$s1,0x20\n"
    "    addiu   $a2,$s1,0x2B4\n"
    "    jal   func_8004A808\n"
    "    nop\n"
    "    lbu   $t1,1($s2)\n"
    "    lui   $s4,%hi(D_80095328)\n"
    "    addiu   $s4,$s4,%lo(D_80095328)\n"
    "    andi   $t1,$t1,0x1\n"
    "    beqz   $t1,.L8004A680\n"
    "    nop\n"
    "    addi   $s4,$s4,0xB0\n"
    ".L8004A680:\n"
    "    jal   func_8004A940\n"
    "    nop\n"
    "    lw   $s4,28($s1)\n"
    "    j   .L8004A5C0\n"
    "    nop\n"
    "    .global .L8004A694\n"
    ".L8004A694:\n"
    "    addiu   $t0,$zero,0x3\n"
    "    subu   $t0,$t0,$s5\n"
    "    lui   $at,0x1F80\n"
    "    sw   $t0,8($at)\n"
    "    addiu   $a1,$s1,0x20\n"
    "    addiu   $a2,$s1,0x2B4\n"
    "    lui   $t0,0x50\n"
    "    lui   $t1,0x3C\n"
    "    ctc2   $t0,$24\n"
    "    ctc2   $t1,$25\n"
    "    jal   func_8004A76C\n"
    "    nop\n"
    "    lui   $t0,0x140\n"
    "    lui   $t1,0x78\n"
    "    ctc2   $t0,$24\n"
    "    ctc2   $t1,$25\n"
    "    lui   $s4,%hi(D_80095508)\n"
    "    addiu   $s4,$s4,%lo(D_80095508)\n"
    "    jal   func_8004A940\n"
    "    nop\n"
    "    lw   $s4,28($s1)\n"
    "    j   .L8004A5C0\n"
    "    nop\n"
    "    .global .L8004A6F0\n"
    ".L8004A6F0:\n"
    "    addiu   $t0,$zero,0x3\n"
    "    subu   $t0,$t0,$s5\n"
    "    lui   $at,0x1F80\n"
    "    sw   $t0,8($at)\n"
    "    addiu   $a1,$s1,0x20\n"
    "    addiu   $a2,$s1,0x2B4\n"
    "    jal   func_8004A938\n"
    "    nop\n"
    "    lui   $s4,%hi(D_80095328)\n"
    "    addiu   $s4,$s4,%lo(D_80095328)\n"
    "    nop\n"
    "    jal   func_8004A940\n"
    "    nop\n"
    "    lw   $s4,28($s1)\n"
    "    j   .L8004A5C0\n"
    "    nop\n"
    "    .global .L8004A730\n"
    ".L8004A730:\n"
    "    addiu   $t0,$zero,0x3\n"
    "    subu   $t0,$t0,$s5\n"
    "    lui   $at,0x1F80\n"
    "    sw   $t0,8($at)\n"
    "    addiu   $a1,$s1,0x20\n"
    "    addiu   $a2,$s1,0x2B4\n"
    "    jal   func_8004A808\n"
    "    nop\n"
    "    lui   $s4,%hi(D_80095488)\n"
    "    addiu   $s4,$s4,%lo(D_80095488)\n"
    "    jal   func_8004A940\n"
    "    nop\n"
    "    lw   $s4,28($s1)\n"
    "    j   .L8004A5C0\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004A76C\n"
    "    addiu  $a0, $a0, -1\n"
    "    lwc2   $0, 0($s0)\n"
    "    lwc2   $1, 4($s0)\n"
    "    lwc2   $2, 8($s0)\n"
    "    lwc2   $3, 12($s0)\n"
    "    lwc2   $4, 16($s0)\n"
    "    lwc2   $5, 20($s0)\n"
    "    nop\n"
    "    nop\n"
    "    rtpt\n"
    "    j      .L8004A7B0\n"
    "    addiu  $s0, $s0, 24\n"
    ".L8004A79C:\n"
    "    rtpt\n"
    "    addiu  $a2, $a2, 6\n"
    "    sh     $t0, -6($a2)\n"
    "    sh     $t1, -4($a2)\n"
    "    sh     $t2, -2($a2)\n"
    ".L8004A7B0:\n"
    "    swc2   $12, 0($a1)\n"
    "    swc2   $13, 4($a1)\n"
    "    swc2   $14, 8($a1)\n"
    "    mfc2   $t0, $17\n"
    "    mfc2   $t1, $18\n"
    "    mfc2   $t2, $19\n"
    "    addiu  $a1, $a1, 12\n"
    "    beqz   $a0, .L8004A7F4\n"
    "    addiu  $a0, $a0, -1\n"
    "    lwc2   $0, 0($s0)\n"
    "    lwc2   $1, 4($s0)\n"
    "    lwc2   $2, 8($s0)\n"
    "    lwc2   $3, 12($s0)\n"
    "    lwc2   $4, 16($s0)\n"
    "    lwc2   $5, 20($s0)\n"
    "    j      .L8004A79C\n"
    "    addiu  $s0, $s0, 24\n"
    ".L8004A7F4:\n"
    "    sh     $t0, 0($a2)\n"
    "    sh     $t1, 2($a2)\n"
    "    sh     $t2, 4($a2)\n"
    "    jr     $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004A808\n"
    "    addiu  $a0, $a0, -3\n"
    "    lhu    $t6, 0($s0)\n"
    "    lhu    $t7, 2($s0)\n"
    "    lhu    $t8, 4($s0)\n"
    "    sll    $t7, $t7, 16\n"
    "    or     $t6, $t6, $t7\n"
    "    mtc2   $t6, $0\n"
    "    mtc2   $t8, $1\n"
    "    lhu    $t6, 6($s0)\n"
    "    lhu    $t7, 8($s0)\n"
    "    lhu    $t8, 10($s0)\n"
    "    sll    $t7, $t7, 16\n"
    "    or     $t6, $t6, $t7\n"
    "    mtc2   $t6, $2\n"
    "    mtc2   $t8, $3\n"
    "    lhu    $t6, 12($s0)\n"
    "    lhu    $t7, 14($s0)\n"
    "    lhu    $t8, 16($s0)\n"
    "    sll    $t7, $t7, 16\n"
    "    or     $t6, $t6, $t7\n"
    "    mtc2   $t6, $4\n"
    "    mtc2   $t8, $5\n"
    "    nop\n"
    "    nop\n"
    "    rtpt\n"
    "    j      .L8004A888\n"
    "    addiu  $s0, $s0, 18\n"
    ".L8004A874:\n"
    "    rtpt\n"
    "    addiu  $a2, $a2, 6\n"
    "    sh     $t0, -6($a2)\n"
    "    sh     $t1, -4($a2)\n"
    "    sh     $t2, -2($a2)\n"
    ".L8004A888:\n"
    "    swc2   $12, 0($a1)\n"
    "    swc2   $13, 4($a1)\n"
    "    swc2   $14, 8($a1)\n"
    "    mfc2   $t0, $17\n"
    "    mfc2   $t1, $18\n"
    "    mfc2   $t2, $19\n"
    "    addiu  $a1, $a1, 12\n"
    "    blez   $a0, .L8004A90C\n"
    "    nop\n"
    "    addiu  $a0, $a0, -3\n"
    "    lhu    $t6, 0($s0)\n"
    "    lhu    $t7, 2($s0)\n"
    "    lhu    $t8, 4($s0)\n"
    "    sll    $t7, $t7, 16\n"
    "    or     $t6, $t6, $t7\n"
    "    mtc2   $t6, $0\n"
    "    mtc2   $t8, $1\n"
    "    lhu    $t6, 6($s0)\n"
    "    lhu    $t7, 8($s0)\n"
    "    lhu    $t8, 10($s0)\n"
    "    sll    $t7, $t7, 16\n"
    "    or     $t6, $t6, $t7\n"
    "    mtc2   $t6, $2\n"
    "    mtc2   $t8, $3\n"
    "    lhu    $t6, 12($s0)\n"
    "    lhu    $t7, 14($s0)\n"
    "    lhu    $t8, 16($s0)\n"
    "    sll    $t7, $t7, 16\n"
    "    or     $t6, $t6, $t7\n"
    "    mtc2   $t6, $4\n"
    "    mtc2   $t8, $5\n"
    "    j      .L8004A874\n"
    "    addiu  $s0, $s0, 18\n"
    ".L8004A90C:\n"
    "    sh     $t0, 0($a2)\n"
    "    sh     $t1, 2($a2)\n"
    "    sh     $t2, 4($a2)\n"
    "    add    $a0, $a0, $a0\n"
    "    add    $t0, $a0, $a0\n"
    "    add    $a0, $t0, $a0\n"
    "    add    $s0, $s0, $a0\n"
    "    andi   $t0, $s0, 3\n"
    "    add    $s0, $s0, $t0\n"
    "    jr     $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
void func_8004A938(void) {
}
INCLUDE_ASM("asm/funcs", func_8004A940);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004BB68\n"
    "    bnez   $a0,.L8004BBF0\n"
    "    nop\n"
    "    lwc2   $0,8($s3)\n"
    "    lwc2   $1,12($s3)\n"
    "    lwc2   $2,16($s3)\n"
    "    lwc2   $3,20($s3)\n"
    "    lwc2   $4,24($s3)\n"
    "    lwc2   $5,28($s3)\n"
    "    nop\n"
    "    nop\n"
    "    rtpt\n"
    "    lhu   $t4,460($s7)\n"
    "    lhu   $t5,462($s7)\n"
    "    lhu   $t0,14($s3)\n"
    "    lhu   $t1,22($s3)\n"
    "    lhu   $t2,30($s3)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s4)\n"
    "    sw   $t1,20($s4)\n"
    "    sh   $t2,28($s4)\n"
    "    swc2   $12,8($s4)\n"
    "    swc2   $13,16($s4)\n"
    "    swc2   $14,24($s4)\n"
    "    lw   $t0,464($s7)\n"
    "    sw   $s1,0($s4)\n"
    "    sw   $t0,4($s4)\n"
    "    lui   $t1,0x700\n"
    "    and   $s1,$s4,$fp\n"
    "    or   $s1,$s1,$t1\n"
    "    jr   $ra\n"
    "    addi   $s4,$s4,0x20\n"
    ".L8004BBF0:\n"
    "    addi   $s3,$s3,0x28\n"
    "    addi   $a0,$a0,-0x1\n"
    "    sw   $ra,0($s3)\n"
    "    sh   $a0,4($s3)\n"
    "    lw   $t0,-32($s3)\n"
    "    lw   $t1,-28($s3)\n"
    "    sw   $t0,8($s3)\n"
    "    sw   $t1,12($s3)\n"
    "    addi   $a0,$s3,-0x20\n"
    "    addi   $a1,$s3,-0x18\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x10\n"
    "    addi   $a0,$s3,-0x20\n"
    "    addi   $a1,$s3,-0x10\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x18\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004BB68\n"
    "    nop\n"
    "    addi   $a0,$s3,-0x18\n"
    "    addi   $a1,$s3,-0x10\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x8\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004BB68\n"
    "    nop\n"
    "    lw   $t0,16($s3)\n"
    "    lw   $t1,20($s3)\n"
    "    sw   $t0,32($s3)\n"
    "    sw   $t1,36($s3)\n"
    "    lw   $t0,-16($s3)\n"
    "    lw   $t1,-12($s3)\n"
    "    sw   $t0,16($s3)\n"
    "    sw   $t1,20($s3)\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004BB68\n"
    "    nop\n"
    "    lw   $t0,32($s3)\n"
    "    lw   $t1,36($s3)\n"
    "    sw   $t0,16($s3)\n"
    "    sw   $t1,20($s3)\n"
    "    lw   $t0,-24($s3)\n"
    "    lw   $t1,-20($s3)\n"
    "    sw   $t0,24($s3)\n"
    "    sw   $t1,28($s3)\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004BB68\n"
    "    nop\n"
    "    lw   $ra,0($s3)\n"
    "    nop\n"
    "    jr   $ra\n"
    "    addi   $s3,$s3,-0x28\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004BCC0\n"
    "    lui   $t0,0x1F80\n"
    "    sw   $ra,492($t0)\n"
    "    sw   $fp,496($t0)\n"
    "    sw   $s7,500($t0)\n"
    "    sw   $s6,504($t0)\n"
    "    sw   $s5,508($t0)\n"
    "    sw   $s4,512($t0)\n"
    "    sw   $s3,516($t0)\n"
    "    sw   $s2,520($t0)\n"
    "    sw   $s1,524($t0)\n"
    "    sw   $s0,528($t0)\n"
    "    addu   $s7,$t0,$zero\n"
    "    sh   $a3,480($s7)\n"
    "    sw   $a1,472($s7)\n"
    "    addu   $s0,$a2,$zero\n"
    "    lw   $s2,12($s7)\n"
    "    lui   $s4,%hi(D_800A38B4)\n"
    "    lw   $s4,%lo(D_800A38B4)($s4)\n"
    "    addi   $s5,$s7,32\n"
    "    addi   $s6,$s7,272\n"
    "    addi   $s3,$s7,532\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    addi   $a0,$a0,-0x1\n"
    ".L8004BD20:\n"
    "    lb   $t1,481($s7)\n"
    "    lw   $t0,0($s0)\n"
    "    andi   $t1,$t1,0x1\n"
    "    sll   $t1,$t1,25\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$6\n"
    "    srl   $t0,$t0,16\n"
    "    sh   $t0,452($s7)\n"
    "    lhu   $v1,16($s0)\n"
    "    lhu   $v0,14($s0)\n"
    "    sll   $v1,$v1,16\n"
    "    or   $v0,$v0,$v1\n"
    "    sw   $a0,468($s7)\n"
    "    sll   $t4,$v0,2\n"
    "    andi   $t4,$t4,0x3FC\n"
    "    addu   $t0,$t4,$s5\n"
    "    lw   $t0,0($t0)\n"
    "    srl   $t5,$v0,6\n"
    "    andi   $t5,$t5,0x3FC\n"
    "    addu   $t1,$t5,$s5\n"
    "    lw   $t1,0($t1)\n"
    "    srl   $t6,$v0,14\n"
    "    andi   $t6,$t6,0x3FC\n"
    "    addu   $t2,$t6,$s5\n"
    "    lw   $t2,0($t2)\n"
    "    srl   $t7,$v0,22\n"
    "    andi   $t7,$t7,0x3FC\n"
    "    addu   $t3,$t7,$s5\n"
    "    lw   $t3,0($t3)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $v1,$t0,$t1\n"
    "    and   $v1,$v1,$t2\n"
    "    and   $v1,$v1,$t3\n"
    "    nclip\n"
    "    bltz   $v1,.L8004C1A4\n"
    "    nop\n"
    "    andi   $v1,$v1,0x8000\n"
    "    mfc2   $v0,$24\n"
    "    bnez   $v1,.L8004C1A4\n"
    "    nop\n"
    "    bgez   $v0,.L8004C1A4\n"
    "    neg   $v1,$v0\n"
    "    sra   $a0,$t0,16\n"
    "    addi   $a0,$a0,-0xF0\n"
    "    sra   $a1,$t1,16\n"
    "    addi   $a1,$a1,-0xF0\n"
    "    or   $a0,$a0,$a1\n"
    "    sra   $a1,$t2,16\n"
    "    addi   $a1,$a1,-0xF0\n"
    "    or   $a0,$a0,$a1\n"
    "    sra   $a1,$t3,16\n"
    "    addi   $a1,$a1,-0xF0\n"
    "    or   $a0,$a0,$a1\n"
    "    andi   $a0,$a0,0x8000\n"
    "    beqz   $a0,.L8004C1A4\n"
    "    nop\n"
    "    srl   $t4,$t4,1\n"
    "    add   $a0,$t4,$s6\n"
    "    lh   $a0,0($a0)\n"
    "    srl   $t5,$t5,1\n"
    "    add   $a1,$t5,$s6\n"
    "    lh   $a1,0($a1)\n"
    "    srl   $t6,$t6,1\n"
    "    add   $a2,$t6,$s6\n"
    "    lh   $a2,0($a2)\n"
    "    srl   $t7,$t7,1\n"
    "    add   $a3,$t7,$s6\n"
    "    lh   $a3,0($a3)\n"
    "    slt   $v0,$a0,$a1\n"
    "    beqz   $v0,.L8004BE4C\n"
    "    slt   $v0,$a0,$a2\n"
    "    addu   $a0,$a1,$zero\n"
    "    slt   $v0,$a0,$a2\n"
    ".L8004BE4C:\n"
    "    beqz   $v0,.L8004BE5C\n"
    "    slt   $v0,$a0,$a3\n"
    "    addu   $a0,$a2,$zero\n"
    "    slt   $v0,$a0,$a3\n"
    ".L8004BE5C:\n"
    "    beqz   $v0,.L8004BE68\n"
    "    nop\n"
    "    addu   $a0,$a3,$zero\n"
    ".L8004BE68:\n"
    "    addu   $v0,$a0,$zero\n"
    "    addu   $a3,$v0,$zero\n"
    "    lbu   $t8,2($s0)\n"
    "    nop\n"
    "    andi   $t8,$t8,0x1\n"
    "    beqz   $t8,.L8004BE88\n"
    "    nop\n"
    "    addi   $v0,$v0,0x3E8\n"
    ".L8004BE88:\n"
    "    lh   $t8,454($s7)\n"
    "    lw   $a1,16($s7)\n"
    "    lw   $a2,24($s7)\n"
    "    srlv   $v0,$v0,$t8\n"
    "    srl   $t8,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t8\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    slt   $at,$a1,$v0\n"
    "    bnez   $at,.L8004C1A4\n"
    "    nop\n"
    "    slt   $at,$v0,$a2\n"
    "    bnez   $at,.L8004C1A4\n"
    "    nop\n"
    "    lhu   $a1,18($s0)\n"
    "    lwc2   $0,20($s0)\n"
    "    mtc2   $a1,$1\n"
    "    lh   $a0,4($s0)\n"
    "    lh   $a1,0($s0)\n"
    "    sh   $a0,460($s7)\n"
    "    sh   $a1,462($s7)\n"
    "    ncs\n"
    "    slti   $a0,$v1,0x400\n"
    "    slti   $v1,$v1,0x700\n"
    "    add   $v1,$a0,$v1\n"
    "    lw   $a1,448($s7)\n"
    "    nop\n"
    "    andi   $a1,$a1,0x3\n"
    "    beqz   $a1,.L8004BF44\n"
    "    addu   $a0,$zero,$zero\n"
    "    addi   $a2,$a1,-0x2\n"
    "    beqz   $a2,.L8004BF48\n"
    "    nop\n"
    "    lb   $a0,480($s7)\n"
    "    nop\n"
    "    addi   $a0,$a0,-0x2\n"
    "    neg   $a0,$a0\n"
    "    addi   $a2,$a1,-0x1\n"
    "    beqz   $a2,.L8004BF44\n"
    "    nop\n"
    "    lb   $a0,480($s7)\n"
    "    nop\n"
    "    slt   $a1,$a0,$v1\n"
    "    neg   $a1,$a1\n"
    "    sub   $a0,$v1,$a0\n"
    "    and   $a0,$a0,$a1\n"
    ".L8004BF44:\n"
    "    addu   $v1,$a0,$zero\n"
    ".L8004BF48:\n"
    "    lhu   $a0,6($s0)\n"
    "    lhu   $a1,8($s0)\n"
    "    lhu   $a2,10($s0)\n"
    "    lhu   $a3,12($s0)\n"
    "    beqz   $v1,.L8004BF8C\n"
    "    addiu   $t8,$zero,0xFF\n"
    "    srlv   $t8,$t8,$v1\n"
    "    sll   $t9,$t8,8\n"
    "    or   $t8,$t8,$t9\n"
    "    srlv   $a0,$a0,$v1\n"
    "    and   $a0,$a0,$t8\n"
    "    srlv   $a1,$a1,$v1\n"
    "    and   $a1,$a1,$t8\n"
    "    srlv   $a2,$a2,$v1\n"
    "    and   $a2,$a2,$t8\n"
    "    srlv   $a3,$a3,$v1\n"
    "    and   $a3,$a3,$t8\n"
    ".L8004BF8C:\n"
    "    sh   $a0,482($s7)\n"
    "    sh   $a1,484($s7)\n"
    "    sh   $a2,486($s7)\n"
    "    sh   $a3,488($s7)\n"
    "    lbu   $t8,2($s0)\n"
    "    lb   $a3,480($s7)\n"
    "    srl   $t8,$t8,1\n"
    "    andi   $t8,$t8,0x3\n"
    "    beqz   $t8,.L8004BFC0\n"
    "    addi   $t8,$t8,-0x1\n"
    "    sub   $a3,$a3,$t8\n"
    "    bgtz   $a3,.L8004C048\n"
    "    nop\n"
    ".L8004BFC0:\n"
    "    lh   $t4,460($s7)\n"
    "    lh   $t5,462($s7)\n"
    "    sw   $t0,8($s4)\n"
    "    sw   $t1,16($s4)\n"
    "    sw   $t2,24($s4)\n"
    "    sw   $t3,32($s4)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,482($s7)\n"
    "    lhu   $t1,484($s7)\n"
    "    lhu   $t2,486($s7)\n"
    "    lhu   $t3,488($s7)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s4)\n"
    "    sw   $t1,20($s4)\n"
    "    sh   $t2,28($s4)\n"
    "    sh   $t3,36($s4)\n"
    "    swc2   $22,4($s4)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s2\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s4,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    lui   $t2,0x900\n"
    "    or   $t1,$t1,$t2\n"
    "    sw   $t1,0($s4)\n"
    "    lw   $a0,468($s7)\n"
    "    addi   $s0,$s0,0x18\n"
    "    addi   $s4,$s4,0x28\n"
    "    bnez   $a0,.L8004BD20\n"
    "    addi   $a0,$a0,-0x1\n"
    "    j   .L8004C1B4\n"
    "    nop\n"
    ".L8004C048:\n"
    "    lw   $a0,448($s7)\n"
    "    nop\n"
    "    andi   $a0,$a0,0x4\n"
    "    beqz   $a0,.L8004C0A8\n"
    "    nop\n"
    "    addu   $a0,$a3,$zero\n"
    "    lw   $a1,0($s0)\n"
    "    nop\n"
    "    lui   $at,0xFF\n"
    "    ori   $at,$at,65535\n"
    "    or   $a1,$a1,$at\n"
    "    beqz   $a0,.L8004C09C\n"
    "    nop\n"
    "    addiu   $at,$zero,-0x100\n"
    "    and   $a1,$a1,$at\n"
    "    addi   $a0,$a0,-0x1\n"
    "    beqz   $a0,.L8004C09C\n"
    "    nop\n"
    "    lui   $at,0xFFFF\n"
    "    ori   $at,$at,16384\n"
    "    and   $a1,$a1,$at\n"
    ".L8004C09C:\n"
    "    mtc2   $a1,$22\n"
    "    nop\n"
    "    nop\n"
    ".L8004C0A8:\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s2\n"
    "    lw   $s1,0($v0)\n"
    "    sw   $v0,476($s7)\n"
    "    lui   $t0,0x900\n"
    "    or   $s1,$s1,$t0\n"
    "    lw   $a1,472($s7)\n"
    "    swc2   $22,464($s7)\n"
    "    add   $t0,$t4,$t4\n"
    "    add   $t4,$t4,$t0\n"
    "    add   $t4,$t4,$a1\n"
    "    lh   $t0,0($t4)\n"
    "    lh   $t1,2($t4)\n"
    "    lh   $t2,4($t4)\n"
    "    lh   $t3,482($s7)\n"
    "    sh   $t0,8($s3)\n"
    "    sh   $t1,10($s3)\n"
    "    sh   $t2,12($s3)\n"
    "    sh   $t3,14($s3)\n"
    "    add   $t0,$t5,$t5\n"
    "    add   $t5,$t5,$t0\n"
    "    add   $t5,$t5,$a1\n"
    "    lh   $t0,0($t5)\n"
    "    lh   $t1,2($t5)\n"
    "    lh   $t2,4($t5)\n"
    "    lh   $t3,484($s7)\n"
    "    sh   $t0,16($s3)\n"
    "    sh   $t1,18($s3)\n"
    "    sh   $t2,20($s3)\n"
    "    sh   $t3,22($s3)\n"
    "    add   $t0,$t6,$t6\n"
    "    add   $t6,$t6,$t0\n"
    "    add   $t6,$t6,$a1\n"
    "    lh   $t0,0($t6)\n"
    "    lh   $t1,2($t6)\n"
    "    lh   $t2,4($t6)\n"
    "    lh   $t3,486($s7)\n"
    "    sh   $t0,24($s3)\n"
    "    sh   $t1,26($s3)\n"
    "    sh   $t2,28($s3)\n"
    "    sh   $t3,30($s3)\n"
    "    add   $t0,$t7,$t7\n"
    "    add   $t7,$t7,$t0\n"
    "    add   $t7,$t7,$a1\n"
    "    lh   $t0,0($t7)\n"
    "    lh   $t1,2($t7)\n"
    "    lh   $t2,4($t7)\n"
    "    lh   $t3,488($s7)\n"
    "    sh   $t0,32($s3)\n"
    "    sh   $t1,34($s3)\n"
    "    sh   $t2,36($s3)\n"
    "    sh   $t3,38($s3)\n"
    "    jal   func_8004C1F4\n"
    "    addu   $a0,$a3,$zero\n"
    "    lw   $t0,476($s7)\n"
    "    and   $t1,$s1,$fp\n"
    "    sw   $t1,0($t0)\n"
    "    lw   $a0,468($s7)\n"
    "    addi   $s0,$s0,0x18\n"
    "    bnez   $a0,.L8004BD20\n"
    "    addi   $a0,$a0,-0x1\n"
    "    j   .L8004C1B4\n"
    "    nop\n"
    ".L8004C1A4:\n"
    "    lw   $a0,468($s7)\n"
    "    addi   $s0,$s0,0x18\n"
    "    bnez   $a0,.L8004BD20\n"
    "    addi   $a0,$a0,-0x1\n"
    ".L8004C1B4:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s4,%lo(D_800A38B4)($at)\n"
    "    addu   $v0,$s0,$zero\n"
    "    addu   $t0,$s7,$zero\n"
    "    lw   $ra,492($t0)\n"
    "    lw   $fp,496($t0)\n"
    "    lw   $s7,500($t0)\n"
    "    lw   $s6,504($t0)\n"
    "    lw   $s5,508($t0)\n"
    "    lw   $s4,512($t0)\n"
    "    lw   $s3,516($t0)\n"
    "    lw   $s2,520($t0)\n"
    "    lw   $s1,524($t0)\n"
    "    lw   $s0,528($t0)\n"
    "    jr   $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004C1F4\n"
    "    bnez   $a0,.L8004C298\n"
    "    nop\n"
    "    lwc2   $0,8($s3)\n"
    "    lwc2   $1,12($s3)\n"
    "    lwc2   $2,16($s3)\n"
    "    lwc2   $3,20($s3)\n"
    "    lwc2   $4,24($s3)\n"
    "    lwc2   $5,28($s3)\n"
    "    nop\n"
    "    nop\n"
    "    rtpt\n"
    "    lhu   $t4,460($s7)\n"
    "    lhu   $t5,462($s7)\n"
    "    lhu   $t0,14($s3)\n"
    "    lhu   $t1,22($s3)\n"
    "    lhu   $t2,30($s3)\n"
    "    lhu   $t3,38($s3)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s4)\n"
    "    sw   $t1,20($s4)\n"
    "    sh   $t2,28($s4)\n"
    "    sh   $t3,36($s4)\n"
    "    swc2   $12,8($s4)\n"
    "    swc2   $13,16($s4)\n"
    "    swc2   $14,24($s4)\n"
    "    lwc2   $0,32($s3)\n"
    "    lwc2   $1,36($s3)\n"
    "    lw   $t0,464($s7)\n"
    "    nop\n"
    "    rtps\n"
    "    sw   $t0,4($s4)\n"
    "    sw   $s1,0($s4)\n"
    "    swc2   $14,32($s4)\n"
    "    lui   $t1,0x900\n"
    "    and   $s1,$s4,$fp\n"
    "    or   $s1,$s1,$t1\n"
    "    jr   $ra\n"
    "    addi   $s4,$s4,0x28\n"
    ".L8004C298:\n"
    "    addi   $s3,$s3,0x28\n"
    "    addi   $a0,$a0,-0x1\n"
    "    sw   $ra,0($s3)\n"
    "    sh   $a0,4($s3)\n"
    "    lw   $t0,-32($s3)\n"
    "    lw   $t1,-28($s3)\n"
    "    sw   $t0,8($s3)\n"
    "    sw   $t1,12($s3)\n"
    "    addi   $a0,$s3,-0x20\n"
    "    addi   $a1,$s3,-0x18\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x10\n"
    "    addi   $a0,$s3,-0x20\n"
    "    addi   $a1,$s3,-0x10\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x18\n"
    "    addi   $a0,$s3,-0x20\n"
    "    addi   $a1,$s3,-0x8\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x20\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004C1F4\n"
    "    nop\n"
    "    lw   $t0,-24($s3)\n"
    "    lw   $t1,-20($s3)\n"
    "    sw   $t0,8($s3)\n"
    "    sw   $t1,12($s3)\n"
    "    addi   $a0,$s3,-0x18\n"
    "    addi   $a1,$s3,-0x8\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x18\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004C1F4\n"
    "    nop\n"
    "    lw   $t0,-8($s3)\n"
    "    lw   $t1,-4($s3)\n"
    "    sw   $t0,8($s3)\n"
    "    sw   $t1,12($s3)\n"
    "    addi   $a0,$s3,-0x10\n"
    "    addi   $a1,$s3,-0x8\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x10\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004C1F4\n"
    "    nop\n"
    "    lw   $t0,-16($s3)\n"
    "    lw   $t1,-12($s3)\n"
    "    sw   $t0,8($s3)\n"
    "    sw   $t1,12($s3)\n"
    "    addi   $a0,$s3,-0x20\n"
    "    addi   $a1,$s3,-0x10\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x18\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004C1F4\n"
    "    nop\n"
    "    lw   $ra,0($s3)\n"
    "    nop\n"
    "    jr   $ra\n"
    "    addi   $s3,$s3,-0x28\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
INCLUDE_ASM("asm/funcs", func_8004C388);
PAD_NOPS_1; /* padding after func_8004C388 */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004C404\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    addu   $s2,$a0,$zero\n"
    "    addu   $s1,$a1,$zero\n"
    "    lui   $s3,0x1F80\n"
    "    lui   $s4,%hi(D_800F6658)\n"
    "    lh   $s4,%lo(D_800F6658)($s4)\n"
    "    addiu   $t0,$zero,0x3\n"
    "    sub   $t0,$t0,$s4\n"
    "    sh   $t0,454($s3)\n"
    "    lui   $t1,%hi(D_80103624)\n"
    "    lw   $t1,%lo(D_80103624)($t1)\n"
    "    nop\n"
    "    sw   $t1,456($s3)\n"
    "    lui   $a0,%hi(D_800A3790)\n"
    "    lw   $a0,%lo(D_800A3790)($a0)\n"
    "    nop\n"
    "    sw   $a0,448($s3)\n"
    "    lw   $t0,0($a2)\n"
    "    lw   $t1,4($a2)\n"
    "    lw   $t2,8($a2)\n"
    "    lw   $t3,12($a2)\n"
    "    lh   $t4,16($a2)\n"
    "    sw   $t0,392($s3)\n"
    "    sw   $t1,396($s3)\n"
    "    sw   $t2,400($s3)\n"
    "    sw   $t3,404($s3)\n"
    "    sh   $t4,408($s3)\n"
    "    addi   $s4,$s4,0x10\n"
    "    srav   $t0,$t0,$s4\n"
    "    srav   $t1,$t1,$s4\n"
    "    srav   $t2,$t2,$s4\n"
    "    srav   $t3,$t3,$s4\n"
    "    addi   $s4,$s4,-0x10\n"
    "    lh   $t5,392($s3)\n"
    "    lh   $t6,396($s3)\n"
    "    lh   $t7,400($s3)\n"
    "    lh   $t8,404($s3)\n"
    "    srav   $t4,$t4,$s4\n"
    "    srav   $t5,$t5,$s4\n"
    "    srav   $t6,$t6,$s4\n"
    "    srav   $t7,$t7,$s4\n"
    "    srav   $t8,$t8,$s4\n"
    "    sra   $t1,$t1,1\n"
    "    sra   $t7,$t7,1\n"
    "    sra   $t2,$t2,1\n"
    "    sh   $t5,412($s3)\n"
    "    sh   $t0,414($s3)\n"
    "    sh   $t6,416($s3)\n"
    "    sh   $t1,418($s3)\n"
    "    sh   $t7,420($s3)\n"
    "    sh   $t2,422($s3)\n"
    "    sh   $t8,424($s3)\n"
    "    sh   $t3,426($s3)\n"
    "    sh   $t4,428($s3)\n"
    "    lui   $t0,%hi(D_800A3708)\n"
    "    lw   $t0,%lo(D_800A3708)($t0)\n"
    "    nop\n"
    "    lw   $t1,44($t0)\n"
    "    lw   $t2,48($t0)\n"
    "    lw   $t3,52($t0)\n"
    "    sw   $t1,436($s3)\n"
    "    sw   $t2,440($s3)\n"
    "    sw   $t3,444($s3)\n"
    "    addiu   $t0,$zero,0x4\n"
    "    lui   $t2,%hi(D_800F62E0)\n"
    "    addiu   $t2,$t2,%lo(D_800F62E0)\n"
    "    sll   $t1,$t0,1\n"
    "    add   $t1,$t1,$t0\n"
    "    sll   $t1,$t1,5\n"
    "    addu   $t0,$t1,$t2\n"
    "    lw   $t1,24($t0)\n"
    "    lw   $t2,28($t0)\n"
    "    lw   $t3,32($t0)\n"
    "    lw   $t4,36($t0)\n"
    "    lw   $t5,40($t0)\n"
    "    ctc2   $t1,$8\n"
    "    ctc2   $t2,$9\n"
    "    ctc2   $t3,$10\n"
    "    ctc2   $t4,$11\n"
    "    ctc2   $t5,$12\n"
    "    lw   $t6,88($t0)\n"
    "    lw   $t1,56($t0)\n"
    "    lw   $t2,60($t0)\n"
    "    lw   $t3,64($t0)\n"
    "    lw   $t4,68($t0)\n"
    "    lw   $t5,72($t0)\n"
    "    ctc2   $t1,$16\n"
    "    ctc2   $t2,$17\n"
    "    ctc2   $t3,$18\n"
    "    ctc2   $t4,$19\n"
    "    ctc2   $t5,$20\n"
    "    addiu   $t0,$zero,0xFF0\n"
    "    sll   $t1,$t6,4\n"
    "    srl   $t2,$t6,4\n"
    "    srl   $t3,$t6,12\n"
    "    and   $t1,$t1,$t0\n"
    "    and   $t2,$t2,$t0\n"
    "    and   $t3,$t3,$t0\n"
    "    ctc2   $t1,$13\n"
    "    ctc2   $t2,$14\n"
    "    ctc2   $t3,$15\n"
    ".L8004C5BC:\n"
    "    lw   $t0,436($s3)\n"
    "    lw   $t1,440($s3)\n"
    "    lw   $t2,444($s3)\n"
    "    lw   $t3,8($s1)\n"
    "    addu   $t4,$zero,$zero\n"
    "    lw   $t5,12($s1)\n"
    "    sub   $t3,$t3,$t0\n"
    "    sub   $t4,$t4,$t1\n"
    "    sub   $t5,$t5,$t2\n"
    "    srav   $t3,$t3,$s4\n"
    "    srav   $t4,$t4,$s4\n"
    "    srav   $t5,$t5,$s4\n"
    "    andi   $t3,$t3,0xFFFF\n"
    "    sll   $t4,$t4,16\n"
    "    or   $t3,$t3,$t4\n"
    "    mtc2   $t3,$0\n"
    "    mtc2   $t5,$1\n"
    "    lw   $t0,392($s3)\n"
    "    lw   $t1,396($s3)\n"
    "    lw   $t2,400($s3)\n"
    "    lw   $t3,404($s3)\n"
    "    lw   $t4,408($s3)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    ctc2   $zero,$5\n"
    "    ctc2   $zero,$6\n"
    "    ctc2   $zero,$7\n"
    "    nop\n"
    "    nop\n"
    "    mvmva   1,0,0,0,0\n"
    "    nop\n"
    "    nop\n"
    "    mfc2   $t0,$9\n"
    "    mfc2   $t1,$10\n"
    "    mfc2   $t2,$11\n"
    "    sra   $t1,$t1,1\n"
    "    ctc2   $t0,$5\n"
    "    ctc2   $t1,$6\n"
    "    ctc2   $t2,$7\n"
    "    lw   $t0,412($s3)\n"
    "    lw   $t1,416($s3)\n"
    "    lw   $t2,420($s3)\n"
    "    lw   $t3,424($s3)\n"
    "    lw   $t4,428($s3)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    lh   $t0,2($s1)\n"
    "    lh   $t1,4($s1)\n"
    "    sll   $t0,$t0,2\n"
    "    lui   $t2,%hi(D_80103608)\n"
    "    addiu   $t2,$t2,%lo(D_80103608)\n"
    "    addu   $t0,$t0,$t2\n"
    "    lw   $t0,0($t0)\n"
    "    sll   $t1,$t1,2\n"
    "    addu   $t0,$t0,$t1\n"
    "    lw   $s0,0($t0)\n"
    "    lh   $s7,6($s1)\n"
    "    nop\n"
    "    lw   $a1,0($s0)\n"
    "    nop\n"
    "    andi   $a0,$a1,0xFFFF\n"
    "    srl   $a1,$a1,16\n"
    "    beqz   $a0,.L8004C930\n"
    "    addi   $s0,$s0,0x4\n"
    "    sw   $s0,432($s3)\n"
    "    addiu   $t0,$zero,0x4\n"
    "    bne   $t0,$a0,.L8004C7A8\n"
    "    nop\n"
    "    lhu   $t6,0($s0)\n"
    "    lhu   $t7,2($s0)\n"
    "    lhu   $t8,4($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$0\n"
    "    mtc2   $t8,$1\n"
    "    lhu   $t6,6($s0)\n"
    "    lhu   $t7,8($s0)\n"
    "    lhu   $t8,10($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$2\n"
    "    mtc2   $t8,$3\n"
    "    lhu   $t6,12($s0)\n"
    "    lhu   $t7,14($s0)\n"
    "    lhu   $t8,16($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$4\n"
    "    mtc2   $t8,$5\n"
    "    lhu   $t6,18($s0)\n"
    "    lhu   $t7,20($s0)\n"
    "    rtpt\n"
    "    lhu   $t8,22($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    addi   $s0,$s0,0x18\n"
    "    addiu   $s5,$s3,%lo(D_1F800020)\n"
    "    addiu   $s6,$s3,%lo(D_1F800110)\n"
    "    swc2   $12,0($s5)\n"
    "    swc2   $13,4($s5)\n"
    "    swc2   $14,8($s5)\n"
    "    mfc2   $t0,$17\n"
    "    mfc2   $t1,$18\n"
    "    mfc2   $t2,$19\n"
    "    mtc2   $t6,$0\n"
    "    mtc2   $t8,$1\n"
    "    nop\n"
    "    nop\n"
    "    rtps\n"
    "    sh   $t0,0($s6)\n"
    "    sh   $t1,2($s6)\n"
    "    sh   $t2,4($s6)\n"
    "    mfc2   $t0,$19\n"
    "    swc2   $14,12($s5)\n"
    "    sh   $t0,6($s6)\n"
    "    bgez   $zero,.L8004C8D0\n"
    "    nop\n"
    ".L8004C7A8:\n"
    "    addiu   $a0,$a0,-0x3\n"
    "    lhu   $t6,0($s0)\n"
    "    lhu   $t7,2($s0)\n"
    "    lhu   $t8,4($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$0\n"
    "    mtc2   $t8,$1\n"
    "    lhu   $t6,6($s0)\n"
    "    lhu   $t7,8($s0)\n"
    "    lhu   $t8,10($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$2\n"
    "    mtc2   $t8,$3\n"
    "    lhu   $t6,12($s0)\n"
    "    lhu   $t7,14($s0)\n"
    "    lhu   $t8,16($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$4\n"
    "    mtc2   $t8,$5\n"
    "    addiu   $s5,$s3,%lo(D_1F800020)\n"
    "    addiu   $s6,$s3,%lo(D_1F800110)\n"
    "    rtpt\n"
    "    j   .L8004C828\n"
    "    addiu   $s0,$s0,0x12\n"
    ".L8004C814:\n"
    "    rtpt\n"
    "    addiu   $s6,$s6,0x6\n"
    "    sh   $t0,-6($s6)\n"
    "    sh   $t1,-4($s6)\n"
    "    sh   $t2,-2($s6)\n"
    ".L8004C828:\n"
    "    swc2   $12,0($s5)\n"
    "    swc2   $13,4($s5)\n"
    "    swc2   $14,8($s5)\n"
    "    mfc2   $t0,$17\n"
    "    mfc2   $t1,$18\n"
    "    mfc2   $t2,$19\n"
    "    addiu   $s5,$s5,0xC\n"
    "    blez   $a0,.L8004C8AC\n"
    "    nop\n"
    "    addiu   $a0,$a0,-0x3\n"
    "    lhu   $t6,0($s0)\n"
    "    lhu   $t7,2($s0)\n"
    "    lhu   $t8,4($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$0\n"
    "    mtc2   $t8,$1\n"
    "    lhu   $t6,6($s0)\n"
    "    lhu   $t7,8($s0)\n"
    "    lhu   $t8,10($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$2\n"
    "    mtc2   $t8,$3\n"
    "    lhu   $t6,12($s0)\n"
    "    lhu   $t7,14($s0)\n"
    "    lhu   $t8,16($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$4\n"
    "    mtc2   $t8,$5\n"
    "    j   .L8004C814\n"
    "    addiu   $s0,$s0,0x12\n"
    ".L8004C8AC:\n"
    "    sh   $t0,0($s6)\n"
    "    sh   $t1,2($s6)\n"
    "    sh   $t2,4($s6)\n"
    "    add   $a0,$a0,$a0\n"
    "    add   $t0,$a0,$a0\n"
    "    add   $a0,$t0,$a0\n"
    "    add   $s0,$s0,$a0\n"
    "    andi   $t0,$s0,0x3\n"
    "    add   $s0,$s0,$t0\n"
    ".L8004C8D0:\n"
    "    lhu   $a0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    ".L8004C8D8:\n"
    "    lhu   $t0,0($s0)\n"
    "    lw   $a1,432($s3)\n"
    "    addiu   $s0,$s0,0x2\n"
    "    srl   $t0,$t0,3\n"
    "    sll   $t0,$t0,2\n"
    "    lui   $at,%hi(D_80015470)\n"
    "    addu   $at,$at,$t0\n"
    "    lw   $t0,%lo(D_80015470)($at)\n"
    "    addu   $a2,$s0,$zero\n"
    "    jalr   $t0\n"
    "    addu   $a3,$s7,$zero\n"
    "    addu   $s0,$v0,$zero\n"
    "    lhu   $a0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    "    bnez   $a0,.L8004C8D8\n"
    "    nop\n"
    "    lhu   $t0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    "    beqz   $t0,.L8004C930\n"
    "    nop\n"
    "    j   func_80052C10\n"
    "    nop\n"
    ".L8004C930:\n"
    "    lui   $v0,%hi(D_800A3820)\n"
    "    lw   $v0,%lo(D_800A3820)($v0)\n"
    "    nop\n"
    "    beq   $v0,$s2,.L8004C964\n"
    "    nop\n"
    "    lw   $s1,0($s2)\n"
    "    nop\n"
    "    lh   $v0,0($s1)\n"
    "    nop\n"
    "    addiu   $at,$zero,0xC\n"
    "    beq   $v0,$at,.L8004C5BC\n"
    "    addi   $s2,$s2,0x4\n"
    "    addi   $s2,$s2,-0x4\n"
    ".L8004C964:\n"
    "    addu   $v0,$s2,$zero\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004C994\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004CB5C\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004C9F4:\n"
    "    lw   $a0,12($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004CB50\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bnez   $t6,.L8004CB50\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L8004CB50\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004CB50\n"
    "    nop\n"
    "    lw   $t0,16($s0)\n"
    "    lw   $t1,20($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    mtc2   $t0,$0\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t1,$1\n"
    "    addiu   $at,$zero,-0x50\n"
    "    xor   $t0,$a3,$at\n"
    "    add   $v0,$v0,$t0\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    ncs\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    swc2   $22,4($s6)\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x20\n"
    ".L8004CB50:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004C9F4\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004CB5C:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004CB8C\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004CD80\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004CBEC:\n"
    "    lhu   $a0,14($s0)\n"
    "    lhu   $a1,16($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004CD74\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bnez   $t8,.L8004CD74\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $t3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$t3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004CD74\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004CD74\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    lhu   $t0,18($s0)\n"
    "    lw   $t1,20($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    mtc2   $t0,$1\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t1,$0\n"
    "    addiu   $at,$zero,-0x50\n"
    "    xor   $t0,$a3,$at\n"
    "    add   $v0,$v0,$t0\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    ncs\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    swc2   $22,4($s6)\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004CD74:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004CBEC\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004CD80:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004CDB0\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004CFB0\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004CE14:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004CFA4\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bnez   $t6,.L8004CFA4\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,20($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,32($s6)\n"
    "    blez   $v0,.L8004CFA4\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004CFA4\n"
    "    nop\n"
    "    lw   $t0,16($s0)\n"
    "    lw   $t1,20($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t1,$1\n"
    "    lw   $t2,24($s0)\n"
    "    srl   $t1,$t1,16\n"
    "    xor   $t2,$t2,$a3\n"
    "    mtc2   $t2,$2\n"
    "    mtc2   $t1,$3\n"
    "    lw   $t0,28($s0)\n"
    "    lw   $t1,32($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t0,$4\n"
    "    mtc2   $t1,$5\n"
    "    addiu   $at,$zero,-0x50\n"
    "    xor   $t0,$a3,$at\n"
    "    add   $v0,$v0,$t0\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    nct\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,4($s6)\n"
    "    swc2   $21,16($s6)\n"
    "    swc2   $22,28($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004CFA4:\n"
    "    addiu   $s0,$s0,0x24\n"
    "    bnez   $s3,.L8004CE14\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004CFB0:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004CFE0\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004D214\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0xC00\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004D044:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004D208\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bnez   $t8,.L8004D208\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $t3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$t3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004D208\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004D208\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,44($s6)\n"
    "    lw   $t2,20($s0)\n"
    "    lw   $t1,24($s0)\n"
    "    xor   $t2,$t2,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t2,$0\n"
    "    mtc2   $t1,$1\n"
    "    addiu   $at,$zero,-0x50\n"
    "    xor   $t0,$a3,$at\n"
    "    add   $v0,$v0,$t0\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    swc2   $22,4($s6)\n"
    "    lw   $t0,28($s0)\n"
    "    srl   $t1,$t1,16\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t1,$1\n"
    "    lw   $t2,32($s0)\n"
    "    lw   $t1,36($s0)\n"
    "    xor   $t2,$t2,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t2,$2\n"
    "    mtc2   $t1,$3\n"
    "    lw   $t0,40($s0)\n"
    "    srl   $t1,$t1,16\n"
    "    mtc2   $t0,$4\n"
    "    mtc2   $t1,$5\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    nct\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sh   $t3,48($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,16($s6)\n"
    "    swc2   $21,28($s6)\n"
    "    swc2   $22,40($s6)\n"
    "    addiu   $s6,$s6,0x34\n"
    ".L8004D208:\n"
    "    addiu   $s0,$s0,0x2C\n"
    "    bnez   $s3,.L8004D044\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004D214:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004D244\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004D3F4\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004D2A8:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004D3E8\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L8004D3E8\n"
    "    nop\n"
    "    bnez   $t6,.L8004D3E8\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L8004D3E8\n"
    "    sll   $v0,$v0,2\n"
    "    lwc2   $0,16($s0)\n"
    "    lwc2   $1,20($s0)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    lui   $a0,0x1F80\n"
    "    lw   $a0,24($a0)\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    slt   $at,$v0,$a0\n"
    "    bnez   $at,.L8004D3E8\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    swc2   $22,4($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x20\n"
    ".L8004D3E8:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004D2A8\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004D3F4:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004D424\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004D604\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004D484:\n"
    "    lhu   $a0,14($s0)\n"
    "    lhu   $a1,16($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004D5F8\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8004D5F8\n"
    "    nop\n"
    "    bnez   $t8,.L8004D5F8\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004D5F8\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    lhu   $t0,18($s0)\n"
    "    lwc2   $0,20($s0)\n"
    "    mtc2   $t0,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    lui   $a0,0x1F80\n"
    "    lw   $a0,24($a0)\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    slt   $at,$v0,$a0\n"
    "    bnez   $at,.L8004D5F8\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    swc2   $22,4($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004D5F8:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004D484\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004D604:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004D634\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004D808\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004D698:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004D7FC\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L8004D7FC\n"
    "    nop\n"
    "    bnez   $t6,.L8004D7FC\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,20($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,32($s6)\n"
    "    blez   $v0,.L8004D7FC\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004D7FC\n"
    "    nop\n"
    "    lwc2   $0,16($s0)\n"
    "    lw   $t0,20($s0)\n"
    "    lwc2   $2,24($s0)\n"
    "    mtc2   $t0,$1\n"
    "    lwc2   $4,28($s0)\n"
    "    lwc2   $5,32($s0)\n"
    "    srl   $t0,$t0,16\n"
    "    mtc2   $t0,$3\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    nct\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,4($s6)\n"
    "    swc2   $21,16($s6)\n"
    "    swc2   $22,28($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004D7FC:\n"
    "    addiu   $s0,$s0,0x24\n"
    "    bnez   $s3,.L8004D698\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004D808:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004D838\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004DA44\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0xC00\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004D89C:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004DA38\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8004DA38\n"
    "    nop\n"
    "    bnez   $t8,.L8004DA38\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004DA38\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004DA38\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,44($s6)\n"
    "    lw   $t9,24($s0)\n"
    "    lwc2   $0,20($s0)\n"
    "    mtc2   $t9,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    swc2   $22,4($s6)\n"
    "    lwc2   $0,28($s0)\n"
    "    srl   $t9,$t9,16\n"
    "    mtc2   $t9,$1\n"
    "    lwc2   $2,32($s0)\n"
    "    lw   $t9,36($s0)\n"
    "    lwc2   $4,40($s0)\n"
    "    mtc2   $t9,$3\n"
    "    srl   $t8,$t9,16\n"
    "    mtc2   $t8,$5\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    nct\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sh   $t3,48($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,16($s6)\n"
    "    swc2   $21,28($s6)\n"
    "    swc2   $22,40($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x34\n"
    ".L8004DA38:\n"
    "    addiu   $s0,$s0,0x2C\n"
    "    bnez   $s3,.L8004D89C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004DA44:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004DA74\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    addu   $s1,$a0,$zero\n"
    "    addu   $s5,$a1,$zero\n"
    "    lh   $t0,4($s1)\n"
    "    lh   $t1,2($s1)\n"
    "    sll   $t0,$t0,2\n"
    "    lui   $t2,%hi(D_80103608)\n"
    "    addiu   $t2,$t2,%lo(D_80103608)\n"
    "    addu   $t0,$t0,$t2\n"
    "    lw   $t0,0($t0)\n"
    "    sll   $t1,$t1,2\n"
    "    addu   $t0,$t0,$t1\n"
    "    lw   $s0,0($t0)\n"
    "    nop\n"
    "    lw   $a1,0($s0)\n"
    "    nop\n"
    "    andi   $a0,$a1,0xFFFF\n"
    "    srl   $a1,$a1,16\n"
    "    beqz   $a0,.L8004DD84\n"
    "    addi   $s0,$s0,0x4\n"
    "    add   $t0,$a0,$a0\n"
    "    add   $t0,$t0,$a0\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $s0,$s0,$t0\n"
    "    andi   $t0,$s0,0x3\n"
    "    add   $s0,$s0,$t0\n"
    "    lw   $s2,96($s1)\n"
    "    lui   $v0,%hi(D_800A3708)\n"
    "    lw   $v0,%lo(D_800A3708)($v0)\n"
    "    addiu   $a0,$a0,-0x1\n"
    "    lw   $t3,44($v0)\n"
    "    lw   $t4,48($v0)\n"
    "    lw   $t5,52($v0)\n"
    "    lw   $t0,0($s2)\n"
    "    lw   $t1,4($s2)\n"
    "    lw   $t2,8($s2)\n"
    "    sra   $t0,$t0,7\n"
    "    sra   $t1,$t1,7\n"
    "    sra   $t2,$t2,7\n"
    "    sub   $t0,$t0,$t3\n"
    "    sub   $t1,$t1,$t4\n"
    "    sub   $t2,$t2,$t5\n"
    "    srav   $t0,$t0,$s5\n"
    "    srav   $t1,$t1,$s5\n"
    "    srav   $t2,$t2,$s5\n"
    "    sll   $t1,$t1,16\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t2,$1\n"
    "    lw   $t0,64($s2)\n"
    "    lw   $t1,68($s2)\n"
    "    lw   $t2,72($s2)\n"
    "    sra   $t0,$t0,7\n"
    "    sra   $t1,$t1,7\n"
    "    sra   $t2,$t2,7\n"
    "    sub   $t0,$t0,$t3\n"
    "    sub   $t1,$t1,$t4\n"
    "    sub   $t2,$t2,$t5\n"
    "    srav   $t0,$t0,$s5\n"
    "    srav   $t1,$t1,$s5\n"
    "    srav   $t2,$t2,$s5\n"
    "    sll   $t1,$t1,16\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$2\n"
    "    mtc2   $t2,$3\n"
    "    lw   $t0,128($s2)\n"
    "    lw   $t1,132($s2)\n"
    "    lw   $t2,136($s2)\n"
    "    sra   $t0,$t0,7\n"
    "    sra   $t1,$t1,7\n"
    "    sra   $t2,$t2,7\n"
    "    sub   $t0,$t0,$t3\n"
    "    sub   $t1,$t1,$t4\n"
    "    sub   $t2,$t2,$t5\n"
    "    srav   $t0,$t0,$s5\n"
    "    srav   $t1,$t1,$s5\n"
    "    srav   $t2,$t2,$s5\n"
    "    sll   $t1,$t1,16\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$4\n"
    "    mtc2   $t2,$5\n"
    "    lui   $s3,0x1F80\n"
    "    ori   $s3,$s3,32\n"
    "    lui   $s4,0x1F80\n"
    "    ori   $s4,$s4,692\n"
    "    rtpt\n"
    "    j   .L8004DC18\n"
    "    addi   $s2,$s2,0xC0\n"
    ".L8004DC04:\n"
    "    rtpt\n"
    "    addiu   $s4,$s4,0x6\n"
    "    sh   $t6,-6($s4)\n"
    "    sh   $t7,-4($s4)\n"
    "    sh   $t8,-2($s4)\n"
    ".L8004DC18:\n"
    "    swc2   $12,0($s3)\n"
    "    swc2   $13,4($s3)\n"
    "    swc2   $14,8($s3)\n"
    "    mfc2   $t6,$17\n"
    "    mfc2   $t7,$18\n"
    "    mfc2   $t8,$19\n"
    "    addiu   $s3,$s3,0xC\n"
    "    beqz   $a0,.L8004DD10\n"
    "    addiu   $a0,$a0,-0x1\n"
    "    lw   $t0,0($s2)\n"
    "    lw   $t1,4($s2)\n"
    "    lw   $t2,8($s2)\n"
    "    sra   $t0,$t0,7\n"
    "    sra   $t1,$t1,7\n"
    "    sra   $t2,$t2,7\n"
    "    sub   $t0,$t0,$t3\n"
    "    sub   $t1,$t1,$t4\n"
    "    sub   $t2,$t2,$t5\n"
    "    srav   $t0,$t0,$s5\n"
    "    srav   $t1,$t1,$s5\n"
    "    srav   $t2,$t2,$s5\n"
    "    sll   $t1,$t1,16\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t2,$1\n"
    "    lw   $t0,64($s2)\n"
    "    lw   $t1,68($s2)\n"
    "    lw   $t2,72($s2)\n"
    "    sra   $t0,$t0,7\n"
    "    sra   $t1,$t1,7\n"
    "    sra   $t2,$t2,7\n"
    "    sub   $t0,$t0,$t3\n"
    "    sub   $t1,$t1,$t4\n"
    "    sub   $t2,$t2,$t5\n"
    "    srav   $t0,$t0,$s5\n"
    "    srav   $t1,$t1,$s5\n"
    "    srav   $t2,$t2,$s5\n"
    "    sll   $t1,$t1,16\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$2\n"
    "    mtc2   $t2,$3\n"
    "    lw   $t0,128($s2)\n"
    "    lw   $t1,132($s2)\n"
    "    lw   $t2,136($s2)\n"
    "    sra   $t0,$t0,7\n"
    "    sra   $t1,$t1,7\n"
    "    sra   $t2,$t2,7\n"
    "    sub   $t0,$t0,$t3\n"
    "    sub   $t1,$t1,$t4\n"
    "    sub   $t2,$t2,$t5\n"
    "    srav   $t0,$t0,$s5\n"
    "    srav   $t1,$t1,$s5\n"
    "    srav   $t2,$t2,$s5\n"
    "    sll   $t1,$t1,16\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$4\n"
    "    mtc2   $t2,$5\n"
    "    j   .L8004DC04\n"
    "    addiu   $s2,$s2,0xC0\n"
    ".L8004DD10:\n"
    "    sh   $t6,0($s4)\n"
    "    sh   $t7,2($s4)\n"
    "    sh   $t8,4($s4)\n"
    "    addiu   $t0,$zero,0x3\n"
    "    subu   $t0,$t0,$s5\n"
    "    lui   $at,0x1F80\n"
    "    sw   $t0,8($at)\n"
    "    lhu   $a0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    ".L8004DD34:\n"
    "    lhu   $t0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    "    srl   $t0,$t0,3\n"
    "    sll   $t0,$t0,2\n"
    "    lbu   $t1,1($s1)\n"
    "    lui   $t2,%hi(D_80015480)\n"
    "    addiu   $t2,$t2,%lo(D_80015480)\n"
    "    add   $t0,$t0,$t2\n"
    "    andi   $t1,$t1,0x1\n"
    "    beqz   $t1,.L8004DD64\n"
    "    nop\n"
    "    addi   $t0,$t0,0x10\n"
    ".L8004DD64:\n"
    "    lw   $t0,0($t0)\n"
    "    addu   $a1,$s0,$zero\n"
    "    jalr   $t0\n"
    "    nop\n"
    "    lhu   $a0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    "    bnez   $a0,.L8004DD34\n"
    "    nop\n"
    ".L8004DD84:\n"
    "    addu   $v0,$s1,$zero\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004DDB4\n"
    "    addiu   $sp,$sp,-0x8C\n"
    "    sw   $ra,136($sp)\n"
    "    sw   $fp,132($sp)\n"
    "    sw   $s7,128($sp)\n"
    "    sw   $s6,124($sp)\n"
    "    sw   $s5,120($sp)\n"
    "    sw   $s4,116($sp)\n"
    "    sw   $s3,112($sp)\n"
    "    sw   $s2,108($sp)\n"
    "    sw   $s1,104($sp)\n"
    "    sw   $s0,100($sp)\n"
    "    addu   $fp,$a0,$zero\n"
    "    addu   $s0,$a1,$zero\n"
    "    addu   $s7,$a2,$zero\n"
    "    addu   $s1,$a3,$zero\n"
    "    addiu   $s5,$zero,0xC\n"
    "    lui   $s6,0x1F80\n"
    "    ori   $s6,$s6,32\n"
    "    addu   $s3,$s6,$zero\n"
    "    addi   $s4,$s6,0x224\n"
    "    addi   $a0,$s0,0x2\n"
    "    jal   func_80054440\n"
    "    addu   $a1,$fp,$zero\n"
    "    addi   $s0,$v0,-0x2\n"
    ".L8004DE14:\n"
    "    lui   $a0,%hi(D_800A3708)\n"
    "    lw   $a0,%lo(D_800A3708)($a0)\n"
    "    lw   $s2,0($fp)\n"
    "    lw   $t0,44($a0)\n"
    "    lw   $t1,48($a0)\n"
    "    lw   $t2,52($a0)\n"
    "    lw   $t3,20($s2)\n"
    "    lw   $t4,24($s2)\n"
    "    lw   $t5,28($s2)\n"
    "    sub   $t3,$t3,$t0\n"
    "    sub   $t4,$t4,$t1\n"
    "    sub   $t5,$t5,$t2\n"
    "    srav   $t3,$t3,$s1\n"
    "    srav   $t4,$t4,$s1\n"
    "    srav   $t5,$t5,$s1\n"
    "    sw   $t3,872($s6)\n"
    "    sw   $t4,876($s6)\n"
    "    sw   $t5,880($s6)\n"
    "    addi   $v0,$s6,0x340\n"
    "    lh   $t0,0($s2)\n"
    "    lh   $t1,2($s2)\n"
    "    lh   $t2,4($s2)\n"
    "    lh   $t3,6($s2)\n"
    "    lh   $t4,8($s2)\n"
    "    lh   $t5,10($s2)\n"
    "    lh   $t6,12($s2)\n"
    "    lh   $t7,14($s2)\n"
    "    lh   $t8,16($s2)\n"
    "    srav   $t0,$t0,$s1\n"
    "    srav   $t1,$t1,$s1\n"
    "    srav   $t2,$t2,$s1\n"
    "    srav   $t3,$t3,$s1\n"
    "    srav   $t4,$t4,$s1\n"
    "    srav   $t5,$t5,$s1\n"
    "    srav   $t6,$t6,$s1\n"
    "    srav   $t7,$t7,$s1\n"
    "    srav   $t8,$t8,$s1\n"
    "    sh   $t0,0($v0)\n"
    "    sh   $t1,2($v0)\n"
    "    sh   $t2,4($v0)\n"
    "    sh   $t3,6($v0)\n"
    "    sh   $t4,8($v0)\n"
    "    sh   $t5,10($v0)\n"
    "    sh   $t6,12($v0)\n"
    "    sh   $t7,14($v0)\n"
    "    sh   $t8,16($v0)\n"
    "    addu   $s2,$v0,$zero\n"
    "    addi   $fp,$fp,0x4\n"
    "    addu   $a0,$s7,$zero\n"
    "    addu   $a1,$s2,$zero\n"
    "    jal   func_80052930\n"
    "    addi   $a2,$s6,0x354\n"
    "    lw   $t0,0($s7)\n"
    "    lw   $t1,4($s7)\n"
    "    lw   $t2,8($s7)\n"
    "    lw   $t3,12($s7)\n"
    "    lw   $t4,16($s7)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    ctc2   $zero,$5\n"
    "    ctc2   $zero,$6\n"
    "    ctc2   $zero,$7\n"
    "    lw   $t0,872($s6)\n"
    "    lw   $t1,876($s6)\n"
    "    lw   $t2,880($s6)\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    nop\n"
    "    nop\n"
    "    mvmva   1,0,0,0,0\n"
    "    nop\n"
    "    nop\n"
    "    swc2   $25,884($s6)\n"
    "    swc2   $26,888($s6)\n"
    "    swc2   $27,892($s6)\n"
    "    nop\n"
    "    addi   $s0,$s0,0x2\n"
    "    lh   $v1,0($s0)\n"
    "    nop\n"
    "    beqz   $v1,.L8004E01C\n"
    "    addi   $s0,$s0,0x2\n"
    "    addi   $v1,$v1,-0x1\n"
    "    lw   $t0,0($s2)\n"
    "    lw   $t1,4($s2)\n"
    "    lw   $t2,8($s2)\n"
    "    lw   $t3,12($s2)\n"
    "    lw   $t4,16($s2)\n"
    "    lw   $t5,872($s6)\n"
    "    lw   $t6,876($s6)\n"
    "    lw   $t7,880($s6)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    ctc2   $t5,$5\n"
    "    ctc2   $t6,$6\n"
    "    ctc2   $t7,$7\n"
    ".L8004DFB4:\n"
    "    lhu   $t0,0($s0)\n"
    "    lh   $t1,2($s0)\n"
    "    lh   $t2,4($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    lh   $t0,6($s0)\n"
    "    nop\n"
    "    mvmva   1,0,0,0,0\n"
    "    lh   $t1,8($s0)\n"
    "    lh   $t2,10($s0)\n"
    "    addi   $s0,$s0,0xC\n"
    "    addi   $s3,$s3,0x4\n"
    "    addi   $s4,$s4,0x2\n"
    "    mfc2   $t3,$25\n"
    "    mfc2   $t4,$26\n"
    "    mfc2   $t5,$27\n"
    "    add   $t0,$t0,$t3\n"
    "    add   $t1,$t1,$t4\n"
    "    add   $t2,$t2,$t5\n"
    "    sh   $t0,-4($s3)\n"
    "    sh   $t1,-2($s3)\n"
    "    sh   $t2,-2($s4)\n"
    "    bnez   $v1,.L8004DFB4\n"
    "    addi   $v1,$v1,-0x1\n"
    ".L8004E01C:\n"
    "    lh   $v1,0($s0)\n"
    "    nop\n"
    "    beqz   $v1,.L8004E140\n"
    "    addi   $s0,$s0,0x2\n"
    "    addi   $v1,$v1,-0x1\n"
    ".L8004E030:\n"
    "    lhu   $t0,2($s0)\n"
    "    lh   $t1,4($s0)\n"
    "    lh   $t2,6($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    lh   $t0,0($s0)\n"
    "    lh   $t3,8($s0)\n"
    "    lh   $t4,10($s0)\n"
    "    lh   $t5,12($s0)\n"
    "    add   $t0,$t0,$t0\n"
    "    addi   $t9,$s6,0x224\n"
    "    add   $t9,$t9,$t0\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $t8,$s6,$t0\n"
    "    lh   $t0,0($t8)\n"
    "    lh   $t1,2($t8)\n"
    "    lh   $t2,0($t9)\n"
    "    add   $t0,$t0,$t3\n"
    "    add   $t1,$t1,$t4\n"
    "    add   $t2,$t2,$t5\n"
    "    lw   $t3,0($s2)\n"
    "    lw   $t4,4($s2)\n"
    "    lw   $t5,8($s2)\n"
    "    lw   $t6,12($s2)\n"
    "    lw   $t7,16($s2)\n"
    "    ctc2   $t3,$0\n"
    "    ctc2   $t4,$1\n"
    "    ctc2   $t5,$2\n"
    "    ctc2   $t6,$3\n"
    "    ctc2   $t7,$4\n"
    "    ctc2   $t0,$5\n"
    "    ctc2   $t1,$6\n"
    "    ctc2   $t2,$7\n"
    "    nop\n"
    "    nop\n"
    "    mvmva   1,0,0,0,0\n"
    "    lw   $t0,0($s7)\n"
    "    lw   $t1,4($s7)\n"
    "    lw   $t2,8($s7)\n"
    "    lw   $t3,12($s7)\n"
    "    lw   $t4,16($s7)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    ctc2   $zero,$5\n"
    "    ctc2   $zero,$6\n"
    "    ctc2   $zero,$7\n"
    "    mfc2   $t0,$25\n"
    "    mfc2   $t1,$26\n"
    "    mfc2   $t2,$27\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    nop\n"
    "    nop\n"
    "    rtps\n"
    "    addi   $s0,$s0,0xE\n"
    "    mfc2   $t0,$19\n"
    "    swc2   $14,0($t8)\n"
    "    sh   $t0,0($t9)\n"
    "    bnez   $v1,.L8004E030\n"
    "    addi   $v1,$v1,-0x1\n"
    ".L8004E140:\n"
    "    lh   $v1,0($s0)\n"
    "    nop\n"
    "    beqz   $v1,.L8004E1DC\n"
    "    addi   $s0,$s0,0x2\n"
    "    addi   $v1,$v1,-0x1\n"
    "    lw   $t0,852($s6)\n"
    "    lw   $t1,856($s6)\n"
    "    lw   $t2,860($s6)\n"
    "    lw   $t3,864($s6)\n"
    "    lw   $t4,868($s6)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    ctc2   $zero,$5\n"
    "    ctc2   $zero,$6\n"
    "    ctc2   $zero,$7\n"
    ".L8004E188:\n"
    "    lhu   $t0,0($s0)\n"
    "    lh   $t1,2($s0)\n"
    "    lh   $t2,4($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    nop\n"
    "    nop\n"
    "    mvmva   1,0,0,0,0\n"
    "    addi   $s0,$s0,0x6\n"
    "    addi   $s3,$s3,0x4\n"
    "    addi   $s4,$s4,0x2\n"
    "    mfc2   $t0,$25\n"
    "    mfc2   $t1,$26\n"
    "    mfc2   $t2,$27\n"
    "    sh   $t0,-4($s3)\n"
    "    sh   $t1,-2($s3)\n"
    "    sh   $t2,-2($s4)\n"
    "    bnez   $v1,.L8004E188\n"
    "    addi   $v1,$v1,-0x1\n"
    ".L8004E1DC:\n"
    "    lh   $v1,0($s0)\n"
    "    nop\n"
    "    beqz   $v1,.L8004E2A0\n"
    "    addi   $s0,$s0,0x2\n"
    "    lw   $t0,852($s6)\n"
    "    lw   $t1,856($s6)\n"
    "    lw   $t2,860($s6)\n"
    "    lw   $t3,864($s6)\n"
    "    lw   $t4,868($s6)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    lw   $t3,884($s6)\n"
    "    lw   $t4,888($s6)\n"
    "    lw   $t5,892($s6)\n"
    "    addi   $v1,$v1,-0x1\n"
    ".L8004E224:\n"
    "    lhu   $t0,2($s0)\n"
    "    lh   $t1,4($s0)\n"
    "    lh   $t2,6($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    lh   $t0,0($s0)\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    add   $t0,$t0,$t0\n"
    "    addi   $t9,$s6,0x224\n"
    "    add   $t9,$t9,$t0\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $t8,$s6,$t0\n"
    "    lh   $t0,0($t8)\n"
    "    lh   $t1,2($t8)\n"
    "    lh   $t2,0($t9)\n"
    "    add   $t0,$t0,$t3\n"
    "    add   $t1,$t1,$t4\n"
    "    add   $t2,$t2,$t5\n"
    "    ctc2   $t0,$5\n"
    "    ctc2   $t1,$6\n"
    "    ctc2   $t2,$7\n"
    "    nop\n"
    "    nop\n"
    "    rtps\n"
    "    addi   $s0,$s0,0x8\n"
    "    mfc2   $t0,$19\n"
    "    swc2   $14,0($t8)\n"
    "    sh   $t0,0($t9)\n"
    "    bnez   $v1,.L8004E224\n"
    "    addi   $v1,$v1,-0x1\n"
    ".L8004E2A0:\n"
    "    lh   $v1,0($s0)\n"
    "    nop\n"
    "    beqz   $v1,.L8004E428\n"
    "    addi   $s0,$s0,0x2\n"
    "    lw   $t0,852($s6)\n"
    "    lw   $t1,856($s6)\n"
    "    lw   $t2,860($s6)\n"
    "    lw   $t3,864($s6)\n"
    "    lw   $t4,868($s6)\n"
    "    lw   $t5,884($s6)\n"
    "    lw   $t6,888($s6)\n"
    "    lw   $t7,892($s6)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    ctc2   $t5,$5\n"
    "    ctc2   $t6,$6\n"
    "    ctc2   $t7,$7\n"
    "    addiu   $v1,$v1,-0x3\n"
    "    lhu   $t0,0($s0)\n"
    "    lh   $t1,2($s0)\n"
    "    lh   $t2,4($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    lhu   $t0,6($s0)\n"
    "    lh   $t1,8($s0)\n"
    "    lh   $t2,10($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$2\n"
    "    mtc2   $t2,$3\n"
    "    lhu   $t0,12($s0)\n"
    "    lh   $t1,14($s0)\n"
    "    lh   $t2,16($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$4\n"
    "    mtc2   $t2,$5\n"
    "    nop\n"
    "    nop\n"
    "    rtpt\n"
    "    j   .L8004E370\n"
    "    addiu   $s0,$s0,0x12\n"
    ".L8004E35C:\n"
    "    rtpt\n"
    "    sh   $t0,0($s4)\n"
    "    sh   $t1,2($s4)\n"
    "    sh   $t2,4($s4)\n"
    "    addiu   $s4,$s4,0x6\n"
    ".L8004E370:\n"
    "    blez   $v1,.L8004E3F4\n"
    "    nop\n"
    "    addiu   $v1,$v1,-0x3\n"
    "    lhu   $t0,0($s0)\n"
    "    lh   $t2,2($s0)\n"
    "    lh   $t1,4($s0)\n"
    "    sll   $t2,$t2,16\n"
    "    or   $t0,$t0,$t2\n"
    "    lhu   $t2,6($s0)\n"
    "    lh   $t4,8($s0)\n"
    "    lh   $t3,10($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    or   $t2,$t2,$t4\n"
    "    lhu   $t4,12($s0)\n"
    "    lh   $t6,14($s0)\n"
    "    lh   $t5,16($s0)\n"
    "    sll   $t6,$t6,16\n"
    "    or   $t4,$t4,$t6\n"
    "    swc2   $12,0($s3)\n"
    "    swc2   $13,4($s3)\n"
    "    swc2   $14,8($s3)\n"
    "    addiu   $s3,$s3,0xC\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t1,$1\n"
    "    mtc2   $t2,$2\n"
    "    mtc2   $t3,$3\n"
    "    mtc2   $t4,$4\n"
    "    mtc2   $t5,$5\n"
    "    mfc2   $t0,$17\n"
    "    mfc2   $t1,$18\n"
    "    mfc2   $t2,$19\n"
    "    j   .L8004E35C\n"
    "    addiu   $s0,$s0,0x12\n"
    ".L8004E3F4:\n"
    "    swc2   $12,0($s3)\n"
    "    swc2   $13,4($s3)\n"
    "    swc2   $14,8($s3)\n"
    "    mfc2   $t0,$17\n"
    "    mfc2   $t1,$18\n"
    "    mfc2   $t2,$19\n"
    "    sh   $t0,0($s4)\n"
    "    sh   $t1,2($s4)\n"
    "    sh   $t2,4($s4)\n"
    "    add   $v1,$v1,$v1\n"
    "    add   $t0,$v1,$v1\n"
    "    add   $v1,$t0,$v1\n"
    "    add   $s0,$s0,$v1\n"
    ".L8004E428:\n"
    "    addi   $s0,$s0,0x2\n"
    "    addiu   $t1,$zero,0x4\n"
    "    andi   $t0,$s0,0x3\n"
    "    sub   $t0,$t1,$t0\n"
    "    andi   $t0,$s0,0x3\n"
    "    add   $s0,$s0,$t0\n"
    "    addi   $s4,$s6,-0x20\n"
    "    lw   $s3,28($s4)\n"
    ".L8004E448:\n"
    "    lh   $a0,0($s0)\n"
    "    addi   $s0,$s0,0x2\n"
    "    beqz   $a0,.L8004E480\n"
    "    lhu   $t0,0($s0)\n"
    "    addi   $s0,$s0,0x2\n"
    "    sll   $t0,$t0,2\n"
    "    add   $t0,$t0,$s3\n"
    "    lw   $t0,0($t0)\n"
    "    nop\n"
    "    jalr   $t0\n"
    "    nop\n"
    "    nop\n"
    "    j   .L8004E448\n"
    "    nop\n"
    ".L8004E480:\n"
    "    beqz   $s5,.L8004E52C\n"
    "    ori   $s2,$zero,0xFFFF\n"
    ".L8004E488:\n"
    "    lhu   $t0,0($s0)\n"
    "    lhu   $t1,2($s0)\n"
    "    beq   $t0,$s2,.L8004E500\n"
    "    addu   $t4,$s6,$zero\n"
    "    addi   $t6,$s6,0x224\n"
    "    lh   $t3,4($s0)\n"
    "    addu   $t5,$t4,$zero\n"
    "    addu   $t7,$t6,$zero\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $t1,$t1,$t1\n"
    "    add   $t6,$t6,$t0\n"
    "    add   $t7,$t7,$t1\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $t1,$t1,$t1\n"
    "    add   $t4,$t4,$t0\n"
    "    add   $t5,$t5,$t1\n"
    "    addi   $s0,$s0,0x6\n"
    "    addi   $t3,$t3,-0x1\n"
    ".L8004E4D0:\n"
    "    lw   $t0,0($t4)\n"
    "    lh   $t1,0($t6)\n"
    "    sw   $t0,0($t5)\n"
    "    sh   $t1,0($t7)\n"
    "    addi   $t4,$t4,-0x4\n"
    "    addi   $t5,$t5,0x4\n"
    "    addi   $t6,$t6,-0x2\n"
    "    addi   $t7,$t7,0x2\n"
    "    bnez   $t3,.L8004E4D0\n"
    "    addi   $t3,$t3,-0x1\n"
    "    j   .L8004E488\n"
    "    nop\n"
    ".L8004E500:\n"
    "    addi   $s0,$s0,0x2\n"
    "    lh   $t0,0($s0)\n"
    "    addu   $s3,$s6,$zero\n"
    "    addi   $s4,$s6,0x224\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $s4,$s4,$t0\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $s3,$s3,$t0\n"
    "    addi   $s0,$s0,0x2\n"
    "    bgez   $zero,.L8004DE14\n"
    "    addi   $s5,$s5,-0x1\n"
    ".L8004E52C:\n"
    "    addu   $v0,$s0,$zero\n"
    "    lw   $ra,136($sp)\n"
    "    lw   $fp,132($sp)\n"
    "    lw   $s7,128($sp)\n"
    "    lw   $s6,124($sp)\n"
    "    lw   $s5,120($sp)\n"
    "    lw   $s4,116($sp)\n"
    "    lw   $s3,112($sp)\n"
    "    lw   $s2,108($sp)\n"
    "    lw   $s1,104($sp)\n"
    "    lw   $s0,100($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x8C\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_8004DDB4 */
void func_8004E564(void) {
}
void func_8004E56C(void) {
}
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004E574\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004E7B0\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004E5D8:\n"
    "    lw   $a0,12($s0)\n"
    "    lwc2   $6,8($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004E7A4\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L8004E7A4\n"
    "    nop\n"
    "    bnez   $t6,.L8004E7A4\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L8004E7A4\n"
    "    sll   $v0,$v0,2\n"
    "    lw   $a0,16($s0)\n"
    "    addiu   $t0,$zero,0x555\n"
    "    mtc2   $t0,$8\n"
    "    lui   $t0,%hi(D_800F2B70)\n"
    "    addiu   $t0,$t0,%lo(D_800F2B70)\n"
    "    srl   $t4,$a0,18\n"
    "    andi   $t4,$t4,0xFFC\n"
    "    add   $t4,$t4,$t0\n"
    "    lw   $t4,0($t4)\n"
    "    nop\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    srl   $t4,$a0,8\n"
    "    andi   $t4,$t4,0xFFC\n"
    "    add   $t4,$t4,$t0\n"
    "    lw   $t4,0($t4)\n"
    "    gpf   1\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    sll   $t4,$a0,2\n"
    "    andi   $t4,$t4,0xFFC\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    add   $t4,$t4,$t0\n"
    "    lw   $t4,0($t4)\n"
    "    gpl   1\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    gpl   1\n"
    "    lw   $t0,0($s0)\n"
    "    lw   $t1,4($s0)\n"
    "    lhu   $t2,8($s0)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    "    addiu   $s6,$s6,0x20\n"
    ".L8004E7A4:\n"
    "    addiu   $s0,$s0,0x14\n"
    "    bnez   $s3,.L8004E5D8\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004E7B0:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004E7E4\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004EA94\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004E848:\n"
    "    lw   $a0,16($s0)\n"
    "    lwc2   $6,8($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004EA88\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8004EA88\n"
    "    nop\n"
    "    bnez   $t8,.L8004EA88\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004EA88\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    lw   $a2,20($s0)\n"
    "    lh   $a3,14($s0)\n"
    "    addu   $a0,$a2,$zero\n"
    "    andi   $a0,$a0,0x3FF\n"
    "    srl   $a2,$a2,10\n"
    "    addu   $a1,$a2,$zero\n"
    "    andi   $a1,$a1,0x3FF\n"
    "    srl   $a2,$a2,10\n"
    "    andi   $a2,$a2,0x3FF\n"
    "    addiu   $t0,$zero,0x400\n"
    "    mtc2   $t0,$8\n"
    "    lui   $t0,%hi(D_800F2B70)\n"
    "    addiu   $t0,$t0,%lo(D_800F2B70)\n"
    "    sll   $a0,$a0,2\n"
    "    add   $a0,$a0,$t0\n"
    "    lw   $t4,0($a0)\n"
    "    nop\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    sll   $a1,$a1,2\n"
    "    add   $a1,$a1,$t0\n"
    "    lw   $t4,0($a1)\n"
    "    gpf   1\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    sll   $a2,$a2,2\n"
    "    add   $a2,$a2,$t0\n"
    "    lw   $t4,0($a2)\n"
    "    gpl   1\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    sll   $a3,$a3,2\n"
    "    add   $a3,$a3,$t0\n"
    "    lw   $t4,0($a3)\n"
    "    gpl   1\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    nop\n"
    "    nop\n"
    "    gpl   1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lw   $t0,0($s0)\n"
    "    lw   $t1,4($s0)\n"
    "    lh   $t2,8($s0)\n"
    "    lh   $t3,12($s0)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004EA88:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004E848\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004EA94:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004EAC8\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004EC94\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004EB2C:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004EC88\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L8004EC88\n"
    "    nop\n"
    "    bnez   $t6,.L8004EC88\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,20($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,32($s6)\n"
    "    blez   $v0,.L8004EC88\n"
    "    sll   $v0,$v0,2\n"
    "    lw   $a0,16($s0)\n"
    "    lui   $t0,%hi(D_800F2B70)\n"
    "    addiu   $t0,$t0,%lo(D_800F2B70)\n"
    "    srl   $t1,$a0,18\n"
    "    srl   $t2,$a0,8\n"
    "    sll   $a0,$a0,2\n"
    "    andi   $t1,$t1,0xFFC\n"
    "    andi   $t2,$t2,0xFFC\n"
    "    andi   $a0,$a0,0xFFC\n"
    "    add   $t1,$t1,$t0\n"
    "    add   $t2,$t2,$t0\n"
    "    add   $a0,$a0,$t0\n"
    "    lw   $t0,0($t1)\n"
    "    lw   $t1,0($t2)\n"
    "    lw   $t2,0($a0)\n"
    "    lui   $a0,0x3400\n"
    "    or   $t0,$t0,$a0\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,28($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lw   $t0,0($s0)\n"
    "    lw   $t1,4($s0)\n"
    "    lh   $t2,8($s0)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004EC88:\n"
    "    addiu   $s0,$s0,0x14\n"
    "    bnez   $s3,.L8004EB2C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004EC94:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004ECC8\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004EEDC\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0xC00\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004ED2C:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004EED0\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8004EED0\n"
    "    nop\n"
    "    bnez   $t8,.L8004EED0\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004EED0\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,44($s6)\n"
    "    lw   $a2,20($s0)\n"
    "    lh   $a3,14($s0)\n"
    "    addu   $a0,$a2,$zero\n"
    "    andi   $a0,$a0,0x3FF\n"
    "    srl   $a2,$a2,10\n"
    "    addu   $a1,$a2,$zero\n"
    "    andi   $a1,$a1,0x3FF\n"
    "    srl   $a2,$a2,10\n"
    "    andi   $a2,$a2,0x3FF\n"
    "    lui   $t0,%hi(D_800F2B70)\n"
    "    addiu   $t0,$t0,%lo(D_800F2B70)\n"
    "    sll   $a0,$a0,2\n"
    "    sll   $a1,$a1,2\n"
    "    sll   $a2,$a2,2\n"
    "    sll   $a3,$a3,2\n"
    "    add   $a0,$a0,$t0\n"
    "    add   $a1,$a1,$t0\n"
    "    add   $a2,$a2,$t0\n"
    "    add   $a3,$a3,$t0\n"
    "    lw   $t0,0($a0)\n"
    "    lw   $t1,0($a1)\n"
    "    lw   $t2,0($a2)\n"
    "    lw   $t3,0($a3)\n"
    "    lui   $a0,0x3C00\n"
    "    or   $t0,$t0,$a0\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,28($s6)\n"
    "    sw   $t3,40($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lw   $t0,0($s0)\n"
    "    lw   $t1,4($s0)\n"
    "    lh   $t2,8($s0)\n"
    "    lh   $t3,12($s0)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sh   $t3,48($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x34\n"
    ".L8004EED0:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004ED2C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004EEDC:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004EF10\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004F0CC\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004EF70:\n"
    "    lw   $a0,12($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004F0C0\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bnez   $t6,.L8004F0C0\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L8004F0C0\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004F0C0\n"
    "    nop\n"
    "    lw   $t0,16($s0)\n"
    "    lw   $t1,20($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    mtc2   $t0,$0\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t1,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    ncs\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    swc2   $22,4($s6)\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x20\n"
    ".L8004F0C0:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004EF70\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004F0CC:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004F0FC\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004F2E4\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004F15C:\n"
    "    lhu   $a0,14($s0)\n"
    "    lhu   $a1,16($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004F2D8\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bnez   $t8,.L8004F2D8\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $t3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$t3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004F2D8\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004F2D8\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    lhu   $t0,18($s0)\n"
    "    lw   $t1,20($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    mtc2   $t0,$1\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t1,$0\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    ncs\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    swc2   $22,4($s6)\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004F2D8:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004F15C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004F2E4:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004F314\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004F50C\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004F378:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004F500\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bnez   $t6,.L8004F500\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,20($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,32($s6)\n"
    "    blez   $v0,.L8004F500\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004F500\n"
    "    nop\n"
    "    lw   $t0,16($s0)\n"
    "    lw   $t1,20($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t1,$1\n"
    "    lw   $t2,24($s0)\n"
    "    srl   $t1,$t1,16\n"
    "    xor   $t2,$t2,$a3\n"
    "    mtc2   $t2,$2\n"
    "    mtc2   $t1,$3\n"
    "    lw   $t0,28($s0)\n"
    "    lw   $t1,32($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t0,$4\n"
    "    mtc2   $t1,$5\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    nct\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,4($s6)\n"
    "    swc2   $21,16($s6)\n"
    "    swc2   $22,28($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004F500:\n"
    "    addiu   $s0,$s0,0x24\n"
    "    bnez   $s3,.L8004F378\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004F50C:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004F53C\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004F768\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0xC00\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004F5A0:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004F75C\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bnez   $t8,.L8004F75C\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $t3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$t3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004F75C\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004F75C\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,44($s6)\n"
    "    lw   $t2,20($s0)\n"
    "    lw   $t1,24($s0)\n"
    "    xor   $t2,$t2,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t2,$0\n"
    "    mtc2   $t1,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    swc2   $22,4($s6)\n"
    "    lw   $t0,28($s0)\n"
    "    srl   $t1,$t1,16\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t1,$1\n"
    "    lw   $t2,32($s0)\n"
    "    lw   $t1,36($s0)\n"
    "    xor   $t2,$t2,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t2,$2\n"
    "    mtc2   $t1,$3\n"
    "    lw   $t0,40($s0)\n"
    "    srl   $t1,$t1,16\n"
    "    mtc2   $t0,$4\n"
    "    mtc2   $t1,$5\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    nct\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sh   $t3,48($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,16($s6)\n"
    "    swc2   $21,28($s6)\n"
    "    swc2   $22,40($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x34\n"
    ".L8004F75C:\n"
    "    addiu   $s0,$s0,0x2C\n"
    "    bnez   $s3,.L8004F5A0\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004F768:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004F798\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004F940\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004F7F8:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004F934\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L8004F934\n"
    "    nop\n"
    "    bnez   $t6,.L8004F934\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L8004F934\n"
    "    sll   $v0,$v0,2\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    lui   $a0,0x1F80\n"
    "    lw   $a0,24($a0)\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    slt   $at,$v0,$a0\n"
    "    bnez   $at,.L8004F934\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    lui   $t4,0x2580\n"
    "    ori   $t4,$t4,32896\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sw   $t4,4($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x20\n"
    ".L8004F934:\n"
    "    addiu   $s0,$s0,0x10\n"
    "    bnez   $s3,.L8004F7F8\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004F940:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004F970\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004FB44\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004F9D0:\n"
    "    lhu   $a1,16($s0)\n"
    "    lhu   $a0,14($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004FB38\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8004FB38\n"
    "    nop\n"
    "    bnez   $t8,.L8004FB38\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004FB38\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    lui   $a0,0x1F80\n"
    "    lw   $a0,24($a0)\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    slt   $at,$v0,$a0\n"
    "    bnez   $at,.L8004FB38\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    lui   $t4,0x2D80\n"
    "    ori   $t4,$t4,32896\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    sw   $t4,4($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004FB38:\n"
    "    addiu   $s0,$s0,0x14\n"
    "    bnez   $s3,.L8004F9D0\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004FB44:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004FB74\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004FD10\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004FBD4:\n"
    "    lw   $a0,12($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004FD04\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L8004FD04\n"
    "    nop\n"
    "    bnez   $t6,.L8004FD04\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L8004FD04\n"
    "    sll   $v0,$v0,2\n"
    "    lwc2   $0,16($s0)\n"
    "    lwc2   $1,20($s0)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    swc2   $22,4($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x20\n"
    ".L8004FD04:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004FBD4\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004FD10:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004FD40\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004FF10\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004FDA0:\n"
    "    lhu   $a0,14($s0)\n"
    "    lhu   $a1,16($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004FF04\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8004FF04\n"
    "    nop\n"
    "    bnez   $t8,.L8004FF04\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004FF04\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    lhu   $t0,18($s0)\n"
    "    lwc2   $0,20($s0)\n"
    "    mtc2   $t0,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    swc2   $22,4($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004FF04:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004FDA0\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004FF10:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004FF40\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L800500EC\n"
    "    lh   $s7,96($s2)\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004FFA8:\n"
    "    lw   $a0,12($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L800500DC\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L800500DC\n"
    "    nop\n"
    "    bnez   $t6,.L800500DC\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L800500DC\n"
    "    sll   $v0,$v0,2\n"
    "    lwc2   $0,16($s0)\n"
    "    lwc2   $1,20($s0)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    mtc2   $s7,$8\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    gpf   1\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    ".L800500DC:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    addiu   $s6,$s6,0x20\n"
    "    bnez   $s3,.L8004FFA8\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L800500EC:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050120\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050300\n"
    "    lh   $s7,96($s2)\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80050188:\n"
    "    lhu   $a0,14($s0)\n"
    "    lhu   $a1,16($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L800502F0\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L800502F0\n"
    "    nop\n"
    "    bnez   $t8,.L800502F0\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L800502F0\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    lhu   $t0,18($s0)\n"
    "    lwc2   $0,20($s0)\n"
    "    mtc2   $t0,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    mtc2   $s7,$8\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    gpf   1\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    ".L800502F0:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    addiu   $s6,$s6,0x28\n"
    "    bnez   $s3,.L80050188\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050300:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050334\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050508\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L80050398:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L800504FC\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L800504FC\n"
    "    nop\n"
    "    bnez   $t6,.L800504FC\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,20($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,32($s6)\n"
    "    blez   $v0,.L800504FC\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L800504FC\n"
    "    nop\n"
    "    lwc2   $0,16($s0)\n"
    "    lw   $t0,20($s0)\n"
    "    lwc2   $2,24($s0)\n"
    "    mtc2   $t0,$1\n"
    "    lwc2   $4,28($s0)\n"
    "    lwc2   $5,32($s0)\n"
    "    srl   $t0,$t0,16\n"
    "    mtc2   $t0,$3\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    nct\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,4($s6)\n"
    "    swc2   $21,16($s6)\n"
    "    swc2   $22,28($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L800504FC:\n"
    "    addiu   $s0,$s0,0x24\n"
    "    bnez   $s3,.L80050398\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050508:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050538\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050744\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0xC00\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8005059C:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L80050738\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L80050738\n"
    "    nop\n"
    "    bnez   $t8,.L80050738\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L80050738\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L80050738\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,44($s6)\n"
    "    lw   $t9,24($s0)\n"
    "    lwc2   $0,20($s0)\n"
    "    mtc2   $t9,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    swc2   $22,4($s6)\n"
    "    lwc2   $0,28($s0)\n"
    "    srl   $t9,$t9,16\n"
    "    mtc2   $t9,$1\n"
    "    lwc2   $2,32($s0)\n"
    "    lw   $t9,36($s0)\n"
    "    lwc2   $4,40($s0)\n"
    "    mtc2   $t9,$3\n"
    "    srl   $t8,$t9,16\n"
    "    mtc2   $t8,$5\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    nct\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sh   $t3,48($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,16($s6)\n"
    "    swc2   $21,28($s6)\n"
    "    swc2   $22,40($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x34\n"
    ".L80050738:\n"
    "    addiu   $s0,$s0,0x2C\n"
    "    bnez   $s3,.L8005059C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050744:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050774\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L800508D8\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x400\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L800507D4:\n"
    "    lw   $a0,0($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L800508C8\n"
    "    andi   $t6,$t6,0x8000\n"
    "    mfc2   $v0,$24\n"
    "    bnez   $t6,.L800508C8\n"
    "    nop\n"
    "    bgez   $v0,.L800508C8\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    blez   $v0,.L800508C8\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $t0,0x2000\n"
    "    sw   $t0,4($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    ".L800508C8:\n"
    "    addiu   $s0,$s0,0x4\n"
    "    addiu   $s6,$s6,0x14\n"
    "    bnez   $s3,.L800507D4\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L800508D8:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050908\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050A88\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80050968:\n"
    "    lw   $a0,0($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L80050A78\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L80050A78\n"
    "    nop\n"
    "    bnez   $t8,.L80050A78\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L80050A78\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t7,20($s6)\n"
    "    lui   $t0,0x2800\n"
    "    sw   $t0,4($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    ".L80050A78:\n"
    "    addiu   $s0,$s0,0x4\n"
    "    addiu   $s6,$s6,0x18\n"
    "    bnez   $s3,.L80050968\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050A88:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050AB8\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050C34\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x600\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80050B1C:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L80050C28\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L80050C28\n"
    "    nop\n"
    "    bnez   $t6,.L80050C28\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t0,20($s6)\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80050C28\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    lui   $t0,0x4C80\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    addiu   $s6,$s6,0x1C\n"
    ".L80050C28:\n"
    "    addiu   $s0,$s0,0x14\n"
    "    bnez   $s3,.L80050B1C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050C34:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050C68\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050E2C\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80050CCC:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L80050E20\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L80050E20\n"
    "    nop\n"
    "    bnez   $t8,.L80050E20\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80050E20\n"
    "    nop\n"
    "    sw   $t1,8($s6)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,36($s6)\n"
    "    sw   $t1,40($s6)\n"
    "    lui   $t0,0x4880\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t0,28($s6)\n"
    "    sw   $t1,44($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x18\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x18\n"
    ".L80050E20:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L80050CCC\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050E2C:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050E60\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050FDC\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x600\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80050EC4:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L80050FD0\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L80050FD0\n"
    "    nop\n"
    "    bnez   $t6,.L80050FD0\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t0,20($s6)\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80050FD0\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    lui   $t0,0x4C80\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    addiu   $s6,$s6,0x1C\n"
    ".L80050FD0:\n"
    "    addiu   $s0,$s0,0x14\n"
    "    bnez   $s3,.L80050EC4\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050FDC:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051010\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L800511D4\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80051074:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L800511C8\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L800511C8\n"
    "    nop\n"
    "    bnez   $t8,.L800511C8\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L800511C8\n"
    "    nop\n"
    "    sw   $t1,8($s6)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,36($s6)\n"
    "    sw   $t1,40($s6)\n"
    "    lui   $t0,0x4880\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t0,28($s6)\n"
    "    sw   $t1,44($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x18\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x18\n"
    ".L800511C8:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L80051074\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L800511D4:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051208\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80051380\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x600\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80051268:\n"
    "    lw   $a0,12($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L80051374\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L80051374\n"
    "    nop\n"
    "    bnez   $t6,.L80051374\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t0,20($s6)\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80051374\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    lui   $t0,0x4C80\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    addiu   $s6,$s6,0x1C\n"
    ".L80051374:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L80051268\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80051380:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800513B0\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8005157C\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80051410:\n"
    "    lhu   $a0,14($s0)\n"
    "    lhu   $a1,16($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L80051570\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L80051570\n"
    "    nop\n"
    "    bnez   $t8,.L80051570\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80051570\n"
    "    nop\n"
    "    sw   $t1,8($s6)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,36($s6)\n"
    "    sw   $t1,40($s6)\n"
    "    lui   $t0,0x4880\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t0,28($s6)\n"
    "    sw   $t1,44($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x18\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x18\n"
    ".L80051570:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L80051410\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8005157C:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800515AC\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80051724\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x600\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8005160C:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L80051718\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L80051718\n"
    "    nop\n"
    "    bnez   $t6,.L80051718\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t0,20($s6)\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80051718\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    lui   $t0,0x4C80\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    addiu   $s6,$s6,0x1C\n"
    ".L80051718:\n"
    "    addiu   $s0,$s0,0x24\n"
    "    bnez   $s3,.L8005160C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80051724:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051754\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80051914\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L800517B4:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L80051908\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L80051908\n"
    "    nop\n"
    "    bnez   $t8,.L80051908\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80051908\n"
    "    nop\n"
    "    sw   $t1,8($s6)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,36($s6)\n"
    "    sw   $t1,40($s6)\n"
    "    lui   $t0,0x4880\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t0,28($s6)\n"
    "    sw   $t1,44($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x18\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x18\n"
    ".L80051908:\n"
    "    addiu   $s0,$s0,0x2C\n"
    "    bnez   $s3,.L800517B4\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80051914:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051944\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80051AD8\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x400\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $t0,0x1F80\n"
    "    lw   $t0,20($t0)\n"
    "    addiu   $t1,$zero,0x14\n"
    "    and   $t0,$t0,$t1\n"
    "    add   $s6,$s0,$t0\n"
    "    addi   $s0,$s0,0x28\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,4($s6)\n"
    ".L800519B8:\n"
    "    lw   $a0,0($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L80051AC8\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L80051AC8\n"
    "    nop\n"
    "    bnez   $t6,.L80051AC8\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    blez   $v0,.L80051AC8\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L80051AC8\n"
    "    nop\n"
    "    lwc2   $0,4($s0)\n"
    "    lwc2   $1,8($s0)\n"
    "    lwc2   $6,12($s0)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    nccs\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    ".L80051AC8:\n"
    "    addiu   $s0,$s0,0x38\n"
    "    addiu   $s6,$s6,0x38\n"
    "    bnez   $s3,.L800519B8\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80051AD8:\n"
    "    addi   $s0,$s0,-0x28\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051B04\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80051CD8\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $t0,0x1F80\n"
    "    lw   $t0,20($t0)\n"
    "    addiu   $t1,$zero,0x18\n"
    "    and   $t1,$t0,$t1\n"
    "    add   $s6,$s0,$t1\n"
    "    addi   $s0,$s0,0x30\n"
    "    addiu   $t1,$zero,0xC\n"
    "    and   $t1,$t0,$t1\n"
    "    addi   $t1,$t1,0x8\n"
    "    add   $s7,$s0,$t1\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80051B88:\n"
    "    lw   $a0,0($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L80051CC4\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L80051CC4\n"
    "    nop\n"
    "    bnez   $t8,.L80051CC4\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L80051CC4\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L80051CC4\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t7,20($s6)\n"
    "    lw   $t2,4($s0)\n"
    "    nop\n"
    "    add   $v0,$v0,$t2\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s7,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    lui   $at,0x200\n"
    "    or   $t1,$t1,$at\n"
    "    sw   $t1,0($s7)\n"
    ".L80051CC4:\n"
    "    addiu   $s0,$s0,0x50\n"
    "    addiu   $s6,$s6,0x50\n"
    "    addiu   $s7,$s7,0x50\n"
    "    bnez   $s3,.L80051B88\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80051CD8:\n"
    "    addi   $s0,$s0,-0x30\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051D08\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80051EA8\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,%hi(D_800A38D8)\n"
    "    lw   $s4,%lo(D_800A38D8)($s4)\n"
    "    lui   $s5,0x400\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $t0,0x1F80\n"
    "    lw   $t0,20($t0)\n"
    "    addiu   $t1,$zero,0x14\n"
    "    and   $t0,$t0,$t1\n"
    "    add   $s6,$s0,$t0\n"
    "    addi   $s0,$s0,0x28\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,4($s6)\n"
    ".L80051D7C:\n"
    "    lw   $a0,0($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L80051E98\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L80051E98\n"
    "    nop\n"
    "    bnez   $t6,.L80051E98\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    blez   $v0,.L80051E98\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L80051E98\n"
    "    nop\n"
    "    addi   $v0,$v0,-0x3E8\n"
    "    lwc2   $0,4($s0)\n"
    "    lwc2   $1,8($s0)\n"
    "    lwc2   $6,12($s0)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    nccs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    srl   $v0,$v0,4\n"
    "    andi   $v0,$v0,0xFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    ".L80051E98:\n"
    "    addiu   $s0,$s0,0x38\n"
    "    addiu   $s6,$s6,0x38\n"
    "    bnez   $s3,.L80051D7C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80051EA8:\n"
    "    addi   $s0,$s0,-0x28\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051ED4\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8005208C\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,%hi(D_800A38D8)\n"
    "    lw   $s4,%lo(D_800A38D8)($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $t0,0x1F80\n"
    "    lw   $t0,20($t0)\n"
    "    addiu   $t1,$zero,0x18\n"
    "    and   $t0,$t0,$t1\n"
    "    add   $s6,$s0,$t0\n"
    "    addi   $s0,$s0,0x30\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,4($s6)\n"
    ".L80051F48:\n"
    "    lw   $a0,0($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8005207C\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8005207C\n"
    "    nop\n"
    "    bnez   $t8,.L8005207C\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8005207C\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8005207C\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t7,20($s6)\n"
    "    addi   $v0,$v0,-0x3E8\n"
    "    lwc2   $0,4($s0)\n"
    "    lwc2   $1,8($s0)\n"
    "    lwc2   $6,12($s0)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    nccs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    srl   $v0,$v0,4\n"
    "    andi   $v0,$v0,0xFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    ".L8005207C:\n"
    "    addiu   $s0,$s0,0x40\n"
    "    addiu   $s6,$s6,0x40\n"
    "    bnez   $s3,.L80051F48\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8005208C:\n"
    "    addi   $s0,$s0,-0x30\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
INCLUDE_ASM("asm/funcs", func_800520B8);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800523E0\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s5,36($sp)\n"
    "    sw   $s4,32($sp)\n"
    "    sw   $s3,28($sp)\n"
    "    sw   $s2,24($sp)\n"
    "    sw   $s1,20($sp)\n"
    "    sw   $s0,16($sp)\n"
    "    addu   $s0,$a0,$zero\n"
    "    addu   $s1,$a1,$zero\n"
    "    addu   $s3,$a2,$zero\n"
    "    addu   $s2,$a3,$zero\n"
    "    addiu   $s4,$zero,0x1000\n"
    "    sub   $s4,$s4,$s3\n"
    "    lh   $t0,0($s0)\n"
    "    lh   $t1,2($s0)\n"
    "    lh   $t2,4($s0)\n"
    "    mtc2   $s4,$8\n"
    "    mtc2   $t0,$9\n"
    "    mtc2   $t1,$10\n"
    "    mtc2   $t2,$11\n"
    "    lh   $t3,0($s1)\n"
    "    lh   $t4,2($s1)\n"
    "    gpf   1\n"
    "    lh   $t5,4($s1)\n"
    "    lh   $a0,6($s0)\n"
    "    lh   $a1,8($s0)\n"
    "    lh   $a2,10($s0)\n"
    "    mtc2   $s3,$8\n"
    "    mtc2   $t3,$9\n"
    "    mtc2   $t4,$10\n"
    "    mtc2   $t5,$11\n"
    "    lh   $s5,6($s1)\n"
    "    lh   $fp,8($s1)\n"
    "    gpl   1\n"
    "    lh   $a3,10($s1)\n"
    "    nop\n"
    "    jal   func_800525D8\n"
    "    nop\n"
    "    mtc2   $s4,$8\n"
    "    mtc2   $a0,$9\n"
    "    mtc2   $a1,$10\n"
    "    mtc2   $a2,$11\n"
    "    addu   $a0,$t0,$zero\n"
    "    addu   $a1,$t1,$zero\n"
    "    gpf   1\n"
    "    addu   $a2,$t2,$zero\n"
    "    sh   $a0,0($s2)\n"
    "    sh   $a1,2($s2)\n"
    "    sh   $a2,4($s2)\n"
    "    mtc2   $s3,$8\n"
    "    mtc2   $s5,$9\n"
    "    mtc2   $fp,$10\n"
    "    mtc2   $a3,$11\n"
    "    nop\n"
    "    nop\n"
    "    gpl   1\n"
    "    nop\n"
    "    nop\n"
    "    mfc2   $t0,$9\n"
    "    mfc2   $t1,$10\n"
    "    mfc2   $t2,$11\n"
    "    mult   $a0,$t0\n"
    "    mflo   $t3\n"
    "    nop\n"
    "    nop\n"
    "    mult   $a1,$t1\n"
    "    mflo   $t4\n"
    "    nop\n"
    "    nop\n"
    "    mult   $a2,$t2\n"
    "    mflo   $t5\n"
    "    nop\n"
    "    add   $t3,$t3,$t4\n"
    "    add   $t3,$t3,$t5\n"
    "    sra   $t3,$t3,12\n"
    "    neg   $t3,$t3\n"
    "    mtc2   $t3,$8\n"
    "    mtc2   $a0,$9\n"
    "    mtc2   $a1,$10\n"
    "    mtc2   $a2,$11\n"
    "    mtc2   $t0,$25\n"
    "    mtc2   $t1,$26\n"
    "    mtc2   $t2,$27\n"
    "    nop\n"
    "    nop\n"
    "    gpl   1\n"
    "    nop\n"
    "    nop\n"
    "    jal   func_800525D8\n"
    "    nop\n"
    "    cfc2   $t3,$0\n"
    "    cfc2   $t4,$2\n"
    "    cfc2   $t5,$4\n"
    "    ctc2   $a0,$0\n"
    "    ctc2   $a1,$2\n"
    "    ctc2   $a2,$4\n"
    "    mtc2   $t0,$9\n"
    "    mtc2   $t1,$10\n"
    "    mtc2   $t2,$11\n"
    "    nop\n"
    "    nop\n"
    "    op   1\n"
    "    sh   $t0,6($s2)\n"
    "    sh   $t1,8($s2)\n"
    "    sh   $t2,10($s2)\n"
    "    mfc2   $t0,$25\n"
    "    mfc2   $t1,$26\n"
    "    mfc2   $t2,$27\n"
    "    sh   $t0,12($s2)\n"
    "    sh   $t1,14($s2)\n"
    "    sh   $t2,16($s2)\n"
    "    ctc2   $t3,$0\n"
    "    ctc2   $t4,$2\n"
    "    ctc2   $t5,$4\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s5,36($sp)\n"
    "    lw   $s4,32($sp)\n"
    "    lw   $s3,28($sp)\n"
    "    lw   $s2,24($sp)\n"
    "    lw   $s1,20($sp)\n"
    "    lw   $s0,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800525D8\n"
    "    mfc2   $t0, $9\n"
    "    mfc2   $t1, $10\n"
    "    mfc2   $t2, $11\n"
    "    nop\n"
    "    nop\n"
    "    sqr    0\n"
    "    nop\n"
    "    mfc2   $t3, $25\n"
    "    mfc2   $t4, $26\n"
    "    mfc2   $t5, $27\n"
    "    add    $t3, $t3, $t4\n"
    "    add    $v0, $t3, $t5\n"
    "    mtc2   $v0, $30\n"
    "    nop\n"
    "    nop\n"
    "    mfc2   $v1, $31\n"
    "    nop\n"
    "    andi   $v1, $v1, 0x1E\n"
    "    addiu  $t6, $zero, 0x1F\n"
    "    sub    $t6, $t6, $v1\n"
    "    addi   $t3, $v1, -0x18\n"
    "    bltz   $t3, .L8005263C\n"
    "    sra    $t6, $t6, 1\n"
    "    j      .L80052648\n"
    "    sllv   $t4, $v0, $t3\n"
    ".L8005263C:\n"
    "    addiu  $t3, $zero, 0x18\n"
    "    sub    $t3, $t3, $v1\n"
    "    srav   $t4, $v0, $t3\n"
    ".L80052648:\n"
    "    addi   $t4, $t4, -0x40\n"
    "    sll    $t4, $t4, 1\n"
    "    lui    $t5, %hi(D_800154A0)\n"
    "    addiu  $t5, $t5, %lo(D_800154A0)\n"
    "    addu   $t5, $t5, $t4\n"
    "    lh     $t5, 0($t5)\n"
    "    nop\n"
    "    mtc2   $t5, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    nop\n"
    "    nop\n"
    "    gpf    0\n"
    "    mfc2   $t0, $25\n"
    "    mfc2   $t1, $26\n"
    "    mfc2   $t2, $27\n"
    "    srav   $t0, $t0, $t6\n"
    "    srav   $t1, $t1, $t6\n"
    "    srav   $t2, $t2, $t6\n"
    "    jr     $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800526A0\n"
    "    mtc2   $a0, $30\n"
    "    beqz   $a0, .L80052710\n"
    "    addiu  $t0, $zero, 0x1F\n"
    "    mfc2   $v0, $31\n"
    "    lui    $t1, %hi(D_80015620)\n"
    "    addiu  $t1, $t1, %lo(D_80015620)\n"
    "    andi   $v0, $v0, 0x1E\n"
    "    addi   $t0, $v0, -0x18\n"
    "    bltz   $t0, .L800526E0\n"
    "    nop\n"
    "    sll    $v0, $a0, 1\n"
    "    add    $v0, $v0, $t1\n"
    "    lh     $v0, 0($v0)\n"
    "    nop\n"
    "    jr     $ra\n"
    "    srl    $v0, $v0, 9\n"
    ".L800526E0:\n"
    "    addiu  $t0, $zero, 0x18\n"
    "    sub    $t0, $t0, $v0\n"
    "    srav   $t0, $a0, $t0\n"
    "    sll    $t0, $t0, 1\n"
    "    add    $t0, $t0, $t1\n"
    "    lh     $t0, 0($t0)\n"
    "    addiu  $t1, $zero, 0x1F\n"
    "    sub    $t1, $t1, $v0\n"
    "    sra    $t1, $t1, 1\n"
    "    sllv   $v0, $t0, $t1\n"
    "    jr     $ra\n"
    "    srl    $v0, $v0, 12\n"
    ".L80052710:\n"
    "    jr     $ra\n"
    "    addu   $v0, $zero, $zero\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_2; /* padding after func_800526A0 */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052720\n"
    "    mtc2   $a0, $9\n"
    "    mtc2   $a1, $10\n"
    "    mtc2   $a2, $11\n"
    "    nop\n"
    "    nop\n"
    "    sqr    0\n"
    "    nop\n"
    "    mfc2   $t0, $25\n"
    "    mfc2   $t1, $26\n"
    "    mfc2   $t2, $27\n"
    "    add    $a0, $t0, $t1\n"
    "    j      func_800526A0\n"
    "    add    $a0, $a0, $t2\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052754\n"
    "    mtc2   $a0, $9\n"
    "    mtc2   $a1, $10\n"
    "    mtc2   $a2, $11\n"
    "    nop\n"
    "    nop\n"
    "    sqr    0\n"
    "    nop\n"
    "    mfc2   $t0, $25\n"
    "    mfc2   $t1, $26\n"
    "    mfc2   $t2, $27\n"
    "    addu   $v0, $t0, $t1\n"
    "    jr     $ra\n"
    "    addu   $v0, $v0, $t2\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052788\n"
    "    lh     $t0, 0($a0)\n"
    "    lh     $t1, 2($a0)\n"
    "    lh     $t2, 4($a0)\n"
    "    ori    $t3, $zero, 0x1000\n"
    "    sub    $t3, $t3, $a2\n"
    "    mtc2   $t3, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    lh     $t0, 0($a1)\n"
    "    nop\n"
    "    gpf    1\n"
    "    lh     $t1, 2($a1)\n"
    "    lh     $t2, 4($a1)\n"
    "    mtc2   $a2, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    nop\n"
    "    nop\n"
    "    gpl    1\n"
    "    mfc2   $t0, $9\n"
    "    mfc2   $t0, $9\n"
    "    mfc2   $t1, $10\n"
    "    mfc2   $t2, $11\n"
    "    sh     $t0, 0($a3)\n"
    "    sh     $t1, 2($a3)\n"
    "    jr     $ra\n"
    "    sh     $t2, 4($a3)\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800527FC\n"
    "    lw     $t0, 0($a0)\n"
    "    lh     $t2, 4($a0)\n"
    "    srl    $t1, $t0, 16\n"
    "    ori    $v0, $zero, 0x1000\n"
    "    sub    $v0, $v0, $a2\n"
    "    mtc2   $v0, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    lw     $t0, 0($a1)\n"
    "    nop\n"
    "    gpf    1\n"
    "    lh     $t2, 4($a1)\n"
    "    sra    $t1, $t0, 16\n"
    "    mtc2   $a2, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    lw     $t1, 8($a0)\n"
    "    nop\n"
    "    gpl    1\n"
    "    lh     $t0, 6($a0)\n"
    "    srl    $t2, $t1, 16\n"
    "    mfc2   $t3, $9\n"
    "    mfc2   $t4, $10\n"
    "    mfc2   $t5, $11\n"
    "    sh     $t3, 0($a3)\n"
    "    sh     $t4, 2($a3)\n"
    "    sh     $t5, 4($a3)\n"
    "    mtc2   $v0, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    lw     $t1, 8($a1)\n"
    "    nop\n"
    "    gpf    1\n"
    "    lh     $t0, 6($a1)\n"
    "    srl    $t2, $t1, 16\n"
    "    mtc2   $a2, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    lw     $t0, 12($a0)\n"
    "    nop\n"
    "    gpl    1\n"
    "    lh     $t2, 16($a0)\n"
    "    srl    $t1, $t0, 16\n"
    "    mfc2   $t3, $9\n"
    "    mfc2   $t4, $10\n"
    "    mfc2   $t5, $11\n"
    "    sh     $t3, 6($a3)\n"
    "    sh     $t4, 8($a3)\n"
    "    sh     $t5, 10($a3)\n"
    "    mtc2   $v0, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    lw     $t0, 12($a1)\n"
    "    nop\n"
    "    gpf    1\n"
    "    lh     $t2, 16($a1)\n"
    "    srl    $t1, $t0, 16\n"
    "    mtc2   $a2, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    nop\n"
    "    nop\n"
    "    gpl    1\n"
    "    mfc2   $t3, $9\n"
    "    mfc2   $t3, $9\n"
    "    mfc2   $t4, $10\n"
    "    mfc2   $t5, $11\n"
    "    sh     $t3, 12($a3)\n"
    "    sh     $t4, 14($a3)\n"
    "    jr     $ra\n"
    "    sh     $t5, 16($a3)\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052930\n"
    "    lw     $t0, 0($a0)\n"
    "    lw     $t1, 4($a0)\n"
    "    lw     $t2, 8($a0)\n"
    "    lw     $t3, 12($a0)\n"
    "    lw     $t4, 16($a0)\n"
    "    lui    $t9, 0xFFFF\n"
    "    ctc2   $t0, $0\n"
    "    ctc2   $t1, $1\n"
    "    ctc2   $t2, $2\n"
    "    ctc2   $t3, $3\n"
    "    ctc2   $t4, $4\n"
    "    ctc2   $zero, $5\n"
    "    ctc2   $zero, $6\n"
    "    ctc2   $zero, $7\n"
    "    lw     $t0, 0($a1)\n"
    "    lw     $t1, 4($a1)\n"
    "    lw     $t2, 8($a1)\n"
    "    lw     $t3, 12($a1)\n"
    "    lw     $t4, 16($a1)\n"
    "    and    $t5, $t1, $t9\n"
    "    andi   $t6, $t0, 0xFFFF\n"
    "    or     $t5, $t5, $t6\n"
    "    mtc2   $t5, $0\n"
    "    andi   $t6, $t3, 0xFFFF\n"
    "    mtc2   $t6, $1\n"
    "    sll    $v0, $t2, 16\n"
    "    nop\n"
    "    rtv0tr\n"
    "    srl    $v1, $t0, 16\n"
    "    or     $v0, $v0, $v1\n"
    "    srl    $v1, $t3, 16\n"
    "    mfc2   $t5, $9\n"
    "    mfc2   $t6, $10\n"
    "    mfc2   $t7, $11\n"
    "    mtc2   $v0, $0\n"
    "    mtc2   $v1, $1\n"
    "    sh     $t5, 0($a2)\n"
    "    sh     $t6, 6($a2)\n"
    "    sh     $t7, 12($a2)\n"
    "    rtv0tr\n"
    "    and    $v0, $t2, $t9\n"
    "    andi   $v1, $t1, 0xFFFF\n"
    "    or     $v0, $v0, $v1\n"
    "    andi   $v1, $t4, 0xFFFF\n"
    "    mfc2   $t5, $9\n"
    "    mfc2   $t6, $10\n"
    "    mfc2   $t7, $11\n"
    "    mtc2   $v0, $0\n"
    "    mtc2   $v1, $1\n"
    "    sh     $t5, 2($a2)\n"
    "    sh     $t6, 8($a2)\n"
    "    sh     $t7, 14($a2)\n"
    "    rtv0tr\n"
    "    mfc2   $t5, $9\n"
    "    mfc2   $t6, $10\n"
    "    mfc2   $t7, $11\n"
    "    sh     $t5, 4($a2)\n"
    "    sh     $t6, 10($a2)\n"
    "    jr     $ra\n"
    "    sh     $t7, 16($a2)\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052A20\n"
    "    lw     $t0, 0($a0)\n"
    "    lw     $t1, 4($a0)\n"
    "    lw     $t2, 8($a0)\n"
    "    lw     $t3, 12($a0)\n"
    "    lw     $t4, 16($a0)\n"
    "    lw     $t5, 20($a0)\n"
    "    lw     $t6, 24($a0)\n"
    "    lw     $t7, 28($a0)\n"
    "    ctc2   $t0, $0\n"
    "    ctc2   $t1, $1\n"
    "    ctc2   $t2, $2\n"
    "    ctc2   $t3, $3\n"
    "    ctc2   $t4, $4\n"
    "    ctc2   $t5, $5\n"
    "    ctc2   $t6, $6\n"
    "    ctc2   $t7, $7\n"
    "    lwc2   $0, 0($a1)\n"
    "    lwc2   $1, 4($a1)\n"
    "    nop\n"
    "    nop\n"
    "    rtv0tr\n"
    "    nop\n"
    "    swc2   $9, 0($a2)\n"
    "    swc2   $10, 4($a2)\n"
    "    jr     $ra\n"
    "    swc2   $11, 8($a2)\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052A88\n"
    "    lw     $t0, 0($a0)\n"
    "    lw     $t1, 4($a0)\n"
    "    lw     $t2, 8($a0)\n"
    "    lw     $t3, 12($a0)\n"
    "    lw     $t4, 16($a0)\n"
    "    lw     $t5, 20($a0)\n"
    "    lw     $t6, 24($a0)\n"
    "    lw     $t7, 28($a0)\n"
    "    ctc2   $t0, $0\n"
    "    ctc2   $t1, $1\n"
    "    ctc2   $t2, $2\n"
    "    ctc2   $t3, $3\n"
    "    lh     $t1, 4($a1)\n"
    "    lhu    $t0, 0($a1)\n"
    "    sll    $t1, $t1, 16\n"
    "    or     $t0, $t0, $t1\n"
    "    ctc2   $t4, $4\n"
    "    ctc2   $t5, $5\n"
    "    ctc2   $t6, $6\n"
    "    ctc2   $t7, $7\n"
    "    mtc2   $t0, $0\n"
    "    lwc2   $1, 8($a1)\n"
    "    nop\n"
    "    nop\n"
    "    rtv0tr\n"
    "    nop\n"
    "    swc2   $9, 0($a2)\n"
    "    swc2   $10, 4($a2)\n"
    "    jr     $ra\n"
    "    swc2   $11, 8($a2)\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052B00\n"
    "    lw     $t0, 0($a0)\n"
    "    lw     $t1, 4($a0)\n"
    "    lw     $t2, 8($a0)\n"
    "    lw     $t3, 12($a0)\n"
    "    lw     $t4, 16($a0)\n"
    "    lw     $t5, 20($a0)\n"
    "    lw     $t6, 24($a0)\n"
    "    lw     $t7, 28($a0)\n"
    "    ctc2   $t0, $0\n"
    "    ctc2   $t1, $1\n"
    "    ctc2   $t2, $2\n"
    "    ctc2   $t3, $3\n"
    "    ctc2   $t4, $4\n"
    "    ctc2   $t5, $5\n"
    "    ctc2   $t6, $6\n"
    "    jr     $ra\n"
    "    ctc2   $t7, $7\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052B44\n"
    "    lw     $t0, 0($a0)\n"
    "    lw     $t1, 4($a0)\n"
    "    lw     $t2, 8($a0)\n"
    "    lw     $t3, 12($a0)\n"
    "    lw     $t4, 16($a0)\n"
    "    ctc2   $t0, $0\n"
    "    ctc2   $t1, $1\n"
    "    ctc2   $t2, $2\n"
    "    ctc2   $t3, $3\n"
    "    ctc2   $t4, $4\n"
    "    ctc2   $zero, $5\n"
    "    ctc2   $zero, $6\n"
    "    jr     $ra\n"
    "    ctc2   $zero, $7\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052B7C\n"
    "    lw     $t0, 0($a0)\n"
    "    lw     $t1, 4($a0)\n"
    "    lw     $t2, 8($a0)\n"
    "    lw     $t3, 12($a0)\n"
    "    lw     $t4, 16($a0)\n"
    "    lh     $t5, 0($a1)\n"
    "    lh     $t6, 2($a1)\n"
    "    lh     $t7, 4($a1)\n"
    "    ctc2   $t0, $0\n"
    "    ctc2   $t1, $1\n"
    "    ctc2   $t2, $2\n"
    "    ctc2   $t3, $3\n"
    "    ctc2   $t4, $4\n"
    "    ctc2   $t5, $5\n"
    "    ctc2   $t6, $6\n"
    "    ctc2   $t7, $7\n"
    "    lwc2   $0, 0($a2)\n"
    "    lwc2   $1, 4($a2)\n"
    "    nop\n"
    "    nop\n"
    "    rtv0tr\n"
    "    nop\n"
    "    swc2   $9, 0($a3)\n"
    "    swc2   $10, 4($a3)\n"
    "    jr     $ra\n"
    "    swc2   $11, 8($a3)\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052BE4\n"
    "    cfc2   $t0, $21\n"
    "    cfc2   $t1, $22\n"
    "    cfc2   $t2, $23\n"
    "    srl    $t0, $t0, 4\n"
    "    srl    $t1, $t1, 4\n"
    "    srl    $t2, $t2, 4\n"
    "    sb     $t0, 0($a0)\n"
    "    sb     $t1, 1($a0)\n"
    "    sb     $t2, 2($a0)\n"
    "    jr     $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
INCLUDE_ASM("asm/funcs", func_80052C10);
PAD_NOPS_1; /* padding after func_80052C10 */
INCLUDE_ASM("asm/funcs", func_80052C28);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052C4C\n"
    "    lhu    $t0, 6($a0)\n"
    "    lh     $t1, 8($a0)\n"
    "    lhu    $t2, 10($a0)\n"
    "    lh     $t3, 12($a0)\n"
    "    lhu    $t4, 14($a0)\n"
    "    lh     $t5, 16($a0)\n"
    "    sll    $t1, $t1, 16\n"
    "    sll    $t3, $t3, 16\n"
    "    sll    $t5, $t5, 16\n"
    "    or     $t0, $t0, $t1\n"
    "    or     $t2, $t2, $t3\n"
    "    or     $t4, $t4, $t5\n"
    "    ctc2   $t0, $0\n"
    "    ctc2   $t2, $1\n"
    "    ctc2   $t4, $2\n"
    "    ctc2   $zero, $5\n"
    "    ctc2   $zero, $6\n"
    "    ctc2   $zero, $7\n"
    "    lh     $t0, 0($a0)\n"
    "    lh     $t1, 2($a0)\n"
    "    lh     $t2, 4($a0)\n"
    "    sub    $a1, $a1, $t0\n"
    "    sub    $a2, $a2, $t1\n"
    "    sub    $a3, $a3, $t2\n"
    "    andi   $a1, $a1, 0xFFFF\n"
    "    sll    $a2, $a2, 16\n"
    "    or     $a1, $a1, $a2\n"
    "    mtc2   $a1, $0\n"
    "    mtc2   $a3, $1\n"
    "    nop\n"
    "    nop\n"
    "    rtv0tr\n"
    "    jr     $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052CD4\n"
    "    mfc2   $t0, $9\n"
    "    mfc2   $t1, $10\n"
    "    sra    $t0, $t0, 2\n"
    "    sra    $t1, $t1, 2\n"
    "    sw     $t0, 0($a0)\n"
    "    sw     $t1, 0($a1)\n"
    "    jr     $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_3; /* padding after func_80052CD4 */
INCLUDE_ASM("asm/funcs", func_80052D00);
INCLUDE_ASM("asm/funcs", func_80053304);
INCLUDE_ASM("asm/funcs", func_8005344C);
INCLUDE_ASM("asm/funcs", func_80053584);
INCLUDE_ASM("asm/funcs", func_80053614);
INCLUDE_ASM("asm/funcs", func_80053694);
INCLUDE_ASM("asm/funcs", func_80053754);
INCLUDE_ASM("asm/funcs", func_80053E9C);
extern s32 D_800A33F0;
void func_80054410(s32 a0) {
    D_800A33F0 = a0;
}
void func_8005441C(s32 a0) {
    D_800A33F0 += a0;
}
extern s16 D_800A33F8;
s16 func_80054434(void) {
    return D_800A33F8;
}
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80054440\n"
    "    addiu  $sp, $sp, -0x38\n"
    "    sw     $ra, 40($sp)\n"
    "    sw     $fp, 36($sp)\n"
    "    sw     $s5, 32($sp)\n"
    "    sw     $s4, 28($sp)\n"
    "    sw     $s3, 24($sp)\n"
    "    sw     $s2, 20($sp)\n"
    "    sw     $s1, 16($sp)\n"
    "    addu   $s1, $a0, $zero\n"
    "    addu   $s2, $a1, $zero\n"
    "    lui    $s4, %hi(D_800A3828)\n"
    "    lw     $s4, %lo(D_800A3828)($s4)\n"
    "    nop\n"
    "    addi   $s4, $s4, 24\n"
    "    lui    $s5, 0x1F80\n"
    "    mtc2   $zero, $6\n"
    "    lui    $s3, %hi(D_800F2B70)\n"
    "    addiu  $s3, $s3, %lo(D_800F2B70)\n"
    "    bgez   $zero, .L800545B8\n"
    "    nop\n"
    ".L80054490:\n"
    "    addi   $a2, $s5, 0x0020\n"
    "    sll    $t0, $v0, 2\n"
    "    add    $a1, $s2, $t0\n"
    "    lw     $a1, 0($a1)\n"
    "    jal    func_80052930\n"
    "    addu   $a0, $s4, $zero\n"
    "    lw     $t0, 32($s5)\n"
    "    lw     $t1, 36($s5)\n"
    "    lw     $t2, 40($s5)\n"
    "    lw     $t3, 44($s5)\n"
    "    lw     $t4, 48($s5)\n"
    "    ctc2   $t0, $8\n"
    "    ctc2   $t1, $9\n"
    "    ctc2   $t2, $10\n"
    "    ctc2   $t3, $11\n"
    "    ctc2   $t4, $12\n"
    "    lh     $v0, 0($s1)\n"
    "    addi   $s1, $s1, 2\n"
    "    lhu    $t0, 0($s1)\n"
    "    lh     $t2, 2($s1)\n"
    "    lh     $t1, 4($s1)\n"
    "    sll    $t2, $t2, 16\n"
    "    or     $t0, $t0, $t2\n"
    "    lhu    $t2, 6($s1)\n"
    "    lh     $t4, 8($s1)\n"
    "    lh     $t3, 10($s1)\n"
    "    sll    $t4, $t4, 16\n"
    "    or     $t2, $t2, $t4\n"
    "    lhu    $t4, 12($s1)\n"
    "    lh     $t6, 14($s1)\n"
    "    lh     $t5, 16($s1)\n"
    "    sll    $t6, $t6, 16\n"
    "    or     $t4, $t4, $t6\n"
    "    bgez   $zero, .L80054568\n"
    "    nop\n"
    ".L8005451C:\n"
    "    lhu    $t0, 0($s1)\n"
    "    lh     $t2, 2($s1)\n"
    "    lh     $t1, 4($s1)\n"
    "    sll    $t2, $t2, 16\n"
    "    or     $t0, $t0, $t2\n"
    "    lhu    $t2, 6($s1)\n"
    "    lh     $t4, 8($s1)\n"
    "    lh     $t3, 10($s1)\n"
    "    sll    $t4, $t4, 16\n"
    "    or     $t2, $t2, $t4\n"
    "    lhu    $t4, 12($s1)\n"
    "    lh     $t6, 14($s1)\n"
    "    lh     $t5, 16($s1)\n"
    "    sll    $t6, $t6, 16\n"
    "    or     $t4, $t4, $t6\n"
    "    swc2   $20, 0($s3)\n"
    "    swc2   $21, 4($s3)\n"
    "    swc2   $22, 8($s3)\n"
    "    addi   $s3, $s3, 12\n"
    ".L80054568:\n"
    "    mtc2   $t0, $0\n"
    "    mtc2   $t1, $1\n"
    "    mtc2   $t2, $2\n"
    "    mtc2   $t3, $3\n"
    "    mtc2   $t4, $4\n"
    "    mtc2   $t5, $5\n"
    "    addi   $v0, $v0, -3\n"
    "    nop\n"
    "    nct\n"
    "    bgtz   $v0, .L8005451C\n"
    "    addi   $s1, $s1, 18\n"
    "    swc2   $20, 0($s3)\n"
    "    swc2   $21, 4($s3)\n"
    "    swc2   $22, 8($s3)\n"
    "    addi   $s3, $s3, 12\n"
    "    add    $v0, $v0, $v0\n"
    "    add    $t0, $v0, $v0\n"
    "    add    $s3, $s3, $t0\n"
    "    add    $t0, $t0, $v0\n"
    "    add    $s1, $s1, $t0\n"
    ".L800545B8:\n"
    "    lh     $v0, 0($s1)\n"
    "    nop\n"
    "    addi   $t0, $v0, 1\n"
    "    bnez   $t0, .L80054490\n"
    "    addi   $s1, $s1, 2\n"
    "    addu   $v0, $s1, $zero\n"
    "    lw     $ra, 40($sp)\n"
    "    lw     $fp, 36($sp)\n"
    "    lw     $s5, 32($sp)\n"
    "    lw     $s4, 28($sp)\n"
    "    lw     $s3, 24($sp)\n"
    "    lw     $s2, 20($sp)\n"
    "    lw     $s1, 16($sp)\n"
    "    jr     $ra\n"
    "    addiu  $sp, $sp, 0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
INCLUDE_ASM("asm/funcs", func_800545F4);
INCLUDE_ASM("asm/funcs", func_80054604);
extern s16 D_80099D54[];
extern void func_80054604(s32, s32, s32, s32, s32, s32, s32);
void func_80054884(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7) {
    func_80054604(D_80099D54[a0] + a1 - 0x131, a2, a3, a4, a5, a6, a7);
}
void func_8007B33C(s32);
void func_8004659C(s32);
void func_80046A60(void);
void func_800548DC(void) {
    func_8007B33C(0);
    func_8004659C(-1);
    func_80046A60();
}
INCLUDE_ASM("asm/funcs", func_8005490C);
extern u32 D_80102C00;
extern u32 *D_800A3820;
extern u16 D_800A38D6;
extern s32 D_800A374C;
extern s32 D_800A3808;
extern s32 D_800A378C;
extern s32 func_8005490C(void);
extern void func_800444E0(void);
s32 func_80054F68(void) {
    s32 v3;
    s32 s0;
    D_800A3820 = &D_80102C00;
    v3 = D_800A374C;
    D_800A38D6 = D_800A38D6 + 1;
    D_800A3808 = v3;
    D_800A378C = v3 + 0x10;
    s0 = func_8005490C();
    func_800444E0();
    return s0;
}
extern s32 D_800EFB14;
extern s32 D_800EFB18;
extern s32 D_800EFB1C;
extern s32 D_800EFB20;
extern s32 D_800EFB24;
extern s32 D_800EFB28;
void func_80054FDC(s32 a0) {
    s32 *p = &D_800EFB14;
    *p = a0 + *p;
    D_800EFB18 = a0 + D_800EFB18;
    if (D_800EFB1C) {
        D_800EFB1C = a0 + D_800EFB1C;
    }
    if (D_800EFB20) {
        D_800EFB20 = a0 + D_800EFB20;
    }
    if (D_800EFB24) {
        D_800EFB24 = a0 + D_800EFB24;
    }
    if (D_800EFB28) {
        D_800EFB28 = a0 + D_800EFB28;
    }
}
extern s32 D_800EFB0C;
s32* func_8005507C(void) {
    return &D_800EFB0C;
}
extern s32 D_80101E1C;
s32* func_8005508C(void) {
    return &D_80101E1C;
}
INCLUDE_ASM("asm/funcs", func_8005509C);
INCLUDE_ASM("asm/funcs", func_800550E8);
INCLUDE_ASM("asm/funcs", func_80055138);
INCLUDE_ASM("asm/funcs", func_80055948);
void func_80055B44(u8 *a0, s32 a1, s32 a2, s32 a3) {
    *(s32 *)(a0 + 0x3B4) = a1;
    a0[0x3BC] = (u8)a2;
    a0[0x3B8] = (u8)a3;
    *(s32 *)(a0 + 0x3C8) = 0;
    *(s32 *)(a0 + 0x3CC) = -1;
}
INCLUDE_ASM("asm/funcs", func_80055B60);
INCLUDE_ASM("asm/funcs", func_80056CB8);
INCLUDE_ASM("asm/funcs", func_80056FE8);
INCLUDE_ASM("asm/funcs", func_80057094);
INCLUDE_ASM("asm/funcs", func_800571C0);
INCLUDE_ASM("asm/funcs", func_8005763C);
INCLUDE_ASM("asm/funcs", func_80057ACC);
INCLUDE_ASM("asm/funcs", func_80057CC8);
INCLUDE_ASM("asm/funcs", func_80057E84);
INCLUDE_ASM("asm/funcs", func_80058580);
INCLUDE_ASM("asm/funcs", func_8005B43C);
void func_800858D0(s32);
void func_80085F98(void);
void func_80085EE4(s32);
void func_80085E4C(s32, s32);
void func_80083954(void);
void func_80083B30(void);
extern s32 D_800EFB38[];
extern s32 D_800EFC38[];
extern s32 D_800A3408;
void func_8005B50C(void) {
    s32 i;
    s32 *a0;
    s32 *v1;
    func_800858D0(0);
    func_80085F98();
    func_80085EE4(0);
    func_80085E4C(0, 0);
    func_80083954();
    func_80083B30();
    i = 0;
    a0 = D_800EFB38;
    v1 = D_800EFC38;
    do {
        *v1 = 0;
        *a0 = 0;
        a0++;
        i++;
        v1++;
    } while (i < 0x10);
    D_800A3408 = 0;
}

void func_800858D0(s32);
void func_8005B58C(void) {
    func_800858D0(0);
}
extern void func_800858D0(s32);
extern void func_80086130(s32, s32, s32);
extern u32 D_800EFB78[];
extern u8 D_800EFB7C[];
void func_8005B5AC(void) {
    s32 s1;
    s32 s3;
    u8 *s2;
    s32 s0;
    func_800858D0(0);
    s1 = 0;
    s3 = 0x7F;
    s2 = (u8 *)D_800EFB78;
    s0 = 0;
    do {
        *(u32 *)((u8 *)D_800EFB78 + s0) = 0;
        s2[5] = s3;
        *((u8 *)D_800EFB7C + s0) = s3;
        func_80086130((s16)s1, 0, 0);
        s2 += 8;
        s1++;
        s0 += 8;
    } while (s1 < 24);
}
void func_800858D0(s32);
void func_80087F64(s32);
extern s32 D_800EFC38;
extern s32 D_80015470;
extern u8 D_8008E778;
extern s16 D_800973FC;
extern u8 D_80099BCC;
extern u8 D_80099CC8;
extern u8 D_80099CC9;
extern u16 D_80099D88;
extern u8 D_80099D8B;
extern u8 D_80099D8C;
extern u8 D_80099D8D;
extern u8 D_80099D8E;
extern u8 D_80099D8F;
extern u8 D_80099D94;
extern u8 D_80099D9C;
extern u8 D_80099D9D;
extern u8 D_8009A088;
extern u8 D_8009A820;
extern u8 D_8009A821;
extern u8 D_8009A830;
extern s8 D_8009A838;
extern u8 D_8009A840;
extern u8 D_8009A850;
extern u8 D_8009A851;
extern u8 D_8009A852;
extern u8 D_8009A853;
extern u16 D_8009A8CA;
extern s32 D_8009AA50;
extern u8 D_8009AD18;
extern u8 D_8009B14E;
extern s16 D_8009B16C;
extern s16 D_8009B17C;
extern s16 D_8009B18C;
extern s16 D_8009B2BC;
extern s16 D_8009B2BE;
extern s16 D_8009B2C4;
extern u16 D_8009B450;
extern u16 D_8009B452;
extern u8 D_8009B48E;
extern u8 D_8009B58C;
extern u8 D_8009BA60;
extern s32 D_8009BA7C;
extern s32 D_8009BC04;
extern s32 D_8009BC08;
extern u8 D_8009BC0C;
extern u8 D_8009BC0D;
extern s32 D_8009BC1C;
extern u8 D_8009BC38;
extern u8 D_8009BC40;
extern u8 D_8009BC41;
extern u8 D_8009BC44;
extern u8 D_8009BC72;
extern u8 D_8009BC76;
extern u8 D_8009BC7C;
extern s16 D_8009BC94;
extern s16 D_8009BC96;
extern u16 D_8009BCC4;
extern u16 D_8009BCC6;
extern s16 D_8009BCD0;
extern s16 D_8009BCD2;
extern u8 D_8009BCE4;
extern u8 D_8009BCF8;
extern u8 D_8009BCF9;
extern u8 D_8009BD20;
extern u8 D_8009BD21;
extern s32 D_8009BD38;
extern u8 D_8009BD3C;
extern u8 D_8009BD41;
extern u8 D_8009BD42;
extern s32 D_8009BD44;
extern u8 D_8009BD58;
extern u8 D_8009BD59;
extern s32 D_8009BD68;
extern s32 D_8009BD6C;
extern s32 D_8009BD70;
extern s32 D_8009BD84;
extern s32 D_8009BD88;
extern u8 D_800A3270;
extern s32 D_800A32C8;
extern s16 D_800A33E8;
extern s16 D_800A3438;
extern s32 D_800A344C;
extern s16 D_800A34E8;
extern s16 D_800A3530;
extern s16 D_800A3534;
extern s16 D_800A3540;
extern s16 D_800A3544;
extern u8 D_800A3560;
extern u8 D_800A3561;
extern u8 D_800A3562;
extern s16 D_800A3588;
extern s16 D_800A358C;
extern s16 D_800A3590;
extern s16 D_800A3594;
extern s32 D_800A3618;
extern s32 D_800A3628;
extern s32 D_800A362C;
extern s32 D_800A3638;
extern u8 D_800A3671;
extern u8 D_800A3680;
extern s16 D_800A36A4;
extern s32 D_800A36AC;
extern s32 D_800A3708;
extern s32 D_800A370C;
extern u8 D_800A3783;
extern u8 D_800A3788;
extern s32 D_800A3790;
extern u8 D_800A37A0;
extern u8 D_800A37D2;
extern s32 D_800A3820;
extern s32 D_800A3828;
extern s16 D_800A3834;
extern s32 D_800A387C;
extern u8 D_800A389A;
extern s16 D_800A38AE;
extern s32 D_800A38B4;
extern s32 D_800A38D8;
extern s16 D_800A38DC;
extern u8 D_800A38E2;
extern s16 D_800EF980;
extern s16 D_800EF9F4;
extern s32 D_800EFC44;
extern s32 D_800EFC50;
extern u16 D_800EFC8A;
extern s16 D_800F0B78;
extern s16 D_800F0B7C;
extern s16 D_800F0B98;
extern s16 D_800F0BA4;
extern s16 D_800F0BB2;
extern s16 D_800F0BCC;
extern s16 D_800F0BEC;
extern s32 D_800F0C10;
extern s32 D_800F0C14;
extern s32 D_800F0C18;
extern s32 D_800F0CA0;
extern s32 D_800F0CA4;
extern s32 D_800F0CA8;
extern s32 D_800F0CAC;
extern s32 D_800F0CB0;
extern s32 D_800F0CB4;
extern s32 D_800F0CB8;
extern s32 D_800F0CBC;
extern s32 D_800F0CC0;
extern s32 D_800F0CC4;
extern s32 D_800F0CC8;
extern s32 D_800F0CCC;
extern s32 D_800F0CD0;
extern s32 D_800F0CD4;
extern s32 D_800F0CD8;
extern s32 D_800F0CDC;
extern s32 D_800F0CE0;
extern s32 D_800F0CE4;
extern s32 D_800F0CE8;
extern s32 D_800F0CEC;
extern s32 D_800F0CF0;
extern s32 D_800F0CF4;
extern s32 D_800F0CF8;
extern s32 D_800F0CFC;
extern s32 D_800F0D18;
extern s32 D_800F0D1C;
extern s32 D_800F0D20;
extern s32 D_800F0D24;
extern s32 D_800F0D28;
extern s32 D_800F0D2C;
extern s32 D_800F0D30;
extern s32 D_800F0D34;
extern s32 D_800F0D38;
extern s32 D_800F0D3C;
extern s32 D_800F0D40;
extern s32 D_800F0D44;
extern s32 D_800F0D48;
extern s32 D_800F0D4C;
extern s32 D_800F0D50;
extern s32 D_800F0D54;
extern s32 D_800F0D58;
extern s32 D_800F0D5C;
extern s32 D_800F0D60;
extern s32 D_800F0D64;
extern s32 D_800F0D68;
extern s32 D_800F0D6C;
extern s32 D_800F0D70;
extern s32 D_800F0D74;
extern s32 D_800F0D78;
extern s32 D_800F0D7C;
extern s32 D_800F0D80;
extern s32 D_800F0E38;
extern s32 D_800F0E3C;
extern s32 D_800F0E40;
extern s32 D_800F0EC8;
extern s32 D_800F0ECC;
extern s32 D_800F0ED0;
extern s32 D_800F0FB8;
extern s32 D_800F0FBC;
extern s32 D_800F0FC0;
extern s16 D_800F1000;
extern s16 D_800F1002;
extern s16 D_800F1004;
extern s16 D_800F10A0;
extern s16 D_800F10A2;
extern s16 D_800F10A4;
extern s32 D_800F10D0;
extern s32 D_800F10D4;
extern s32 D_800F10E0;
extern s32 D_800F10E4;
extern s32 D_800F10E8;
extern s32 D_800F10EC;
extern s32 D_800F10F0;
extern s32 D_800F10F4;
extern s32 D_800F10F8;
extern s32 D_800F10FC;
extern s32 D_800F1100;
extern s32 D_800F1104;
extern s32 D_800F1108;
extern s32 D_800F110C;
extern s32 D_800F1110;
extern s32 D_800F1114;
extern s32 D_800F1118;
extern s32 D_800F111C;
extern s32 D_800F1138;
extern s32 D_800F1140;
extern s32 D_800F1144;
extern s32 D_800F1148;
extern u8 D_800F1150;
extern s32 D_800F1178;
extern s32 D_800F117C;
extern s32 D_800F1180;
extern s32 D_800F1198;
extern s32 D_800F119C;
extern s32 D_800F11A0;
extern s32 D_800F1850;
extern u8 D_800F6338;
extern u8 D_800F6339;
extern u8 D_800F633A;
extern s32 D_800F6608;
extern s32 D_800F6610;
extern s16 D_800F6658;
extern s32 D_800F66A0;
extern u8 D_800F74A4;
extern u8 D_800F74A5;
extern u8 D_800FB534;
extern u8 D_800FB535;
extern s16 D_800FF558;
extern s16 D_800FF55A;
extern s16 D_800FF55C;
extern s16 D_800FF55E;
extern s16 D_800FF560;
extern s16 D_800FF562;
extern s16 D_800FF564;
extern s16 D_800FF566;
extern s16 D_800FF568;
extern s32 D_800FF570;
extern s32 D_800FF574;
extern u8 D_800FF639;
extern s16 D_800FF640;
extern s32 D_800FF644;
extern s16 D_800FF648;
extern s16 D_800FF64A;
extern s16 D_800FF64C;
extern s32 D_800FF684;
extern s32 D_800FF688;
extern s32 D_800FF68C;
extern u8 D_80101DF1;
extern s16 D_80101DF8;
extern s32 D_80101DFC;
extern s16 D_80101E00;
extern u16 D_80101E02;
extern u16 D_80101E04;
extern s32 D_80101E3C;
extern s32 D_80101E40;
extern s32 D_80101E44;
extern s16 D_8010278C;
extern s16 D_8010278E;
extern s32 D_80102790;
extern s32 D_80103624;
extern s32 D_800EFB38;
void func_8005B644(s32 a0) {
    s32 v;
    func_800858D0(0);
    v = a0 * 2 + a0 + 1;
    func_80087F64((s16)v);
    *(s32*)((u8*)&D_800EFC38 + (v * 4)) = 0;
    *(s32*)((u8*)&D_800EFB38 + (v * 4)) = 0;
}
extern s32 D_800EFC40;
extern s32 D_800EFB40;
extern s32 D_800EFC4C;
extern s32 D_800EFB4C;
void func_800858D0(s32);
void func_80087F64(s32);
void func_8005B6AC(void) {
    func_800858D0(0);
    func_80087F64(2);
    D_800EFC40 = 0;
    D_800EFB40 = 0;
    func_80087F64(5);
    D_800EFC4C = 0;
    D_800EFB4C = 0;
}
extern s32 D_800EFC3C;
extern s32 D_800EFB3C;
void func_80087F64(s32);
void func_8005B6FC(void) {
    func_80087F64(1);
    D_800EFC3C = 0;
    D_800EFB3C = 0;
}
void func_800858D0(s32);
void func_80085F98(void);
void func_80085EE4(s32);
void func_80085E4C(s32, s32);
void func_80087F64(s16);
void func_8005B5AC(void);
extern s32 D_800EFB3C[];
extern s32 D_800EFC3C[];
extern s32 D_800A3408;
void func_8005B72C(void) {
    s32 s0;
    s32 *s2;
    s32 *s1;
    func_800858D0(0);
    func_80085F98();
    func_80085EE4(0);
    func_80085E4C(0, 0);
    s2 = D_800EFB3C;
    s1 = D_800EFC3C;
    for (s0 = 1; s0 < 0x10; s0++) {
        func_80087F64((s16)s0);
        *s1 = 0;
        *s2 = 0;
        s2++;
        s1++;
    }
    D_800A3408 = 0;
    func_8005B5AC();
}

INCLUDE_ASM("asm/funcs", func_8005B7C4);
extern s32 D_800EFC58;
extern s32 D_800EFB58;
extern s32 D_800EFC48;
extern s32 D_800EFB48;
void func_800858D0(s32);
void func_80087F64(s32);
void func_8005B868(void) {
    func_800858D0(0);
    func_80087F64(8);
    D_800EFC58 = 0;
    D_800EFB58 = 0;
    func_80087F64(4);
    D_800EFC48 = 0;
    D_800EFB48 = 0;
}
INCLUDE_ASM("asm/funcs", func_8005B8B8);
void func_8005C4C0(s32, s32);
void func_8005B98C(s32 a0) {
    func_8005C4C0(a0, 8);
    func_8005C4C0(a0, 4);
}
extern s32 D_800EFC5C;
extern s32 D_800EFB5C;
void func_800858D0(s32);
void func_80087F64(s32);
void func_8005B9C4(void) {
    func_800858D0(0);
    func_80087F64(9);
    D_800EFC5C = 0;
    D_800EFB5C = 0;
}
void func_8005B9C4(void);
s32 func_80036EA8(s32, s32);
s32 func_80036F40(void);
void func_80036D98(s32, s32);
s32 func_80036F28(s32);
void func_8005C2A8(s32, s32, s32);
void func_8005B9FC(s32 a0) {
    s32 s1;
    func_8005B9C4();
    s1 = func_80036EA8(2, 8);
    func_80036F40();
    func_80036D98(s1, a0);
    s1 = func_80036F28(s1);
    func_80036F40();
    func_8005C2A8(a0, 9, a0 + s1);
}
void func_8005C4C0(s32, s32);
void func_8005BA6C(s32 a0) {
    func_8005C4C0(a0, 9);
}
INCLUDE_ASM("asm/funcs", func_8005BA8C);
INCLUDE_ASM("asm/funcs", func_8005BD30);
}
extern u32 D_800EFC38[];
extern u32 D_800EFB38[];
extern u8 D_8009AD18[];
extern void func_80087F64(s32);
void func_8005BDF0(void) {
    u32 *s3 = D_800EFC38;
    u32 *s2 = D_800EFB38;
    u8 *s0 = D_8009AD18;
    u8 *s1 = (u8 *)((s32)s0 + 3);
    do {
        func_80087F64(*s0);
        s3[*s0] = 0;
        s2[*s0] = 0;
        s0++;
    } while ((s32)s0 < (s32)s1);
}
INCLUDE_ASM("asm/funcs", func_8005BE84);
void func_800858D0(s32);
void func_80085F98(void);
void func_80085EE4(s32);
void func_80085E4C(s32, s32);
void func_8005BF3C(void) {
    func_800858D0(0);
    func_80085F98();
    func_80085EE4(0);
    func_80085E4C(0, 0);
}
INCLUDE_ASM("asm/funcs", func_8005BF78);
INCLUDE_ASM("asm/funcs", func_8005C074);
INCLUDE_ASM("asm/funcs", func_8005C2A8);
INCLUDE_ASM("asm/funcs", func_8005C4C0);
extern s32 D_800A3404;
void func_80087F64(s32);
void func_80088088(s32, s16, s32);
s32 func_800884C4(s32, s16);
s16 func_8005C5A8(s32 *a0, s16 a1) {
    s32 *s1 = a0;
    s16 s0 = a1;
    func_80087F64(s0);
    func_80088088(s1[1], s0, D_800A3404);
    *(s32 *)(s1[1] + 8) = s0;
    return (s16)func_800884C4(s1[2], s0);
}
void func_80083BE4(s32, s32);
void func_800858D0(s32);
void func_80087F24(void);
void func_80087F00(s32);
void func_8005C614(void) {
    func_80083BE4(0x7F, 0x7F);
    func_800858D0(0);
    func_80087F24();
    func_80087F00(0);
}
extern s32 D_8009AA70;
extern s32 D_800EFB78;
extern u8 D_800EFB7C;
extern u8 D_800EFB7D;
void func_8005C650(s32 a0, s32 a1, s32 a2) {
    s16 a3 = 0;
    s32 *base = (s32 *)((u8 *)&D_8009AA70 + a0 * 4);
    do {
        s32 off = a3 * 8;
        if (!*(s32 *)((u8 *)&D_800EFB78 + off)) {
            *(s32 *)((u8 *)&D_800EFB78 + off) = (s32)base;
            *((u8 *)&D_800EFB7C + off) = (u8)a1;
            *((u8 *)&D_800EFB7D + off) = (u8)a2;
            return;
        }
        a3 = (s16)(a3 + 1);
    } while ((s16)a3 < 0x18);
}
INCLUDE_ASM("asm/funcs", func_8005C6D0);
INCLUDE_ASM("asm/funcs", func_8005C8A8);
INCLUDE_ASM("asm/funcs", func_8005D46C);
INCLUDE_ASM("asm/funcs", func_8005D554);
INCLUDE_ASM("asm/funcs", func_8005D814);
INCLUDE_ASM("asm/funcs", func_8005E098);
s32 func_8005E098(s32, s32, s32, s32);
s32 func_8005E51C(s32 a0, s32 a1, s32 a2) {
    return func_8005E098(-1, a0 - 1, a1, a2);
}
INCLUDE_ASM("asm/funcs", func_8005E54C);
INCLUDE_ASM("asm/funcs", func_8005F1C8);
INCLUDE_ASM("asm/funcs", func_8005FA98);
INCLUDE_ASM("asm/funcs", func_8005FBC8);
INCLUDE_ASM("asm/funcs", func_8005FC9C);
INCLUDE_ASM("asm/funcs", func_800600C8);
INCLUDE_ASM("asm/funcs", func_800602AC);
INCLUDE_ASM("asm/funcs", func_80060414);
INCLUDE_ASM("asm/funcs", func_80060544);
extern s16 D_800A32B6;
extern s16 D_800A32B4;
void func_80060758(void) {
    D_800A32B6 = 0;
    D_800A32B4 = 0;
}
INCLUDE_ASM("asm/funcs", func_80060768);
INCLUDE_ASM("asm/funcs", func_80060A68);
INCLUDE_ASM("asm/funcs", func_80060B70);
extern s32 D_800F10D0[];
extern u8 D_800F1150[];
extern s16 D_800A345E;
extern s16 D_800A345C;
extern s32 D_800A3458;
extern s32 D_800A3454;
extern s32 D_800A3450;
extern s32 D_800A344C;
extern s32 D_800A3460;
extern s32 D_800A3444;
extern s32 D_800A3448;
void func_80060C60(void) {
    s32 i = 0;
    s32 *p = D_800F10D0;
    do {
        *p = 0;
        D_800F1150[i] = 0;
        i++;
        p++;
    } while (i < 0x1C);
    D_800A345E = 0;
    D_800A345C = 0;
    D_800A3458 = 0;
    D_800A3454 = 0;
    D_800A3450 = 0;
    D_800A344C = 0;
    D_800A3460 = 0;
    D_800A3444 = 0;
    D_800A3448 = 0;
}

INCLUDE_ASM("asm/funcs", func_80060CB8);
INCLUDE_ASM("asm/funcs", func_80060E04);
INCLUDE_ASM("asm/funcs", func_80060E38);
INCLUDE_ASM("asm/funcs", func_80061064);
extern s32 D_800A32BC;
void func_80060C60(void);
void func_800421A4(void);
void func_80061178(void) {
    func_80060C60();
    func_800421A4();
    D_800A32BC = 0;
}
INCLUDE_ASM("asm/funcs", func_800611A4);
INCLUDE_ASM("asm/funcs", func_80061250);
INCLUDE_ASM("asm/funcs", func_8006133C);
INCLUDE_ASM("asm/funcs", func_800613C8);
INCLUDE_ASM("asm/funcs", func_80061454);
INCLUDE_ASM("asm/funcs", func_800614E0);
INCLUDE_ASM("asm/funcs", func_8006156C);
INCLUDE_ASM("asm/funcs", func_80061658);
INCLUDE_ASM("asm/funcs", func_80061710);
INCLUDE_ASM("asm/funcs", func_800617C8);
INCLUDE_ASM("asm/funcs", func_800618B4);
extern s32 D_800F116C;
extern s32 D_800A3468;
extern s32 D_800F1178;
extern s32 D_800F1180;
extern s32 D_800F1158;
void func_80060A68(void);
void func_800619A4(s32 *a0) {
    s32 *v1 = (s32 *)&D_800F116C;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)&D_800F1158;
    *v1 = 0x10008;
    func_80060A68();
}

extern s32 D_800F116C;
extern s32 D_800A3468;
extern s32 D_800F1178;
extern s32 D_800F1180;
extern u8 D_800F1154[];
void func_80060A68(void);
void func_800619F0(s32 *a0) {
    s32 *v1 = (s32 *)&D_800F116C;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)(D_800F1154 + 3);
    *v1 = 0x10007;
    func_80060A68();
}

INCLUDE_ASM("asm/funcs", func_80061A3C);
INCLUDE_ASM("asm/funcs", func_80061ACC);
INCLUDE_ASM("asm/funcs", func_80061C00);
INCLUDE_ASM("asm/funcs", func_80061D74);
INCLUDE_ASM("asm/funcs", func_80061EC0);
extern s32 D_800A34EC;
extern u8 D_8009BB74[];
void func_8007F35C(u16*, u8*);
void func_8007E8DC(u8*, u8*);
void func_8007EEEC(u8*);
void func_80061FAC(u16 *a0, s32 a1, u8 *a2) {
    u16 *v1 = a0;
    u16 *dest = (u16 *)D_800A34EC;
    u8 *s0 = a2;
    dest[0] = v1[0];
    dest[1] = v1[1];
    dest[2] = v1[2];
    func_8007F35C(dest, s0);
    *(s32 *)(s0 + 0x1C) = 0;
    *(s32 *)(s0 + 0x18) = 0;
    *(s32 *)(s0 + 0x14) = 0;
    func_8007E8DC(s0, D_8009BB74);
    func_8007EEEC(s0);
}
INCLUDE_ASM("asm/funcs", func_80062020);
INCLUDE_ASM("asm/funcs", func_800620B8);
INCLUDE_ASM("asm/funcs", func_8006288C);
INCLUDE_ASM("asm/funcs", func_8006295C);
INCLUDE_ASM("asm/funcs", func_80062FEC);
INCLUDE_ASM("asm/funcs", func_80063084);
extern s32 D_800A3468;
extern s32 D_800F10D0;
extern s16 D_800A345C;
u8 func_80063BD0(s32);
u8 func_80063AF0(void) {
    s32 *v1 = (s32 *)D_800A3468;
    D_800F10D0 = 1;
    D_800A345C = (*v1 >> 17) & 3;
    return func_80063BD0(0);
}
extern s32 D_800F10D4;
extern s16 D_800A345E;
u8 func_80063B34(void) {
    s32 *v1 = (s32 *)D_800A3468;
    D_800F10D4 = 1;
    D_800A345E = (*v1 >> 17) & 3;
    return func_80063BD0(1);
}
extern s32 D_800A3480;
extern s16 D_800A345C;
s32 func_80063E10(s32);
u8 func_80063B78(void) {
    *(s32 *)D_800A3480 = D_800A345C;
    return func_80063E10(0);
}
extern s16 D_800A345E;
u8 func_80063BA4(void) {
    *(s32 *)D_800A3480 = D_800A345E;
    return func_80063E10(1);
}
INCLUDE_ASM("asm/funcs", func_80063BD0);
INCLUDE_ASM("asm/funcs", func_80063E10);
INCLUDE_ASM("asm/funcs", func_800644FC);
INCLUDE_ASM("asm/funcs", func_800645B0);
INCLUDE_ASM("asm/funcs", func_800646E8);
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0CA0;
extern volatile s32 D_800F0CA4;
extern volatile s32 D_800F0CA8;
extern volatile s32 D_800F10E0;
extern volatile s16 D_800F0BA8;
void func_80064E90(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0CA0 = p[0];
    D_800F0CA4 = p[1];
    p = (volatile s32 *)p[2];
    D_800F10E0 = 1;
    D_800F0BA8 = 0;
    D_800F0CA8 = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0CAC;
extern volatile s32 D_800F0CB0;
extern volatile s32 D_800F0CB4;
extern volatile s32 D_800F10E4;
extern volatile s16 D_800F0BAA;
void func_80064ED8(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0CAC = p[0];
    D_800F0CB0 = p[1];
    p = (volatile s32 *)p[2];
    D_800F10E4 = 1;
    D_800F0BAA = 0;
    D_800F0CB4 = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0CB8;
extern volatile s32 D_800F0CBC;
extern volatile s32 D_800F0CC0;
extern volatile s32 D_800F10E8;
extern volatile s16 D_800F0BAC;
void func_80064F20(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0CB8 = p[0];
    D_800F0CBC = p[1];
    p = (volatile s32 *)p[2];
    D_800F10E8 = 1;
    D_800F0BAC = 0;
    D_800F0CC0 = (s32)p;
}
INCLUDE_ASM("asm/funcs", func_80064F68);
INCLUDE_ASM("asm/funcs", func_80064FB4);
INCLUDE_ASM("asm/funcs", func_80065000);
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0CE8;
extern volatile s32 D_800F0CEC;
extern volatile s32 D_800F0CF0;
extern volatile s32 D_800F1100;
extern volatile s16 D_800F0BB4;
void func_8006505C(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0CE8 = p[0];
    D_800F0CEC = p[1];
    p = (volatile s32 *)p[2];
    D_800F1100 = 1;
    D_800F0BB4 = 0;
    D_800F0CF0 = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0CF4;
extern volatile s32 D_800F0CF8;
extern volatile s32 D_800F0CFC;
extern volatile s32 D_800F1104;
extern volatile s16 D_800F0BB6;
void func_800650A4(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0CF4 = p[0];
    D_800F0CF8 = p[1];
    p = (volatile s32 *)p[2];
    D_800F1104 = 1;
    D_800F0BB6 = 0;
    D_800F0CFC = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0D18;
extern volatile s32 D_800F0D1C;
extern volatile s32 D_800F0D20;
extern volatile s32 D_800F1108;
extern volatile s16 D_800F0BBC;
void func_800650EC(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0D18 = p[0];
    D_800F0D1C = p[1];
    p = (volatile s32 *)p[2];
    D_800F1108 = 1;
    D_800F0BBC = 0;
    D_800F0D20 = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0D24;
extern volatile s32 D_800F0D28;
extern volatile s32 D_800F0D2C;
extern volatile s32 D_800F110C;
extern volatile s16 D_800F0BBE;
void func_80065134(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0D24 = p[0];
    D_800F0D28 = p[1];
    p = (volatile s32 *)p[2];
    D_800F110C = 1;
    D_800F0BBE = 0;
    D_800F0D2C = (s32)p;
}
INCLUDE_ASM("asm/funcs", func_8006517C);
INCLUDE_ASM("asm/funcs", func_800651F0);
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0D60;
extern volatile s32 D_800F0D64;
extern volatile s32 D_800F0D68;
extern volatile s32 D_800F1118;
extern volatile s16 D_800F0BC8;
void func_80065264(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0D60 = p[0];
    D_800F0D64 = p[1];
    p = (volatile s32 *)p[2];
    D_800F1118 = 1;
    D_800F0BC8 = 0;
    D_800F0D68 = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0D6C;
extern volatile s32 D_800F0D70;
extern volatile s32 D_800F0D74;
extern volatile s32 D_800F111C;
extern volatile s16 D_800F0BCA;
void func_800652AC(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0D6C = p[0];
    D_800F0D70 = p[1];
    p = (volatile s32 *)p[2];
    D_800F111C = 1;
    D_800F0BCA = 0;
    D_800F0D74 = (s32)p;
}
extern s16 D_800F0BA8;
u8 func_80065800(s32);
u8 func_800652F4(void) {
    u8 v0 = func_80065800(0);
    s16 *p = &D_800F0BA8;
    s16 v1 = *p;
    v1 += 0x1FF;
    *p = v1;
    if ((s16)v1 < 0x1001) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BAA;
u8 func_80065344(void) {
    u8 v0 = func_80065800(1);
    s16 *p = &D_800F0BAA;
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BAC;
u8 func_80065394(void) {
    u8 v0 = func_80065800(2);
    s16 *p = &D_800F0BAC;
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BAE;
u8 func_800653E4(void) {
    u8 v0 = func_80065800(3);
    s16 *p = &D_800F0BAE;
    s16 v1 = *p;
    v1 += 0x19;
    *p = v1;
    if ((s16)v1 < 0xC9) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BB0;
u8 func_80065434(void) {
    u8 v0 = func_80065800(4);
    s16 *p = &D_800F0BB0;
    s16 v1 = *p;
    v1 += 0x19;
    *p = v1;
    if ((s16)v1 < 0xC9) {
        return v0;
    }
    return 0;
}
INCLUDE_ASM("asm/funcs", func_80065484);
extern s16 D_800F0BB4;
u8 func_80065540(void) {
    u8 v0 = func_80065800(6);
    s16 *p = &D_800F0BB4;
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BB6;
u8 func_80065590(void) {
    u8 v0 = func_80065800(7);
    s16 *p = &D_800F0BB6;
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BBC;
u8 func_800655E0(void) {
    u8 v0 = func_80065800(0xA);
    s16 *p = &D_800F0BBC;
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BBE;
u8 func_80065630(void) {
    u8 v0 = func_80065800(0xB);
    s16 *p = &D_800F0BBE;
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}
extern s32 func_80065800(s32);
extern u16 D_800F0BC0;
extern s16 D_800F0BC4;
s32 func_80065680(void) {
    u16 *v1;
    s32 v0;
    func_80065800(0xC);
    v1 = &D_800F0BC0;
    v0 = *v1 + 1;
    *v1 = v0;
    v0 = func_80065800(0xE);
    D_800F0BC4 = D_800F0BC4 + 1;
    if ((s16)D_800F0BC4 < 11) {
        return v0 & 0xFF;
    }
    return 0;
}
extern u16 D_800F0BC2;
extern u16 D_800F0BC6;
s32 func_800656EC(void) {
    u16 *s0 = &D_800F0BC2;
    s32 v0;
    func_80065800(0xD);
    *s0 = *s0 + 1;
    v0 = func_80065800(0xF);
    D_800F0BC6 = D_800F0BC6 + 1;
    if ((s16)*s0 < 11) {
        return v0 & 0xFF;
    }
    return 0;
}
extern s16 D_800F0BC8;
u8 func_80065760(void) {
    u8 v0 = func_80065800(0x10);
    s16 *p = &D_800F0BC8;
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BCA;
u8 func_800657B0(void) {
    u8 v0 = func_80065800(0x11);
    s16 *p = &D_800F0BCA;
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}
INCLUDE_ASM("asm/funcs", func_80065800);
extern s32 D_800F10D8;
u8 func_80067200(s32, s32, s32);
u8 func_80066EC0(void) {
    u8 ret = func_80067200(0, 0, 0);
    D_800F10D8 = 1;
    return ret;
}
extern s32 D_800F10D8;
u8 func_80067200(s32, s32, s32);
u8 func_80066EF4(void) {
    u8 ret = func_80067200(0, 0, 1);
    D_800F10D8 = 2;
    return ret;
}
extern s32 D_800F10DC;
u8 func_80067200(s32, s32, s32);
u8 func_80066F28(void) {
    u8 ret = func_80067200(1, 1, 0);
    D_800F10DC = 1;
    return ret;
}
extern s32 D_800F10DC;
u8 func_80067200(s32, s32, s32);
u8 func_80066F5C(void) {
    u8 ret = func_80067200(1, 1, 1);
    D_800F10DC = 2;
    return ret;
}
extern s32 D_800F1120;
u8 func_80067200(s32, s32, s32);
u8 func_80066F90(void) {
    u8 ret = func_80067200(2, 1, 0);
    D_800F1120 = 1;
    return ret;
}
extern s32 D_800F1120;
u8 func_80067200(s32, s32, s32);
u8 func_80066FC4(void) {
    u8 ret = func_80067200(2, 1, 1);
    D_800F1120 = 2;
    return ret;
}
extern s32 D_800F1124;
u8 func_80067200(s32, s32, s32);
u8 func_80066FF8(void) {
    u8 ret = func_80067200(3, 0, 0);
    D_800F1124 = 1;
    return ret;
}
extern s32 D_800F1124;
u8 func_80067200(s32, s32, s32);
u8 func_8006702C(void) {
    u8 ret = func_80067200(3, 0, 1);
    D_800F1124 = 2;
    return ret;
}
extern s32 D_800F1128;
u8 func_80067200(s32, s32, s32);
u8 func_80067060(void) {
    u8 ret = func_80067200(4, 2, 0);
    D_800F1128 = 1;
    return ret;
}
extern s32 D_800F1128;
u8 func_80067200(s32, s32, s32);
u8 func_80067094(void) {
    u8 ret = func_80067200(4, 2, 1);
    D_800F1128 = 2;
    return ret;
}
extern s32 D_800F112C;
u8 func_80067200(s32, s32, s32);
u8 func_800670C8(void) {
    u8 ret = func_80067200(5, 3, 0);
    D_800F112C = 1;
    return ret;
}
extern s32 D_800F112C;
u8 func_80067200(s32, s32, s32);
u8 func_800670FC(void) {
    u8 ret = func_80067200(5, 3, 1);
    D_800F112C = 2;
    return ret;
}
extern s32 D_800F1130;
u8 func_80067200(s32, s32, s32);
u8 func_80067130(void) {
    u8 ret = func_80067200(6, 3, 0);
    D_800F1130 = 1;
    return ret;
}
extern s32 D_800F1130;
u8 func_80067200(s32, s32, s32);
u8 func_80067164(void) {
    u8 ret = func_80067200(6, 3, 1);
    D_800F1130 = 2;
    return ret;
}
extern s32 D_800F1134;
u8 func_80067200(s32, s32, s32);
u8 func_80067198(void) {
    u8 ret = func_80067200(7, 2, 0);
    D_800F1134 = 1;
    return ret;
}
extern s32 D_800F1134;
u8 func_80067200(s32, s32, s32);
u8 func_800671CC(void) {
    u8 ret = func_80067200(7, 2, 1);
    D_800F1134 = 2;
    return ret;
}
INCLUDE_ASM("asm/funcs", func_80067200);
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_800676C8(void) {
    func_800678A8(0, 0);
    func_80067D14(0, 0);
    return func_80068D88(0, 0);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_80067704(void) {
    func_800678A8(1, 1);
    func_80067D14(1, 1);
    return func_80068D88(1, 1);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_80067740(void) {
    func_800678A8(2, 1);
    func_80067D14(2, 1);
    return func_80068D88(2, 1);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_8006777C(void) {
    func_800678A8(3, 0);
    func_80067D14(3, 0);
    return func_80068D88(3, 0);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_800677B8(void) {
    func_800678A8(4, 2);
    func_80067D14(4, 2);
    return func_80068D88(4, 2);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_800677F4(void) {
    func_800678A8(5, 3);
    func_80067D14(5, 3);
    return func_80068D88(5, 3);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_80067830(void) {
    func_800678A8(6, 3);
    func_80067D14(6, 3);
    return func_80068D88(6, 3);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_8006786C(void) {
    func_800678A8(7, 2);
    func_80067D14(7, 2);
    return func_80068D88(7, 2);
}
INCLUDE_ASM("asm/funcs", func_800678A8);
INCLUDE_ASM("asm/funcs", func_80067D14);
INCLUDE_ASM("asm/funcs", func_80068D88);
INCLUDE_ASM("asm/funcs", func_80068ECC);
INCLUDE_ASM("asm/funcs", func_80068F70);
extern s32 D_800A3524;
extern s32 D_800A34FC;
extern s32 D_800A372C;
extern s32 D_800A351C;
void func_8006E8CC(s32);
s32 *func_80069120(s32 a0) {
    s32 *v0 = (s32 *)D_800A3524;
    u8 *v1 = (u8 *)D_800A34FC;
    if (v1[0x30] != (v0[8] & 1)) {
        func_8006E8CC(D_800A372C);
    }
    v0 = (s32 *)D_800A3524;
    v1 = (u8 *)D_800A34FC;
    v1[0x30] = (u8)(v0[8] & 1);
    return (s32 *)((u8 *)D_800A351C + a0 * 44);
}

void func_8006920C(s32 *, s32);
void func_8005C2A8(s32, s32, s32);
s32 func_8006919C(s32 *a0) {
    s32 i = 0;
    s32 *p = &a0[5];
    do {
        func_8006920C(a0, *p);
        p++;
        i++;
    } while (i < 12);
    func_8005C2A8(a0[0], 1, a0[1]);
    return a0[1];
}
void func_8006920C(s32 *a0, s32 a1) {
    s32 *p = (s32 *)a1;
    if (!*p) return;
    while (*p) {
        if (*p != -1) {
            *p = *p + (s32)a0;
        }
        p++;
    }
}
INCLUDE_ASM("asm/funcs", func_80069250);
INCLUDE_ASM("asm/funcs", func_800692C0);
INCLUDE_ASM("asm/funcs", func_800693CC);
INCLUDE_ASM("asm/funcs", func_80069898);
s32 *func_80077D00(void);
void func_80069A30(u8 *a0) {
    s32 *p = func_80077D00();
    s32 v0;
    if (p[8] & 1) {
        v0 = 0x22;
        a0[4] = (u8)v0;
        a0[5] = (u8)v0;
    } else {
        v0 = 0x4C;
        a0[4] = (u8)v0;
        a0[5] = (u8)v0;
        v0 = 0x6C;
    }
    a0[6] = (u8)v0;
}
s32 *func_80077D00(void);
void func_80069A8C(u8 *a0) {
    s32 *p = func_80077D00();
    s32 v0;
    if (p[8] & 1) {
        v0 = 8;
        a0[4] = (u8)v0;
        a0[5] = (u8)v0;
    } else {
        v0 = 0x31;
        a0[4] = 0;
        a0[5] = 0;
    }
    a0[6] = (u8)v0;
}
INCLUDE_ASM("asm/funcs", func_80069AE4);
INCLUDE_ASM("asm/funcs", func_80069E18);
INCLUDE_ASM("asm/funcs", func_80069F80);
INCLUDE_ASM("asm/funcs", func_8006A1A0);
INCLUDE_ASM("asm/funcs", func_8006A3CC);
INCLUDE_ASM("asm/funcs", func_8006A494);
INCLUDE_ASM("asm/funcs", func_8006A564);
INCLUDE_ASM("asm/funcs", func_8006A880);
INCLUDE_ASM("asm/funcs", func_8006B120);
INCLUDE_ASM("asm/funcs", func_8006B578);
INCLUDE_ASM("asm/funcs", func_8006B898);
INCLUDE_ASM("asm/funcs", func_8006B92C);
INCLUDE_ASM("asm/funcs", func_8006BB68);
INCLUDE_ASM("asm/funcs", func_8006BD28);
INCLUDE_ASM("asm/funcs", func_8006BEC4);
INCLUDE_ASM("asm/funcs", func_8006C168);
s32 func_8006C168(s32, s32);
s32 func_8006C1FC(s32 a0, s32 a1) {
    return func_8006C168(a0, a1);
}
INCLUDE_ASM("asm/funcs", func_8006C21C);
INCLUDE_ASM("asm/funcs", func_8006CBD4);
INCLUDE_ASM("asm/funcs", func_8006CCC8);
INCLUDE_ASM("asm/funcs", func_8006CFBC);
extern s32 D_800A34FC;
void func_8006D324(void) {
    s16 *v1 = (s16 *)D_800A34FC;
    v1[0x15] = 5;
    v1[0x14] = 5;
}
INCLUDE_ASM("asm/funcs", func_8006D338);
INCLUDE_ASM("asm/funcs", func_8006D3DC);
INCLUDE_ASM("asm/funcs", func_8006D5D4);
INCLUDE_ASM("asm/funcs", func_8006D74C);
extern s32 D_800A352C;
s32 func_8006D7FC(void) {
    D_800A352C = 0;
    return 1;
}
INCLUDE_ASM("asm/funcs", func_8006D808);
INCLUDE_ASM("asm/funcs", func_8006DD94);
INCLUDE_ASM("asm/funcs", func_8006DF68);
INCLUDE_ASM("asm/funcs", func_8006E068);
INCLUDE_ASM("asm/funcs", func_8006E10C);
INCLUDE_ASM("asm/funcs", func_8006E2A8);
extern s32 D_800A34FC;
extern s32 D_800A3520;
s32 *func_80069120(s32);
void func_8006E390(s32 *a0, s32 *a1) {
    s32 *s0 = a0;
    s32 *v0;
    a1[0]++;
    v0 = func_80069120(a1[0] & 1);
    s0[1] = ((s32 *)D_800A34FC)[9];
    s0[3] = v0[1];
    s0[2] = v0[0];
    s0[4] = v0[2];
    s0[5] = v0[4];
    s0[6] = v0[3];
    s0[7] = v0[5];
    s0[8] = v0[6];
    D_800A3520 = (s32)v0;
    s0[9] = v0[7];
}

void func_8006E440(s32 *a0) {
    s32 *p = a0;
    if (*p == -1) return;
    while (*p != -1) {
        *p = *p + (s32)a0;
        p++;
    }
}
s32 func_8006E480(u8 *a0, s32 a1) {
    s32 v0 = a0[0] & 0xFE1F;
    s32 v1 = a0[1] << 7;
    return v0 + v1 + a1;
}
INCLUDE_ASM("asm/funcs", func_8006E49C);
INCLUDE_ASM("asm/funcs", func_8006E534);
extern s32 D_800A35AC;
s32 func_8006E8AC(s32 a0) {
    return D_800A35AC + a0 * 44;
}
s32* func_80077D00(void);
void func_8007B33C(s32);
void func_8007B600(s16*, s32);
void func_8006E8CC(s32 *a0) {
    s32 *p;
    s32 data;
    s16 rect[4];
    p = func_80077D00();
    if (p[8] & 1) {
        data = a0[4];
    } else {
        data = a0[3];
    }
    rect[0] = 0;
    rect[1] = 0x1E0;
    rect[2] = 0x280;
    rect[3] = 0x20;
    func_8007B33C(0);
    func_8007B600(rect, data);
    func_8007B33C(0);
}
void func_8006E950(s32 *a0, s32 *a1) {
    s32 *s1 = a1;
    s32 s2;
    s32 s3;
    s32 s0;
    s32 s0_addr;
    s32 v0;
    s16 rect[4];

    s0_addr = (s32)a0;
    func_80036F40();
    v0 = func_80036EA8(2, s0_addr);
    func_80036D98(v0, (s32)s1);
    func_80036F40();
    s2 = 0x280;
    func_8006E440(s1);

    s3 = ((s32 *)((unsigned char *)s1 + 8))[0];
    s0 = 0x1DC;

    rect[0] = (s16)s2;
    rect[1] = 0;
    rect[2] = 0x180;
    rect[3] = (s16)s0;
    func_8007B33C(0);
    func_8007B600(rect, s3);

    rect[2] = 0x170;
    rect[0] = (s16)s2;
    rect[1] = (s16)s0;
    rect[3] = 0x24;
    func_8007B33C(0);
    func_8007B600(rect, s3 + 0x59400);

    func_8006E8CC(s1);
}
void func_8006920C(s32 *, s32);
void func_8005C2A8(s32, s32, s32);
s32 func_8006EA28(s32 *a0) {
    func_8006920C(a0, a0[21]);
    func_8006920C(a0, a0[22]);
    func_8006920C(a0, a0[23]);
    func_8006920C(a0, a0[24]);
    func_8006920C(a0, a0[25]);
    func_8006920C(a0, a0[26]);
    func_8006920C(a0, a0[27]);
    func_8006920C(a0, a0[28]);
    func_8006920C(a0, a0[29]);
    func_8005C2A8(a0[0], 1, a0[1]);
    return a0[1];
}
INCLUDE_ASM("asm/funcs", func_8006EACC);
INCLUDE_ASM("asm/funcs", func_8006EC0C);
INCLUDE_ASM("asm/funcs", func_8006ECF4);
INCLUDE_ASM("asm/funcs", func_8006F038);
INCLUDE_ASM("asm/funcs", func_8006F100);
INCLUDE_ASM("asm/funcs", func_8006F528);
INCLUDE_ASM("asm/funcs", func_8006F97C);
INCLUDE_ASM("asm/funcs", func_80070188);
INCLUDE_ASM("asm/funcs", func_80070C70);
INCLUDE_ASM("asm/funcs", func_80070F78);
extern u8 D_800A3561;
extern u8 D_8009BC7C[];
s32 func_80071C20(void) {
    s32 v1;
    v1 = 3;
    if (D_8009BC7C[D_800A3561] & 2) {
        v1 = 9;
    }
    return v1;
}
INCLUDE_ASM("asm/funcs", func_80071C4C);
extern s32 D_800A35A8;
void func_800720FC(s32, s32, s32);
void func_80072084(s32 a0) {
    s32 *v0 = (s32 *)D_800A35A8;
    func_800720FC(a0, v0[0x1A], 0);
}
void func_800720AC(s32 a0) {
    s32 *v0 = (s32 *)D_800A35A8;
    func_800720FC(a0, v0[0x1B], 1);
}
void func_800720D4(s32 a0) {
    s32 *v0 = (s32 *)D_800A35A8;
    func_800720FC(a0, v0[0x1C], 2);
}
INCLUDE_ASM("asm/funcs", func_800720FC);
INCLUDE_ASM("asm/funcs", func_80072BC4);
INCLUDE_ASM("asm/funcs", func_80072CD4);
INCLUDE_ASM("asm/funcs", func_80072E10);
void func_8007AAD0(s32);
void func_8007A968(s32, s32);
extern s32 D_800A374C;
void func_8007A8B4(s32, s32);
s32 *func_80072F30(s32 a0, u8 *a1) {
    func_8007AAD0((s32)a1);
    if (a0 < 4) {
        a1[4] = 0x9E;
        a1[5] = 0x64;
        a1[6] = 0;
        func_8007A968((s32)a1, 1);
    } else {
        a1[4] = 0x28;
        a1[5] = 0x28;
        a1[6] = 0x18;
        func_8007A968((s32)a1, 0);
    }
    func_8007A8B4(D_800A374C + 0x5C, (s32)a1);
    return (s32 *)(a1 + 0x10);
}
INCLUDE_ASM("asm/funcs", func_80072FCC);
INCLUDE_ASM("asm/funcs", func_80073060);
INCLUDE_ASM("asm/funcs", func_80073200);
INCLUDE_ASM("asm/funcs", func_8007352C);
INCLUDE_ASM("asm/funcs", func_80073728);
INCLUDE_ASM("asm/funcs", func_80073C78);
INCLUDE_ASM("asm/funcs", func_80074220);
INCLUDE_ASM("asm/funcs", func_80074488);
INCLUDE_ASM("asm/funcs", func_800747D8);
INCLUDE_ASM("asm/funcs", func_80074B18);
INCLUDE_ASM("asm/funcs", func_80074D2C);
INCLUDE_ASM("asm/funcs", func_80074E08);
INCLUDE_ASM("asm/funcs", func_8007526C);
INCLUDE_ASM("asm/funcs", func_800753D8);
INCLUDE_ASM("asm/funcs", func_80075670);
INCLUDE_ASM("asm/funcs", func_80075830);
INCLUDE_ASM("asm/funcs", func_800759D0);
INCLUDE_ASM("asm/funcs", func_80075F80);
INCLUDE_ASM("asm/funcs", func_8007636C);
INCLUDE_ASM("asm/funcs", func_800768DC);
INCLUDE_ASM("asm/funcs", func_80076D74);
void func_8006920C(s32 *, s32);
s32 func_80076FF8(s32 *a0) {
    func_8006920C(a0, a0[5]);
    func_8006920C(a0, a0[6]);
    func_8006920C(a0, a0[7]);
    func_8006920C(a0, a0[8]);
    func_8006920C(a0, a0[9]);
    func_8006920C(a0, a0[10]);
    func_8006920C(a0, a0[11]);
    func_8006920C(a0, a0[12]);
    func_8006920C(a0, a0[13]);
    func_8006920C(a0, a0[14]);
    return a0[1];
}
extern s32 D_800A35D8;
s32 func_80077098(s32 a0) {
    return D_800A35D8 + a0 * 44;
}
INCLUDE_ASM("asm/funcs", func_800770B8);
INCLUDE_ASM("asm/funcs", func_80077374);
INCLUDE_ASM("asm/funcs", func_80077724);
extern s32 D_800A35E4;
void func_80068F70(s32, s32 *);
extern s32 D_8009BD24;
s32 func_80016768(s32, s32, s32, s32);
s32 func_80077820(s32 a0) {
    func_80068F70(a0, (s32 *)&D_8009BD24);
    func_80016768(1, 0, 0, 0);
    D_800A35E4 = 0;
    return 1;
}

extern s32 D_800A35E4;
s32 func_80069250(void);
s32 func_80077860(void) {
    if (func_80069250() == 1) {
        D_800A35E4 = 0;
        return 1;
    }
    return 0;
}
INCLUDE_ASM("asm/funcs", func_80077894);
INCLUDE_ASM("asm/funcs", func_80077904);
INCLUDE_ASM("asm/funcs", func_80077940);
extern s32 D_800A35E0;
extern s32 D_800A35E8;
extern u8 D_8009BD24[];
void func_8006E534(s32, s32, u8*, s32);
s32 func_80077984(s32 a0) {
    func_8006E534(a0, D_800A35E0, D_8009BD24, D_800A35E8);
    func_80016768(1, 0, 0, 0);
    return 1;
}
s32 func_8006EACC(void);
void func_8005B6FC(void);
s32 func_800779C8(void) {
    s32 ret = func_8006EACC();
    if (ret) {
        func_8005B6FC();
    }
    return ret;
}
extern s32 D_800A35E4;
void func_8006D74C(s32, s32);
void func_80077A04(s32 a0, s32 a1) {
    D_800A35E4 = 0;
    func_8006D74C(a0, a1);
}
extern s32 D_800A35E4;
void func_80016768(s32, s32, s32, s32);
s32 func_8006D7FC(void);
void func_80077A28(void) {
    D_800A35E4 = 0;
    func_80016768(1, 0, 0, 0);
    func_8006D7FC();
}
void func_8006E068(void);
void func_80077A60(void) {
    func_8006E068();
}
extern s32 D_800A35E8;
extern s32 D_8009BD24;
void func_800770B8(s32, s32 *, s32);
s32 func_80016768(s32, s32, s32, s32);
s32 func_80077A80(s32 a0) {
    func_800770B8(a0, (s32 *)&D_8009BD24, D_800A35E8);
    func_80016768(1, 0, 0, 0);
    return 1;
}

void func_80077724(void);
void func_80077AC0(void) {
    func_80077724();
}
void func_8006E10C(void);
void func_80077AE0(void) {
    func_8006E10C();
}
void func_8006E2A8(void);
void func_80077B00(void) {
    func_8006E2A8();
}
extern s32 D_800A35E4;
void func_80077B20(void) {
    D_800A35E4 = 1;
}
INCLUDE_ASM("asm/funcs", func_80077B30);
extern s32 D_8009BD24;
s32* func_80077D00(void) {
    return &D_8009BD24;
}
void func_8006920C(s32*, s32);
s32 func_80077D10(s32 *a0) {
    func_8006920C(a0, a0[6]);
    func_8006920C(a0, a0[7]);
    func_8006920C(a0, a0[8]);
    func_8006920C(a0, a0[9]);
    func_8006920C(a0, a0[10]);
    return a0[1];
}
extern s32 D_800A35F4;
s32 func_80077D74(s32 a0) {
    return D_800A35F4 + a0 * 44;
}
INCLUDE_ASM("asm/funcs", func_80077D94);
INCLUDE_ASM("asm/funcs", func_800784E4);
INCLUDE_ASM("asm/funcs", func_8007855C);
s32 func_80078628(s32 *a0) {
    return a0[1];
}
extern s32 D_800A360C;
s32 func_80078634(s32 a0) {
    return D_800A360C + a0 * 44;
}
INCLUDE_ASM("asm/funcs", func_80078654);
INCLUDE_ASM("asm/funcs", func_80078824);
extern s32 D_800A3304;
extern s32 D_800A3608;
s32 *func_80078634(s32);
void func_80078654(s32 *);
s32 func_800788B0(void) {
    s32 buf[8];
    s32 *v0;
    D_800A3304++;
    v0 = func_80078634(D_800A3304 & 1);
    buf[0] = v0[0];
    buf[2] = v0[2];
    buf[3] = v0[4];
    buf[4] = v0[3];
    buf[5] = v0[5];
    buf[6] = v0[6];
    buf[7] = v0[7];
    func_80078654(buf);
    D_800A3608++;
    return D_800A3608 >= 0xB40;
}

__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078948\n"
    "addiu $t2, $zero, 0xA0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x43\n"
    "endlabel func_80078948\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078948 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078958\n"
    "addiu $t2, $zero, 0xA0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x70\n"
    "endlabel func_80078958\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078958 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078968\n"
    "addiu $t2, $zero, 0xA0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x9F\n"
    "endlabel func_80078968\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078968 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078978\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x8\n"
    "endlabel func_80078978\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078978 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078988\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x9\n"
    "endlabel func_80078988\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078988 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078998\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0xB\n"
    "endlabel func_80078998\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078998 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800789A8\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0xC\n"
    "endlabel func_800789A8\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_800789A8 */
INCLUDE_ASM("asm/funcs", func_800789B8);
INCLUDE_ASM("asm/funcs", func_800789C8);
INCLUDE_ASM("asm/funcs", func_800789D8);
PAD_NOPS_1; /* padding after func_800789D8 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800789E8\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x32\n"
    "endlabel func_800789E8\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_800789E8 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800789F8\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x34\n"
    "endlabel func_800789F8\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_800789F8 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078A08\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x35\n"
    "endlabel func_80078A08\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078A08 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078A18\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x36\n"
    "endlabel func_80078A18\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078A18 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078A28\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x41\n"
    "endlabel func_80078A28\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078A28 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078A38\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x42\n"
    "endlabel func_80078A38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078A38 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078A48\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x43\n"
    "endlabel func_80078A48\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078A48 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078A58\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x5B\n"
    "endlabel func_80078A58\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078A58 */
INCLUDE_ASM("asm/funcs", func_80078A68);
INCLUDE_ASM("asm/funcs", func_80078B04);
INCLUDE_ASM("asm/funcs", func_80078B3C);
INCLUDE_ASM("asm/funcs", func_80078B70);
INCLUDE_ASM("asm/funcs", func_80078BA8);
extern s32 D_8009BD80;
void func_80078BE0(s32 a0) {
    D_8009BD80 = a0;
}
extern s32 D_8009BD80;
s32 func_80078BF0(void) {
    return D_8009BD80;
}
void func_800790C0(void);
void func_800789B8(void);
void func_80078F88(void);
void func_800789C8(void);
void func_80078A58(s32);
s32 func_80078DA0(void);
void func_80078F30(s32, s32, s32, s32);
void func_80079028(void);
extern s32 D_8009BD80;
void func_80078C00(s32 a0, s32 a1, s32 a2, s32 a3) {
    func_800790C0();
    func_800789B8();
    func_80078F88();
    func_800789C8();
    func_80078A58(0);
    func_80078DA0();
    func_80078F30(a0, a1, a2, a3);
    func_80079028();
    D_8009BD80 = 1;
}
void func_800790C0(void);
void func_800789B8(void);
void func_80078F88(void);
void func_800789C8(void);
void func_80078A58(s32);
s32 func_80078DA0(void);
void func_80078F00(s32, s32, s32, s32);
void func_80079028(void);
extern s32 D_8009BD80;
void func_80078C9C(s32 a0, s32 a1, s32 a2, s32 a3) {
    func_800790C0();
    func_800789B8();
    func_80078F88();
    func_800789C8();
    func_80078A58(0);
    func_80078DA0();
    func_80078F00(a0, a1, a2, a3);
    func_80079028();
    D_8009BD80 = 1;
}
void func_80078F10(void);
void func_80078A58(s32);
void func_80078F60(void);
void func_80078D38(void) {
    func_80078F10();
    func_80078A58(0);
    func_80078F60();
}
extern s32 D_8009BD80;
void func_80078F74(void);
void func_80078F20(void);
s32 func_80078E20(void);
void func_80078D68(void) {
    func_80078F74();
    func_80078F20();
    func_80078E20();
    D_8009BD80 = 0;
}
extern void func_800789B8(void);
extern void func_800789C8(void);
extern void func_80078F50(s32, u32 *);
extern void func_80078F40(s32, u32 *);
extern void func_80078E58(void);
extern s32 func_80078EC0(void);
extern u32 D_800F183C;
extern u32 D_800F1840;
extern u32 D_800F1838;
extern u32 D_800F1844;
s32 func_80078DA0(void) {
    u32 *v1 = &D_800F183C;
    u32 *s0 = v1 - 1;
    func_800789B8();
    *v1 = (u32)func_80078E58;
    D_800F1840 = (u32)func_80078EC0;
    D_800F1838 = 0;
    D_800F1844 = 0;
    func_80078F50(1, s0);
    func_80078F40(1, s0);
    func_800789C8();
    return 1;
}
void func_800789B8(void);
void func_80078F50(s32, s16*);
void func_800789C8(void);
extern s16 D_800F1838;
s32 func_80078E20(void) {
    func_800789B8();
    func_80078F50(1, &D_800F1838);
    func_800789C8();
    return 1;
}
INCLUDE_ASM("asm/funcs", func_80078E58);
INCLUDE_ASM("asm/funcs", func_80078EC0);
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F00\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x12\n"
    "endlabel func_80078F00\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078F00 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F10\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x13\n"
    "endlabel func_80078F10\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078F10 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F20\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x14\n"
    "endlabel func_80078F20\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078F20 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F30\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x15\n"
    "endlabel func_80078F30\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078F30 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F40\n"
    "addiu $t2, $zero, 0xC0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x2\n"
    "endlabel func_80078F40\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078F40 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F50\n"
    "addiu $t2, $zero, 0xC0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x3\n"
    "endlabel func_80078F50\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078F50 */
INCLUDE_ASM("asm/funcs", func_80078F60);
INCLUDE_ASM("asm/funcs", func_80078F74);
INCLUDE_ASM("asm/funcs", func_80078F88);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078FF0\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x44\n"
    "    nop\n"
    "    lui   $t1, %hi(D_800A362C)\n"
    "    lw    $t1, %lo(D_800A362C)($t1)\n"
    "    addiu $sp, $sp, -24\n"
    "    sw    $ra, 20($sp)\n"
    "    jalr  $t1\n"
    "    nop\n"
    "    lw    $ra, 20($sp)\n"
    "    addiu $sp, $sp, 24\n"
    "    jr    $ra\n"
    "    nop\n"
    "endlabel func_80078FF0\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
INCLUDE_ASM("asm/funcs", func_80079028);
INCLUDE_ASM("asm/funcs", func_800790A4);
PAD_NOPS_3; /* padding after func_800790A4 */
PAD_NOPS_3; /* padding after func_800790A4 */
INCLUDE_ASM("asm/funcs", func_800790C0);
PAD_NOPS_1; /* padding after func_800790C0 */
u8* func_80079120(u8 *dst, u8 *src, s32 len) {
    u8 *ret;
    if (!dst) {
        return 0;
    }
    ret = dst;
    while (len > 0) {
        *dst = *src;
        src++;
        len--;
        dst++;
    }
    return ret;
}
extern u32 D_800F1848;
s32 func_80079154(void) {
    D_800F1848 = D_800F1848 * 0x41C64E6D + 0x3039;
    return (D_800F1848 >> 16) & 0x7FFF;
}
extern s32 D_800F1848;
void func_80079184(s32 a0) {
    D_800F1848 = a0;
}
u8 *func_80079194(u8 *a0, u8 *a1) {
    u8 *v1;
    if (!a0) {
        return 0;
    }
    if (!a1) {
        return 0;
    }
    v1 = a0;
    while ((*a0++ = *a1++) != 0) {
    }
    return v1;
}
s32 func_800791D8(u8 *a0) {
    s32 v1 = 0;
    if (!a0) {
        return 0;
    }
    while (*a0++ != 0) {
        v1++;
    }
    return v1;
}
INCLUDE_ASM("asm/funcs", func_80079208);
INCLUDE_ASM("asm/funcs", func_80079244);
extern u8 D_8009BD8D;
u8 func_800798CC(u8 a0) {
    u8 c = a0;
    if ((&D_8009BD8D)[c] & 2) {
        c = a0 - 0x20;
    }
    return c;
}
extern u8 D_8009BD8D;
u8 func_800798FC(u8 a0) {
    u8 c = a0;
    if ((&D_8009BD8D)[c] & 1) {
        c = a0 + 0x20;
    }
    return c;
}
INCLUDE_ASM("asm/funcs", func_8007992C);
INCLUDE_ASM("asm/funcs", func_8007997C);
INCLUDE_ASM("asm/funcs", func_80079A30);
