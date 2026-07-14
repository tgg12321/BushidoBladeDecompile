/* REJECTED s2 (2026-07-14): distance 12 (judge probe (a), prediction from s1
   evidence CONFIRMED). Diff shape: store becomes the assembler-macro form
   `lui $at; sh $a0,%lo(D_80102806)($at)` and is scheduler-HOISTED above the
   base-table lw (which sinks 8 slots); the return reload becomes an
   independent `lui $v0; lh $v0,%lo(...)($v0)`. No addiu, no shared $a3.
   GCC 2.7.2 does not CSE symbol-address HIGHs across a store/load pair.
   Identical bytes are produced by BOTH half-forms (store direct + read via
   ptr; store via ptr + read direct — single-use pointer is CSE-folded back
   to mem(symbol)) and by the `*(&D_80102806)` addr-of-deref spelling: 4
   spellings, one non-matching codegen. See tmp/grind/func_80087CAC/s2/. */
s16 func_80087CAC(s32 a0, s16 *a1, s16 *a2) {
    u8 *base;
    s32 slot;
    u8 *p;
    base = (u8 *)((s32 *)&D_80106F28)[(u8)a0];
    D_80102806 = a0;
    slot = (a0 & 0xFF00) >> 8;
    p = base + slot * 176;
    *a1 = *(u16 *)(p + 0x58);
    *a2 = *(u16 *)(p + 0x5A);
    return D_80102806;
}
