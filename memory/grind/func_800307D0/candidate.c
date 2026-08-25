/* func_800307D0 (formerly cpu_check_tubazeri_2) — honest sandbox distance 0.
 *
 * Head reconstruction (s8): the queue-slot read is a plain dereference with an
 * inline scaled subscript, `*(s16 *)((u8 *)a0 + s1 * 2 + 0x332)`, instead of
 * first materialising a named byte offset and a named base pointer. That is the
 * simpler and more idiomatic spelling (one local fewer, same semantics), and it
 * is also the spelling the original source must have used: GCC 2.7.2 expands a
 * `+` inside a MEM address with modifier EXPAND_SUM (expr.c:5238), which reaches
 * the `both_summands` "put a multiplication first" swap (expr.c:5289) fed by the
 * MULT_EXPR indexed-address path (expr.c:5362) — producing the target's
 * index-first `addu $s2,$v0,$s0` at idx25. An assignment RHS goes to `binop`
 * instead and keeps the front end's pointer-first order (c-typeck.c:2696), which
 * is the 1-insn residual sessions s2-s7 measured as an "endgame lock".
 *
 * The copy loop is the s2 index-off-a0 form (mirrors the byte-matched sibling
 * func_80030900, same TU) that fixed the loop.c strength_reduce/combine_givs
 * divergence at idx37/38/40.
 */
s32 func_800307D0(s32 *a0) {
    s32 count;
    s32 s1;
    s32 v0;
    s32 v1;
    s32 s3;
    s32 *a2;
    s32 i;

    count = *(s16 *)((u8 *)a0 + 0x330);
    if (count == 0) {
        return -1;
    }
    s1 = 0;
    if (count >= 2 && *(s16 *)((u8 *)a0 + 0x88) != -1) {
        v0 = *(s16 *)((u8 *)a0 + 0x332);
        v1 = *(s16 *)((u8 *)a0 + 0x14);
        v0 = v0 ^ v1;
        s1 = (u32)v0 < 1;
    }
    s3 = *(s16 *)((u8 *)a0 + s1 * 2 + 0x332);
    a2 = func_80030580(a0, s3);
    for (i = s1; i < *(s16 *)((u8 *)a0 + 0x330) - 1; i++) {
        *(u16 *)((u8 *)a0 + 0x332 + i * 2) = *(u16 *)((u8 *)a0 + 0x334 + i * 2);
    }

    *(u16 *)((u8 *)a0 + 0x330) = *(u16 *)((u8 *)a0 + 0x330) - 1;
    v1 = *(s16 *)((u8 *)a2 + 2);
    if (v1 == 0xE) {
        func_80032854((D_800A36F2 ^ 0xE) != 0, 0x2F, (u8 *)a2 + 0x2C, 0);
    } else {
        func_80032854((v1 ^ D_800A36F2) != 0, 0x2A, (u8 *)a2 + 0x2C, 0);
    }
    return s3;
}
