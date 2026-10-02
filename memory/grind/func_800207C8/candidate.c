/* func_800207C8 — places character rec->unk_04's points in world space with its bone
 * matrices (game_GetPlayerData()): each of the 22 hit records' offsets through its bone
 * into bone_out (SPAD->unkA8[ch]); the attachment point set D_8008D86C[unk_0E] (D_8008D774
 * when unk_12 == 50) through bone 18 into att_out; when unk_8C != 0, the pair
 * D_8008D88C[unk_14] through bone 19 into extra_out.  A point is the GTE MVMVA rotation
 * (sf=1) of the SVECTOR plus the matrix translation.  Then: bone 11's y axis into
 * rec->unk_1EC, bone_out[0] into rec->unk_180, the translations of bones 17 and 14 (y
 * raised by (unk_1A * 71) >> 11) into rec->unk_198[], the floor height under each
 * (func_80053614 down a probe from y - 100 to y + 2000; the probe's lower end when it
 * finds nothing) into rec->unk_1B0[], and the two bones' headings into unk_1BA / unk_1C2.
 * GTE islands: PsyQ Run-time Library Release 4.3 inline_o.h statements, character for
 * character (engine/gtemacro.py PINNED); each gte_rtv0 carries the post-DMPSX word
 * .word 0x4A486012 in place of the header's DMPSX placeholder .word 0x0000013f (MVMVA sf=1
 * mx=rot v=V0 cv=none lm=0; per-function grant GRANT_REF). */
void func_800207C8(PracticeMenuRec *rec, LeafPos *bone_out, LeafPos *att_out, LeafPos *extra_out) {
    /* the func_80053614 probe in scratchpad: from (words 0..2), to (4..6), hit (8..10),
     * normal (12..13), work area (14..) */
    s32 *probe = (s32 *)0x1F8002B8;
    MATRIX **bones;
    MATRIX *m;
    s32 *pos;
    SVec4i16 *v;
    BoneHitRec *hr;
    LeafPos *o;
    s32 i;

    bones = (MATRIX **)game_GetPlayerData(rec->unk_04);
    hr = (BoneHitRec *)&D_800F5F68[rec->unk_04 * 0x1B8];
    o = bone_out;
    for (i = 0; i < 22; i++, hr++, o++) {
        m = bones[hr->bone];
        /* inline_o.h: gte_SetRotMatrix :272-284 */
        __asm__ volatile ("move  $12,%0": :"r"(m):"$12","$13","$14","$15","memory");
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
        /* inline_o.h: gte_ldv0 :16-20 */
        __asm__ volatile ("move  $12,%0": :"r"(&hr->ofs):"$12","$13","$14","$15","memory");
        __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
        /* inline_o.h: gte_rtv0 :426-430, post-DMPSX command word (see above) */
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
        /* inline_o.h: gte_stlvnl :904-909 */
        __asm__ volatile ("move  $12,%0": :"r"(o):"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
        o->x += m->t[0];
        o->y += m->t[1];
        o->z += m->t[2];
    }

    m = bones[18];
    /* inline_o.h: gte_SetRotMatrix :272-284 */
    __asm__ volatile ("move  $12,%0": :"r"(m):"$12","$13","$14","$15","memory");
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
    if (rec->unk_12 == 50) {
        v = D_8008D774;
    } else {
        v = D_8008D86C[rec->unk_0E];
    }
    o = att_out;
    for (i = 0; i < D_8008D864[rec->unk_0E]; i++, v++, o++) {
        /* inline_o.h: gte_ldv0 :16-20 */
        __asm__ volatile ("move  $12,%0": :"r"(v):"$12","$13","$14","$15","memory");
        __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
        /* inline_o.h: gte_rtv0 :426-430, post-DMPSX command word (see above) */
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
        /* inline_o.h: gte_stlvnl :904-909 */
        __asm__ volatile ("move  $12,%0": :"r"(o):"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
        o->x += m->t[0];
        o->y += m->t[1];
        o->z += m->t[2];
    }

    if (rec->unk_8C != 0) {
        m = bones[19];
        /* inline_o.h: gte_SetRotMatrix :272-284 */
        __asm__ volatile ("move  $12,%0": :"r"(m):"$12","$13","$14","$15","memory");
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
        v = D_8008D88C[rec->unk_14];
        o = extra_out;
        for (i = 0; i < 2; i++, v++, o++) {
            /* inline_o.h: gte_ldv0 :16-20 */
            __asm__ volatile ("move  $12,%0": :"r"(v):"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
            /* inline_o.h: gte_rtv0 :426-430, post-DMPSX command word (see above) */
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
            /* inline_o.h: gte_stlvnl :904-909 */
            __asm__ volatile ("move  $12,%0": :"r"(o):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
            o->x += m->t[0];
            o->y += m->t[1];
            o->z += m->t[2];
        }
    }

    m = bones[11];
    /* inline_o.h: gte_SetRotMatrix :272-284 */
    __asm__ volatile ("move  $12,%0": :"r"(m):"$12","$13","$14","$15","memory");
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
    /* inline_o.h: gte_ldv0 :16-20 */
    __asm__ volatile ("move  $12,%0": :"r"(&D_800A3138):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
    /* inline_o.h: gte_rtv0 :426-430, post-DMPSX command word (see above) */
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    rec->unk_180 = bone_out[0];
    /* inline_o.h: gte_stlvnl :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(&rec->unk_1EC):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");

    pos = bones[17]->t;
    rec->unk_198[0].x = pos[0];
    rec->unk_198[0].y = pos[1] + ((rec->unk_1A * 71) >> 11);
    rec->unk_198[0].z = pos[2];
    pos = bones[14]->t;
    rec->unk_198[1].x = pos[0];
    rec->unk_198[1].y = pos[1] + ((rec->unk_1A * 71) >> 11);
    rec->unk_198[1].z = pos[2];
    for (i = 0; i < 2; i++) {
        probe[0] = rec->unk_198[i].x;
        probe[1] = rec->unk_198[i].y - 100;
        probe[2] = rec->unk_198[i].z;
        probe[4] = rec->unk_198[i].x;
        probe[5] = rec->unk_198[i].y + 2000;
        probe[6] = rec->unk_198[i].z;
        if (func_80053614(&probe[0], &probe[4], &probe[8], &probe[12], (s32)&probe[14])) {
            rec->unk_1B0[i] = probe[9];
        } else {
            rec->unk_1B0[i] = probe[5];
        }
    }
    m = bones[17];
    rec->unk_1BA = ratan2(m->m[0][2], m->m[2][2]) + 0x800;
    m = bones[14];
    rec->unk_1C2 = ratan2(m->m[0][2], m->m[2][2]) + 0x800;
}
