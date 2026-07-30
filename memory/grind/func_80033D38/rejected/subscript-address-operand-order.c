/* REJECTED -- func_80033D38 -- distance 1 (loop1) / 2 (loop1 + tail).
 * Everything else byte-exact; the sole residual is the element-address `addu`
 * operand order.
 *
 *   TARGET   sll v0,v1,3 ; addu v0,t1,v0     (base + scaled index)
 *   BUILD    sll v0,v1,3 ; addu v0,v0,t1     (scaled index + base)
 *
 * Mechanism: a subscript or a parenthesised pointer sum used directly inside an
 * expression builds the address as plus(scaled_index, base) and emits the
 * index-first `addu`. Measured equivalent-and-still-wrong spellings, all
 * index-first: `recs[j + 1].t`, `(j + 1)[recs].t` (the commuted subscript),
 * `(recs + j + 1)->t`, and in the tail `recs[n + 1].x = ...` and
 * `(n + 1)[recs].x = ...`.
 *
 * The fix: materialise the element address in a NAMED POINTER VARIABLE first --
 * `p = recs + j + 1;` then `p->t`, and `ins = recs + n + 1;` then `ins->x`.
 * That flips the operand order to base-first and matches. Confirmed at
 * function scope, at loop-block scope, and as `p = recs + j;` + `p[1].t`; NOT
 * fixed by `p = &recs[j + 1];` (the address-of-subscript spelling keeps the
 * index-first order and additionally costs distance 8).
 *
 * This file is the distance-2 version (subscript in both places) kept as the
 * marker for the mechanism; see candidate.c lever 4.
 */
void func_80033D38(void) {
    struct HitRec {
        u8 x;
        u8 y;
        s32 t;
    };
    struct HitRec *recs = (struct HitRec *)&D_80106A50;
    s32 n = 3;
    s32 j;
    s32 k;

    while (1) {
        j = n - 1;
        if (recs[j + 1].t < D_800A3858) {
            break;
        }
        n = j;
        if (n <= 0) {
            break;
        }
    }
    D_800A38E9 = (u8)n;
    if (n < 3) {
        for (k = 2; k > n; k--) {
            recs[k + 1] = recs[k];
        }
        recs[n + 1].x = (u8)D_80101ED2;
        recs[n + 1].y = (u8)D_80101ED6;
        recs[n + 1].t = D_800A3858;
    }
}
