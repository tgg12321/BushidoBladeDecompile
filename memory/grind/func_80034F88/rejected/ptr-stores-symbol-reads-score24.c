/* REJECTED (measured, s1): every STORE through `ptr`, every READ through the
 * plain symbol.  sandbox --disable all = 24 (worse than the 23 best form).
 * 51 build insns vs target 49.
 *
 * This form is STRUCTURALLY the closest to target that was reached this
 * session: it produces all three `lbu` reloads of D_80106A73 that the target
 * performs (the store/read address-expression mismatch defeats cse.c's
 * store-to-load forwarding at every block, not just the first one).  It still
 * scores worse than the 23 form for two reasons, both visible in the objdump:
 *
 *  1. Each read is `lui a0; lbu a0,0(a0)` (combine folds %lo into the load
 *     because the read address has exactly one use), whereas target reads
 *     `lbu v1,0(a0)` off a shared unfolded `lui/addiu` base that its store
 *     also uses.  So the reads are 2 insns each here and 1 insn each there.
 *  2. Because the read's address is produced by a fresh `lui` right before
 *     it, the scheduler has to emit the read ahead of `lw v0,0x20(a1)`,
 *     leaving a `nop` in the lw's load-delay slot on all three blocks.  In
 *     target the reload fills that delay slot exactly.
 *
 * Keep this file: it is the cleanest demonstration that "get the reloads" and
 * "get the shared base register" are, under every spelling measured so far,
 * mutually exclusive.  Resolving that tension is the frontier.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 val;
    u8 val2;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    val = D_80106A73;
    val2 = val | 1;
    if (!(p[8] & 1)) {
        val2 = val;
    }
    *ptr = val2;

    val = D_80106A73;
    val2 = val | 2;
    if (!(p[8] & 2)) {
        val2 = val;
    }
    *ptr = val2;

    val = D_80106A73;
    val2 = val | 4;
    if (!(p[8] & 4)) {
        val2 = val;
    }
    *ptr = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + 0x17 + i);
    }
}
