/* func_80017FA0 (code6cac.c) — BEST honest pure-C candidate (rules NEUTRALIZED).
 * Honest distance 16 vs HEAD's cheated-but-matching form (6 regfix subst/delete
 * rules + asm barriers). NOT committable: the residual 16 is two irreducible,
 * non-pure-C gaps — see notes.md. DO NOT re-derive; this is the floor.
 *
 * Inner loop (the bulk) MATCHES byte-for-byte with this form: keep the inner
 * scratchpad writes as FULL-constant-address `*(volatile s32*)(0x1F800064 + n)`
 * so GCC re-materializes `lui 0x1f80` each iteration (target does NOT LICM-hoist
 * it). The fixed writes via `((volatile s32*)0x1F800000)[idx]` fold inconsistently. */
void func_80017FA0(s32 *a0) {
    s32 _frame;
    s32 temp;
    s32 *ptr;
    temp = a0[3];
    if (temp == 0) { goto end; }
    ptr = (s32 *)temp;
    ((volatile s32 *)0x1F800000)[0x2E] = ptr[0] << 7;   /* 0x1F8000B8 — folds inconsistently */
    {
        s32 i = 0;
        if (ptr[1] > 0) {
            s32 *p68 = ptr;
            volatile s32 *ac_base = (volatile s32 *)0x1F800000;
            s32 sp_off = 0;
            do {
                s32 j = 0;
                s32 data_off = i << 5;
                s32 sp_inner = sp_off;
                do {
                    s32 *dp = (s32 *)((u8 *)ptr + data_off);
                    *(volatile s32 *)(0x1F800064 + sp_inner) = dp[2] << 2;
                    data_off += 0x10;
                    *(volatile s32 *)(0x1F800068 + sp_inner) = dp[3] << 2;
                    j++;
                    *(volatile s32 *)(0x1F80006C + sp_inner) = dp[4] << 2;
                    sp_inner += 0xC;
                } while (j < 2);
                ac_base[0x2B] = *(s32 *)((u8 *)p68 + 0x68) << 2;
                p68 = (s32 *)((u8 *)p68 + 4);
                sp_off += 0x18;
                i++;
                ac_base++;
            } while (i < ptr[1]);
        }
    }
    ((volatile s32 *)0x1F800000)[0x18] = ((s32 *)a0[3])[1];   /* 0x1F800060 — does NOT fold */
end:
    (void)_frame;
}
