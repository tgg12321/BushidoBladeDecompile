/* REJECTED s1: plain *(s32 *)0x1F8000xx scratchpad accesses. sandbox 81. Block-1 loads hoist above the
 * stores (sched.c true_dependence MEM_IN_STRUCT_P / rtx_addr_varies_p exception: a non-struct
 * constant-address store has no dependence against an in-struct varying load); the block-2
 * read-modify-write materialises the address with li and shares it between lw and sw.
 * Diff: tmp/grind/func_80018094/s1/v1_pairdiff.txt */
void func_80018094(s32 *arg0, s32 *arg1) {
    s32 sp_tmp;
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
    *(s32 *)0x1F800024 = dx;
    dy = ((s32 *)arg0[1])[6] - arg1[11];
    *(s32 *)0x1F800028 = dy;
    dz = ((s32 *)arg0[1])[7] - arg1[12];
    *(s32 *)0x1F80002C = dz;

    sum_sq = (dx * dx) + (dy * dy) + (dz * dz);

    scale = 0x100;
    if (sum_sq <= 250000) {
        if (sum_sq < 0) {
            scale = 0;
        } else {
            s32 log2_val;
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
                    : "=m"(sp_tmp)
                    : "r"(sum_sq)
                    : "$2", "$12");
                shift_a = 0x16 - (sp_tmp & -2);
                shift_b = shift_a >> 1;
                log2_val = ((u8)(*(&D_8008D118 + (sum_sq >> shift_a))) << 16) >> (0x13 - shift_b);
            }
            scale = ((log2_val << 6) / 500) + 0xC0;
        }
    }

    *(s32 *)0x1F800024 = (*(s32 *)0x1F800024 * scale) >> 1;
    *(s32 *)0x1F800028 = (*(s32 *)0x1F800028 * scale) >> 1;
    *(s32 *)0x1F80002C = (*(s32 *)0x1F80002C * scale) >> 1;

    dst = arg1;
    *(MATRIX *)(dst + 5) = *(MATRIX *)arg0[1];
    func_80018300(dst);
}
