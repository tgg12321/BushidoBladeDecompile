/* REJECTED s2e (2026-07-14): same pointer alias but declared at FUNCTION
   scope with the other locals (no inner block).
   sandbox --disable all = 7 (55/56) — WORSE than block-scoped 0 and worse
   than direct-write 6. Mechanism (variantG_objdump_full.txt): GCC expands
   the initializer at the declaration point, so the address materializes
   into $a2 (lui/addiu at 0x884-0x888) BEFORE the first value diamond
   (a1 && D_800A37A0==1), not into $v1 between the diamonds as target
   requires; the scheduler then hoists `li v0,29` into the first beqz delay
   slot, cascading through both diamonds. PROVES the inner block scope is
   LOAD-BEARING (pins the init after the first diamond), not stylistic —
   the committed shape must keep the block.
   Artifacts: tmp/grind/func_8003B3A4/s2/variantG_function_scope_pointer.txt
   + variantG_objdump_full.txt */
s32 func_8003B3A4(u8 *arg0) {
    u8 idx;
    u8 a1;
    u8 *p = &D_8010277D;
    D_800A3712 = 0;
    idx = (&D_8008D538)[(s8)D_8010277C];
    a1 = (&D_8008D9EC)[idx];
    if (a1 != 0 && D_800A37A0 == 1) {
        a1 = 0;
    }
    if (a1 != 0) {
        *p = 0xE;
    } else {
        *p = 0x1D;
    }
    D_8010277F = 0;
    {
        u8 v = arg0[0];
        D_800A3680 = v;
        D_800A3671 = v;
    }
    D_80102783 = arg0[1];
    D_800A37B4 = arg0[2];
    D_800A37B5 = arg0[3];
    D_800A37B6 = arg0[4];
    func_8003AF40(1);
    func_8003AFFC();
    return 5;
}
