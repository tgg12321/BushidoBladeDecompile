/* func_80018094 candidate -- s9 (solver, 2026-09-09).  sandbox --disable all == 0
 * (153/153 insns, rules_dropped 0, 20 island insns stripped on both sides), measured this
 * session with this body spliced into src/code6cac.c.  Chassis: -mel -msoft-float.
 *
 * LINEAGE: s4 candidate (scratchpad-in-struct + else-arm scale=0x100 + s32 sp_tmp[4] oversized
 * locals) + s5's do{...}while(0) wrap and TIED island output, then this session's four steps:
 *   b4  7 -> 5  the tied "=r" output is taken by an EXISTING allocno instead of a fresh local,
 *               which turns the reload copy from a block-local qty ($v1) into a global allocno
 *               that carries `preferences: 4` and is allocated first -> the copy lands on $a0,
 *               the target's seat, and sum_sq stays at $a1.
 *   d1  5 -> 5  the tied output is moved off log2_val onto a NEW named local `lut` that really
 *               holds the LUT byte later in the arm (byte-neutral; splits the copy away from
 *               log2_val without cost).
 *   e1  5 -> 3  the s4 `log2_val` staging is DROPPED (`shift_a = 0x16 - li_v0;`): with the copy
 *               already seated it is no longer load-bearing, and removing it frees log2_val's
 *               allocno from its conflict with sum_sq.  One FAKE construct retired.
 *   f1  3 -> 0  log2_val and sum_sq are spelled as ONE variable -- which is what the target's own
 *               registers say ($a1 carries sum_sq, then the arm result, then feeds the
 *               (x<<6)/500 + 0xC0 tail).  This is the last 3 insns.
 *
 * ABLATIONS MEASURED THIS SESSION (all three remaining devices are load-bearing at 0):
 *   drop the tied "=r"(lut)/"1"(sum_sq) operand pair -> 10   (tmp/.../s9/g1.c)
 *   s32 sp_tmp[4] -> s32 sp_tmp                      ->  8   (tmp/.../s9/g3.c)
 *   drop the do{...}while(0) wrap                    -> 13   (tmp/.../s9/g4.c)
 *   honest `lz_in = sum_sq;` copy instead of the tied operand, at three declaration scopes
 *                                                    -> 10 x3 (h1/h2/h3) -- cse.c:8102 still
 *   deletes it, re-confirming the s6 class kill on this new chassis.
 *
 * POLICY STATUS -- NOT submitted as candidate-ready.  The tied asm operand pair
 * (`"=r"(lut)` + `"1"(sum_sq)`) declares an output the island template never writes; its only
 * effect is to make reload materialise the target's `move $a0,$a1`.  No frozen SOTN family
 * covers an inline-asm OPERAND device (docs/reference/sotn-construct-index.md has no
 * inline-asm-operand class; s8 recorded the same negative), so s9 files a ruling-request
 * instead of a submission.  If the ruling goes against the operand, everything from b4 to f1
 * still stands: the residual is then exactly ONE insn-family (the pre-island copy) and the
 * other four seats are closed for free.
 */
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
    u32 lut;

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
            do {
            lut = sum_sq;
            if (sum_sq < 0x400) {
                do {
                    sum_sq = (u8)(*(&D_8008D118 + lut)) >> 3;
                    goto lzc_done;
                } while (0);
            }
            {
                s32 shift_a, shift_b;
                __asm__ volatile(
                    "addu   $t4, %1, $zero\n"
                    "mtc2   $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addiu  $v0, $sp, 0x10\n"
                    "addu   $t4, $v0, $zero\n"
                    "swc2   $31, 0($t4)\n"
                    : "=m"(sp_tmp[0])
                    : "r"(lut)
                    : "$2", "$12");
                {
                    s32 lw_v1 = sp_tmp[0];
                    s32 li_v0 = -2;
                    li_v0 = lw_v1 & li_v0;
                    shift_a = 0x16 - li_v0;
                }
                shift_b = shift_a >> 1;
                lut = (u8)(*(&D_8008D118 + (sum_sq >> shift_a)));
                sum_sq = ((s32)(lut << 16)) >> (0x13 - shift_b);
            }
        lzc_done:
            scale = ((sum_sq << 6) / 500) + 0xC0;
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
