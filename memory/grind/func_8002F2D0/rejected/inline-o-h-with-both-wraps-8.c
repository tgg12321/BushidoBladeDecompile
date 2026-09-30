void func_8002F2D0(s32 *a0, s32 *a1) {
    MATRIX *m;
    u8 *scr;
    s32 *mat;
    s32 *vec;
    s32 c0, c1, c2;
    s32 det;
    s32 d0;
    s32 i2;
    s32 r0, r1, r2;
    s32 ang_z, ang_y;
    s32 sum;
    s32 sp_tmp;

    m = (MATRIX *)0x1F800390;
    *m = *(MATRIX *)a0;

    c0 = m->m[1][2] * m->m[2][1] - m->m[1][1] * m->m[2][2];
    d0 = m->m[0][0] * (c0 >> 12);
    c1 = m->m[0][1] * m->m[2][2] - m->m[0][2] * m->m[2][1];
    c2 = m->m[0][2] * m->m[1][1] - m->m[0][1] * m->m[1][2];
    det = (d0 + m->m[1][0] * (c1 >> 12) + m->m[2][0] * (c2 >> 12)) >> 12;
    c0 = c0 / det;
    c1 = c1 / det;
    do {
        i2 = c2 / det;
    } while (0);
    r0 = (m->m[1][0] * m->m[2][2] - m->m[1][2] * m->m[2][0]) / det;
    r1 = (m->m[0][2] * m->m[2][0] - m->m[0][0] * m->m[2][2]) / det;
    r2 = (m->m[0][0] * m->m[1][2] - m->m[0][2] * m->m[1][0]) / det;

    ang_z = -ratan2(c1, c0);
    scr = (u8 *)0x1F8002B8;
    sum = c0 * c0 + c1 * c1;
    if ((u32)sum < 0x400) {
        det = (u32)*(((u8 *)&g_sqrt_table_u8) + sum) >> 3;
    } else {
        s32 lzcr = 0;
        if (sum >= 0) {
            /* inline_o.h: gte_ldlzc :207-210, gte_nop :1095-1097, gte_nop :1095-1097, gte_stlzc :1074-1077 */
            __asm__ volatile ("move  $12,%0": :"r"(sum):"$12","$13","$14","$15","memory");
            __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("move  $12,%0": :"r"(&sp_tmp):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            sum = *(((u8 *)&g_sqrt_table_u8) + ((u32)sum >> shift));
            det = (u32)(sum << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }

    ang_y = ratan2(i2, det);
    mat = (s32 *)(scr + 0xD8);
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

    do {
        /* inline_o.h: gte_SetRotMatrix :272-284 */
        __asm__ volatile ("move  $12,%0": :"r"(mat):"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory");
    } while (0);
    vec = (s32 *)(scr + 0xA8);
    vec[0] = r0;
    vec[1] = r1;
    vec[2] = r2;
    /* inline_o.h: gte_ldlv0 :95-103, gte_rtv0 :426-430 */
    __asm__ volatile ("move  $12,%0": :"r"(vec):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("sll   $14,$14,16": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("or    $13,$13,$14": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("mtc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    /* inline_o.h: gte_stlvnl :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(vec):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");

    ((s16 *)a1)[0] = ratan2(vec[2], vec[1]);
    ((s16 *)a1)[1] = -ang_y;
    ((s16 *)a1)[2] = -ang_z;
}
