extern Unk80101DF0Record *D_800A3708;
extern s32 D_800A336C;
extern s32 D_800A322C;
extern s32 D_800927C0[64][32];
extern s32 D_80090740[64][32];
extern s32 D_80094840[];
extern s32 D_800A7EF0[];
extern s32 *func_8003EB84(s32, s32, s32 *);
extern s16 *camera_CalcAngles(void);
extern void func_80042A88(Unk80101DF0Rot *, s32 *);
extern s32 ratan2(s32, s32);
extern s32 stage_GetId(void);
extern void func_800620B8(s16 *, s32 *);
/* func_8003E6D8 - grid pass. D_800A3708's xf.rot is turned into a matrix
 * (func_80042A88) and applied to {0,0,0x1000}; ratan2 of the result, stored
 * to D_800A336C, picks ((a >> 6) & 0x3F) one of 64 tables of 32-bit row
 * masks (ANDed with D_80094840 into the scratchpad when neither D_800A322C
 * nor P1 bit 0 is set). The 31x31 grid window centred on the cell of
 * D_800A3708's work.t ((t + 0x7D00) / 2000) is walked, and each cell whose
 * mask bit is set emits its records, as in func_8003EB84.
 *
 * COMPLETED-INLINE-ASM-CANONICAL: pure-C body plus four PsyQ SDK GTE macro
 * islands (the sequence gte_ApplyMatrix expands to -- gtemac.h: SetRotMatrix,
 * ldv0, rtv0, stlvnl -- with its gte_rtv0 step cited below as the identical
 * word gte_mvmva(1,0,0,3,0)), the same spelling and clobber
 * provenance as func_80019310 (src/code6cac.c) and func_800203B4:
 *   gte_SetRotMatrix(r0) -- inline_c.h:297-310 (clobbers as published + $15)
 *   gte_ldv0(r0)         -- inline_c.h:16-20; publishes no clobbers, "$12" and
 *                           "memory" ADDED (it reads vec[] through $12),
 *                           precedent func_80019310 / func_8002D320
 *   gte_mvmva(1,0,0,3,0) -- inline_c.h:816-817 (body gte_mvmva_core,
 *                           :809-814); its two leading nops ride at the
 *                           tail of the gte_ldv0 island, as in func_80019310
 *   gte_stlvnl(r0)       -- inline_c.h:1111-1117; "memory" is its own clobber
 * Ledger: memory/grind/func_8003E6D8/. */
void func_8003E6D8(s32 arg0) {
    s32 mat[8];
    s16 vec[4];
    s32 res[3];
    s16 pos[4];
    s32 cx;
    s32 cz;
    s16 x;
    s16 z;
    s32 *mask;
    s32 *out;
    s32 i;
    s32 j;
    s32 bits;
    s16 row;
    s16 col;
    s16 vidx;
    u16 t0;
    s32 v1;
    s32 a3;
    u8 *e;
    u8 *e2;
    s32 *list;
    s32 ang;
    s32 idx;

    cx = (D_800A3708->work.t[0] + 0x7D00) / 2000;
    cz = (D_800A3708->work.t[2] + 0x7D00) / 2000;
    camera_CalcAngles();
    func_80042A88(&D_800A3708->xf.rot, mat);
    vec[2] = 0x1000;
    vec[0] = 0;
    vec[1] = 0;
    /* PsyQ libgte inline macro gte_SetRotMatrix(r0) --- inline_c.h:297-310. */
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
    /* PsyQ libgte inline macro gte_ldv0(r0) --- inline_c.h:16-20 (the lwc2
     * pair). The two nops that follow are NOT gte_ldv0 text: they are the
     * leading `nop; nop` of gte_mvmva_core (inline_c.h:809-814), carried at
     * the tail of this island exactly as func_80019310 does. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $0, 0($12)\n"
        "lwc2   $1, 4($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(vec) : "$12", "memory");
    /* Sony libgte macro gte_mvmva(sf,mx,v,cv,lm) --- inline_c.h:816-817, whose body
     * is gte_mvmva_core(r0) at inline_c.h:809-814 (`nop; nop; .word <literal>`).
     * This instance is gte_mvmva(1,0,0,3,0): sf=1, mx=rotation, v=V0, cv=none,
     * lm=0. It is spelled as the bare `.word 0x4A486012`, the final cop2 word:
     * the macro's own literal is a DMPSX placeholder that Sony's dmpsx post-pass
     * rewrites, and this build has no such pass. The core's two nops are
     * carried at the tail of the gte_ldv0 island above (func_80019310's form). */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ libgte inline macro gte_stlvnl(r0) --- inline_c.h:1111-1117. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(res) : "$12", "memory");
    ang = ratan2(res[0], res[2]);
    D_800A336C = ang;
    x = cx - 0xF;
    z = cz - 0xF;
    idx = (ang >> 6) & 0x3F;
    if (D_800A322C != 0) {
        mask = D_800927C0[idx];
    } else if (D_800F6656 & 1) {
        mask = D_80090740[idx];
    } else {
        s32 *src2;
        s32 *dst;
        src2 = D_80094840;
        mask = D_80090740[idx];
        dst = (s32 *)0x1F800004;
        for (i = 0; i < 0x1F; i++) {
            *dst++ = *src2++ & *mask++;
        }
        mask = (s32 *)0x1F800004;
    }

    out = D_800A7EF0;
    for (i = 0; i < 0x1F; i++) {
        row = z + i;
        if (row < 0) {
            mask++;
            continue;
        }
        if (row >= 0x20) {
            break;
        }
        bits = *mask++;
        if (bits == 0) {
            continue;
        }
        bits <<= 1;
        for (j = 0; j < 0x1F; j++) {
            col = x + j;
            if (col < 0) {
                /* FAKE: the column shift is written in both arms instead of
                 * once in the for-increment. jump2 cross-jumps the two copies
                 * back into the one shift in the loop branch's delay slot
                 * (bytes unchanged); the second copy lifts bits' reg_n_refs
                 * 17->23 so global-alloc ranks it above a3 (pri 11500 vs
                 * 8823), giving the target's bits->$t1 / a3->$t2. */
                bits <<= 1;
                continue;
            }
            if (col >= 0x20) {
                break;
            }
            if (bits < 0) {
                vidx = D_800A7FE0[row][col];
                if (vidx >= 0) {
                    a3 = D_800A8FB0[row * 0x20 + col];
                    do {
                        t0 = D_800A87E0[vidx++];
                        v1 = t0 & 0x7FFF;
                        if (v1 < D_800A3368) {
                            e = &D_800A4750[v1 * 0x10];
                            e[6] = a3 & 3;
                            if ((a3 & 8) || ((a3 & 4) && (e[7] & 8))) {
                                e[7] |= 1;
                            } else {
                                e[7] &= 0xFE;
                            }
                            list = (s32 *)D_800A3820;
                            D_800A3820 = (s32)(list + 1);
                            *list = (s32)e;
                        } else {
                            e2 = &D_800A6690[(v1 - D_800A3368) * 0x68];
                            if (e2[0x58] == 0) {
                                *out++ = (s32)e2;
                                e2[0x58] = 1;
                            }
                        }
                    } while (!(t0 & 0x8000));
                }
            }
            bits <<= 1;
        }
    }

    if ((D_800F6656 & 1) && D_800A322C == 0) {
        out = func_8003EB84(x, z, out);
    }
    while (out != D_800A7EF0) {
        out--;
        *(s32 *)D_800A3820 = *out;
        (*(u8 **)D_800A3820)[0x58] = 0;
        D_800A3820 += 4;
    }
    if (g_stage_init_tbl[stage_GetId()].unk4 != 0) {
        g_stage_init_tbl[stage_GetId()].unk4();
    }
    {
        Unk80101DF0Xform *cam = &D_80101DF0.xf;
        pos[0] = -cam->rot.vx;
        pos[1] = -cam->rot.vy;
        pos[2] = -cam->rot.vz;
        func_800620B8(pos, cam->mat.t);
    }
}
