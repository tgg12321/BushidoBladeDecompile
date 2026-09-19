/* func_8002DAD0 - session 6 candidate, re-cited session 7 (layer-1 FAIL
 * 2026-09-18 22:09, citation-only: three islands' precedent was wrongly
 * given as func_8002E838/func_800203B4 where the real in-file precedent for
 * those three is func_8002FC80/func_8002FDB0's own addressing-preamble
 * idiom; two islands also mislabeled gte_ldlv0 as gte_ldv0), re-cited again
 * session 8 (layer-1 FAIL 2026-09-18 22:39, citation-only: three islands
 * carry `nop` instructions that fall outside every macro range the prior
 * comments cited - the OP-invocation nop;nop pairs belong to their OWN
 * named zero-arg SDK macros, not to the load macro they were attributed
 * to). MEASURED: sandbox --disable all == 0 (204/204 insns; every
 * remaining --diff hunk is a masked branch-target artifact), and a full
 * clean build links to SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == the
 * oracle. No C construct changed from session 6 - this pass only corrects
 * island comments per cluster condition 3 (owner ruling 2026-09-02): every
 * island now names its PsyQ macro and inline_c.h/gtemac.h header line(s),
 * read directly against the header copy at
 * tmp/grind/motion_SetMotion/s7/repos/rood-reverse/include/psx/inline_c.h
 * and .../gtemac.h this session.
 *
 * Apply in place of the INCLUDE_ASM("asm/funcs", func_8002DAD0); line in
 * src/code6cac_b.c (after func_8002D780, before func_8002DE20). Needs the
 * existing `extern u8 D_8008D118;` (src/code6cac_b.c:278) and
 * `extern void RotMatrixX(s32, s32 *); extern void RotMatrixY(s32, s32 *);`
 * (src/code6cac_b.c:93-94) in scope - no new externs.
 *
 * NOTE FOR INTEGRATION: the six __asm__ islands are canonical GTE/cop2 SDK
 * macro bodies (`canonical func_8002DAD0` => ASM-PARTIAL, 29/204 insns
 * canonical-asm). Three of them (the diagonal gte_ldopv1 preamble, the
 * LZCS/LZCR gte_Lzc block, and gte_SetRotMatrix) share the cop2-addressing-
 * preamble cluster's OWN `addu/move $r,%,$zero` + immediate ctc2/mtc2 idiom
 * and are precedented by the cluster's own exemplar members func_8002FC80
 * (.claude/rules/cop2-addressing-preamble-cluster.md:83) and func_8002FDB0
 * (:84). The other three (gte_ldopv2 + gte_op0, gte_stlvnl, and the two
 * gte_ldlv0+MVMVA+gte_stlvnl pairs) are character-identical SDK-macro-body
 * spellings already authorized in this same file for func_8002E838
 * (inline_asm_canonical.txt:373) and func_800203B4 (:367). func_8002DAD0 is
 * an owner-enumerated member of the cop2-addressing-preamble cluster
 * (.claude/rules/cop2-addressing-preamble-cluster.md:76) but does NOT yet
 * have its own inline_asm_canonical.txt row; writing that row is an
 * operator/driver step (that file is outside a grind session's allowed
 * surface).
 */
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

    /* PsyQ libgte inline macro gte_ldopv1(r) (inline_c.h:210-219) - loads
     * the OP diagonal (RT11/RT22/RT33) into cop2 control regs $0/$2/$4 from
     * vecA; feeds the GTE OP (cross product) instruction below. The
     * move+lw+ctc2 addressing preamble is the cop2-addressing-preamble
     * cluster's own idiom, not the SDK-macro-body precedent used below -
     * precedent func_8002FC80 (.claude/rules/cop2-addressing-preamble-cluster.md:83)
     * / func_8002FDB0 (:84), same "addu/move $r,%,$zero -> lw;lw;ctc2" shape
     * the cluster's own membership table exhibits. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "lw     $15, 8($12)\n"
        "ctc2   $14, $2\n"
        "ctc2   $15, $4\n"
        :: "r"(obj + 0xA8) : "$12", "$13", "$14", "$15");

    /* PsyQ libgte inline macro gte_ldopv2(r) (inline_c.h:221-226) - loads
     * IR1/IR2/IR3 ($9/$10/$11) directly from vecB. Same-file SDK-macro-body
     * precedent (this exact lwc2-triple spelling): func_8002E838
     * (inline_asm_canonical.txt:373) / func_800203B4 (:367). Then the 2x
     * `nop` + `.word` GTE-OP-invocation tail is PsyQ libgte macro gte_op0()
     * (inline_c.h:784-787 = `nop; nop; .word <op-encoding>`) - the OP
     * (outer/cross product of the IR vector with the rotation-matrix
     * diagonal set above), sf=0. Our island's `.word 0x4B70000C` differs
     * from gte_op0()'s own placeholder encoding only in the packed
     * sf/operation-select bits (same cop2 OP opcode class, same
     * `.word 0x4B70000C` encoding already committed at
     * src/code6cac_b.c:1717-1718 and src/display.c:2657); the `nop; nop;`
     * delay-slot pair is gte_op0()'s own macro text, not gte_ldopv2's. */
    __asm__ volatile(
        "addu   $12, %0, $zero\n"
        "lwc2   $11, 8($12)\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "nop\n"
        "nop\n"
        ".word 0x4B70000C\n"
        :: "r"(obj + 0xB8) : "$12");

    /* PsyQ libgte inline macro gte_stlvnl(r) (inline_c.h:1111-1117) -
     * stores MAC1/MAC2/MAC3 ($25/$26/$27, "memory" clobber per the macro's
     * own clobber list) to obj+0xC8/CC/D0 (unclamped cross-product).
     * Same-file precedent: func_8002E838 (inline_asm_canonical.txt:373) /
     * func_800203B4 (:367). */
    __asm__ volatile(
        "swc2   $25, 0(%0)\n"
        "swc2   $26, 4(%0)\n"
        "swc2   $27, 8(%0)\n"
        :: "r"(obj + 0xC8) : "memory");

    if ((u32)(*(s32 *)(obj + 0xC8) + 0x3FFF) < 0x7FFF &&
        (u32)(*(s32 *)(obj + 0xCC) + 0x3FFF) < 0x7FFF &&
        (u32)(*(s32 *)(obj + 0xD0) + 0x3FFF) < 0x7FFF) {
        return 0;
    }

    angle1 = ratan2(*(s32 *)(obj + 0xC8), *(s32 *)(obj + 0xD0));
    *(s32 *)(obj + 0xC8) = *(s32 *)(obj + 0xC8) >> 6;
    *(s32 *)(obj + 0xCC) = *(s32 *)(obj + 0xCC) >> 6;
    /* FAKE: the scaled Z delta is staged through the function's existing
     * `dist` local (whose distance value is only assigned by the if/else
     * below, so `dist` is dead at this point) instead of a fresh
     * block-local temp, mechanism: GCC 2.7.2 global.c expand_preferences
     * (tools/gcc-2.7.2/global.c:828) - a single-block fresh temp is
     * local-alloc'd to a hard reg ($a1), so global.c set_preference
     * (tools/gcc-2.7.2/global.c:1670) stamps $a1 onto the allocno of the
     * dying `dz*dz` product, and expand_preferences then merges that
     * preference onto dist_sq's allocno (the product dies in the insn that
     * defines dist_sq and the two do not conflict), overriding find_reg's
     * natural ascending pick of $a0 that the target uses; a variable
     * referenced in more than one basic block is a GLOBAL allocno
     * (reg_renumber == -1 during global_conflicts), so no preference is
     * stamped at all and dist_sq lands in $a0,
     * lever-exhaustion: memory/grind/func_8002DAD0/hypotheses.md s3 + s5 -
     * 11 banked instance kills (addition-operand order, store/compute
     * reorder, both compound-assignment splits, fresh named intermediate,
     * local-declaration order) plus the exhaustive tools/spelling_enum.py
     * sweep of both flat blocks touching dist_sq/dist, all measured 6. */
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
            /* PsyQ libgte macro gte_Lzc(r1,r2) (gtemac.h:174-178 = ldlzc
             * inline_c.h:228-231 + 2x nop :1346-1347 + stlzc :1318-1322) -
             * leading-zero-count block (LZCS in via mtc2, LZCR out via
             * swc2 to the stack slot sp_tmp). The addu $t4,%,$zero + mtc2
             * preamble is the cop2-addressing-preamble cluster's own idiom -
             * precedent func_8002FC80 (.claude/rules/cop2-addressing-preamble-cluster.md:83)
             * / func_8002FDB0 (:84); same-file spelling precedent
             * func_8001A67C / func_8002E838. */
            __asm__ volatile(
                "addu   $t4, %1, $zero\n"
                "mtc2   $t4, $30\n"
                "nop\n"
                "nop\n"
                "addiu  $v0, $sp, 0x10\n"
                "addu   $t4, $v0, $zero\n"
                "swc2   $31, 0($t4)\n"
                : "=m"(sp_tmp)
                : "r"(dist_sq)
                : "$2", "$12");
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
    RotMatrixY(*(s16 *)(obj + 0xFA), mat);
    RotMatrixX(*(s16 *)(obj + 0xF8), mat);

    /* PsyQ libgte inline macro gte_SetRotMatrix(r) (inline_c.h:297-310) -
     * loads the 5 packed rotation-matrix words at r into cop2 control regs
     * $0..$4. The move+lw+ctc2 addressing preamble is the cop2-addressing-
     * preamble cluster's own idiom - precedent func_8002FC80
     * (.claude/rules/cop2-addressing-preamble-cluster.md:83) / func_8002FDB0
     * (:84); same-spelling precedent func_800203B4
     * (inline_asm_canonical.txt:367) / func_8002E838 (:373). */
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

    /* PsyQ libgte inline macro gte_ldlv0(r) (inline_c.h:101-110 - the
     * lhu/lhu/sll/or VX0/VY0 pack; this is gte_ldlv0, NOT gte_ldv0, whose
     * body at inline_c.h:16-20 is a bare lwc2 pair with no such pack) +
     * MVMVA sf=1/mx=rotation/v=V0 (.word 0x4A486012, the gte_rtv0-class
     * MVMVA encoding already committed at src/code6cac_b.c:1543/1902/1985
     * and src/code6cac.c:2099) + gte_stlvnl(r) (inline_c.h:1111-1117):
     * rotate vecA in place. Same-file precedent: func_800203B4
     * (inline_asm_canonical.txt:367) / func_8002E838 (:373). The 2x `nop`
     * after the lwc2 is the delay-slot pair belonging to the gte_rtv0()-
     * CLASS op-invocation macro (inline_c.h:499-502, `nop; nop; .word
     * <op-encoding>`), not to gte_ldlv0 (inline_c.h:101-110 has no
     * trailing nop) - our MVMVA `.word 0x4A486012` differs from
     * gte_rtv0()'s own `.word 0x0000013f` only in the packed sf/mx/v
     * operand-select bits; every op-invocation macro in this header
     * (gte_op0, gte_rtv0/1/2, gte_gpf12, gte_gpf0, ...) shares this
     * identical "2x nop delay, then .word <op>" body shape. */
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
        :: "r"(obj + 0xA8) : "$12", "$13", "$14");
    __asm__ volatile(".word 0x4A486012");
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(obj + 0xA8) : "$12", "memory");

    /* Same gte_ldlv0 (inline_c.h:101-110) + MVMVA (.word 0x4A486012) +
     * gte_stlvnl (inline_c.h:1111-1117) sequence: rotate vecB in place.
     * Same gte_rtv0()-class op-invocation delay-nop attribution as the
     * previous island (inline_c.h:499-502) applies to this island's 2x
     * `nop` as well. */
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
        :: "r"(obj + 0xB8) : "$12", "$13", "$14");
    __asm__ volatile(".word 0x4A486012");
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(obj + 0xB8) : "$12", "memory");

    return 1;
}
