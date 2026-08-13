/* REJECTED (measured, s1): the mirror of ptr-stores-symbol-reads — every READ
 * through `ptr`, every STORE through the plain symbol.
 * sandbox --disable all = 27.  52 build insns vs target 49.
 *
 * It DOES achieve two things target has and the other forms don't:
 *   - all three reloads survive (address-expression mismatch again), AND
 *   - `ptr` has 4 uses so combine cannot fold %lo, giving the unfolded
 *     `lui a2; addiu a2` base and `lbu a0,0(a2)` single-insn reads.
 * But the stores now pay `lui at; sb v1,0(at)` twice per block, and the
 * initial `D_80106A73 &= 0xF8;` pays its own separate `lui` for the load and
 * another for the store.  Net: 3 insns longer than target and 4 worse than
 * the 23 form.
 *
 * Conclusion banked: putting the shared unfolded base on the READ side alone
 * is not enough — target shares ONE base between the read and the write of
 * the same block, which is exactly the configuration that re-enables cse
 * forwarding in our fork.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 val;
    u8 val2;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    D_80106A73 &= 0xF8;

    val = *ptr;
    val2 = val | 1;
    if (!(p[8] & 1)) {
        val2 = val;
    }
    D_80106A73 = val2;

    val = *ptr;
    val2 = val | 2;
    if (!(p[8] & 2)) {
        val2 = val;
    }
    D_80106A73 = val2;

    val = *ptr;
    val2 = val | 4;
    if (!(p[8] & 4)) {
        val2 = val;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + 0x17 + i);
    }
}
