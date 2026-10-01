static inline s32 isqrt(s32 x) {
    s32 slot;
    s32 lzcr;
    s32 shift;
    if ((u32)x < 0x400) {
        return (u32)g_sqrt_table_u8[x] >> 3;
    }
    lzcr = 0;
    if (x >= 0) {
        __asm__ volatile(
            "move   $12, %0
"
            "mtc2   $12, $30
"
            "nop
"
            "nop
"
            :: "r"(x) : "$12");
        __asm__ volatile(
            "move   $12, %0
"
            "swc2   $31, 0($12)
"
            :: "r"(&slot) : "$12", "memory");
        lzcr = slot;
    }
    shift = 0x16 - (lzcr & ~1);
    x = g_sqrt_table_u8[(u32)x >> shift];
    return (u32)(x << 16) >> (0x13 - ((u32)shift >> 1));
}
s32 func_8002CD58(u8 *obj) {
    s32 len_sq;
    s32 xz_sq;
    s32 nxz_sq;
    s32 yaw;
    s32 pitch;
    s32 nyaw;
    s32 npitch;
    s32 dist;
    s32 xz_dist;
    s32 nxz_dist;

    *(s32 *)(obj + 0xA8) = (*(s32 **)(obj + 0x64))[0] - (*(s32 **)(obj + 0x60))[0];
    *(s32 *)(obj + 0xAC) = (*(s32 **)(obj + 0x64))[1] - (*(s32 **)(obj + 0x60))[1];
    *(s32 *)(obj + 0xB0) = (*(s32 **)(obj + 0x64))[2] - (*(s32 **)(obj + 0x60))[2];
    *(s32 *)(obj + 0xB8) = (*(s32 **)(obj + 0x68))[0] - (*(s32 **)(obj + 0x60))[0];
    *(s32 *)(obj + 0xBC) = (*(s32 **)(obj + 0x68))[1] - (*(s32 **)(obj + 0x60))[1];
    *(s32 *)(obj + 0xC0) = (*(s32 **)(obj + 0x68))[2] - (*(s32 **)(obj + 0x60))[2];

    /* gte_ldopv1(a) -- inline_o.h:595: a into the rotation matrix
     * diagonal (cop2 control $0/$2/$4). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "lw     $15, 8($12)\n"
        "ctc2   $14, $2\n"
        "ctc2   $15, $4\n"
        :: "r"(obj + 0xA8) : "$12", "$13", "$14", "$15");
    /* gte_ldopv2(b) -- inline_o.h:626: b into IR1-IR3, then gte_op0's
     * two nops (inline_o.h:1866). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $11, 8($12)\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(obj + 0xB8) : "$12");
    /* gte_op0() command word -- inline_o.h:1866, OP sf=0. */
    __asm__ volatile(".word 0x4B70000C");
    /* gte_stlvnl(n) -- inline_o.h:2422: MAC1-MAC3 to obj+0xC8. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(obj + 0xC8) : "$12", "memory");

    if ((u32)(*(s32 *)(obj + 0xC8) + 0x3FFF) < 0x7FFF
        && (u32)(*(s32 *)(obj + 0xCC) + 0x3FFF) < 0x7FFF
        && (u32)(*(s32 *)(obj + 0xD0) + 0x3FFF) < 0x7FFF) {
        /* gte_sqr0() -- inline_o.h:1749: square IR1-IR3 (still n). */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4AA00428\n");
        /* gte_stlvnl -- inline_o.h:2422: the squares to obj+0x100. */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            :: "r"(obj + 0x100) : "$12", "memory");
        len_sq = *(s32 *)(obj + 0x100) + *(s32 *)(obj + 0x104) + *(s32 *)(obj + 0x108);
        dist = isqrt(len_sq);
        if ((u32)dist < 0x4000) {
            yaw = ratan2(*(s32 *)(obj + 0xA8), *(s32 *)(obj + 0xB0));
            xz_sq = *(s32 *)(obj + 0xA8) * *(s32 *)(obj + 0xA8)
                  + *(s32 *)(obj + 0xB0) * *(s32 *)(obj + 0xB0);
            *(s16 *)(obj + 0xFA) = 0x800 - yaw;
            xz_dist = isqrt(xz_sq);
            pitch = ratan2(*(s32 *)(obj + 0xAC), xz_dist);
            *(s16 *)(obj + 0xF8) = 0x800 - pitch;
            *(s16 *)(obj + 0xD8) = 0x1000;
            *(s16 *)(obj + 0xDA) = 0;
            *(s16 *)(obj + 0xDC) = 0;
            *(s16 *)(obj + 0xDE) = 0;
            *(s16 *)(obj + 0xE0) = 0x1000;
            *(s16 *)(obj + 0xE2) = 0;
            *(s16 *)(obj + 0xE4) = 0;
            *(s16 *)(obj + 0xE6) = 0;
            *(s16 *)(obj + 0xE8) = 0x1000;
            RotMatrixY(*(s16 *)(obj + 0xFA), (s32 *)(obj + 0xD8));
            RotMatrixX(*(s16 *)(obj + 0xF8), (s32 *)(obj + 0xD8));
            /* gte_SetRotMatrix(obj+0xD8) -- inline_o.h:860: the five
             * packed rotation-matrix words into cop2 control $0..$4. */
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
                :: "r"(obj + 0xD8) : "$12", "$13", "$14", "$15");
            /* gte_ldlv0(a) -- inline_o.h:277: pack VX0/VY0, VZ0 via lwc2,
             * then gte_rtv0's two nops (inline_o.h:1353). */
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
                :: "r"(obj + 0xA8) : "$12", "$13", "$14");
            /* gte_rtv0() command word -- inline_o.h:1353, MVMVA sf=1,
             * rotation x V0. */
            __asm__ volatile(".word 0x4A486012");
            /* gte_stlvnl(a) -- inline_o.h:2422: rotated a written back. */
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $25, 0($12)\n"
                "swc2   $26, 4($12)\n"
                "swc2   $27, 8($12)\n"
                :: "r"(obj + 0xA8) : "$12", "memory");
            /* gte_ldlv0(b) (inline_o.h:277) / gte_rtv0() (:1353) /
             * gte_stlvnl(b) (:2422): same for b. */
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
                :: "r"(obj + 0xB8) : "$12", "$13", "$14");
            __asm__ volatile(".word 0x4A486012");
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $25, 0($12)\n"
                "swc2   $26, 4($12)\n"
                "swc2   $27, 8($12)\n"
                :: "r"(obj + 0xB8) : "$12", "memory");
            return 0;
        }
    }

    nyaw = ratan2(*(s32 *)(obj + 0xC8), *(s32 *)(obj + 0xD0));
    *(s32 *)(obj + 0xC8) >>= 6;
    *(s32 *)(obj + 0xCC) >>= 6;
    *(s32 *)(obj + 0xD0) >>= 6;
    nxz_sq = *(s32 *)(obj + 0xC8) * *(s32 *)(obj + 0xC8)
           + *(s32 *)(obj + 0xD0) * *(s32 *)(obj + 0xD0);
    *(s16 *)(obj + 0xFA) = 0x800 - nyaw;
    nxz_dist = isqrt(nxz_sq);
    npitch = ratan2(*(s32 *)(obj + 0xCC), nxz_dist);
    *(s16 *)(obj + 0xF8) = 0x800 - npitch;
    *(s16 *)(obj + 0xD8) = 0x1000;
    *(s16 *)(obj + 0xDA) = 0;
    *(s16 *)(obj + 0xDC) = 0;
    *(s16 *)(obj + 0xDE) = 0;
    *(s16 *)(obj + 0xE0) = 0x1000;
    *(s16 *)(obj + 0xE2) = 0;
    *(s16 *)(obj + 0xE4) = 0;
    *(s16 *)(obj + 0xE6) = 0;
    *(s16 *)(obj + 0xE8) = 0x1000;
    RotMatrixY(*(s16 *)(obj + 0xFA), (s32 *)(obj + 0xD8));
    RotMatrixX(*(s16 *)(obj + 0xF8), (s32 *)(obj + 0xD8));
    /* gte_SetRotMatrix (inline_o.h:860), then gte_ldlv0 (:277) / gte_rtv0
     * (:1353) / gte_stlvnl (:2422) for a and for b: the same islands as the
     * first path. */
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
        :: "r"(obj + 0xD8) : "$12", "$13", "$14", "$15");
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
        :: "r"(obj + 0xA8) : "$12", "$13", "$14");
    __asm__ volatile(".word 0x4A486012");
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(obj + 0xA8) : "$12", "memory");
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
        :: "r"(obj + 0xB8) : "$12", "$13", "$14");
    __asm__ volatile(".word 0x4A486012");
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(obj + 0xB8) : "$12", "memory");
    return 1;
}
