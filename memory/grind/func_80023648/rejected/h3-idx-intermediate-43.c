/* REJECTED s1: H3 variant (c) — named idx intermediate before the split.
   Measured sandbox 43 (worse than floor 30), insns 159/159. */
            {
                s32 idx = a0 * 3;
                new_var = &D_8008EB40;
                row = new_var + idx;
                a2 = row[a1];
            }
