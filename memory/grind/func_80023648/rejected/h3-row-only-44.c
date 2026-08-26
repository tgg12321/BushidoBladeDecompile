/* REJECTED s1: H3 variant (b) — drop new_var, keep row. Measured sandbox 44
   (worse than floor 30), insns 159/159. Removing the base-pointer named
   intermediate reshuffles seats further from target. */
            row = &D_8008EB40 + a0 * 3;
            a2 = row[a1];
