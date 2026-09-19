s32 func_8002DAD0(u8 *obj) {
    s32 *mat;
    s32 sp_tmp;
    s32 dist_sq;
    s32 angle1;
    s32 angle2;
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
        "addu   $12, %0, $zero\n"
        "lwc2   $11, 8($12)\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "nop\n"
        "nop\n"
        ".word 0x4B70000C\n"
        :: "r"(obj + 0xB8) : "$12");

    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(obj + 0xC8) : "$12", "memory");

    if ((u32)(*(s32 *)(obj + 0xC8) + 0x3FFF) < 0x7FFF &&
        (u32)(*(s32 *)(obj + 0xCC) + 0x3FFF) < 0x7FFF &&
        (u32)(*(s32 *)(obj + 0xD0) + 0x3FFF) < 0x7FFF) {
        return 0;
    }

    angle1 = ratan2(*(s32 *)(obj + 0xC8), *(s32 *)(obj + 0xD0));
    *(s32 *)(obj + 0xC8) = *(s32 *)(obj + 0xC8) >> 6;
    *(s32 *)(obj + 0xCC) = *(s32 *)(obj + 0xCC) >> 6;
    {
        s32 dz = *(s32 *)(obj + 0xD0) >> 6;
        dist_sq = *(s32 *)(obj + 0xC8) * *(s32 *)(obj + 0xC8) + dz * dz;
        *(s32 *)(obj + 0xD0) = dz;
    }
    *(s16 *)(obj + 0xFA) = 0x800 - angle1;

    if ((u32)dist_sq < 0x400) {
        dist = (u32)*(((u8 *)&D_8008D118) + dist_sq) >> 3;
    } else {
        s32 lzcr = 0;
        if (dist_sq >= 0) {
            __asm__ volatile(
                "addu   $t4, %1, $zero\n"
                "mtc2   $t4, $30\n"
                "nop\n"
                "nop\n"
                "addiu  $v0, $sp, 0x10\n"
                "addu   $t4, $v0, $zero\n"
                "swc2   $31, 0($t4)\n"
                : "=m"(sp_tmp)
                : "r"(dist_sq)
                : "$2", "$12");
            lzcr = sp_tmp;
        }
        {
            dist = (u32)((*(((u8 *)&D_8008D118) + ((u32)dist_sq >> (0x16 - (lzcr & ~1))))) << 16) >> (0x13 - ((u32)(0x16 - (lzcr & ~1)) >> 1));
        }
    }

    angle2 = ratan2(*(s32 *)(obj + 0xCC), dist);
    mat = (s32 *)(obj + 0xD8);
    *(s16 *)(obj + 0xF8) = 0x800 - angle2;

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
