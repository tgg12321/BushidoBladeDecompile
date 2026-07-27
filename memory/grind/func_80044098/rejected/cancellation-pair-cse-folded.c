/* REJECTED s2: cancellation-pair decoration (t1 = (s32)v1 - 1; one = (s32)v1 - t1;
 * a4 -= one;) intended to add +4 flow-time pointer refs with combine folding
 * `one` back to the constant 1.
 * MEASURED: cse's fold_rtx sees through same-EBB register-arithmetic
 * cancellations via qty equivalences — it folded `one = 1` BEFORE flow counted
 * anything (pointer stayed 12 refs / pri 20000), and the folded constant was
 * hoisted out of the loop as `li $5,1` + `subu $3,$3,$5` (byte-WORSE: loop
 * decrement no longer addiu -1, -1 const displaced to $7).
 * GENERALIZES: any same-extended-BB reg-arithmetic identity is cse-folded
 * pre-flow; only same-REGISTER re-set chains are cse-blind (cse cannot express
 * the second set in terms of the clobbered old value) while remaining
 * combine-foldable. That insight produced the winning split-increment form. */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 a4;
    s32 *a6;

    v1 = D_80103608[a0];
    a4 = *(v1 - 1);
    a6 = v1 - 1;
    if (a4 & 0x8000) {
        a4 = a4 & 0x7FFF;
        *(v1 - 1) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            do {
                s32 t1;
                s32 one;
                *v1 -= (s32)a6;
                v1++;
                t1 = (s32)v1 - 1;
                one = (s32)v1 - t1;
                a4 -= one;
            } while (a4 != -1);
        }
    }
}
