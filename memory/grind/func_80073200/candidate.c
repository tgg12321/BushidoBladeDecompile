/* func_80073200 - session 8 (synthesis). FLOOR 16 -> 2 (build_insns 203 ==
 * target_insns 203).  This body supersedes the s4-s7 floor-16 candidate.
 *
 * Three independent levers, found by merging partial results the ledger had
 * already banked as separate (instance-scoped) kills.  Each was measured on
 * top of the previous one:
 *
 * 1. 16 -> 10  "cond" swap.  The s4 permuter find hoisted the D_800A35C4+8
 *    address into `new_var` BEFORE the `if (D_800A3580 < 2)`; s5 killed the
 *    complementary move (pushing that address computation back INSIDE the
 *    if-block) because it left the early slot empty.  Reading
 *    asm/funcs/func_80073200.s:151-157 shows the target fills that slot with
 *    the *D_800A3580 read*, not the address: `lw v0,0x18(s0); lh
 *    v1,%gp_rel(D_800A3580); addiu v0,v0,0xC; slti v1,v1,2; beqz v1,...` with
 *    `lw v1,%gp_rel(D_800A35C4)` only AFTER the branch.  So both halves are
 *    needed at once: replace `new_var = (s32)D_800A35C4 + 8;` with
 *    `cond = D_800A3580;` at the SAME statement position, and put the
 *    dereference back inline inside the if.  Closes the whole hunk-12/13/14
 *    cluster (target `lh` + `slti` + `beqz` vs our `lw` + load-delay nop).
 *
 * 2. 10 -> 8   u8 color/code bytes.  S73200's sp40..sp43 are the PsyQ
 *    primitive's code/r/g/b bytes.  Declared `s8`, GCC narrows the source
 *    constants and emits `li v0,-68` / `li v0,-88`; the target emits
 *    `li v0,188` / `li v0,168` (asm lines 50 and 55), i.e. the fields are
 *    UNSIGNED char.  This is a data-model correction, not a codegen trick.
 *
 * 3. 8 -> 2    v12-at-the-join + s1 reuse, TOGETHER.  These two had each been
 *    killed alone.  Measured this session, separately, on the floor-8 chassis:
 *      - v12 assigned at the join instead of a top-of-body initializer (vL):
 *        the `li ...,0x12` moves to the correct target[114] slot, but the
 *        pseudo takes s1 and the function drops to 3 callee-saved regs
 *        (88-byte frame vs target's 96) - score 15.
 *      - reusing the existing `s1` local for the two later `+0xC` table
 *        pointers, with v12 still a top-of-body initializer (vJE): score 9,
 *        build_insns 205.
 *    Together they are exact: forcing the table-pointer pseudo onto s1 makes
 *    it CONFLICT with v12's join-to-end live range, so global.c is forced to
 *    give v12 the 4th callee-saved register s3 - reproducing the target's
 *    s0/s1/s2/s3 + 96-byte frame AND its single `addiu s3,zero,0x12` at the
 *    join label .L800733C8.  All four operand-only register-seat hunks and
 *    both `li s3,18` placement hunks close at once.
 *
 * REMAINING RESIDUAL (score 2, ONE hunk pair, source-level):
 *    ours[55] `addiu a0,sp,24` inserted / target[58] `addiu a0,sp,24` deleted.
 *    The `(s32)&s` argument materialization for the FIRST of the four
 *    func_80073728 calls is scheduled to the very head of the join basic
 *    block; the target emits it 4th, after `sb v0,0x42(sp); li v0,0x14;
 *    sb v0,0x43(sp)` (asm/funcs/func_80073200.s:59-62).  The other three call
 *    groups already match byte-for-byte.  Everything else in the function -
 *    every register, every other instruction - is identical to the target.
 *    Pass attribution IS done: tmp/grind/func_80073200/dumps/text1b.sched,
 *    basic block 4 (insns 134..267, all four call groups in ONE block).  The
 *    a0 set is insn 158, INSN_PRIORITY 1 - tied with the seven stores/loads
 *    137/140/142/145/147/150/153 - and it sorts to the HEAD of the ready list
 *    every step (highest LUID wins the rank_for_schedule tie), but the
 *    "insn N has a greater potential hazard" swap in schedule_block promotes a
 *    store over it at T-47, T-48, T-50 and T-52, so it is left as the very
 *    last pick of the backward pass = the FIRST insn emitted in the block.
 *    The target's schedule picks it at the T-50 step instead.
 *
 * Measured DEAD on this chassis this session (do not re-propose):
 *    all 23 non-identity orderings of the 4 statements after `s.sp42` in the
 *    first call group (best 4, none < 2); moving `s.sp42 = var_v0;` out of
 *    first position (all 5); naming `(s32)&s` as an `addr` local used by all
 *    four or by only the first call (52, build_insns 204); var_v0 typed
 *    s8/s16/s32/u32 (all 2, inert); spelling the argument `(s32)&s.sp18`
 *    (2, inert); duplicating the sp42 store into both arms instead of staging
 *    it through var_v0 (2, inert - the s2 H2 re-audit); declaring v12 or s1
 *    first among the locals (2, inert).
 */
typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    u8 sp40, sp41, sp42, sp43;
} S73200;
void func_80073200(s32 arg0) {
    S73200 s;
    s32 *ctx;
    s32 base1;
    s32 base2;
    s32 s1;
    s32 tmp;
    s32 v1;
    s32 idx;
    u8 var_v0;
    s32 v12;
    s32 cond;

    s.sp30 = 0;
    s.sp34 = 0;
    s.sp38 = 0x100;
    s.sp3C = 0x100;
    ctx = *(s32 **)(D_800A35A8 + 0x5C);
    base1 = *(s32 *)((s32)ctx + 0xC);
    s.sp18 = base1;
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(base1, 0), 0);
    AddPrim(D_800A374C + 0x70, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    s.sp40 = 1;
    base2 = *(s32 *)((s32)ctx + 0x10);
    s.sp18 = base2;
    s1 = base2 + 0xC;
    if (D_800A3580 < 4) {
        s.sp28 = 1;
        if (*(s32 *)((s32)D_800A35C4 + 8) & 4) {
            s.sp41 = 0xBC;
            var_v0 = 0x78;
        } else {
            s.sp41 = 0xA8;
            var_v0 = 0x6E;
        }
        s.sp42 = var_v0;
        s.sp43 = 0x14;
        s.sp2C = 0x14;
        s.sp1C = s1;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 0);
        s.sp1C = s1 + 8;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 1);
        s.sp1C = s1 + 0x10;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 2);
        s.sp1C = s1 + 0x18;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 3);
        SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, 0x60, 0);
        AddPrim(D_800A374C + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
        *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    } else {
        s.sp41 = 0x32;
        s.sp42 = 0x32;
        s.sp43 = 0x5A;
    }
    v12 = 0x12;
    s.sp34 = 0;
    s.sp30 = 0;
    s.sp2C = v12;
    s.sp28 = 0;
    tmp = *(s32 *)((s32)ctx + 0x14);
    s.sp18 = tmp;
    s1 = tmp + 0xC;
    s.sp1C = s1;
    s.sp20 = *(s32 *)(arg0 + 0x10);
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&s.sp18);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(s.sp18, 0x60), 0);
    AddPrim(D_800A374C + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
    cond = D_800A3580;
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    if (cond < 2) {
        s.sp2C = v12;
        s.sp28 = 1;
        v1 = *(s32 *)((s32)D_800A35C4 + 8);
        idx = *(s32 *)((s32)ctx + 0x28 + (v1 % 4) * 4);
        s.sp18 = idx;
        s1 = idx + 0xC;
        s.sp1C = s1;
        s.sp20 = *(s32 *)(arg0 + 0x10);
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&s.sp18);
        SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(s.sp18, 0x20), 0);
        AddPrim(D_800A374C + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
        *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    }
}
