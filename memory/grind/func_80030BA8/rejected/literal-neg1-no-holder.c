/* REJECTED — sandbox --disable all = 14, build 85 insns vs target 87.
 * The Judge's prescribed remediation for `neg1`: use literal -1 at both sites.
 * The two MISSING instructions are `sw $s3,0x1C($sp)` / `lw $s3,0x1C($sp)`:
 * with no single pseudo, GCC never allocates a callee-save home for -1 and
 * re-materializes it at each use instead. Target keeps -1 in $s3 across the
 * jal (prologue `addiu $s3,$zero,-1`; consumed by the per-iteration
 * `beq $a1,$s3` AND by `sh $s3,0($s0)` AFTER the call).
 * CONFIRMS the s1 hypothesis: the -1 holder is load-bearing.
 */
s32 func_80030BA8(u8 *arg0) {
    s32 i = 0;
    u8 *p = (u8 *)&D_80106A7A;
    s32 old_val;

    loop:;
    {
        u16 val = *(u16 *)p;
        s32 sval;
        if ((unsigned)(val - 0x12) < 12u) { goto next; }
        sval = (s16)val;
        if (sval == -1) { goto next; }        /* was: sval == neg1 */
        /* ... unchanged ... */
        *(s16 *)p = -1;                        /* was: (s16)neg1 */
    }
    /* ... */
}
