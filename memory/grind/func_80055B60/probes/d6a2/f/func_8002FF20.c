void func_8002FF20(Obj80106A78 *arg0, s16 arg1) {
    s32 mat_local[8];
    s32 *playerData;
    s32 *s2_ptr;
    s32 *rot_mat;
    Vec3i32 *vec;

    arg0->unk_08 = 1;
    arg0->unk_09 = arg1;
    playerData = (s32 *)game_GetPlayerData(arg0->unk_06 < 1);
    rot_mat = (s32 *)&arg0->unk_0C;
    s2_ptr = (s32 *)playerData[arg0->unk_09];

    /* 3x3 identity rotation in the record's matrix. */
    arg0->unk_0C.m[0][0] = 0x1000;
    arg0->unk_0C.m[0][1] = 0;
    arg0->unk_0C.m[0][2] = 0;
    arg0->unk_0C.m[1][0] = 0;
    arg0->unk_0C.m[1][1] = 0x1000;
    arg0->unk_0C.m[1][2] = 0;
    arg0->unk_0C.m[2][0] = 0;
    arg0->unk_0C.m[2][1] = 0;
    arg0->unk_0C.m[2][2] = 0x1000;
    RotMatrixX(arg0->unk_54[0], rot_mat);
    RotMatrixY(arg0->unk_54[1], rot_mat);
    RotMatrixZ(arg0->unk_54[2], rot_mat);
    func_8002EECC(s2_ptr, mat_local);
    MulMatrix0(mat_local, rot_mat, rot_mat);

    /* Subtract opponent reference position from self position. */
    arg0->unk_2C.x -= s2_ptr[5];
    arg0->unk_2C.y -= s2_ptr[6];
    arg0->unk_2C.z -= s2_ptr[7];

    /* PsyQ 4.5 inline_c.h macro gte_SetRotMatrix(r0) --- verbatim macro body:
     * copies the operand into $12, loads the 5 packed rotation-matrix words
     * through $13-$15 and ctc2's them into cop2 control regs $0..$4. */
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
        :: "r"(mat_local) : "$12", "$13", "$14", "$15");
    vec = &arg0->unk_2C;
    /* PsyQ 4.5 inline_c.h:101-110 macro gte_ldlv0(r0) --- verbatim macro body:
     * lhu/lhu/sll/or packs VX0/VY0 (s32 x,y) into one word, mtc2 to $0, lwc2 VZ0
     * into $1; the 2-cycle GTE load delay is carried as explicit nops (maspsx does
     * not supply them in the full-build context -- measured 2026-09-01). */
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
    /* PsyQ 4.5 inline_c.h macro gte_rtv0() --- cop2 MVMVA sf=1, mx=rotation,
     * v=V0, cv=none: the macro's single `.word 0x4A486012` (cop2 0x0486012). */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ 4.5 inline_c.h macro gte_stlvnl(r0) --- verbatim macro body: copies
     * the operand into $12 and swc2's MAC1/MAC2/MAC3 ($25/$26/$27) to r0. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(vec) : "$12");

    /* Halve x, y, z (signed arithmetic shift). */
    arg0->unk_2C.x >>= 1;
    arg0->unk_2C.y >>= 1;
    arg0->unk_2C.z >>= 1;
}
