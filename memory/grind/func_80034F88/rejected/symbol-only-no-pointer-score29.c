/* REJECTED (measured, s1): the clean "no pointer local at all" spelling.
 * sandbox --disable all = 29 (worse than the 24 baseline, worse than the 23
 * best form).  48 build insns vs target 49.
 *
 * Why it loses: with the address used by exactly one memory operand per
 * statement, GCC's combine folds the %lo into the load/store itself, giving
 * `lui at; sb v1,0(at)` (2 insns, no shared base).  The target instead keeps
 * an unfolded `lui/addiu` base register shared by a load and a store, so
 * every access in this form is spelled differently from target.  On top of
 * that, cse forwards every read of D_80106A73 into the previously stored
 * value, so all three `lbu` reloads that the target performs are missing.
 *
 * The s32-typed-temps variant of this same shape measured 28 (see
 * symbol-only-int-temps-score28.c) — int typing does NOT defeat the forward.
 */
void func_80034F88(void) {
    s32 *p;
    u8 val;
    u8 val2;
    s32 i;

    p = func_80077D00();
    D_80106A73 &= 0xF8;

    val = D_80106A73;
    val2 = val | 1;
    if (!(p[8] & 1)) {
        val2 = val;
    }
    D_80106A73 = val2;

    val = D_80106A73;
    val2 = val | 2;
    if (!(p[8] & 2)) {
        val2 = val;
    }
    D_80106A73 = val2;

    val = D_80106A73;
    val2 = val | 4;
    if (!(p[8] & 4)) {
        val2 = val;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + 0x17 + i);
    }
}
