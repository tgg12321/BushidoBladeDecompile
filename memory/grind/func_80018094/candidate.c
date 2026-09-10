/* func_80018094 -- MATCHED IN PURE C, s10 (forensics, 2026-09-09).
 * `sandbox func_80018094 --disable all` = 0 (target_insns 153, build_insns 153,
 * rules_dropped 0, cheat_asm_stripped 20 -- the two PsyQ gte_Set*Matrix islands and the
 * LZC island, stripped on BOTH sides).  Chassis: -mel -msoft-float.
 * NO asm-operand device: the LZC island's operand list is exactly the granted form
 * `: "=m"(sp_tmp[0]) : "r"(lut) : "$2", "$12"` (the Judge's binding constraint from the
 * 2026-09-09 23:11 ruling in docs/grind/decisions.md).
 *
 * THE TWO RESIDUALS THAT CLOSED, AND WHY.
 * (1) s9b closed the pre-island `move $a0,$a1` (target parks it in the `beqz` delay slot):
 *     an honest `lut = sum_sq;` copy survives cse only if a NOTE_INSN_LOOP_END sits between
 *     the small arm's terminating BARRIER and the LZC arm's label, which is what an `if`
 *     with NO else whose body is `do { ...; goto lzc_done; } while (0);` emits
 *     (tools/gcc-2.7.2/cse.c:8100-8125, the follow-jumps gate's backward walk).
 * (2) s10 closes the last two insns (the small arm's LUT byte: target `lbu $v0,0($at)`,
 *     ours `lbu $a0,0($at)`).  `lut` and `sum_sq` have IDENTICAL conflict sets and both
 *     prefer $4, so global.c's allocno_compare priority sort alone decides which takes $a0.
 *     s9b bought the sort by parking the small arm's byte in `lut` (n_refs 8 -> 14) -- which
 *     is exactly what put that byte in $a0 and cost the last two insns.  s10 buys the same
 *     sort from the DENOMINATOR/weight side instead: a third do-while(0) wrap around the LZC
 *     arm raises that region's flow.c loop depth, so the SAME references count for more
 *     (n_refs 8 -> 11 at unchanged live_length 7, pri 34285 -> 47142 versus sum_sq's 40000),
 *     and the small arm's byte stays in its own short-lived pseudo at $v0.
 *     Measured: tmp/grind/func_80018094/s10/e1.allocdbg.txt (BB2_ALLOC_DEBUG on the
 *     instrumented cc1) -- the priorities were PREDICTED from the m1/candidate arrays before
 *     the form was written and came out exact.
 *
 * EVERY CONSTRUCT IS ABLATION-MEASURED THIS SESSION (all still 153 build insns):
 *   drop the outer do-while(0)        -> 13  (s10/f1.c)
 *   drop the small-arm do-while(0)    -> 10  (s10/f3.c)
 *   drop the LZC-arm do-while(0)      -> 13  (s10/a0.c)
 *   s32 sp_tmp scalar instead of [4]  ->  8  (s10/f2.c)
 * Self-vet: memory/grind/func_80018094/self_vet.md.
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
            /* FAKE: do{...}while(0) around the whole else-arm body, mechanism: flow.c
             * life_analysis / basic_block_loop_depth (tools/gcc-2.7.2/flow.c:440-471) --
             * NOTE_INSN_LOOP_BEG/END raise the block's loop depth, and every reference in
             * the region is then weighted by that depth in `reg_n_refs[regno] += loop_depth`
             * (flow.c:2081), which is the numerator of global.c's allocno_compare priority.
             * Ablation: dropping this wrap scores 13 (tmp/grind/func_80018094/s10/f1.c).
             * lever-exhaustion: memory/grind/func_80018094/hypotheses.md s5 H26-H28,
             * s6 H29-H32, s7 H31-H37, s8, s9 H42-H49.
             * Family: do-while-zero-exception (owner ruling 2026-07-06). */
            do {
            /* FAKE: the LZC island's input operand staged through `lut`, the local the LZC arm
             * already owns for its LUT byte, mechanism: cse.c cse_end_of_basic_block's
             * follow-jumps gate (see the small arm's note) lets this copy survive to RA, where
             * global.c find_reg seats it at $a0 and reorg.c fills the `beqz` delay slot with it
             * -- reproducing the target's pre-island `move $a0,$a1` with no asm-operand device.
             * Liveness (bound 3): `lut` holds nothing at this point (its LZC-arm write comes
             * later), and the staged value is consumed by the island BEFORE that write, so the
             * borrow is safe in both directions.  lever-exhaustion:
             * memory/grind/func_80018094/hypotheses.md s5 H26-H28, s6 H29-H32, s7 H31-H37, s8,
             * s9 H42-H46 (every fresh-local and every declaration-scope spelling measured).
             * Family: staged-value-reused-variable (owner ruling 2026-07-03). */
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
                    sum_sq = (u8)(*(&D_8008D118 + sum_sq)) >> 3;
                    goto lzc_done;
                } while (0);
            }
            {
                s32 shift_a, shift_b;
                /* FAKE: third do{...}while(0), around the LZC arm's body, mechanism: the same
                 * flow.c loop-depth weighting -- it lifts `lut`'s three in-arm references
                 * (the island operand, the LUT byte set, the <<16 use) from weight 2 to
                 * weight 3, so allocno_n_refs[lut] goes 8 -> 11 while sum_sq's goes 19 -> 21,
                 * and global.c's allocno_compare priority
                 * (floor_log2(n_refs)*n_refs/live_length*10000) becomes 47142 for `lut` versus
                 * 40000 for `sum_sq` -- measured, tmp/grind/func_80018094/s10/e1.allocdbg.txt.
                 * `lut` is then allocated FIRST and takes $a0, sum_sq $a1, exactly as the
                 * target seats them, and the small arm's LUT byte is free to stay in its own
                 * short-lived pseudo at $v0.
                 * SINGLE LEVEL IS INSUFFICIENT (nested-wrap prerequisite, measured this
                 * session): with only the outer wrap and the small-arm wrap the body scores
                 * 13 (tmp/grind/func_80018094/s10/a0.c); dropping the outer wrap instead
                 * scores 13 (s10/f1.c); dropping the small-arm wrap scores 10 (s10/f3.c).
                 * Each of the three wraps is load-bearing and none subsumes another.
                 * lever-exhaustion: memory/grind/func_80018094/hypotheses.md s9b H46-H49 (the
                 * numerator side is spelled out -- eight reference-site spellings measured at
                 * 4/5/8/13) and s10 H50-H52 (the two denominator-side and the
                 * assignment-in-condition levers, all measured dead).
                 * Family: do-while-zero-exception (owner ruling 2026-07-06). */
                /* PsyQ libgte inline macro gte_Lzc(r1,r2) --- gtemac.h:174-178, which
                 * expands to gte_ldlzc(r1) (inline_c.h:228-231, `mtc2 %0,$30`), two
                 * gte_nop() (inline_c.h:1346-1347), then gte_stlzc(r2)
                 * (inline_c.h:1318-1322, `swc2 $31,0(%0)`).  DISCLOSURE OF THE ADDRESSING
                 * PREAMBLE: the two `addu $t4, ..., $zero` moves and the `addiu $v0,$sp,0x10`
                 * are NOT part of those macros own text -- they are the operand-addressing
                 * preamble the original build inline expansion emitted around them (the
                 * `addu $t4,$aN,$zero` + cop2 idiom of the 28-function cluster in
                 * .claude/rules/cop2-addressing-preamble-cluster.md, of which this function
                 * is an enumerated member, line 60).  They are written literally here because
                 * the island must reproduce those bytes; `"=m"(sp_tmp[0])` names the frame
                 * slot the `addiu $v0,$sp,0x10` computes, and `"r"(lut)` the ldlzc input.
                 * The operand list is exactly the granted form -- no extra output, tied, or
                 * clobber operand (Judge constraint, docs/grind/decisions.md 2026-09-09 23:11). */
                do {
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
                } while (0);
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
