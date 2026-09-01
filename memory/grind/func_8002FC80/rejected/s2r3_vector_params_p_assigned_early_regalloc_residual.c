typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;

typedef struct VECTOR  { s32 vx, vy, vz, pad; } VECTOR;

extern s32 ratan2(s32, s32);

s32 func_8002FC80(VECTOR *a0, VECTOR *a1, VECTOR *a2) {
    s32 v1, v2;
    s32 *p;
    s32 ret;

    /* Difference vectors (a1-a0) and (a2-a0) into the two scratchpad
     * VECTOR slots the GTE macros below read back. */
    v1 = a1->vx;
    v2 = a0->vx;
    ((VECTOR *)0x1F800360)->vx = v1 - v2;

    v1 = a1->vy;
    v2 = a0->vy;
    ((VECTOR *)0x1F800360)->vy = v1 - v2;

    v1 = a1->vz;
    v2 = a0->vz;
    ((VECTOR *)0x1F800360)->vz = v1 - v2;

    v1 = a2->vx;
    v2 = a0->vx;
    ((VECTOR *)0x1F800370)->vx = v1 - v2;

    v1 = a2->vy;
    v2 = a0->vy;
    ((VECTOR *)0x1F800370)->vy = v1 - v2;

    v1 = a2->vz;
    v2 = a0->vz;
    ((VECTOR *)0x1F800370)->vz = v1 - v2;

    p = (s32 *)0x1F800380;

    /* PsyQ libgte inline macro gte_SetRotMatrix(r) - loads the 3 packed
     * rotation-matrix words at r into cop2 control regs R11R12/R13R21/R22R23.
     * The SDK macro body hardcodes $12-$15 and copies the operand into $12. */
    __asm__ volatile(
        "move   $12, %0
"
        "lw     $13, 0($12)
"
        "lw     $14, 4($12)
"
        "ctc2   $13, $0
"
        "lw     $15, 8($12)
"
        "ctc2   $14, $2
"
        "ctc2   $15, $4
"
        :: "r"((VECTOR *)0x1F800360) : "$12", "$13", "$14", "$15");
    /* PsyQ libgte inline macro gte_ldlvl(r) - load long vector at r into
     * IR1/IR2/IR3 ($9/$10/$11), IR3 first, then the 2-cycle GTE load delay. */
    __asm__ volatile(
        "move   $12, %0
"
        "lwc2   $11, 8($12)
"
        "lwc2   $9, 0($12)
"
        "lwc2   $10, 4($12)
"
        "nop
"
        "nop
"
        :: "r"((VECTOR *)0x1F800370) : "$12");
    /* GTE OP - cop2 command 0x0170000C. */
    __asm__ volatile(".word 0x4B70000C");
    /* PsyQ libgte inline macro gte_stlvnl(r) - store MAC1/MAC2/MAC3. */
    __asm__ volatile(
        "move   $12, %0
"
        "swc2   $25, 0($12)
"
        "swc2   $26, 4($12)
"
        "swc2   $27, 8($12)
"
        :: "r"(p) : "$12");
    ret = ratan2(p[0], p[2]);
    if (p[1] > 0) {
        ret += 0x800;
    }
    return ret;
}
