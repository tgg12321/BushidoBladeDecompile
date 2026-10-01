void func_80031890(u8 *obj, Obj80106A78 *ent, s32 idx) {
    s32 *mat;
    Vec3i32 *vec;
    s32 angle1;
    s32 angle2;
    s32 sum_sq;
    s32 adj;

    if (ent->unk_02 != 0xE) {
        s32 vx = ent->unk_44.x;
        s32 vz = ent->unk_44.z;
        s32 av = ent->unk_5C[1];
        sum_sq = vx * vx + vz * vz;
        if (rng_Next() & 1) {
            adj = sum_sq / 64;
        } else {
            adj = -sum_sq / 64;
        }
        ent->unk_5C[1] = av + adj;
    }

    mat = (s32 *)(obj + 0xD8);
    angle1 = D_8008EBA0[idx] & 0xFFF;
    angle2 = (((ent->unk_2C.x * 16) + ent->unk_2C.y + (ent->unk_2C.z * 8)) & 0x7FF) - 0x400;
    /* identity 3x3 rotation matrix at obj+0xD8 */
    *(s16 *)(obj + 0xD8) = 0x1000;
    *(s16 *)(obj + 0xDA) = 0;
    *(s16 *)(obj + 0xDC) = 0;
    *(s16 *)(obj + 0xDE) = 0;
    *(s16 *)(obj + 0xE0) = 0x1000;
    *(s16 *)(obj + 0xE2) = 0;
    *(s16 *)(obj + 0xE4) = 0;
    *(s16 *)(obj + 0xE6) = 0;
    *(s16 *)(obj + 0xE8) = 0x1000;
    RotMatrixY(angle1, mat);
    RotMatrixX(angle2, mat);

    /* PsyQ libgte inline macro gte_SetRotMatrix(r) --- loads the 5 packed
     * rotation-matrix words at r into cop2 control regs $0..$4.  Same island
     * as the matched func_800203B4 (src/code6cac.c) / func_8002E838. */
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
    vec = &ent->unk_44;
    /* PsyQ libgte inline macro gte_ldv0(r) --- pack VX0/VY0 into one word,
     * mtc2 to $0, lwc2 VZ0 into $1, then the 2-cycle GTE load delay. */
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
    /* GTE MVMVA sf=1, mx=rotation, v=V0, cv=none --- cop2 command 0x0486012. */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ libgte inline macro gte_stlvnl(r) --- store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r (rotated vector written back in place). */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(vec) : "$12", "memory");

    if ((u32)(angle1 - 0x401) < 0x7FFU) {
        ent->unk_44.x /= 8;
        ent->unk_44.y /= 8;
        ent->unk_44.z /= 8;
    } else {
        ent->unk_44.x /= 4;
        ent->unk_44.y /= 4;
        ent->unk_44.z /= 4;
    }
    ent->unk_2C.x += ent->unk_44.x / 2;
    ent->unk_2C.y += ent->unk_44.y / 2;
    ent->unk_2C.z += ent->unk_44.z / 2;
}
