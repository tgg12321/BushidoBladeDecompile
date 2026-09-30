/* RETRO-AUDIT 2026-09-29 FAIL -- func_8002EBDC reopened (Q38, owner rulings 803d0fea1).
 * Landed on main in 9b6db9034 (src/code6cac_b.c); this is that landed text, verbatim from main
 * as of the reopen, banked before the body went back to INCLUDE_ASM.
 * FAIL: canonical grant rows (9bdfcc6cc, 'owner-instructed 2026-09-21') have no recorded owner instruction, and the islands do not qualify under the 2026-09-26 inline_o.h class grant (audit-q38).
 * Detail: tmp/audit-2026-09-29/review/batch_00.md / batch_01.md (gitignored), tmp/audit-2026-09-29/SUMMARY.md;
 * rulings: docs/grind/owner-rulings-2026-09-26.md Q37/Q38 (803d0fea1).
 * Reopen notes: Registry rows removed (commented): inline_asm_canonical.txt, tools/grinder/owner_cluster_grants.txt, tools/canonical_asm_regions.json. func_8002F2D0's islands cite this body as 'character-identical' precedent.
 * DO NOT resubmit this body as-is. */

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
            /* PsyQ libgte macro gte_Lzc(r1,r2) --- gtemac.h:174-178, which
             * composes gte_ldlzc (inline_c.h:228-231, "mtc2 %0,$30"), two
             * gte_nop (inline_c.h:1346-1347) for the GTE result delay, and
             * gte_stlzc (inline_c.h:1318-1322, "swc2 $31,0(%0)").
             * ADDRESSING PREAMBLE: the "addiu $v0,$sp,0x10; addu $t4,$v0,$zero"
             * pair is not macro text -- it is the widened materialize-then-copy
             * anchor admitted by the owner grant of 2026-09-01
             * (docs/grind/decisions.md:17921), materialising &sp_tmp for
             * gte_stlzc's %0.
             * CLOBBER PROVENANCE: neither half publishes a GPR clobber --
             * gte_ldlzc publishes none at all and gte_stlzc publishes only
             * "memory" -- so "$2" ($v0, written by the preamble) and "$12"
             * ($t4, written by both copies) are ADDED here and are truthful:
             * the island writes both. The "=m"(sp_tmp) output covers the store.
             * The template is character-identical to the user-authorized block
             * in func_8001A67C (src/code6cac.c, inline_asm_canonical.txt entry
             * of 2026-06-10, which names "addiu v0,sp,16" explicitly) and to
             * the matched sibling func_8002E838 in this file. */
            __asm__ volatile(
                "addu   $t4, %1, $zero\n"
                "mtc2   $t4, $30\n"        /* LZCS <- dist_sq */
                "nop\n"
                "nop\n"
                "addiu  $v0, $sp, 0x10\n"  /* &sp_tmp */
                "addu   $t4, $v0, $zero\n"
                "swc2   $31, 0($t4)\n"     /* sp_tmp <- LZCR */
                : "=m"(sp_tmp)
                : "r"(dist_sq)
                : "$2", "$12");
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            s32 tbl = *(((u8 *)&g_sqrt_table_u8) + ((u32)dist_sq >> shift));
            dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }

    angle = ratan2(dir[1], dist);
    mat = (s32 *)(scr + 0xD8);
    *(s16 *)(scr + 0xF8) = 0x800 - angle;

    /* identity 3x3 rotation matrix at scr+0xD8 */
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

    /* PsyQ libgte macro gte_SetRotMatrix(r) --- inline_c.h:297-310. Loads the
     * 5 packed rotation-matrix words at r into cop2 control regs $0..$4.
     * ADDRESSING PREAMBLE: "move $12, %0" is the granted anchor
     * (owner grant 2026-09-01, docs/grind/decisions.md:17921), not macro text.
     * CLOBBER PROVENANCE: the macro publishes "$12","$13","$14" only; "$15" is
     * ADDED here, a consequence of the island's register shift (the macro's
     * $12/$13/$14 become $13/$14/$15 once $12 holds the anchored base). It is
     * truthful -- the island writes $15 at "lw $15, 16($12)".
     * Same island as the matched func_8002E838 (this file) / func_800203B4. */
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
    /* PsyQ libgte macro gte_ldlv0(r) --- PsyQ 4.5 inline_c.h:101-110. Packs
     * VX0/VY0 into one word (the lhu/lhu/sll/or is the macro's own published
     * text, admitted by cluster condition 3), mtc2 to $0, lwc2 VZ0 into $1,
     * then the 2-cycle GTE load delay.
     * ADDRESSING PREAMBLE: "move $12, %0" is the granted anchor
     * (owner grant 2026-09-01, docs/grind/decisions.md:17921).
     * CLOBBER PROVENANCE: the macro publishes "$12","$13" only; "$14" is ADDED
     * here by the same register shift and is truthful (the island writes $14).
     * Same island as func_8002E838. */
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
        :: "r"(vec_in) : "$12", "$13", "$14");
    /* PsyQ libgte macro gte_mvmva(sf,mx,v,cv,lm) --- inline_c.h:816-817 over
     * gte_mvmva_core inline_c.h:809-814. cop2 command 0x0486012
     * (.word 0x4A486012) = MVMVA sf=1, mx=rotation, v=V0, cv=none, lm=0.
     * NOT gte_rtv0: that macro (inline_c.h:499-502) emits .word 0x0000013f, a
     * different encoding. gte_mvmva over gte_mvmva_core is the cite the Judge
     * accepted for this same word class in func_80019310
     * (docs/grind/decisions.md:24515). No clobber list on either side. */
    __asm__ volatile(".word 0x4A486012");
    vec = (s32 *)(scr + 0xA8);
    /* PsyQ libgte macro gte_stlvnl(r) --- inline_c.h:1111-1117. Stores
     * MAC1/MAC2/MAC3 ($25/$26/$27) to r (the rotated input vector, in
     * scratchpad).
     * ADDRESSING PREAMBLE: "move $12, %0" is the granted anchor
     * (owner grant 2026-09-01, docs/grind/decisions.md:17921).
     * CLOBBER PROVENANCE: the macro publishes only "memory" (inline_c.h:1117);
     * "$12" is ADDED here and is truthful -- the preamble writes it. Same
     * addition as func_8002D320's committed lwc2 read island in this file,
     * whose clobber line is `: : "r"(vin) : "$12", "memory"` (:1123 at time of
     * writing -- anchored to the function because this reference has already
     * drifted twice in the record, cited as :935 at decisions.md:20489 and
     * :1038 at decisions.md:26228 as the file grew). That addition was accepted
     * in the func_800300B4 PASS (docs/grind/decisions.md:20489). */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(vec) : "$12", "memory");

    /* scale the rotated vector: z by scale_z, x/y by scale_xy, /256 */
    vec[2] = (vec[2] * scale_z) / 256;
    vec[0] = (vec[0] * scale_xy) / 256;
    vec[1] = (vec[1] * scale_xy) / 256;

    /* identity again, then the inverse rotation (negated angles) */
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

    /* Second GTE pass: the same four macro islands, each identical in template
     * to its first-pass twin above, whose full macro/header citations and
     * ADDED-clobber disclosures apply here unchanged -- gte_SetRotMatrix
     * (inline_c.h:297-310), gte_ldlv0 (inline_c.h:101-110) on the scaled
     * scratchpad vector, gte_mvmva (inline_c.h:816-817 over gte_mvmva_core
     * :809-814), gte_stlvnl (inline_c.h:1111-1117) into `out`. */
    /* PsyQ libgte macro gte_SetRotMatrix(r) --- inline_c.h:297-310; publishes
     * "$12","$13","$14", so "$15" is ADDED (see the first-pass island). */
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
    /* PsyQ libgte macro gte_ldlv0(r) --- PsyQ 4.5 inline_c.h:101-110, on the
     * scaled vector; publishes "$12","$13", so "$14" is ADDED (as above). */
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
     * gte_mvmva_core :809-814; .word 0x4A486012 = MVMVA sf=1/mx=rot/v=V0/cv=none
     * (not gte_rtv0 :499-502, which emits .word 0x0000013f). */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ libgte macro gte_stlvnl(r) --- inline_c.h:1111-1117, into the
     * caller's out[]; publishes only "memory", so "$12" is ADDED (as above). */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(out) : "$12", "memory");
}
