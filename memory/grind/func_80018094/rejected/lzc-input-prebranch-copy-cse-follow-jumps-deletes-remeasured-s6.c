/* REJECTED s6 (structural, 2026-09-09). A plain pre-branch copy `lz_in = sum_sq;` in the
 * else-arm, with the island reading "r"(lz_in). Byte-identical to candidate.c (floor 7):
 * tmp/grind/func_80018094/s6/v1.s == w0.s == v2.s (v2 = same copy declared at function top and
 * assigned right after sum_sq). PASS ATTRIBUTION (dump-proven, not inferred): the copy is
 * insn 103 `(set (reg/v:SI 99) (reg/v:SI 77))` in s6/d_v1/v1.i.jump and the asm's input operand
 * is still reg 99 there; in v1.i.cse the operand is reg 77 and the copy is gone. cse.c extends
 * the path across the `beqz` into the island block because cse_end_of_basic_block's follow-jumps
 * arm (tools/gcc-2.7.2/cse.c:8102-8117) fires: LABEL_NUSES(island label) == 1 and the insn before
 * the label is the BARRIER emitted after the LUT arm's `j` to the merge. Re-spelling the copy's
 * declaration scope cannot change either predicate. */
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
            s32 lz_in;
            lz_in = sum_sq;
            if (sum_sq < 0x400) {
                log2_val = (u8)(*(&D_8008D118 + sum_sq)) >> 3;
            } else {
                s32 shift_a, shift_b;
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
                    : "=m"(sp_tmp[0])
                    : "r"(lz_in)
                    : "$2", "$12");
                {
                    s32 lw_v1 = sp_tmp[0];
                    s32 li_v0 = -2;
                    li_v0 = lw_v1 & li_v0;
                    /* FAKE: the LZCR-read result is staged through log2_val -- an existing
                     * enclosing-block local that is dead at this point and is re-assigned with the
                     * LUT result below -- which seats sum_sq in $a1 for every one of its uses,
                     * mechanism: global.c find_reg / local-alloc allocno priority (the extra
                     * reference on log2_val's allocno reorders the ascending first-free scan so the
                     * sum_sq allocno no longer takes $a0), lever-exhaustion:
                     * memory/grind/func_80018094/hypotheses.md s2 H13/H14/H18 + s3 H19-H22, plus
                     * this session's 8,906- and 24,345-iteration permuter campaigns (the find itself
                     * is s4 campaign s4-v20g-framefixed output-230-1, tmp/grind/func_80018094/s4/
                     * perm_find_230.c). Family: staged-value-reused-variable (owner ruling
                     * 2026-07-03). */
                    log2_val = li_v0;
                    shift_a = 0x16 - log2_val;
                }
                shift_b = shift_a >> 1;
                log2_val = ((u8)(*(&D_8008D118 + (sum_sq >> shift_a))) << 16) >> (0x13 - shift_b);
            }
            scale = ((log2_val << 6) / 500) + 0xC0;
        }
    }

    SCRV->x = (SCRV->x * scale) >> 1;
    SCRV->y = (SCRV->y * scale) >> 1;
    SCRV->z = (SCRV->z * scale) >> 1;

    dst = arg1;
    *(MATRIX *)(dst + 5) = *(MATRIX *)arg0[1];
    func_80018300(dst);
}
