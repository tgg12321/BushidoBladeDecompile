/* REJECTED s1: builds 84 insns vs target 83. Folding the shift into s4's
 * initializer and re-deriving v1 = a0<<4 after i = a0 makes GCC emit an extra
 * copy for the CSE-reused sll temp. */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + (a0 << 4));
    s32 i = a0;
    s32 v1 = a0 << 4;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* ... rest identical to candidate.c ... */
}
