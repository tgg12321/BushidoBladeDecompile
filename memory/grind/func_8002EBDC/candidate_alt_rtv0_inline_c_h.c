/* ALTERNATIVE (laneB 2026-09-30): gte_rtv0 spelled from inline_c.h, every other island inline_o.h.
 * Mixes two Sony headers in one function (they define the same macro names, so one TU could not
 * include both); banked for the reviewer/owner, NOT the preferred form. Scores 0 (score_nostrip). */
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
        dist = (u32)*(((u8 *)&g_sqrt_table_u8) + dist_sq) >> 3;
    } else {
        s32 lzcr = 0;
        if (dist_sq >= 0) {
            /* inline_o.h: gte_ldlzc :207-210, gte_nop :1095-1097, gte_nop :1095-1097, gte_stlzc :1074-1077 */
            __asm__ volatile ("move  $12,%0": :"r"(dist_sq):"$12","$13","$14","$15","memory");
            __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("move  $12,%0": :"r"(&sp_tmp):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            dist = (u32)(*(((u8 *)&g_sqrt_table_u8) + ((u32)dist_sq >> shift)) << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }

    angle = ratan2(dir[1], dist);
    mat = (s32 *)(scr + 0xD8);
    *(s16 *)(scr + 0xF8) = 0x800 - angle;

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
    /* inline_o.h: gte_ldlv0 :95-103; gte_rtv0 from inline_c.h 4.3 :499-502 (one statement, no clobber list; post-DMPSX word 0x4A486012 for 0x0000013f) */
    __asm__ volatile ("move  $12,%0": :"r"(vec_in):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("sll   $14,$14,16": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("or    $13,$13,$14": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("mtc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop;" "nop;" ".word 0x4A486012");
    vec = (s32 *)(scr + 0xA8);
    /* inline_o.h: gte_stlvnl :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(vec):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");

    vec[2] = (vec[2] * scale_z) / 256;
    vec[0] = (vec[0] * scale_xy) / 256;
    vec[1] = (vec[1] * scale_xy) / 256;

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
    /* inline_o.h: gte_ldlv0 :95-103; gte_rtv0 from inline_c.h 4.3 :499-502 (one statement, no clobber list; post-DMPSX word 0x4A486012 for 0x0000013f) */
    __asm__ volatile ("move  $12,%0": :"r"(vec):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lhu   $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("sll   $14,$14,16": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("or    $13,$13,$14": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("mtc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop;" "nop;" ".word 0x4A486012");
    /* inline_o.h: gte_stlvnl :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(out):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
}
