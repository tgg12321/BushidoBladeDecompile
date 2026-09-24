/*
 * REJECTED (session 1) — declaration-order swap does NOT move the s0/s1 role
 * assignment.
 *
 * Target assigns s0 = p (the table pointer) and s1 = count. The natural C form
 * gets the opposite (s1 = p, s0 = count), even though both forms EMIT the two
 * `lui/addiu` address materializations in source order (p first). So the role
 * split is an allocator-priority outcome, not an emission-order outcome.
 *
 * Probe: declare `count` before `p` (below), leaving the assignment order in
 * the body unchanged.
 * Result: sandbox --disable all score 20, build_insns 25 — IDENTICAL to the
 * natural p-first order. No register moved. KILLED.
 *
 * Do not re-run this probe. If a future session attacks the register-role axis,
 * it needs a lever that changes reference counts or live ranges, not one that
 * changes declaration order. Note that this axis is subordinate to the frame
 * kill recorded in evidence.md H1 — closing the registers alone cannot produce
 * a match, because the frame size and all six save/restore offsets would still
 * differ.
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        p = &D_8008D070;
        count = (s32)&D_00000000;
        if (count != 0) {
            do {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            } while (count != 0);
        }
    }
}
