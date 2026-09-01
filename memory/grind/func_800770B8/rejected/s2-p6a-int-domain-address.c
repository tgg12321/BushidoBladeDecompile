/* REJECTED (s2, 2026-09-01) — floor 34 (174 insns) from a 10 chassis. Catastrophic.
 * WHY DEAD: pushing the p_6a/p_7e base addresses into the INT domain (the trick that
 * fixed the row-43 addu operand order in s1, H4) does NOT transfer to rows 62-64.
 * Here the +0x6A / +0x7E constants fold into the int expression, which destroys the
 * CSE of the shared (t0 * 10) subexpression and restructures the whole row-55..70
 * region. Confirms s1's K3 conclusion from the other side: rows 62-64 are a
 * local-alloc dest-coalesce decision, not an address-expression-shape decision.
 */
            s16 *p_6a = (s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x6A);
            s16 *p_7e = (s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x7E);
