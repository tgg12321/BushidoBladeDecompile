/* func_80018094 -- s5 (permuter, 2026-09-09). sandbox --disable all == 7, i.e. EQUAL to the banked
 * floor, not better -- kept because it is a STRUCTURALLY DIFFERENT chassis that the next session
 * should seed from. Built on candidate.c (s32 sp_tmp[4] + the log2_val staging) by adding
 *   (a) a TIED asm output on the LZC island (`: "=m"(sp_tmp[0]), "=r"(lz_in) : "1"(sum_sq)`), which
 *       forces GCC to materialise the island-input COPY that candidate.c is missing, and
 *   (b) a do{...}while(0) wrap around the inner if-chain + the `scale = ...` statement (the s5
 *       permuter campaign s5-w1-tiedcopy find output-35-1, permuter score 55 -> 35), which recovers
 *       the 3 insns the tied operand costs on its own (w1 = 10, w6 = 7).
 * WHY IT IS NOT AN IMPROVEMENT: the copy is emitted, in the right place (reorg parks it in the
 * `beqz v0` delay slot exactly as the target does) and sum_sq is correctly seated in $a1 -- but the
 * copy takes $v1 where the target takes $a0, so the same 7 insns differ, only re-spelled:
 *   ours 69 `move v1,a1` / 75 `move t4,v1`   vs target 69 `move a0,a1` / 75 `move t4,a0`
 *   ours 74 `srl a0` 82/84 li_v0 in $a0 93/96 log2_val in $a0  vs target $a1 / $v0 / $a1.
 * MEASURED CAUSE (tools/ra_solver, this session): the copy is a BLOCK-LOCAL quantity of the else
 * block (code6cac.local.json func_80018094 blk 6 qty 0: birth 4, death 5, refs 2, got 3), and
 * local_alloc's find_free_reg scans ascending over `fixed_reg_set | union(regs_live_at[4..5])`;
 * $2 is in that set (the island clobbers it) and NOTHING else is, so $3 is the first free register.
 * No local qty of block 6 overlaps [4,5] (next birth is 6), so no reordering of the block's own
 * quantities can occupy $3 there. In the TARGET the same copy must therefore be a cross-block
 * GLOBAL allocno (defined in the pre-branch block, used at the island), which is the one thing this
 * spelling cannot produce: the tied operand always emits its copy immediately before the asm.
 * tools/ra_solver/inverse.py local --block 6 --goal '{"0": 4}' returns exactly one minimal vector,
 * [live_extend] qty 0 dies later (5 -> 9); the only honest spelling of it tried here (w8: feed the
 * post-island LUT index from lz_in instead of sum_sq) extends the death to ~18 instead and CSE
 * folds the copy away again, sandbox 12. */
typedef struct { s32 pad[9]; s32 x, y, z; } ScrV;
#define SCRV ((ScrV *)0x1F800000)
void func_80018094(s32 *arg0, s32 *arg1) {
    /* n.b.! sp_tmp must be 9-16 bytes (inclusive): s32[3] and s32[4] are byte-identical (measured,
     * tmp/grind/func_80018094/s4/v20g.s == v20h.s). Frame derivation from the target bytes alone
     * (asm/funcs/func_80018094.s): frame 0x30 = outgoing args 0x10 + locals 0x10 + callee-saves 0x10
     * (s0/s1/ra at 0x20/0x24/0x28); the ONLY locals traffic in the whole target is the island's
     * `swc2 $31,0($t4)` with $t4 = $sp+0x10 and the matching `lw $v1,0x10($sp)`, i.e. 4 bytes written
     * of a 16-byte locals region. A fully-written 4-byte locals set yields ALIGN8(4)+16+16 = 0x28 != 0x30,
     * so the original declared this object strictly larger than the bytes it writes. OVERSIZED-LOCALS
     * carve-out (.claude/rules/dead-vars-local-array.md, owner ruling 2026-07-13), prerequisite 2
     * (extend the LIVE locals object, not a dead pad): sp_tmp[0] is the live LZC output.
     * FAKE: unwritten tail sp_tmp[1..3] on the live LZC-output locals object, mechanism:
     * function.c assign_stack_local / mips.c compute_frame_size (get_frame_size raw 16 -> MIPS_STACK_ALIGN
     * keeps 16 where the scalar form rounds 4 -> 8), lever-exhaustion:
     * memory/grind/func_80018094/hypotheses.md s2 H15 (declaration scope/order/hoisting), s3 H19-H22
     * (HImode narrowing, named-intermediate scalar splits, live 8-byte aggregate, BLKmode-only FRAMEDBG
     * census) and s4 (8,906 permuter iterations on the scalar chassis, 0 novel finds). */
    s32 sp_tmp[4];
    s32 dx, dy, dz;
    s32 sum_sq;
    s32 scale;
    s32 *dst;

    /* PsyQ libgte inline macro gte_SetRotMatrix(r0) --- inline_c.h:297-310 (same spelling
     * as func_80019310 / func_800300B4; "memory" clobber ADDED per src/code6cac_b.c:935). */
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
        :: "r"(arg0[1]) : "$12", "$13", "$14", "$15", "memory");
    /* PsyQ libgte inline macro gte_SetTransMatrix(r0) --- inline_c.h:360-369. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 20($12)\n"
        "lw     $14, 24($12)\n"
        "ctc2   $13, $5\n"
        "lw     $15, 28($12)\n"
        "ctc2   $14, $6\n"
        "ctc2   $15, $7\n"
        :: "r"(arg0[1]) : "$12", "$13", "$14", "$15", "memory");

    func_80017FA0(arg0);

    dx = ((s32 *)arg0[1])[5] - arg1[10];
    SCRV->x = dx;
    dy = ((s32 *)arg0[1])[6] - arg1[11];
    SCRV->y = dy;
    dz = ((s32 *)arg0[1])[7] - arg1[12];
    SCRV->z = dz;

    sum_sq = (dx * dx) + (dy * dy) + (dz * dz);

    if (sum_sq > 250000) {
        scale = 0x100;
    } else if (sum_sq < 0) {
        scale = 0;
    } else {
        {
            s32 log2_val;
            do {
            if (sum_sq < 0x400) {
                log2_val = (u8)(*(&D_8008D118 + sum_sq)) >> 3;
            } else {
                s32 shift_a, shift_b;
                s32 lz_in;
                /* GTE LZCS/LZCR leading-zero-count island --- the authorized func_8001A67C
                 * template (inline_asm_canonical.txt:266), sp_tmp at 0x10($sp). */
                __asm__ volatile(
                    "addu   $t4, %1, $zero\n"
                    "mtc2   $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addiu  $v0, $sp, 0x10\n"
                    "addu   $t4, $v0, $zero\n"
                    "swc2   $31, 0($t4)\n"
                    : "=m"(sp_tmp[0]), "=r"(lz_in)
                    : "1"(sum_sq)
                    : "$2", "$12");
                {
                    s32 lw_v1 = sp_tmp[0];
                    lz_in = -2;
                    lz_in = lw_v1 & lz_in;
                    log2_val = lz_in;
                    shift_a = 0x16 - log2_val;
                }
                shift_b = shift_a >> 1;
                log2_val = ((u8)(*(&D_8008D118 + (sum_sq >> shift_a))) << 16) >> (0x13 - shift_b);
            }
            scale = ((log2_val << 6) / 500) + 0xC0;
            } while (0);
        }
    }

    SCRV->x = (SCRV->x * scale) >> 1;
    SCRV->y = (SCRV->y * scale) >> 1;
    SCRV->z = (SCRV->z * scale) >> 1;

    dst = arg1;
    *(MATRIX *)(dst + 5) = *(MATRIX *)arg0[1];
    func_80018300(dst);
}
