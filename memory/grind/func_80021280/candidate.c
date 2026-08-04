/*
 * func_80021280 — best form, sandbox floor 2 (s1, 2026-08-03).
 *
 * Form "C": block-scope declarations WITHOUT initializers + explicit
 * assignments in the order t1=val; a1=0; t4; t3; t2; mode; t0.
 * Emitted preamble order follows source EXACTLY (no pass reorders it):
 *   move t1,v1 / move a1,zero / li t4,4 / li t3,3 / li t2,1 / lui+lh mode / lui+lbu t0
 * Target order: move a1,zero FIRST, then t1,t4,t3,t2,mode,t0.
 * The whole remaining distance-2 is the ADJACENT swap of the first two
 * instructions (move t1,v1 <-> move a1,zero). Everything else matches.
 *
 * CRITICAL constraint discovered s1: writing `a1 = 0;` textually BEFORE
 * `t1 = val;` extends a1's live range across t1's birth, adds an a1<->t1
 * pseudo conflict, and flips the allocator's low-reg tie -> the old
 * $a1/$a2 register swap returns (floor 19). So the naive fix is dead;
 * the flip must come from a mechanism that does not add that conflict
 * (post-RA scheduling, or a t1-set spelling whose conflict RA tolerates).
 *
 * Also dead (s1): any real for/while/do loop form — loop notes make
 * loop.c hoist the constant 5 (li 5) into the preamble; target keeps
 * `addiu v0,zero,5` INSIDE the loop (11B4C), so the original was
 * compiled without LICM on loop2 (goto-shaped loop is correct).
 */
void func_80021280(s32 a0) {
    s32 a1 = 0;
    u8 *a2 = (u8 *)&D_80101EC8 + a0 * 1100;
    u16 a3 = *(u16 *)(a2 + 0x48);
    u16 *v1 = (u16 *)&D_800A38C4;

loop1_21280:
    if (a3 == *v1) goto done1_21280;
    a1++;
    v1++;
    if (a1 < 2) goto loop1_21280;
done1_21280:

    {
        u16 val = *(u16 *)(a2 + 0x48);
        *(s16 *)(a2 + 0x4A) = a1;
        *(s16 *)(a2 + 0x4C) = 0;

        if ((u32)(val >> 12) < 2) {
            u16 t1;
            s32 t4;
            s32 t3;
            s32 t2;
            s32 mode;
            u8 t0;

            t1 = val;
            a1 = 0;
            t4 = 4;
            t3 = 3;
            t2 = 1;
            mode = D_800A38DC;
            t0 = D_800A384C;
        loop2_21280:
            {
                u16 nibble = (t1 >> (a1 << 2)) & 0xF;
                if (nibble != t4) goto not4_21280;
                *(s16 *)(a2 + 0x88) = a1;
                if (mode != t3) goto store4_21280;
                if (a0 != t2) goto store4_21280;
                if (t0 != nibble) goto next_21280;
            store4_21280:
                *(u16 *)(a2 + 0x8A) = *(u16 *)(a2 + 0x26C);
                goto next_21280;
            not4_21280:
                if (nibble != 5) goto next_21280;
                *(s16 *)(a2 + 0x8E) = a1;
                if (mode != 0) goto store5_21280;
                if (D_800A385C == 0) goto store5_21280;
                if (a0 == 0) goto next_21280;
            store5_21280:
                *(u16 *)(a2 + 0x90) = *(u16 *)(a2 + 0x26C);
            }
        next_21280:
            a1++;
            if (a1 < 3) goto loop2_21280;
        }
    }
}
