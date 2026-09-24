/* Matrix -> Euler angles. Copies the MATRIX at a0 into scratchpad 0x1F800390,
 * takes the first column (c0,c1,c2)/det and a second cofactor triple (r0,r1,r2)
 * of its inverse, derives ang_z = -atan(c1/c0) and ang_y = atan(c2/|c0,c1|)
 * (integer sqrt through the D_8008D118 byte LUT, GTE leading-zero count above
 * 0x400), rebuilds that rotation at scr+0xD8 (the same 0x1F800390 slot),
 * rotates (r0,r1,r2) through it and stores the remaining angle; a1 receives
 * three s16 angles. The whole body also appears inlined at the tail of
 * func_8002F770 (asm/funcs/func_8002F770.s L224-L268 for the sqrt). */
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
    /* FAKE: do-while(0) wrap, effect: loop-note ref weighting (flow.c counts a
     * reference at loop_depth) lifts i2 to nrefs 3, global.c priority 697,
     * between ang_z (517) and the a1 parameter (714), so i2 seats in $s4 and
     * ang_z in $s5; unwrapped they swap (5/270). */
    do {
        i2 = c2 / det;
    } while (0);
    r0 = (m->m[1][0] * m->m[2][2] - m->m[1][2] * m->m[2][0]) / det;
    r1 = (m->m[0][2] * m->m[2][0] - m->m[0][0] * m->m[2][2]) / det;
    r2 = (m->m[0][0] * m->m[1][2] - m->m[0][2] * m->m[1][0]) / det;

    ang_z = -ratan2(c1, c0);
    scr = (u8 *)0x1F8002B8;
    sum = c0 * c0 + c1 * c1;
    /* FAKE: det is reused for the sqrt result -- the target keeps both in one
     * pseudo ($t2); a separate variable costs 35. */
    if ((u32)sum < 0x400) {
        det = (u32)*(((u8 *)&D_8008D118) + sum) >> 3;
    } else {
        s32 lzcr = 0;
        if (sum >= 0) {
            /* PsyQ libgte macro gte_Lzc(r1,r2) --- gtemac.h:174-178, which
             * composes gte_ldlzc (inline_c.h:228-231, "mtc2 %0,$30"), two
             * gte_nop (inline_c.h:1346-1347) for the GTE result delay, and
             * gte_stlzc (inline_c.h:1318-1322, "swc2 $31,0(%0)").
             * ADDRESSING PREAMBLE: the "addu $t4,%1,$zero" copy and the
             * "addiu $v0,$sp,0x10; addu $t4,$v0,$zero" pair are not macro text
             * -- they are the widened materialize-then-copy anchor admitted by
             * the owner grant of 2026-09-01 (docs/grind/decisions.md:18082).
             * CLOBBER PROVENANCE: neither half publishes a GPR clobber, so "$2"
             * ($v0, written by the preamble) and "$12" ($t4, written by both
             * copies) are ADDED here and are truthful. The "=m"(sp_tmp) output
             * covers the store. Template character-identical to the matched
             * func_8002EBDC island in this file. */
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
            /* FAKE: sum is reused for the table byte. It keeps the sum a
             * multi-set pseudo whose full-preferences come from the byte's
             * load, so global.c seats it in $a0 like the target; with a fresh
             * `tbl` local the sum inherits $s0/$s1 from the squares' operands
             * (expand_preferences) and lands in $s0 (6/270). */
            s32 tbl = *(((u8 *)&D_8008D118) + ((u32)sum >> shift));
            det = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
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
     * weighting lifts mat's local-alloc quantity to 5 refs (priority .294 over
     * ang_y's .278), so mat seats in $s0 and ang_y in $s1; unwrapped they swap
     * (8/270). Same device as the func_800300B4 gte_stlvnl wrap (this file). */
    do {
        /* PsyQ libgte macro gte_SetRotMatrix(r) --- inline_c.h:297-310. Loads
         * the 5 packed rotation-matrix words at r into cop2 control regs
         * $0..$4.
         * ADDRESSING PREAMBLE: "move $12, %0" is the granted anchor
         * (owner grant 2026-09-01, docs/grind/decisions.md:18082), not macro
         * text.
         * CLOBBER PROVENANCE: the macro publishes "$12","$13","$14" only; "$15"
         * is ADDED here, a consequence of the island's register shift (the
         * macro's $12/$13/$14 become $13/$14/$15 once $12 holds the anchored
         * base). It is truthful -- the island writes $15 at "lw $15, 16($12)".
         * Character-identical to the matched func_8002EBDC island. */
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
    /* PsyQ libgte macro gte_ldlv0(r) --- PsyQ 4.5 inline_c.h:101-110. Packs
     * VX0/VY0 into one word (the lhu/lhu/sll/or is the macro's own published
     * text, admitted by cluster condition 3), mtc2 to $0, lwc2 VZ0 into $1,
     * then the 2-cycle GTE load delay.
     * ADDRESSING PREAMBLE: "move $12, %0" is the granted anchor
     * (owner grant 2026-09-01, docs/grind/decisions.md:18082).
     * CLOBBER PROVENANCE: the macro publishes "$12","$13" only; "$14" is ADDED
     * here by the same register shift and is truthful (the island writes $14).
     * Character-identical to the matched func_8002EBDC island. */
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
    /* PsyQ libgte macro gte_mvmva(sf,mx,v,cv,lm) --- inline_c.h:816-817 over
     * gte_mvmva_core inline_c.h:809-814. cop2 command 0x0486012
     * (.word 0x4A486012) = MVMVA sf=1, mx=rotation, v=V0, cv=none, lm=0.
     * NOT gte_rtv0: that macro (inline_c.h:499-502) emits .word 0x0000013f.
     * No clobber list on either side. */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ libgte macro gte_stlvnl(r) --- inline_c.h:1111-1117. Stores
     * MAC1/MAC2/MAC3 ($25/$26/$27) back to r.
     * ADDRESSING PREAMBLE: "move $12, %0" is the granted anchor
     * (owner grant 2026-09-01, docs/grind/decisions.md:18082).
     * CLOBBER PROVENANCE: the macro publishes only "memory" (inline_c.h:1117);
     * "$12" is ADDED here and is truthful -- the preamble writes it. Same
     * addition as the matched func_8002EBDC gte_stlvnl island. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(vec) : "$12", "memory");

    ((s16 *)a1)[0] = ratan2(vec[2], vec[1]);
    ((s16 *)a1)[1] = -ang_y;
    ((s16 *)a1)[2] = -ang_z;
}
