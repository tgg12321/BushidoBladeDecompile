/* REJECTED (s2 2026-08-24) — declaring the Copy8-loop list-push walker (a2p,
 * s4+0x10D4) and the later s4+0x8B4 walker (a2p2) as TWO separate locals.
 * This was s1's floor-24 form.  It inverts the $a2/$a3 seats in the Copy8 loop
 * (14 diffs) and the $a2/$a0 seats in the tail walker (6 diffs).
 *
 * WHY IT IS DEAD (measured, .flow dump):
 *   pseudo 186 (a2p)  refs=4  live_length=13  -> pri = log2(4)*4/13 = 0.615
 *   pseudo 187 (a3p)  refs=5  live_length=12  -> pri = log2(5)*5/12 = 0.833
 * global.c allocno_compare sorts priority-descending, so 187 is allocated
 * BEFORE 186 and takes the first free hard reg ($6); 186 gets $7.  Target is
 * the opposite.  With a2p and a2p2 merged into one variable the merged
 * pseudo's refs rise to ~9 over a ~25-insn live range (pri ~1.08), it sorts
 * ahead of a3p, takes $6, and a3p falls to $7 — target exactly.
 * Do NOT re-propose the split form.
 */
    {
        u8 *a2p;
        u8 *a3p;
        a2p = s4 + 0x10D4;
        a3p = s4 + 0x10EC;
    copyloop:
        {
            s5 = *(u8 **)(a3p + 0x40);
            if (s5 == 0) goto copydone;
            *(Copy8_40D48 *)a3p = *(Copy8_40D48 *)(s5 + 0x18);
            {
                s32 *list3;
                list3 = (s32 *)D_800A3820;
                a3p += 0x68;
                D_800A3820 = (s32)(list3 + 1);
                *list3 = (s32)a2p;
            }
            a2p += 0x68;
            goto copyloop;
        }
    copydone:;
    }

    {
        s16 *a2p2;                       /* <-- the second local: the defect */
        a2p2 = (s16 *)(s4 + 0x8B4);
        if (*(s16 *)(s4 + 0x8B6) != -1) {
            do {
                s32 *list4;
                list4 = (s32 *)D_800A3820;
                D_800A3820 = (s32)(list4 + 1);
                *list4 = (s32)a2p2;
                a2p2 = (s16 *)((u8 *)a2p2 + 0x68);
            } while (a2p2[1] != -1);
        }
    }
