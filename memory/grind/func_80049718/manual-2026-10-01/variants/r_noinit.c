extern u8 *D_800A38B4;
extern s32 g_anim_func_table[];
extern MATRIX *MulMatrix0(MATRIX *, MATRIX *, MATRIX *);

/* Appends one or two 0x68-byte draw objects at D_800A38B4 for animation entry
 * arg0 and links each into the ordering table at D_800A3820.  The first object
 * (type 0) gets its rotation and position either from rot/pos (flags == 1:
 * g_anim_func_table[0] turns rot into the object's matrix at +0x18) or from
 * part (flags & 1) of vehicle flags >> 1: the part's offset (+0x4C) is scaled
 * by the vehicle's +0x12, the vehicle matrix (+0x44) times the part's matrix
 * (+0x38) becomes the object's matrix, the scaled offset run through the
 * parent's matrix (part +0xC, matrix +0x18, translation +0x2C) gives its
 * position, and the matrix is copied back into the part.  The second object
 * (type 3, parent = the first) follows unless flags is still 1.  Objects and
 * parts are walked by byte offset, as func_80049A2C below does. */
void func_80049718(s32 arg0, s32 flags, s32 *pos, s16 *rot_in) {
    SVECTOR rot;
    s32 val58;
    u8 *vehicle;
    u8 *obj;
    u8 *part;
    /* FAKE: named intermediate (no-new-park-categories entry 6).  Set before
     * the call, sched1 moves the andi past func_8004153C but ahead of the copy
     * of its result (`andi v1,s3,1; move s1,v0`, as in the target); written
     * inside the part expression or after the call it follows the copy and
     * takes $v0 (5).  Ledger: memory/grind/func_80049718/manual-2026-10-01/scores.txt */
    s32 side;
    if (D_800EF980[arg0] < 0) {
        func_80052C10();
    }
    obj = D_800A38B4;
    obj[0] = 0;
    obj[1] = 0;
    *(s16 *)(obj + 2) = D_800EF980[arg0] * 2;
    *(s16 *)(obj + 4) = 6;
    *(s16 *)(obj + 8) = 0;
    *(s32 *)(obj + 0xC) = 0;
    *(s16 *)(obj + 0xA) = 4;
    if (flags != 0) {
        if (flags == 1) {
            *(u16 *)(obj + 0x10) = rot_in[0];
            *(u16 *)(obj + 0x12) = rot_in[1];
            *(u16 *)(obj + 0x14) = rot_in[2];
            ((void (*)(SVECTOR *, MATRIX *))g_anim_func_table[0])((SVECTOR *)(obj + 0x10), (MATRIX *)(obj + 0x18));
            *(s32 *)(obj + 0x2C) = pos[0];
            *(s32 *)(obj + 0x30) = pos[1];
            *(s32 *)(obj + 0x34) = pos[2];
        } else {
            /* flags is rewritten in place (SOTN: src/main/psxsdk/libgte/geo_01.c:10 @aa53500,
             * rcos's `a &= 0xFFF;` on its parameter). */
            flags &= 0x7FFF;
            side = flags & 1;
            vehicle = (u8 *)func_8004153C(flags >> 1);
            part = vehicle + (side * 0x68 + 0x7E4);
            *(s32 *)(part + 0x4C) = (*(s32 *)(part + 0x4C) * *(s16 *)(vehicle + 0x12)) >> 12;
            *(s32 *)(part + 0x50) = (*(s32 *)(part + 0x50) * *(s16 *)(vehicle + 0x12)) >> 12;
            *(s32 *)(part + 0x54) = (*(s32 *)(part + 0x54) * *(s16 *)(vehicle + 0x12)) >> 12;
            MulMatrix0((MATRIX *)(vehicle + 0x44), (MATRIX *)(part + 0x38), (MATRIX *)(obj + 0x18));
            rot.vx = *(s32 *)(part + 0x4C);
            rot.vy = *(s32 *)(part + 0x50);
            rot.vz = *(s32 *)(part + 0x54);
            ApplyMatrix((MATRIX *)(*(u8 **)(part + 0xC) + 0x18), &rot, (VECTOR *)(obj + 0x2C));
            *(s32 *)(obj + 0x2C) = *(s32 *)(obj + 0x2C) + *(s32 *)(*(u8 **)(part + 0xC) + 0x2C);
            *(s32 *)(obj + 0x30) = *(s32 *)(obj + 0x30) + *(s32 *)(*(u8 **)(part + 0xC) + 0x30);
            *(s32 *)(obj + 0x34) = *(s32 *)(obj + 0x34) + *(s32 *)(*(u8 **)(part + 0xC) + 0x34);
            flags |= 0x8000;
            *(MATRIX *)(part + 0x18) = *(MATRIX *)(obj + 0x18);
            val58 = *(s16 *)(vehicle + 0x1A84);
        }
        {
            u8 *ot = (u8 *)D_800A3820;
            D_800A3820 = (s32)(ot + 4);
            *(u8 **)ot = obj;
        }
        obj += 0x68;
        if (flags != 1) {
            /* FAKE: named intermediate (no-new-park-categories entry 6).  The
             * table is read before the object's fields are written, as in the
             * target (lh first); storing D_800EF980[arg0] * 2 + 1 directly at
             * the +2 store reads it last (17), and moving that store first
             * reorders the stores (10).  Ledger:
             * memory/grind/func_80049718/manual-2026-10-01/scores.txt */
            s32 frame = D_800EF980[arg0];
            u8 *ot;
            obj[0] = 3;
            obj[1] = 0;
            *(s32 *)(obj + 0x58) = val58;
            ot = (u8 *)D_800A3820;
            *(s32 *)(obj + 0xC) = (s32)(obj - 0x68);
            *(s16 *)(obj + 6) = 1;
            *(s16 *)(obj + 8) = 0;
            *(s16 *)(obj + 0xA) = 0;
            *(s16 *)(obj + 4) = 6;
            *(s16 *)(obj + 2) = frame * 2 + 1;
            D_800A3820 = (s32)(ot + 4);
            *(u8 **)ot = obj;
            obj += 0x68;
        }
        D_800A38B4 = obj;
    }
}
