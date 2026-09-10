/* REJECTED s1b 2026-09-10 — thin-island probe: the second gte_ldlv0 VX0/VY0
 * pack written in C with single-insn cop2 islands. sandbox --disable all = 7
 * (181/182) vs 0 for candidate.c: GCC seats the lhu pair in $v0/$v1 off the
 * folded $s0 base and drops the redundant `addu $t4,$s3,$zero` copy; the
 * target's $t4/$t5/$t6 island GPRs are hand-asm (cluster rule :159-160).
 * Kill scope: instance (this island, this chassis, zero FAKE constructs). */
void func_8002EBDC(s16 *vec_in, s16 *dir, s32 *out, s32 scale_z, s32 scale_xy) {
    s32 sp_tmp;
    u8 *scr = (u8 *)0x1F8002B8;
    s32 *mat;
    s32 *vec;
    s32 angle;
    s32 dist_sq;
    s32 dist;

    angle = ratan2(dir[0], dir[2]);
    *(s16 *)(scr + 0xFA) = 0x800 - angle;
    dist_sq = dir[0] * dir[0] + dir[2] * dir[2];

    if ((u32)dist_sq < 0x400) {
        dist = (u32)*(((u8 *)&D_8008D118) + dist_sq) >> 3;
    } else {
        s32 lzcr = 0;
        if (dist_sq >= 0) {
            /* Hand-written GTE leading-zero-count block (LZCS in, LZCR out) —
             * canonical inline asm, identical to the user-authorized block in
             * the matched sibling func_8002E838 (this file) / func_8001A67C. */
            __asm__ volatile(
                "addu   $t4, %1, $zero\n"
                "mtc2   $t4, $30\n"        /* LZCS <- dist_sq */
                "nop\n"
                "nop\n"
                "addiu  $v0, $sp, 0x10\n"  /* &sp_tmp */
                "addu   $t4, $v0, $zero\n"
                "swc2   $31, 0($t4)\n"     /* sp_tmp <- LZCR */
                : "=m"(sp_tmp)
                : "r"(dist_sq)
                : "$2", "$12");
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            s32 tbl = *(((u8 *)&D_8008D118) + ((u32)dist_sq >> shift));
            dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }

    angle = ratan2(dir[1], dist);
    mat = (s32 *)(scr + 0xD8);
    *(s16 *)(scr + 0xF8) = 0x800 - angle;

    /* identity 3x3 rotation matrix at scr+0xD8 */
    *(s16 *)(scr + 0xD8) = 0x1000;
    *(s16 *)(scr + 0xDA) = 0;
    *(s16 *)(scr + 0xDC) = 0;
    *(s16 *)(scr + 0xDE) = 0;
    *(s16 *)(scr + 0xE0) = 0x1000;
    *(s16 *)(scr + 0xE2) = 0;
    *(s16 *)(scr + 0xE4) = 0;
    *(s16 *)(scr + 0xE6) = 0;
    *(s16 *)(scr + 0xE8) = 0x1000;
    RotMatrixY(*(s16 *)(scr + 0xFA), mat);
    RotMatrixX(*(s16 *)(scr + 0xF8), mat);

    /* PsyQ libgte inline macro gte_SetRotMatrix(r) --- loads the 5 packed
     * rotation-matrix words at r into cop2 control regs $0..$4.  Same island
     * as the matched func_8002E838 (this file) / func_800203B4. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(mat) : "$12", "$13", "$14", "$15");
    /* PsyQ 4.5 inline_c.h:101-110 macro gte_ldlv0(r) --- pack VX0/VY0 into
     * one word (lhu/lhu/sll/or is the macro's own text), mtc2 to $0, lwc2 VZ0
     * into $1, then the 2-cycle GTE load delay.  Same island as func_8002E838. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(vec_in) : "$12", "$13", "$14");
    /* gte_rtv0: GTE MVMVA sf=1, mx=rotation, v=V0, cv=none --- cop2 command
     * 0x0486012 (.word 0x4A486012). */
    __asm__ volatile(".word 0x4A486012");
    vec = (s32 *)(scr + 0xA8);
    /* PsyQ libgte inline macro gte_stlvnl(r) --- store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r (the rotated input vector, in scratchpad). */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(vec) : "$12", "memory");

    /* scale the rotated vector: z by scale_z, x/y by scale_xy, /256 */
    vec[2] = (vec[2] * scale_z) / 256;
    vec[0] = (vec[0] * scale_xy) / 256;
    vec[1] = (vec[1] * scale_xy) / 256;

    /* identity again, then the inverse rotation (negated angles) */
    *(s16 *)(scr + 0xD8) = 0x1000;
    *(s16 *)(scr + 0xDA) = 0;
    *(s16 *)(scr + 0xDC) = 0;
    *(s16 *)(scr + 0xDE) = 0;
    *(s16 *)(scr + 0xE0) = 0x1000;
    *(s16 *)(scr + 0xE2) = 0;
    *(s16 *)(scr + 0xE4) = 0;
    *(s16 *)(scr + 0xE6) = 0;
    *(s16 *)(scr + 0xE8) = 0x1000;
    RotMatrixX(-*(s16 *)(scr + 0xF8), mat);
    RotMatrixY(-*(s16 *)(scr + 0xFA), mat);

    /* Second GTE pass: the same four macro islands (gte_SetRotMatrix, gte_ldlv0
     * on the scaled scratchpad vector, gte_rtv0, gte_stlvnl into `out`). */
    /* PsyQ libgte inline macro gte_SetRotMatrix(r). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(mat) : "$12", "$13", "$14", "$15");
    /* PROBE s1: the ldlv0 VX0/VY0 pack written in C, cop2 transfers as
     * single-insn islands (measures the "C-expressible islands" branch). */
    {
        u32 packed = *(u16 *)vec | ((u32)*(u16 *)((u8 *)vec + 4) << 16);
        __asm__ volatile("mtc2 %0, $0" :: "r"(packed));
        __asm__ volatile("lwc2 $1, 8(%0)" :: "r"(vec));
        __asm__ volatile("nop");
        __asm__ volatile("nop");
    }
    /* gte_rtv0 (MVMVA, cop2 0x0486012). */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ libgte inline macro gte_stlvnl(r) --- MAC1..3 into the caller's out. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(out) : "$12", "memory");
}
