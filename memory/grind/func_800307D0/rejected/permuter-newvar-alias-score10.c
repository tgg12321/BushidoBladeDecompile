/* REJECTED (s4, permuter seed-1 output-10-1) — does NOT close; score stays 10 (=sandbox 1).
 *
 * decomp-permuter's ONLY novel find across ~102k iterations (two structurally
 * distinct chassis seeds) was this pointer-alias variant: it hoists `new_var =
 * (u8*)a0;` and threads it through the byte-offset casts. The residual add still
 * lowers base-first:
 *     s2 = (s32*)(new_var + v0)   ->   addu s2,s0,v0   (target wants addu s2,v0,s0)
 * i.e. the alias is score-NEUTRAL. This is the empirical corroboration of the s3
 * RTL proof: pointer_int_sum canonicalizes ptr+int to base-first at tree-lowering,
 * so NO pointer-domain restructuring (alias, reassoc, temp split, s16* scaling,
 * while vs for loop) can flip idx25. The only distance-0 form remains the
 * integer-domain offset-first add v0+(s32)a0 — reviewer-FAILED cheat
 * (rejected/int-cast-operand-swap.c). Permuter never synthesized even that.
 */
s32 cpu_check_tubazeri_2(s32 *a0) {
    s32 count;
    u8 *new_var;
    s32 s1;
    s32 v0;
    s32 v1;
    s32 *s2;
    s32 s3;
    s32 *a2;
    s32 i;
    new_var = (u8 *)a0;
    count = *(s16 *)(new_var + 0x330);
    if (count == 0) {
        return -1;
    }
    s1 = 0;
    if (count < 2) {
        goto do_sll;
    }
    if (*(s16 *)(new_var + 0x88) == -1) {
        goto do_sll;
    }
    v0 = *(s16 *)((u8 *)a0 + 0x332);
    v1 = *(s16 *)(new_var + 0x14);
    v0 = v0 ^ v1;
    s1 = (u32)v0 < 1;
do_sll:
    v0 = s1 << 1;
    s2 = (s32 *)(new_var + v0);
    s3 = *(s16 *)((u8 *)s2 + 0x332);
    a2 = coli_hit_body_weapon(a0, s3);
    for (i = s1; i < *(s16 *)(new_var + 0x330) - 1; i++) {
        *(u16 *)((new_var + 0x332) + i * 2) = *(u16 *)((new_var + 0x334) + i * 2);
    }
    *(u16 *)(new_var + 0x330) = *(u16 *)(new_var + 0x330) - 1;
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
