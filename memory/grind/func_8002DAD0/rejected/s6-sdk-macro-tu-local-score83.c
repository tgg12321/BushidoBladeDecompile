/* PROBE (session 6): func_8002DAD0 written as authentic Sony DMPSX GTE macro
 * INVOCATIONS. Every macro body below is transcribed VERBATIM from Sony's
 * PsyQ `inline_o.h` ("Macro definitions of DMPSX version 3",
 * $PSLibId: Run-time Library Release 4.5$, Copyright(C) 1996 Sony Computer
 * Entertainment Inc.) — the DMPSX macro set whose expansions are, register
 * for register, the islands this function's target bytes contain. Line
 * references are to that header. */

#define bb2_gte_ldopv1(r1) { __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("lw    $15,8($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("ctc2  $14,$2": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory"); }
#define bb2_gte_ldopv2(r1) { __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); __asm__ volatile ("lwc2  $11,8($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("lwc2  $9,($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("lwc2  $10,4($12)": : :"$12","$13","$14","$15","memory"); }
#define bb2_gte_nop() { __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); }
#define bb2_gte_op0() { __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); __asm__ volatile (".word 0x4B70000C": : :"$12","$13","$14","$15","memory"); }
#define bb2_gte_stlvnl(r1) { __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory"); }
#define bb2_gte_ldlzc(r1) { __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory"); }
#define bb2_gte_stlzc(r1) { __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory"); }
#define bb2_gte_Lzc(r1,r2) { bb2_gte_ldlzc(r1); bb2_gte_nop(); bb2_gte_nop(); bb2_gte_stlzc(r2); }
#define bb2_gte_SetRotMatrix(r1) { __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory"); }
#define bb2_gte_ldlv0(r1) { __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); __asm__ volatile ("lhu   $14,4($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("lhu   $13,($12)": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("sll   $14,$14,16": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("or    $13,$13,$14": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("mtc2  $13,$0": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("lwc2  $1,8($12)": : :"$12","$13","$14","$15","memory"); }
#define bb2_gte_rtv0() { __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory"); }

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

    bb2_gte_ldopv1(obj + 0xA8);
    bb2_gte_ldopv2(obj + 0xB8);
    bb2_gte_op0();
    bb2_gte_stlvnl(obj + 0xC8);

    if ((u32)(*(s32 *)(obj + 0xC8) + 0x3FFF) < 0x7FFF &&
        (u32)(*(s32 *)(obj + 0xCC) + 0x3FFF) < 0x7FFF &&
        (u32)(*(s32 *)(obj + 0xD0) + 0x3FFF) < 0x7FFF) {
        return 0;
    }

    angle1 = ratan2(*(s32 *)(obj + 0xC8), *(s32 *)(obj + 0xD0));
    *(s32 *)(obj + 0xC8) = *(s32 *)(obj + 0xC8) >> 6;
    *(s32 *)(obj + 0xCC) = *(s32 *)(obj + 0xCC) >> 6;
    dist = *(s32 *)(obj + 0xD0);
    dist >>= 6;
    dist_sq = *(s32 *)(obj + 0xC8) * *(s32 *)(obj + 0xC8) + dist * dist;
    *(s32 *)(obj + 0xD0) = dist;
    *(s16 *)(obj + 0xFA) = 0x800 - angle1;

    if ((u32)dist_sq < 0x400) {
        dist = (u32)*(((u8 *)&D_8008D118) + dist_sq) >> 3;
    } else {
        s32 lzcr = 0;
        if (dist_sq >= 0) {
            bb2_gte_Lzc(dist_sq, &sp_tmp);
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            s32 tbl = *(((u8 *)&D_8008D118) + ((u32)dist_sq >> shift));
            dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
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

    bb2_gte_SetRotMatrix(mat);

    bb2_gte_ldlv0(obj + 0xA8);
    bb2_gte_rtv0();
    bb2_gte_stlvnl(obj + 0xA8);

    bb2_gte_ldlv0(obj + 0xB8);
    bb2_gte_rtv0();
    bb2_gte_stlvnl(obj + 0xB8);

    return 1;
}
