/* REJECTED -- func_80033D38 -- distance 24 (51 insns, FOUR over the 47-insn
 * target) despite having the correct two-variable structure.
 *
 * Mechanism: spelling the scan as `do { ... } while (n > 0);` makes GCC PEEL the
 * first array test out of the loop -- it emits `lui v0 / lw v0,28(v0)` on the
 * constant-folded address of recs[3].t ahead of the loop, then rotates so the
 * array compare (`beqz v0,<top>`) becomes the backedge instead of the `bgtz` on
 * n. Target does neither: its loop top IS the load and its backedge IS `bgtz a3`.
 *
 * Measured across the whole do/while sub-family at 24-25: with `j = n - 1;` last
 * in the body, with it first in the body, with `j--`, with a literal `j = 2;`
 * seed, with `n = j + 1;` seeding, with the bottom test on j (`while (j-- > 0)`),
 * and with the `for (j = n - 1; ; j = n - 1)` equivalent. Also in this bucket:
 * the `while (recs[j+1].t >= D_800A3858) { n = j; ... }` condition-in-the-test
 * form (25) and the goto-based bridge form the pinned HEAD body used (19).
 *
 * The winning spelling is `while (1)` with two `break`s -- see candidate.c
 * lever 3. Sibling trap recorded there: the exit test must be `n <= 0` (which
 * becomes the bgtz backedge); the arithmetically identical `n < 1` emits
 * slti+beqz and costs distance 25.
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

    do {
        j = n - 1;
        if (recs[j + 1].t < D_800A3858) {
            break;
        }
        n = j;
    } while (n > 0);
    D_800A38E9 = (u8)n;
    if (n < 3) {
        struct HitRec *ins;
        for (k = 2; k > n; k--) {
            recs[k + 1] = recs[k];
        }
        ins = recs + n + 1;
        ins->x = (u8)D_80101ED2;
        ins->y = (u8)D_80101ED6;
        ins->t = D_800A3858;
    }
}
