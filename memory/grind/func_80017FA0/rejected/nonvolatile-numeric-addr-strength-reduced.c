/* REJECTED (s5, 2026-08-20) - "just drop the volatile".
 *
 * This is the previously-banked s4 candidate with `volatile` removed from the
 * two scratchpad pointers and the three inner-loop store casts, i.e. the
 * minimal form that satisfies the Judge's ban on volatile-qualified access to
 * 0x1F800000-0x1F8003FF.
 *
 * MEASURED: 57 insns vs target's 61, frame still `vars= 8` (so the s4 frame
 * lever is INDEPENDENT of the volatile - it survives untouched).  The whole
 * 4-insn shortfall is GCC 2.7.2 loop.c strength reduction on the inner j-loop:
 * with a NUMERIC constant address, `(mem (plus (reg sp_inner) (const_int
 * 0x1F800064)))` is a general induction variable, and loop.c combines the three
 * address givs into ONE biased base register, hoisting
 *     lui v0,0x1f80 ; ori v0,v0,0x6c ; addu a1,t2,v0
 * into the outer loop and rewriting the stores as `sw v0,-8(a1) / -4(a1) /
 * 0(a1)`.  Target instead keeps `move a1,t2` and re-materialises the base at
 * every store (`lui at,0x1f80 ; addu at,a1,at ; sw v0,100(at)`).
 *
 * Five further numeric-address spellings were measured and are ALSO strength
 * reduced (all in tmp/grind/func_80017FA0/s4/): u32 sp_inner (vb1, 57),
 * (u8 *)0x1F800064 + sp_inner pointer arithmetic (vb2, 57), operand order
 * sp_inner + 0x1F800064 (va2, 57), sp_off = sp_inner so the unbiased value is
 * consumed after the inner loop (va1, 57), a named address intermediate
 * `s32 ad = 0x1F800064 + sp_inner` with ad/ad+4/ad+8 (vc1, 60), and dropping
 * sp_inner entirely for `0x1F800064 + i*0x18 + j*0xC` (vc4, 63 - three
 * separate hoisted bases).  The numeric-constant address form is dead.
 *
 * The live successor is the extern-symbol address form banked in candidate.c,
 * which blocks the giv without any volatile (61 insns, 58/61 identical). */
void func_80017FA0(s32 *a0) {
    s32 *scr = (s32 *)0x1F800000;
    s32 temp;
    s32 *ptr;

    temp = a0[3];
    if (temp == 0) {
        goto end;
    }
    ptr = (s32 *)temp;

    scr[0x2E] = ptr[0] << 7;

    {
        s32 i = 0;
        if (i < ptr[1]) {
            s32 *p68 = ptr;
            s32 *ac_base = (s32 *)0x1F800000;
            s32 sp_off = 0;
            do {
                s32 j = 0;
                s32 data_off = i << 5;
                s32 sp_inner = sp_off;
                do {
                    s32 *dp = (s32 *)((u8 *)ptr + data_off);
                    *(s32 *)(0x1F800064 + sp_inner) = dp[2] << 2;
                    data_off += 0x10;
                    *(s32 *)(0x1F800068 + sp_inner) = dp[3] << 2;
                    j++;
                    *(s32 *)(0x1F80006C + sp_inner) = dp[4] << 2;
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

    scr[0x18] = ((s32 *)a0[3])[1];
end:
    ;
}
