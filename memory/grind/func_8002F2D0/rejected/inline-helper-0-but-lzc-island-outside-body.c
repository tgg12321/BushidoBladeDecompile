static inline s32 isqrt_lut(s32 x) {
    s32 sp_tmp;
    if ((u32)x < 0x400) {
        return (u32)*(((u8 *)&D_8008D118) + x) >> 3;
    } else {
        s32 lzcr = 0;
        s32 shift;
        if (x >= 0) {
            __asm__ volatile(
                "addu   $t4, %1, $zero
"
                "mtc2   $t4, $30
"
                "nop
"
                "nop
"
                "addiu  $v0, $sp, 0x10
"
                "addu   $t4, $v0, $zero
"
                "swc2   $31, 0($t4)
"
                : "=m"(sp_tmp)
                : "r"(x)
                : "$2", "$12");
            lzcr = sp_tmp;
        }
        shift = 0x16 - (lzcr & ~1);
        x = *(((u8 *)&D_8008D118) + ((u32)x >> shift));
        return (u32)(x << 16) >> (0x13 - ((u32)shift >> 1));
    }
}
void func_8002F2D0(s32 *a0, s32 *a1) {
    MATRIX *m;
    u8 *scr;
    s32 *mat;
    s32 *new_var;
    s32 *vec;
    s32 c0, c1, c2;
    s32 det;
    s32 d0;
    s32 i0, i1, i2;
    s32 r0, r1, r2;
    s32 ang_z, ang_y;
    s32 dist;

    m = (MATRIX *)0x1F800390;
    *m = *(MATRIX *)a0;

    c0 = m->m[1][2] * m->m[2][1] - m->m[1][1] * m->m[2][2];
    d0 = m->m[0][0] * (c0 >> 12);
    c1 = m->m[0][1] * m->m[2][2] - m->m[0][2] * m->m[2][1];
    c2 = m->m[0][2] * m->m[1][1] - m->m[0][1] * m->m[1][2];
    det = (d0 + m->m[1][0] * (c1 >> 12) + m->m[2][0] * (c2 >> 12)) >> 12;
    c0 = c0 / det;
    c1 = c1 / det;
    do { i2 = c2 / det; } while (0);
    r0 = (m->m[1][0] * m->m[2][2] - m->m[1][2] * m->m[2][0]) / det;
    r1 = (m->m[0][2] * m->m[2][0] - m->m[0][0] * m->m[2][2]) / det;
    r2 = (m->m[0][0] * m->m[1][2] - m->m[0][2] * m->m[1][0]) / det;

    ang_z = -ratan2(c1, c0);
    scr = (u8 *)0x1F8002B8;
    det = isqrt_lut(c0 * c0 + c1 * c1);

    ang_y = ratan2(i2, det);
    new_var = (s32 *)(scr + 0xD8);
    mat = new_var;
    *(s16 *)(scr + 0xD8) = 0x1000;
    *(s16 *)(scr + 0xDA) = 0;
    *(s16 *)(scr + 0xDC) = 0;
    *(s16 *)(scr + 0xDE) = 0;
    *(s16 *)(scr + 0xE0) = 0x1000;
    *(s16 *)(scr + 0xE2) = 0;
    *(s16 *)(scr + 0xE4) = 0;
    *(s16 *)(scr + 0xE6) = 0;
    *(s16 *)(scr + 0xE8) = 0x1000;
    RotMatrixZ(ang_z, mat);
    RotMatrixY(ang_y, mat);

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
    vec = (s32 *)(scr + 0xA8);
    vec[0] = r0;
    vec[1] = r1;
    vec[2] = r2;
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

    ((s16 *)a1)[0] = ratan2(vec[2], vec[1]);
    ((s16 *)a1)[1] = -ang_y;
    ((s16 *)a1)[2] = -ang_z;
}
