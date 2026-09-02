/* s10 REJECTED (scores 7, no change). The rederive-modality whole-function reshape: the
 * parameter is re-typed through a declared struct (Obj_800300B4) instead of u8* + casts, the
 * vector at +0x2C is a union member (`o->lv.h[0] | ((u32)o->lv.h[2] << 16)` - no pointer casts
 * anywhere in the pack), the matrix translation is read through a materialised `s32 *tr = mat + 5;`
 * and the D_8008EB80 lookup subscripts a typed pointer. Only the three cop2 islands and the
 * proven-necessary do-while(0) wrap carry over from the banked chassis.
 * Result: 7, and the island-2 window is BYTE-IDENTICAL to best_ban_compliant.c's
 *   lhu v0,48(s3) ; lhu v1,44(s3) ; sll v0,v0,0x10 ; or v1,v1,v0 ; addiu v0,s3,44 ;
 *   move t4,v0 ; mtc2 v1,$0 ; lwc2 $1,8(t4)
 * Dump: tmp/grind/func_800300B4/s10/dis_v_struct.txt. See hypotheses.md s10 H40. */
/* s10 REDERIVE probe v_struct: structurally different whole-function shape.
 * Not a tweak of the banked chassis: the param is re-typed through a declared
 * struct, the vector at +0x2C is a union member (no pointer casts anywhere in
 * the pack), the matrix translation is read through a materialised
 * `s32 *tr = mat + 5;` and the D_8008EB80 lookup is an array subscript on a
 * typed pointer. Only the three cop2 islands and the proven-necessary
 * do-while(0) wrap are carried over verbatim. */
typedef struct Obj_800300B4 {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 kind;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ u8  disabled;
    /* 0x07 */ u8  unk7;
    /* 0x08 */ u8  unk8;
    /* 0x09 */ u8  slot;
    /* 0x0A */ u8  mode;
    /* 0x0B */ u8  unkB;
    /* 0x0C */ s32 rot[8];
    /* 0x2C */ union { s32 w[3]; u16 h[6]; } lv;
} Obj_800300B4;

void func_800300B4(u8 *arg0) {
    Obj_800300B4 *o = (Obj_800300B4 *)arg0;
    s32 mac[3];
    s32 dir[2];
    s32 mtx[8];
    s32 *playerData;
    s32 *mat;
    s32 *tr;
    s32 lookup;
    u32 packed;

    playerData = (s32 *)game_GetPlayerData(o->disabled == 0);
    mat = (s32 *)playerData[o->slot];

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

    packed = o->lv.h[0] | ((u32)o->lv.h[2] << 16);
    __asm__ volatile(
        "move   $12, %0\n"
        "mtc2   %1, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        ".word  0x4A486012\n"
        :: "r"(o->lv.w), "r"(packed) : "$12");

    do { /* FAKE: single do-while(0) spanning the gte_stlvnl island through the
          * end of the function, mechanism: flow.c loop_depth ref weighting feeding
          * local-alloc.c:1670 qty_compare_1, lever-exhaustion:
          * memory/grind/func_800300B4/hypotheses.md H2/H14/H16/H19/H21/H22/H24/H25
          * plus the s5 H27 class kill */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            :: "r"(mac) : "$12", "memory");

        tr = mat + 5;
        mac[0] += tr[0];
        mac[1] += tr[1];
        mac[2] += tr[2];

        MulMatrix0(mat, o->rot, mtx);
        func_8002F2D0(mtx, dir);
        lookup = (&D_8008EB80)[o->kind];
        func_80049718(lookup, 1, mac, dir);
        func_800393C8(o->mode, lookup, mac, dir);
    } while (0);
}
