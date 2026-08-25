/* func_800307D0 (formerly cpu_check_tubazeri_2) -- honest sandbox distance 0,
 * full-build SHA1 == oracle (both re-measured on the s9 chassis, 2026-08-25).
 * NOT APPLIED TO src/ -- HEAD stays INCLUDE_ASM pending a ruling on the s8 ban
 * (see the s9 outcome ruling_question and evidence.md == s9 ==).
 *
 * s9 SIBLING-TRANSPLANT DERIVATION (rederive modality). The body is the
 * s2-s7 reviewer-PASSed floor-1 candidate with exactly two changes, both
 * transplanted verbatim from this TU's own byte-matched, zero-rule,
 * COMPLETED-C siblings that operate on the SAME 0x330/0x332 queue array of the
 * SAME object:
 *   1. the parameter is typed `u8 *a0`, as in func_80030900(u8 *a0, ...)
 *      (src/code6cac_b.c:1163) and func_80030B10(u8 *arg0, ...) (:1255) --
 *      which removes every `(u8 *)` cast from the body;
 *   2. the queue-slot read is the file's own idiom for this array,
 *      `*(s16 *)(a0 + s1 * 2 + 0x332)`, mirroring func_80030B10:1283
 *      `*(u16 *)(arg0 + (s16)c * 2 + 0x332)` and func_8003047C:1123
 *      `*(s16 *)(a0 + 0x332 + i * 2)` -- instead of first materialising a named
 *      byte offset (`v0 = s1 << 1`) and a named base pointer.
 * The copy loop is unchanged from the floor-1 candidate (the s2 index-off-a0
 * form mirroring func_80030900, which fixed the loop.c strength_reduce /
 * combine_givs divergence at idx37/38/40).
 *
 * MECHANISM (my own cc1 -da dump this session,
 * tmp/grind/func_800307D0/s9/idx25_rtl.txt): reading through the dereference
 * expands the `+` in MEM-address context (EXPAND_SUM), where expr.c:5288-5290
 * ("put a multiplication first") swaps the summands because the inline `s1 * 2`
 * is still a MULT rtx; initial RTL insn 78 = (plus (reg 94 = index)
 * (reg/v 72 = a0)) -- index-first even though a0 holds the LOWER pseudo -- and
 * the final asm is `addu $s2,$v0,$s0` == target idx25. An assignment RHS goes
 * to `binop` instead (expr.c:5238) and keeps the front end's pointer-first
 * tree order (c-typeck.c:2696), which is the 1-insn floor-1 residual that
 * sessions s2-s7 measured. optabs.c:399-421 expand_binop never swaps two
 * pseudo REGs, so it is not the actor here (owner-directed check: negative).
 *
 * FRESH m2c CORROBORATION (tmp/grind/func_800307D0/s9/m2c_fresh.c): m2c,
 * with no knowledge of this grind, reconstructs the same add index-first
 * (`temp_s2 = var_v0 + arg0;`).
 */
s32 func_800307D0(u8 *a0) {
    s32 count;
    s32 s1;
    s32 v0;
    s32 v1;
    s32 s3;
    s32 *a2;
    s32 i;

    count = *(s16 *)(a0 + 0x330);
    if (count == 0) {
        return -1;
    }
    s1 = 0;
    if (count < 2) {
        goto do_sll;
    }
    if (*(s16 *)(a0 + 0x88) == -1) {
        goto do_sll;
    }
    v0 = *(s16 *)(a0 + 0x332);
    v1 = *(s16 *)(a0 + 0x14);
    v0 = v0 ^ v1;
    s1 = (u32)v0 < 1;
do_sll:
    s3 = *(s16 *)(a0 + s1 * 2 + 0x332);
    a2 = func_80030580((s32 *)a0, s3);
    for (i = s1; i < *(s16 *)(a0 + 0x330) - 1; i++) {
        *(u16 *)(a0 + 0x332 + i * 2) = *(u16 *)(a0 + 0x334 + i * 2);
    }

    *(u16 *)(a0 + 0x330) = *(u16 *)(a0 + 0x330) - 1;
    v1 = *(s16 *)((u8 *)a2 + 2);
    if (v1 == 0xE) {
        s32 a0_arg = D_800A36F2 ^ 0xE;
        func_80032854(a0_arg != 0, 0x2F, (u8 *)a2 + 0x2C, 0);
    } else {
        s32 a0_arg = v1 ^ D_800A36F2;
        func_80032854(a0_arg != 0, 0x2A, (u8 *)a2 + 0x2C, 0);
    }
    return s3;
}
