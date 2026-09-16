/* func_8006CCC8 -- src/text1b.c -- session s7 (synthesis modality), 2026-09-16
 * [s7 solver addendum, 2026-09-16] RE-MEASURED 0 (189/189) on HEAD 4b2d932bc
 * via tmp/grind/func_8006CCC8/s7/apply2.py (the older apply.py is broken).
 * STATUS: layer-1 FAILed 12:25 for `nib = 0xF` un-annotated (constant-holder
 * classification); the driver BANNED the construct, so this body is held for
 * a ruling (outcome: ruling-request). Both solvers type the nib-free residual
 * as "the constant-15 set must be a source statement between i = 0 and
 * fade = 0" (single sched vector luid_move 547 -> before 54); five nib-free /
 * repositioned spellings measured 2/4/2/29/21 (rejected/). Do NOT respell:
 * submit this body exactly once the ruling lands, annotated iff ruled FAKE.
 * Sandbox --disable all: score 0 (target_insns=189, build_insns=189), measured
 * repeatedly this session with THIS body resident in src/text1b.c. The caller
 * func_8006D338 also sandboxes at 0 with the corrected forward declaration
 * (`extern s32 func_8006CCC8(s32 *, s32 *, s16);` replaced the stale
 * `extern void func_8006CCC8(s32, s32, s32);`; the definition now precedes
 * the caller so the extern is simply deleted in the applied diff).
 *
 * MIGRATION BANNER: main still carries INCLUDE_ASM("asm/funcs", func_8006CCC8)
 * (asm-until-matched). This body is applied to the WORKING TREE only; it lands
 * on main at COMPLETED-C after layer-1 + Judge review.
 *
 * s7 changes from the s6 body (floor 18 -> 0), all ordinary C, no FAKE
 * constructs, no sanctioned-family claims:
 *
 * FIX 1 (18 -> 10): the field28 dispatch is a `switch (field)` with
 * `case 0: case 1: case 2:` (the func_8006CBD4 arm), `case 3:` (+0x1A record
 * arm) and `case 4:` (+0x1D record arm), bodies in that source order. GCC
 * 2.7.2 stmt.c expand_end_case takes the decision-tree path (3 case nodes <
 * CASE_VALUES_THRESHOLD 5, mips has no casesi) and emit_case_nodes on the
 * balanced tree {[0..2], [3], [4]} emits exactly target's test group:
 * `beq field,3 -> case3` (do_jump_if_equal on the root), a `field > 3`
 * split (`slti v0,field,4` + branch), the [0..2] leaf's `bltz field ->
 * default` low-bound range check (node_has_low_bound fails because no
 * parent tests -1; node_has_high_bound succeeds because 2+1 == root low 3,
 * so no upper check), `beq field,4 -> case4`, then `j default`. The
 * s4-s6 `field >= 0 &&` guard was the switch's own range check, and the
 * nested if/else could only approximate the block layout (L, A3, A4) but not
 * the contiguous jump-if-true test group. Byte-proven; the exact jump.c
 * rewrite that turns `beqz -> test_label; bltz; j L02; test_label:` into
 * target's `bnez -> L02'; ...; L02': bltz` was NOT read off a .jump dump
 * (the bytes made it moot) -- see evidence.md s7 if a sibling needs it.
 *
 * FIX 2 (10 -> 0): the two record-update j-loops select the NIBBLE MASK with
 * a ternary on a single read of the +0x17 byte:
 *   *(rec + 0x17) = (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + masked);
 * This is target's literal shape (one lbu 0x17, bnez s1 with a nop slot,
 * `j; andi 0xF0` / `andi 0xF`, addu with the masked +0x1A/+0x1D byte, sb).
 * The s5/s6 split-read if/else (two reads, one per arm) and the s3 hoisted
 * `byte17` local were BOTH wrong: the first duplicates the lbu, the second
 * changes register allocation (58 on this chassis). A ternary over two
 * direct reads is byte-identical to the split-read if/else (10) -- the win
 * is selecting the CONSTANT, not the read.
 *
 * ALSO REMOVED: the s2 variable-reuse lever `t` (`t = arg2; ... t = i;`).
 * On this chassis `lim = ((arg2 >> i) & 1) ? 4 : 5;` and
 * `func_8006CBD4(i, *arg1)` measure 0 too, so the body carries no
 * defeat-licm-hoist-var-reuse claim at all.
 *
 * KEPT: `nib = 0xF;` (read twice as `nib << fade`). Spelling the literal
 * `0xF << fade` scores 2: loop.c hoists the constant load to the END of the
 * loop preheader (after `shift = 0`), whereas target's `addiu $s6,$zero,0xF`
 * sits BETWEEN `move $s1,0` (i = 0) and `move $s5,0` (fade = 0) -- source
 * statement order, i.e. the original declared and initialised a mask
 * variable there. `nib` reconstructs that variable; it is a consumed value,
 * not a codegen-control holder (see self_vet.md).
 */
s32 func_8006CCC8(s32 *arg0, s32 *arg1, s16 arg2) {
    s32 i;
    s16 lim;
    s32 shift;
    s32 nib;
    s32 fade;
    s32 j;
    u8 *rec;
    s32 ret;
    s32 masked;
    s16 field;

    ret = 0;
    if ((*(s32 *)((u8 *)D_800A34FC + 0x28) == 0x50005) && (*arg1 & 0x400040)) {
        func_8005C650(1, 0x7F, 0x7F);
        ret = 1;
    }

    i = 0;
    nib = 0xF;
    fade = 0;
    shift = 0;
    for (; i < 2; shift += 0x10, i++) {
        lim = ((arg2 >> i) & 1) ? 4 : 5;

        if (*arg1 & (0x1000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (((s16 *)((u8 *)D_800A34FC + 0x28))[i] <= 0) {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = lim;
            } else {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = (s16)(((s16 *)((u8 *)D_800A34FC + 0x28))[i] - 1);
            }
        } else if (*arg1 & (0x4000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (((s16 *)((u8 *)D_800A34FC + 0x28))[i] >= lim) {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = 0;
            } else {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = (s16)(((s16 *)((u8 *)D_800A34FC + 0x28))[i] + 1);
            }
        }

        field = ((s16 *)((u8 *)D_800A34FC + 0x28))[i];
        switch (field) {
        case 0:
        case 1:
        case 2:
            if (*arg1 & (0xF0 << shift)) {
                func_8005C650(0, 0x7F, 0x7F);
                func_8006CBD4(i, *arg1);
            }
            break;
        case 3:
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    masked = *(rec + 0x1A) & (nib << fade);
                    *(rec + 0x17) = (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + masked);
                }
            }
            break;
        case 4:
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    masked = *(rec + 0x1D) & (nib << fade);
                    *(rec + 0x17) = (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + masked);
                }
            }
            break;
        }
        fade += 4;
    }
    return ret;
}
