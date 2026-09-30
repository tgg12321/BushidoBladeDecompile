/* RETRO-AUDIT 2026-09-29 FAIL -- func_8002F770 reopened (Q37 class C + Q38, owner rulings 803d0fea1).
 * Landed on main in d052c5ee7 (src/code6cac_b.c); this is that landed text, verbatim from main
 * as of the reopen, banked before the body went back to INCLUDE_ASM.
 * FAIL: owner_cluster_grants row operator-added (974ce052c) with no recorded owner instruction (the file's own :73-77 correction says so), and the islands do not qualify under the 2026-09-26 inline_o.h class grant (audit-q38); secondary: unannotated det/sum multi-role reuse.
 * Detail: tmp/audit-2026-09-29/review/batch_02.md (gitignored), tmp/audit-2026-09-29/SUMMARY.md;
 * rulings: docs/grind/owner-rulings-2026-09-26.md Q37/Q38 (803d0fea1).
 * Reopen notes: Registry rows removed (commented): inline_asm_canonical.txt, tools/grinder/owner_cluster_grants.txt, tools/canonical_asm_regions.json.
 * DO NOT resubmit this body as-is. */

/* Composes X(x), Y(y), and Z(-z) with the Euler rotation in angles, then
 * converts the resulting matrix back to Euler angles in place. */
void func_8002F770(s16 *angles, s32 z, s32 y, s32 x) {
    MATRIX *m;
    u8 *init_scr;
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
    s32 m00;

    /* Identity matrix at scratchpad arena + 0xD8 (0x1F800390). */
    init_scr = (u8 *)0x1F8002B8;
    *(s16 *)(init_scr + 0xD8) = 0x1000;
    *(s16 *)(init_scr + 0xDA) = 0;
    *(s16 *)(init_scr + 0xDC) = 0;
    *(s16 *)(init_scr + 0xDE) = 0;
    *(s16 *)(init_scr + 0xE0) = 0x1000;
    *(s16 *)(init_scr + 0xE2) = 0;
    *(s16 *)(init_scr + 0xE4) = 0;
    *(s16 *)(init_scr + 0xE6) = 0;
    *(s16 *)(init_scr + 0xE8) = 0x1000;
    RotMatrixX(x, (s32 *)0x1F800390);
    RotMatrixY(y, (s32 *)0x1F800390);
    RotMatrixZ(-z, (s32 *)0x1F800390);
    RotMatrixX(angles[0], (s32 *)0x1F800390);
    RotMatrixY(angles[1], (s32 *)0x1F800390);
    RotMatrixZ(angles[2], (s32 *)0x1F800390);

    m = (MATRIX *)0x1F800390;
    c0 = m->m[1][2] * m->m[2][1] - m->m[1][1] * m->m[2][2];
    m00 = m->m[0][0];
    d0 = m00 * (c0 >> 12);
    c1 = m->m[0][1] * m->m[2][2] - m->m[0][2] * m->m[2][1];
    c2 = m->m[0][2] * m->m[1][1] - m->m[0][1] * m->m[1][2];
    det = (d0 + m->m[1][0] * (c1 >> 12) + m->m[2][0] * (c2 >> 12)) >> 12;
    c0 = c0 / det;
    c1 = c1 / det;
    /* FAKE: do-while(0) wrap, effect: loop-note ref weighting lifts i2 above
     * ang_z in global.c allocation priority, seating i2 in $s4 and ang_z in
     * $s5; removing this complete wrapper scores 5/298. */
    do {
        i2 = c2 / det;
    } while (0);
    r0 = (m->m[1][0] * m->m[2][2] - m->m[1][2] * m->m[2][0]) / det;
    r1 = (m->m[0][2] * m->m[2][0] - m00 * m->m[2][2]) / det;
    r2 = (m00 * m->m[1][2] - m->m[0][2] * m->m[1][0]) / det;

    ang_z = -ratan2(c1, c0);
    scr = (u8 *)0x1F8002B8;
    sum = c0 * c0 + c1 * c1;
    if ((u32)sum < 0x400) {
        det = (u32)*(((u8 *)&g_sqrt_table_u8) + sum) >> 3;
    } else {
        s32 lzcr = 0;
        if (sum >= 0) {
            /* PsyQ gte_Lzc (gtemac.h:174-178): load LZCS, wait two cycles,
             * then store LZCR. Character-identical to func_8002F2D0. */
            __asm__ volatile(
                "addu   $t4, %1, $zero\n"
                "mtc2   $t4, $30\n"
                "nop\n"
                "nop\n"
                "addiu  $v0, $sp, 0x10\n"
                "addu   $t4, $v0, $zero\n"
                "swc2   $31, 0($t4)\n"
                : "=m"(sp_tmp)
                : "r"(sum)
                : "$2", "$12");
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

    /* FAKE: do-while(0) wrap around gte_SetRotMatrix, effect: loop-note ref
     * weighting lifts mat above ang_y in global.c allocation priority, seating
     * mat in $s0 and ang_y in $s1; removing this complete wrapper scores
     * 8/298. */
    do {
        /* PsyQ gte_SetRotMatrix (inline_c.h:297-310): load the five packed
         * rotation words. Character-identical to func_8002F2D0. */
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
    } while (0);
    vec = (s32 *)(scr + 0xA8);
    vec[0] = r0;
    vec[1] = r1;
    vec[2] = r2;
    /* PsyQ gte_ldlv0 (inline_c.h:101-110): load the packed vector into V0;
     * character-identical to func_8002F2D0. */
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
    /* PsyQ gte_mvmva(1, 0, 0, 3, 0), inline_c.h:809-817;
     * character-identical to func_8002F2D0. */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ gte_stlvnl (inline_c.h:1111-1117): store MAC1/MAC2/MAC3;
     * character-identical to func_8002F2D0. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(vec) : "$12", "memory");

    angles[0] = ratan2(vec[2], vec[1]);
    angles[1] = -ang_y;
    angles[2] = -ang_z;
}
