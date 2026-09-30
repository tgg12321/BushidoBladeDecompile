/* func_8002C22C landed body (8a3843d5e, src/code6cac_b.c; since the split src/code6cac_b_tu2.c),
 * banked verbatim when it went back to INCLUDE_ASM per owner Q37 (retro-audit 2026-09-29 FAIL:
 * `d_tbl = &D_80102314` alias indexed up to +0x248 past a scalar symbol). Sandbox 0 (252/252). */
/* Accumulates a character's shadow vectors in PSX scratchpad RAM.  Everything
 * this function writes lives in the one record at 0x1F8002B8 that `scr` points
 * at: two 3-word vectors at +0xA8 and +0xB8 (the +0xB4 and +0xC4 words are not
 * touched, so both are 16-byte-strided like a PsyQ VECTOR), and the 3-word
 * result at +0x13C.  func_8002CA8C and func_8002EBDC in this file already
 * address that record the same way.
 *
 * Two details are load-bearing and are worth stating because the obvious
 * alternatives measure worse:
 *
 * 1. `scr` is a `u8 *` with displaced `*(s32 *)(scr + off)` accesses rather than
 *    an `s32 *` with `scr[off/4]`.  The array spelling sets MEM_IN_STRUCT_P, and
 *    true_dependence (tools/gcc-2.7.2/sched.c:817) then treats an in-struct
 *    varying-address store as non-aliasing with a plain constant-address load,
 *    which lets the scheduler hoist the scratchpad source loads in the second
 *    block ahead of the preceding store.  The target keeps that dependence.
 *
 * 2. The first block's accesses come out absolute (`lui $at; sw $x,off($at)`)
 *    while the second block's identical spelling comes out as register+
 *    displacement (`sw $x,off($scr)`) - what the target does in both places.
 *    cse decides that, not the source: find_best_addr
 *    (tools/gcc-2.7.2/cse.c:2622) folds a `(plus reg const)` address to an
 *    absolute one when that register's constant value is in cse's table, and
 *    cse clears the table at the head of every extended-basic-block path it
 *    processes (new_basic_block, cse.c:766).  scr is set to its constant at
 *    the top of the function (insn 8) and no path reaching the second if/else
 *    begins before insn 240, so the first block's references fold and the
 *    second block's do not.  Counted in the .cse dump: 30 folded references in
 *    the first block - 6 pre-branch zero-stores plus 12 in each arm, none at
 *    the join, the shared tail still being duplicated in both arms because
 *    cross-jumping is a later pass - against 57 left as `(plus scr off)` from
 *    insn 251 on.
 *
 * Both if/else blocks are uniform per-arm statement runs.  The stores that
 * appear after each join in the target are jump.c cross-jumping merging the
 * arms' instruction-identical tails, not source-level statements.
 */
/* Record 1 of a 2-element table, stride 0x44C from D_80101EC8 (= func_8002C61C's
 * s1 + 0x44C); fields are reached base+offset here rather than through the
 * individually-named scalars record 0 uses. */
extern s32 D_80102314;
void func_8002C22C(void) {
    u8 *scr = (u8 *)0x1F8002B8;
    s32 *d_tbl = &D_80102314;

    *(s32 *)(scr + 0xA8) = 0;
    *(s32 *)(scr + 0xAC) = 0;
    *(s32 *)(scr + 0xB0) = 0;
    *(s32 *)(scr + 0xB8) = 0;
    *(s32 *)(scr + 0xBC) = 0;
    *(s32 *)(scr + 0xC0) = 0;

    if (D_800A3824 & 1) {
        *(s32 *)(scr + 0xA8) = *(s32 *)0x1F800048;
        *(s32 *)(scr + 0xAC) = *(s32 *)0x1F80004C;
        *(s32 *)(scr + 0xB0) = *(s32 *)0x1F800050;
        *(s32 *)(scr + 0xA8) += *(s32 *)0x1F800054;
        *(s32 *)(scr + 0xAC) += *(s32 *)0x1F800058;
        *(s32 *)(scr + 0xB0) += *(s32 *)0x1F80005C;
        *(s32 *)(scr + 0xB8) = D_801020FC;
        *(s32 *)(scr + 0xBC) = D_80102100;
        *(s32 *)(scr + 0xC0) = D_80102104;
        *(s32 *)(scr + 0xB8) += D_80102108;
        *(s32 *)(scr + 0xBC) += D_8010210C;
        *(s32 *)(scr + 0xC0) += D_80102110;
    } else {
        *(s32 *)(scr + 0xA8) = *(s32 *)0x1F800000;
        *(s32 *)(scr + 0xAC) = *(s32 *)0x1F800004;
        *(s32 *)(scr + 0xB0) = *(s32 *)0x1F800008;
        *(s32 *)(scr + 0xA8) += *(s32 *)0x1F80000C;
        *(s32 *)(scr + 0xAC) += *(s32 *)0x1F800010;
        *(s32 *)(scr + 0xB0) += *(s32 *)0x1F800014;
        *(s32 *)(scr + 0xB8) = D_801020D8;
        *(s32 *)(scr + 0xBC) = D_801020DC;
        *(s32 *)(scr + 0xC0) = D_801020E0;
        *(s32 *)(scr + 0xB8) += D_801020E4;
        *(s32 *)(scr + 0xBC) += D_801020E8;
        *(s32 *)(scr + 0xC0) += D_801020EC;
    }
    if (D_800A3824 & 2) {
        *(s32 *)(scr + 0xA8) += *(s32 *)0x1F800060;
        *(s32 *)(scr + 0xAC) += *(s32 *)0x1F800064;
        *(s32 *)(scr + 0xB0) += *(s32 *)0x1F800068;
        *(s32 *)(scr + 0xB8) += d_tbl[0x234/4];
        *(s32 *)(scr + 0xBC) += d_tbl[0x238/4];
        *(s32 *)(scr + 0xC0) += d_tbl[0x23C/4];
        *(s32 *)(scr + 0xA8) += *(s32 *)0x1F80006C;
        *(s32 *)(scr + 0xAC) += *(s32 *)0x1F800070;
        *(s32 *)(scr + 0xB0) += *(s32 *)0x1F800074;
        *(s32 *)(scr + 0xB8) += d_tbl[0x240/4];
        *(s32 *)(scr + 0xBC) += d_tbl[0x244/4];
        *(s32 *)(scr + 0xC0) += d_tbl[0x248/4];
    } else {
        *(s32 *)(scr + 0xA8) += *(s32 *)0x1F800024;
        *(s32 *)(scr + 0xAC) += *(s32 *)0x1F800028;
        *(s32 *)(scr + 0xB0) += *(s32 *)0x1F80002C;
        *(s32 *)(scr + 0xB8) += d_tbl[0x210/4];
        *(s32 *)(scr + 0xBC) += d_tbl[0x214/4];
        *(s32 *)(scr + 0xC0) += d_tbl[0x218/4];
        *(s32 *)(scr + 0xA8) += *(s32 *)0x1F800030;
        *(s32 *)(scr + 0xAC) += *(s32 *)0x1F800034;
        *(s32 *)(scr + 0xB0) += *(s32 *)0x1F800038;
        *(s32 *)(scr + 0xB8) += d_tbl[0x21C/4];
        *(s32 *)(scr + 0xBC) += d_tbl[0x220/4];
        *(s32 *)(scr + 0xC0) += d_tbl[0x224/4];
    }
    *(s32 *)(scr + 0x13C) = ((*(s32 *)(scr + 0xA8) * 3) + *(s32 *)(scr + 0xB8)) >> 4;
    *(s32 *)(scr + 0x140) = ((*(s32 *)(scr + 0xAC) * 3) + *(s32 *)(scr + 0xBC)) >> 4;
    *(s32 *)(scr + 0x144) = ((*(s32 *)(scr + 0xB0) * 3) + *(s32 *)(scr + 0xC0)) >> 4;
}
