/* REJECTED — K2. Re-base the pointer at D_80102326 so the +0x44C site becomes
 * offset 0. MEASURED sandbox --disable all == 24 (WORSE than baseline 21): the
 * symbol fold simply migrated to the three now-displaced low-halfword sites
 * (`lui/lh -1100`, `lui/sh -1100`). Proves the fold keys on the displacement being
 * NON-ZERO — independent of sign and of which symbol names the base. This kills the
 * whole pointer-rebasing / offset-respelling axis for region A. Do not re-propose. */
        if (qf & 0x30) {
            s16 *eda = &D_80102326;
            saved_first = eda[-0x226];
            saved_44c = eda[0];
            if (qf & 0x10) eda[-0x226] = 0x32;
            if (q[3] & 0x20) eda[0] = 0x32;
            func_8003AFFC();
            eda[-0x226] = saved_first;
            eda[0] = saved_44c;
        }
