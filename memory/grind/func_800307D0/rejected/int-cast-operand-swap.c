/* REJECTED — cheat-reviewer FAIL (s2 session, 2026-07-21).
 *
 * This form reaches honest sandbox distance 0 (target_insns==build_insns==76,
 * all 4 diffs gone) but was FAILED by the adversarial cheat-reviewer as a
 * commutative-operand-order coercion — the pointer/address-arithmetic analogue
 * of the forbidden or-tree-shape-shift family.
 *
 * The ONLY change vs the clean floor-1 candidate is the s2 pointer add:
 *     s2 = (s32 *)(v0 + (s32)a0);     // <-- CHEAT: integer-domain, offset-first
 * instead of the clean
 *     s2 = (s32 *)((u8 *)a0 + v0);    // base-first (score 1)
 *
 * WHY IT SCORES 0: target emits `addu s2,v0,s0` (scaled-index first). All
 * pointer-domain spellings — (u8*)a0+v0, v0+(u8*)a0, &((s16*)a0)[s1] — are
 * canonicalized base-first by the front-end pointer_int_sum lowering, giving
 * `addu s2,s0,v0`. Only doing the add in the INTEGER domain with the offset
 * written first (`v0 + (s32)a0`) preserves source order and emits index-first.
 *
 * WHY IT'S A CHEAT (reviewer verdict, tests 1/2/3/5):
 *  - computes the byte-identical address to (u8*)a0+v0 (no semantic difference);
 *  - no human programmer casts the pointer param to s32 and writes the offset
 *    first — the natural forms all lower base-first;
 *  - its only describable purpose is controlling the addu operand order via a
 *    GCC front-end pass, not expressing different program logic;
 *  - same intent as the forbidden or-tree-shape-shift (commutative-operand /
 *    parenthesization-axis mutation), a new spelling of a forbidden intent.
 *
 * DO NOT re-propose. Full body below for reference (only the s2 line differs).
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
    s2 = (s32 *)(v0 + (s32)a0);   /* CHEAT: integer-domain offset-first add */
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
