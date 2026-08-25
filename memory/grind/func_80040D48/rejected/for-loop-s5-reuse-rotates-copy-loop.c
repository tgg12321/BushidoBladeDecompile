/* REJECTED (s1 2026-08-24): reusing s5 for the Copy8-loop source pointer while
 * keeping the `for (;;) { ...; break; }` spelling. Measured 31 -> 29 but
 * build_insns 273 vs 272: with the outer variable as the loop pointer, GCC
 * rotates the loop (duplicated bottom test `bnez s5`) and cse then folds the
 * first-iteration load to `lw s5,0x112C(s4)` (s4-relative), peeling an extra
 * insn. Target keeps the top-test + unconditional-j shape with the load through
 * the a3p base even on iteration 0. The FIX that keeps the reuse AND the shape:
 * spell the loop with explicit labels (copyloop:/goto copyloop; exit via goto
 * copydone) so no loop notes are emitted and loop.c never rotates — that form
 * measured 24 and is in candidate.c. Banked so no session re-tries the for(;;)
 * spelling with a reused outer pointer. */
    {
        u8 *a2p;
        u8 *a3p;
        a2p = s4 + 0x10D4;
        a3p = s4 + 0x10EC;
        for (;;) {
            s5 = *(u8 **)(a3p + 0x40);
            if (s5 == 0) break;

            *(Copy8_40D48 *)a3p = *(Copy8_40D48 *)(s5 + 0x18);

            {
                s32 *list3;
                list3 = (s32 *)D_800A3820;
                a3p += 0x68;
                D_800A3820 = (s32)(list3 + 1);
                *list3 = (s32)a2p;
            }
            a2p += 0x68;
        }
    }
