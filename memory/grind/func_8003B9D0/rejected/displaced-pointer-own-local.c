/* REJECTED — K1. [[defeat-combine-symbol-fold]] lever: pre-compute the displaced
 * address into its own local. MEASURED sandbox --disable all == 21 (no change from
 * baseline 21); region A bytes identical — cc1 still folds all three +0x44C accesses
 * to %lo(D_80101EDA+0x44C). The rule's precondition (a call between the pointer def
 * and the displaced use) does not hold for 2 of the 3 sites. Do not re-propose. */
        if (qf & 0x30) {
            s16 *eda = &D_80101EDA;
            s16 *eda_alt = eda + 0x226;
            saved_first = eda[0];
            saved_44c = *eda_alt;
            if (qf & 0x10) eda[0] = 0x32;
            if (q[3] & 0x20) *eda_alt = 0x32;
            func_8003AFFC();
            eda[0] = saved_first;
            *eda_alt = saved_44c;
        }
