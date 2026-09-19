/* func_8002DAD0 - session 6 (synthesis). MEASURED THIS SESSION: `sandbox
 * func_8002DAD0 --disable all` == 0 (204/204 insns) with this body applied to
 * src/code6cac_b.c. The C body is UNCHANGED from the session-6/7/8 form (same
 * body hash); this pass only re-cites the island comments, which the two
 * layer-1 citation FAILs of 2026-09-18 were about.
 *
 * ISLAND PROVENANCE (the finding that closes those FAILs): the islands are
 * NOT hand-written and are NOT inline_c.h macro bodies. Each one is the
 * verbatim expansion of a named Sony macro from PsyQ's DMPSX header
 * `inline_o.h` ("Macro definitions of DMPSX version 3", $PSLibId: Run-time
 * Library Release 4.5$, Copyright(C) 1996 Sony Computer Entertainment Inc.),
 * which spells every GTE primitive as a run of single-instruction
 * `__asm__ volatile` blocks that stage the address through a hard `$12`
 * (`move $12,%0`) and hard-code `$13/$14/$15`. inline_c.h spells the same
 * primitives `%0`-relative with no preamble - which is why every earlier
 * attribution left the `move`/`addu` preamble and the delay `nop`s
 * unaccounted for. Full island-by-island table, with header line numbers and
 * the header banner:  memory/grind/func_8002DAD0/psyq_inline_o_provenance.md
 *
 * MEASURED ALTERNATIVES (session 6, all on this chassis):
 *   - island 3 respelled as the authentic inline_c.h `%0`-relative
 *     gte_stlvnl (no `move $12` preamble, zero hard GPRs): score 0 -> 4,
 *     build_insns 204 -> 203. The preamble instruction is IN the target
 *     bytes; C cannot supply it.  rejected/s6-stlvnl-pct0-offset-score4.c
 *   - island 1 respelled as the VERBATIM inline_o.h per-instruction form:
 *     the engine's cheat-stripper classifies each GPR-only single-insn block
 *     as cheat-asm (stripped 21 -> 25) and the function no longer builds in
 *     the sandbox - unscorable.  rejected/s6-inline-o-per-insn-stripped.c
 *   - whole body rewritten as Sony macro INVOCATIONS with the macro set
 *     defined TU-locally: the stripper removes the macro bodies too (21 ->
 *     46 stripped, score 83). Only a HEADER (include/gte.h) can host them -
 *     write_stripped only ever rewrites src/<stem>.c - and that file is
 *     outside a grind session's scope.  rejected/s6-sdk-macro-tu-local-score83.c
 *
 * Apply in place of the INCLUDE_ASM("asm/funcs", func_8002DAD0); line in
 * src/code6cac_b.c (after func_8002D780, before func_8002DE20). Needs the
 * existing `extern u8 D_8008D118;` (src/code6cac_b.c:278) and
 * `extern void RotMatrixX(s32, s32 *); extern void RotMatrixY(s32, s32 *);`
 * (src/code6cac_b.c:93-94) in scope - no new externs. The `(u8 *)&D_8008D118`
 * indexing spelling is the one already committed on main in this same file
 * for its matched siblings (src/code6cac_b.c:1468 and :1490).
 *
 * INTEGRATION: the 9 islands are cop2/canonical but carry the macros' own
 * GPR preamble instructions, so the driver's island gate (owner Ruling C
 * 2026-09-02) refuses the merge until func_8002DAD0 is admitted by a grant
 * door. See the 2026-09-18 INTEGRATION HANDOFF entry in docs/grind/decisions.md.
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

    /* Sony PsyQ DMPSX macro gte_ldopv1(r1) - inline_o.h:192-200, expanded
     * verbatim (`move $12,%0` · `lw $13,($12)` · `lw $14,4($12)` ·
     * `ctc2 $13,$0` · `lw $15,8($12)` · `ctc2 $14,$2` · `ctc2 $15,$4`).
     * Loads the OP diagonal (RT11/RT22/RT33) into cop2 control regs
     * $0/$2/$4 from vecA. Table: memory/grind/func_8002DAD0/psyq_inline_o_provenance.md
     * Cluster membership: .claude/rules/cop2-addressing-preamble-cluster.md:76 */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "lw     $15, 8($12)\n"
        "ctc2   $14, $2\n"
        "ctc2   $15, $4\n"
        :: "r"(obj + 0xA8) : "$12", "$13", "$14", "$15");

    /* Sony PsyQ DMPSX macro gte_ldopv2(r1) - inline_o.h:201-206, expanded
     * verbatim (`move $12,%0` · `lwc2 $11,8($12)` · `lwc2 $9,($12)` ·
     * `lwc2 $10,4($12)`): loads IR1/IR2/IR3 from vecB. The trailing
     * `nop; nop; .word` is gte_op0() (inline_o.h:711-715 / inline_c.h:784-787),
     * the GTE OP (outer product) invocation; the SDK headers carry the DMPSX
     * placeholder word (0x0000127f) where we carry the real cop2 encoding
     * 0x4B70000C, the same real-encoding convention include/gte.h:88-89 uses
     * for gte_mvmva and already committed at src/code6cac_b.c:1717-1718. */
    __asm__ volatile(
        "addu   $12, %0, $zero\n"
        "lwc2   $11, 8($12)\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "nop\n"
        "nop\n"
        ".word 0x4B70000C\n"
        :: "r"(obj + 0xB8) : "$12");

    /* Sony PsyQ DMPSX macro gte_stlvnl(r1) - inline_o.h:904-909, expanded
     * verbatim (`move $12,%0` · `swc2 $25,($12)` · `swc2 $26,4($12)` ·
     * `swc2 $27,8($12)`): stores MAC1/MAC2/MAC3 (the unclamped cross
     * product) to obj+0xC8/CC/D0. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(obj + 0xC8) : "$12", "memory");

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
            /* Sony PsyQ macro gte_Lzc(r1,r2) - gtemac.h:174-178, whose
             * body is gte_ldlzc (inline_o.h:207-211) + 2x gte_nop
             * (inline_o.h:1095) + gte_stlzc (inline_o.h:1074-1077), each
             * expanded verbatim: `move $12,%0` · `mtc2 $12,$30` · `nop` ·
             * `nop` · `move $12,%0` · `swc2 $31,($12)`. The second
             * `move $12,%0` takes &sp_tmp, which is why GCC materialises
             * `addiu $v0,$sp,0x10` ahead of it. LZCS in, LZCR out. */
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

    /* Sony PsyQ DMPSX macro gte_SetRotMatrix(r1) - inline_o.h:272-284,
     * expanded verbatim (all 11 instructions, including the $13/$14 re-use
     * order): loads the 5 packed rotation-matrix words at r into cop2
     * control regs $0..$4. */
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

    /* Sony PsyQ DMPSX macro gte_ldlv0(r1) - inline_o.h:95-103, expanded
     * verbatim (`move $12,%0` · `lhu $14,4($12)` · `lhu $13,($12)` ·
     * `sll $14,$14,16` · `or $13,$13,$14` · `mtc2 $13,$0` ·
     * `lwc2 $1,8($12)`): packs VX0/VY0 into one word and loads VZ0. The
     * trailing `nop; nop` belongs to the following op-invocation macro
     * (gte_rtv0()-class, inline_o.h:426-430), not to gte_ldlv0. The
     * `.word 0x4A486012` is MVMVA sf=1/mx=R/v=V0/cv=none =
     * gte_mvmva(1,0,0,3,0) under include/gte.h:88-89, already committed at
     * src/code6cac_b.c:1543. Then gte_stlvnl (inline_o.h:904-909) stores the
     * rotated vector back: vecA is rotated in place. */
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

    /* Same gte_ldlv0 (inline_o.h:95-103) + gte_rtv0()-class MVMVA
     * (inline_o.h:426-430, `.word 0x4A486012`) + gte_stlvnl
     * (inline_o.h:904-909) sequence, this time rotating vecB in place. */
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
