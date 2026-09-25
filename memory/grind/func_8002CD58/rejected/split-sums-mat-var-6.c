s32 func_8002CD58(u8 *obj) {
    s32 sp_tmp;
    s32 sp_tmp2;
    s32 sp_tmp3;
    s32 *mat;
    s32 *vec;
    s32 sum1;
    s32 sum2;
    s32 sum3;
    s32 angle;
    s32 dist;

    *(s32 *)(obj + 0xA8) = (*(s32 **)(obj + 0x64))[0] - (*(s32 **)(obj + 0x60))[0];
    *(s32 *)(obj + 0xAC) = (*(s32 **)(obj + 0x64))[1] - (*(s32 **)(obj + 0x60))[1];
    *(s32 *)(obj + 0xB0) = (*(s32 **)(obj + 0x64))[2] - (*(s32 **)(obj + 0x60))[2];
    *(s32 *)(obj + 0xB8) = (*(s32 **)(obj + 0x68))[0] - (*(s32 **)(obj + 0x60))[0];
    *(s32 *)(obj + 0xBC) = (*(s32 **)(obj + 0x68))[1] - (*(s32 **)(obj + 0x60))[1];
    *(s32 *)(obj + 0xC0) = (*(s32 **)(obj + 0x68))[2] - (*(s32 **)(obj + 0x60))[2];

    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "lw     $15, 8($12)\n"
        "ctc2   $14, $2\n"
        "ctc2   $15, $4\n"
        :: "r"(obj + 0xA8) : "$12", "$13", "$14", "$15");
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $11, 8($12)\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(obj + 0xB8) : "$12");
    __asm__ volatile(".word 0x4B70000C");
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(obj + 0xC8) : "$12", "memory");

    if ((u32)(*(s32 *)(obj + 0xC8) + 0x3FFF) < 0x7FFF
        && (u32)(*(s32 *)(obj + 0xCC) + 0x3FFF) < 0x7FFF
        && (u32)(*(s32 *)(obj + 0xD0) + 0x3FFF) < 0x7FFF) {
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4AA00428\n");
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            :: "r"(obj + 0x100) : "$12", "memory");
        sum1 = *(s32 *)(obj + 0x100) + *(s32 *)(obj + 0x104) + *(s32 *)(obj + 0x108);
        if ((u32)sum1 < 0x400) {
            dist = (u32)*(((u8 *)&D_8008D118) + sum1) >> 3;
        } else {
            s32 lzcr = 0;
            if (sum1 >= 0) {
                __asm__ volatile(
                    "addu   $t4, %1, $zero\n"
                    "mtc2   $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addiu  $v0, $sp, 0x10\n"
                    "addu   $t4, $v0, $zero\n"
                    "swc2   $31, 0($t4)\n"
                    : "=m"(sp_tmp)
                    : "r"(sum1)
                    : "$2", "$12");
                lzcr = sp_tmp;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(((u8 *)&D_8008D118) + ((u32)sum1 >> shift));
                dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }
        if ((u32)dist < 0x4000) {
            s32 *mat;

            angle = ratan2(*(s32 *)(obj + 0xA8), *(s32 *)(obj + 0xB0));
            sum2 = *(s32 *)(obj + 0xA8) * *(s32 *)(obj + 0xA8)
                + *(s32 *)(obj + 0xB0) * *(s32 *)(obj + 0xB0);
            *(s16 *)(obj + 0xFA) = 0x800 - angle;
            if ((u32)sum2 < 0x400) {
                dist = (u32)*(((u8 *)&D_8008D118) + sum2) >> 3;
            } else {
                s32 lzcr = 0;
                if (sum2 >= 0) {
                    __asm__ volatile(
                        "addu   $t4, %1, $zero\n"
                        "mtc2   $t4, $30\n"
                        "nop\n"
                        "nop\n"
                        "addiu  $v0, $sp, 0x14\n"
                        "addu   $t4, $v0, $zero\n"
                        "swc2   $31, 0($t4)\n"
                        : "=m"(sp_tmp2)
                        : "r"(sum2)
                        : "$2", "$12");
                    lzcr = sp_tmp2;
                }
                {
                    s32 shift = 0x16 - (lzcr & ~1);
                    s32 tbl = *(((u8 *)&D_8008D118) + ((u32)sum2 >> shift));
                    dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
                }
            }
            angle = ratan2(*(s32 *)(obj + 0xAC), dist);
            mat = (s32 *)(obj + 0xD8);
            *(s16 *)(obj + 0xF8) = 0x800 - angle;
            *(s16 *)(obj + 0xD8) = 0x1000;
            *(s16 *)(obj + 0xDA) = 0;
            *(s16 *)(obj + 0xDC) = 0;
            *(s16 *)(obj + 0xDE) = 0;
            *(s16 *)(obj + 0xE0) = 0x1000;
            *(s16 *)(obj + 0xE2) = 0;
            *(s16 *)(obj + 0xE4) = 0;
            *(s16 *)(obj + 0xE6) = 0;
            *(s16 *)(obj + 0xE8) = 0x1000;
            RotMatrixY(*(s16 *)(obj + 0xFA), mat);
            RotMatrixX(*(s16 *)(obj + 0xF8), mat);
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
            vec = (s32 *)(obj + 0xA8);
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
                :: "r"(vec) : "$12", "$13", "$14");
            __asm__ volatile(".word 0x4A486012");
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $25, 0($12)\n"
                "swc2   $26, 4($12)\n"
                "swc2   $27, 8($12)\n"
                :: "r"(vec) : "$12", "memory");
            vec = (s32 *)(obj + 0xB8);
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
                :: "r"(vec) : "$12", "$13", "$14");
            __asm__ volatile(".word 0x4A486012");
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $25, 0($12)\n"
                "swc2   $26, 4($12)\n"
                "swc2   $27, 8($12)\n"
                :: "r"(vec) : "$12", "memory");
            return 0;
        }
    }

    angle = ratan2(*(s32 *)(obj + 0xC8), *(s32 *)(obj + 0xD0));
    *(s32 *)(obj + 0xC8) >>= 6;
    *(s32 *)(obj + 0xCC) >>= 6;
    *(s32 *)(obj + 0xD0) >>= 6;
    sum3 = *(s32 *)(obj + 0xC8) * *(s32 *)(obj + 0xC8)
        + *(s32 *)(obj + 0xD0) * *(s32 *)(obj + 0xD0);
    *(s16 *)(obj + 0xFA) = 0x800 - angle;
    if ((u32)sum3 < 0x400) {
        dist = (u32)*(((u8 *)&D_8008D118) + sum3) >> 3;
    } else {
        s32 lzcr = 0;
        if (sum3 >= 0) {
            __asm__ volatile(
                "addu   $t4, %1, $zero\n"
                "mtc2   $t4, $30\n"
                "nop\n"
                "nop\n"
                "addiu  $v0, $sp, 0x18\n"
                "addu   $t4, $v0, $zero\n"
                "swc2   $31, 0($t4)\n"
                : "=m"(sp_tmp3)
                : "r"(sum3)
                : "$2", "$12");
            lzcr = sp_tmp3;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            s32 tbl = *(((u8 *)&D_8008D118) + ((u32)sum3 >> shift));
            dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }
    angle = ratan2(*(s32 *)(obj + 0xCC), dist);
    mat = (s32 *)(obj + 0xD8);
    *(s16 *)(obj + 0xF8) = 0x800 - angle;
    *(s16 *)(obj + 0xD8) = 0x1000;
    *(s16 *)(obj + 0xDA) = 0;
    *(s16 *)(obj + 0xDC) = 0;
    *(s16 *)(obj + 0xDE) = 0;
    *(s16 *)(obj + 0xE0) = 0x1000;
    *(s16 *)(obj + 0xE2) = 0;
    *(s16 *)(obj + 0xE4) = 0;
    *(s16 *)(obj + 0xE6) = 0;
    *(s16 *)(obj + 0xE8) = 0x1000;
    RotMatrixY(*(s16 *)(obj + 0xFA), mat);
    RotMatrixX(*(s16 *)(obj + 0xF8), mat);
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
    vec = (s32 *)(obj + 0xA8);
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
        :: "r"(vec) : "$12", "$13", "$14");
    __asm__ volatile(".word 0x4A486012");
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(vec) : "$12", "memory");
    vec = (s32 *)(obj + 0xB8);
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
        :: "r"(vec) : "$12", "$13", "$14");
    __asm__ volatile(".word 0x4A486012");
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(vec) : "$12", "memory");
    return 1;
}
