/* func_8002FDB0 — BEST FORM, session 1 (recon).
 *
 * Honest sandbox `--disable all` (cheat-asm stripped, 0 regfix/asmfix rules):
 * distance 0, build_insns 90 == target_insns 90.  Verified byte-exact against
 * asm/funcs/func_8002FDB0.s by objdump of tmp/sandbox/func_8002FDB0/code6cac_b.o.
 *
 * DISPOSITION IS NOT COMPLETED-C.  The GTE tail is reconstructed as three
 * canonical GTE inline-asm ISLANDS whose bodies are the verbatim PsyQ libgte
 * inline-macro expansions (gte_SetRotMatrix / gte_ldlvl / gte_stlvnl).  Those
 * SDK macro bodies hardcode $12-$15 and open with `move $12, %0`, so the
 * finished state is COMPLETED-INLINE-ASM-CANONICAL and needs an
 * inline_asm_canonical.txt entry — a surface the grind session may not touch.
 * See docs/grind/decisions.md (2026-08-11 entry) for the integration handoff.
 *
 * Precedent for the shape: func_800274BC (same file, inline_asm_canonical.txt:263,
 * owner-authorized 2026-06-10) is a canonical GTE island that likewise carries
 * the original island's GPR scaffolding (`addu $t4, %1, $zero`) inside a single
 * multi-instruction __asm__ block.
 *
 * Evidence the tail is SDK-macro-expanded and not compiler output:
 *   - three redundant `lui $a1,0x1F80; ori $a1,0x...; addu $t4,$a1,$zero`
 *     sequences: GCC never materializes a constant into one register and then
 *     copies it to a second for no consumer;
 *   - a fixed $12/$13/$14/$15 footprint repeated across all three islands;
 *   - two unfilled `nop`s (the GTE lwc2 load delay) that no scheduler emits;
 *   - the instruction sequences are byte-identical to the published PsyQ
 *     inline_c.h macro bodies for SetRotMatrix / ldlvl / stlvnl.
 *
 * The 63-instruction head (stride computation + six scratchpad subtract/store
 * blocks) is ordinary pure C and matched byte-exactly with no coercion of any
 * kind — including the `lui/ori` for the first island scheduling into the 6th
 * block's load-delay slot, which falls out naturally from the asm operand.
 *
 * The w1/w2 split (fresh locals for the sixth subtract block instead of reusing
 * v1/v2) is the last 3-instruction lever: with v1/v2 reused the sixth block's
 * second operand lands in $a0 and the difference in $v0; with distinct locals
 * the operand takes $v0 (freed when `stride` dies at its last index use) and the
 * difference stays in $v1, matching target.  Distance 3 -> 0.
 */
/* kengo:HIGH  |  nm_cpu/cpu_check_tubazeri  |  76i  |  x2 size collision */
s32 func_8002FDB0(s32 *arg0) {
    s32 stride;
    s32 v1, v2;
    s32 w1, w2;
    s32 ret;

    stride = (s32)((s16 *)arg0)[2] * 264;

    /* Compute (point_a - center) into scratchpad SCR[0x60..0x68] and
     * (point_b - center) into SCR[0x70..0x78].  Source vectors live at
     * stride-offset slots in scratchpad (0xB4/0xB8/0xBC = center xyz;
     * 0xC0/0xC4/0xC8 = a xyz; 0xCC/0xD0/0xD4 = b xyz). */
    v1 = *(s32 *)((u8 *)0x1F8000C0 + stride);
    v2 = *(s32 *)((u8 *)0x1F8000B4 + stride);
    *(s32 *)0x1F800360 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000C4 + stride);
    v2 = *(s32 *)((u8 *)0x1F8000B8 + stride);
    *(s32 *)0x1F800364 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000C8 + stride);
    v2 = *(s32 *)((u8 *)0x1F8000BC + stride);
    *(s32 *)0x1F800368 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000CC + stride);
    v2 = *(s32 *)((u8 *)0x1F8000B4 + stride);
    *(s32 *)0x1F800370 = v1 - v2;

    v1 = *(s32 *)((u8 *)0x1F8000D0 + stride);
    v2 = *(s32 *)((u8 *)0x1F8000B8 + stride);
    *(s32 *)0x1F800374 = v1 - v2;

    w1 = *(s32 *)((u8 *)0x1F8000D4 + stride);
    w2 = *(s32 *)((u8 *)0x1F8000BC + stride);
    *(s32 *)0x1F800378 = w1 - w2;

    /* PsyQ libgte inline macro gte_SetRotMatrix(r) — loads the 3 packed
     * rotation-matrix words at r into cop2 control regs R11R12/R13R21/R22R23.
     * The SDK macro body hardcodes $12-$15 and copies the operand into $12. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "lw     $15, 8($12)\n"
        "ctc2   $14, $2\n"
        "ctc2   $15, $4\n"
        :: "r"((s32 *)0x1F800360) : "$12", "$13", "$14", "$15");
    /* PsyQ libgte inline macro gte_ldlvl(r) — load long vector at r into
     * IR1/IR2/IR3 ($9/$10/$11), IR3 first, then the 2-cycle GTE load delay. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $11, 8($12)\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "nop\n"
        "nop\n"
        :: "r"((s32 *)0x1F800370) : "$12");
    /* GTE OP (outer/cross product of the IR vector with the rotation matrix
     * diagonal), sf=0 — cop2 command 0x0170000C. */
    __asm__ volatile(".word 0x4B70000C");
    /* PsyQ libgte inline macro gte_stlvnl(r) — store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"((s32 *)0x1F800380) : "$12");
    /* Read MAC2 from scratchpad and return slt(0, MAC2) — i.e. MAC2 > 0. */
    ret = *(s32 *)0x1F800384;
    return 0 < ret;
}
