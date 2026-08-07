/* cpu_check_tubazeri_2 — honest sandbox floor 1 (was 4).
 * s2 loop-copy rewritten from a walking-pointer do-while to an INDEX-off-a0
 * for-loop, mirroring the byte-matched sibling func_80030900 (same file/TU).
 * That fixed the loop.c strength_reduce/combine_givs divergence (idx37/38/40,
 * 3 insns) — cheat-reviewer PASSED this loop shape.
 * Sole residual diff (1 insn): idx25 `addu s2,s0,v0` (ours, base-first) vs
 * target `addu s2,v0,s0` (index-first). Only reachable via integer-domain
 * offset-first add `(s32*)(v0 + (s32)a0)` — cheat-reviewer FAILED that as a
 * commutative-operand-order coercion (rejected/int-cast-operand-swap.c).
 * All pointer-domain spellings canonicalize base-first (front-end pointer_int_sum).
 */
s32 cpu_check_tubazeri_2(s32 *a0) {
    s32 count;
    s32 s1;
    s32 v0;
    s32 v1;
    s32 *s2;
    s32 s3;
    s32 *a2;
    s32 i;

    count = *(s16 *)((u8 *)a0 + 0x330);
    if (count == 0) {
        return -1;
    }
    s1 = 0;
    if (count < 2) {
        goto do_sll;
    }
    if (*(s16 *)((u8 *)a0 + 0x88) == -1) {
        goto do_sll;
    }
    v0 = *(s16 *)((u8 *)a0 + 0x332);
    v1 = *(s16 *)((u8 *)a0 + 0x14);
    v0 = v0 ^ v1;
    s1 = (u32)v0 < 1;
do_sll:
    v0 = s1 << 1;
    s2 = (s32 *)((u8 *)a0 + v0);
    s3 = *(s16 *)((u8 *)s2 + 0x332);
    a2 = coli_hit_body_weapon(a0, s3);
    for (i = s1; i < *(s16 *)((u8 *)a0 + 0x330) - 1; i++) {
        *(u16 *)((u8 *)a0 + 0x332 + i * 2) = *(u16 *)((u8 *)a0 + 0x334 + i * 2);
    }

    *(u16 *)((u8 *)a0 + 0x330) = *(u16 *)((u8 *)a0 + 0x330) - 1;
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
