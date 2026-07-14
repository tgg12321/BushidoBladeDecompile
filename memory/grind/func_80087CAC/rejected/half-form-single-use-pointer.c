/* REJECTED s2 (2026-07-14): distance 12 — byte-identical to the direct form
   (verified by objdump diff, tmp/grind/func_80087CAC/s2/built_v*.s). A
   pointer used at only ONE of the two access sites is CSE-substituted back
   to mem(symbol) and folded away entirely; both half-forms (this one:
   store direct + read via ptr; and the mirror: store via ptr + read direct)
   compile to the same non-matching $at-macro code. Conclusion: BOTH accesses
   must go through the pointer for the address-valued pseudo to survive to RA
   and produce the target's lui+addiu $a3 / 0($a3) shape. */
s16 func_80087CAC(s32 a0, s16 *a1, s16 *a2) {
    u8 *base;
    s16 *ptr;
    s32 slot;
    u8 *p;
    base = (u8 *)((s32 *)&D_80106F28)[(u8)a0];
    ptr = &D_80102806;
    D_80102806 = a0;
    slot = (a0 & 0xFF00) >> 8;
    p = base + slot * 176;
    *a1 = *(u16 *)(p + 0x58);
    *a2 = *(u16 *)(p + 0x5A);
    return *ptr;
}
