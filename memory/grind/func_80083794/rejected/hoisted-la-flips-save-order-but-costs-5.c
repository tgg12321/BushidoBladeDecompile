/* REJECTED — session 3 (structural, 2026-08-13).
 *
 * WHY IT IS DEAD: it is the ONE form measured that reproduces the target's
 * ascending prologue callee-save order (residual class D), and it costs 5
 * points to do so. Sandbox `--disable all`: score 23 (vs the 18 floor).
 *
 * What it produces (objdump of tmp/sandbox/func_80083794/ings2.o):
 *     lui   v0,0x0            ; flag hi
 *     lw    v0,0(v0)          ; flag load
 *     addiu sp,sp,-32
 *     sw    s0,16(sp)         <-- ASCENDING, as in target
 *     lui   s0,0x0
 *     addiu s0,s0,0
 *     sw    s1,20(sp)         <-- ASCENDING, as in target
 *     lui   s1,0x0
 *     addiu s1,s1,0
 *     bnez  v0,...
 *     sw    ra,24(sp)         <-- ASCENDING, as in target
 *
 * MECHANISM (confirmed, and it is the same one the corpus uses): cc1's
 * `save_restore_insns` (mips.c:4680) emits the saves descending
 * (GP_REG_LAST -> GP_REG_FIRST) in BOTH the prologue and the epilogue, from a
 * single loop. The only thing that can produce an ascending prologue is the
 * post-reload scheduler reordering the stores, and the only pressure that makes
 * it do so is an in-block ANTI-DEPENDENCE: a body instruction in the SAME basic
 * block that WRITES $sN must be scheduled after `sw $sN`, so the scheduler
 * hoists that store to unblock it. Corpus corroboration (session-3 scans): of
 * the 38 contiguous ascending save runs in the 1437 shipped functions, the 21
 * that are matched pure-C all have exactly this shape — e.g. func_80069A30 /
 * player_Destroy / func_8004046C emit `sw $s0,0x10($sp); sw $ra,0x14($sp); jal
 * ...; addu $s0,$a0,$zero` with the $s0-write in the jal delay slot.
 *
 * WHY IT CANNOT BE COMBINED WITH THE FLOOR (the coupled constraint): to create
 * that anti-dependence, the writes to $s0/$s1 must be in the ENTRY block, and
 * for this function the writes to $s0/$s1 ARE the two `la` pairs. Target emits
 * both `la` pairs AFTER the `bnez`, in the post-branch block, where they exert
 * no scheduling pressure on the prologue. Target therefore has ascending
 * prologue saves AND post-branch `la`s simultaneously; cc1 + this scheduler
 * gives you one or the other. Hoisting the `la`s buys class D (~3 insns) and
 * loses more than that on the `la` placement, netting 18 -> 23.
 *
 * COROLLARY BANKED: func_80083794 is the ONLY function in the executable with a
 * contiguous ascending save run of length 3 (`tmp/grind/func_80083794/s3/
 * contig.txt`; the second listing is its own unlabelled twin body at
 * 0x80083804). Every other contiguous ascending run in the corpus is length 2.
 */
extern s32 D_800A2668;
extern void (*D_8008D070)(void);
extern s32 D_00000000;

void func_80083794(void) {
    s32 count;
    void (**p)(void);

    count = (s32)&D_00000000;
    p = &D_8008D070;
    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        while (count != 0) {
            (*p++)();
            count--;
        }
    }
}
