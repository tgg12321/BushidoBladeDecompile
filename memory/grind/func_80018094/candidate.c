/* func_80018094 candidate -- s9b (solver, 2026-09-09).  sandbox --disable all == 2
 * (153/153 insns, rules_dropped 0, 20 island insns stripped on both sides), measured this
 * session with this body spliced into src/code6cac.c.  Chassis: -mel -msoft-float.
 * Previous honest floor was 7; the s9a body that reached 0 did so with a TIED ASM OPERAND
 * that the Judge FAILed (docs/grind/decisions.md, 2026-09-09 23:11).  This body carries NO
 * asm-operand device: the island operand list is exactly the granted form
 * `: "=m"(sp_tmp[0]) : "r"(lut) : "$2","$12"`.
 *
 * WHAT CLOSED.  The whole residual was the target's pre-island `move $a0,$a1`
 * (asm/funcs/func_80018094.s, parked by reorg in the `beqz` delay slot).  An honest
 * `lut = sum_sq;` copy was deleted by cse at EVERY declaration scope for eight sessions.
 * The mechanism is cse_end_of_basic_block's extended-path walk: it follows the conditional
 * jump into the LZC block, so the copy and the asm land in ONE extended block, canon_reg
 * rewrites the asm operand back to sum_sq, and the copy dies.  cse only follows that jump
 * when the insn preceding the target label is a BARRIER (tools/gcc-2.7.2/cse.c:8112-8125);
 * the backward walk stops early on a NOTE_INSN_LOOP_END, and that stop has NO `after_loop`
 * guard, so it blocks cse1 AND cse2.  Spelling the small (sum_sq < 0x400) arm as an
 * `if` WITHOUT an else whose body is a `do { ...; goto lzc_done; } while (0);` puts the
 * loop-end note exactly between the barrier and the else-arm label -- the copy survives to
 * RA, becomes a global allocno, and reorg parks it in the delay slot (insn count is
 * unchanged: the slot was a nop before).
 *
 * WHAT IS LEFT (2 insns).  The small arm's LUT byte: target `lbu $v0,0($at)` / `srl $a1,$v0,3`,
 * ours `lbu $a0,0($at)` / `srl $a1,$a0,3`.  The byte is held in `lut` ONLY to give that
 * allocno enough references to be sorted ahead of `sum_sq` by global.c's priority sort --
 * without it (tmp/grind/func_80018094/s9/m1.c, p5.c, p1.c) the two allocnos, which have
 * IDENTICAL conflict sets and both carry `preferences: 4`, swap seats and the score is 13.
 * The next session's job is to make `sum_sq` lose that sort without parking the byte in
 * `lut` (see hypotheses.md s9b H49).
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
                /* FAKE: do{...}while(0) around the small arm's body, with the arm exited by
                 * `goto lzc_done` so this `if` has NO else, mechanism: cse.c
                 * cse_end_of_basic_block's follow-jumps gate -- expand_end_loop emits
                 * NOTE_INSN_LOOP_END after the `goto`'s BARRIER and before the if's false
                 * label, and the gate's backward walk (tools/gcc-2.7.2/cse.c:8112-8118)
                 * stops on a LOOP_END note, so cse1 AND cse2 refuse to extend the block into
                 * the LZC arm and the `lut = sum_sq;` island-input copy survives.
                 * lever-exhaustion: memory/grind/func_80018094/hypotheses.md s5 H26-H28,
                 * s6 H29-H32, s7 H31-H37, s8, s9 H42 (cse class kill), s9b H44-H46.
                 * Family: do-while-zero-exception (owner ruling 2026-07-06). */
                do {
                    lut = (u8)(*(&D_8008D118 + sum_sq));
                    sum_sq = lut >> 3;
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
