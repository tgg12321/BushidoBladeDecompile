/* NOT a floor improvement (sandbox --disable all = 14 > HEAD 13), but a
 * STRUCTURAL finding worth preserving: moving `i=0; tbl=base;` ahead of the
 * `outer:` bits==0 check makes GCC emit the SECOND `beqz $s1,$0` branch the
 * target has (build_insns 96 -> 97, matching target) — i.e. it reproduces the
 * instruction that the regfix `insert "beq $17,$0,{lbl#4}" @ 19` injects, in
 * pure C. So the missing-branch half of the 12-rule cheat is pure-C-solvable.
 *
 * The REMAINING gap (score 14) is the coupled register rotation that did NOT
 * yield to this restructure: one=1 -> $s4 / mask_const=0xFFFFFF -> $s3 / base
 * -> $s5 in target, vs my $s5/$s4 swap; plus a $v0<->$v1 swap in the done
 * block (lw/and/addu operands). Those are the 11 `subst` register-swap rules.
 * A future agent should START from this branch-correct base and attack the
 * RA swap (declaration order did NOT move it; needs Lever A / ALLOCDBG).
 */
void D_80083418(void) {
    s32 i;
    u32 bits;
    s32 *tbl;
    s32 *base;
    s32 one;
    u32 mask_const;

    bits = ((u32)*(volatile s32 *)D_800A263C >> 24) & 0x7F;
    if (bits == 0) goto done;

    one = 1;
    mask_const = 0x00FFFFFF;
    base = D_800A2640;

outer:
    i = 0;
    tbl = base;
    if (bits == 0) goto reload;
inner:
    if (i >= 7) goto reload;
    if (bits & 1) {
        s32 *ctrl = D_800A263C;
        s32 val = *ctrl;
        val &= (one << (i + 24)) | mask_const;
        *ctrl = val;

        if (*tbl != 0) {
            ((void (*)(void))*tbl)();
        }
    }
    tbl++;
    bits >>= 1;
    i++;
    if (bits != 0) goto inner;
reload:
    bits = ((u32)*(volatile s32 *)D_800A263C >> 24) & 0x7F;
    if (bits != 0) goto outer;

done:
    {
        s32 *ctrl = D_800A263C;
        s32 val = *ctrl;
        if ((val & (s32)0xFF000000u) == (s32)0x80000000u) goto error;
        val = *(volatile s32 *)ctrl;
        if (!(val & 0x8000)) return;
    error:
        debug_printf(&D_80016394, *ctrl);
        i = 0;
        do {
            s32 *p = D_800A2660;
            debug_printf(&D_800163B0, i, *(s32 *)((u8 *)p + (i << 4)));
            i++;
        } while (i < 7);
    }
}
