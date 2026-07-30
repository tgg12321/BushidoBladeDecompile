/* REJECTED -- func_80033D38 -- hard plateau at honest distance 9 (45 insns,
 * TWO instructions SHORT of the 47-insn target).
 *
 * Mechanism: with one loop variable, loop.c sees a single DEST_ADDR giv
 * (`recs[i].t`) over the biv i and strength-reduces it to a walking pointer --
 * `addiu v1,t1,24` in the preheader and `addiu v1,v1,-8` in the bgtz delay slot.
 * The in-loop `sll v0,v1,3` / `addu v0,t1,v0` index math that target keeps is
 * therefore never emitted, and no single-variable spelling can restore it.
 *
 * This is a PLATEAU, not one form: session s1 measured distance 9 for EIGHT
 * distinct single-variable spellings -- plain `for` (below), `for` with the
 * array test in the loop condition, `while (i > 0 && ...)`, `for (i=3;i>0;)`
 * with the decrement in the body, `while (1)` with two breaks, an in-body
 * `struct HitRec *p = &recs[i];`, a `s32 (*)[2]` two-dimensional view, and a
 * flat `s32 *` view indexed `slots[i * 2 + 1]`. All 45 insns, all distance 9.
 * Do not re-probe the single-variable family.
 *
 * The fix is structural, not a spelling: the source carries the scan index and
 * the insertion slot as two separate live variables -- see candidate.c lever 1.
 */
void func_80033D38(void) {
    struct HitRec {
        u8 x;
        u8 y;
        s32 t;
    };
    struct HitRec *recs = (struct HitRec *)&D_80106A50;
    s32 i;
    s32 k;

    for (i = 3; i > 0; i--) {
        if (recs[i].t < D_800A3858) {
            break;
        }
    }
    D_800A38E9 = (u8)i;
    if (i < 3) {
        for (k = 2; k > i; k--) {
            recs[k + 1] = recs[k];
        }
        recs[i + 1].x = (u8)D_80101ED2;
        recs[i + 1].y = (u8)D_80101ED6;
        recs[i + 1].t = D_800A3858;
    }
}
